/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "asset/fbx_loader.hpp"
#include <panima/animation.hpp>
#include <panima/channel.hpp>
#include <pragma/model/c_modelmesh.h>
#include <pragma/model/c_model.h>
#include <pragma/model/animation/bone.hpp>
#include <pragma/model/animation/vertex_animation.hpp>
#include <pragma/game/game_resources.hpp>
#include <pragma/asset/util_asset.hpp>
#include "pragma/rendering/shaders/util/c_shader_specular_glossiness_to_metalness_roughness.hpp"
#include "pragma/rendering/shaders/util/c_shader_compose_rma.hpp"
#include "pragma/rendering/shaders/util/c_shader_combine_image_channels.hpp"
#include <cmaterial.h>
#include <ofbx.h>
#include <span>
#include <util_image.hpp>
#include <util_image_buffer.hpp>
#include <fsys/ifile.hpp>
#include "pragma/asset/c_util_model.hpp"
#include <cmaterial_manager2.hpp>
#include <texturemanager/texture_manager2.hpp>

using namespace pragma::asset::fbx;
extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CEngine *c_engine;

static std::string_view to_string_view(ofbx::DataView data) { return std::string_view {(const char *)data.begin, (const char *)data.end}; }

static std::shared_ptr<prosper::Texture> load_texture_image(const std::string &mdlPath, const std::string &mdlName, const std::string &texPath, bool isAbsPath)
{
	auto fullTexFilePath = util::FilePath(texPath);
	if(!isAbsPath)
		fullTexFilePath = mdlPath + fullTexFilePath;
	auto ext = fullTexFilePath.GetFileExtension();
	if(ext && (*ext == "dds" || *ext == "ktx")) {
		auto &texManager = static_cast<msys::CMaterialManager &>(client->GetMaterialManager()).GetTextureManager();
		auto tex = texManager.LoadAsset(fullTexFilePath.GetString(), util::AssetLoadFlags::AbsolutePath | util::AssetLoadFlags::DontCache | util::AssetLoadFlags::IgnoreCache);
		if(!tex)
			return nullptr;
		return tex->GetVkTexture();
	}
	auto f = filemanager::open_file(fullTexFilePath.GetString(), filemanager::FileMode::Read | filemanager::FileMode::Binary);
	if(!f)
		f = filemanager::open_system_file(fullTexFilePath.GetString(), filemanager::FileMode::Read | filemanager::FileMode::Binary);
	if(!f)
		return nullptr;
	fsys::File file {f};
	auto imgBuf = uimg::load_image(file);
	if(!imgBuf)
		return nullptr;
	imgBuf->SwapChannels(uimg::Channel::Red, uimg::Channel::Blue);
	auto img = c_engine->GetRenderContext().CreateImage(*imgBuf);
	prosper::util::TextureCreateInfo texCreateInfo {};
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	return c_engine->GetRenderContext().CreateTexture(texCreateInfo, *img, imgViewCreateInfo, samplerCreateInfo);
}

static std::optional<std::string> import_texture(const std::string &mdlPath, const std::string &mdlName, const std::string &texPath, bool isAbsPath)
{
	auto fullTexFilePath = util::FilePath(texPath);
	if(!isAbsPath)
		fullTexFilePath = mdlPath + fullTexFilePath;
	std::string convertPath = util::CONVERT_PATH;
	auto matPath = util::DirPath(pragma::asset::get_asset_root_directory(pragma::asset::Type::Material), "models");
	auto texFileName = ufile::get_file_from_filename(texPath);
	auto fullOutputPath = util::DirPath(convertPath, matPath);
	auto ext = fullTexFilePath.GetFileExtension();
	if(ext && (*ext == "dds" || *ext == "ktx")) {
		auto srcPath = fullTexFilePath.GetString();
		auto dstPath = fullOutputPath + std::string {fullTexFilePath.GetFileName()};
		if(isAbsPath) {
			auto absFullPath = (util::get_program_path() + dstPath).GetString();
			auto res = filemanager::copy_system_file(srcPath, absFullPath);
			return res ? absFullPath : std::optional<std::string> {};
		}
		auto res = filemanager::copy_file(srcPath, dstPath.GetString());
		return res ? dstPath.GetString() : std::optional<std::string> {};
	}
	auto f = filemanager::open_file(fullTexFilePath.GetString(), filemanager::FileMode::Read | filemanager::FileMode::Binary);
	if(!f)
		f = filemanager::open_system_file(fullTexFilePath.GetString(), filemanager::FileMode::Read | filemanager::FileMode::Binary);
	if(!f)
		return {};
	fsys::File file {f};
	auto img = uimg::load_image(file);
	if(!img)
		return {};
	img->SwapChannels(uimg::Channel::Red, uimg::Channel::Blue);
	// TODO
	auto greyScale = false;
	auto normalMap = false;
	auto alphaMode = AlphaMode::Opaque;
	auto texInfo = pragma::asset::get_texture_info(greyScale, normalMap, alphaMode);

	const auto &importExtensions = pragma::asset::get_supported_extensions(pragma::asset::Type::Texture, pragma::asset::FormatType::All);
	fullOutputPath += util::DirPath(mdlName);
	auto outputFilePath = fullOutputPath + texFileName;
	outputFilePath.RemoveFileExtension(importExtensions);
	auto res = c_game->SaveImage(*img, outputFilePath.GetString(), texInfo);
	return res ? outputFilePath.GetString() : std::optional<std::string> {};
}

static bool is_const_curve(const ofbx::AnimationCurve *curve)
{
	if(!curve)
		return true;
	if(curve->getKeyCount() <= 1)
		return true;
	const float *values = curve->getKeyValue();
	if(curve->getKeyCount() == 2 && fabsf(values[1] - values[0]) < 1e-6)
		return true;
	return false;
}

static Mat4 to_pragma_matrix(const ofbx::DMatrix &mtx)
{
	Mat4 res;
	for(size_t x = 0; x < 4; ++x) {
		for(size_t y = 0; y < 4; ++y)
			res[x][y] = mtx.m[x * 4 + y];
	}
	return res;
}

static Vector3 fix_orientation(const Vector3 &v, pragma::asset::fbx::UpVector orientation)
{
	switch(orientation) {
	case pragma::asset::fbx::UpVector::Y:
		return Vector3 {v.x, v.y, v.z};
	case pragma::asset::fbx::UpVector::Z:
		return Vector3 {v.x, v.z, -v.y};
	case pragma::asset::fbx::UpVector::X:
		return Vector3 {-v.y, v.x, v.z};
	}
	return Vector3 {v.x, v.y, v.z};
}

