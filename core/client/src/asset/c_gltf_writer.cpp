#include "stdafx_client.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "c_gltf_writer.hpp"
#include <pragma/clientstate/clientstate.h>
#include <pragma/model/model.h>
#include <pragma/model/modelmesh.h>
#include <pragma/model/animation/vertex_animation.hpp>
#include <pragma/model/animation/animation.h>
#include <sharedutils/util_path.hpp>
#include <sharedutils/util_file.h>
#include <pragma/engine_info.hpp>
#include <pragma/engine_version.h>
#include <pragma/util/resource_watcher.h>
#include <datasystem_color.h>

extern DLLCLIENT ClientState *client;

bool pragma::asset::GLTFWriter::Export(::Model &mdl,const pragma::asset::ModelExportInfo &exportInfo,std::string &outErrMsg,const std::string *optModelName)
{
	GLTFWriter writer {mdl,exportInfo,std::optional<std::string>{}};
	return writer.Export(outErrMsg,optModelName);
}
bool pragma::asset::GLTFWriter::Export(::Model &mdl,const std::string &animName,const pragma::asset::ModelExportInfo &exportInfo,std::string &outErrMsg,const std::string *optModelName)
{
	GLTFWriter writer {mdl,exportInfo,animName};
	return writer.Export(outErrMsg,optModelName);
}

pragma::asset::GLTFWriter::GLTFWriter(::Model &mdl,const ModelExportInfo &exportInfo,const std::optional<std::string> &animName)
	: m_model{mdl},m_exportInfo{exportInfo},m_animName{animName}
{}

uint32_t pragma::asset::GLTFWriter::AddAccessor(const std::string &name,int componentType,int type,uint64_t byteOffset,uint64_t count,BufferViewIndex bufferViewIdx)
{
	m_gltfMdl.accessors.push_back({});
	auto &accessor = m_gltfMdl.accessors.back();
	accessor.componentType = componentType;
	accessor.count = count;
	accessor.byteOffset = byteOffset;
	accessor.type = type;
	accessor.name = name;
	accessor.bufferView = bufferViewIdx;
	return m_gltfMdl.accessors.size() -1;
};

void pragma::asset::GLTFWriter::InitializeMorphSets()
{
	auto &flexes = m_model.GetFlexes();
	for(auto flexId=decltype(flexes.size()){0u};flexId<flexes.size();++flexId)
	{
		auto &flex = flexes.at(flexId);
		auto &name = flex.GetName();
		auto *va = flex.GetVertexAnimation();
		//auto *anim = flex.GetMeshVertexAnimation();
		//auto *frame = flex.GetMeshVertexFrame();
		//if(anim == nullptr || frame == nullptr || va == nullptr)
		//	continue;
		if(va == nullptr)
			continue;
		auto &meshAnims = va->GetMeshAnimations();
		uint32_t morphSetIndex = 0u;
		for(auto &meshAnim : meshAnims)
		{
			auto *subMesh = meshAnim->GetSubMesh();
			if(subMesh == nullptr)
				continue;
			auto *frame = va->GetMeshFrame(*subMesh,0);
			if(frame == nullptr)
				continue;
			auto &morphSets = m_meshMorphSets[subMesh];
			morphSets.push_back({});
			auto &morphSet = morphSets.back();
			morphSet.name = name;
			if(morphSetIndex > 0)
				morphSet.name += '_' +std::to_string(morphSetIndex);
			morphSet.frame = frame;
			morphSet.flexId = flexId;
			++morphSetIndex;
		}
	}
}

bool pragma::asset::GLTFWriter::IsSkinned() const
{
	if(ShouldExportMeshes() == false)
		return false;
	auto &skeleton = m_model.GetSkeleton();
	return (skeleton.GetBoneCount() > 1 && m_exportInfo.exportSkinnedMeshData);
}
bool pragma::asset::GLTFWriter::IsAnimated() const
{
	auto &anims = m_model.GetAnimations();
	auto &skeleton = m_model.GetSkeleton();
	return (skeleton.GetBoneCount() > 1 && anims.empty() == false && (m_exportInfo.exportAnimations || m_animName.has_value()));
}
bool pragma::asset::GLTFWriter::ShouldExportMeshes() const {return m_animName.has_value() == false;}
void pragma::asset::GLTFWriter::WriteMorphTargets(ModelSubMesh &mesh,tinygltf::Mesh &gltfMesh,tinygltf::Primitive &primitive,uint32_t nodeIdx)
{
	auto itMorphSets = m_meshMorphSets.find(&mesh);
	if(itMorphSets != m_meshMorphSets.end())
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
				pos = TransformPos(pos);
				vertices.push_back(pos);

				uvec::min(&min,pos);
				uvec::max(&max,pos);
			}

			uint32_t morphBufferIdx;
			auto &morphBuffer = AddBuffer("morph_" +morphSet.name,&morphBufferIdx);
			auto &morphData = morphBuffer.data;
			morphData.resize(vertices.size() *sizeof(vertices.front()));
			memcpy(morphData.data(),vertices.data(),vertices.size() *sizeof(vertices.front()));
			auto morphBufferView = AddBufferView("morph" +morphSet.name,morphBufferIdx,0,vertices.size() *sizeof(vertices.front()),{});
			auto morphAccessor = AddAccessor("morph_" +morphSet.name +"_positions",TINYGLTF_COMPONENT_TYPE_FLOAT,TINYGLTF_TYPE_VEC3,0,vertices.size(),morphBufferView);
			m_gltfMdl.accessors.at(morphAccessor).minValues = {min.x,min.y,min.z};
			m_gltfMdl.accessors.at(morphAccessor).maxValues = {max.x,max.y,max.z};

			primitive.targets.push_back({});
			auto &map = primitive.targets.back();
			map["POSITION"] = morphAccessor;
		}

		std::map<std::string,tinygltf::Value> extras {};
		extras["targetNames"] = tinygltf::Value{morphNames};
		gltfMesh.extras = tinygltf::Value{extras};

		m_meshesWithMorphTargets[&mesh] = nodeIdx;
	}
}

