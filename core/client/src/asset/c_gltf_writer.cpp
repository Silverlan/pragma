/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "c_gltf_writer.hpp"
#include "pragma/math/c_util_math.hpp"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include <pragma/clientstate/clientstate.h>
#include <pragma/model/model.h>
#include <pragma/model/modelmesh.h>
#include <pragma/model/animation/vertex_animation.hpp>
#include <pragma/model/animation/animation.hpp>
#include <sharedutils/alpha_mode.hpp>
#include <sharedutils/util_path.hpp>
#include <sharedutils/util_file.h>
#include <mathutil/umath_lighting.hpp>
#include <pragma/engine_info.hpp>
#include <pragma/engine_version.h>
#include <pragma/util/resource_watcher.h>
#include <datasystem_color.h>
#include <datasystem_vector.h>
#include <pragma/model/animation/skeleton.hpp>
#include <pragma/model/animation/bone.hpp>

// #define ENABLE_GLTF_VALIDATION
#define GLTF_ASSERT(c, msg)                                                                                                                                                                                                                                                                      \
	if(!(c)) {                                                                                                                                                                                                                                                                                   \
		Con::cwar << "glTF assertion failure: " << msg << Con::endl;                                                                                                                                                                                                                             \
		throw std::logic_error {"glTF assertion failed!"};                                                                                                                                                                                                                                       \
	}

extern DLLCLIENT ClientState *client;

bool pragma::asset::GLTFWriter::Export(const SceneDesc &sceneDesc, const std::string &outputFileName, const pragma::asset::ModelExportInfo &exportInfo, std::string &outErrMsg, std::string *optOutPath)
{
	GLTFWriter writer {sceneDesc, exportInfo, std::optional<std::string> {}};
	return writer.Export(outErrMsg, outputFileName, optOutPath);
}
bool pragma::asset::GLTFWriter::Export(const SceneDesc &sceneDesc, const std::string &outputFileName, const std::string &animName, const pragma::asset::ModelExportInfo &exportInfo, std::string &outErrMsg, std::string *optOutPath)
{
	GLTFWriter writer {sceneDesc, exportInfo, animName};
	return writer.Export(outErrMsg, outputFileName, optOutPath);
}

bool pragma::asset::GLTFWriter::Export(::Model &model, const ModelExportInfo &exportInfo, std::string &outErrMsg, const std::optional<std::string> &outputFileName, std::string *optOutPath)
{
	auto fileName = outputFileName.has_value() ? *outputFileName : model.GetName();
	return Export({{model}}, fileName, exportInfo, outErrMsg, optOutPath);
}
bool pragma::asset::GLTFWriter::Export(::Model &model, const std::string &animName, const ModelExportInfo &exportInfo, std::string &outErrMsg, const std::optional<std::string> &outputFileName, std::string *optOutPath)
{
	auto fileName = outputFileName.has_value() ? *outputFileName : model.GetName();
	return Export({{model}}, fileName, animName, exportInfo, outErrMsg, optOutPath);
}

pragma::asset::GLTFWriter::GLTFWriter(const SceneDesc &sceneDesc, const ModelExportInfo &exportInfo, const std::optional<std::string> &animName) : m_sceneDesc {sceneDesc}, m_exportInfo {exportInfo}, m_animName {animName} {}

uint32_t pragma::asset::GLTFWriter::AddAccessor(const std::string &name, int componentType, int type, uint64_t byteOffset, uint64_t count, BufferViewIndex bufferViewIdx)
{
	m_gltfMdl.accessors.push_back({});
	auto &accessor = m_gltfMdl.accessors.back();
	accessor.componentType = componentType;
	accessor.count = count;
	accessor.byteOffset = byteOffset;
	accessor.type = type;
	accessor.name = name;
	accessor.bufferView = bufferViewIdx;
	return m_gltfMdl.accessors.size() - 1;
};

void pragma::asset::GLTFWriter::InitializeMorphSets(::Model &mdl)
{
	auto &flexes = mdl.GetFlexes();
	for(auto flexId = decltype(flexes.size()) {0u}; flexId < flexes.size(); ++flexId) {
		auto &flex = flexes.at(flexId);
		auto &name = flex.GetName();
		auto *va = flex.GetVertexAnimation();
		//auto *anim = flex.GetMeshVertexAnimation();
		//auto *frame = flex.GetMeshVertexFrame();
		//if(anim == nullptr || frame == nullptr || va == nullptr)
		//	continue;
		if(va == nullptr)
			continue;
		auto frameId = flex.GetFrameIndex();
		auto &meshAnims = va->GetMeshAnimations();
		//uint32_t morphSetIndex = 0u;
		for(auto &meshAnim : meshAnims) {
			auto *subMesh = meshAnim->GetSubMesh();
			if(subMesh == nullptr)
				continue;
			auto *frame = va->GetMeshFrame(*subMesh, frameId);
			if(frame == nullptr)
				continue;
			auto &morphSets = m_meshMorphSets[subMesh];
			morphSets.push_back({});
			auto &morphSet = morphSets.back();
			morphSet.name = name;
			//if(morphSetIndex > 0)
			//	morphSet.name += '_' +std::to_string(morphSetIndex);
			morphSet.frame = frame;
			morphSet.flexId = flexId;
			//++morphSetIndex;
		}
	}
}