static Quat fix_orientation(const Quat &v, pragma::asset::fbx::UpVector orientation)
{
	switch(orientation) {
	case pragma::asset::fbx::UpVector::Y:
		return Quat {v.w, v.x, v.y, v.z};
	case pragma::asset::fbx::UpVector::Z:
		return Quat {v.w, v.x, v.z, -v.y};
	case pragma::asset::fbx::UpVector::X:
		return Quat {v.w, -v.y, v.x, v.z};
	}
	return Quat {v.w, v.x, v.y, v.z};
}

FbxImporter::~FbxImporter() { m_fbxScene->destroy(); }

std::optional<uint32_t> FbxImporter::LoadMaterial(const ofbx::Material &fbxMat, uint32_t partitionIdx, std::string &outErr)
{
	auto importTexture = [this, &fbxMat](ofbx::Texture::TextureType etex) -> std::optional<std::string> {
		auto *tex = fbxMat.getTexture(etex);
		if(!tex)
			return {};
		ofbx::DataView filename = tex->getRelativeFileName();
		auto isAbsPath = false;
		if(filename == "") {
			filename = tex->getFileName();
			isAbsPath = true;
		}
		auto path = util::FilePath(std::string {to_string_view(filename)});
		return ::import_texture(m_mdlPath, m_mdlName, path.GetString(), isAbsPath);
	};
	auto loadTexture = [this, &fbxMat](ofbx::Texture::TextureType etex) -> std::shared_ptr<prosper::Texture> {
		auto *tex = fbxMat.getTexture(etex);
		if(!tex)
			return {};
		ofbx::DataView filename = tex->getRelativeFileName();
		auto isAbsPath = false;
		if(filename == "") {
			filename = tex->getFileName();
			isAbsPath = true;
		}
		auto path = util::FilePath(std::string {to_string_view(filename)});
		return ::load_texture_image(m_mdlPath, m_mdlName, path.GetString(), isAbsPath);
	};

	auto matFilePath = util::DirPath(::util::CONVERT_PATH, pragma::asset::get_asset_root_directory(pragma::asset::Type::Material), "models", m_mdlName);
	std::string fbxMatName = fbxMat.name;
	if(!fbxMatName.empty())
		matFilePath += fbxMatName;
	else
		matFilePath += "material" + std::to_string(partitionIdx);

	auto relMatPath = matFilePath;
	relMatPath.MakeRelative(util::CONVERT_PATH);
	relMatPath.MakeRelative(std::string {pragma::asset::get_asset_root_directory(pragma::asset::Type::Material)});
	auto mat = client->CreateMaterial(relMatPath.GetString(), "pbr");
	auto *cmat = static_cast<CMaterial *>(mat.get());

	auto &dataBlock = mat->GetDataBlock();

	auto importAndAssignTexture = [&importTexture, cmat](ofbx::Texture::TextureType etex, const std::string &matIdentifier) -> std::optional<std::string> {
		auto texPath = importTexture(etex);
		if(!texPath)
			return {};
		auto relTexPath = util::FilePath(*texPath);
		relTexPath.MakeRelative(util::CONVERT_PATH);
		relTexPath.MakeRelative(std::string {pragma::asset::get_asset_root_directory(pragma::asset::Type::Material)});
		cmat->SetTexture(matIdentifier, relTexPath.GetString());
		return texPath;
	};
	importAndAssignTexture(ofbx::Texture::TextureType::DIFFUSE, Material::ALBEDO_MAP_IDENTIFIER);
	importAndAssignTexture(ofbx::Texture::TextureType::NORMAL, Material::NORMAL_MAP_IDENTIFIER);
	importAndAssignTexture(ofbx::Texture::TextureType::EMISSIVE, Material::EMISSION_MAP_IDENTIFIER);

	auto applyColorFactor = [&fbxMat, &dataBlock](const ofbx::Color &fbxColor, double factor, const std::string &matProp) {
		Vector3 colorFactor {fbxColor.r, fbxColor.g, fbxColor.b};
		colorFactor *= factor;
		if(umath::abs(1.f - colorFactor.x) > 0.001f || umath::abs(1.f - colorFactor.y) > 0.001f || umath::abs(1.f - colorFactor.z) > 0.001f)
			dataBlock->AddValue("vector", matProp, std::to_string(colorFactor.x) + ' ' + std::to_string(colorFactor.y) + ' ' + std::to_string(colorFactor.z));
	};
	applyColorFactor(fbxMat.getDiffuseColor(), fbxMat.getDiffuseFactor(), "color_factor");
	applyColorFactor(fbxMat.getEmissiveColor(), fbxMat.getEmissiveFactor(), "emission_factor");

	// TODO
	//dataBlock->AddValue("float", "roughness_factor", std::to_string(pbrMetallicRoughness.roughnessFactor));
	//dataBlock->AddValue("float", "metalness_factor", std::to_string(pbrMetallicRoughness.metallicFactor));


	auto *combine = static_cast<pragma::ShaderCombineImageChannels *>(c_engine->GetShader("combine_image_channels").get());
	auto *rma = static_cast<pragma::ShaderSpecularGlossinessToMetalnessRoughness *>(c_engine->GetShader("specular_glossiness_to_metalness_roughness").get());
	if(combine && rma) {
		auto specularMap = loadTexture(ofbx::Texture::TextureType::SPECULAR);
		auto shininessMap = loadTexture(ofbx::Texture::TextureType::SHININESS);
		auto ambientMap = loadTexture(ofbx::Texture::TextureType::AMBIENT);
		auto reflectionMap = loadTexture(ofbx::Texture::TextureType::REFLECTION); // TODO

		if(specularMap || shininessMap || ambientMap) {
			auto tex = static_cast<msys::CMaterialManager &>(client->GetMaterialManager()).GetTextureManager().LoadAsset("white");
			if(tex) {
				auto whiteMap = tex->GetVkTexture();
				if(!specularMap)
					specularMap = whiteMap;
				if(!shininessMap)
					shininessMap = whiteMap;

				auto &context = c_engine->GetRenderContext();
				pragma::ShaderCombineImageChannels::PushConstants pushConstants {};
				pushConstants.alphaChannel = umath::to_integral(uimg::Channel::Red);
				auto specularGlossinessMap = combine->CombineImageChannels(context, *specularMap, *specularMap, *specularMap, *shininessMap, pushConstants);
				if(specularGlossinessMap) {
					pragma::ShaderSpecularGlossinessToMetalnessRoughness::PushConstants pushConstants {};
					auto metallicRoughnessSet = rma->ConvertToMetalnessRoughness(c_engine->GetRenderContext(), nullptr, specularGlossinessMap.get(), pushConstants, ambientMap.get());
					if(metallicRoughnessSet.has_value()) {
						auto texPath = matFilePath;
						texPath.MakeRelative(util::CONVERT_PATH);
						pragma::asset::assign_texture(*cmat, ::util::CONVERT_PATH, Material::RMA_MAP_IDENTIFIER, texPath.GetString() + "_rma", *metallicRoughnessSet->rmaMap, false, false, ambientMap ? AlphaMode::Blend : AlphaMode::Opaque);
					}
				}
			}
		}
	}

	mat->UpdateTextures();

	std::string err;
	mat->Save(matFilePath.GetString(), err, true);

	return m_model->AddMaterial(0, mat.get());
}