void pragma::asset::GLTFWriter::MergeSplitMeshes()
{
	if(m_exportInfo.verbose)
		Con::cout<<"Merging meshes by materials..."<<Con::endl;
	std::unordered_map<uint32_t,std::vector<std::shared_ptr<ModelSubMesh>>> groupedMeshes {};
	for(auto &mesh : m_meshes)
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
	m_meshes = mergedMeshes;
	if(m_exportInfo.verbose)
		Con::cout<<numMerged<<" meshes have been merged!"<<Con::endl;
}

bool pragma::asset::GLTFWriter::Export(std::string &outErrMsg,const std::string *optModelName)
{
	// HACK: If the model was just ported, we need to make sure the material and textures are in order by invoking the
	// resource watcher (in case they have been changed)
	// TODO: This doesn't belong here!
	client->GetResourceWatcher().Poll();

	auto name = optModelName ? *optModelName : m_model.GetName();
	::util::Path exportPath {name};
	exportPath.Canonicalize();
	auto exportPathStr = exportPath.GetString();
	auto outputPath = ufile::get_path_from_filename(exportPathStr) +ufile::get_file_from_filename(exportPathStr);
	ufile::remove_extension_from_filename(outputPath);
	outputPath += '/';
	m_exportPath = outputPath;
	outputPath = EXPORT_PATH +m_exportPath;

	auto &gltfMdl = m_gltfMdl;
	gltfMdl.meshes.reserve(m_model.GetSubMeshCount());
	gltfMdl.asset.generator = "Pragma Engine " +get_pretty_engine_version();
	gltfMdl.asset.version = "2.0"; // GLTF version

	gltfMdl.defaultScene = 0;
	gltfMdl.scenes.push_back({});
	ufile::remove_extension_from_filename(name);
	std::replace(name.begin(),name.end(),'\\','/');
	auto &gltfScene = gltfMdl.scenes.back();
	gltfScene.name = name;

	//auto &mdl = GetModel();
	//if(mdl != nullptr)
	//	mdl->GetBodyGroupMeshes(GetBodyGroups(),m_lodMeshes);
	//const_cast<Model*>(this)->GetMeshes(meshIds,outMeshes);

	std::vector<std::shared_ptr<ModelMesh>> meshList {};
	if(ShouldExportMeshes())
	{
		meshList.reserve(m_model.GetMeshCount());

		std::vector<uint32_t> bodyGroups {};
		bodyGroups.resize(m_model.GetBodyGroupCount(),0);
		if(m_exportInfo.fullExport == false)
			m_model.GetBodyGroupMeshes(bodyGroups,meshList);
		else
		{
			for(auto &meshGroup : m_model.GetMeshGroups())
			{
				for(auto &mesh : meshGroup->GetMeshes())
					meshList.push_back(mesh);
			}
		}
	}

	auto &meshes = m_meshes;
	meshes.reserve(m_model.GetSubMeshCount());

	if(m_exportInfo.mergeMeshesByMaterial)
		MergeSplitMeshes();

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

	// Initialize buffers
	if(m_exportInfo.verbose)
		Con::cout<<"Initializing GLTF buffers..."<<Con::endl;

	gltfMdl.buffers.reserve(BufferIndices::Count +m_model.GetAnimationCount() +m_model.GetVertexAnimations().size());

	constexpr auto szVertex = sizeof(Vector3) *2 +sizeof(Vector2);
	uint32_t numFramesTotal = 0;
	for(auto &anim : m_model.GetAnimations())
		numFramesTotal += anim->GetFrameCount();
	gltfMdl.bufferViews.reserve(BufferViewIndices::Count +m_model.GetAnimationCount() +numFramesTotal);
	if(ShouldExportMeshes())
	{
		auto &indexBuffer = AddBuffer("indices",&m_bufferIndices.indices);
		auto &vertexBuffer = AddBuffer("vertices",&m_bufferIndices.vertices);
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
				auto pos = TransformPos(v.position);
				memcpy(gltfVertexData +i *szVertex,&pos,sizeof(pos));
				memcpy(gltfVertexData +i *szVertex +sizeof(Vector3),&v.normal,sizeof(v.normal));
				memcpy(gltfVertexData +i *szVertex +sizeof(Vector3) *2,&v.uv,sizeof(v.uv));
			}
			vertOffset += verts.size();
		}

		m_bufferViewIndices.indices = AddBufferView("indices",m_bufferIndices.indices,0,indexBuffer.data.size(),{});
		m_bufferViewIndices.positions = AddBufferView("positions",m_bufferIndices.vertices,0,vertexBuffer.data.size(),szVertex);
		m_bufferViewIndices.normals = AddBufferView("normals",m_bufferIndices.vertices,sizeof(Vector3),vertexBuffer.data.size() -sizeof(Vector3),szVertex);
		m_bufferViewIndices.texCoords = AddBufferView("texcoords",m_bufferIndices.vertices,sizeof(Vector3) *2,vertexBuffer.data.size() -sizeof(Vector3) *2,szVertex);
	}

	gltfMdl.accessors.reserve(meshes.size() *BufferViewIndices::Count);

	// Skeleton
	WriteSkeleton();

	if(m_exportInfo.exportMorphTargets)
		InitializeMorphSets();

	// Initialize accessors
	uint64_t indexOffset = 0;
	uint64_t vertOffset = 0;
	uint64_t vertexWeightOffset = 0;
	gltfMdl.meshes.reserve(meshes.size());
	gltfMdl.nodes.reserve(meshes.size());
	uint32_t meshIdx = 0;
	for(auto &mesh : meshes)
	{
		auto nodeIdx = AddNode(name +'_' +std::to_string(meshIdx),true);
		auto &gltfNode = gltfMdl.nodes.at(nodeIdx);
		gltfNode.mesh = gltfMdl.meshes.size();
		if(m_skinIdx != -1)
			gltfNode.skin = m_skinIdx;

		auto &verts = mesh->GetVertices();
		auto &tris = mesh->GetTriangles();
		gltfMdl.accessors.reserve(gltfMdl.accessors.size() +4);
		auto indicesAccessor = AddAccessor("mesh" +std::to_string(meshIdx) +"_indices",TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT,TINYGLTF_TYPE_SCALAR,indexOffset *sizeof(uint32_t),tris.size(),m_bufferViewIndices.indices);
		auto posAccessor = AddAccessor("mesh" +std::to_string(meshIdx) +"_positions",TINYGLTF_COMPONENT_TYPE_FLOAT,TINYGLTF_TYPE_VEC3,vertOffset *szVertex,verts.size(),m_bufferViewIndices.positions);
		auto normalAccessor = AddAccessor("mesh" +std::to_string(meshIdx) +"_normals",TINYGLTF_COMPONENT_TYPE_FLOAT,TINYGLTF_TYPE_VEC3,vertOffset *szVertex,verts.size(),m_bufferViewIndices.normals);
		auto uvAccessor = AddAccessor("mesh" +std::to_string(meshIdx) +"_uvs",TINYGLTF_COMPONENT_TYPE_FLOAT,TINYGLTF_TYPE_VEC2,vertOffset *szVertex,verts.size(),m_bufferViewIndices.texCoords);

		// Calculate bounds
		Vector3 min {std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max()};
		Vector3 max {std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest()};
		for(auto &v : verts)
		{
			uvec::min(&min,v.position);
			uvec::max(&max,v.position);
		}
		min = TransformPos(min);
		max = TransformPos(max);
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

		if(IsSkinned() && mesh->GetVertexWeights().empty() == false)
		{
			auto jointsAccessor = AddAccessor("mesh" +std::to_string(meshIdx) +"_joints",TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT,TINYGLTF_TYPE_VEC4,vertexWeightOffset *sizeof(GLTFVertexWeight),verts.size(),m_bufferViewIndices.joints);
			auto weightsAccessor = AddAccessor("mesh" +std::to_string(meshIdx) +"_weights",TINYGLTF_COMPONENT_TYPE_FLOAT,TINYGLTF_TYPE_VEC4,vertexWeightOffset *sizeof(GLTFVertexWeight),verts.size(),m_bufferViewIndices.weights);
			primitive.attributes["JOINTS_0"] = jointsAccessor;
			primitive.attributes["WEIGHTS_0"] = weightsAccessor;

			vertexWeightOffset += verts.size();
		}

		// Morphs
		if(m_exportInfo.exportMorphTargets)
			WriteMorphTargets(*mesh,gltfMesh,primitive,nodeIdx);

		indexOffset += tris.size();
		vertOffset += verts.size();
		++meshIdx;
	}

	if(m_skinIdx != -1)
	{
		// Inverse bind matrix accessor has to be added last
		auto &skeleton = m_model.GetSkeleton();
		auto bindPoseAccessor = AddAccessor("inversebindposematrices",TINYGLTF_COMPONENT_TYPE_FLOAT,TINYGLTF_TYPE_MAT4,0,skeleton.GetBoneCount(),m_bufferViewIndices.inverseBindMatrices);
		gltfMdl.skins.at(m_skinIdx).inverseBindMatrices = bindPoseAccessor;
	}

	if(IsAnimated() && (m_exportInfo.embedAnimations || m_animName.has_value()))
		WriteAnimations();

	if(m_exportInfo.generateAo && ShouldExportMeshes())
		GenerateAO();

	WriteMaterials();

