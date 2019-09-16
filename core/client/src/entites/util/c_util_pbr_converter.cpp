#include "stdafx_client.h"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/math/intersection.h>
#include <pragma/physics/collisionmesh.h>
#include <sharedutils/util_file.h>
#include <image/prosper_image.hpp>
#include <image/prosper_render_target.hpp>
#include <image/prosper_sampler.hpp>
#include <prosper_util.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/util/c_util_pbr_converter.hpp"
#include "pragma/rendering/shaders/util/c_shader_specular_to_roughness.hpp"
#include "pragma/rendering/shaders/util/c_shader_extract_diffuse_ambient_occlusion.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pr_dds.hpp"

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

using namespace pragma;

LINK_ENTITY_TO_CLASS(util_pbr_converter,CUtilPBRConverter);

#pragma optimize("",off)
namespace pragma
{
	struct PBRConverterMaterialMeshData
	{
		std::vector<MaterialHandle> hMaterials = {};
		std::vector<Vector3> vertices = {};
		std::vector<Vector2> uvs = {};
		std::vector<uint16_t> indices = {};

		bool generateAoMapData = false;
		bool generateNormalMapData = false;

		uint32_t imageResolution = 0u;

		std::vector<std::array<float,4>> aoMapPixelData = {};
		std::vector<std::array<float,4>> normalMapPixelData = {};
	};
	struct PBRConverterModelData
	{
		ModelHandle hModel = {};
		std::vector<std::unique_ptr<PBRConverterMaterialMeshData>> materialMeshes = {};
	};
};

luabind::object CPBRConverterComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CPBRConverterComponentHandleWrapper>(l);}

void CPBRConverterComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	auto &ent = GetEntity();
	ent.AddComponent<LogicComponent>();
	BindEventUnhandled(LogicComponent::EVENT_ON_TICK,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		PollEvents();
	});

	auto libGpl = client->InitializeLibrary("pr_gpl");
	if(libGpl == nullptr)
	{
		GetEntity().RemoveSafely();
		return;
	}
	auto *fCalcGeometryData = libGpl->FindSymbolAddress<void(*)(const std::vector<Vector3>&,const std::vector<uint16_t>&,std::vector<float>*,std::vector<Vector3>*,uint32_t)>("pr_gpl_calc_geometry_data");
	if(fCalcGeometryData == nullptr)
	{
		GetEntity().RemoveSafely();
		return;
	}
	m_fCalcGeometryData = fCalcGeometryData;
}

void CPBRConverterComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	if(m_cbOnModelLoaded.IsValid())
		m_cbOnModelLoaded.Remove();
	if(m_cbOnMaterialLoaded.IsValid())
		m_cbOnMaterialLoaded.Remove();
	for(auto &pair : m_onModelMaterialsLoadedCallbacks)
	{
		auto &cb = pair.second;
		if(cb.IsValid())
			cb.Remove();
	}
	EndThread();
}

void CPBRConverterComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();

	m_cbOnModelLoaded = c_game->AddCallback("OnModelLoaded",FunctionCallback<void,std::reference_wrapper<std::shared_ptr<Model>>>::Create([this](std::reference_wrapper<std::shared_ptr<Model>> mdl) {
		auto pMdl = mdl.get();
		auto cb = mdl.get()->CallOnMaterialsLoaded([this,pMdl]() {
			GenerateGeometryBasedTextures(*pMdl);
			auto it = m_onModelMaterialsLoadedCallbacks.find(pMdl.get());
			if(it != m_onModelMaterialsLoadedCallbacks.end())
			{
				auto &cb = it->second;
				if(cb.IsValid())
					cb.Remove();
				m_onModelMaterialsLoadedCallbacks.erase(it);
			}
		});
		if(cb.IsValid())
			m_onModelMaterialsLoadedCallbacks.insert(std::make_pair(mdl.get().get(),cb));
	}));
	m_cbOnMaterialLoaded = client->AddCallback("OnMaterialLoaded",FunctionCallback<void,CMaterial*>::Create([this](CMaterial *mat) {
		if(ShouldConvertMaterial(*mat) == false)
			return;
		ConvertToPBR(*mat);
	}));

	for(auto &pair : client->GetMaterialManager().GetMaterials())
	{
		auto &hMat = pair.second;
		if(hMat.IsValid() == false || hMat.get()->IsLoaded() == false || ShouldConvertMaterial(static_cast<CMaterial&>(*hMat.get())) == false)
			continue;
		ConvertToPBR(static_cast<CMaterial&>(*hMat.get()));
	}

	auto &models = c_game->GetModels();
	for(auto &pair : models)
	{
		auto &mdl = pair.second;
		GenerateGeometryBasedTextures(*mdl);
	}

	StartThread();
}
bool CPBRConverterComponent::ShouldConvertMaterial(CMaterial &mat) const
{
	auto shader = mat.GetShaderIdentifier();
	ustring::to_lower(shader);
	return (shader == "textured" || shader == "texturedalphatransition") && shader != "pbr";
}
void CPBRConverterComponent::StartThread()
{
	m_running = true;
	m_worker = std::thread{[this]() {
		while(m_running)
		{
			std::this_thread::sleep_for(std::chrono::seconds{1});
			if(m_hasWork)
			{
				m_workQueueMutex.lock();
				auto currentItem = m_workQueue.front();
				m_workQueue.pop();
				if(m_workQueue.empty())
					m_hasWork = false;
				m_workQueueMutex.unlock();

				constexpr uint32_t resolution = 1024u;
				constexpr uint32_t sampleCount = 512u;
				for(auto &matMesh : currentItem->materialMeshes)
				{
					if(m_running == false)
						goto endThread;
					std::vector<float> aoValues;
					std::vector<Vector3> normals;
					m_fCalcGeometryData(matMesh->vertices,matMesh->indices,&aoValues,&normals,sampleCount);

					if(m_running == false)
						goto endThread;

					matMesh->imageResolution = resolution;
					if(matMesh->generateAoMapData)
					{
						GenerateImageDataFromGeometryData(*matMesh,resolution,[&aoValues](uint16_t idx0,uint16_t idx1,uint16_t idx2,const std::array<float,3> &area,std::array<float,4> &pixelData) {
							auto ao = area.at(0) *aoValues.at(idx0) +area.at(1) *aoValues.at(idx1) +area.at(2) *aoValues.at(idx2);
							pixelData = {ao,ao,ao,1.f};
						},matMesh->aoMapPixelData);
					}

					if(m_running == false)
						goto endThread;

					if(matMesh->generateNormalMapData)
					{
						auto numVerts = matMesh->vertices.size();
						std::vector<Vector3> tangents {};
						std::vector<Vector3> biTangents {};
						tangents.resize(numVerts);
						biTangents.resize(numVerts);
						for(unsigned int i=0;i<matMesh->indices.size();i+=3)
						{
						auto &v0 = matMesh->vertices[matMesh->indices[i]];
						auto &v1 = matMesh->vertices[matMesh->indices[i +1]];
						auto &v2 = matMesh->vertices[matMesh->indices[i +2]];

						auto deltaPos1 = v1 -v0;
						auto deltaPos2 = v2 -v0;

						auto deltaUV1 = matMesh->uvs[matMesh->indices[i +1]] -matMesh->uvs[matMesh->indices[i]];
						auto deltaUV2 = matMesh->uvs[matMesh->indices[i +2]] -matMesh->uvs[matMesh->indices[i]];
						//auto deltaUV1 = Vector2(v1.uv.x,1.f -v1.uv.y) -Vector2(v0.uv.x,1.f -v0.uv.y);
						//auto deltaUV2 = Vector2(v2.uv.x,1.f -v2.uv.y) -Vector2(v0.uv.x,1.f -v0.uv.y);
						//auto deltaUV1 = Vector2(1.f -v1.uv.x,1.f -v1.uv.y) -Vector2(1.f -v0.uv.x,1.f -v0.uv.y);
						//auto deltaUV2 = Vector2(1.f -v2.uv.x,1.f -v2.uv.y) -Vector2(1.f -v0.uv.x,1.f -v0.uv.y);

						auto d = deltaUV1.x *deltaUV2.y -deltaUV1.y *deltaUV2.x;
						auto r = (d != 0.f) ? (1.f /d) : 0.f;
						auto tangent = (deltaPos1 *deltaUV2.y -deltaPos2 *deltaUV1.y) *r;
						auto biTangent = (deltaPos2 *deltaUV1.x -deltaPos1 *deltaUV2.x) *r;
						uvec::normalize(&biTangent);

						tangents.at(matMesh->indices[i]) = tangent;
						tangents.at(matMesh->indices[i +1]) = tangent;
						tangents.at(matMesh->indices[i +2]) = tangent;

						biTangents.at(matMesh->indices[i]) = biTangent;
						biTangents.at(matMesh->indices[i +1]) = biTangent;
						biTangents.at(matMesh->indices[i +2]) = biTangent;
						}
						std::vector<Vector3> tangentSpaceNormals {};
						tangentSpaceNormals.resize(numVerts);
						for(unsigned int i=0;i<matMesh->indices.size();i++)
						{
							auto &v = matMesh->vertices[matMesh->indices[i]];
							auto &t = tangents[matMesh->indices[i]];
							auto &b = biTangents[matMesh->indices[i]];

							auto &n = normals[matMesh->indices[i]];
							t = t -n *glm::dot(n,t);
							uvec::normalize(&t);

							if(glm::dot(glm::cross(n,t),b) < 0.f)
								t *= -1.f;

							Mat3 TBN {
								t.x,t.y,t.z,
								n.x,n.y,n.z,
								b.x,b.y,b.z,
							};
							auto tbnNormal = TBN *n;

							tangentSpaceNormals[matMesh->indices[i]] = tbnNormal;
						}



						GenerateImageDataFromGeometryData(*matMesh,resolution,[&tangentSpaceNormals](uint16_t idx0,uint16_t idx1,uint16_t idx2,const std::array<float,3> &area,std::array<float,4> &pixelData) {
							auto n = area.at(0) *tangentSpaceNormals.at(idx0) +area.at(1) *tangentSpaceNormals.at(idx1) +area.at(2) *tangentSpaceNormals.at(idx2);

							pixelData = {
								(n.x +1.f) /2.f,
								(n.y +1.f) /2.f,
								(n.z +1.f) /2.f,
								1.f
							};
						},matMesh->normalMapPixelData);
					}
				}

				// All AO maps have been generated; Add to event queue for main thread
				m_completeQueueMutex.lock();
				m_completeQueue.push(currentItem);
				m_completeQueueMutex.unlock();
			}
		}
	endThread:
		;
	}};
}
void CPBRConverterComponent::GenerateImageDataFromGeometryData(
	const pragma::PBRConverterMaterialMeshData &meshData,uint32_t resolution,
	std::function<void(uint16_t,uint16_t,uint16_t,const std::array<float,3>&,std::array<float,4>&)> applyPixelData,
	std::vector<std::array<float,4>> &outPixelData
)
{
	auto &pixelData = outPixelData;
	auto &tris = meshData.indices;
	auto &verts = meshData.vertices;
	auto &uvs = meshData.uvs;

	auto width = resolution;
	auto height = resolution;
	auto numPixels = width *height;
	pixelData.resize(numPixels);

	// 3D points for each pixel in the image
	struct UVPointInfo
	{
		int32_t triangleIndex = -1;
		std::array<float,3> area = {0.f,0.f,0.f};
	};
	std::vector<UVPointInfo> uvPoints {};
	uvPoints.resize(numPixels,UVPointInfo{});

	for(auto x=decltype(width){0u};x<width;++x)
	{
		for(auto y=decltype(height){0u};y<height;++y)
		{
			Vector2 uv {
				x /static_cast<float>(width),
				y /static_cast<float>(height)
			};
			auto pxOffset = y *width +x;
			for(auto i=decltype(tris.size()){0u};i<tris.size();i+=3)
			{
				auto idx0 = tris.at(i);
				auto idx1 = tris.at(i +1);
				auto idx2 = tris.at(i +2);
				auto &v0 = verts.at(idx0);
				auto &v1 = verts.at(idx1);
				auto &v2 = verts.at(idx2);

				auto &uv0 = uvs.at(idx0);
				auto &uv1 = uvs.at(idx1);
				auto &uv2 = uvs.at(idx2);

				float a0,a1,a2;
				if(Geometry::calc_barycentric_coordinates(uv0,uv1,uv2,uv,a0,a1,a2) == false)
					continue; // UV coordinates do not lie on this triangle
				uvPoints.at(pxOffset) = {static_cast<int32_t>(i),{a0,a1,a2}};
				break;
			}
		}
	}

	for(auto x=decltype(width){0u};x<width;++x)
	{
		for(auto y=decltype(height){0u};y<height;++y)
		{
			auto pxOffset = y *width +x;
			auto &uvPoint = uvPoints.at(pxOffset);
			if(uvPoint.triangleIndex == -1)
				continue; // These uv coordinates are not used by the mesh
			auto idx0 = tris.at(uvPoint.triangleIndex);
			auto idx1 = tris.at(uvPoint.triangleIndex +1);
			auto idx2 = tris.at(uvPoint.triangleIndex +2);

			applyPixelData(idx0,idx1,idx2,uvPoint.area,pixelData.at(pxOffset));
		}
	}
}
void CPBRConverterComponent::EndThread()
{
	m_running = false;
	if(m_worker.joinable())
		m_worker.join();
}

