#include "mdl.h"
#include "wv_source.hpp"
#include "source2/source2.hpp"
#include "source2/pr_impl.hpp"
#include <util_source2.hpp>
#include <source2/resource.hpp>
#include <source2/resource_data.hpp>
#include <source2/resource_edit_info.hpp>
#include <fsys/filesystem.h>
#include <util_archive.hpp>
#include <sharedutils/util_string.h>
#include <sharedutils/util_path.hpp>
#include <pragma/model/vertex.h>
#include <pragma/model/model.h>
#include <pragma/model/modelmesh.h>
#include <pragma/physics/collisionmesh.h>
#include <pragma/networkstate/networkstate.h>
#include <pragma/game/game_resources.hpp>
#include <pragma/game/game.h>
#include <pragma/asset_types/world.hpp>
#include <util_image_buffer.hpp>
#include <util_image.hpp>
#include <unordered_set>

#pragma optimize("",off)
static uint32_t add_material(NetworkState &nw,Model &mdl,const std::string &mat,std::optional<uint32_t> skinId={})
{
	::util::Path path{mat};
	path.RemoveFileExtension();
	path += ".vmat_c";

	std::string inputPath = path.GetString();
	auto front = path.GetFront();
	path.PopFront();

	auto outputPath = path;
	if(ustring::compare(front,"materials",false) == false)
		outputPath = "models/" +outputPath;

	// Materials in Source 2 can be in arbitrary locations (relative to the game), not necessarily
	// in the materials directory. For this reason we'll have to port the material immediately and then
	// change its path for Pragma.
	::util::port_file(&nw,inputPath,"materials/" +outputPath.GetString());

	auto strPath = outputPath.GetPath();
	auto strFile = outputPath.GetFileName();
	mdl.AddTexturePath(strPath);

	if(skinId.has_value())
	{
		auto *texGroup = mdl.GetTextureGroup(*skinId);
		if(texGroup == nullptr)
			texGroup = mdl.CreateTextureGroup();
		return import::util::add_texture(nw,mdl,strFile,texGroup,true);
	}
	auto *texGroup = mdl.GetTextureGroup(0);
	if(texGroup)
	{
		auto n = texGroup->textures.size();
		auto idx = import::util::add_texture(nw,mdl,strFile,texGroup);
		if(texGroup->textures.size() > n) // Texture was added, so we'll have to add it to the other texture groups as well
		{
			auto &texGroups = mdl.GetTextureGroups();
			for(auto i=decltype(texGroups.size()){1u};i<texGroups.size();++i)
				import::util::add_texture(nw,mdl,strFile,&texGroups.at(i),true);
		}
		return idx;
	}
	return 0;
}