#if 0
	auto fVertexData = FileManager::OpenFile<VFilePtrReal>("vertices.bin","wb");
	fVertexData->Write(vertexData.data(),vertexData.size() *sizeof(vertexData.front()));
	fVertexData = nullptr;

	auto fIndexData = FileManager::OpenFile<VFilePtrReal>("indices.bin","wb");
	fIndexData->Write(indexData.data(),indexData.size() *sizeof(indexData.front()));
	fIndexData = nullptr;
#endif

	if(m_animName.has_value())
	{
		outputPath += "animations/";
		name = *m_animName;
	}
	FileManager::CreatePath(outputPath.c_str());

	std::string ext = "gltf";
	if(m_exportInfo.saveAsBinary)
		ext = "glb";

	auto fileName = ufile::get_file_from_filename(name) +'.' +ext;
	auto writePath = FileManager::GetProgramPath() +'/' +outputPath +fileName;
	tinygltf::TinyGLTF writer {};
	std::string err;
	std::string warn;
	std::string output_filename(writePath);

	if(m_exportInfo.verbose)
		Con::cout<<"Writing output file as '"<<output_filename<<"'..."<<Con::endl;
	FileManager::RemoveSystemFile(output_filename.c_str()); // The glTF writer doesn't write anything if the file already exists
	auto result = writer.WriteGltfSceneToFile(&gltfMdl,output_filename,false,true,true,m_exportInfo.saveAsBinary);
	if(m_exportInfo.verbose)
	{
		if(result)
			Con::cout<<"Successfully exported model '"<<name<<"' as '"<<output_filename<<"'!"<<Con::endl;
		else if(err.empty() == false)
			Con::cwar<<"WARNING: Unable to export model '"<<name<<"' as '"<<output_filename<<"': "<<err<<Con::endl;
		else
			Con::cwar<<"WARNING: Unable to export model '"<<name<<"' as '"<<output_filename<<"': "<<warn<<Con::endl;
	}
	if(result == false)
	{
		if(err.empty() == false)
			outErrMsg = err;
		else
			outErrMsg = warn;
		return false;
	}
	if(IsAnimated() && m_exportInfo.embedAnimations == false && m_animName.has_value() == false)
	{
		if(m_exportInfo.verbose)
			Con::cout<<"Exporting animations..."<<Con::endl;
		std::unordered_map<std::string,uint32_t> *anims = nullptr;
		m_model.GetAnimations(&anims);
		for(auto &pair : *anims)
		{
			std::string errMsg;
			if(pragma::asset::export_animation(m_model,pair.first,m_exportInfo,errMsg))
				continue;
			if(m_exportInfo.verbose)
				Con::cwar<<"WARNING: Unable to export animation '"<<pair.first<<"': "<<errMsg<<Con::endl;
		}
	}
	return true;
}