std::optional<std::string> FbxImporter::Finalize(std::string &outErr)
{
	m_model->Update(ModelUpdateFlags::All);
	std::string mdlPath = m_mdlName;
	auto fullMdlPath = util::FilePath(::util::CONVERT_PATH, pragma::asset::get_asset_root_directory(pragma::asset::Type::Model), m_outputPath, mdlPath);
	if(!m_model->Save(*c_game, fullMdlPath.GetString(), outErr)) {
		Con::cerr << "Error saving model: " << outErr << Con::endl;
		return {};
	}
	return mdlPath;
}

static char makeLowercase(char ch) { return std::tolower(static_cast<unsigned char>(ch)); }

// Converts a string to lowercase and writes to an output span
static bool makeLowercase(std::span<char> output, std::string_view src)
{
	if(src.size() + 1 > output.size()) // +1 for null terminator
		return false;

	auto destination = output.begin();
	for(const char &ch : src) {
		*destination++ = makeLowercase(ch);
	}
	*destination = '\0'; // Null-terminate the output
	return true;
}

const char *findInsensitive(std::string_view haystack, std::string_view needle)
{
	assert(!needle.empty());
	if(needle.size() > haystack.size())
		return nullptr;

	const char needle0 = makeLowercase(needle[0]);

	for(size_t i = 0; i <= haystack.size() - needle.size(); ++i) {
		if(makeLowercase(haystack[i]) == needle0) {
			size_t j = 1;
			while(j < needle.size() && makeLowercase(haystack[i + j]) == makeLowercase(needle[j])) {
				++j;
			}
			if(j == needle.size())
				return haystack.data() + i; // Match found
		}
	}
	return nullptr;
}

int stringLength(const char *str) { return (int)strlen(str); }

std::string FbxImporter::GetImportMeshName(const FbxImporter::FbxMeshInfo &mesh)
{
	const char *name = mesh.mesh->name;
	const ofbx::Material *material = mesh.material;

	if(name[0] == '\0' && mesh.mesh->getParent())
		name = mesh.mesh->getParent()->name;
	if(name[0] == '\0' && material)
		name = material->name;
	std::string out = name;
	for(char &c : out) {
		if(c == 0)
			break;
		// we use ':' as a separator between subresource:resource, so we can't have
		// use it in mesh name
		if(c == ':')
			c = '_';
	}
	if(mesh.subMesh >= 0) {
		out += "_" + std::to_string(mesh.subMesh);
	}
	return out;
}

int FbxImporter::DetectMeshLOD(const FbxImporter::FbxMeshInfo &mesh)
{
	const char *node_name = mesh.mesh->name;
	const char *lod_str = findInsensitive(node_name, "_LOD");
	if(!lod_str) {
		auto meshName = GetImportMeshName(mesh);
		lod_str = findInsensitive(meshName, "_LOD");
		if(!lod_str)
			return 0;
	}

	lod_str += stringLength("_LOD");

	auto lod = ustring::to_int(lod_str);
	return lod;
}