static std::vector<std::shared_ptr<ModelSubMesh>> generate_split_meshes(NetworkState &nw,const source2::impl::MeshData &meshData,const std::vector<uint64_t> &indices)
{
	constexpr auto MAX_VERTEX_COUNT_PER_MESH = std::numeric_limits<uint16_t>::max();
	// Pragma doesn't support a vertex count > MAX_VERTEX_COUNT_PER_MESH,
	// so we'll have to split the mesh
	struct SplitMesh
	{
		std::vector<Vertex> verts {};
		std::vector<VertexWeight> vertWeights {};
		std::vector<Vector2> lightmapUvs {};
	};
	std::vector<SplitMesh> splitMeshes {};

	auto hasVertexWeights = (meshData.vertWeights.empty() == false);
	auto hasLightmapUvs = (meshData.lightmapUvs.empty() == false);

	for(auto i=decltype(indices.size()){0u};i<indices.size();i+=3)
	{
		auto idx0 = indices.at(i);
		auto idx1 = indices.at(i +1);
		auto idx2 = indices.at(i +2);

	}

	auto numVerts = meshData.verts.size();
	auto &prVerts = meshData.verts;
	auto &vertWeights = meshData.vertWeights;
	auto &lightmapUvs = meshData.lightmapUvs;
	auto numSplitMeshes = numVerts /std::numeric_limits<uint16_t>::max() +1;
	splitMeshes.reserve(numSplitMeshes);

	for(auto i=decltype(numVerts){0u};i<numVerts;++i)
	{
		auto meshIdx = i /MAX_VERTEX_COUNT_PER_MESH;
		if(meshIdx >= splitMeshes.size())
		{
			splitMeshes.push_back({});
			auto &splitMesh = splitMeshes.back();
			splitMesh.verts.reserve(MAX_VERTEX_COUNT_PER_MESH);
			if(hasVertexWeights)
				splitMesh.vertWeights.reserve(MAX_VERTEX_COUNT_PER_MESH);
			if(hasLightmapUvs)
				splitMesh.lightmapUvs.reserve(MAX_VERTEX_COUNT_PER_MESH);
		}
		auto &splitMesh = splitMeshes.back();
		splitMesh.verts.push_back(prVerts.at(i));
		if(hasVertexWeights)
			splitMesh.vertWeights.push_back(vertWeights.at(i));

		if(hasLightmapUvs)
			splitMesh.lightmapUvs.push_back(lightmapUvs.at(i));
	}

	std::vector<std::shared_ptr<ModelSubMesh>> meshes {};
	meshes.reserve(splitMeshes.size());
	for(auto &splitMesh : splitMeshes)
	{
		auto mesh = std::shared_ptr<ModelSubMesh>{nw.CreateSubMesh()};
		meshes.push_back(mesh);

		mesh->GetVertices() = splitMesh.verts;
		if(hasVertexWeights)
			mesh->GetVertexWeights() = splitMesh.vertWeights;

		if(hasLightmapUvs)
		{
			auto &uvSet = mesh->AddUVSet("lightmap");
			uvSet = splitMesh.lightmapUvs;
		}
	}
	return meshes;
}