Vector3 pragma::asset::GLTFWriter::TransformPos(const Vector3 &v) const {return v *m_exportInfo.scale;};

tinygltf::Scene &pragma::asset::GLTFWriter::GetScene() {return m_gltfMdl.scenes.back();}

uint32_t pragma::asset::GLTFWriter::AddNode(const std::string &name,bool isRootNode)
{
	m_gltfMdl.nodes.push_back({});
	if(isRootNode == true)
		GetScene().nodes.push_back(m_gltfMdl.nodes.size() -1);
	auto &node = m_gltfMdl.nodes.back();
	node.name = name;
	return m_gltfMdl.nodes.size() -1;
};

tinygltf::Buffer &pragma::asset::GLTFWriter::AddBuffer(const std::string &name,uint32_t *optOutBufIdx)
{
	m_gltfMdl.buffers.push_back({});
	auto &buffer = m_gltfMdl.buffers.back();
	buffer.name = name;
	if(optOutBufIdx)
		*optOutBufIdx = m_gltfMdl.buffers.size() -1;
	return buffer;
}

uint32_t pragma::asset::GLTFWriter::AddBufferView(const std::string &name,BufferIndex bufferIdx,uint64_t byteOffset,uint64_t byteLength,std::optional<uint64_t> byteStride)
{
	m_gltfMdl.bufferViews.push_back({});
	auto &bufferView = m_gltfMdl.bufferViews.back();
	bufferView.name = name;
	bufferView.buffer = bufferIdx;
	bufferView.byteOffset = byteOffset;
	bufferView.byteLength = byteLength;
	if(byteStride.has_value())
		bufferView.byteStride = *byteStride;
	return m_gltfMdl.bufferViews.size() -1;
};