bool FbxImporter::LoadMeshes(std::string &outErr)
{
	int mesh_count = m_fbxScene->getMeshCount();

	m_fbxMeshes.reserve(mesh_count);
	for(int mesh_idx = 0; mesh_idx < mesh_count; ++mesh_idx) {
		const ofbx::Mesh *fbx_mesh = (const ofbx::Mesh *)m_fbxScene->getMesh(mesh_idx);
		int mat_count = fbx_mesh->getMaterialCount();
		auto hasMat = (mat_count > 0);
		if(mat_count == 0)
			mat_count = 1;
		for(int j = 0; j < mat_count; ++j) {
			m_fbxMeshes.push_back({});
			auto &mesh = m_fbxMeshes.back();
			mesh.skinned = false;
			const ofbx::Skin *skin = fbx_mesh->getSkin();
			if(skin) {
				for(int i = 0; i < skin->getClusterCount(); ++i) {
					if(skin->getCluster(i)->getIndicesCount() > 0) {
						mesh.skinned = true;
						break;
					}
				}
			}
			mesh.mesh = fbx_mesh;
			mesh.material = hasMat ? fbx_mesh->getMaterial(j) : nullptr;
			mesh.subMesh = mat_count > 1 ? j : -1;
			mesh.lod = DetectMeshLOD(mesh);
		}
	}

	auto anySkinned = false;
	for(auto &mesh : m_fbxMeshes) {
		if(mesh.skinned) {
			anySkinned = true;
			break;
		}
	}
	GatherBones(anySkinned);

	auto mg = m_model->AddMeshGroup("reference");
	for(int mesh_idx = 0; mesh_idx < mesh_count; ++mesh_idx) {
		const ofbx::Mesh &fbxMesh = *m_fbxScene->getMesh(mesh_idx);
		const ofbx::GeometryData &geom = fbxMesh.getGeometryData();
		const ofbx::Vec3Attributes positions = geom.getPositions();
		const ofbx::Vec3Attributes normals = geom.getNormals();
		const ofbx::Vec2Attributes uvs = geom.getUVs();
		auto tangents = geom.getTangents();

		struct BlendShapeData {
			std::vector<Vector3> posDeltas;
			std::vector<Vector3> normDeltas;
			std::shared_ptr<VertexAnimation> va;
		};
		std::vector<BlendShapeData> blendShapeData;
		auto *blendShape = fbxMesh.getBlendShape();
		if(blendShape) {
			auto *geom = fbxMesh.getGeometry();
			auto numChannels = blendShape->getBlendShapeChannelCount();
			blendShapeData.reserve(numChannels);
			for(size_t i = 0; i < numChannels; ++i) {
				blendShapeData.push_back({});
				auto &shapeData = blendShapeData.back();
				auto *channel = blendShape->getBlendShapeChannel(i);
				auto numShapes = channel->getShapeCount();
				if(numShapes == 0)
					continue;
				auto *shape = channel->getShape(numShapes - 1);
				const int *shapeIndices = shape->getIndices();

				const int shapeVertexCount = shape->getVertexCount();
				const int shapeIndexCount = shape->getIndexCount();
				if(shapeVertexCount != shapeIndexCount)
					continue;
				std::string morphTargetName = shape->name;

				if(m_model->GetFlexController(morphTargetName) == nullptr) {
					auto weight = channel->getShapeCount() > 1 ? (float)(channel->getDeformPercent() / 100.0) : 1.0f;
					auto &fc = m_model->AddFlexController(morphTargetName);
					fc.min = 0.f;
					fc.max = 1.f;
					// TODO: Apply default
				}
				uint32_t fcId = 0;
				m_model->GetFlexControllerId(morphTargetName, fcId);

				if(m_model->GetFlex(morphTargetName) == nullptr) {
					auto &flex = m_model->AddFlex(morphTargetName);
					auto va = m_model->AddVertexAnimation(morphTargetName);
					flex.SetVertexAnimation(*va);

					auto &operations = flex.GetOperations();
					operations.push_back({});
					auto &op = flex.GetOperations().back();
					op.type = Flex::Operation::Type::Fetch;
					op.d.index = fcId;
				}
				uint32_t flexId;
				m_model->GetFlexId(morphTargetName, flexId);
				auto va = m_model->GetFlex(flexId)->GetVertexAnimation()->shared_from_this();
				shapeData.va = va;

				auto *shapeVertices = shape->getVertices();
				auto *shapeNormals = shape->getNormals();

				shapeData.posDeltas.resize(fbxMesh.getGeometryData().getPositions().count);
				if(shapeNormals)
					shapeData.normDeltas.resize(fbxMesh.getGeometryData().getPositions().count);
				for(int32_t i = 0; i < shapeIndexCount; i++) {
					int shapeIndex = shapeIndices[i];
					auto &shapeVert = shapeVertices[i];
					Vector3 posDelta {shapeVert.x, shapeVert.y, shapeVert.z};
					shapeData.posDeltas[shapeIndex] = posDelta * m_fbxScale;

					if(shapeNormals) {
						auto &shapeNorm = shapeNormals[i];
						Vector3 normDelta {shapeNorm.x, shapeNorm.y, shapeNorm.z};
						shapeData.normDeltas[shapeIndex] = normDelta;
					}
				}
			}
		}

		std::vector<umath::VertexWeight> vertWeights {};
		std::vector<umath::VertexWeight> extendedVertWeights {};
		auto initVertexWeights = [&positions](std::vector<umath::VertexWeight> &vws) {
			if(!vws.empty())
				return;
			vws.resize(positions.values_count);
			for(auto &vw : vws) {
				for(size_t i = 0; i < 4; ++i)
					vw.boneIds[i] = -1;
			}
		};

		auto meshBone = AddBone(fbxMesh);
		FillSkinInfo(vertWeights, &fbxMesh, meshBone->ID);

		auto geoMatrix = to_pragma_matrix(fbxMesh.getGeometricMatrix());
		auto transformMatrix = to_pragma_matrix(fbxMesh.getGlobalTransform()) * geoMatrix;
		umath::ScaledTransform pose {transformMatrix};
		auto scale = pose.GetScale();
		pose.SetScale(uvec::IDENTITY_SCALE);
		auto mesh = c_game->CreateModelMesh();
		for(int partition_idx = 0; partition_idx < geom.getPartitionCount(); ++partition_idx) {
			const ofbx::GeometryPartition &partition = geom.getPartition(partition_idx);

			auto subMesh = c_game->CreateModelSubMesh();
			auto &verts = subMesh->GetVertices();

			auto *mat = (fbxMesh.getMaterialCount() > 0) ? fbxMesh.getMaterial(partition_idx) : nullptr;
			if(mat) {
				std::string err;
				auto matIdx = LoadMaterial(*mat, partition_idx, err);
				if(matIdx)
					subMesh->SetSkinTextureIndex(*matIdx);
			}

			std::unordered_map<int64_t, int64_t> fbxIndexToPragmaIndex;
			for(int polygon_idx = 0; polygon_idx < partition.polygon_count; ++polygon_idx) {
				const ofbx::GeometryPartition::Polygon &polygon = partition.polygons[polygon_idx];

				std::vector<int> triIndices;
				triIndices.resize(3 * (polygon.vertex_count - 2));
				auto numIndices = ofbx::triangulate(geom, polygon, triIndices.data());

				auto startIdx = subMesh->GetVertexCount();
				for(size_t i = 0; i < numIndices; ++i) {
					auto idx = triIndices[i];
					umath::Vertex v {};
					ofbx::Vec3 pos = positions.get(idx);
					v.position = {pos.x, pos.y, pos.z};
					if(normals.values != nullptr) {
						ofbx::Vec3 n = normals.get(idx);
						v.normal = {n.x, n.y, n.z};
					}
					if(uvs.values != nullptr) {
						ofbx::Vec2 uv = uvs.get(idx);
						v.uv = {uv.x, 1.f - uv.y};
					}
					if(tangents.values != nullptr) {
						ofbx::Vec3 t = tangents.get(idx);
						v.tangent = {t.x, t.y, t.z, 1.f /* handedness */};
					}
					auto vertIdx = subMesh->AddVertex(v);
					if(!vertWeights.empty()) {
						auto &vw = vertWeights[positions.indices[idx]];
						subMesh->SetVertexWeight(vertIdx, vw);
					}
					fbxIndexToPragmaIndex[idx] = vertIdx;
				}

				for(size_t i = 0; i < numIndices; i += 3)
					subMesh->AddTriangle(startIdx + i, startIdx + i + 1, startIdx + i + 2);
			}

			//if(pose != umath::ScaledTransform {}) {
			for(auto &v : subMesh->GetVertices()) {
				v.position *= scale;
				v.position = pose * v.position;
				v.position *= m_fbxScale;
				uvec::rotate(&v.normal, pose.GetRotation());
				v.position = fix_orientation(v.position, m_upVector);
			}
			//}

			mesh->AddSubMesh(subMesh);

			auto numVerts = subMesh->GetVertexCount();
			for(auto &shapeData : blendShapeData) {
				if(!shapeData.va || shapeData.posDeltas.empty())
					continue;
				auto mva = shapeData.va->AddMeshFrame(*mesh, *subMesh);
				mva->SetVertexCount(numVerts);
				auto hasNormals = !shapeData.normDeltas.empty();
				if(hasNormals)
					mva->SetFlagEnabled(MeshVertexFrame::Flags::HasNormals);

				for(size_t i = 0; i < shapeData.posDeltas.size(); ++i) {
					auto it = fbxIndexToPragmaIndex.find(i);
					if(it == fbxIndexToPragmaIndex.end())
						continue;
					mva->SetVertexPosition(it->second, shapeData.posDeltas[positions.indices[i]]);
				}
				if(hasNormals) {
					for(size_t i = 0; i < shapeData.normDeltas.size(); ++i) {
						auto it = fbxIndexToPragmaIndex.find(i);
						if(it == fbxIndexToPragmaIndex.end())
							continue;
						mva->SetVertexNormal(it->second, shapeData.normDeltas[positions.indices[i]]);
					}
				}
			}
		}

		mg->AddMesh(mesh);
	}
	return true;
}