void CPBRConverterComponent::GenerateGeometryBasedTextures(Model &mdl)
{
	using MaterialIndex = uint32_t;
	struct MaterialMeshes
	{
		std::vector<ModelSubMesh*> meshes = {};
		bool generateAoMapData = false;
		bool generateNormalMapData = false;
		bool updateMetalness = false;
	};
	// Note: One material may belong to multiple meshes (e.g. body-groups).
	// We'll collect all of them (but only the ones for LOD 0) and treat them
	// as a single mesh.
	std::unordered_map<MaterialIndex,MaterialMeshes> materialToSubMesh {};
	std::unordered_set<uint32_t> baseMeshes {};
	for(auto idx : mdl.GetBaseMeshes())
		baseMeshes.insert(idx);
	for(auto &bg : mdl.GetBodyGroups())
	{
		for(auto idx : bg.meshGroups)
			baseMeshes.insert(idx);
	}
	for(auto meshGroupIdx : baseMeshes)
	{
		auto meshGroup = mdl.GetMeshGroup(meshGroupIdx);
		if(meshGroup == nullptr)
			continue;
		for(auto &mesh : meshGroup->GetMeshes())
		{
			for(auto &subMesh : mesh->GetSubMeshes())
			{
				auto matIdx = subMesh->GetTexture();
				auto *mat = mdl.GetMaterial(matIdx);
				if(mat == nullptr)
					continue;
				auto *normalMap = mat->GetNormalMap();
				auto *aoMap = mat->GetAmbientOcclusionMap();
				auto *metalnessMap = mat->GetMetalnessMap();
				// Note: Generating normal map currently has wrong results, and would not
				// provide any advantage over regular vertex normals anyway.
				// This behavior is therefore disabled for now.
				auto genNormalMap = false; //(!normalMap || !normalMap->texture || std::static_pointer_cast<Texture>(normalMap->texture)->IsError());
				auto genAoMap = (!aoMap || !aoMap->texture || std::static_pointer_cast<Texture>(aoMap->texture)->IsError());
				auto updateMetalness = (!metalnessMap || !metalnessMap->texture || std::static_pointer_cast<Texture>(metalnessMap->texture)->IsError());
				if(genNormalMap == false && genAoMap == false && updateMetalness == false)
					continue; // Model already has a normal and an ambient occlusion map; We don't need to do anything
				auto it = materialToSubMesh.find(matIdx);
				if(it == materialToSubMesh.end())
					it = materialToSubMesh.insert(std::make_pair(matIdx,MaterialMeshes{})).first;

				auto &matMeshes = it->second;
				matMeshes.generateAoMapData = genAoMap;
				matMeshes.generateNormalMapData = genNormalMap;
				matMeshes.updateMetalness = updateMetalness;
				// Make sure the mesh is only added once
				auto itMesh = std::find_if(matMeshes.meshes.begin(),matMeshes.meshes.end(),[&subMesh](const ModelSubMesh *subMeshOther) {return subMeshOther == subMesh.get();});
				if(itMesh == matMeshes.meshes.end())
					matMeshes.meshes.push_back(subMesh.get());
			}
		}
	}

	auto modelData = std::make_shared<pragma::PBRConverterModelData>();
	modelData->materialMeshes.reserve(materialToSubMesh.size());
	modelData->hModel = mdl.GetHandle();
	for(auto &pair : materialToSubMesh)
	{
		std::vector<Material*> materials {};
		auto matIdx = pair.first;
		auto *mat = mdl.GetMaterial(matIdx);
		if(mat)
			materials.push_back(mat);
		for(auto &texGroup : mdl.GetTextureGroups())
		{
			if(matIdx >= texGroup.textures.size())
				continue;
			auto skinMatIdx = texGroup.textures.at(matIdx);
			auto *mat = mdl.GetMaterial(skinMatIdx);
			if(mat)
				materials.push_back(mat);
		}
		if(materials.empty())
			continue;
		auto matMesh = std::make_unique<pragma::PBRConverterMaterialMeshData>();
		matMesh->generateAoMapData = pair.second.generateAoMapData;
		matMesh->generateNormalMapData = pair.second.generateNormalMapData;
		matMesh->hMaterials.reserve(materials.size());
		for(auto *mat : materials)
			matMesh->hMaterials.push_back(mat->GetHandle());

		// Build mesh from all sub-meshes using this material
		auto &verts = matMesh->vertices;
		auto &uvs = matMesh->uvs;
		auto &tris = matMesh->indices;
		uint32_t numVerts = 0u;
		uint32_t numTris = 0u;
		for(auto *mesh : pair.second.meshes)
		{
			numVerts += mesh->GetVertexCount();
			numTris += mesh->GetTriangleCount();
		}
		verts.reserve(numVerts);
		uvs.reserve(numVerts);
		tris.reserve(numTris);
		for(auto *mesh : pair.second.meshes)
		{
			for(auto &v : mesh->GetVertices())
			{
				verts.push_back(v.position);
				uvs.push_back(v.uv);
			}
			for(auto idx : mesh->GetTriangles())
				tris.push_back(idx);
		}


		if(pair.second.updateMetalness)
		{
			// Material has no surface material. To find out whether it is a metal material,
			// we'll try to find a collision mesh near the visual mesh, and check its surface material
			// instead. If its metal, we'll assume the visual material is metal as well.
			Vector3 meshCenter {};
			for(auto &v : matMesh->vertices)
				meshCenter += v;
			auto numVerts = matMesh->vertices.size();
			meshCenter /= static_cast<float>(numVerts);

			auto dClosest = std::numeric_limits<float>::max();
			CollisionMesh *colMeshClosest = nullptr;
			for(auto &colMesh : mdl.GetCollisionMeshes())
			{
				auto boneId = colMesh->GetBoneParent();
				auto tBone = mdl.CalcReferenceBonePose(boneId);
				pragma::physics::Transform t {};
				if(tBone.has_value())
					t = *tBone;

				Vector3 colMeshCenter {};
				auto &verts = colMesh->GetVertices();
				for(auto &v : verts)
					colMeshCenter += v;
				colMeshCenter /= static_cast<float>(verts.size());
				
				colMeshCenter = t *colMeshCenter;
				auto d = uvec::distance_sqr(meshCenter,colMeshCenter);
				if(d < dClosest)
				{
					dClosest = d;
					colMeshClosest = colMesh.get();
				}
			}
			auto metalness = false;
			std::string surfMatName = "undefined";
			if(colMeshClosest)
			{
				auto surfMatIdx = colMeshClosest->GetSurfaceMaterial();
				auto &surfMats = colMeshClosest->GetSurfaceMaterials();
				if(surfMatIdx == -1 && surfMats.empty() == false)
					surfMatIdx = surfMats.front();
				auto *surfMat = c_game->GetSurfaceMaterial(surfMatIdx);
				if(surfMat)
				{
					auto name = surfMat->GetIdentifier();
					surfMatName = name;
					ustring::to_lower(name);
					if(name.find("metal") != std::string::npos)
						metalness = true;
				}
			}
			for(auto *mat : materials)
			{
				if(mat == nullptr)
					continue;
				if(ShouldConvertMaterial(static_cast<CMaterial&>(*mat)) == false || ConvertToPBR(static_cast<CMaterial&>(*mat)) == true)
				{
					Con::cout<<"Assigning "<<(metalness ? "metalness" : "non-metalness")<<" texture to material '"<<mat->GetName()<<"', based on surface material '"<<surfMatName<<"' of model '"<<mdl.GetName()<<"'!"<<Con::endl;
					if(metalness)
						mat->GetDataBlock()->AddValue("texture",Material::METALNESS_MAP_IDENTIFIER,"pbr/metal"); // 100% metal
					else
						mat->GetDataBlock()->AddValue("texture",Material::METALNESS_MAP_IDENTIFIER,"pbr/nonmetal"); // 0% metal
					mat->UpdateTextures();
					mat->Save(mat->GetName(),"addons/converted/");
				}
			}
		}
		if(pair.second.generateAoMapData == false && pair.second.generateNormalMapData == false)
			continue; // Nothing else to update!
		modelData->materialMeshes.emplace_back(std::move(matMesh));
	}
	if(modelData->materialMeshes.empty())
		return; // Nothing to be done for this model
	Con::cout<<"Generating PBR textures for model '"<<mdl.GetName()<<"'..."<<Con::endl;

	m_workQueueMutex.lock();
	m_workQueue.push(modelData);
	m_workQueueMutex.unlock();
	m_hasWork = true;
}
void CPBRConverterComponent::PollEvents()
{
	m_completeQueueMutex.lock();
	while(m_completeQueue.empty() == false)
	{
		auto item = m_completeQueue.front();
		m_completeQueue.pop();

		if(item->hModel.IsValid() == false)
			continue;
		Con::cout<<"PBR texture generation for model '"<<item->hModel->GetName()<<"' has been completed! Saving texture files..."<<Con::endl;
		for(auto &matMesh : item->materialMeshes)
		{
			// Mesh info may contain multiple materials. In this case, the first one is the base material and
			// all others are alternative skins.
			auto hMatMain = matMesh->hMaterials.empty() ? MaterialHandle{} : matMesh->hMaterials.front();
			if(hMatMain.IsValid() == false)
				continue;
			auto &dev = c_engine->GetDevice();
			prosper::util::ImageCreateInfo createInfo {};
			createInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::CPUToGPU;
			createInfo.format = Anvil::Format::R32G32B32A32_SFLOAT; // TODO: 16 bit should probably be enough for ambient occlusion values?
			createInfo.postCreateLayout = Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL;
			createInfo.width = matMesh->imageResolution;
			createInfo.height = matMesh->imageResolution;
			createInfo.flags = prosper::util::ImageCreateInfo::Flags::None;
			createInfo.usage = Anvil::ImageUsageFlagBits::SAMPLED_BIT | Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT | Anvil::ImageUsageFlagBits::TRANSFER_SRC_BIT;
			createInfo.tiling = Anvil::ImageTiling::LINEAR;
			prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
			prosper::util::SamplerCreateInfo samplerCreateInfo {};

			std::string materialsRootDir = "materials/";
			auto matName = materialsRootDir +hMatMain->GetName();
			ufile::remove_extension_from_filename(matName);

			ImageWriteInfo imgWriteInfo {};
			imgWriteInfo.inputFormat = ImageWriteInfo::InputFormat::R32G32B32A32_Float;
			imgWriteInfo.outputFormat = ImageWriteInfo::OutputFormat::GradientMap;
			imgWriteInfo.flags |= ImageWriteInfo::Flags::GenerateMipmaps | ImageWriteInfo::Flags::SRGB;
			std::string aoName = "";
			if(matMesh->aoMapPixelData.empty() == false)
			{
				auto imgAo = prosper::util::create_image(dev,createInfo,reinterpret_cast<uint8_t*>(matMesh->aoMapPixelData.data()));
				aoName = matName +"_ao";
				Con::cout<<"Writing ambient occlusion map '"<<aoName<<"'..."<<Con::endl;
				if(c_game->SaveImage(*imgAo,"addons/converted/" +aoName,imgWriteInfo) == false)
					aoName = "";
			}
			imgWriteInfo.SetNormalMap();
			// imgWriteInfo.flags |= ImageWriteInfo::Flags::ConvertToNormalMap;
			std::string normalName = "";
			if(matMesh->normalMapPixelData.empty() == false)
			{
				auto imgNormal = prosper::util::create_image(dev,createInfo,reinterpret_cast<uint8_t*>(matMesh->normalMapPixelData.data()));
				normalName = matName +"_n";
				Con::cout<<"Writing normal map '"<<normalName<<"'..."<<Con::endl;
				if(c_game->SaveImage(*imgNormal,"addons/converted/" +normalName,imgWriteInfo) == false)
					normalName = "";
			}
			for(auto &hMat : matMesh->hMaterials)
			{
				if(hMat.IsValid() == false)
					continue;
				auto &dataBlock = hMat->GetDataBlock();
				if(aoName.empty() == false)
					dataBlock->AddValue("texture",Material::AO_MAP_IDENTIFIER,aoName.substr(materialsRootDir.length()));
				if(normalName.empty() == false)
					dataBlock->AddValue("texture",Material::NORMAL_MAP_IDENTIFIER,normalName.substr(materialsRootDir.length()));
				hMat->UpdateTextures();
				if(hMat->Save(hMat->GetName(),"addons/converted/"))
				{
					auto nameNoExt = hMat->GetName();
					ufile::remove_extension_from_filename(nameNoExt);
					client->LoadMaterial(nameNoExt,true,true); // Reload material immediately
				}
			}
		}
	}
	m_completeQueueMutex.unlock();
}