void pragma::asset::GLTFWriter::WriteSkeleton()
{
	auto &skeleton = m_model.GetSkeleton();
	auto &anims = m_model.GetAnimations();
	std::vector<Mat4> inverseBindPoseMatrices {};
	if(IsAnimated() || IsSkinned())
	{
		if(m_exportInfo.verbose)
			Con::cout<<"Initializing GLTF Skeleton..."<<Con::endl;
		auto gltfRootNodeIdx = AddNode("skeleton_root",true);

		// Transform pose to relative
		auto referenceRelative = Frame::Create(m_model.GetReference());
		inverseBindPoseMatrices.resize(skeleton.GetBoneCount());
		std::function<void(::Bone&,const pragma::physics::Transform&)> fToRelativeTransforms = nullptr;
		fToRelativeTransforms = [this,&fToRelativeTransforms,&referenceRelative,&inverseBindPoseMatrices](::Bone &bone,const pragma::physics::Transform &parentPose) {
			auto pose = referenceRelative->GetBoneTransform(bone.ID) ? *referenceRelative->GetBoneTransform(bone.ID) : pragma::physics::Transform{};

			auto scaledPose = pose;
			scaledPose.SetOrigin(TransformPos(scaledPose.GetOrigin()));
			inverseBindPoseMatrices.at(bone.ID) = scaledPose.GetInverse().ToMatrix();

			auto relPose = parentPose.GetInverse() *pose;
			referenceRelative->SetBonePose(bone.ID,relPose);
			for(auto &pair : bone.children)
				fToRelativeTransforms(*pair.second,pose);
		};
		for(auto &pair : skeleton.GetRootBones())
			fToRelativeTransforms(*pair.second,pragma::physics::Transform{});


		m_gltfMdl.nodes.reserve(m_gltfMdl.nodes.size() +skeleton.GetBoneCount());
		auto &bones = skeleton.GetBones();

		m_skinIdx = m_gltfMdl.skins.size();
		m_gltfMdl.skins.push_back({});
		auto &skin = m_gltfMdl.skins.back();
		skin.skeleton = gltfRootNodeIdx;
		skin.joints.reserve(bones.size());
		for(auto &bone : bones)
		{
			auto nodeIdx = AddNode(bone->name,false);
			m_boneIdxToNodeIdx[bone->ID] = nodeIdx;
			skin.joints.push_back(nodeIdx);
			if(bone->ID != skin.joints.size() -1)
				throw std::logic_error{"Joint ID does not match bone ID!"};
		}

		std::unordered_set<uint32_t> traversedJoints {};
		std::function<void(::Bone&,tinygltf::Node&)> fIterateSkeleton = nullptr;
		fIterateSkeleton = [this,&fIterateSkeleton,&referenceRelative,&traversedJoints](::Bone &bone,tinygltf::Node &parentNode) {
			auto nodeIdx = m_boneIdxToNodeIdx[bone.ID];
			parentNode.children.push_back(nodeIdx);
			traversedJoints.insert(nodeIdx);
			auto &node = m_gltfMdl.nodes.at(nodeIdx);

			auto pose = referenceRelative->GetBoneTransform(bone.ID) ? *referenceRelative->GetBoneTransform(bone.ID) : pragma::physics::Transform{};
			auto pos = TransformPos(pose.GetOrigin());
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
			fIterateSkeleton(*pair.second,m_gltfMdl.nodes.at(gltfRootNodeIdx));

		auto numBones = skeleton.GetBoneCount();
		for(auto i=decltype(numBones){0u};i<numBones;++i)
		{
			auto it = traversedJoints.find(m_boneIdxToNodeIdx[i]);
			if(it != traversedJoints.end())
				continue;
			if(m_exportInfo.verbose)
				Con::cwar<<"WARNING: Bone '"<<skeleton.GetBone(i).lock()->name<<"' has no parent but is not in list of root bones! Forcing into root bone list manually..."<<Con::endl;
			fIterateSkeleton(*skeleton.GetBone(i).lock(),m_gltfMdl.nodes.at(gltfRootNodeIdx));
		}

		// Validation
//#define ENABLE_SKELETON_VALIDATION
#ifdef ENABLE_SKELETON_VALIDATION
		std::unordered_map<uint32_t,uint32_t> nodeParents {};
		for(auto i=decltype(m_gltfMdl.nodes.size()){0u};i<m_gltfMdl.nodes.size();++i)
		{
			auto &node = m_gltfMdl.nodes.at(i);
			for(auto childIdx : node.children)
			{
				auto it = nodeParents.find(childIdx);
				if(it != nodeParents.end())
					throw std::logic_error{"Child has multiple parents! This is not allowed!"};
				nodeParents.insert(std::make_pair(childIdx,i));
			}
		}
		std::function<uint32_t(uint32_t)> fFindRoot = nullptr;
		fFindRoot = [this,&nodeParents,&fFindRoot](uint32_t nodeIdx) -> uint32_t {
			auto it = nodeParents.find(nodeIdx);
			if(it == nodeParents.end())
				return nodeIdx;
			return fFindRoot(it->second);
		};
		uint32_t commonRoot = std::numeric_limits<uint32_t>::max();
		for(auto nodeIdx : skin.joints)
		{
			auto rootIdx = fFindRoot(nodeIdx);
			if(commonRoot == std::numeric_limits<uint32_t>::max())
			{
				commonRoot = rootIdx;
				continue;
			}
			std::cout<<"Node "<<m_gltfMdl.nodes.at(nodeIdx).name<<" has root "<<m_gltfMdl.nodes.at(rootIdx).name<<std::endl;
			if(rootIdx != commonRoot)
				throw std::logic_error{"All joints have to have the same common root!"};
		}
		//
#endif
	}

	if(IsSkinned())
	{
		// Initialize skin buffer
		auto &skinBuffer = AddBuffer("skin",&m_bufferIndices.skin);
		auto &skinData = skinBuffer.data;
		uint64_t numVertWeights = 0;
		for(auto &mesh : m_meshes)
		{
			if(mesh->GetVertexWeights().empty())
				continue;
			numVertWeights += mesh->GetVertexCount();
		}
		skinData.resize(numVertWeights *sizeof(GLTFVertexWeight));
		uint64_t vertWeightOffset = 0;
		for(auto &mesh : m_meshes)
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
					if(idx < 0 || vw.weights[i] == 0.f)
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
		m_bufferViewIndices.joints = AddBufferView("joints",m_bufferIndices.skin,0,skinData.size(),sizeof(GLTFVertexWeight));
		m_bufferViewIndices.weights = AddBufferView("weights",m_bufferIndices.skin,sizeof(GLTFVertexWeight::joints),skinData.size() -sizeof(GLTFVertexWeight::joints),sizeof(GLTFVertexWeight));
	}
	if(IsAnimated() || IsSkinned())
	{
		// Inverse bind pose
		auto &invBindPoseBuffer = AddBuffer("inversebindpose",&m_bufferIndices.inverseBindMatrices);
		auto &invBindPoseData = invBindPoseBuffer.data;
		invBindPoseData.resize(inverseBindPoseMatrices.size() *sizeof(inverseBindPoseMatrices.front()));
		memcpy(invBindPoseData.data(),inverseBindPoseMatrices.data(),inverseBindPoseMatrices.size() *sizeof(inverseBindPoseMatrices.front()));
		m_bufferViewIndices.inverseBindMatrices = AddBufferView("inversebindpose",m_bufferIndices.inverseBindMatrices,0,invBindPoseData.size(),{});
	}
}

void pragma::asset::GLTFWriter::WriteAnimations()
{
	if(m_exportInfo.verbose)
		Con::cout<<"Initializing GLTF animations..."<<Con::endl;

	// Animations
	auto &skeleton = m_model.GetSkeleton();
	auto &anims = m_model.GetAnimations();
	auto &bones = skeleton.GetBones();
	m_gltfMdl.animations.reserve(anims.size());
	auto *animList = m_exportInfo.GetAnimationList();
	for(auto i=decltype(anims.size()){0u};i<anims.size();++i)
	{
		auto &anim = anims.at(i);
		auto animName = m_model.GetAnimationName(i);

		if(m_animName.has_value() && ustring::compare(animName,*m_animName,false) == false)
			continue;
		if(animList && std::find(animList->begin(),animList->end(),animName) == animList->end())
			continue;
		auto &frames = anim->GetFrames();
		auto numFrames = frames.size();
		if(numFrames == 0)
			continue;
		m_gltfMdl.animations.push_back({});
		auto &gltfAnim = m_gltfMdl.animations.back();
		gltfAnim.name = animName;
		if(m_exportInfo.verbose)
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
		auto &animBuffer = AddBuffer("anim_" +gltfAnim.name,&animBufferIdx);
		auto &animData = animBuffer.data;
		auto sizePerVertex = sizeof(Vector3) +sizeof(Vector4);
		if(useScales)
			sizePerVertex += sizeof(Vector3);
		auto bufferSize = times.size() *sizeof(times.front()) +
			numBones *numFrames *sizePerVertex;
		animData.resize(bufferSize);
		memcpy(animData.data(),times.data(),times.size() *sizeof(times.front()));

		auto bufViewTimes = AddBufferView("anim_" +gltfAnim.name +"_times",animBufferIdx,0,times.size() *sizeof(times.front()),{});
		auto timesAccessor = AddAccessor("anim_" +gltfAnim.name +"_times",TINYGLTF_COMPONENT_TYPE_FLOAT,TINYGLTF_TYPE_SCALAR,0,times.size(),bufViewTimes);
		m_gltfMdl.accessors.at(timesAccessor).minValues = {0.f};
		m_gltfMdl.accessors.at(timesAccessor).maxValues = {tMax};

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

				auto pos = TransformPos(pose.GetOrigin());
				translations.push_back(pos);
				auto &rot = pose.GetRotation();
				rotations.push_back({rot.x,rot.y,rot.z,rot.w});

				if(useScales)
				{
					auto scale = frame->GetBoneScale(i) ? *frame->GetBoneScale(i) : Vector3{1.f,1.f,1.f};
					scales.push_back(scale);
				}
			}
			auto bufBone = AddBufferView(
				"anim_" +gltfAnim.name +"_bone_" +bone.name +"_data",animBufferIdx,
				dataOffset,sizePerVertex *numFrames,{}
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
			auto translationsAccessor = AddAccessor(
				"anim_" +gltfAnim.name +"_bone_" +bone.name +"_translations",
				TINYGLTF_COMPONENT_TYPE_FLOAT,TINYGLTF_TYPE_VEC3,0,
				numFrames,bufBone
			);
			auto rotationsAccessor = AddAccessor(
				"anim_" +gltfAnim.name +"_bone_" +bone.name +"_rotations",
				TINYGLTF_COMPONENT_TYPE_FLOAT,TINYGLTF_TYPE_VEC4,translations.size() *sizeof(translations.front()),
				numFrames,bufBone
			);
			auto scalesAccessor = std::numeric_limits<uint32_t>::max();
			if(useScales)
			{
				scalesAccessor = AddAccessor(
					"anim_" +gltfAnim.name +"_bone_" +bone.name +"_scales",
					TINYGLTF_COMPONENT_TYPE_FLOAT,TINYGLTF_TYPE_VEC3,translations.size() *sizeof(translations.front()) +rotations.size() *sizeof(rotations.front()),
					numFrames,bufBone
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
			if(useScales)
			{
				gltfAnim.channels.push_back({});
				auto &channelScale = gltfAnim.channels.back();
				channelScale.target_node = nodeIdx;
				channelScale.target_path = "scale";
				channelScale.sampler = scaleSamplerIdx;
			}
		}

		if(m_exportInfo.exportMorphTargets)
		{
			// Calculate flex weights
			std::vector<std::vector<float>> flexWeights {};
			auto numFlexes = m_model.GetFlexCount();
			flexWeights.resize(numFrames);
			for(auto &w : flexWeights)
				w.resize(numFlexes,0.f);
			for(auto iFrame=decltype(numFrames){0u};iFrame<numFrames;++iFrame)
			{
				// Collect flex controller weights
				std::vector<float> flexControllerWeights {};
				auto numFlexControllers = m_model.GetFlexControllerCount();
				flexControllerWeights.resize(numFlexControllers,0.f);

				auto &flexFrameData = anim->GetFrames().at(iFrame)->GetFlexFrameData();
				for(auto i=decltype(flexFrameData.flexControllerIds.size()){0u};i<flexFrameData.flexControllerIds.size();++i)
				{
					auto flexConId = flexFrameData.flexControllerIds.at(i);
					auto weight = flexFrameData.flexControllerWeights.at(i);
					flexControllerWeights.at(flexConId) = weight;
				}
				for(auto flexId=decltype(numFlexes){0u};flexId<numFlexes;++flexId)
				{
					auto weight = m_model.CalcFlexWeight(flexId,[&flexControllerWeights](uint32_t flexConId) -> std::optional<float> {
						return flexControllerWeights.at(flexConId);
					},[&flexControllerWeights](uint32_t flexId) -> std::optional<float> {
						return std::optional<float>{};
					});
					if(weight.has_value() == false)
						continue;
					flexWeights.at(iFrame).at(flexId) = *weight;
				}
			}
			
			for(auto &pair : m_meshesWithMorphTargets)
			{
				auto &morphSet = m_meshMorphSets.find(pair.first)->second;
				auto meshNodeIdx = pair.second;
				auto numMorphs = morphSet.size();

				uint32_t morphBufferIdx;
				auto &morphTargetBuffer = AddBuffer("anim_" +gltfAnim.name +"_morph_target",&morphBufferIdx);
				auto &morphTargetData = morphTargetBuffer.data;

				std::vector<float> weights {};
				weights.resize(numMorphs *numFrames,0.f);
				for(auto iFrame=decltype(numFrames){0u};iFrame<numFrames;++iFrame)
				{
					for(auto iMorph=decltype(numMorphs){0u};iMorph<numMorphs;++iMorph)
					{
						auto weightIdx = iFrame *numMorphs +iMorph;
						auto &set = morphSet.at(iMorph);
						auto flexId = set.flexId;
						auto flexWeight = flexWeights.at(iFrame).at(flexId);
						weights.at(weightIdx) = flexWeight;
					}
				}
				morphTargetData.resize(weights.size() *sizeof(weights.front()));
				memcpy(morphTargetData.data(),weights.data(),weights.size() *sizeof(weights.front()));

				auto bufViewMorphTargets = AddBufferView(
					"anim_" +gltfAnim.name +"_morph_target_data",morphBufferIdx,
					0,morphTargetData.size() *sizeof(morphTargetData.front()),{}
				);

				// Initialize accessors
				auto weightsAccessor = AddAccessor(
					"anim_" +gltfAnim.name +"_morph_target_weights",
					TINYGLTF_COMPONENT_TYPE_FLOAT,TINYGLTF_TYPE_SCALAR,0,
					numMorphs *numFrames,bufViewMorphTargets
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

void pragma::asset::GLTFWriter::GenerateAO()
{
	if(m_exportInfo.verbose)
		Con::cout<<"Generating ambient occlusion maps..."<<Con::endl;
	std::string errMsg;
	auto job = pragma::asset::generate_ambient_occlusion(m_model,errMsg,false,m_exportInfo.aoResolution,m_exportInfo.aoSamples,m_exportInfo.aoDevice);
	if(job.has_value() == false)
	{
		if(m_exportInfo.verbose)
			Con::cwar<<"WARNING: Unable to create parallel jobs for ambient occlusion map generation! Ambient occlusion maps will not be available."<<Con::endl;
	}
	else
	{
		job->Start();
		auto lastProgress = -1.f;
		if(m_exportInfo.verbose)
			Con::cout<<"Waiting for ao job completion. This may take a while..."<<Con::endl;
		while(job->IsComplete() == false)
		{
			job->Poll();
			auto progress = job->GetProgress();
			if(progress != lastProgress)
			{
				lastProgress = progress;
				if(m_exportInfo.verbose)
					Con::cout<<"Ao progress: "<<progress<<Con::endl;
			}
			if(progress < 1.f)
				std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		if(job->IsSuccessful() == false)
		{
			if(m_exportInfo.verbose)
				Con::cwar<<"WARNING: Ao job has failed: "<<job->GetResultMessage()<<". Ambient occlusion maps will not be available."<<Con::endl;
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
	auto &materials = m_model.GetMaterials();
	if(m_exportInfo.verbose)
		Con::cout<<"Initializing "<<materials.size()<<" GLTF materials..."<<Con::endl;
	auto fAddTexture = [this](const std::string &texPath) -> uint32_t {
		m_gltfMdl.images.push_back({});
		auto &img = m_gltfMdl.images.back();
		img.uri = ufile::get_file_from_filename(texPath);

		m_gltfMdl.textures.push_back({});
		auto &gltfTex = m_gltfMdl.textures.back();
		gltfTex.source = m_gltfMdl.images.size() -1;
		auto texIdx = m_gltfMdl.textures.size() -1;
		return texIdx;
	};

	m_gltfMdl.materials.reserve(materials.size());

	uint32_t matIdx = 0;
	std::unordered_map<uint32_t,uint32_t> matTranslationTable {};
	for(auto &mat : materials)
	{
		if(mat.IsValid() == false)
		{
			++matIdx;
			continue;
		}

		std::string errMsg;
		auto texturePaths = pragma::asset::export_material(*mat.get(),m_exportInfo.imageFormat,errMsg,&m_exportPath);
		if(texturePaths.has_value() == false)
		{
			++matIdx;
			continue;
		}

		m_gltfMdl.materials.push_back({});
		auto &gltfMat = m_gltfMdl.materials.back();
		if(m_exportInfo.verbose)
			Con::cout<<"Initializing GLTF material '"<<mat.get()->GetName()<<"'..."<<Con::endl;
		gltfMat.name = ufile::get_file_from_filename(mat.get()->GetName());
		ufile::remove_extension_from_filename(gltfMat.name);

		auto &data = mat->GetDataBlock();
		auto translucent = data->GetBool("translucent");

		auto itAlbedo = texturePaths->find(Material::ALBEDO_MAP_IDENTIFIER);
		if(itAlbedo != texturePaths->end())
			gltfMat.pbrMetallicRoughness.baseColorTexture.index = fAddTexture(itAlbedo->second);

		auto &color = data->GetValue("color");
		if(color != nullptr)
		{
			auto &col = static_cast<ds::Color*>(color.get())->GetValue();
			auto vCol = col.ToVector4();
			gltfMat.pbrMetallicRoughness.baseColorFactor = {vCol[0],vCol[1],vCol[2],vCol[3]};
		}

		auto itNormal = texturePaths->find(Material::NORMAL_MAP_IDENTIFIER);
		if(itNormal != texturePaths->end())
			gltfMat.normalTexture.index = fAddTexture(itNormal->second);

		auto metalnessFactor = 0.f;
		auto roughnessFactor = 0.5f;
		auto itRMA = texturePaths->find(Material::RMA_MAP_IDENTIFIER);
		if(itRMA != texturePaths->end())
		{
			gltfMat.pbrMetallicRoughness.metallicRoughnessTexture.index = fAddTexture(itRMA->second);
			metalnessFactor = 1.f;
			roughnessFactor = 1.f;
		}

		if(translucent)
			gltfMat.alphaMode = "BLEND";

		auto itEmissive = texturePaths->find(Material::EMISSION_MAP_IDENTIFIER);
		if(itEmissive != texturePaths->end())
			gltfMat.emissiveTexture.index = fAddTexture(itEmissive->second);

		data->GetFloat("metalness_factor",&metalnessFactor);
		data->GetFloat("roughness_factor",&roughnessFactor);

		gltfMat.pbrMetallicRoughness.metallicFactor = metalnessFactor;
		gltfMat.pbrMetallicRoughness.roughnessFactor = roughnessFactor;

		auto &emissionFactor = data->GetValue("emission_factor");
		if(emissionFactor != nullptr)
		{
			auto &col = static_cast<ds::Color*>(emissionFactor.get())->GetValue();
			auto vCol = col.ToVector4();
			gltfMat.emissiveFactor = {vCol[0],vCol[1],vCol[2]};
		}
		else if(gltfMat.emissiveTexture.index != -1)
			gltfMat.emissiveFactor = {1.0,1.0,1.0};
		matTranslationTable[matIdx] = m_gltfMdl.materials.size() -1;
		++matIdx;
	}

	// Translate material indices
	for(auto &mesh : m_gltfMdl.meshes)
	{
		for(auto &prim : mesh.primitives)
		{
			auto it = matTranslationTable.find(prim.material);
			prim.material = (it != matTranslationTable.end()) ? it->second : -1;
		}
	}
}