static void initialize_scene_objects(
	NetworkState &nw,Model &mdl,ModelMeshGroup &meshGroup,source2::resource::VBIB &vbib,source2::resource::IKeyValueCollection &data,
	std::unordered_map<int32_t,uint32_t> &skinIndexToBoneIndex
)
{
	auto &vbufs = vbib.GetVertexBuffers();
	auto &ibufs = vbib.GetIndexBuffers();

	auto sceneObjects = data.FindArrayValues<source2::resource::IKeyValueCollection*>("m_sceneObjects");
	for(auto *sceneObject : sceneObjects)
	{
		auto prMesh = std::shared_ptr<ModelMesh>{nw.CreateMesh()};
		meshGroup.AddMesh(prMesh);
		auto drawCalls = sceneObject->FindArrayValues<source2::resource::IKeyValueCollection*>("m_drawCalls");
		for(auto *drawCall : drawCalls)
		{
			//auto prSubMesh = std::shared_ptr<ModelSubMesh>{nw.CreateSubMesh()};
			//prMesh->AddSubMesh(prSubMesh);
			//prSubMesh->SetSkinTextureIndex(0);
			auto vertexCount = drawCall->FindValue<int64_t>("m_nVertexCount");
			auto baseVertex = drawCall->FindValue<int64_t>("m_nBaseVertex");
			auto primitiveType = drawCall->FindValue<std::string>("m_nPrimitiveType","");
			auto startIndex = drawCall->FindValue<int64_t>("m_nStartIndex",0);
			auto indexCount = drawCall->FindValue<int64_t>("m_nIndexCount",0);
			auto uvDensity = drawCall->FindValue<double>("m_flUvDensity");
			auto startInstance = drawCall->FindValue<int64_t>("m_nStartInstance");
			auto instanceCount = drawCall->FindValue<int64_t>("m_nInstanceCount");
			auto tintColor = drawCall->FindValue<Vector4>("m_vTintColor");
			auto cullDataIndex = drawCall->FindValue<int64_t>("m_CullDataIndex");
			auto hasBakedLightingFromLightMap = drawCall->FindValue<bool>("m_bHasBakedLightingFromLightMap");
			auto material = drawCall->FindValue<std::string>("m_material");

			if(primitiveType != "RENDER_PRIM_TRIANGLES")
			{
				Con::cwar<<"WARNING: Unsupported primitive type '"<<primitiveType<<"'! Skipping mesh..."<<Con::endl;
				continue;
			}

			uint32_t skinTexIdx = 0;
			if(material.has_value())
				skinTexIdx = add_material(nw,mdl,*material);

			source2::impl::MeshData meshData {};
			auto vertexBuffers = drawCall->FindArrayValues<source2::resource::IKeyValueCollection*>("m_vertexBuffers");
			for(auto *vbuf : vertexBuffers)
			{
				auto bindOffsetBytes = vbuf->FindValue<int64_t>("m_nBindOffsetBytes");
				auto bufferIndex = vbuf->FindValue<int64_t>("m_hBuffer");
				if(bufferIndex.has_value() && *bufferIndex < vbufs.size())
				{
					auto &vbuf = vbufs.at(*bufferIndex);
					// TODO: bindOffsetBytes?
					meshData = std::move(source2::impl::initialize_vertices(vbuf,&skinIndexToBoneIndex));
				}
				break; // TODO: How to handle multiple vertex buffers?
			}

			auto *indexBuffer = drawCall->FindSubCollection("m_indexBuffer");
			if(indexBuffer)
			{
				auto bindOffsetBytes = indexBuffer->FindValue<int64_t>("m_nBindOffsetBytes");
				auto bufferIndex = indexBuffer->FindValue<int64_t>("m_hBuffer");
				if(bufferIndex.has_value() && *bufferIndex < ibufs.size())
				{
					auto &ibuf = ibufs.at(*bufferIndex);
					auto indexElementSize = ibuf.size;

					// Note: Pragma only supports indices in a range that fit into a uint16, but Souce 2
					// also supports uint32, so we have to use a container with a larger type to store
					// the indices before assigning them to the mesh
					std::vector<uint64_t> tris {};
					tris.reserve(indexCount);
					if(indexElementSize == 2)
					{
						for(auto i=startIndex;i<(startIndex +indexCount);++i)
						{
							auto idx = *(reinterpret_cast<const uint16_t*>(ibuf.buffer.data()) +i);
							tris.push_back(idx);
						}
					}
					else if(indexElementSize == 4)
					{
						for(auto i=startIndex;i<(startIndex +indexCount);++i)
						{
							auto idx = *(reinterpret_cast<const uint32_t*>(ibuf.buffer.data()) +i);
							tris.push_back(idx);
						}
					}
					else
						Con::cwar<<"WARNING: Unsupported index buffer element size of "<<indexElementSize<<" bytes!"<<Con::endl;

					// We don't need all vertices, so we'll just remove the unused ones
					std::vector<bool> verticesUsed {};
					auto &verts = meshData.verts;
					auto numVerts = verts.size();
					verticesUsed.resize(numVerts,false);
					for(auto idx : tris)
						verticesUsed.at(idx) = true;

					if(numVerts > 0u)
					{
						// We have to iterate backwards, otherwise erasing one of the vertices
						// would invalidate the indices
						for(int64_t i=(numVerts -1);i>=0;--i)
						{
							if(verticesUsed.at(i))
								continue;
							meshData.verts.erase(meshData.verts.begin() +i);
						}
					}

					std::vector<uint64_t> indexTranslationTable {};
					indexTranslationTable.resize(numVerts);
					int64_t indexOffset = 0;
					for(auto i=decltype(numVerts){0u};i<numVerts;++i)
					{
						if(verticesUsed.at(i) == false)
							--indexOffset;
						indexTranslationTable.at(i) = i +indexOffset;
					}

					for(auto &idx : tris)
						idx = indexTranslationTable[idx];

					auto meshes = generate_split_meshes(nw,meshData,tris);

					//prSubMesh->SetSkinTextureIndex(idx);

#if 0
					auto &meshTris = prSubMesh->GetTriangles();
					meshTris.reserve(tris.size());
					auto hasIndicesThatExceedSupportedRange = false;
					for(auto idx : tris)
					{
						if(idx > std::numeric_limits<uint16_t>::max())
						{
							hasIndicesThatExceedSupportedRange = true;
							idx = std::numeric_limits<uint16_t>::max();
						}
						prSubMesh->GetTriangles().push_back(idx);
					}
					if(hasIndicesThatExceedSupportedRange)
						Con::cwar<<"WARNING: Mesh vertex count exceeds maximum supported number of vertices per mesh! Mesh may be rendered incorrectly..."<<Con::endl;
#endif
				}
			}
		}
	}
}

