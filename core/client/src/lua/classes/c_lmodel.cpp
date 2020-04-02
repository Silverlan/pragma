#include "stdafx_client.h"
#include "pragma/lua/classes/c_lmodel.h"
#include "luasystem.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include <pragma/model/animation/vertex_animation.hpp>
#include <pragma/lua/classes/lmodel.h>
#include "pragma/lua/classes/ldef_model.h"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include "pragma/util/util_image.hpp"
#include "pragma/rendering/raytracing/cycles.hpp"
#include <util_image.hpp>
#include <util_texture_info.hpp>
#include <sharedutils/util_file.h>
#include <sharedutils/util_path.hpp>
#include <pragma/engine_version.h>
#include <pragma/lua/libraries/lfile.h>
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

#pragma optimize("",off)
void Lua::Model::Client::Export(lua_State *l,::Model &mdl,const ModelExportInfo &exportInfo)
{
	auto name = mdl.GetName();
	::util::Path exportPath {name,true};
	exportPath = "export/" +exportPath;
	auto exportPathStr = exportPath.GetString();
	if(Lua::file::validate_write_operation(l,exportPathStr) == false)
	{
		Lua::PushBool(l,false);
		return;
	}
	auto outputPath = ufile::get_path_from_filename(exportPathStr);
	FileManager::CreatePath(outputPath.c_str());
	auto exportImages = exportInfo.exportImages;
	auto exportAnimations = exportInfo.exportAnimations;
	auto exportSkinned = exportInfo.exportSkinnedMeshData;
	auto exportMorphTargets = exportInfo.exportMorphTargets;
	auto saveAsBinary = exportInfo.saveAsBinary;
	auto imageFormat = exportInfo.imageFormat;
	auto verbose = exportInfo.verbose;
	auto enableExtendedDDS = exportInfo.enableExtendedDDS;
	auto generateAo = exportInfo.generateAo;
	auto aoSamples = exportInfo.aoSamples;
	auto aoResolution = exportInfo.aoResolution;
	auto aoDevice = exportInfo.aoDevice;
	auto scale = exportInfo.scale;
	auto mergeSplitMeshes = exportInfo.mergeMeshesByMaterial;

	auto fTransformPos = [scale](const Vector3 &v) -> Vector3 {
		return v *scale;
	};

	tinygltf::Model gltfMdl {};
	gltfMdl.meshes.reserve(mdl.GetSubMeshCount());
	gltfMdl.asset.generator = "Pragma Engine " +get_pretty_engine_version();
	gltfMdl.asset.version = "2.0"; // GLTF version

	gltfMdl.defaultScene = 0;
	gltfMdl.scenes.push_back({});
	ufile::remove_extension_from_filename(name);
	std::replace(name.begin(),name.end(),'\\','/');
	auto &gltfScene = gltfMdl.scenes.back();
	gltfScene.name = name;

	auto fAddNode = [&gltfMdl,&gltfScene](const std::string &name,bool isRootNode) -> uint32_t {
		gltfMdl.nodes.push_back({});
		if(isRootNode == true)
			gltfScene.nodes.push_back(gltfMdl.nodes.size() -1);
		auto &node = gltfMdl.nodes.back();
		node.name = name;
		return gltfMdl.nodes.size() -1;
	};

	//auto &mdl = GetModel();
	//if(mdl != nullptr)
	//	mdl->GetBodyGroupMeshes(GetBodyGroups(),m_lodMeshes);
	//const_cast<Model*>(this)->GetMeshes(meshIds,outMeshes);

	std::vector<std::shared_ptr<ModelMesh>> meshList {};
	std::vector<uint32_t> bodyGroups {};
	bodyGroups.resize(mdl.GetBodyGroupCount(),0);
	mdl.GetBodyGroupMeshes(bodyGroups,meshList);

	std::vector<std::shared_ptr<ModelSubMesh>> meshes {};
	meshes.reserve(mdl.GetSubMeshCount());

	uint64_t indexCount = 0;
	uint64_t vertCount = 0;
	for(auto &mesh : meshList)
	{
		for(auto &subMesh : mesh->GetSubMeshes())
		{
			if(subMesh->GetTriangles().empty())
				continue;
			meshes.push_back(subMesh);
			indexCount += subMesh->GetTriangles().size();
			vertCount += subMesh->GetVertices().size();
		}
	}

	if(mergeSplitMeshes)
	{
		if(verbose)
			Con::cout<<"Merging meshes by materials..."<<Con::endl;
		std::unordered_map<uint32_t,std::vector<std::shared_ptr<ModelSubMesh>>> groupedMeshes {};
		for(auto &mesh : meshes)
		{
			auto texIdx = mesh->GetSkinTextureIndex();
			auto it = groupedMeshes.find(texIdx);
			if(it == groupedMeshes.end())
				it = groupedMeshes.insert(std::make_pair(texIdx,std::vector<std::shared_ptr<ModelSubMesh>>{})).first;
			it->second.push_back(mesh);
		}

		uint32_t numMerged = 0;
		std::vector<std::shared_ptr<ModelSubMesh>> mergedMeshes {};
		mergedMeshes.reserve(groupedMeshes.size());
		for(auto &pair : groupedMeshes)
		{
			auto &meshes = pair.second;
			if(meshes.size() == 1)
			{
				mergedMeshes.push_back(meshes.front());
				continue;
			}
			auto mesh = meshes.front()->Copy();
			for(auto it=meshes.begin() +1;it!=meshes.end();++it)
			{
				auto &meshOther = *it;
				mesh->Merge(*meshOther);
				++numMerged;
			}
			mergedMeshes.push_back(mesh);
		}
		meshes = mergedMeshes;
		if(verbose)
			Con::cout<<numMerged<<" meshes have been merged!"<<Con::endl;
	}

	// Initialize buffers
	if(verbose)
		Con::cout<<"Initializing GLTF buffers..."<<Con::endl;
	enum class Buffer : uint32_t
	{
		Indices = 0,
		Vertices,
		Skin,
		InverseBindMatrices,

		Count
	};
	auto fAddBuffer = [&gltfMdl](const std::string &name,uint32_t *optOutBufIdx=nullptr) -> tinygltf::Buffer& {
		gltfMdl.buffers.push_back({});
		auto &buffer = gltfMdl.buffers.back();
		buffer.name = name;
		if(optOutBufIdx)
			*optOutBufIdx = gltfMdl.buffers.size() -1;
		return buffer;
	};

	auto szVertex = sizeof(Vector3) *2 +sizeof(Vector2);
	gltfMdl.buffers.reserve(umath::to_integral(Buffer::Count) +mdl.GetAnimationCount() +mdl.GetVertexAnimations().size());
	auto &indexBuffer = fAddBuffer("indices");
	auto &vertexBuffer = fAddBuffer("vertices");
	//indexBuffer.uri = "indices.bin";
	//vertexBuffer.uri = "vertices.bin";

	uint64_t indexOffset = 0;
	uint64_t vertOffset = 0;
	//std::vector<uint8_t> indexData {};
	auto &indexData = indexBuffer.data;
	indexData.resize(indexCount *sizeof(uint32_t));
	//std::vector<uint8_t> vertexData {};
	auto &vertexData = vertexBuffer.data;
	vertexData.resize(vertCount *szVertex);
	for(auto &mesh : meshes)
	{
		auto &indices = mesh->GetTriangles();
		auto *gltfIndexData = indexData.data() +indexOffset *sizeof(uint32_t);
		for(auto i=decltype(indices.size()){0u};i<indices.size();++i)
		{
			auto idx = static_cast<uint32_t>(indices.at(i));
			memcpy(gltfIndexData +i *sizeof(uint32_t),&idx,sizeof(idx));
		}
		indexOffset += indices.size();

		auto &verts = mesh->GetVertices();
		auto *gltfVertexData = vertexData.data() +vertOffset *szVertex;
		for(auto i=decltype(verts.size()){0u};i<verts.size();++i)
		{
			auto &v = verts.at(i);
			auto pos = fTransformPos(v.position);
			memcpy(gltfVertexData +i *szVertex,&pos,sizeof(pos));
			memcpy(gltfVertexData +i *szVertex +sizeof(Vector3),&v.normal,sizeof(v.normal));
			memcpy(gltfVertexData +i *szVertex +sizeof(Vector3) *2,&v.uv,sizeof(v.uv));
		}
		vertOffset += verts.size();
	}

	// Initialize buffer views
	enum class BufferView : uint32_t
	{
		Indices = 0,
		Positions,
		Normals,
		Texcoords,
		Joints,
		Weights,
		InverseBindMatrices,

		Count
	};
	auto fAddBufferView = [&gltfMdl](const std::string &name,Buffer bufferIdx,uint64_t byteOffset,uint64_t byteLength,std::optional<uint64_t> byteStride) -> uint32_t {
		gltfMdl.bufferViews.push_back({});
		auto &bufferView = gltfMdl.bufferViews.back();
		bufferView.name = name;
		bufferView.buffer = umath::to_integral(bufferIdx);
		bufferView.byteOffset = byteOffset;
		bufferView.byteLength = byteLength;
		if(byteStride.has_value())
			bufferView.byteStride = *byteStride;
		return gltfMdl.bufferViews.size() -1;
	};
	uint32_t numFramesTotal = 0;
	for(auto &anim : mdl.GetAnimations())
		numFramesTotal += anim->GetFrameCount();
	gltfMdl.bufferViews.reserve(umath::to_integral(BufferView::Count) +mdl.GetAnimationCount() +numFramesTotal);
	fAddBufferView("indices",Buffer::Indices,0,indexBuffer.data.size(),{});
	fAddBufferView("positions",Buffer::Vertices,0,vertexBuffer.data.size(),szVertex);
	fAddBufferView("normals",Buffer::Vertices,sizeof(Vector3),vertexBuffer.data.size() -sizeof(Vector3),szVertex);
	fAddBufferView("texcoords",Buffer::Vertices,sizeof(Vector3) *2,vertexBuffer.data.size() -sizeof(Vector3) *2,szVertex);

	auto fAddAccessor = [&gltfMdl](const std::string &name,int componentType,int type,uint64_t byteOffset,uint64_t count,BufferView bufferViewIdx) -> uint32_t {
		gltfMdl.accessors.push_back({});
		auto &accessor = gltfMdl.accessors.back();
		accessor.componentType = componentType;
		accessor.count = count;
		accessor.byteOffset = byteOffset;
		accessor.type = type;
		accessor.name = name;
		accessor.bufferView = umath::to_integral(bufferViewIdx);
		return gltfMdl.accessors.size() -1;
	};
	gltfMdl.accessors.reserve(meshes.size() *umath::to_integral(BufferView::Count));

	// Skeleton
	struct GLTFVertexWeight
	{
		std::array<uint16_t,4> joints = {0,0,0,0};
		std::array<float,4> weights = {0.f,0.f,0.f,0.f};
	};
	auto &skeleton = mdl.GetSkeleton();
	auto &anims = mdl.GetAnimations();
	auto isSkinned = (skeleton.GetBoneCount() > 1 && exportSkinned);
	auto isAnimated = (isSkinned && anims.empty() == false && exportAnimations);
	int32_t skinIdx = -1;
	std::unordered_map<uint32_t,uint32_t> boneIdxToNodeIdx {};
	if(isSkinned)
	{
		if(verbose)
			Con::cout<<"Initializing GLTF Skeleton..."<<Con::endl;
		skinIdx = gltfMdl.skins.size();
		auto gltfRootNodeIdx = fAddNode("skeleton_root",true);

		gltfMdl.skins.push_back({});
		auto &skin = gltfMdl.skins.back();
		skin.skeleton = gltfRootNodeIdx;

		// Transform pose to relative
		auto referenceRelative = Frame::Create(mdl.GetReference());
		std::vector<Mat4> inverseBindPoseMatrices {};
		inverseBindPoseMatrices.resize(skeleton.GetBoneCount());
		std::function<void(::Bone&,const pragma::physics::Transform&)> fToRelativeTransforms = nullptr;
		fToRelativeTransforms = [&gltfMdl,&fAddNode,&fToRelativeTransforms,&fTransformPos,&skin,&referenceRelative,&inverseBindPoseMatrices](::Bone &bone,const pragma::physics::Transform &parentPose) {
			auto pose = referenceRelative->GetBoneTransform(bone.ID) ? *referenceRelative->GetBoneTransform(bone.ID) : pragma::physics::Transform{};

			auto scaledPose = pose;
			scaledPose.SetOrigin(fTransformPos(scaledPose.GetOrigin()));
			inverseBindPoseMatrices.at(bone.ID) = scaledPose.GetInverse().ToMatrix();

			auto relPose = parentPose.GetInverse() *pose;
			referenceRelative->SetBonePose(bone.ID,relPose);
			for(auto &pair : bone.children)
				fToRelativeTransforms(*pair.second,pose);
		};
		for(auto &pair : skeleton.GetRootBones())
			fToRelativeTransforms(*pair.second,pragma::physics::Transform{});


		gltfMdl.nodes.reserve(gltfMdl.nodes.size() +skeleton.GetBoneCount());
		auto &bones = skeleton.GetBones();
		skin.joints.reserve(bones.size());
		for(auto &bone : bones)
		{
			auto nodeIdx = fAddNode(bone->name,false);
			boneIdxToNodeIdx[bone->ID] = nodeIdx;
			skin.joints.push_back(nodeIdx);
			if(bone->ID != skin.joints.size() -1)
				throw std::logic_error{"Joint ID does not match bone ID!"};
		}

		std::function<void(::Bone&,tinygltf::Node&)> fIterateSkeleton = nullptr;
		fIterateSkeleton = [&gltfMdl,&fTransformPos,&fAddNode,&fIterateSkeleton,&skin,&referenceRelative,&boneIdxToNodeIdx](::Bone &bone,tinygltf::Node &parentNode) {
			auto nodeIdx = boneIdxToNodeIdx[bone.ID];
			parentNode.children.push_back(nodeIdx);
			auto &node = gltfMdl.nodes.at(nodeIdx);

			auto pose = referenceRelative->GetBoneTransform(bone.ID) ? *referenceRelative->GetBoneTransform(bone.ID) : pragma::physics::Transform{};
			auto pos = fTransformPos(pose.GetOrigin());
			auto &rot = pose.GetRotation();
			auto *scale = referenceRelative->GetBoneScale(bone.ID);
			if(scale)
				node.scale = {scale->x,scale->y,scale->z};
			node.translation = {pos.x,pos.y,pos.z};
			node.rotation = {rot.x,rot.y,rot.z,rot.w};
			for(auto &pair : bone.children)
				fIterateSkeleton(*pair.second,node);
		};
		for(auto &pair : skeleton.GetRootBones())
			fIterateSkeleton(*pair.second,gltfMdl.nodes.at(gltfRootNodeIdx));

		// Initialize skin buffer
		auto &skinBuffer = fAddBuffer("skin");
		auto &skinData = skinBuffer.data;
		uint64_t numVertWeights = 0;
		for(auto &mesh : meshes)
		{
			if(mesh->GetVertexWeights().empty())
				continue;
			numVertWeights += mesh->GetVertexCount();
		}
		skinData.resize(numVertWeights *sizeof(GLTFVertexWeight));
		uint64_t vertWeightOffset = 0;
		for(auto &mesh : meshes)
		{
			if(mesh->GetVertexWeights().empty())
				continue;
			auto &vertWeights = mesh->GetVertexWeights();
			auto numVerts = mesh->GetVertexCount();
			for(auto i=decltype(numVerts){0u};i<numVerts;++i)
			{
				auto vw = (i < vertWeights.size()) ? vertWeights.at(i) : VertexWeight{};
				auto *gltfVwData = skinData.data() +vertWeightOffset *sizeof(GLTFVertexWeight);
				GLTFVertexWeight gltfVw {};
				auto weightSum = 0.f;
				for(uint8_t i=0;i<4;++i)
				{
					auto idx = vw.boneIds[i];
					if(idx <= 0 || vw.weights[i] == 0.f)
					{
						idx = 0;
						vw.weights[i] = 0.f;
					}
					gltfVw.joints[i] = idx;
					weightSum += vw.weights[i];
				}
				if(weightSum > 0.f)
				{
					// Normalize weights
					for(uint8_t i=0;i<4;++i)
						vw.weights[i] /= weightSum;
				}
				else
					vw.weights[0] = 1.f;
				gltfVw.weights = {vw.weights[0],vw.weights[1],vw.weights[2],vw.weights[3]};
				memcpy(gltfVwData,&gltfVw,sizeof(gltfVw));
				++vertWeightOffset;
			}
		}
		fAddBufferView("joints",Buffer::Skin,0,skinData.size(),sizeof(GLTFVertexWeight));
		fAddBufferView("weights",Buffer::Skin,sizeof(GLTFVertexWeight::joints),skinData.size() -sizeof(GLTFVertexWeight::joints),sizeof(GLTFVertexWeight));

		// Inverse bind pose
		auto &invBindPoseBuffer = fAddBuffer("inversebindpose");
		auto &invBindPoseData = invBindPoseBuffer.data;
		invBindPoseData.resize(inverseBindPoseMatrices.size() *sizeof(inverseBindPoseMatrices.front()));
		memcpy(invBindPoseData.data(),inverseBindPoseMatrices.data(),inverseBindPoseMatrices.size() *sizeof(inverseBindPoseMatrices.front()));
		fAddBufferView("inversebindpose",Buffer::InverseBindMatrices,0,invBindPoseData.size(),{});
	}

	struct MorphSet
	{
		std::string name;
		MeshVertexFrame *frame = nullptr;
		Flex *flex = nullptr;
	};
	std::unordered_map<ModelSubMesh*,std::vector<MorphSet>> meshMorphSets;
	for(auto &flex : mdl.GetFlexes())
	{
		auto &name = flex.GetName();
		auto *anim = flex.GetMeshVertexAnimation();
		auto *frame = flex.GetMeshVertexFrame();
		auto *subMesh = anim ? anim->GetSubMesh() : nullptr;
		if(anim == nullptr || frame == nullptr || subMesh == nullptr)
			continue;
		auto &morphSets = meshMorphSets[subMesh];
		morphSets.push_back({});
		auto &morphSet = morphSets.back();
		morphSet.name = name;
		morphSet.frame = frame;
		morphSet.flex = &flex;
	}

	// Initialize accessors
	indexOffset = 0;
	vertOffset = 0;
	uint64_t vertexWeightOffset = 0;
	gltfMdl.meshes.reserve(meshes.size());
	gltfMdl.nodes.reserve(meshes.size());
	uint32_t meshIdx = 0;
	std::unordered_map<ModelSubMesh*,uint32_t> meshesWithMorphTargets {};
	for(auto &mesh : meshes)
	{
		auto nodeIdx = fAddNode(name +'_' +std::to_string(meshIdx),true);
		auto &gltfNode = gltfMdl.nodes.at(nodeIdx);
		gltfNode.mesh = gltfMdl.meshes.size();
		if(skinIdx != -1)
			gltfNode.skin = skinIdx;

		auto &verts = mesh->GetVertices();
		auto &tris = mesh->GetTriangles();
		gltfMdl.accessors.reserve(gltfMdl.accessors.size() +4);
		auto indicesAccessor = fAddAccessor("mesh" +std::to_string(meshIdx) +"_indices",TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT,TINYGLTF_TYPE_SCALAR,indexOffset *sizeof(uint32_t),tris.size(),BufferView::Indices);
		auto posAccessor = fAddAccessor("mesh" +std::to_string(meshIdx) +"_positions",TINYGLTF_COMPONENT_TYPE_FLOAT,TINYGLTF_TYPE_VEC3,vertOffset *szVertex,verts.size(),BufferView::Positions);
		auto normalAccessor = fAddAccessor("mesh" +std::to_string(meshIdx) +"_normals",TINYGLTF_COMPONENT_TYPE_FLOAT,TINYGLTF_TYPE_VEC3,vertOffset *szVertex,verts.size(),BufferView::Normals);
		auto uvAccessor = fAddAccessor("mesh" +std::to_string(meshIdx) +"_uvs",TINYGLTF_COMPONENT_TYPE_FLOAT,TINYGLTF_TYPE_VEC2,vertOffset *szVertex,verts.size(),BufferView::Texcoords);

		// Calculate bounds
		Vector3 min {std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max()};
		Vector3 max {std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest()};
		for(auto &v : verts)
		{
			uvec::min(&min,v.position);
			uvec::max(&max,v.position);
		}
		min = fTransformPos(min);
		max = fTransformPos(max);
		gltfMdl.accessors.at(posAccessor).minValues = {min.x,min.y,min.z};
		gltfMdl.accessors.at(posAccessor).maxValues = {max.x,max.y,max.z};
		//

		// Calculate index bounds
		auto minIndex = std::numeric_limits<int64_t>::max();
		auto maxIndex = std::numeric_limits<int64_t>::lowest();
		for(auto idx : tris)
		{
			minIndex = umath::min(minIndex,static_cast<int64_t>(idx));
			maxIndex = umath::max(maxIndex,static_cast<int64_t>(idx));
		}
		gltfMdl.accessors.at(indicesAccessor).minValues = {static_cast<double>(minIndex)};
		gltfMdl.accessors.at(indicesAccessor).maxValues = {static_cast<double>(maxIndex)};
		//

		gltfMdl.meshes.push_back({});
		auto &gltfMesh = gltfMdl.meshes.back();

		gltfMesh.primitives.push_back({});
		auto &primitive = gltfMesh.primitives.back();
		primitive.material = mesh->GetSkinTextureIndex();

		auto geometryType = mesh->GetGeometryType();
		switch(geometryType)
		{
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

		if(isSkinned && mesh->GetVertexWeights().empty() == false)
		{
			auto jointsAccessor = fAddAccessor("mesh" +std::to_string(meshIdx) +"_joints",TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT,TINYGLTF_TYPE_VEC4,vertexWeightOffset *sizeof(GLTFVertexWeight),verts.size(),BufferView::Joints);
			auto weightsAccessor = fAddAccessor("mesh" +std::to_string(meshIdx) +"_weights",TINYGLTF_COMPONENT_TYPE_FLOAT,TINYGLTF_TYPE_VEC4,vertexWeightOffset *sizeof(GLTFVertexWeight),verts.size(),BufferView::Weights);
			primitive.attributes["JOINTS_0"] = jointsAccessor;
			primitive.attributes["WEIGHTS_0"] = weightsAccessor;

			vertexWeightOffset += verts.size();
		}

		// Morphs
		if(exportMorphTargets)
		{
			auto itMorphSets = meshMorphSets.find(mesh.get());
			if(itMorphSets != meshMorphSets.end())
			{
				std::vector<tinygltf::Value> morphNames {};
				auto &morphSets = itMorphSets->second;
				morphNames.reserve(morphSets.size());
				for(auto &morphSet : morphSets)
				{
					morphNames.push_back(tinygltf::Value{morphSet.name});

					auto numVerts = morphSet.frame->GetVertexCount();
					std::vector<Vector3> vertices {};
					vertices.reserve(numVerts);
					Vector3 min {std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max()};
					Vector3 max {std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest()};
					for(auto i=decltype(numVerts){0u};i<numVerts;++i)
					{
						Vector3 pos {};
						morphSet.frame->GetVertexPosition(i,pos);
						pos = fTransformPos(pos);
						vertices.push_back(pos);

						uvec::min(&min,pos);
						uvec::max(&max,pos);
					}

					uint32_t morphBufferIdx;
					auto &morphBuffer = fAddBuffer("morph_" +morphSet.name,&morphBufferIdx);
					auto &morphData = morphBuffer.data;
					morphData.resize(vertices.size() *sizeof(vertices.front()));
					memcpy(morphData.data(),vertices.data(),vertices.size() *sizeof(vertices.front()));
					auto morphBufferView = fAddBufferView("morph" +morphSet.name,static_cast<Buffer>(morphBufferIdx),0,vertices.size() *sizeof(vertices.front()),{});
					auto morphAccessor = fAddAccessor("morph_" +morphSet.name +"_positions",TINYGLTF_COMPONENT_TYPE_FLOAT,TINYGLTF_TYPE_VEC3,0,vertices.size(),static_cast<BufferView>(morphBufferView));
					gltfMdl.accessors.at(morphAccessor).minValues = {min.x,min.y,min.z};
					gltfMdl.accessors.at(morphAccessor).maxValues = {max.x,max.y,max.z};

					primitive.targets.push_back({});
					auto &map = primitive.targets.back();
					map["POSITION"] = morphAccessor;
				}

				std::map<std::string,tinygltf::Value> extras {};
				extras["targetNames"] = tinygltf::Value{morphNames};
				gltfMesh.extras = tinygltf::Value{extras};

				meshesWithMorphTargets[mesh.get()] = nodeIdx;
			}
		}

		indexOffset += tris.size();
		vertOffset += verts.size();
		++meshIdx;
	}

	if(skinIdx != -1)
	{
		// Inverse bind matrix accessor has to be added last
		auto bindPoseAccessor = fAddAccessor("inversebindposematrices",TINYGLTF_COMPONENT_TYPE_FLOAT,TINYGLTF_TYPE_MAT4,0,skeleton.GetBoneCount(),BufferView::InverseBindMatrices);
		gltfMdl.skins.at(skinIdx).inverseBindMatrices = bindPoseAccessor;
	}

	if(isAnimated)
	{
		if(verbose)
			Con::cout<<"Initializing GLTF animations..."<<Con::endl;

		// Animations
		auto &bones = skeleton.GetBones();
		gltfMdl.animations.reserve(anims.size());
		for(auto i=decltype(anims.size()){0u};i<anims.size();++i)
		{
			auto &anim = anims.at(i);
			auto &frames = anim->GetFrames();
			auto numFrames = frames.size();
			if(numFrames == 0)
				continue;
			gltfMdl.animations.push_back({});
			auto &gltfAnim = gltfMdl.animations.back();
			gltfAnim.name = mdl.GetAnimationName(i);
			if(verbose)
				Con::cout<<"Initializing GLTF animation '"<<gltfAnim.name<<"'..."<<Con::endl;

			auto &boneList = anim->GetBoneList();
			auto numBones = boneList.size();

			enum class Channel : uint8_t
			{
				Translation = 0,
				Rotation,
				Scale,

				Count
			};

			auto fps = anim->GetFPS();

			auto useScales = false;
			for(auto &frame : anim->GetFrames())
			{
				if(frame->GetBoneScales().empty())
					continue;
				auto &scales = frame->GetBoneScales();
				// Check if there are any actual meaningful scales
				auto it = std::find_if(scales.begin(),scales.end(),[](const Vector3 &scale) {
					constexpr auto EPSILON = 0.001f;
					return umath::abs(1.f -scale.x) > EPSILON ||
						umath::abs(1.f -scale.y) > EPSILON || 
						umath::abs(1.f -scale.z) > EPSILON;
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
			for(auto i=decltype(frames.size()){0u};i<frames.size();++i)
			{
				times.push_back((fps > 0) ? (i /static_cast<float>(fps)) : 0.f);
				tMax = umath::max(tMax,times.back());
			}
			
			uint32_t animBufferIdx;
			auto &animBuffer = fAddBuffer("anim_" +gltfAnim.name,&animBufferIdx);
			auto &animData = animBuffer.data;
			auto sizePerVertex = sizeof(Vector3) +sizeof(Vector4);
			if(useScales)
				sizePerVertex += sizeof(Vector3);
			auto bufferSize = times.size() *sizeof(times.front()) +
				numBones *numFrames *sizePerVertex;
			animData.resize(bufferSize);
			memcpy(animData.data(),times.data(),times.size() *sizeof(times.front()));

			auto bufViewTimes = fAddBufferView("anim_" +gltfAnim.name +"_times",static_cast<Buffer>(animBufferIdx),0,times.size() *sizeof(times.front()),{});
			auto timesAccessor = fAddAccessor("anim_" +gltfAnim.name +"_times",TINYGLTF_COMPONENT_TYPE_FLOAT,TINYGLTF_TYPE_SCALAR,0,times.size(),static_cast<BufferView>(bufViewTimes));
			gltfMdl.accessors.at(timesAccessor).minValues = {0.f};
			gltfMdl.accessors.at(timesAccessor).maxValues = {tMax};

			// Setup frame buffers
			uint64_t dataOffset = times.size() *sizeof(times.front());
			gltfAnim.channels.reserve(numFrames *numBones *umath::to_integral(Channel::Count));
			gltfAnim.samplers.reserve(numFrames *umath::to_integral(Channel::Count));

			for(auto i=decltype(numBones){0u};i<numBones;++i)
			{
				auto boneId = boneList.at(i);
				auto &bone = *skeleton.GetBone(boneId).lock();
				std::vector<Vector3> translations {};
				std::vector<Vector4> rotations {};
				std::vector<Vector3> scales {};
				translations.reserve(numFrames);
				rotations.reserve(numFrames);
				if(useScales)
					scales.reserve(numFrames);

				for(auto &frame : frames)
				{
					auto pose = frame->GetBoneTransform(i) ? *frame->GetBoneTransform(i) : pragma::physics::Transform{};

					auto pos = fTransformPos(pose.GetOrigin());
					translations.push_back(pos);
					auto &rot = pose.GetRotation();
					rotations.push_back({rot.x,rot.y,rot.z,rot.w});

					if(useScales)
					{
						auto scale = frame->GetBoneScale(i) ? *frame->GetBoneScale(i) : Vector3{1.f,1.f,1.f};
						scales.push_back(scale);
					}
				}
				auto bufBone = fAddBufferView(
					"anim_" +gltfAnim.name +"_bone_" +bone.name +"_data",static_cast<Buffer>(animBufferIdx),
					dataOffset,(sizeof(Vector3) +sizeof(Vector4) +sizeof(Vector3)) *numFrames,{}
				);

				// Write animation data to buffer
				memcpy(animData.data() +dataOffset,translations.data(),translations.size() *sizeof(translations.front()));
				dataOffset += translations.size() *sizeof(translations.front());

				memcpy(animData.data() +dataOffset,rotations.data(),rotations.size() *sizeof(rotations.front()));
				dataOffset += rotations.size() *sizeof(rotations.front());

				if(useScales)
				{
					memcpy(animData.data() +dataOffset,scales.data(),scales.size() *sizeof(scales.front()));
					dataOffset += scales.size() *sizeof(scales.front());
				}

				// Initialize accessors
				auto translationsAccessor = fAddAccessor(
					"anim_" +gltfAnim.name +"_bone_" +bone.name +"_translations",
					TINYGLTF_COMPONENT_TYPE_FLOAT,TINYGLTF_TYPE_VEC3,0,
					numFrames,static_cast<BufferView>(bufBone)
				);
				auto rotationsAccessor = fAddAccessor(
					"anim_" +gltfAnim.name +"_bone_" +bone.name +"_rotations",
					TINYGLTF_COMPONENT_TYPE_FLOAT,TINYGLTF_TYPE_VEC4,translations.size() *sizeof(translations.front()),
					numFrames,static_cast<BufferView>(bufBone)
				);
				auto scalesAccessor = std::numeric_limits<uint32_t>::max();
				if(useScales)
				{
					scalesAccessor = fAddAccessor(
						"anim_" +gltfAnim.name +"_bone_" +bone.name +"_scales",
						TINYGLTF_COMPONENT_TYPE_FLOAT,TINYGLTF_TYPE_VEC3,translations.size() *sizeof(translations.front()) +rotations.size() *sizeof(rotations.front()),
						numFrames,static_cast<BufferView>(bufBone)
					);
				}

				// Initialize samplers
				gltfAnim.samplers.push_back({});
				auto &samplerTranslations = gltfAnim.samplers.back();
				samplerTranslations.input = timesAccessor;
				samplerTranslations.output = translationsAccessor;
				samplerTranslations.interpolation = "LINEAR";
				auto translationSamplerIdx = gltfAnim.samplers.size() -1;

				gltfAnim.samplers.push_back({});
				auto &samplerRotations = gltfAnim.samplers.back();
				samplerRotations.input = timesAccessor;
				samplerRotations.output = rotationsAccessor;
				samplerRotations.interpolation = "LINEAR";
				auto rotationSamplerIdx = gltfAnim.samplers.size() -1;

				auto scaleSamplerIdx = std::numeric_limits<size_t>::max();
				if(useScales)
				{
					gltfAnim.samplers.push_back({});
					auto &samplerScales = gltfAnim.samplers.back();
					samplerScales.input = timesAccessor;
					samplerScales.output = scalesAccessor;
					samplerScales.interpolation = "LINEAR";
					scaleSamplerIdx = gltfAnim.samplers.size() -1;
				}

				// Initialize channels
				auto nodeIdx = boneIdxToNodeIdx[boneId];

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
				if(useScales)
				{
					gltfAnim.channels.push_back({});
					auto &channelScale = gltfAnim.channels.back();
					channelScale.target_node = nodeIdx;
					channelScale.target_path = "scale";
					channelScale.sampler = scaleSamplerIdx;
				}
			}

			if(exportMorphTargets)
			{
				for(auto &pair : meshesWithMorphTargets)
				{
					auto &morphSet = meshMorphSets.find(pair.first)->second;
					auto meshNodeIdx = pair.second;
					auto numMorphs = morphSet.size();

					uint32_t morphBufferIdx;
					auto &morphTargetBuffer = fAddBuffer("anim_" +gltfAnim.name +"_morph_target",&morphBufferIdx);
					auto &morphTargetData = morphTargetBuffer.data;

					std::vector<float> weights {};
					weights.resize(numMorphs *numFrames,1.f);
					morphTargetData.resize(weights.size() *sizeof(weights.front()));
					memcpy(morphTargetData.data(),weights.data(),weights.size() *sizeof(weights.front()));

					auto bufViewMorphTargets = fAddBufferView(
						"anim_" +gltfAnim.name +"_morph_target_data",static_cast<Buffer>(morphBufferIdx),
						0,morphTargetData.size() *sizeof(morphTargetData.front()),{}
					);

					// Initialize accessors
					auto weightsAccessor = fAddAccessor(
						"anim_" +gltfAnim.name +"_morph_target_weights",
						TINYGLTF_COMPONENT_TYPE_FLOAT,TINYGLTF_TYPE_SCALAR,0,
						numMorphs *numFrames,static_cast<BufferView>(bufViewMorphTargets)
					);

					// Initialize samplers
					gltfAnim.samplers.push_back({});
					auto &samplerWeights = gltfAnim.samplers.back();
					samplerWeights.input = timesAccessor;
					samplerWeights.output = weightsAccessor;
					samplerWeights.interpolation = "LINEAR";
					auto weightsSamplerIdx = gltfAnim.samplers.size() -1;

					// Weights
					gltfAnim.channels.push_back({});
					auto &channelWeight = gltfAnim.channels.back();
					channelWeight.target_path = "weights";
					channelWeight.sampler = weightsSamplerIdx;
					channelWeight.target_node = meshNodeIdx;
				}
			}
		}
	}

	// Textures
#if 0
	auto fGetTexturePath = [](TextureInfo *texInfo) -> std::optional<std::string> {
		if(texInfo == nullptr || texInfo->texture == nullptr)
			return {};
		auto &tex = *static_cast<Texture*>(texInfo->texture.get());
		return tex.GetName();
	};
	//

	uint32_t defaultTexture = std::numeric_limits<uint32_t>::max();
	std::unordered_map<std::string,uint32_t> textureCache {};
	auto fAddTexture = [&textureCache,&defaultTexture,&gltfMdl,&fGetTexturePath](TextureInfo *texInfo) -> uint32_t {
		auto texPath = fGetTexturePath(texInfo);
		if(texPath.has_value() == false)
		{
			if(defaultTexture == std::numeric_limits<uint32_t>::max())
			{
				gltfMdl.textures.push_back({});
				defaultTexture = gltfMdl.textures.size() -1;
			}
			return defaultTexture;
		}
		auto normalizedPath = *texPath;
		std::replace(normalizedPath.begin(),normalizedPath.end(),'\\','/');
		auto it = textureCache.find(normalizedPath);
		if(it != textureCache.end())
			return it->second;

		gltfMdl.images.push_back({});
		auto &img = gltfMdl.images.back();
		img.uri = "materials/" +normalizedPath +".dds";

		gltfMdl.textures.push_back({});
		auto &gltfTex = gltfMdl.textures.back();
		gltfTex.source = gltfMdl.images.size() -1;
		auto texIdx = gltfMdl.textures.size() -1;
		textureCache[normalizedPath] = texIdx;
		return texIdx;
	};
#endif

	auto fGetTextureWriteInfo = [imageFormat,enableExtendedDDS](bool normalMap,bool srgb,std::string &outExt) -> uimg::TextureInfo {
		uimg::TextureInfo texWriteInfo {};
		switch(imageFormat)
		{
		case ModelExportInfo::ImageFormat::DDS:
			texWriteInfo.containerFormat = uimg::TextureInfo::ContainerFormat::DDS;
			outExt = "dds";
			break;
		case ModelExportInfo::ImageFormat::KTX:
			texWriteInfo.containerFormat = uimg::TextureInfo::ContainerFormat::KTX;
			outExt = "ktx";
			break;
		}
		texWriteInfo.flags = uimg::TextureInfo::Flags::GenerateMipmaps;
		if(normalMap)
			texWriteInfo.flags |= uimg::TextureInfo::Flags::NormalMap;
		// umath::set_flag(texWriteInfo.flags,uimg::TextureInfo::Flags::SRGB,srgb);
		texWriteInfo.inputFormat = uimg::TextureInfo::InputFormat::KeepInputImageFormat;
		texWriteInfo.outputFormat = uimg::TextureInfo::OutputFormat::KeepInputImageFormat;
		return texWriteInfo;
	};

	auto fSaveImage = [&fGetTextureWriteInfo,imageFormat,verbose](uimg::ImageBuffer &imgBuf,std::string &inOutImgOutputPath,bool normalMap,bool srgb) -> bool {
		imgBuf.Convert(uimg::ImageBuffer::Format::RGBA8); // TODO

		if(imageFormat == ModelExportInfo::ImageFormat::DDS || imageFormat == ModelExportInfo::ImageFormat::KTX)
		{
			std::string ext;
			auto texWriteInfo = fGetTextureWriteInfo(normalMap,srgb,ext);
			texWriteInfo.inputFormat = uimg::TextureInfo::InputFormat::R8G8B8A8_UInt;
			inOutImgOutputPath += '.' +ext;
			if(verbose)
				Con::cout<<"Saving image as '"<<inOutImgOutputPath<<"'..."<<Con::endl;
			return c_game->SaveImage(imgBuf,inOutImgOutputPath,texWriteInfo);
		}

		auto saveFormat = uimg::ImageFormat::PNG;
		switch(imageFormat)
		{
		case ModelExportInfo::ImageFormat::PNG:
			saveFormat = uimg::ImageFormat::PNG;
			break;
		case ModelExportInfo::ImageFormat::BMP:
			saveFormat = uimg::ImageFormat::BMP;
			break;
		case ModelExportInfo::ImageFormat::TGA:
			saveFormat = uimg::ImageFormat::TGA;
			break;
		case ModelExportInfo::ImageFormat::JPG:
			saveFormat = uimg::ImageFormat::JPG;
			break;
		case ModelExportInfo::ImageFormat::HDR:
			saveFormat = uimg::ImageFormat::HDR;
			break;
		}
		auto ext = uimg::get_file_extension(saveFormat);
		inOutImgOutputPath += '.' +ext;

		if(verbose)
			Con::cout<<"Saving image as '"<<inOutImgOutputPath<<"'..."<<Con::endl;
		auto fImg = FileManager::OpenFile<VFilePtrReal>(inOutImgOutputPath.c_str(),"wb");
		return uimg::save_image(fImg,imgBuf,saveFormat);
	};

	auto fSaveTexture = [&gltfMdl,&fAddBuffer,&fAddBufferView,&outputPath,&fSaveImage,&fGetTextureWriteInfo,verbose,exportImages,imageFormat,enableExtendedDDS](TextureInfo *texInfo,bool normalMap) -> std::optional<uint32_t> {
		if(exportImages == false)
			return {};
		auto *texture = texInfo ? static_cast<Texture*>(texInfo->texture.get()) : nullptr;
		if(texture == nullptr)
			return {};
		auto &vkImg = texture->GetVkTexture()->GetImage();

		auto imgPath = ufile::get_file_from_filename(texture->GetName());
		ufile::remove_extension_from_filename(imgPath);
		auto imgOutputPath = outputPath +imgPath;

		auto exportSuccess = false;
		if(imageFormat == ModelExportInfo::ImageFormat::DDS || imageFormat == ModelExportInfo::ImageFormat::KTX)
		{
			if(verbose)
				Con::cout<<"Saving image as '"<<imgOutputPath<<"'..."<<Con::endl;

			std::string ext;
			auto texWriteInfo = fGetTextureWriteInfo(normalMap,texture->HasFlag(Texture::Flags::SRGB),ext);
			imgOutputPath += '.' +ext;
			if(texWriteInfo.containerFormat == uimg::TextureInfo::ContainerFormat::DDS && enableExtendedDDS == false)
			{
				auto anvFormat = vkImg->GetFormat();
				switch(anvFormat)
				{
					// These formats require DDS10, which is not well supported, so we'll fall back to
					// a different compression format
				case Anvil::Format::BC6H_SFLOAT_BLOCK:
				case Anvil::Format::BC6H_UFLOAT_BLOCK:
				case Anvil::Format::BC7_SRGB_BLOCK:
				case Anvil::Format::BC7_UNORM_BLOCK:
					texWriteInfo.outputFormat = uimg::TextureInfo::OutputFormat::BC3;
					break;
				}
			}
			exportSuccess = c_game->SaveImage(*vkImg,imgOutputPath,texWriteInfo);
		}
		else
		{
			std::vector<std::vector<std::shared_ptr<uimg::ImageBuffer>>> imgBuffers;
			if(::util::to_image_buffer(*vkImg,imgBuffers) == false)
				return {};
			auto &imgBuf = imgBuffers.front().front();
			exportSuccess = fSaveImage(*imgBuf,imgOutputPath,normalMap,texture->HasFlag(Texture::Flags::SRGB));
		}
		if(exportSuccess == false)
		{
			if(verbose)
				Con::cwar<<"WARNING: Unable to save image '"<<imgOutputPath<<"'!"<<Con::endl;
			return {};
		}

		gltfMdl.images.push_back({});
		auto &img = gltfMdl.images.back();
		img.uri = ufile::get_file_from_filename(imgOutputPath);

		gltfMdl.textures.push_back({});
		auto &gltfTex = gltfMdl.textures.back();
		gltfTex.source = gltfMdl.images.size() -1;
		auto texIdx = gltfMdl.textures.size() -1;
		return texIdx;
	};

	auto fGenerateAo = [verbose,generateAo,aoSamples,aoResolution,aoDevice,&mdl,&fSaveImage,&gltfMdl,&outputPath](Material &mat,uint32_t matIdx) -> std::optional<uint32_t> {
		if(generateAo == false)
			return {};
		if(verbose)
			Con::cout<<"Generating ao map for material '"<<mat.GetName()<<"'..."<<Con::endl;
		pragma::rendering::cycles::SceneInfo sceneInfo {};
		sceneInfo.denoise = true;
		sceneInfo.hdrOutput = false;
		sceneInfo.width = aoResolution;
		sceneInfo.height = aoResolution;
		sceneInfo.samples = aoSamples;
		sceneInfo.device = aoDevice;
		std::shared_ptr<uimg::ImageBuffer> aoImg = nullptr;
		auto job = pragma::rendering::cycles::bake_ambient_occlusion(*client,sceneInfo,mdl,matIdx);
		if(job.IsValid())
		{
			bool complete = false;
			job.SetCompletionHandler([&complete,&fSaveImage,&aoImg](::util::ParallelWorker<std::shared_ptr<uimg::ImageBuffer>> &worker) {
				complete = true;
				if(worker.IsSuccessful() == false)
					return;
				aoImg = worker.GetResult();
			});
			job.Start();
			auto lastProgress = -1.f;
			while(complete == false)
			{
				job.Poll();
				auto progress = job.GetProgress();
				if(progress != lastProgress)
				{
					lastProgress = progress;
					Con::cout<<"AO progress: "<<progress<<Con::endl;
				}
				if(progress < 1.f)
					std::this_thread::sleep_for(std::chrono::seconds(1));
			}
		}
		if(aoImg == nullptr)
		{
			Con::cwar<<"WARNING: Unable to generate ambient occlusion map for material '"<<mat.GetName()<<"':"<<job.GetResultMessage()<<Con::endl;
			return {};
		}

		auto name = ufile::get_file_from_filename(mat.GetName());
		ufile::remove_extension_from_filename(name);
		name += "_ao";
		std::string aoPath = outputPath +name;
		auto success = fSaveImage(*aoImg,aoPath,false,false);
		if(success == false)
			return {};
		
		gltfMdl.images.push_back({});
		auto &img = gltfMdl.images.back();
		img.uri = ufile::get_file_from_filename(aoPath);

		gltfMdl.textures.push_back({});
		auto &gltfTex = gltfMdl.textures.back();
		gltfTex.source = gltfMdl.images.size() -1;
		auto texIdx = gltfMdl.textures.size() -1;
		return texIdx;
	};

	auto &materials = mdl.GetMaterials();
	gltfMdl.materials.reserve(materials.size());
	uint32_t matIdx = 0;
	std::unordered_map<uint32_t,uint32_t> matTranslationTable {};
	for(auto &mat : materials)
	{
		if(mat.IsValid() == false)
		{
			++matIdx;
			continue;
		}
		gltfMdl.materials.push_back({});
		auto &gltfMat = gltfMdl.materials.back();
		if(verbose)
			Con::cout<<"Initializing GLTF material '"<<mat.get()->GetName()<<"'..."<<Con::endl;
		gltfMat.name = mat.get()->GetName();
		auto albedoIdx = fSaveTexture(mat->GetAlbedoMap(),false);
		if(albedoIdx.has_value())
			gltfMat.pbrMetallicRoughness.baseColorTexture.index = *albedoIdx;

		auto normalIdx = fSaveTexture(mat->GetNormalMap(),true);
		if(normalIdx.has_value())
			gltfMat.normalTexture.index = *normalIdx;

		float metalnessFactor;
		float roughnessFactor;
		auto metallicRoughnessIdx = fSaveTexture(mat->GetTextureInfo("metalness_roughness_map"),true);
		if(metallicRoughnessIdx.has_value())
		{
			gltfMat.pbrMetallicRoughness.metallicRoughnessTexture.index = *metallicRoughnessIdx;
			metalnessFactor = 1.f;
			roughnessFactor = 1.f;
		}

		auto emissiveIdx = fSaveTexture(mat->GetGlowMap(),false);

		if(emissiveIdx.has_value())
			gltfMat.emissiveTexture.index = *emissiveIdx;

		auto aoIdx = fSaveTexture(mat->GetAmbientOcclusionMap(),false);
		if(aoIdx.has_value() == false)
			aoIdx = fGenerateAo(*mat.get(),matIdx);
		if(aoIdx.has_value())
			gltfMat.occlusionTexture.index = *aoIdx;

		auto &data = mat->GetDataBlock();
		data->GetFloat("metalness_factor",&metalnessFactor);
		
		// TODO: Specular factor!
		data->GetFloat("roughness_factor",&roughnessFactor);

		gltfMat.pbrMetallicRoughness.metallicFactor = metalnessFactor;
		gltfMat.pbrMetallicRoughness.roughnessFactor = roughnessFactor;

		float emissionFactor;
		if(data->GetFloat("emission_factor",&emissionFactor))
			gltfMat.emissiveFactor = {emissionFactor,emissionFactor,emissionFactor};
		matTranslationTable[matIdx] = gltfMdl.materials.size() -1;
		++matIdx;
	}

	// Translate material indices
	for(auto &mesh : gltfMdl.meshes)
	{
		for(auto &prim : mesh.primitives)
		{
			auto it = matTranslationTable.find(prim.material);
			prim.material = (it != matTranslationTable.end()) ? it->second : -1;
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
	auto fileName = ufile::get_file_from_filename(name) +".gltf";
	auto writePath = FileManager::GetProgramPath() +'/' +outputPath +fileName;
	tinygltf::TinyGLTF writer {};
	std::string err;
	std::string warn;
	std::string output_filename(writePath);

	if(verbose)
		Con::cout<<"Writing output file as '"<<output_filename<<"'..."<<Con::endl;
	FileManager::RemoveSystemFile(output_filename.c_str()); // The glTF writer doesn't write anything if the file already exists
	auto result = writer.WriteGltfSceneToFile(&gltfMdl,output_filename,false,true,true,saveAsBinary);
	Lua::PushBool(l,result);
	if(result == false)
	{
		Lua::PushString(l,err);
		Lua::PushString(l,warn);
	}
	if(verbose)
	{
		if(result)
			Con::cout<<"Successfully exported model '"<<name<<"' as '"<<output_filename<<"'!"<<Con::endl;
		else if(err.empty() == false)
			Con::cwar<<"WARNING: Unable to export model '"<<name<<"' as '"<<output_filename<<"': "<<err<<Con::endl;
		else
			Con::cwar<<"WARNING: Unable to export model '"<<name<<"' as '"<<output_filename<<"': "<<warn<<Con::endl;
	}
}
void Lua::Model::Client::AddMaterial(lua_State *l,::Model &mdl,uint32_t textureGroup,const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto *mat = client->LoadMaterial(name);
	Lua::Model::AddMaterial(l,mdl,textureGroup,mat);
}
void Lua::Model::Client::SetMaterial(lua_State *l,::Model &mdl,uint32_t texIdx,const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto *mat = client->LoadMaterial(name);
	Lua::Model::SetMaterial(l,mdl,texIdx,mat);
}
void Lua::Model::Client::GetVertexAnimationBuffer(lua_State *l,::Model &mdl)
{
	auto &buf = static_cast<CModel&>(mdl).GetVertexAnimationBuffer();
	if(buf == nullptr)
		return;
	Lua::Push(l,buf);
}
#pragma optimize("",on)