bool CPBRConverterComponent::ConvertToPBR(CMaterial &matTraditional)
{
	Con::cout<<"Converting material '"<<matTraditional.GetName()<<"' to PBR..."<<Con::endl;
	auto &setupCmd = c_engine->GetSetupCommandBuffer();
	auto &dev = c_engine->GetDevice();

	auto *matPbr = client->CreateMaterial("pbr");
	auto &dataPbr = matPbr->GetDataBlock();

	auto matName = matTraditional.GetName();
	ufile::remove_extension_from_filename(matName);

	// Albedo map
	// TODO: Remove ambient occlusion from diffuse map, if possible
	auto *diffuseMap = matTraditional.GetDiffuseMap();
	if(diffuseMap && diffuseMap->texture && std::static_pointer_cast<Texture>(diffuseMap->texture)->texture)
		dataPbr->AddValue("texture",Material::ALBEDO_MAP_IDENTIFIER,diffuseMap->name);
	//

	// Roughness map
	auto *specularMap = matTraditional.GetSpecularMap();
	if(specularMap && specularMap->texture && std::static_pointer_cast<Texture>(specularMap->texture)->texture)
	{
		auto roughnessMap = ConvertSpecularMapToRoughness(*std::static_pointer_cast<Texture>(specularMap->texture)->texture);
		if(roughnessMap)
		{
			auto roughnessName = matName +"_roughness";
			ImageWriteInfo imgWriteInfo {};
			imgWriteInfo.inputFormat = ImageWriteInfo::InputFormat::R8G8B8A8_UInt;
			imgWriteInfo.outputFormat = ImageWriteInfo::OutputFormat::GradientMap;
			imgWriteInfo.flags |= ImageWriteInfo::Flags::GenerateMipmaps | ImageWriteInfo::Flags::SRGB;
			c_game->SaveImage(*roughnessMap->GetImage(),"addons/converted/materials/" +roughnessName,imgWriteInfo);
			dataPbr->AddValue("texture",Material::ROUGHNESS_MAP_IDENTIFIER,roughnessName);
		}
	}
	else
		dataPbr->AddValue("texture",Material::ROUGHNESS_MAP_IDENTIFIER,"pbr/rough"); // Generic roughness map with 100% roughness
	//

	// Normal map
	auto bGenerateNormalMap = false;
	auto *normalMap = matTraditional.GetNormalMap();
	if(normalMap && normalMap->texture && std::static_pointer_cast<Texture>(normalMap->texture)->texture)
		dataPbr->AddValue("texture",Material::NORMAL_MAP_IDENTIFIER,normalMap->name);
	//

	// Ambient occlusion map
	auto bGenerateAOMap = false;
	auto *aoMap = matTraditional.GetAmbientOcclusionMap();
	if(aoMap && aoMap->texture && std::static_pointer_cast<Texture>(aoMap->texture)->texture)
		dataPbr->AddValue("texture",Material::AO_MAP_IDENTIFIER,aoMap->name);
	//

	// Metalness map
	auto valSurfMat = dataPbr->GetDataValue("surfacematerial");
	if(valSurfMat)
	{
		// Attempt to determine whether this is a metal material or not
		std::string strVal = valSurfMat->GetString();
		ustring::to_lower(strVal);
		if(strVal.find("metal") != std::string::npos)
			dataPbr->AddValue("texture",Material::METALNESS_MAP_IDENTIFIER,"pbr/metal"); // 100% metalness
	}
	// Note: If no surface material could be found in the material,
	// the model's surface material will be checked as well in 'GenerateGeometryBasedTextures'.
	//

	// Emission map
	auto *glowMap = matTraditional.GetGlowMap();
	if(glowMap && glowMap->texture && std::static_pointer_cast<Texture>(glowMap->texture)->texture)
	{
		dataPbr->AddValue("texture",Material::EMISSION_MAP_IDENTIFIER,glowMap->name);
		dataPbr->AddValue("bool","glow_alpha_only","1");
		dataPbr->AddValue("float","glow_scale","1.0");
		dataPbr->AddValue("int","glow_blend_diffuse_mode","1");
		dataPbr->AddValue("float","glow_blend_diffuse_scale","3.0");
	}
	//

	matPbr->UpdateTextures();
	// Overwrite old material with new PBR settings
	if(matPbr->Save(matTraditional.GetName(),"addons/converted/"))
		client->LoadMaterial(matName,true,true); // Reload material immediately
	Con::cout<<"Conversion complete!"<<Con::endl;

	/*
	// Diffuse descriptor set
	auto dsgDiffuse = prosper::util::create_descriptor_set_group(dev,pragma::ShaderExtractDiffuseAmbientOcclusion::DESCRIPTOR_SET_TEXTURE);
	prosper::util::set_descriptor_set_binding_texture(*(*dsgDiffuse)->get_descriptor_set(0u),diffuseMap,0u);

	// Initialize ao image
	// TODO: Use existing AO map if available
	prosper::util::ImageCreateInfo createInfoAo {};
	diffuseMap.GetImage()->GetCreateInfo(createInfoAo);
	createInfoAo.format = Anvil::Format::R8G8B8A8_UNORM;
	createInfoAo.postCreateLayout = Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL;
	createInfoAo.usage = Anvil::ImageUsageFlagBits::SAMPLED_BIT | Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT;
	auto aoMap = prosper::util::create_image(dev,createInfoAo);
	auto aoTex = prosper::util::create_texture(dev,{},aoMap,&imgViewCreateInfo,&samplerCreateInfo);
	auto aoRt = prosper::util::create_render_target(dev,{aoTex},shaderExtractAo->GetRenderPass());

	// Extract ambient occlusion
	if(prosper::util::record_begin_render_pass(**setupCmd,*aoRt) == true)
	{
	if(shaderExtractAo->BeginDraw(setupCmd) == true)
	{
	shaderExtractAo->Draw(*(*dsgDiffuse)->get_descriptor_set(0u));
	shaderExtractAo->EndDraw();
	}
	prosper::util::record_end_render_pass(**setupCmd);
	}
	c_engine->FlushSetupCommandBuffer();
	*/
	return true;
}
std::shared_ptr<prosper::Texture> CPBRConverterComponent::ConvertSpecularMapToRoughness(prosper::Texture &specularMap)
{
	auto *shaderSpecularToRoughness = static_cast<pragma::ShaderSpecularToRoughness*>(c_engine->GetShader("specular_to_roughness").get());
	if(shaderSpecularToRoughness == nullptr)
		return nullptr;
	auto &setupCmd = c_engine->GetSetupCommandBuffer();
	auto &dev = c_engine->GetDevice();
	// Specular descriptor set
	auto dsgSpecular = prosper::util::create_descriptor_set_group(dev,pragma::ShaderSpecularToRoughness::DESCRIPTOR_SET_TEXTURE);
	prosper::util::set_descriptor_set_binding_texture(*(*dsgSpecular)->get_descriptor_set(0u),specularMap,0u);

	// Initialize roughness image
	prosper::util::ImageCreateInfo createInfoRoughness {};
	specularMap.GetImage()->GetCreateInfo(createInfoRoughness);
	createInfoRoughness.format = Anvil::Format::R8G8B8A8_UNORM;
	createInfoRoughness.postCreateLayout = Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL;
	createInfoRoughness.usage = Anvil::ImageUsageFlagBits::SAMPLED_BIT | Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT;
	auto roughnessMap = prosper::util::create_image(dev,createInfoRoughness);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	auto roughnessTex = prosper::util::create_texture(dev,{},roughnessMap,&imgViewCreateInfo,&samplerCreateInfo);
	auto roughnessRt = prosper::util::create_render_target(dev,{roughnessTex},shaderSpecularToRoughness->GetRenderPass());

	// Specular to roughness
	if(prosper::util::record_begin_render_pass(**setupCmd,*roughnessRt) == true)
	{
		if(shaderSpecularToRoughness->BeginDraw(setupCmd) == true)
		{
			shaderSpecularToRoughness->Draw(*(*dsgSpecular)->get_descriptor_set(0u));
			shaderSpecularToRoughness->EndDraw();
		}
		prosper::util::record_end_render_pass(**setupCmd);
	}
	c_engine->FlushSetupCommandBuffer();
	return roughnessTex;
}

////////

void CUtilPBRConverter::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CPBRConverterComponent>();
}
#pragma optimize("",on)