static double fbxTimeToSeconds(ofbx::i64 value) { return double(value) / 46186158000L; }

static void normalizeScale(Mat4 &m)
{
	Vector3 scale = {1 / length(Vector3(m[0].x, m[1].x, m[2].x)), 1 / length(Vector3(m[0].y, m[1].y, m[2].y)), 1 / length(Vector3(m[0].z, m[1].z, m[2].z))};

	m[0].x *= scale.x;
	m[1].x *= scale.x;
	m[2].x *= scale.x;

	m[0].y *= scale.y;
	m[1].y *= scale.y;
	m[2].y *= scale.y;

	m[0].z *= scale.z;
	m[1].z *= scale.z;
	m[2].z *= scale.z;
}

static void convert(const ofbx::DMatrix &mtx, Vector3 &pos, Quat &rot)
{
	auto m = to_pragma_matrix(mtx);
	normalizeScale(m);
	umath::Transform pose {m};
	pos = pose.GetOrigin();
	rot = pose.GetRotation();
}

static float evalCurve(ofbx::i64 time, const ofbx::AnimationCurve &curve)
{
	const ofbx::i64 *times = curve.getKeyTime();
	const float *values = curve.getKeyValue();
	const int count = curve.getKeyCount();

	time = std::clamp(time, times[0], times[count - 1]);

	for(int i = 0; i < count; ++i) {
		if(time == times[i])
			return values[i];
		if(time < times[i]) {
			const float t = float((time - times[i - 1]) / double(times[i] - times[i - 1]));
			return values[i - 1] * (1 - t) + values[i] * t;
		}
	}
	return 0.f;
}

static ofbx::i64 sampleToFBXTime(ofbx::u32 sample, float fps) { return ofbx::secondsToFbxTime(sample / fps); }

struct Key {
	Key() : pos {}, rot {} {}
	Vector3 pos;
	Quat rot;
};
static void fill(const ofbx::Object &bone, const ofbx::AnimationLayer &layer, std::vector<Key> &keys, ofbx::u32 from_sample, ofbx::u32 samples_count, float fps)
{
	const ofbx::AnimationCurveNode *translation_node = layer.getCurveNode(bone, "Lcl Translation");
	const ofbx::AnimationCurveNode *rotation_node = layer.getCurveNode(bone, "Lcl Rotation");
	if(!translation_node && !rotation_node)
		return;

	keys.resize(samples_count);

	auto fill_rot = [&](ofbx::u32 idx, const ofbx::AnimationCurve *curve) {
		if(!curve) {
			const ofbx::DVec3 lcl_rot = bone.getLocalRotation();
			for(Key &k : keys) {
				(&k.rot.x)[idx] = float((&lcl_rot.x)[idx]);
			}
			return;
		}

		for(ofbx::u32 f = 0; f < samples_count; ++f) {
			Key &k = keys[f];
			(&k.rot.x)[idx] = evalCurve(sampleToFBXTime(from_sample + f, fps), *curve);
		}
	};

	auto fill_pos = [&](ofbx::u32 idx, const ofbx::AnimationCurve *curve) {
		if(!curve) {
			const ofbx::DVec3 lcl_pos = bone.getLocalTranslation();
			for(Key &k : keys) {
				(&k.pos.x)[idx] = float((&lcl_pos.x)[idx]);
			}
			return;
		}

		for(ofbx::u32 f = 0; f < samples_count; ++f) {
			Key &k = keys[f];
			(&k.pos.x)[idx] = evalCurve(sampleToFBXTime(from_sample + f, fps), *curve);
		}
	};

	fill_rot(0, rotation_node ? rotation_node->getCurve(0) : nullptr);
	fill_rot(1, rotation_node ? rotation_node->getCurve(1) : nullptr);
	fill_rot(2, rotation_node ? rotation_node->getCurve(2) : nullptr);

	fill_pos(0, translation_node ? translation_node->getCurve(0) : nullptr);
	fill_pos(1, translation_node ? translation_node->getCurve(1) : nullptr);
	fill_pos(2, translation_node ? translation_node->getCurve(2) : nullptr);

	for(Key &key : keys) {
		const ofbx::DMatrix mtx = bone.evalLocal({key.pos.x, key.pos.y, key.pos.z}, {key.rot.x, key.rot.y, key.rot.z});
		convert(mtx, key.pos, key.rot);
	}
}

void FbxImporter::InsertHierarchy(std::vector<const ofbx::Object *> &bones, const ofbx::Object *node)
{
	if(!node)
		return;
	auto it = std::find(bones.begin(), bones.end(), node);
	if(it != bones.end())
		return;
	ofbx::Object *parent = node->getParent();
	InsertHierarchy(bones, parent);
	bones.push_back(node);
}

void FbxImporter::GatherBones(bool force_skinned)
{
	for(auto &mesh : m_fbxMeshes) {
		const ofbx::Skin *skin = mesh.mesh->getSkin();
		if(skin) {
			for(int i = 0; i < skin->getClusterCount(); ++i) {
				const ofbx::Cluster *cluster = skin->getCluster(i);
				InsertHierarchy(m_fbxBones, cluster->getLink());
			}
		}

		if(force_skinned) {
			InsertHierarchy(m_fbxBones, mesh.mesh);
		}
	}

	for(int i = 0, n = m_fbxScene->getAnimationStackCount(); i < n; ++i) {
		const ofbx::AnimationStack *stack = m_fbxScene->getAnimationStack(i);
		for(int j = 0; stack->getLayer(j); ++j) {
			const ofbx::AnimationLayer *layer = stack->getLayer(j);
			for(int k = 0; layer->getCurveNode(k); ++k) {
				const ofbx::AnimationCurveNode *node = layer->getCurveNode(k);
				if(node->getBone())
					InsertHierarchy(m_fbxBones, node->getBone());
			}
		}
	}

	// removeDuplicates
	for(auto it0 = m_fbxBones.begin(); it0 != m_fbxBones.end();) {
		auto *bone = *it0;
		auto it1 = std::find(it0 + 1, m_fbxBones.end(), bone);
		if(it1 != m_fbxBones.end()) {
			it0 = m_fbxBones.erase(it0);
		}
		else
			++it0;
	}
	SortBones(force_skinned);
}