static std::shared_ptr<source2::resource::Resource> load_resource(NetworkState &nw,const std::string &fileName)
{
	auto f = FileManager::OpenFile(fileName.c_str(),"rb");
	if(f == nullptr)
		f = uarch::load(fileName);
	if(f == nullptr)
		return nullptr;
	return source2::load_resource(f,[&nw](const std::string &path) -> VFilePtr {
		if(::util::port_file(&nw,path) == false)
			return nullptr;
		return FileManager::OpenFile(path.c_str(),"rb");
	});
}

static std::unordered_map<std::string,Flex::Operation::Type> g_flexOpTable {
	{"FLEX_OP_FETCH1",Flex::Operation::Type::Fetch},
	{"FLEX_OP_CONST",Flex::Operation::Type::Const},
	{"FLEX_OP_MAX",Flex::Operation::Type::Max},
	{"FLEX_OP_MIN",Flex::Operation::Type::Min},
	{"FLEX_OP_SUB",Flex::Operation::Type::Sub},
	{"FLEX_OP_DIV",Flex::Operation::Type::Div},
	{"FLEX_OP_MUL",Flex::Operation::Type::Mul}
};

std::shared_ptr<Model> source2::convert::convert_model(
	Game &game,source2::resource::Model &s2Mdl,source2::resource::Resource *optResource
)
{
	auto &nw = *game.GetNetworkState();
	auto ptrMdl = game.CreateModel();
	auto &mdl = *ptrMdl;
	auto skins = s2Mdl.GetSkins();
	for(auto iSkin=decltype(skins.size()){0u};iSkin<skins.size();++iSkin)
	{
		auto &skin = skins.at(iSkin);
		auto &name = skin.GetName();
		// TODO: Pragma has no support for named skins (yet)

		auto &materials = skin.GetMaterials();
		for(auto &mat : materials)
			add_material(nw,mdl,mat,iSkin);
	}

	auto &meshGroup = *mdl.AddMeshGroup("reference");
	mdl.GetBaseMeshes() = {0u};

	auto s2Skeleton = s2Mdl.GetSkeleton();

	std::vector<std::unordered_map<int32_t,uint32_t>> skinIndexToBoneIndexPerMesh {};
	auto &skeleton = mdl.GetSkeleton();
	skeleton.GetBones().clear();
	skeleton.GetRootBones().clear();

	auto s2Meshes = s2Mdl.GetEmbeddedMeshes();
	skinIndexToBoneIndexPerMesh.resize(s2Meshes.size());
	if(s2Skeleton)
	{
		auto reference = Animation::Create();
		mdl.AddAnimation("reference",reference); // Reference always has to be the first animation!

		auto &s2Bones = s2Skeleton->GetBoneList();
		auto numBones = s2Bones.size();
		auto refFrame = Frame::Create(numBones);
		reference->AddFrame(refFrame);

		reference->ReserveBoneIds(numBones);
		for(auto i=decltype(numBones){0u};i<numBones;++i)
			reference->AddBoneId(i);

		auto refPose = Frame::Create(s2Bones.size());
		std::unordered_map<source2::resource::Bone*,Bone*> s2BoneToPragma {};
		for(auto i=decltype(s2Bones.size()){0u};i<s2Bones.size();++i)
		{
			auto &s2Bone = s2Bones.at(i);
			auto *bone = new Bone{};
			bone->name = s2Bone->GetName();
			auto boneId = skeleton.AddBone(bone);

			refPose->SetBonePosition(i,s2Bone->GetPosition());
			refPose->SetBoneOrientation(i,s2Bone->GetRotation());
			refFrame->SetBonePosition(i,s2Bone->GetPosition());
			refFrame->SetBoneOrientation(i,s2Bone->GetRotation());
			s2BoneToPragma[s2Bone.get()] = bone;

			auto &skinIndicesPerMesh = s2Bone->GetSkinIndicesPerMesh();
			if(skinIndicesPerMesh.size() > skinIndexToBoneIndexPerMesh.size())
				skinIndexToBoneIndexPerMesh.resize(skinIndicesPerMesh.size());
			for(auto i=decltype(skinIndicesPerMesh.size()){0u};i<skinIndicesPerMesh.size();++i)
			{
				auto &skinIndices = skinIndicesPerMesh.at(i);
				auto &skinIndexToBoneIndex = skinIndexToBoneIndexPerMesh.at(i);
				for(auto skinIdx : skinIndices)
				{
					auto it = skinIndexToBoneIndex.find(skinIdx);
					if(it != skinIndexToBoneIndex.end() && it->second != boneId)
						Con::cwar<<"WARNING: Multiple bones point to skin index "<<skinIdx<<": "<<bone->name<<" and "<<skeleton.GetBone(it->second).lock()->name<<"! Only one is permitted, this may cause animation issues!"<<Con::endl;
					skinIndexToBoneIndex[skinIdx] = boneId;
				}
			}
		}
		for(auto &s2RootBone : s2Skeleton->GetRootBones())
		{
			auto *bone = s2BoneToPragma[s2RootBone.get()];
			skeleton.GetRootBones()[bone->ID] = bone->shared_from_this();
		}
		for(auto &s2Bone : s2Skeleton->GetBoneList())
		{
			auto *s2Parent = s2Bone->GetParent();
			if(s2Parent == nullptr)
				continue;
			auto *bone = s2BoneToPragma[s2Bone.get()];
			auto *parent = s2BoneToPragma[s2Parent];
			bone->parent = parent->shared_from_this();
			parent->children[bone->ID] = bone->shared_from_this();
		}
		mdl.SetReference(refPose);
	}

	// TODO: Add reference meshes

	// TODO: Grab materials

	// We'll have to rotate the vertices to convert them to Pragma's coordinate system
	//auto rotS2ToPragma = uquat::create(EulerAngles{-90.f,-90.f,0.f});

	auto defaultMeshGroupMask = s2Mdl.GetData()->FindValue<int64_t>("m_nDefaultMeshGroupMask",0);
	auto meshGroups = s2Mdl.GetData()->FindArrayValues<std::string>("m_meshGroups");
	auto refLodGroupMasks = s2Mdl.GetData()->FindArrayValues<int64_t>("m_refLODGroupMasks");

	uint32_t meshIdx = 0;
	for(auto &s2Mesh : s2Meshes)
	{
		ScopeGuard sg {[&meshIdx]() {++meshIdx;}};

		auto lodMask = (meshIdx < refLodGroupMasks.size()) ? refLodGroupMasks.at(meshIdx) : 0;
		if(!(lodMask &1)) // TODO: Load ALL lod meshes!
			continue;
		auto prMesh = std::shared_ptr<ModelMesh>{nw.CreateMesh()};
		meshGroup.AddMesh(prMesh);

		auto prSubMesh = std::shared_ptr<ModelSubMesh>(nw.CreateSubMesh());
		prSubMesh->SetSkinTextureIndex(0);

		auto *data = s2Mesh->GetResourceData()->GetData();
		if(data)
			initialize_scene_objects(nw,mdl,meshGroup,*s2Mesh->GetVBIB(),*data,skinIndexToBoneIndexPerMesh.at(meshIdx));
		prMesh->AddSubMesh(prSubMesh);
	}

	auto refMeshGroupMasks = s2Mdl.GetData()->FindArrayValues<int64_t>("m_refMeshGroupMasks");
	uint32_t refMeshIdx = 0;
	for(auto &meshName : s2Mdl.GetReferencedMeshNames())
	{
		ScopeGuard sg {[&refMeshIdx]() {++refMeshIdx;}};

		auto lodMask = (refMeshIdx < refLodGroupMasks.size()) ? refLodGroupMasks.at(refMeshIdx) : 0;
		if(!(lodMask &1)) // TODO: Load ALL lod meshes!
			continue;
		auto fileName = meshName +"_c";
		auto resource = ::load_resource(nw,fileName);
		if(resource == nullptr)
		{
			Con::cwar<<"WARNING: Unable to load referenced mesh '"<<meshName<<"'!"<<Con::endl;
			continue;
		}
		auto *vbib = dynamic_cast<source2::resource::VBIB*>(resource->FindBlock(source2::BlockType::VBIB));
		auto *dataBlock = dynamic_cast<source2::resource::BinaryKV3*>(resource->FindBlock(source2::BlockType::DATA));
		if(vbib == nullptr || dataBlock == nullptr)
			continue;
		initialize_scene_objects(nw,mdl,meshGroup,*vbib,*dataBlock->GetData(),skinIndexToBoneIndexPerMesh.at(refMeshIdx));
	}

	if(optResource)
	{
		auto animations = s2Mdl.GetEmbeddedAnimations(*optResource);
		for(auto &s2Anim : animations)
		{
			auto anim = ::Animation::Create();
			anim->SetFPS(s2Anim->GetFPS());

			// Find all of the bones used by this animation
			std::unordered_map<std::string,uint32_t> boneNameToAnimBoneListIndex {};
			for(auto &s2Frame : s2Anim->GetFrames())
			{
				for(auto &pair : s2Frame->GetBones())
				{
					auto it = boneNameToAnimBoneListIndex.find(pair.first);
					if(it != boneNameToAnimBoneListIndex.end())
						continue;
					auto boneId = skeleton.LookupBone(pair.first);
					if(boneId == -1)
						continue;
					auto localIdx = anim->AddBoneId(boneId);
					boneNameToAnimBoneListIndex.insert(std::make_pair(pair.first,localIdx));
				}
			}
			//

			auto numBonesAnim = anim->GetBoneList().size();
			for(auto &s2Frame : s2Anim->GetFrames())
			{
				auto frame = Frame::Create(numBonesAnim);
				for(auto &pair : s2Frame->GetBones())
				{
					auto it = boneNameToAnimBoneListIndex.find(pair.first);
					if(it == boneNameToAnimBoneListIndex.end())
						continue;
					auto boneListIdx = it->second;

					auto &s2FrameBone = pair.second;
					pragma::physics::Transform pose {s2FrameBone.position,s2FrameBone.rotation};
					frame->SetBonePose(boneListIdx,pose);
				}
				anim->AddFrame(frame);
			}

			mdl.AddAnimation(s2Anim->GetName(),anim);
		}
	}

	auto *morph = optResource ? dynamic_cast<source2::resource::BinaryKV3*>(optResource->FindBlock(source2::BlockType::MRPH)) : nullptr;
	auto morphData = morph ? morph->GetData() : nullptr;
	if(morphData)
	{
		auto textureAtlasPath = morphData->FindValue<std::string>("m_pTextureAtlas");
		auto texAtlasResource = textureAtlasPath.has_value() ? ::load_resource(nw,*textureAtlasPath +"_c") : nullptr;
		auto *texAtlas = texAtlasResource ? dynamic_cast<source2::resource::Texture*>(texAtlasResource->FindBlock(source2::BlockType::DATA)) : nullptr;
		if(texAtlas == nullptr)
		{
			if(textureAtlasPath.has_value())
				Con::cwar<<"WARNING: Unable to load texture atlas '"<<*textureAtlasPath<<"'!"<<Con::endl;
		}
		else
		{
			assert(texAtlas->GetFormat() == source2::VTexFormat::RGBA8888);
			if(texAtlas->GetFormat() != source2::VTexFormat::RGBA8888)
				throw std::logic_error{"Unexpected format for FACS morph atlas: '" +std::to_string(umath::to_integral(texAtlas->GetFormat())) +"'!"};
			auto atlasWidth = texAtlas->GetWidth();
			auto atlasHeight = texAtlas->GetHeight();
			//uint32_t maxX = 0;
			//uint32_t maxY = 0;
			std::vector<uint8_t> atlasData {};
			texAtlas->ReadTextureData(0,atlasData);

			/*{
				auto imgBuf = uimg::ImageBuffer::Create(atlasData.data(),atlasWidth,atlasHeight,uimg::ImageBuffer::Format::RGBA8);
				auto f = FileManager::OpenFile<VFilePtrReal>("morph_image_atlas.png","wb");
				if(f)
					uimg::save_image(f,*imgBuf,uimg::ImageFormat::PNG);
			}*/

			auto fGetPixelData = [&atlasData,atlasWidth,atlasHeight](int32_t x,int32_t y) -> uint32_t {
				auto pxOffset = y *atlasWidth +x;
				return *(reinterpret_cast<uint32_t*>(atlasData.data()) +pxOffset);
			};

			auto width = morphData->FindValue<int64_t>("m_nWidth");
			auto height = morphData->FindValue<int64_t>("m_nHeight");
			auto lookupType = morphData->FindValue<std::string>("m_nLookupType");
			auto bundleTypes = morphData->FindArrayValues<std::string>("m_bundleTypes");
			for(auto *morphData : morphData->FindArrayValues<source2::resource::IKeyValueCollection*>("m_morphDatas"))
			{
				auto name = morphData->FindValue<std::string>("m_name");
				for(auto *morphRectData : morphData->FindArrayValues<source2::resource::IKeyValueCollection*>("m_morphRectDatas"))
				{
					auto xLeftDst = morphRectData->FindValue<int64_t>("m_nXLeftDst",0);
					auto yTopDst = morphRectData->FindValue<int64_t>("m_nYTopDst",0);

					//maxX = umath::max(maxX,static_cast<uint32_t>(xLeftDst));
					//maxY = umath::max(maxY,static_cast<uint32_t>(yTopDst));

					auto uWidthSrc = morphRectData->FindValue<double>("m_flUWidthSrc",0.0);
					auto uHeightSrc = morphRectData->FindValue<double>("m_flVHeightSrc",0.0);

					auto wRect = umath::round(uWidthSrc *atlasWidth);
					auto hRect = umath::round(uHeightSrc *atlasHeight);

					uint32_t bundleIdx = 0;
					for(auto *bundleData : morphRectData->FindArrayValues<source2::resource::IKeyValueCollection*>("m_bundleDatas"))
					{
						auto &bundleType = bundleTypes.at(bundleIdx++);
						auto uLeftSrc = bundleData->FindValue<double>("m_flULeftSrc",0.0);
						auto vTopSrc = bundleData->FindValue<double>("m_flVTopSrc",0.0);
						auto offsets = bundleData->FindArrayValues<double>("m_offsets");
						auto ranges = bundleData->FindArrayValues<double>("m_ranges");

						auto xRect = umath::round(uLeftSrc *atlasWidth);
						auto yRect = umath::round(vTopSrc *atlasHeight);

						auto pxOffset = yRect *atlasWidth +xRect;

						// TODO: How do we get the index for the vertex morph?
						auto *ptr = (reinterpret_cast<uint8_t*>(atlasData.data()) +pxOffset *sizeof(int8_t) *4);
						auto *fptr = reinterpret_cast<float*>(ptr);
						auto *iptr = reinterpret_cast<uint32_t*>(ptr);

						if(bundleType == "MORPH_BUNDLE_TYPE_POSITION_SPEED")
						{
							Vector4 offset {};
							for(uint8_t i=0;i<umath::min(static_cast<int32_t>(offsets.size()),4);++i)
								offset[i] = offsets.at(i);

							Vector4 range {}; // ??
							for(uint8_t i=0;i<umath::min(static_cast<int32_t>(ranges.size()),4);++i)
								range[i] = ranges.at(i);

							// TODO: ???
						}
					}
				}
			}

			std::unordered_map<uint32_t,uint32_t> s2FlexDescToPragma {};
			uint32_t s2FlexIdx = 0;
			for(auto *flexDesc : morphData->FindArrayValues<source2::resource::IKeyValueCollection*>("m_FlexDesc"))
			{
				auto szFacs = flexDesc->FindValue<std::string>("m_szFacs");
				if(szFacs.has_value())
				{
					auto flexIdx = mdl.GetFlexCount();
					auto &flex = mdl.AddFlex(*szFacs);
					s2FlexDescToPragma[s2FlexIdx] = flexIdx;
				}
				++s2FlexIdx;
			}
			for(auto *flexController : morphData->FindArrayValues<source2::resource::IKeyValueCollection*>("m_FlexControllers"))
			{
				auto szName = flexController->FindValue<std::string>("m_szName");
				if(szName.has_value() == false)
					continue;
				auto szType = flexController->FindValue<std::string>("m_szType");
				auto min = flexController->FindValue<double>("min",0.0);
				auto max = flexController->FindValue<double>("max",0.0);
				auto &fc = mdl.AddFlexController(*szName);
				fc.min = min;
				fc.max = max;
			}

			for(auto *flexRule : morphData->FindArrayValues<source2::resource::IKeyValueCollection*>("m_FlexRules"))
			{
				auto nFlex = flexRule->FindValue<int64_t>("m_nFlex",-1);
				auto it = s2FlexDescToPragma.find(nFlex);
				if(it == s2FlexDescToPragma.end())
					continue;
				auto &flex = *mdl.GetFlex(nFlex);
				auto &ops = flex.GetOperations();
				for(auto *flexOp : flexRule->FindArrayValues<source2::resource::IKeyValueCollection*>("m_FlexOps"))
				{
					auto opCode = flexOp->FindValue<std::string>("m_OpCode","");
					auto data = flexOp->FindValue<int32_t>("m_Data",0);

					auto itOpTpe = g_flexOpTable.find(opCode);
					if(itOpTpe == g_flexOpTable.end())
						Con::cwar<<"WARNING: Unsupported flex operator code '"<<opCode<<"'! Skipping..."<<Con::endl;
					else
					{
						ops.push_back({});
						auto &op = ops.back();
						op.d.index = data;
						op.type = itOpTpe->second;
					}
				}
			}
		}
	}

	// Convert coordinate system to Pragma
	for(auto &meshGroup : mdl.GetMeshGroups())
	{
		for(auto &mesh : meshGroup->GetMeshes())
		{
			for(auto &subMesh : mesh->GetSubMeshes())
			{
				for(auto &v : subMesh->GetVertices())
				{
					v.position = source2::impl::convert_source2_vector_to_pragma(v.position);
					v.normal = source2::impl::convert_source2_vector_to_pragma(v.normal);
				}
			}
		}
	}
	for(auto &colMesh : mdl.GetCollisionMeshes())
	{
		for(auto &v : colMesh->GetVertices())
			v = source2::impl::convert_source2_vector_to_pragma(v);
	}
	auto fTranslateBonePoseToPragma = [](pragma::physics::Transform &pose) {
		auto &pos = pose.GetOrigin();
		auto &rot = pose.GetRotation();
		// These were determined through testing.
		// I'm unsure why the translation looks like this.
		pos = {pos.y,pos.x,-pos.z};
		rot = {-rot.w,-rot.y,-rot.x,rot.z};
	};
	for(auto &anim : mdl.GetAnimations())
	{
		for(auto &frame : anim->GetFrames())
		{
			for(auto &t : frame->GetBoneTransforms())
				fTranslateBonePoseToPragma(t);
		}
	}
	auto &refPose = mdl.GetReference();
	for(auto &t : refPose.GetBoneTransforms())
		fTranslateBonePoseToPragma(t);

	// Reference pose bones are in parent space, we need them in global space
	std::function<void(Bone&,const pragma::physics::Transform&)> fRelativeToGlobal = nullptr;
	fRelativeToGlobal = [&fRelativeToGlobal,&refPose](Bone &bone,const pragma::physics::Transform &parentPose) {
		auto *pPose = refPose.GetBoneTransform(bone.ID);
		auto pose = pPose ? *pPose : pragma::physics::Transform{};
		pose = parentPose *pose;

		refPose.SetBonePose(bone.ID,pose);
		for(auto &pair : bone.children)
			fRelativeToGlobal(*pair.second,pose);
	};
	pragma::physics::Transform rootBoneTransform {Vector3{},uquat::create(EulerAngles{90.f,90.f,0.f})};
	for(auto &rootBone : skeleton.GetRootBones())
		fRelativeToGlobal(*rootBone.second,rootBoneTransform);
	for(auto &anim : mdl.GetAnimations())
	{
		auto &boneList = anim->GetBoneList();
		for(auto &frame : anim->GetFrames())
		{
			for(auto i=decltype(boneList.size()){0u};i<boneList.size();++i)
			{
				auto boneId = boneList.at(i);
				if(skeleton.IsRootBone(boneId) == false)
					continue;
				// Root bones need to be rotated, unsure why
				auto &pose = *frame->GetBoneTransform(i);
				pose = rootBoneTransform *pose;
			}
		}
	}

	// Finalize model
	mdl.Update(ModelUpdateFlags::All);
	import::util::port_model_texture_assets(nw,mdl);
	return ptrMdl;
}
#pragma optimize("",on)