bool pragma::asset::GLTFWriter::IsSkinned(::Model &mdl) const
{
	if(ShouldExportMeshes() == false)
		return false;
	auto &skeleton = mdl.GetSkeleton();
	return (skeleton.GetBoneCount() > 1 && m_exportInfo.exportSkinnedMeshData);
}
bool pragma::asset::GLTFWriter::IsAnimated(::Model &mdl) const
{
	auto &anims = mdl.GetAnimations();
	auto &skeleton = mdl.GetSkeleton();
	return (skeleton.GetBoneCount() > 1 && anims.empty() == false && (m_exportInfo.exportAnimations || m_animName.has_value()));
}
bool pragma::asset::GLTFWriter::ShouldExportMeshes() const { return m_animName.has_value() == false; }
void pragma::asset::GLTFWriter::WriteMorphTargets(ModelSubMesh &mesh, tinygltf::Mesh &gltfMesh, tinygltf::Primitive &primitive, const std::vector<uint32_t> &nodeIndices)
{
	auto itMorphSets = m_meshMorphSets.find(&mesh);
	if(itMorphSets != m_meshMorphSets.end()) {
		std::vector<tinygltf::Value> morphNames {};
		auto &morphSets = itMorphSets->second;
		morphNames.reserve(morphSets.size());
		for(auto &morphSet : morphSets) {
			morphNames.push_back(tinygltf::Value {morphSet.name});

			auto numVerts = morphSet.frame->GetVertexCount();

			auto hasNormals = morphSet.frame->IsFlagEnabled(MeshVertexFrame::Flags::HasNormals);
			uint32_t numAttributes = 1;
			if(hasNormals)
				++numAttributes;

			auto numVertexData = numVerts * numAttributes;
			std::vector<Vector3> vertexData {};
			vertexData.resize(numVertexData);

			Vector3 min {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
			Vector3 max {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()};
			uint32_t offset = 0;
			for(auto i = decltype(numVerts) {0u}; i < numVerts; ++i) {
				Vector3 pos {};
				morphSet.frame->GetVertexPosition(i, pos);
				pos = TransformPos(pos);
				vertexData.at(offset++) = pos;

				uvec::min(&min, pos);
				uvec::max(&max, pos);

				if(hasNormals == false)
					continue;
				Vector3 n {};
				morphSet.frame->GetVertexNormal(i, n);
				vertexData.at(offset++) = n;
			}

			uint32_t morphBufferIdx;
			auto &morphBuffer = AddBuffer("morph_" + morphSet.name, &morphBufferIdx);
			auto &morphData = morphBuffer.data;
			morphData.resize(vertexData.size() * sizeof(vertexData.front()));
			memcpy(morphData.data(), vertexData.data(), vertexData.size() * sizeof(vertexData.front()));
			auto morphBufferView = AddBufferView("morph" + morphSet.name, morphBufferIdx, 0, vertexData.size() * sizeof(vertexData.front()), numAttributes * sizeof(Vector3));
			auto morphAccessor = AddAccessor("morph_" + morphSet.name + "_positions", TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC3, 0, numVerts, morphBufferView);
			m_gltfMdl.accessors.at(morphAccessor).minValues = {min.x, min.y, min.z};
			m_gltfMdl.accessors.at(morphAccessor).maxValues = {max.x, max.y, max.z};

			primitive.targets.push_back({});
			auto &map = primitive.targets.back();
			map["POSITION"] = morphAccessor;

			if(hasNormals)
				map["NORMAL"] = AddAccessor("morph_" + morphSet.name + "_normals", TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC3, sizeof(Vector3), numVerts, morphBufferView);
		}

		std::map<std::string, tinygltf::Value> extras {};
		extras["targetNames"] = tinygltf::Value {morphNames};
		gltfMesh.extras = tinygltf::Value {extras};

		m_meshesWithMorphTargets[&mesh] = nodeIndices;
	}
}

void pragma::asset::GLTFWriter::MergeSplitMeshes(ExportMeshList &meshList)
{
	if(m_exportInfo.verbose)
		Con::cout << "Merging meshes by materials..." << Con::endl;
	std::unordered_map<uint32_t, std::vector<std::shared_ptr<ModelSubMesh>>> groupedMeshes {};
	for(auto &mesh : meshList) {
		auto texIdx = mesh->GetSkinTextureIndex();
		auto it = groupedMeshes.find(texIdx);
		if(it == groupedMeshes.end())
			it = groupedMeshes.insert(std::make_pair(texIdx, std::vector<std::shared_ptr<ModelSubMesh>> {})).first;
		it->second.push_back(mesh);
	}

	uint32_t numMerged = 0;
	std::vector<std::shared_ptr<ModelSubMesh>> mergedMeshes {};
	mergedMeshes.reserve(groupedMeshes.size());
	for(auto &pair : groupedMeshes) {
		auto &meshes = pair.second;
		if(meshes.size() == 1) {
			mergedMeshes.push_back(meshes.front());
			continue;
		}
		auto mesh = meshes.front()->Copy();
		for(auto it = meshes.begin() + 1; it != meshes.end(); ++it) {
			auto &meshOther = *it;
			mesh->Merge(*meshOther);
			++numMerged;
		}
		mergedMeshes.push_back(mesh);
	}
	meshList = mergedMeshes;
	if(m_exportInfo.verbose)
		Con::cout << numMerged << " meshes have been merged!" << Con::endl;
}

void pragma::asset::GLTFWriter::GenerateUniqueModelExportList()
{
	//if(ShouldExportMeshes() == false)
	//	return;
	m_uniqueModelExportList.reserve(m_sceneDesc.modelCollection.size());
	for(auto &mdlDesc : m_sceneDesc.modelCollection) {
		auto it = std::find_if(m_uniqueModelExportList.begin(), m_uniqueModelExportList.end(), [&mdlDesc](const ModelExportData &exportData) { return &exportData.model == &mdlDesc.model; });
		if(it == m_uniqueModelExportList.end()) {
			m_uniqueModelExportList.push_back({mdlDesc.model});
			it = m_uniqueModelExportList.end() - 1;
			auto &exportData = m_uniqueModelExportList.back();

			std::vector<std::shared_ptr<ModelMesh>> meshList {};
			meshList.reserve(mdlDesc.model.GetMeshCount());

			std::vector<uint32_t> bodyGroups {};
			bodyGroups.resize(mdlDesc.model.GetBodyGroupCount(), 0);
			if(m_exportInfo.fullExport == false)
				mdlDesc.model.GetBodyGroupMeshes(bodyGroups, meshList);
			else {
				for(auto &meshGroup : mdlDesc.model.GetMeshGroups()) {
					for(auto &mesh : meshGroup->GetMeshes())
						meshList.push_back(mesh);
				}
			}

			for(auto &mesh : meshList) {
				for(auto &subMesh : mesh->GetSubMeshes()) {
					if(subMesh->GetIndexCount() == 0)
						continue;
					if(exportData.exportMeshes.size() == exportData.exportMeshes.capacity())
						exportData.exportMeshes.reserve(exportData.exportMeshes.size() + 100);
					exportData.exportMeshes.push_back(subMesh);
					exportData.indexCount += subMesh->GetIndexCount();
					exportData.vertCount += subMesh->GetVertices().size();
				}
			}
		}
		it->instances.push_back(mdlDesc.pose);
	}

	if(m_exportInfo.mergeMeshesByMaterial) {
		for(auto &exportData : m_uniqueModelExportList)
			MergeSplitMeshes(exportData.exportMeshes);
	}

	uint64_t indexCount = 0;
	uint64_t vertCount = 0;
}

void pragma::asset::GLTFWriter::ToGLTFPose(const umath::Transform &pose, std::vector<double> &outPos, std::vector<double> &outRot) const
{
	auto pos = pose.GetOrigin() * m_exportInfo.scale;
	auto rot = pose.GetRotation();
	uquat::rotate_z(rot, umath::deg_to_rad(180.f));
	uquat::rotate_x(rot, umath::deg_to_rad(180.f));
	outPos = {pos.x, pos.y, pos.z};
	outRot = {rot.x, rot.y, rot.z, rot.w};
}

bool pragma::asset::GLTFWriter::Export(std::string &outErrMsg, const std::string &outputFileName, std::string *optOutPath)
{
	// HACK: If the model was just ported, we need to make sure the material and textures are in order by invoking the
	// resource watcher (in case they have been changed)
	// TODO: This doesn't belong here!
	client->GetResourceWatcher().Poll();

	auto name = outputFileName;
	::util::Path exportPath {name};
	exportPath.Canonicalize();
	auto exportPathStr = exportPath.GetString();
	auto outputPath = ufile::get_path_from_filename(exportPathStr) + ufile::get_file_from_filename(exportPathStr);
	ufile::remove_extension_from_filename(outputPath);
	outputPath += '/';
	m_exportPath = outputPath;
	outputPath = EXPORT_PATH + m_exportPath;

	// Note: These values are for reserving space. They don't
	// have to be accurate, but they *have* to be larger than (or equal to) the
	// actual counts.
	uint32_t numSubMeshes = 0;
	uint32_t numMeshes = 0;
	uint32_t numAnims = 0;
	uint32_t numVertexAnims = 0;
	uint32_t numFramesTotal = 0;
	uint32_t numNodes = 0;
	auto numLights = m_sceneDesc.lightSources.size();
	for(auto &mdlDesc : m_sceneDesc.modelCollection) {
		numSubMeshes += mdlDesc.model.GetSubMeshCount();
		numMeshes += mdlDesc.model.GetMeshCount();
		numAnims += mdlDesc.model.GetAnimationCount();
		numVertexAnims += mdlDesc.model.GetVertexAnimations().size();
		for(auto &anim : mdlDesc.model.GetAnimations())
			numFramesTotal += anim->GetFrameCount();

		numNodes += mdlDesc.model.GetSkeleton().GetBoneCount();
	}
	numNodes += numSubMeshes;
	numNodes += numLights;

	auto &gltfMdl = m_gltfMdl;
	gltfMdl.meshes.reserve(numSubMeshes);
	gltfMdl.nodes.reserve(numNodes);
	gltfMdl.lights.reserve(numLights);
	//m_gltfMdl.nodes.reserve(m_gltfMdl.nodes.size() +skeleton.GetBoneCount());

	gltfMdl.asset.generator = "Pragma Engine " + get_pretty_engine_version();
	gltfMdl.asset.version = "2.0"; // GLTF version

	gltfMdl.defaultScene = 0;
	gltfMdl.scenes.push_back({});
	ufile::remove_extension_from_filename(name);
	std::replace(name.begin(), name.end(), '\\', '/');
	auto &gltfScene = gltfMdl.scenes.back();
	gltfScene.name = name;

	//auto &mdl = GetModel();
	//if(mdl != nullptr)
	//	mdl->GetBodyGroupMeshes(GetBodyGroups(),m_lodMeshes);
	//const_cast<Model*>(this)->GetMeshes(meshIds,outMeshes);

	GenerateUniqueModelExportList();
	uint64_t indexCount = 0;
	uint64_t vertCount = 0;
	for(auto &list : m_uniqueModelExportList) {
		indexCount += list.indexCount;
		vertCount += list.vertCount;
	}

	// Materials
	if(m_exportInfo.generateAo && ShouldExportMeshes()) {
		for(auto &exportData : m_uniqueModelExportList)
			GenerateAO(exportData.model);
	}

	WriteMaterials();

	// Initialize buffers
	if(m_exportInfo.verbose)
		Con::cout << "Initializing GLTF buffers..." << Con::endl;

	gltfMdl.buffers.reserve(BufferIndices::Count + numAnims + numVertexAnims * 2);

	constexpr auto szVertex = sizeof(Vector3) * 2 + sizeof(Vector2);
	gltfMdl.bufferViews.reserve(BufferViewIndices::Count + numAnims + numFramesTotal);
	if(ShouldExportMeshes()) {
		auto &indexBuffer = AddBuffer("indices", &m_bufferIndices.indices);
		auto &vertexBuffer = AddBuffer("vertices", &m_bufferIndices.vertices);
		//indexBuffer.uri = "indices.bin";
		//vertexBuffer.uri = "vertices.bin";

		std::vector<Vector2> uvSetsData {};
		for(auto &exportData : m_uniqueModelExportList) {
			for(auto &mesh : exportData.exportMeshes) {
				auto &uvSets = mesh->GetUVSets();
				if(uvSets.empty())
					continue;
				uvSetsData.reserve(uvSetsData.size() + uvSets.size() * mesh->GetVertexCount());
				for(auto &pair : uvSets) {
					assert(pair.second.size() == mesh->GetVertexCount());
					for(auto uv : pair.second) {
						for(uint8_t i = 0; i < uv.length(); ++i) {
							if((std::isnan(uv[i]) || std::isinf(uv[i])))
								uv[i] = 0.f;
						}
						uvSetsData.push_back(uv);
					}
				}
			}
		}

		uint64_t indexOffset = 0;
		uint64_t vertOffset = 0;
		//std::vector<uint8_t> indexData {};
		auto &indexData = indexBuffer.data;
		indexData.resize(indexCount * sizeof(uint32_t));
		//std::vector<uint8_t> vertexData {};
		auto &vertexData = vertexBuffer.data;
		vertexData.resize(vertCount * szVertex);
		for(auto &exportData : m_uniqueModelExportList) {
			uint32_t meshIdx = 0;
			for(auto &mesh : exportData.exportMeshes) {
				util::ScopeGuard sg {[&meshIdx]() { ++meshIdx; }};
				auto *gltfIndexData = indexData.data() + indexOffset * sizeof(uint32_t);
				mesh->VisitIndices([gltfIndexData](auto *indexData, uint32_t numIndices) {
					for(auto i = decltype(numIndices) {0u}; i < numIndices; ++i) {
						auto idx = static_cast<uint32_t>(indexData[i]);
						memcpy(gltfIndexData + i * sizeof(uint32_t), &idx, sizeof(idx));
					}
				});
				indexOffset += mesh->GetIndexCount();

				auto &verts = mesh->GetVertices();
				auto *gltfVertexData = vertexData.data() + vertOffset * szVertex;
				for(auto i = decltype(verts.size()) {0u}; i < verts.size(); ++i) {
					auto &v = verts.at(i);
					auto pos = TransformPos(v.position);
					auto n = v.normal;
					auto uv = v.uv;

#ifdef ENABLE_GLTF_VALIDATION
					auto fGetErrMsg = [i, meshIdx, &exportData, &pos, &v]() {
						std::string msg = "Found invalid vertex (index " + std::to_string(i) + ") for mesh " + std::to_string(meshIdx) + " of model " + exportData.model.GetName() + ": " + "pos(" + std::to_string(pos.x) + ',' + std::to_string(pos.y) + ',' + std::to_string(pos.z) + ") "
						  + "normal(" + std::to_string(v.normal.x) + ',' + std::to_string(v.normal.y) + ',' + std::to_string(v.normal.z) + ") " + "uv(" + std::to_string(v.uv.x) + ',' + std::to_string(v.uv.y) + ")";
						return msg;
					};
					GLTF_ASSERT(std::isnan(pos.x) == false && std::isnan(pos.y) == false && std::isnan(pos.z) == false, fGetErrMsg());
					GLTF_ASSERT(std::isinf(pos.x) == false && std::isinf(pos.y) == false && std::isinf(pos.z) == false, fGetErrMsg());

					GLTF_ASSERT(std::isnan(v.normal.x) == false && std::isnan(v.normal.y) == false && std::isnan(v.normal.z) == false, fGetErrMsg());
					GLTF_ASSERT(std::isinf(v.normal.x) == false && std::isinf(v.normal.y) == false && std::isinf(v.normal.z) == false, fGetErrMsg());

					GLTF_ASSERT(std::isnan(v.uv.x) == false && std::isnan(v.uv.y) == false, fGetErrMsg());
					GLTF_ASSERT(std::isinf(v.uv.x) == false && std::isinf(v.uv.y) == false, fGetErrMsg());

					auto l = uvec::length(v.normal);
					GLTF_ASSERT(l >= 0.99f && l <= 1.01f, fGetErrMsg());
#else
					auto l = uvec::length(v.normal);
					auto isNormalValid = (l >= 0.99f && l <= 1.01f);
					for(uint8_t i = 0; i < 3; ++i) {
						if(std::isnan(pos[i]) || std::isinf(pos[i]))
							pos[i] = 0.f;
						if(std::isnan(n[i]) || std::isinf(n[i]))
							isNormalValid = false;
						if(i < 2 && (std::isnan(uv[i]) || std::isinf(uv[i])))
							uv[i] = 0.f;
					}
					if(isNormalValid == false)
						n = uvec::UP;
#endif

					memcpy(gltfVertexData + i * szVertex, &pos, sizeof(pos));
					memcpy(gltfVertexData + i * szVertex + sizeof(Vector3), &n, sizeof(n));
					memcpy(gltfVertexData + i * szVertex + sizeof(Vector3) * 2, &uv, sizeof(uv));
				}
				vertOffset += verts.size();
			}
		}

		m_bufferViewIndices.indices = AddBufferView("indices", m_bufferIndices.indices, 0, indexBuffer.data.size(), {});
		m_bufferViewIndices.positions = AddBufferView("positions", m_bufferIndices.vertices, 0, vertexBuffer.data.size(), szVertex);
		m_bufferViewIndices.normals = AddBufferView("normals", m_bufferIndices.vertices, sizeof(Vector3), vertexBuffer.data.size() - sizeof(Vector3), szVertex);
		m_bufferViewIndices.texCoords = AddBufferView("texcoords", m_bufferIndices.vertices, sizeof(Vector3) * 2, vertexBuffer.data.size() - sizeof(Vector3) * 2, szVertex);
		if(uvSetsData.size() > 0) {
			auto &uvSetsBuffer = AddBuffer("uvsets", &m_bufferIndices.uvSets);
			auto size = uvSetsData.size() * sizeof(uvSetsData.front());
			m_bufferViewIndices.uvSets = AddBufferView("uvsets", m_bufferIndices.uvSets, 0, size, sizeof(Vector2));
			uvSetsBuffer.data.resize(size);
			memcpy(uvSetsBuffer.data.data(), uvSetsData.data(), size);
		}
	}

	gltfMdl.accessors.reserve(numSubMeshes * BufferViewIndices::Count);

	// Skeleton
	for(auto &exportData : m_uniqueModelExportList)
		WriteSkeleton(exportData);

	if(m_exportInfo.exportMorphTargets) {
		for(auto &exportData : m_uniqueModelExportList)
			InitializeMorphSets(exportData.model);
	}

	// Initialize accessors
	uint64_t indexOffset = 0;
	uint64_t vertOffset = 0;
	uint64_t vertexWeightOffset = 0;
	uint32_t uvSetOffset = 0;
	uint32_t meshIdx = 0;
	for(auto &exportData : m_uniqueModelExportList) {
		for(auto &mesh : exportData.exportMeshes) {
			auto meshName = name + '_' + std::to_string(meshIdx);
			std::vector<uint32_t> nodeIndices {};
			nodeIndices.reserve(exportData.instances.size());
			for(auto &pose : exportData.instances) {
				auto pos = pose.GetOrigin() * m_exportInfo.scale;
				if(std::isnan(pos.x) || std::isnan(pos.y) || std::isnan(pos.z))
					pos = {};

				auto rot = pose.GetRotation();
				if(std::isnan(rot.x) || std::isnan(rot.y) || std::isnan(rot.z) || std::isnan(rot.w))
					rot = uquat::identity();
				auto nodeIdx = AddNode(meshName, true);
				auto &gltfNode = gltfMdl.nodes.at(nodeIdx);
				gltfNode.mesh = gltfMdl.meshes.size();
				gltfNode.translation = {pos.x, pos.y, pos.z};
				gltfNode.rotation = {rot.x, rot.y, rot.z, rot.w};
				if(exportData.skinIndex != -1)
					gltfNode.skin = exportData.skinIndex;
				nodeIndices.push_back(nodeIdx);
			}

			auto &verts = mesh->GetVertices();
			gltfMdl.accessors.reserve(gltfMdl.accessors.size() + 4);
			auto numIndices = mesh->GetIndexCount();
			auto indicesAccessor = AddAccessor("mesh" + std::to_string(meshIdx) + "_indices", TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT, TINYGLTF_TYPE_SCALAR, indexOffset * sizeof(uint32_t), numIndices, m_bufferViewIndices.indices);
			auto posAccessor = AddAccessor("mesh" + std::to_string(meshIdx) + "_positions", TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC3, vertOffset * szVertex, verts.size(), m_bufferViewIndices.positions);
			auto normalAccessor = AddAccessor("mesh" + std::to_string(meshIdx) + "_normals", TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC3, vertOffset * szVertex, verts.size(), m_bufferViewIndices.normals);
			auto uvAccessor = AddAccessor("mesh" + std::to_string(meshIdx) + "_uvs", TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC2, vertOffset * szVertex, verts.size(), m_bufferViewIndices.texCoords);

			std::vector<uint32_t> uvSetsAccessors;
			auto &uvSets = mesh->GetUVSets();
			if(!uvSets.empty()) {
				uvSetsAccessors.reserve(uvSets.size());
				uint32_t uvSetIdx = 0;
				for(auto &pair : uvSets) {
					auto &uvSet = pair.second;
					auto uvAccessor = AddAccessor("mesh" + std::to_string(meshIdx) + "_uvset" + std::to_string(uvSetIdx++), TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC2, uvSetOffset * sizeof(Vector2), verts.size(), m_bufferViewIndices.uvSets);
					uvSetsAccessors.push_back(uvAccessor);

					uvSetOffset += verts.size();
				}
			}

			// Calculate bounds
			Vector3 min {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
			Vector3 max {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()};
			for(auto &v : verts) {
				uvec::min(&min, v.position);
				uvec::max(&max, v.position);
			}
			min = TransformPos(min);
			max = TransformPos(max);
			gltfMdl.accessors.at(posAccessor).minValues = {min.x, min.y, min.z};
			gltfMdl.accessors.at(posAccessor).maxValues = {max.x, max.y, max.z};
			//

			// Calculate index bounds
			auto minIndex = std::numeric_limits<int64_t>::max();
			auto maxIndex = std::numeric_limits<int64_t>::lowest();
			mesh->VisitIndices([&minIndex, &maxIndex](auto *indexData, uint32_t numIndices) {
				for(auto i = decltype(numIndices) {0u}; i < numIndices; ++i) {
					auto idx = indexData[i];
					minIndex = umath::min(minIndex, static_cast<int64_t>(idx));
					maxIndex = umath::max(maxIndex, static_cast<int64_t>(idx));
				}
			});
			gltfMdl.accessors.at(indicesAccessor).minValues = {static_cast<double>(minIndex)};
			gltfMdl.accessors.at(indicesAccessor).maxValues = {static_cast<double>(maxIndex)};
			//

			gltfMdl.meshes.push_back({});
			auto &gltfMesh = gltfMdl.meshes.back();

			gltfMesh.primitives.push_back({});
			auto &primitive = gltfMesh.primitives.back();
			auto *mat = exportData.model.GetMaterial(0, mesh->GetSkinTextureIndex());
			if(mat) {
				auto it = m_materialToGltfIndex.find(mat);
				if(it != m_materialToGltfIndex.end())
					primitive.material = it->second;
			}

			auto geometryType = mesh->GetGeometryType();
			switch(geometryType) {
			case ModelSubMesh::GeometryType::Triangles:
				primitive.mode = TINYGLTF_MODE_TRIANGLES;
				break;
			case ModelSubMesh::GeometryType::Lines:
				primitive.mode = TINYGLTF_MODE_LINE;
				break;
			case ModelSubMesh::GeometryType::Points:
				primitive.mode = TINYGLTF_MODE_POINTS;
				break;
			}

			primitive.indices = indicesAccessor;
			primitive.attributes["POSITION"] = posAccessor;
			primitive.attributes["NORMAL"] = normalAccessor;
			primitive.attributes["TEXCOORD_0"] = uvAccessor;
			for(auto i = decltype(uvSets.size()) {0u}; i < uvSets.size(); ++i)
				primitive.attributes["TEXCOORD_" + std::to_string(i + 1)] = uvSetsAccessors[i];

			if(IsSkinned(exportData.model) && mesh->GetVertexWeights().empty() == false) {
				auto jointsAccessor = AddAccessor("mesh" + std::to_string(meshIdx) + "_joints", TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT, TINYGLTF_TYPE_VEC4, vertexWeightOffset * sizeof(GLTFVertexWeight), verts.size(), m_bufferViewIndices.joints);
				auto weightsAccessor = AddAccessor("mesh" + std::to_string(meshIdx) + "_weights", TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC4, vertexWeightOffset * sizeof(GLTFVertexWeight), verts.size(), m_bufferViewIndices.weights);
				primitive.attributes["JOINTS_0"] = jointsAccessor;
				primitive.attributes["WEIGHTS_0"] = weightsAccessor;

				vertexWeightOffset += verts.size();
			}

			// Morphs
			if(m_exportInfo.exportMorphTargets)
				WriteMorphTargets(*mesh, gltfMesh, primitive, nodeIndices);

			indexOffset += numIndices;
			vertOffset += verts.size();
			++meshIdx;
		}
	}

	for(auto &exportData : m_uniqueModelExportList) {
		if(exportData.skinIndex == -1)
			continue;
		// Inverse bind matrix accessor has to be added last
		auto &skeleton = exportData.model.GetSkeleton();
		auto bindPoseAccessor = AddAccessor("inversebindposematrices", TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_MAT4, 0, skeleton.GetBoneCount(), m_bufferViewIndices.inverseBindMatrices);
		gltfMdl.skins.at(exportData.skinIndex).inverseBindMatrices = bindPoseAccessor;
	}

	if(m_sceneDesc.cameras.empty() == false) {
		if(m_exportInfo.verbose)
			Con::cout << "Initializing " << m_sceneDesc.cameras.size() << " light sources..." << Con::endl;

		auto &cameras = gltfMdl.cameras;
		cameras.reserve(m_sceneDesc.cameras.size());
		for(auto &cam : m_sceneDesc.cameras) {
			auto nodeIdx = AddNode(cam.name, true);
			auto &gltfNode = gltfMdl.nodes.at(nodeIdx);

			ToGLTFPose(cam.pose, gltfNode.translation, gltfNode.rotation);
			int32_t camIndex = gltfMdl.cameras.size();
			gltfNode.name = cam.name;
			gltfNode.camera = camIndex;

			std::string type;
			switch(cam.type) {
			case Camera::Type::Orthographic:
				type = "orthographic";
				break;
			case Camera::Type::Perspective:
			default:
				type = "perspective";
				break;
			}

			cameras.push_back({});
			auto &camData = cameras.back();
			camData.name = cam.name;
			camData.type = type;
			if(cam.type == Camera::Type::Perspective) {
				camData.perspective.aspectRatio = cam.aspectRatio;

				// Note: Pragma uses horizontal FOV, but glTF expects vertical FOV, so this value should be converted.
				// However when importing the glTF into Blender, the FOV only matches ours if we're using horizontal FOV here.
				// The reason for this is currently unknown, either Blender (v2.9) interprets the glTF FOV as horizontal by mistake,
				// or something is wrong on this side. TODO: Compare the behavior with another application that supports glTF assets with camera data.
				camData.perspective.yfov = umath::deg_to_rad(cam.vFov);

				camData.perspective.znear = pragma::units_to_metres(cam.zNear);
				camData.perspective.zfar = pragma::units_to_metres(cam.zFar);
			}
		}
	}

	if(m_sceneDesc.lightSources.empty() == false) {
		if(m_exportInfo.verbose)
			Con::cout << "Initializing " << m_sceneDesc.lightSources.size() << " light sources..." << Con::endl;
		tinygltf::Value::Array lights {};
		for(auto i = decltype(m_sceneDesc.lightSources.size()) {0u}; i < m_sceneDesc.lightSources.size(); ++i) {
			auto &lightSource = m_sceneDesc.lightSources.at(i);
			auto outerConeAngle = lightSource.outerConeAngle;
			auto blendFraction = lightSource.blendFraction;
			auto innerConeAngle = 1.f - blendFraction * outerConeAngle;
			if(lightSource.type == LightSource::Type::Spot) {
				if(outerConeAngle <= innerConeAngle) {
					if(innerConeAngle <= 0.f) {
						Con::cwar << "WARNING Spot light has cone angle of 0! Skipping..." << Con::endl;
						continue;
					}
					Con::cwar << "WARNING Spot light has outer cone angle of " << outerConeAngle << ", which is smaller or equal to inner cone angle of " << innerConeAngle << "! This is not allowed! Clamping..." << Con::endl;
					outerConeAngle = innerConeAngle;
					innerConeAngle = umath::max(innerConeAngle - 10.f, 0.1f);
					if(outerConeAngle <= innerConeAngle) {
						Con::cwar << "WARNING Spot light has cone angle of near 0! Skipping..." << Con::endl;
						continue;
					}
				}
			}

			auto nodeName = "light_node" + std::to_string(i);
			auto nodeIdx = AddNode(nodeName, true);
			auto &gltfNode = gltfMdl.nodes.at(nodeIdx);

			int32_t lightSourceIndex = i;
			ToGLTFPose(lightSource.pose, gltfNode.translation, gltfNode.rotation);
			gltfNode.name = nodeName;
			gltfNode.extensions["KHR_lights_punctual"] = tinygltf::Value {tinygltf::Value::Object {{"light", tinygltf::Value {lightSourceIndex}}}};

			std::string type = "point";
			switch(lightSource.type) {
			case LightSource::Type::Point:
				type = "point";
				break;
			case LightSource::Type::Spot:
				type = "spot";
				break;
			case LightSource::Type::Directional:
				type = "directional";
				break;
			}

			// Note: The glTF specification states that the light source intensity should be specified in candela.
			// However, the Blender Cycles renderer does not convert it to radiometric units, and since we're
			// primarily targeting Blender, we'll just do the conversion ourselves.
			// TODO: Add an option to change this via a parameter?
			auto color = lightSource.color.ToVector3();
			auto intensity = lightSource.luminousIntensity;
			auto colMax = umath::max(color.r, color.g, color.b);
			if(colMax > 1.f) {
				color /= colMax;
				intensity *= colMax;
			}
			auto lightType = (lightSource.type == LightSource::Type::Spot) ? pragma::LightType::Spot : (lightSource.type == LightSource::Type::Directional) ? pragma::LightType::Directional : pragma::LightType::Point;
			intensity = (lightType == pragma::LightType::Spot) ? ulighting::cycles::lumen_to_watt_spot(intensity, color, outerConeAngle)
			  : (lightType == pragma::LightType::Point)        ? ulighting::cycles::lumen_to_watt_point(intensity, color)
			                                                         : ulighting::cycles::lumen_to_watt_area(intensity, color);

			auto lightName = lightSource.name;
			if(lightName.empty())
				lightName = type;
			lightName += "_" + std::to_string(i);
			tinygltf::Value::Object light {{"name", tinygltf::Value {lightName}}, {"type", tinygltf::Value {type}}, {"color", tinygltf::Value {tinygltf::Value::Array {{tinygltf::Value {color.r}, tinygltf::Value {color.g}, tinygltf::Value {color.b}}}}},
			  {"intensity", tinygltf::Value {intensity}}};
			if(lightSource.range.has_value() && (lightSource.type == LightSource::Type::Point || lightSource.type == LightSource::Type::Spot))
				light["range"] = tinygltf::Value {*lightSource.range};
			if(lightSource.type == LightSource::Type::Spot) {
				light["spot"] = tinygltf::Value {tinygltf::Value::Object {{"innerConeAngle", tinygltf::Value {umath::deg_to_rad(innerConeAngle)}}, {"outerConeAngle", tinygltf::Value {umath::deg_to_rad(outerConeAngle)}}}};
			}
			lights.push_back(tinygltf::Value {light});
		}

		gltfMdl.extensions["KHR_lights_punctual"] = tinygltf::Value {tinygltf::Value::Object {{"lights", tinygltf::Value {lights}}}};
		gltfMdl.extensionsUsed.push_back("KHR_lights_punctual");
	}

	if(m_exportInfo.embedAnimations || m_animName.has_value()) {
		for(auto &exportData : m_uniqueModelExportList) {
			if(IsAnimated(exportData.model) == false)
				continue;
			WriteAnimations(exportData.model);
		}
	}

#if 0
	auto fVertexData = FileManager::OpenFile<VFilePtrReal>("vertices.bin","wb");
	fVertexData->Write(vertexData.data(),vertexData.size() *sizeof(vertexData.front()));
	fVertexData = nullptr;

	auto fIndexData = FileManager::OpenFile<VFilePtrReal>("indices.bin","wb");
	fIndexData->Write(indexData.data(),indexData.size() *sizeof(indexData.front()));
	fIndexData = nullptr;
#endif

	if(m_animName.has_value()) {
		outputPath += "animations/";
		name = *m_animName;
	}
	FileManager::CreatePath(outputPath.c_str());

	std::string ext = "gltf";
	if(m_exportInfo.saveAsBinary)
		ext = "glb";

	auto fileName = ufile::get_file_from_filename(name) + '.' + ext;

	auto writePath = FileManager::GetProgramPath() + '/' + outputPath + fileName;
	if(optOutPath)
		*optOutPath = outputPath + fileName;
	tinygltf::TinyGLTF writer {};
	std::string err;
	std::string warn;
	std::string output_filename(writePath);

	if(m_exportInfo.verbose)
		Con::cout << "Writing output file as '" << output_filename << "'..." << Con::endl;
	FileManager::RemoveSystemFile(output_filename.c_str()); // The glTF writer doesn't write anything if the file already exists
	auto result = writer.WriteGltfSceneToFile(&gltfMdl, output_filename, false, true, true, m_exportInfo.saveAsBinary);
	if(m_exportInfo.verbose) {
		if(result)
			Con::cout << "Successfully exported model '" << name << "' as '" << output_filename << "'!" << Con::endl;
		else if(err.empty() == false)
			Con::cwar << "Unable to export model '" << name << "' as '" << output_filename << "': " << err << Con::endl;
		else
			Con::cwar << "Unable to export model '" << name << "' as '" << output_filename << "': " << warn << Con::endl;
	}
	if(result == false) {
		if(err.empty() == false)
			outErrMsg = err;
		else
			outErrMsg = warn;
		return false;
	}
	if(m_exportInfo.embedAnimations == false && m_animName.has_value() == false) {
		for(auto &exportData : m_uniqueModelExportList) {
			if(IsAnimated(exportData.model) == false)
				continue;
			if(m_exportInfo.verbose)
				Con::cout << "Exporting animations..." << Con::endl;
			std::unordered_map<std::string, uint32_t> *anims = nullptr;
			exportData.model.GetAnimations(&anims);
			for(auto &pair : *anims) {
				std::string errMsg;
				if(pragma::asset::export_animation(exportData.model, pair.first, m_exportInfo, errMsg))
					continue;
				if(m_exportInfo.verbose)
					Con::cwar << "Unable to export animation '" << pair.first << "': " << errMsg << Con::endl;
			}
		}
	}
	return true;
}

Vector3 pragma::asset::GLTFWriter::TransformPos(const Vector3 &v) const { return v * m_exportInfo.scale; };

tinygltf::Scene &pragma::asset::GLTFWriter::GetScene() { return m_gltfMdl.scenes.back(); }

uint32_t pragma::asset::GLTFWriter::AddNode(const std::string &name, bool isRootNode)
{
	m_gltfMdl.nodes.push_back({});
	if(isRootNode == true)
		GetScene().nodes.push_back(m_gltfMdl.nodes.size() - 1);
	auto &node = m_gltfMdl.nodes.back();
	node.name = name;
	return m_gltfMdl.nodes.size() - 1;
};

tinygltf::Buffer &pragma::asset::GLTFWriter::AddBuffer(const std::string &name, uint32_t *optOutBufIdx)
{
	m_gltfMdl.buffers.push_back({});
	auto &buffer = m_gltfMdl.buffers.back();
	buffer.name = name;
	if(optOutBufIdx)
		*optOutBufIdx = m_gltfMdl.buffers.size() - 1;
	return buffer;
}

uint32_t pragma::asset::GLTFWriter::AddBufferView(const std::string &name, BufferIndex bufferIdx, uint64_t byteOffset, uint64_t byteLength, std::optional<uint64_t> byteStride)
{
	m_gltfMdl.bufferViews.push_back({});
	auto &bufferView = m_gltfMdl.bufferViews.back();
	bufferView.name = name;
	bufferView.buffer = bufferIdx;
	bufferView.byteOffset = byteOffset;
	bufferView.byteLength = byteLength;
	if(byteStride.has_value())
		bufferView.byteStride = *byteStride;
	return m_gltfMdl.bufferViews.size() - 1;
};

void pragma::asset::GLTFWriter::WriteSkeleton(ModelExportData &mdlData)
{
	auto &mdl = mdlData.model;
	auto &skeleton = mdl.GetSkeleton();
	auto &anims = mdl.GetAnimations();
	std::vector<Mat4> inverseBindPoseMatrices {};
	if(IsAnimated(mdl) || IsSkinned(mdl)) {
		if(m_exportInfo.verbose)
			Con::cout << "Initializing GLTF Skeleton..." << Con::endl;
		auto gltfRootNodeIdx = AddNode("skeleton_root", true);

		// Transform pose to relative
		auto referenceRelative = Frame::Create(mdl.GetReference());
		inverseBindPoseMatrices.resize(skeleton.GetBoneCount());
		std::function<void(pragma::animation::Bone &, const umath::Transform &)> fToRelativeTransforms = nullptr;
		fToRelativeTransforms = [this, &fToRelativeTransforms, &referenceRelative, &inverseBindPoseMatrices](pragma::animation::Bone &bone, const umath::Transform &parentPose) {
			auto pose = referenceRelative->GetBoneTransform(bone.ID) ? *referenceRelative->GetBoneTransform(bone.ID) : umath::Transform {};

			auto scaledPose = pose;
			scaledPose.SetOrigin(TransformPos(scaledPose.GetOrigin()));
			inverseBindPoseMatrices.at(bone.ID) = scaledPose.GetInverse().ToMatrix();

			auto relPose = parentPose.GetInverse() * pose;
			referenceRelative->SetBonePose(bone.ID, relPose);
			for(auto &pair : bone.children)
				fToRelativeTransforms(*pair.second, pose);
		};
		for(auto &pair : skeleton.GetRootBones())
			fToRelativeTransforms(*pair.second, umath::Transform {});

		auto &bones = skeleton.GetBones();

		mdlData.skinIndex = m_gltfMdl.skins.size();
		m_gltfMdl.skins.push_back({});
		auto &skin = m_gltfMdl.skins.back();
		skin.skeleton = gltfRootNodeIdx;
		skin.joints.reserve(bones.size());
		for(auto &bone : bones) {
			auto nodeIdx = AddNode(bone->name, false);
			m_boneIdxToNodeIdx[bone->ID] = nodeIdx;
			skin.joints.push_back(nodeIdx);
			if(bone->ID != skin.joints.size() - 1)
				throw std::logic_error {"Joint ID does not match bone ID!"};
		}

		std::unordered_set<uint32_t> traversedJoints {};
		std::function<void(pragma::animation::Bone &, tinygltf::Node &)> fIterateSkeleton = nullptr;
		fIterateSkeleton = [this, &fIterateSkeleton, &referenceRelative, &traversedJoints](pragma::animation::Bone &bone, tinygltf::Node &parentNode) {
			auto nodeIdx = m_boneIdxToNodeIdx[bone.ID];
			parentNode.children.push_back(nodeIdx);
			traversedJoints.insert(nodeIdx);
			auto &node = m_gltfMdl.nodes.at(nodeIdx);

			auto pose = referenceRelative->GetBoneTransform(bone.ID) ? *referenceRelative->GetBoneTransform(bone.ID) : umath::Transform {};
			auto pos = TransformPos(pose.GetOrigin());
			auto &rot = pose.GetRotation();
			auto *scale = referenceRelative->GetBoneScale(bone.ID);
			if(scale)
				node.scale = {scale->x, scale->y, scale->z};
			node.translation = {pos.x, pos.y, pos.z};
			node.rotation = {rot.x, rot.y, rot.z, rot.w};
			for(auto &pair : bone.children)
				fIterateSkeleton(*pair.second, node);
		};
		for(auto &pair : skeleton.GetRootBones())
			fIterateSkeleton(*pair.second, m_gltfMdl.nodes.at(gltfRootNodeIdx));

		auto numBones = skeleton.GetBoneCount();
		for(auto i = decltype(numBones) {0u}; i < numBones; ++i) {
			auto it = traversedJoints.find(m_boneIdxToNodeIdx[i]);
			if(it != traversedJoints.end())
				continue;
			if(m_exportInfo.verbose)
				Con::cwar << "Bone '" << skeleton.GetBone(i).lock()->name << "' has no parent but is not in list of root bones! Forcing into root bone list manually..." << Con::endl;
			fIterateSkeleton(*skeleton.GetBone(i).lock(), m_gltfMdl.nodes.at(gltfRootNodeIdx));
		}

		// Validation
//#define ENABLE_SKELETON_VALIDATION
#ifdef ENABLE_SKELETON_VALIDATION
		std::unordered_map<uint32_t, uint32_t> nodeParents {};
		for(auto i = decltype(m_gltfMdl.nodes.size()) {0u}; i < m_gltfMdl.nodes.size(); ++i) {
			auto &node = m_gltfMdl.nodes.at(i);
			for(auto childIdx : node.children) {
				auto it = nodeParents.find(childIdx);
				if(it != nodeParents.end())
					throw std::logic_error {"Child has multiple parents! This is not allowed!"};
				nodeParents.insert(std::make_pair(childIdx, i));
			}
		}
		std::function<uint32_t(uint32_t)> fFindRoot = nullptr;
		fFindRoot = [this, &nodeParents, &fFindRoot](uint32_t nodeIdx) -> uint32_t {
			auto it = nodeParents.find(nodeIdx);
			if(it == nodeParents.end())
				return nodeIdx;
			return fFindRoot(it->second);
		};
		uint32_t commonRoot = std::numeric_limits<uint32_t>::max();
		for(auto nodeIdx : skin.joints) {
			auto rootIdx = fFindRoot(nodeIdx);
			if(commonRoot == std::numeric_limits<uint32_t>::max()) {
				commonRoot = rootIdx;
				continue;
			}
			std::cout << "Node " << m_gltfMdl.nodes.at(nodeIdx).name << " has root " << m_gltfMdl.nodes.at(rootIdx).name << std::endl;
			if(rootIdx != commonRoot)
				throw std::logic_error {"All joints have to have the same common root!"};
		}
		//
#endif
	}

	if(IsSkinned(mdl)) {
		// Initialize skin buffer
		auto &skinBuffer = AddBuffer("skin", &m_bufferIndices.skin);
		auto &skinData = skinBuffer.data;
		uint64_t numVertWeights = 0;
		for(auto &mesh : mdlData.exportMeshes) {
			if(mesh->GetVertexWeights().empty())
				continue;
			numVertWeights += mesh->GetVertexCount();
		}
		skinData.resize(numVertWeights * sizeof(GLTFVertexWeight));
		uint64_t vertWeightOffset = 0;
		for(auto &mesh : mdlData.exportMeshes) {
			if(mesh->GetVertexWeights().empty())
				continue;
			auto &vertWeights = mesh->GetVertexWeights();
			auto numVerts = mesh->GetVertexCount();
			for(auto i = decltype(numVerts) {0u}; i < numVerts; ++i) {
				auto vw = (i < vertWeights.size()) ? vertWeights.at(i) : umath::VertexWeight {};
				auto *gltfVwData = skinData.data() + vertWeightOffset * sizeof(GLTFVertexWeight);
				GLTFVertexWeight gltfVw {};
				auto weightSum = 0.f;
				for(uint8_t i = 0; i < 4; ++i) {
					auto idx = vw.boneIds[i];
					if(idx < 0 || vw.weights[i] == 0.f) {
						idx = 0;
						vw.weights[i] = 0.f;
					}
					gltfVw.joints[i] = idx;
					weightSum += vw.weights[i];
				}
				if(weightSum > 0.f) {
					// Normalize weights
					for(uint8_t i = 0; i < 4; ++i)
						vw.weights[i] /= weightSum;
				}
				else
					vw.weights[0] = 1.f;
				gltfVw.weights = {vw.weights[0], vw.weights[1], vw.weights[2], vw.weights[3]};
				memcpy(gltfVwData, &gltfVw, sizeof(gltfVw));
				++vertWeightOffset;
			}
		}
		m_bufferViewIndices.joints = AddBufferView("joints", m_bufferIndices.skin, 0, skinData.size(), sizeof(GLTFVertexWeight));
		m_bufferViewIndices.weights = AddBufferView("weights", m_bufferIndices.skin, sizeof(GLTFVertexWeight::joints), skinData.size() - sizeof(GLTFVertexWeight::joints), sizeof(GLTFVertexWeight));
	}
	if(IsAnimated(mdl) || IsSkinned(mdl)) {
		// Inverse bind pose
		auto &invBindPoseBuffer = AddBuffer("inversebindpose", &m_bufferIndices.inverseBindMatrices);
		auto &invBindPoseData = invBindPoseBuffer.data;
		invBindPoseData.resize(inverseBindPoseMatrices.size() * sizeof(inverseBindPoseMatrices.front()));
		memcpy(invBindPoseData.data(), inverseBindPoseMatrices.data(), inverseBindPoseMatrices.size() * sizeof(inverseBindPoseMatrices.front()));
		m_bufferViewIndices.inverseBindMatrices = AddBufferView("inversebindpose", m_bufferIndices.inverseBindMatrices, 0, invBindPoseData.size(), {});
	}
}

void pragma::asset::GLTFWriter::WriteAnimations(::Model &mdl)
{
	if(m_exportInfo.verbose)
		Con::cout << "Initializing GLTF animations..." << Con::endl;

	// Animations
	auto &skeleton = mdl.GetSkeleton();
	auto &anims = mdl.GetAnimations();
	auto &bones = skeleton.GetBones();
	m_gltfMdl.animations.reserve(anims.size());
	auto *animList = m_exportInfo.GetAnimationList();
	for(auto i = decltype(anims.size()) {0u}; i < anims.size(); ++i) {
		auto &anim = anims.at(i);
		auto animName = mdl.GetAnimationName(i);

		if(m_animName.has_value() && ustring::compare(animName, *m_animName, false) == false)
			continue;
		if(animList && std::find(animList->begin(), animList->end(), animName) == animList->end())
			continue;
		auto &frames = anim->GetFrames();
		auto numFrames = frames.size();
		if(numFrames == 0)
			continue;
		m_gltfMdl.animations.push_back({});
		auto &gltfAnim = m_gltfMdl.animations.back();
		gltfAnim.name = animName;
		if(m_exportInfo.verbose)
			Con::cout << "Initializing GLTF animation '" << gltfAnim.name << "'..." << Con::endl;

		auto &boneList = anim->GetBoneList();
		auto numBones = boneList.size();

		enum class Channel : uint8_t {
			Translation = 0,
			Rotation,
			Scale,

			Count
		};

		auto fps = anim->GetFPS();

		auto useScales = false;
		for(auto &frame : anim->GetFrames()) {
			if(frame->GetBoneScales().empty())
				continue;
			auto &scales = frame->GetBoneScales();
			// Check if there are any actual meaningful scales
			auto it = std::find_if(scales.begin(), scales.end(), [](const Vector3 &scale) {
				constexpr auto EPSILON = 0.001f;
				return umath::abs(1.f - scale.x) > EPSILON || umath::abs(1.f - scale.y) > EPSILON || umath::abs(1.f - scale.z) > EPSILON;
			});
			if(it == scales.end())
				continue;
			useScales = true;
			break;
		}

		// Setup buffers
		std::vector<float> times {};
		times.reserve(numFrames);
		auto tMax = std::numeric_limits<float>::lowest();
		for(auto i = decltype(frames.size()) {0u}; i < frames.size(); ++i) {
			times.push_back((fps > 0) ? (i / static_cast<float>(fps)) : 0.f);
			tMax = umath::max(tMax, times.back());
		}

		uint32_t animBufferIdx;
		auto &animBuffer = AddBuffer("anim_" + gltfAnim.name, &animBufferIdx);
		auto &animData = animBuffer.data;
		auto sizePerVertex = sizeof(Vector3) + sizeof(Vector4);
		if(useScales)
			sizePerVertex += sizeof(Vector3);
		auto bufferSize = times.size() * sizeof(times.front()) + numBones * numFrames * sizePerVertex;
		animData.resize(bufferSize);
		memcpy(animData.data(), times.data(), times.size() * sizeof(times.front()));

		auto bufViewTimes = AddBufferView("anim_" + gltfAnim.name + "_times", animBufferIdx, 0, times.size() * sizeof(times.front()), {});
		auto timesAccessor = AddAccessor("anim_" + gltfAnim.name + "_times", TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_SCALAR, 0, times.size(), bufViewTimes);
		m_gltfMdl.accessors.at(timesAccessor).minValues = {0.f};
		m_gltfMdl.accessors.at(timesAccessor).maxValues = {tMax};

		// Setup frame buffers
		uint64_t dataOffset = times.size() * sizeof(times.front());
		gltfAnim.channels.reserve(numFrames * numBones * umath::to_integral(Channel::Count));
		gltfAnim.samplers.reserve(numFrames * umath::to_integral(Channel::Count));

		for(auto i = decltype(numBones) {0u}; i < numBones; ++i) {
			auto boneId = boneList.at(i);
			auto &bone = *skeleton.GetBone(boneId).lock();
			std::vector<Vector3> translations {};
			std::vector<Vector4> rotations {};
			std::vector<Vector3> scales {};
			translations.reserve(numFrames);
			rotations.reserve(numFrames);
			if(useScales)
				scales.reserve(numFrames);

			for(auto &frame : frames) {
				auto pose = frame->GetBoneTransform(i) ? *frame->GetBoneTransform(i) : umath::Transform {};

				auto pos = TransformPos(pose.GetOrigin());
				translations.push_back(pos);
				auto &rot = pose.GetRotation();
				rotations.push_back({rot.x, rot.y, rot.z, rot.w});

				if(useScales) {
					auto scale = frame->GetBoneScale(i) ? *frame->GetBoneScale(i) : Vector3 {1.f, 1.f, 1.f};
					scales.push_back(scale);
				}
			}
			std::string boneName = bone.name;
			auto bufBone = AddBufferView("anim_" + gltfAnim.name + "_bone_" + boneName + "_data", animBufferIdx, dataOffset, sizePerVertex * numFrames, {});

			// Write animation data to buffer
			memcpy(animData.data() + dataOffset, translations.data(), translations.size() * sizeof(translations.front()));
			dataOffset += translations.size() * sizeof(translations.front());

			memcpy(animData.data() + dataOffset, rotations.data(), rotations.size() * sizeof(rotations.front()));
			dataOffset += rotations.size() * sizeof(rotations.front());

			if(useScales) {
				memcpy(animData.data() + dataOffset, scales.data(), scales.size() * sizeof(scales.front()));
				dataOffset += scales.size() * sizeof(scales.front());
			}

			// Initialize accessors
			auto translationsAccessor = AddAccessor("anim_" + gltfAnim.name + "_bone_" + boneName + "_translations", TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC3, 0, numFrames, bufBone);
			auto rotationsAccessor = AddAccessor("anim_" + gltfAnim.name + "_bone_" + boneName + "_rotations", TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC4, translations.size() * sizeof(translations.front()), numFrames, bufBone);
			auto scalesAccessor = std::numeric_limits<uint32_t>::max();
			if(useScales) {
				scalesAccessor = AddAccessor("anim_" + gltfAnim.name + "_bone_" + boneName + "_scales", TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC3, translations.size() * sizeof(translations.front()) + rotations.size() * sizeof(rotations.front()), numFrames, bufBone);
			}

			// Initialize samplers
			gltfAnim.samplers.push_back({});
			auto &samplerTranslations = gltfAnim.samplers.back();
			samplerTranslations.input = timesAccessor;
			samplerTranslations.output = translationsAccessor;
			samplerTranslations.interpolation = "LINEAR";
			auto translationSamplerIdx = gltfAnim.samplers.size() - 1;

			gltfAnim.samplers.push_back({});
			auto &samplerRotations = gltfAnim.samplers.back();
			samplerRotations.input = timesAccessor;
			samplerRotations.output = rotationsAccessor;
			samplerRotations.interpolation = "LINEAR";
			auto rotationSamplerIdx = gltfAnim.samplers.size() - 1;

			auto scaleSamplerIdx = std::numeric_limits<size_t>::max();
			if(useScales) {
				gltfAnim.samplers.push_back({});
				auto &samplerScales = gltfAnim.samplers.back();
				samplerScales.input = timesAccessor;
				samplerScales.output = scalesAccessor;
				samplerScales.interpolation = "LINEAR";
				scaleSamplerIdx = gltfAnim.samplers.size() - 1;
			}

			// Initialize channels
			auto nodeIdx = m_boneIdxToNodeIdx[boneId];

			// Translation
			gltfAnim.channels.push_back({});
			auto &channelTranslation = gltfAnim.channels.back();
			channelTranslation.target_node = nodeIdx;
			channelTranslation.target_path = "translation";
			channelTranslation.sampler = translationSamplerIdx;

			// Rotation
			gltfAnim.channels.push_back({});
			auto &channelRot = gltfAnim.channels.back();
			channelRot.target_node = nodeIdx;
			channelRot.target_path = "rotation";
			channelRot.sampler = rotationSamplerIdx;

			// Scale
			if(useScales) {
				gltfAnim.channels.push_back({});
				auto &channelScale = gltfAnim.channels.back();
				channelScale.target_node = nodeIdx;
				channelScale.target_path = "scale";
				channelScale.sampler = scaleSamplerIdx;
			}
		}

		if(m_exportInfo.exportMorphTargets) {
			// Calculate flex weights
			std::vector<std::vector<float>> flexWeights {};
			auto numFlexes = mdl.GetFlexCount();
			flexWeights.resize(numFrames);
			for(auto &w : flexWeights)
				w.resize(numFlexes, 0.f);
			for(auto iFrame = decltype(numFrames) {0u}; iFrame < numFrames; ++iFrame) {
				// Collect flex controller weights
				std::vector<float> flexControllerWeights {};
				auto numFlexControllers = mdl.GetFlexControllerCount();
				flexControllerWeights.resize(numFlexControllers, 0.f);

				auto &flexFrameData = anim->GetFrames().at(iFrame)->GetFlexFrameData();
				for(auto i = decltype(flexFrameData.flexControllerIds.size()) {0u}; i < flexFrameData.flexControllerIds.size(); ++i) {
					auto flexConId = flexFrameData.flexControllerIds.at(i);
					auto weight = flexFrameData.flexControllerWeights.at(i);
					flexControllerWeights.at(flexConId) = weight;
				}
				for(auto flexId = decltype(numFlexes) {0u}; flexId < numFlexes; ++flexId) {
					auto weight = mdl.CalcFlexWeight(
					  flexId, [&flexControllerWeights](uint32_t flexConId) -> std::optional<float> { return (flexConId < flexControllerWeights.size()) ? flexControllerWeights.at(flexConId) : std::optional<float> {}; },
					  [&flexControllerWeights](uint32_t flexId) -> std::optional<float> { return std::optional<float> {}; });
					if(weight.has_value() == false)
						continue;
					flexWeights.at(iFrame).at(flexId) = *weight;
				}
			}

			for(auto &pair : m_meshesWithMorphTargets) {
				auto &morphSet = m_meshMorphSets.find(pair.first)->second;
				auto &meshNodeIndices = pair.second;
				auto numMorphs = morphSet.size();

				uint32_t morphBufferIdx;
				auto &morphTargetBuffer = AddBuffer("anim_" + gltfAnim.name + "_morph_target", &morphBufferIdx);
				auto &morphTargetData = morphTargetBuffer.data;

				std::vector<float> weights {};
				weights.resize(numMorphs * numFrames, 0.f);
				for(auto iFrame = decltype(numFrames) {0u}; iFrame < numFrames; ++iFrame) {
					for(auto iMorph = decltype(numMorphs) {0u}; iMorph < numMorphs; ++iMorph) {
						auto weightIdx = iFrame * numMorphs + iMorph;
						auto &set = morphSet.at(iMorph);
						auto flexId = set.flexId;
						auto flexWeight = flexWeights.at(iFrame).at(flexId);
						weights.at(weightIdx) = flexWeight;
					}
				}
				morphTargetData.resize(weights.size() * sizeof(weights.front()));
				memcpy(morphTargetData.data(), weights.data(), weights.size() * sizeof(weights.front()));

				auto bufViewMorphTargets = AddBufferView("anim_" + gltfAnim.name + "_morph_target_data", morphBufferIdx, 0, morphTargetData.size() * sizeof(morphTargetData.front()), {});

				// Initialize accessors
				auto weightsAccessor = AddAccessor("anim_" + gltfAnim.name + "_morph_target_weights", TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_SCALAR, 0, numMorphs * numFrames, bufViewMorphTargets);

				// Initialize samplers
				gltfAnim.samplers.push_back({});
				auto &samplerWeights = gltfAnim.samplers.back();
				samplerWeights.input = timesAccessor;
				samplerWeights.output = weightsAccessor;
				samplerWeights.interpolation = "LINEAR";
				auto weightsSamplerIdx = gltfAnim.samplers.size() - 1;

				// Weights
				gltfAnim.channels.reserve(meshNodeIndices.size());
				for(auto meshNodeIdx : meshNodeIndices) {
					gltfAnim.channels.push_back({});
					auto &channelWeight = gltfAnim.channels.back();
					channelWeight.target_path = "weights";
					channelWeight.sampler = weightsSamplerIdx;
					channelWeight.target_node = meshNodeIdx;
				}
			}
		}
	}
}

void pragma::asset::GLTFWriter::GenerateAO(::Model &mdl)
{
	if(m_exportInfo.verbose)
		Con::cout << "Generating ambient occlusion maps..." << Con::endl;
	std::string errMsg;
	auto job = pragma::asset::generate_ambient_occlusion(mdl, errMsg, false, m_exportInfo.aoResolution, m_exportInfo.aoSamples, m_exportInfo.aoDevice);
	if(job.has_value() == false) {
		if(m_exportInfo.verbose)
			Con::cwar << "Unable to create parallel jobs for ambient occlusion map generation! Ambient occlusion maps will not be available." << Con::endl;
	}
	else {
		job->Start();
		auto lastProgress = -1.f;
		if(m_exportInfo.verbose)
			Con::cout << "Waiting for ao job completion. This may take a while..." << Con::endl;
		while(job->IsComplete() == false) {
			job->Poll();
			auto progress = job->GetProgress();
			if(progress != lastProgress) {
				lastProgress = progress;
				if(m_exportInfo.verbose)
					Con::cout << "Ao progress: " << progress << Con::endl;
			}
			if(progress < 1.f)
				std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		if(job->IsSuccessful() == false) {
			if(m_exportInfo.verbose)
				Con::cwar << "Ao job has failed: " << job->GetResultMessage() << ". Ambient occlusion maps will not be available." << Con::endl;
		}
	}
	// Ambient occlusion generator may have applied some changes to some of the materials and/or textures.
	// Make sure we are up-to-date
	client->GetResourceWatcher().Poll();
}

void pragma::asset::GLTFWriter::WriteMaterials()
{
	if(ShouldExportMeshes() == false)
		return; // No point in exporting materials if we're not exporting meshes either

	if(m_exportInfo.verbose)
		Con::cout << "Collecting materials..." << Con::endl;
	std::vector<Material *> materials {};
	for(auto &mdlDesc : m_sceneDesc.modelCollection) {
		for(auto &hMat : mdlDesc.model.GetMaterials()) {
			if(!hMat)
				continue;
			auto *mat = hMat.get();
			auto it = std::find(materials.begin(), materials.end(), mat);
			if(it == materials.end()) {
				if(materials.size() == materials.capacity())
					materials.reserve(materials.size() * 1.5 + 100);
				materials.push_back(mat);
			}
		}
	}

	if(m_exportInfo.verbose)
		Con::cout << "Initializing " << materials.size() << " GLTF materials..." << Con::endl;
	auto fAddTexture = [this](const std::string &texPath) -> uint32_t {
		m_gltfMdl.images.push_back({});
		auto &img = m_gltfMdl.images.back();
		img.uri = ufile::get_file_from_filename(texPath);

		m_gltfMdl.textures.push_back({});
		auto &gltfTex = m_gltfMdl.textures.back();
		gltfTex.source = m_gltfMdl.images.size() - 1;
		auto texIdx = m_gltfMdl.textures.size() - 1;
		return texIdx;
	};

	m_gltfMdl.materials.reserve(materials.size());

	std::unordered_map<Material *, uint32_t> matTranslationTable {};
	for(auto *mat : materials) {
		std::string errMsg;
		auto texturePaths = pragma::asset::export_material(*mat, m_exportInfo.imageFormat, errMsg, &m_exportPath, m_exportInfo.normalizeTextureNames);
		if(texturePaths.has_value() == false)
			continue;

		m_gltfMdl.materials.push_back({});
		auto &gltfMat = m_gltfMdl.materials.back();
		if(m_exportInfo.verbose)
			Con::cout << "Initializing GLTF material '" << mat->GetName() << "'..." << Con::endl;
		gltfMat.name = ufile::get_file_from_filename(mat->GetName());
		ufile::remove_extension_from_filename(gltfMat.name);

		auto &data = mat->GetDataBlock();

		auto itAlbedo = texturePaths->find(Material::ALBEDO_MAP_IDENTIFIER);
		if(itAlbedo != texturePaths->end())
			gltfMat.pbrMetallicRoughness.baseColorTexture.index = fAddTexture(itAlbedo->second);

		Vector4 colorFactor {1.f, 1.f, 1.f, 1.f};
		data->GetVector3("color_factor", reinterpret_cast<Vector3 *>(&colorFactor));
		data->GetFloat("alpha_factor", &colorFactor.a);
		gltfMat.pbrMetallicRoughness.baseColorFactor = {colorFactor[0], colorFactor[1], colorFactor[2], colorFactor[3]};

		auto itNormal = texturePaths->find(Material::NORMAL_MAP_IDENTIFIER);
		if(itNormal != texturePaths->end())
			gltfMat.normalTexture.index = fAddTexture(itNormal->second);

		auto metalnessFactor = 0.f;
		auto roughnessFactor = 0.5f;
		auto itRMA = texturePaths->find(Material::RMA_MAP_IDENTIFIER);
		if(itRMA != texturePaths->end()) {
			gltfMat.pbrMetallicRoughness.metallicRoughnessTexture.index = fAddTexture(itRMA->second);
			metalnessFactor = 1.f;
			roughnessFactor = 1.f;
		}

		auto alphaMode = static_cast<int32_t>(AlphaMode::Opaque);
		data->GetInt("alpha_mode", &alphaMode);

		auto alphaCutoff = 0.5f;
		data->GetFloat("alpha_cutoff", &alphaCutoff);
		switch(static_cast<AlphaMode>(alphaMode)) {
		case AlphaMode::Mask:
			gltfMat.alphaMode = "MASK";
			break;
		case AlphaMode::Blend:
			gltfMat.alphaMode = "BLEND";
			break;
		case AlphaMode::Opaque:
		default:
			gltfMat.alphaMode = "OPAQUE";
			break;
		}
		gltfMat.alphaCutoff = alphaCutoff;

		auto itEmissive = texturePaths->find(Material::EMISSION_MAP_IDENTIFIER);
		if(itEmissive != texturePaths->end())
			gltfMat.emissiveTexture.index = fAddTexture(itEmissive->second);

		data->GetFloat("metalness_factor", &metalnessFactor);
		data->GetFloat("roughness_factor", &roughnessFactor);

		gltfMat.pbrMetallicRoughness.metallicFactor = metalnessFactor;
		gltfMat.pbrMetallicRoughness.roughnessFactor = roughnessFactor;

		auto &emissionFactor = data->GetValue("emission_factor");
		if(emissionFactor != nullptr && typeid(*emissionFactor) == typeid(ds::Vector)) {
			auto &f = static_cast<ds::Vector *>(emissionFactor.get())->GetValue();
			gltfMat.emissiveFactor = {f.r, f.g, f.b};
		}
		else if(gltfMat.emissiveTexture.index != -1)
			gltfMat.emissiveFactor = {1.0, 1.0, 1.0};
		m_materialToGltfIndex[mat] = m_gltfMdl.materials.size() - 1;
	}
}