template<typename T>
void swapAndPop(std::vector<T> &vec, size_t index)
{
	if(index < vec.size()) { // Ensure index is valid
		// Swap the element at 'index' with the last element
		std::swap(vec[index], vec.back());
		// Remove the last element (originally at 'index')
		vec.pop_back();
	}
}

void FbxImporter::SortBones(bool force_skinned)
{
	const int count = m_fbxBones.size();
	ofbx::u32 first_nonroot = 0;
	for(int32_t i = 0; i < count; ++i) {
		if(!m_fbxBones[i]->getParent()) {
			std::swap(m_fbxBones[i], m_fbxBones[first_nonroot]);
			++first_nonroot;
		}
	}

	for(int32_t i = 0; i < count; ++i) {
		for(int j = i + 1; j < count; ++j) {
			if(m_fbxBones[i]->getParent() == m_fbxBones[j]) {
				const ofbx::Object *bone = m_fbxBones[j];
				swapAndPop(m_fbxBones, j);
				m_fbxBones.insert(m_fbxBones.begin() + i, bone);
				--i;
				break;
			}
		}
	}

	if(force_skinned) {
		for(auto &m : m_fbxMeshes) {
			auto it = std::find(m_fbxBones.begin(), m_fbxBones.end(), m.mesh);
			m.boneIndex = (it != m_fbxBones.end()) ? (it - m_fbxBones.begin()) : -1;
			m.skinned = true;
		}
	}
}

bool FbxImporter::LoadAnimations(std::string &outErr)
{
	auto &skel = m_model->GetSkeleton();
	auto &ref = m_model->GetReference();
	auto refAnimFrame = Frame::Create(ref);
	refAnimFrame->Localize(skel);
	//ref.Globalize(skel);
	auto refAnim = pragma::animation::Animation::Create();
	auto numBones = skel.GetBoneCount();
	refAnim->ReserveBoneIds(numBones);
	for(auto i = decltype(numBones) {0u}; i < numBones; ++i)
		refAnim->AddBoneId(i);
	refAnim->AddFrame(refAnimFrame);
	m_model->AddAnimation("reference", refAnim);

	for(int i = 0, n = m_fbxScene->getAnimationStackCount(); i < n; ++i) {
		const ofbx::AnimationStack *stack = m_fbxScene->getAnimationStack(i);
		const ofbx::TakeInfo *take_info = m_fbxScene->getTakeInfo(stack->name);
		std::string name;
		if(take_info) {
			if(take_info->name.begin != take_info->name.end) {
				name = to_string_view(take_info->name);
			}
			if(name.empty() && take_info->filename.begin != take_info->filename.end) {
				auto tmp = to_string_view(take_info->filename);
				name = util::DirPath(tmp).GetFileName();
			}
			if(name.empty())
				name = "anim";
		}
		else {
			name = "";
		}

		const ofbx::AnimationLayer *anim_layer = stack->getLayer(0);
		if(!anim_layer || !anim_layer->getCurveNode(0)) {
			//m_animations.pop();
			continue;
		}

		double full_len;
		if(take_info) {
			full_len = take_info->local_time_to - take_info->local_time_from;
		}
		else if(m_fbxScene->getGlobalSettings()) {
			full_len = m_fbxScene->getGlobalSettings()->TimeSpanStop;
		}
		else {
			//logError("Unsupported animation in ", src);
			continue;
		}

		const float fps = m_fbxScene->getSceneFrameRate();
		auto anim = pragma::animation::Animation::Create();

		auto numBones = m_fbxBones.size();
		std::vector<pragma::animation::BoneId> boneIds;
		boneIds.reserve(numBones);
		for(auto *fbxBone : m_fbxBones) {
			auto bone = AddBone(*fbxBone);
			boneIds.push_back(bone->ID);
		}
		size_t fbxBoneId = 0;
		for(const ofbx::Object *fbxBone : m_fbxBones) {
			std::vector<Key> keys;
			fill(*fbxBone, *anim_layer, keys, 0, ofbx::u32(full_len * fps + 0.5f) + 1, fps);

			while(anim->GetFrameCount() < keys.size())
				anim->AddFrame(Frame::Create(numBones));
			for(size_t i = 0; i < keys.size(); ++i) {
				auto frame = anim->GetFrame(i);
				auto &key = keys[i];

				auto pos = key.pos;
				auto rot = key.rot;

				pos *= m_fbxScale;
				pos = fix_orientation(pos, m_upVector);
				rot = fix_orientation(rot, m_upVector);

				umath::Transform pose {pos, rot};
				frame->SetBonePose(fbxBoneId, pose);
			}
			++fbxBoneId;
		}
		anim->SetBoneList(boneIds);
		anim->SetFPS(fps);
		m_model->AddAnimation(name, anim);

#if 0
		auto panim = std::make_shared<panima::Animation>();
		bool data_found = false;
		for(int k = 0; anim_layer->getCurveNode(k); ++k) {
			const ofbx::AnimationCurveNode *node = anim_layer->getCurveNode(k);
			auto boneLinkProperty = node->getBoneLinkProperty();
			// TODO: Scale?
			auto isBoneTranslation = (boneLinkProperty == "Lcl Translation");
			auto isBoneRotation = (boneLinkProperty == "Lcl Rotation");
			auto isBoneScaling = (boneLinkProperty == "Lcl Scaling");
			if(isBoneTranslation || isBoneRotation || isBoneScaling) {
				auto *fbxBone = node->getBone();
				auto bone = fbxBone ? AddBone(*fbxBone) : nullptr;
				if(bone) {
					auto *curve0 = node->getCurve(0);
					auto *curve1 = node->getCurve(1);
					auto *curve2 = node->getCurve(2);
					// TODO
					if(!is_const_curve(curve0) && !is_const_curve(curve1) && !is_const_curve(curve2)) {

						auto numKeys0 = curve0->getKeyCount();
						auto *times0 = curve0->getKeyTime();
						auto *values0 = curve0->getKeyValue();

						auto numKeys1 = curve0->getKeyCount();
						auto *times1 = curve1->getKeyTime();
						auto *values1 = curve1->getKeyValue();

						auto numKeys2 = curve0->getKeyCount();
						auto *times2 = curve2->getKeyTime();
						auto *values2 = curve2->getKeyValue();

						auto calcInterpolatedValue = [](size_t count, const ofbx::i64 *times, const float *values, ofbx::i64 timestamp) {
							// If the timestamp is outside the range, clamp to the nearest value
							if(timestamp <= times[0])
								return values[0];
							if(timestamp >= times[count - 1])
								return values[count - 1];

							// Find the index of the first element in `times` that is greater than `timestamp`
							const ofbx::i64 *upper = std::lower_bound(times, times + count, timestamp);
							int i = int(upper - times) - 1;

							// Linear interpolation formula
							float t = float(timestamp - times[i]) / float(times[i + 1] - times[i]);
							return values[i] * (1 - t) + values[i + 1] * t;
						};

						std::vector<ofbx::i64> times;
						times.reserve(numKeys0 + numKeys1 + numKeys2);
						for(int l = 0; l < numKeys0; ++l)
							times.push_back(times0[l]);
						for(int l = 0; l < numKeys1; ++l)
							times.push_back(times1[l]);
						for(int l = 0; l < numKeys2; ++l)
							times.push_back(times2[l]);
						std::sort(times.begin(), times.end());
						if(times.size() > 1) {
							// Remove duplicate timestamps
							for(auto it = times.begin(); it != times.end() - 1;) {
								auto itNext = it;
								++itNext;
								auto t0 = *it;
								auto t1 = *itNext;
								if(t0 == t1) {
									// TODO: Check panima epsilon
									// if(umath::abs(t1 - t0) < 0.001f) {
									it = times.erase(it);
									continue;
								}
								++it;
							}
						}

						std::vector<ofbx::DVec3> fbxValues;
						fbxValues.reserve(times.size());
						std::vector<float> ftimes;
						ftimes.reserve(times.size());
						auto duration = 0.f;
						for(auto t : times) {
							auto v0 = calcInterpolatedValue(numKeys0, times0, values0, t);
							auto v1 = calcInterpolatedValue(numKeys1, times1, values1, t);
							auto v2 = calcInterpolatedValue(numKeys2, times2, values2, t);
							ofbx::DVec3 v {v0, v1, v2};
							fbxValues.push_back(v);
							ftimes.push_back(fbxTimeToSeconds(t));
							duration = umath::max(duration, ftimes.back());
						}

						auto channel = std::make_shared<panima::Channel>();
						std::string basePath = "ec/animated/bone/" + std::string {bone->name} + "/";
						if(isBoneTranslation || isBoneScaling) {
							std::vector<Vector3> values;
							values.reserve(fbxValues.size());
							if(isBoneTranslation) {
								for(auto &v : fbxValues)
									values.push_back(GetTranslation(v));
							}
							else {
								for(auto &v : fbxValues)
									values.push_back(GetScale(v));
							}
							channel->SetValueType(udm::Type::Vector3);
							channel->InsertValues<Vector3>(ftimes.size(), ftimes.data(), values.data());
							if(isBoneTranslation)
								channel->targetPath = panima::ChannelPath {basePath + "position"};
							else
								channel->targetPath = panima::ChannelPath {basePath + "scale"};
						}
						else {
							std::vector<Quat> values;
							values.reserve(fbxValues.size());
							for(auto &v : fbxValues)
								values.push_back(GetRotation(v, static_cast<RotationOrder>(curve0->getRotationOrder())));
							channel->SetValueType(udm::Type::Quaternion);
							channel->InsertValues<Quat>(ftimes.size(), ftimes.data(), values.data());
							channel->targetPath = panima::ChannelPath {basePath + "rotation"};
						}
						panim->SetDuration(umath::max(panim->GetDuration(), duration));
						panim->AddChannel(*channel);
					}
				}
			}
		}

		{
			uint8_t fps = 24; // TODO: Determine from frame diff
			auto anim = pragma::animation::Animation::Create(*panim, skel, ref, fps);
			for(auto &frame : anim->GetFrames()) {
				auto numTransforms = frame->GetBoneCount();
				for(size_t i = 0; i < numTransforms; ++i) {
					umath::ScaledTransform pose;
					if(!frame->GetBonePose(i, pose))
						continue;
					auto origin = pose.GetOrigin();
					origin *= m_fbxScale;
					origin = fix_orientation(origin, m_upVector);
					pose.SetOrigin(origin);
					frame->SetBonePose(i, pose);
				}
			}

			m_model->AddAnimation(name, anim);
		}
#endif
	}
	return true;
}

std::optional<pragma::asset::AssetImportResult> FbxImporter::Load(std::string &outErr)
{
	auto mdl = c_game->CreateModel(false);
	m_model = mdl;
	mdl->GetBaseMeshes() = {0u};
	mdl->CreateTextureGroup();

	m_fbxScale = m_fbxScene->getGlobalSettings()->UnitScaleFactor * pragma::metres_to_units(1.0) * 0.01f;
	m_upVector = static_cast<UpVector>(m_fbxScene->getGlobalSettings()->UpAxis);

	if(!LoadMeshes(outErr) || !LoadAnimations(outErr))
		return {};
	auto mdlPath = Finalize(outErr);
	if(!mdlPath)
		return {};
	AssetImportResult result {};
	result.models.push_back(*mdlPath);
	result.modelObjects.push_back(m_model);
	return result;
}
Vector3 FbxImporter::GetTranslation(const ofbx::DVec3 &o) { return {o.x, o.y, o.z}; }
#include <glm/gtx/euler_angles.hpp>
Quat FbxImporter::GetRotation(const ofbx::DVec3 &o, RotationOrder order)
{
	order = RotationOrder::Yxz;
	EulerAngles ang {static_cast<float>(o.x), static_cast<float>(o.y), static_cast<float>(o.z)};
	Mat4 m;
	auto p = umath::deg_to_rad(ang.p);
	auto y = umath::deg_to_rad(ang.y);
	auto r = umath::deg_to_rad(ang.r);
	switch(order) {
	case RotationOrder::Xyz:
		m = glm::eulerAngleXYZ(p, y, r);
		break;
	case RotationOrder::Xzy:
		m = glm::eulerAngleXZY(p, y, r);
		break;
	case RotationOrder::Yzx:
		m = glm::eulerAngleYZX(p, y, r);
		break;
	case RotationOrder::Yxz:
		m = glm::eulerAngleYXZ(y, p, r);
		break;
	case RotationOrder::Zxy:
		m = glm::eulerAngleZXY(p, y, r);
		break;
	case RotationOrder::Zyx:
		m = glm::eulerAngleZYX(p, y, r);
		break;
	}
	return glm::quat_cast(m);
	//return glm::quat_cast(glm::eulerAngleYXZ(umath::deg_to_rad(ang.y), umath::deg_to_rad(ang.p), umath::deg_to_rad(ang.r)));
	//return uquat::create(ang);
}
Vector3 FbxImporter::GetScale(const ofbx::DVec3 &o)
{
	Vector3 scale {o.x, o.y, o.z};
	for(size_t i = 0; i < 3; ++i) {
		if(umath::abs(scale[i] - 1.f) < 0.0001f)
			scale[i] = 1.f;
	}
	return scale;
}
umath::ScaledTransform FbxImporter::GetPose(const ofbx::Object &o)
{
	auto fbxTrans = o.getLocalTranslation();
	auto fbxRot = o.getLocalRotation();
	auto fbxScale = o.getLocalScaling();

	umath::ScaledTransform pose {};
	pose.SetOrigin(GetTranslation(fbxTrans));
	pose.SetRotation(GetRotation(fbxRot, static_cast<RotationOrder>(o.getRotationOrder())));
	pose.SetScale(GetScale(fbxScale));
	return pose;
}
std::shared_ptr<pragma::animation::Bone> FbxImporter::AddBone(const ofbx::Object &o)
{
	std::string name = o.name;
	if(name.empty())
		name = "bone" + std::to_string(o.id);
	auto &skel = m_model->GetSkeleton();
	auto &ref = m_model->GetReference();
	auto boneId = skel.LookupBone(name);
	if(boneId != -1)
		return skel.GetBone(boneId).lock();
	auto *bone = new pragma::animation::Bone {};
	bone->name = name;
	boneId = skel.AddBone(bone);

	ref.SetBoneCount(skel.GetBoneCount());
	auto m = to_pragma_matrix(o.getGlobalTransform());
	umath::ScaledTransform pose {m};
	auto scale = pose.GetScale();
	pose.SetScale(uvec::IDENTITY_SCALE);
	auto origin = pose.GetOrigin();
	origin *= m_fbxScale;
	origin = fix_orientation(origin, m_upVector);
	pose.SetOrigin(origin);

	ref.SetBonePose(boneId, pose);
	auto *parent = o.getParent();
	if(parent) {
		bone->parent = AddBone(*parent);
		if(!bone->parent.expired())
			bone->parent.lock()->children[bone->ID] = skel.GetBone(bone->ID).lock();
	}
	if(bone->parent.expired())
		skel.GetRootBones()[boneId] = skel.GetBone(boneId).lock();
	return skel.GetBone(boneId).lock();
}
void FbxImporter::FillSkinInfo(std::vector<umath::VertexWeight> &skinning, const ofbx::Mesh *mesh, int32_t boneId)
{
	const ofbx::Skin *fbx_skin = mesh->getSkin();
	const ofbx::GeometryData &geom = mesh->getGeometryData();
	skinning.resize(geom.getPositions().values_count);
	for(auto &vw : skinning) {
		for(size_t i = 0; i < 4; ++i) {
			vw.boneIds[i] = -1;
			vw.weights[i] = 0.f;
		}
	}

	if(!fbx_skin) {
		for(auto &skin : skinning) {
			skin.weights[0] = 1;
			skin.boneIds[0] = boneId;
		}
		return;
	}
	auto getWeightCount = [](const umath::VertexWeight &vw) {
		size_t count = 0;
		for(size_t i = 0; i < 4; ++i) {
			if(vw.boneIds[i] == -1)
				break;
			++count;
		}
		return count;
	};

	for(int i = 0, c = fbx_skin->getClusterCount(); i < c; ++i) {
		const ofbx::Cluster *cluster = fbx_skin->getCluster(i);
		if(cluster->getIndicesCount() == 0)
			continue;
		if(!cluster->getLink())
			continue;

		auto bone = AddBone(*cluster->getLink());
		const int *cp_indices = cluster->getIndices();
		const double *weights = cluster->getWeights();
		for(int j = 0; j < cluster->getIndicesCount(); ++j) {
			int idx = cp_indices[j];
			float weight = (float)weights[j];
			auto &s = skinning[idx];
			if(getWeightCount(s) < 4) {
				s.weights[getWeightCount(s)] = weight;
				s.boneIds[getWeightCount(s)] = bone->ID;
			}
			else {
				int min = 0;
				for(int m = 1; m < 4; ++m) {
					if(s.weights[m] < s.weights[min])
						min = m;
				}

				if(s.weights[min] < weight) {
					s.weights[min] = weight;
					s.boneIds[min] = bone->ID;
				}
			}
		}
	}

	for(auto &s : skinning) {
		float sum = 0;
		for(size_t i = 0; i < 4; ++i)
			sum += s.weights[i];
		if(sum > 0.0001f) {
			for(size_t i = 0; i < 4; ++i)
				s.weights[i] /= sum;
		}
	}
}

std::optional<pragma::asset::AssetImportResult> pragma::asset::import_fbx(ufile::IFile &f, std::string &outErrMsg, const util::Path &outputPath)
{
	auto fileName = f.GetFileName();
	if(!fileName) {
		outErrMsg = "No file name.";
		return {};
	}
	std::vector<uint8_t> data;
	auto sz = f.GetSize();
	data.resize(sz);
	f.Read(data.data(), data.size());
	ofbx::LoadFlags flags = ofbx::LoadFlags::IGNORE_VIDEOS;
	auto *scene = ofbx::load(data.data(), data.size(), (ofbx::u16)flags);
	if(!scene) {
		outErrMsg = "Failed to load FBX data: " + std::string {ofbx::getError()};
		return {};
	}
	auto mdlPath = ufile::get_path_from_filename(*fileName);
	auto mdlName = ufile::get_file_from_filename(*fileName);
	ufile::remove_extension_from_filename(mdlName, std::vector<std::string> {"fbx"});
	FbxImporter importer {scene, mdlPath, mdlName, outputPath};
	return importer.Load(outErrMsg);
}

std::optional<pragma::asset::AssetImportResult> pragma::asset::import_fbx(const std::string &fileName, std::string &outErrMsg, const util::Path &outputPath)
{
	auto f = filemanager::open_file(fileName, filemanager::FileMode::Read | filemanager::FileMode::Binary);
	if(!f) {
		outErrMsg = "Failed to open file '" + fileName + "' for reading!";
		return {};
	}
	fsys::File fptr {f};
	return import_fbx(fptr, outErrMsg, outputPath);
}
