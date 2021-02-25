/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/model/model.h"
#include "pragma/model/modelmesh.h"
#include "pragma/physics/collisionmesh.h"
#include "pragma/model/animation/fanim.h"
#include "pragma/physics/physsoftbodyinfo.hpp"
#include "pragma/model/animation/vertex_animation.hpp"
#include "pragma/model/animation/flex_animation.hpp"
#include "pragma/file_formats/wmd.h"
#include "pragma/asset/util_asset.hpp"
#include <fsys/filesystem.h>
#include <sharedutils/util_file.h>
#include <sharedutils/magic_enum.hpp>
#include <udm.hpp>

#define INDEX_OFFSET_INDEX_SIZE sizeof(uint64_t)
#define INDEX_OFFSET_MODEL_DATA 0
#define INDEX_OFFSET_MODEL_MESHES (INDEX_OFFSET_MODEL_DATA +1)
#define INDEX_OFFSET_LOD_DATA (INDEX_OFFSET_MODEL_MESHES +1)
#define INDEX_OFFSET_BODYGROUPS (INDEX_OFFSET_LOD_DATA +1)
#define INDEX_OFFSET_JOINTS (INDEX_OFFSET_BODYGROUPS +1)
#define INDEX_OFFSET_COLLISION_MESHES (INDEX_OFFSET_JOINTS +1)
#define INDEX_OFFSET_BONES (INDEX_OFFSET_COLLISION_MESHES +1)
#define INDEX_OFFSET_ANIMATIONS (INDEX_OFFSET_BONES +1)
#define INDEX_OFFSET_VERTEX_ANIMATIONS (INDEX_OFFSET_ANIMATIONS +1)
#define INDEX_OFFSET_FLEX_CONTROLLERS (INDEX_OFFSET_VERTEX_ANIMATIONS +1)
#define INDEX_OFFSET_FLEXES (INDEX_OFFSET_FLEX_CONTROLLERS +1)
#define INDEX_OFFSET_PHONEMES (INDEX_OFFSET_FLEXES +1)
#define INDEX_OFFSET_IK_CONTROLLERS (INDEX_OFFSET_PHONEMES +1)
#define INDEX_OFFSET_EYEBALLS (INDEX_OFFSET_IK_CONTROLLERS +1)
#define INDEX_OFFSET_FLEX_ANIMATIONS (INDEX_OFFSET_EYEBALLS +1)

static void write_offset(VFilePtrReal f,uint64_t offIndex)
{
	auto cur = f->Tell();
	f->Seek(offIndex);
	f->Write<uint64_t>(cur);
	f->Seek(cur);
}

static void to_vertex_list(ModelMesh &mesh,std::vector<Vertex> &vertices,std::unordered_map<ModelSubMesh*,std::vector<uint32_t>> &vertexIds)
{
	vertices.reserve(mesh.GetVertexCount());
	for(auto &subMesh : mesh.GetSubMeshes())
	{
		auto &verts = subMesh->GetVertices();
		auto itMesh = vertexIds.insert(std::remove_reference_t<decltype(vertexIds)>::value_type(subMesh.get(),{})).first;
		auto &vertIds = itMesh->second;
		vertIds.reserve(verts.size());
		for(auto &v : verts)
		{
			auto it = std::find(vertices.begin(),vertices.end(),v);
			if(it == vertices.end())
			{
				vertices.push_back({v.position,v.uv,v.normal,v.tangent});
				it = vertices.end() -1;
			}
			vertIds.push_back(static_cast<uint32_t>(it -vertices.begin()));
		}
	}
}

struct MeshBoneWeight
{
	MeshBoneWeight(uint64_t vId,float w)
		: vertId(vId),weight(w)
	{}
	uint64_t vertId;
	float weight;
};
static void to_vertex_weight_list(ModelMesh &mesh,std::unordered_map<uint32_t,std::vector<MeshBoneWeight>> &boneWeights,const std::unordered_map<ModelSubMesh*,std::vector<uint32_t>> &vertexIds)
{
	// TODO: Has to be the same order as 'to_vertex_list'!!
	for(auto &subMesh : mesh.GetSubMeshes())
	{
		auto &meshVerts = vertexIds.find(subMesh.get())->second;
		auto &weights = subMesh->GetVertexWeights();
		for(auto i=decltype(weights.size()){0};i<weights.size();++i)
		{
			auto vertId = meshVerts[i];
			auto &vertexWeight = weights[i];
			auto &weights = vertexWeight.weights;
			auto &boneId = vertexWeight.boneIds;
			for(uint8_t i=0;i<4;++i)
			{
				if(boneId[i] == -1)
					continue;
				auto it = boneWeights.find(boneId[i]);
				if(it == boneWeights.end())
					it = boneWeights.insert(std::remove_reference_t<decltype(boneWeights)>::value_type(boneId[i],{})).first;
				it->second.push_back({vertId,weights[i]});
			}
		}
	}
}

std::shared_ptr<Model> Model::Copy(Game *game,CopyFlags copyFlags) const
{
	auto fCreateModel = static_cast<std::shared_ptr<Model>(Game::*)(bool) const>(&Game::CreateModel);
	auto mdl = (game->*fCreateModel)(false);
	if(mdl == nullptr)
		return nullptr;
	mdl->m_metaInfo = m_metaInfo;
	mdl->m_bValid = m_bValid;
	mdl->m_mass = m_mass;
	mdl->m_meshCount = m_meshCount;
	mdl->m_subMeshCount = m_subMeshCount;
	mdl->m_vertexCount = m_vertexCount;
	mdl->m_triangleCount = m_triangleCount;
	mdl->m_maxEyeDeflection = m_maxEyeDeflection;
	mdl->m_eyeballs = m_eyeballs;
	mdl->m_flexAnimationNames = m_flexAnimationNames;
	mdl->m_blendControllers = m_blendControllers;
	mdl->m_meshGroups = m_meshGroups;
	mdl->m_bodyGroups = m_bodyGroups;
	mdl->m_hitboxes = m_hitboxes;
	mdl->m_reference = Frame::Create(*m_reference);
	mdl->m_name = m_name;
	mdl->m_bAllMaterialsLoaded = true;
	mdl->m_animations = m_animations;
	mdl->m_flexAnimations = m_flexAnimations;
	mdl->m_animationIDs = m_animationIDs;
	mdl->m_skeleton = std::make_unique<Skeleton>(*m_skeleton);
	mdl->m_bindPose = m_bindPose;
	mdl->m_eyeOffset = m_eyeOffset;
	mdl->m_collisionMin = m_collisionMin;
	mdl->m_collisionMax = m_collisionMax;
	mdl->m_renderMin = m_renderMin;
	mdl->m_renderMax = m_renderMax;
	mdl->m_collisionMeshes = m_collisionMeshes;
	mdl->m_joints = m_joints;
	mdl->m_baseMeshes = m_baseMeshes;
	mdl->m_lods = m_lods;
	mdl->m_attachments = m_attachments;
	mdl->m_objectAttachments = m_objectAttachments;
	mdl->m_materials = m_materials;
	mdl->m_textureGroups = m_textureGroups;
	mdl->m_phonemeMap = m_phonemeMap;
	mdl->m_flexControllers = m_flexControllers;
	mdl->m_flexes = m_flexes;
	for(auto &ikController : mdl->m_ikControllers)
		ikController = std::make_shared<IKController>(*ikController);
	std::unordered_map<ModelMesh*,ModelMesh*> oldMeshToNewMesh;
	std::unordered_map<ModelSubMesh*,ModelSubMesh*> oldSubMeshToNewSubMesh;
	if((copyFlags &CopyFlags::CopyMeshesBit) != CopyFlags::None)
	{
		for(auto &meshGroup : mdl->m_meshGroups)
		{
			auto newMeshGroup = ModelMeshGroup::Create(meshGroup->GetName());
			static_assert(sizeof(ModelMeshGroup) == 72,"Update this function when making changes to this class!");
			newMeshGroup->GetMeshes() = meshGroup->GetMeshes();
			for(auto &mesh : newMeshGroup->GetMeshes())
			{
				auto newMesh = mesh->Copy();
				oldMeshToNewMesh[mesh.get()] = newMesh.get();
				for(auto &subMesh : newMesh->GetSubMeshes())
				{
					auto newSubMesh = subMesh->Copy(true);
					oldSubMeshToNewSubMesh[subMesh.get()] = newSubMesh.get();
					subMesh = newSubMesh;
				}
				mesh = newMesh;
			}
			meshGroup = newMeshGroup;
		}
	}
	if((copyFlags &CopyFlags::CopyAnimationsBit) != CopyFlags::None)
	{
		for(auto &anim : mdl->m_animations)
			anim = Animation::Create(*anim,Animation::ShareMode::None);
	}
	if((copyFlags &CopyFlags::CopyVertexAnimationsBit) != CopyFlags::None)
	{
		for(auto &vertexAnim : mdl->m_vertexAnimations)
		{
			vertexAnim = VertexAnimation::Create(*vertexAnim);
			for(auto &meshAnim : vertexAnim->GetMeshAnimations())
			{
				auto *mesh = meshAnim->GetMesh();
				auto *subMesh = meshAnim->GetSubMesh();
				if(mesh == nullptr || subMesh == nullptr)
					continue;
				auto itMesh = oldMeshToNewMesh.find(mesh);
				auto itSubMesh = oldSubMeshToNewSubMesh.find(subMesh);
				if(itMesh == oldMeshToNewMesh.end() || itSubMesh == oldSubMeshToNewSubMesh.end())
					continue;
				meshAnim->SetMesh(*itMesh->second,*itSubMesh->second);
			}
		}
	}
	if((copyFlags &CopyFlags::CopyCollisionMeshesBit) != CopyFlags::None)
	{
		for(auto &colMesh : mdl->m_collisionMeshes)
		{
			colMesh = CollisionMesh::Create(*colMesh);
			// TODO: Update shape?
		}
	}
	if((copyFlags &CopyFlags::CopyFlexAnimationsBit) != CopyFlags::None)
	{
		for(auto &flexAnim : mdl->m_flexAnimations)
			flexAnim = std::make_shared<FlexAnimation>(*flexAnim);
	}
	// TODO: Copy collision mesh soft body sub mesh reference
	static_assert(sizeof(Model) == 992,"Update this function when making changes to this class!");
	return mdl;
}

bool Model::FindSubMeshIndex(const ModelMeshGroup *optMeshGroup,const ModelMesh *optMesh,const ModelSubMesh *optSubMesh,uint32_t &outGroupIdx,uint32_t &outMeshIdx,uint32_t &outSubMeshIdx) const
{
	auto &meshGroups = GetMeshGroups();
	uint32_t mgStart = 0;
	uint32_t mgEnd = meshGroups.size();
	if(optMeshGroup)
	{
		auto it = std::find_if(meshGroups.begin(),meshGroups.end(),[optMeshGroup](const std::shared_ptr<ModelMeshGroup> &mmg) {return mmg.get() == optMeshGroup;});
		if(it == meshGroups.end())
			return false;
		mgStart = (it -meshGroups.begin());
		mgEnd = mgStart +1;
		if(!optMesh && !optSubMesh)
		{
			outGroupIdx = mgStart;
			return true;
		}
	}

	for(auto i=mgStart;i<mgEnd;++i)
	{
		auto &mg = meshGroups[i];
		auto &meshes = mg->GetMeshes();
		uint32_t meshStart = 0;
		uint32_t meshEnd = meshes.size();
		if(optMesh)
		{
			auto it = std::find_if(meshes.begin(),meshes.end(),[optMesh](const std::shared_ptr<ModelMesh> &mesh) {return mesh.get() == optMesh;});
			if(it == meshes.end())
				continue;
			meshStart = (it -meshes.begin());
			meshEnd = meshStart +1;
			if(!optSubMesh)
			{
				outGroupIdx = i;
				outMeshIdx = meshStart;
				return true;
			}
		}
		if(optSubMesh == nullptr)
			continue;
		for(auto j=meshStart;j<meshEnd;++j)
		{
			auto &mesh = meshes[j];
			auto &subMeshes = mesh->GetSubMeshes();
			auto it = std::find_if(subMeshes.begin(),subMeshes.end(),[optSubMesh](const std::shared_ptr<ModelSubMesh> &subMesh) {return subMesh.get() == optSubMesh;});
			if(it == subMeshes.end())
				continue;
			outGroupIdx = i;
			outMeshIdx = j;
			outSubMeshIdx = (it -subMeshes.begin());
			return true;
		}
	}
	return false;
}

static constexpr auto PMDL_IDENTIFIER = "PMDL";
static constexpr udm::Version PMDL_VERSION = 1;

bool Model::Save(Game &game,udm::AssetData &outData,std::string &outErr)
{
	return false;
	// WIP
#if 0
	outData.SetAssetType(PMDL_IDENTIFIER);
	outData.SetAssetVersion(PMDL_VERSION);
	auto udm = *outData;

	udm["materialPaths"] = GetTexturePaths();
	udm["includeModels"] = GetMetaInfo().includes;
	udm["eyeOffset"] = GetEyeOffset();

	Vector3 min,max;
	GetRenderBounds(min,max);
	udm["render.bounds.min"] = min;
	udm["render.bounds.max"] = max;

	auto flags = GetMetaInfo().flags;
	auto writeFlag = [](auto udm,auto flag,const std::string &name,auto flags) {
		if(umath::is_flag_set(flags,flag) == false)
			return;
		udm["flags"][name] = true;
	};
	auto writeModelFlag = [&udm,flags,&writeFlag](Model::Flags flag,const std::string &name) {writeFlag(udm,flag,name,flags);};
	writeModelFlag(Model::Flags::Static,"static");
	writeModelFlag(Model::Flags::Inanimate,"inanimate");
	writeModelFlag(Model::Flags::DontPrecacheTextureGroups,"dontPrecacheSkins");
	static_assert(umath::to_integral(Model::Flags::Count) == 8,"Update this list when new flags have been added!");

	auto isStatic = umath::is_flag_set(flags,Model::Flags::Static);
	if(!isStatic)
	{
		auto &skeleton = GetSkeleton();
		auto &ref = GetReference();
		auto &bones = skeleton.GetBones();
		auto udmSkeleton = udm.AddArray("skeleton",bones.size());
		for(auto i=decltype(bones.size()){0u};i<bones.size();++i)
		{
			auto &bone = bones[i];
			auto udmBone = udmSkeleton[i];
			std::vector<uint32_t> childIds {};
			childIds.reserve(bone->children.size());
			for(auto &pair : bone->children)
				childIds.push_back(pair.first);
			umath::ScaledTransform transform;
			ref.GetBonePose(bone->ID,transform);
			udmBone["name"] = bone->name;
			udmBone["children"] = childIds;
			udmBone["pose"] = transform;
		}

		auto &attachments = GetAttachments();
		auto udmAttachments = udm.AddArray("attachments",attachments.size());
		for(auto i=decltype(bones.size()){0u};i<bones.size();++i)
		{
			auto &att = attachments[i];
			auto udmAtt = udmAttachments[i];
			umath::Transform transform {att.offset,uquat::create(att.angles)};
			udmAtt["name"] = att.name;
			udmAtt["bone"] = att.bone;
			udmAtt["pose"] = transform;
		}
		
		auto &objAttachments = GetObjectAttachments();
		auto udmObjAttachments = udm.AddArray("objectAttachments",objAttachments.size());
		for(auto i=decltype(objAttachments.size()){0u};i<objAttachments.size();++i)
		{
			auto &objAtt = objAttachments[i];
			auto udmObjAtt = udmObjAttachments[i];
			udmObjAtt["name"] = objAtt.name;
			udmObjAtt["attachment"] = LookupAttachment(objAtt.attachment);
			udmObjAtt["type"] = magic_enum::enum_name(objAtt.type);
			udmObjAtt["keyValues"] = objAtt.keyValues;
		}

		auto &hitboxes = GetHitboxes();
		auto udmHitboxes = udm.AddArray("hitboxes",hitboxes.size());
		uint32_t hbIdx = 0;
		for(auto &pair : hitboxes)
		{
			auto &hb = pair.second;
			auto udmHb = udmHitboxes[hbIdx++];
			udmHb["hitGroup"] = magic_enum::enum_name(hb.group);
			udmHb["bounds.min"] = hb.min;
			udmHb["bounds.max"] = hb.max;
			udmHb["bone"] = pair.first;
		}
	}
	
	auto &colMeshes = GetCollisionMeshes();
	auto udmColMeshes = udm.AddArray("collisionMeshes",colMeshes.size());
	auto &surfaceMaterials = game.GetSurfaceMaterials();
	for(auto i=decltype(colMeshes.size()){0u};i<colMeshes.size();++i)
	{
		auto &colMesh = colMeshes[i];
		auto udmColMesh = udmColMeshes[i];
		umath::Transform pose {};
		pose.SetOrigin(colMesh->GetOrigin());
		udmColMesh["bone"] = colMesh->GetBoneParent();
		udmColMesh["pose"] = pose;
		udmColMesh["surfaceMaterial"] = surfaceMaterials[colMesh->GetSurfaceMaterial()].GetIdentifier();

		Vector3 min,max;
		colMesh->GetAABB(&min,&max);
		udmColMesh["bounds.min"] = min;
		udmColMesh["bounds.max"] = max;

		udmColMesh["vertices"] = udm::compress_lz4_blob(colMesh->GetVertices());
		udmColMesh["triangles"] = udm::compress_lz4_blob(colMesh->GetTriangles());

		udmColMesh["volume"] = colMesh->GetVolume();
		udmColMesh["centerOfMass"] = colMesh->GetCenterOfMass();

		udmColMesh["flags.convex"] = colMesh->IsConvex();

		// Soft-body
		auto softBody = colMesh->IsSoftBody();
		auto *sbInfo = colMesh->GetSoftBodyInfo();
		auto *sbMesh = colMesh->GetSoftBodyMesh();
		auto *sbTriangles = colMesh->GetSoftBodyTriangles();
		auto *sbAnchors = colMesh->GetSoftBodyAnchors();
		softBody = (softBody && sbInfo != nullptr && sbMesh != nullptr && sbTriangles != nullptr && sbAnchors != nullptr) ? true : false;
		auto meshGroupId = std::numeric_limits<uint32_t>::max();
		auto meshId = std::numeric_limits<uint32_t>::max();
		auto subMeshId = std::numeric_limits<uint32_t>::max();
		ModelSubMesh *subMesh = nullptr;
		auto foundSoftBodyMesh = false;
		if(softBody)
		{
			for(auto i=decltype(meshGroups.size()){0};i<meshGroups.size();++i)
			{
				auto &group = meshGroups.at(i);
				auto &meshes = group->GetMeshes();
				for(auto j=decltype(meshes.size()){0};j<meshes.size();++j)
				{
					auto &mesh = meshes.at(j);
					auto &subMeshes = mesh->GetSubMeshes();
					for(auto k=decltype(subMeshes.size()){0};k<subMeshes.size();++k)
					{
						subMesh = subMeshes.at(k).get();
						meshGroupId = i;
						meshId = j;
						subMeshId = k;
						foundSoftBodyMesh = true;
						goto endLoop;
					}
				}
			}
		}
	endLoop:

		if(foundSoftBodyMesh == false)
			softBody = false;

		if(softBody)
		{
			auto udmSoftBody = udmColMesh["softBody"];
			udmSoftBody["meshGroup"] = meshGroupId;
			udmSoftBody["mesh"] = meshId;
			udmSoftBody["subMesh"] = subMeshId;

			auto udmSettings = udmSoftBody["settings"];

#if 0
	struct MaterialStiffnessCoefficient
	{
		MaterialStiffnessCoefficient()=default;
		MaterialStiffnessCoefficient(float a,float l,float v)
			: MaterialStiffnessCoefficient()
		{
			angular = a;
			linear = l;
			volume = v;
		}
		float angular = 1.f;
		float linear = 1.f;
		float volume = 1.f;
	};
	float poseMatchingCoefficient = 0.5f;
	float anchorsHardness = 0.6999f;
	float dragCoefficient = 0.f;
	float rigidContactsHardness = 1.f;
	float softContactsHardness = 1.f;
	float liftCoefficient = 0.f;
	float kineticContactsHardness = 0.1f;
	float dynamicFrictionCoefficient = 0.2f;
	float dampingCoefficient = 0.f;
	float volumeConversationCoefficient = 0.f;
	float softVsRigidImpulseSplitK = 0.5f;
	float softVsRigidImpulseSplitR = 0.5f;
	float softVsRigidImpulseSplitS = 0.5f;
	float softVsKineticHardness = 1.f;
	float softVsRigidHardness = 0.1f;
	float softVsSoftHardness = 0.5f;
	float pressureCoefficient = 0.f;
	float velocitiesCorrectionFactor = 1.f;

	float bendingConstraintsDistance = 0.2f;
	uint32_t clusterCount = 0u;
	uint32_t maxClusterIterations = 8192u;
	std::unordered_map<uint32_t,MaterialStiffnessCoefficient> materialStiffnessCoefficient;
#endif
				/*f->Write(sbInfo,sizeof(float) *19 +sizeof(uint32_t) *2);
				f->Write<uint32_t>(sbInfo->materialStiffnessCoefficient.size());
				for(auto &pair : sbInfo->materialStiffnessCoefficient)
				{
					f->Write<uint32_t>(pair.first);
					f->Write<float>(pair.second.linear);
					f->Write<float>(pair.second.angular);
					f->Write<float>(pair.second.volume);
				}

				f->Write<uint32_t>(sbTriangles->size());
				for(auto idx : *sbTriangles)
					f->Write<uint32_t>(idx);

				f->Write<uint32_t>(sbAnchors->size());
				for(auto &anchor : *sbAnchors)
					f->Write<CollisionMesh::SoftBodyAnchor>(anchor);*/
		}
	}

	// Joints
	auto &joints = GetJoints();
	if(!joints.empty())
	{
		auto udmJoints = udm.AddArray("joints",joints.size());
		uint32_t jointIdx = 0;
		for(auto &joint : joints)
		{
			auto udmJoint = udmJoints[jointIdx++];
			udmJoint["type"] = magic_enum::enum_name(joint.type);
			udmJoint["parentBone"] = joint.parent;
			udmJoint["childBone"] = joint.child;
			udmJoint["enableCollisions"] = joint.collide;
			udmJoint["args"] = joint.args;
		}
	}

	if(!isStatic)
	{
		auto &animations = GetAnimations();
		if(!animations.empty())
		{
			auto udmAnimations = udm.AddArray("animations",animations.size());
			uint32_t animIdx = 0;
			for(auto &anim : animations)
			{
				auto udmAnim = udmAnimations[animIdx++];
				if(anim->Save(udm::AssetData{*udmAnim},outErr) == false)
					return false;
			}
		}

		auto &blendControllers = GetBlendControllers();
		if(!blendControllers.empty())
		{
			auto udmBlendControllers = udm.AddArray("blendControllers",blendControllers.size());
			uint32_t bcIdx = 0;
			for(auto &bc : blendControllers)
			{
				auto udmBc = udmBlendControllers[bcIdx++];
				udmBc["name"] = bc.name;
				udmBc["min"] = bc.min;
				udmBc["max"] = bc.max;
				udmBc["loop"] = bc.loop;
			}
		}

		auto &ikControllers = GetIKControllers();
		if(!ikControllers.empty())
		{
			auto udmIkControllers = udm.AddArray("ikControllers",ikControllers.size());
			uint32_t ikControllerIdx = 0;
			for(auto &ikc : ikControllers)
			{
				auto udmIkController = udmIkControllers[ikControllerIdx++];
				udmIkController["effectorName"] = ikc->GetEffectorName();
				udmIkController["type"] = ikc->GetType();
				udmIkController["chainLength"] = ikc->GetChainLength();
				udmIkController["method"] = magic_enum::enum_name(ikc->GetMethod());
				udmIkController["keyValues"] = ikc->GetKeyValues();
			}
		}

		auto &morphAnims = GetVertexAnimations();
		if(!morphAnims.empty())
		{
			auto udmMorphAnims = udm.AddArray("morphTargetAnimations",morphAnims.size());
			uint32_t morphAnimIdx = 0;
			for(auto &va : morphAnims)
			{
				auto udmMa = udmMorphAnims[morphAnimIdx++];
				udmMa["name"] = va->GetName();
				
				auto &meshAnims = va->GetMeshAnimations();
				auto udmMeshAnims = udmMa.AddArray("meshAnimations",meshAnims.size());
				uint32_t meshAnimIdx = 0;
				for(auto &ma : meshAnims)
				{
					auto *mesh = ma->GetMesh();
					auto *subMesh = ma->GetSubMesh();
					if(mesh == nullptr || subMesh == nullptr)
						continue;
					auto udmMa = udmMeshAnims[meshAnimIdx++];
					uint32_t groupIdx,meshIdx,subMeshIdx;
					if(FindSubMeshIndex(nullptr,mesh,subMesh,groupIdx,meshIdx,subMeshIdx) == false)
						continue;
					udmMa["meshGroup"] = groupIdx;
					udmMa["mesh"] = meshIdx;
					udmMa["subMesh"] = subMeshIdx;

					auto &frames = ma->GetFrames();
					auto udmFrames = udmMa.AddArray("frames",frames.size());
					uint32_t frameIdx = 0;
					for(auto &frame : frames)
					{
						auto udmFrame = udmFrames[frameIdx++];
						writeFlag(udmFrame,MeshVertexFrame::Flags::HasDeltaValues,"hasDeltaValues",frame->GetFlags());
						static_assert(umath::to_integral(MeshVertexFrame::Flags::Count) == 2,"Update this list when new flags have been added!");

						udmFrame["positions"] = udm::compress_lz4_blob(frame->GetVertices());
						if(umath::is_flag_set(frame->GetFlags(),MeshVertexFrame::Flags::HasNormals))
							udmFrame["normals"] = udm::compress_lz4_blob(frame->GetNormals());
						//frame->GetDeltaValue();

						// TODO??
						/*
						struct Attribute
						{
							Attribute(const std::string &name,const std::vector<std::array<uint16_t,4>> &vertexData)
								: name{name},vertexData{vertexData}
							{}
							std::string name;
							const std::vector<std::array<uint16_t,4>> &vertexData;
						};
						std::vector<Attribute> attributes {};
						attributes.push_back({"position",frame->GetVertices()});
						if(umath::is_flag_set(flags,MeshVertexFrame::Flags::HasNormals))
							attributes.push_back({"normal",frame->GetNormals()});
						std::set<uint16_t> usedVertIndices {};
						for(auto &attr : attributes)
						{
							auto vertIdx = 0u;
							auto &vdata = attr.vertexData.at(vertIdx);
							for(auto &vdata : attr.vertexData)
							{
								auto itUsed = std::find_if(vdata.begin(),vdata.end(),[](const uint16_t &v) {return v != 0;});
								if(itUsed != vdata.end())
									usedVertIndices.insert(vertIdx);
								++vertIdx;
							}
						}

						f->Write<uint16_t>(usedVertIndices.size());
						for(auto idx : usedVertIndices)
							f->Write<uint16_t>(idx);
						f->Write<uint16_t>(attributes.size());
						for(auto &attr : attributes)
						{
							f->WriteString(attr.name);
							for(auto idx : usedVertIndices)
								f->Write<std::array<uint16_t,4>>(attr.vertexData.at(idx));
						}
						write_offset(f,offsetToEndOfFrameOffset);*/
					}
				}
			}

			auto &flexes = GetFlexes();
			auto udmFlexes = udm.AddArray("flexes",flexes.size());
			uint32_t flexIdx = 0;
			for(auto &flex : flexes)
			{
				auto udmFlex = udmFlexes[flexIdx++];
				udmFlex["name"] = flex.GetName();

				auto *va = flex.GetVertexAnimation();
				if(va != nullptr)
				{
					auto &vertAnims = GetVertexAnimations();
					auto itVa = std::find(vertAnims.begin(),vertAnims.end(),va->shared_from_this());
					if(itVa != vertAnims.end())
						udmFlex["morphTargetAnimation"] = (itVa -vertAnims.begin());
				}
				if(!udmFlex["morphTargetAnimation"])
					udmFlex.Add("morphTargetAnimation",udm::Type::Nil);
				udmFlex["frame"] = flex.GetFrameIndex();

				auto &ops = flex.GetOperations();
				auto udmOps = udmFlex.AddArray("operations",ops.size());
				uint32_t opIdx = 0u;
				for(auto &op : ops)
				{
					auto udmOp = udmOps[opIdx++];
					udmOp["type"] = magic_enum::enum_name(op.type);
					udmOp["value"] = op.d.value;
				}
			}

			auto &flexControllers = GetFlexControllers();
			auto udmFlexControllers = udm.AddArray("flexControllers",flexControllers.size());
			uint32_t flexCIdx = 0;
			for(auto &flexC : flexControllers)
			{
				auto udmFlexC = udmFlexControllers[flexCIdx++];
				udmFlexC["name"] = flexC.name;
				udmFlexC["min"] = flexC.min;
				udmFlexC["max"] = flexC.max;
			}

			auto &eyeballs = GetEyeballs();
			auto udmEyeballs = udm.AddArray("eyeballs",eyeballs.size());
			uint32_t eyeballIdx = 0;
			for(auto &eyeball : eyeballs)
			{
				auto udmEyeball = udmFlexControllers[eyeballIdx++];
				udmEyeball["name"] = eyeball.name;
				udmEyeball["bone"] = eyeball.boneIndex;
				udmEyeball["origin"] = eyeball.origin;
				udmEyeball["zOffset"] = eyeball.zOffset;
				udmEyeball["radius"] = eyeball.radius;
				udmEyeball["up"] = eyeball.up;
				udmEyeball["forward"] = eyeball.forward;
				udmEyeball["maxDilationFactor"] = eyeball.maxDilationFactor;

				udmEyeball["iris.material"] = eyeball.irisMaterialIndex;
				udmEyeball["iris.uvRadius"] = eyeball.irisUvRadius;
				udmEyeball["iris.scale"] = eyeball.irisScale;

				udmEyeball["eyelids.upper.raised.flex"] = eyeball.upperFlexDesc[0];
				udmEyeball["eyelids.upper.raised.targetAngle"] = umath::rad_to_deg(eyeball.upperTarget[0]);

				udmEyeball["eyelids.upper.neutral.flex"] = eyeball.upperFlexDesc[1];
				udmEyeball["eyelids.upper.neutral.targetAngle"] = umath::rad_to_deg(eyeball.upperTarget[1]);

				udmEyeball["eyelids.upper.lowered.flex"] = eyeball.upperFlexDesc[2];
				udmEyeball["eyelids.upper.lowered.targetAngle"] = umath::rad_to_deg(eyeball.upperTarget[2]);

				udmEyeball["eyelids.lower.raised.flex"] = eyeball.lowerFlexDesc[0];
				udmEyeball["eyelids.lower.raised.targetAngle"] = umath::rad_to_deg(eyeball.lowerTarget[0]);

				udmEyeball["eyelids.lower.neutral.flex"] = eyeball.lowerFlexDesc[1];
				udmEyeball["eyelids.lower.neutral.targetAngle"] = umath::rad_to_deg(eyeball.lowerTarget[1]);

				udmEyeball["eyelids.lower.lowered.flex"] = eyeball.lowerFlexDesc[2];
				udmEyeball["eyelids.lower.lowered.targetAngle"] = umath::rad_to_deg(eyeball.lowerTarget[2]);

				// TODO
				udmEyeball["upperFlexDesc"] = eyeball.upperFlexDesc; // ??
				udmEyeball["lowerFlexDesc"] = eyeball.lowerFlexDesc; // ??
				udmEyeball["upperTarget"] = eyeball.upperTarget; // ??  // Angle in radians of raised, neutral, and lowered lid positions
				udmEyeball["lowerTarget"] = eyeball.lowerTarget; // ??

				udmEyeball["upperLidFlexDesc"] = eyeball.upperLidFlexDesc; // Index of flex desc that actual lid flexes look to
				udmEyeball["lowerLidFlexDesc"] = eyeball.lowerLidFlexDesc; // ??
			}
#if 0
struct DLLNETWORK Eyeball
{
	std::array<int32_t,3> upperFlexDesc = {};
	std::array<int32_t,3> lowerFlexDesc = {};
	std::array<float,3> upperTarget; // Angle in radians of raised, neutral, and lowered lid positions
	std::array<float,3> lowerTarget;

	int32_t upperLidFlexDesc = -1; // Index of flex desc that actual lid flexes look to
	int32_t lowerLidFlexDesc = -1;
};
#endif
			
			auto &phonemeMap = GetPhonemeMap();
			auto udmPhonemes = udm.AddArray("phonemes",eyeballs.size());
			for(auto &pairPhoneme : phonemeMap.phonemes)
				udmPhonemes[pairPhoneme.first] = pairPhoneme.second.flexControllers;
			
			auto &flexAnims = GetFlexAnimations();
			auto &flexAnimNames = GetFlexAnimationNames();
			auto udmFlexAnims = udm.AddArray("flexAnimation",flexAnims.size());
			for(auto i=decltype(flexAnims.size()){0u};i<flexAnims.size();++i)
			{
				auto &flexAnim = flexAnims[i];
				auto udmFlexAnim = udmFlexAnims.Add(flexAnimNames[i]);
				if(flexAnim->Save(udm::AssetData{*udmFlexAnim},outErr) == false)
					return false;
			}
		}
	}


	auto &meshGroups = GetMeshGroups();
	auto udmMeshGroups = udm.AddArray("meshGroups",meshGroups.size());
	uint32_t meshGroupIdx = 0;
	for(auto &meshGroup : meshGroups)
	{
		auto udmMeshGroup = udmMeshGroups[meshGroupIdx++];
		auto &meshes = meshGroup->GetMeshes();
		auto udmMeshes = udmMeshGroup.AddArray("meshes",meshes.size());
		uint32_t meshIdx = 0;
		for(auto &mesh : meshes)
		{
			auto udmMesh = udmMeshes[meshIdx++];
			udmMesh["referenceId"] = mesh->GetReferenceId();
			auto &subMeshes = mesh->GetSubMeshes();
			auto udmSubMeshes = udmMesh.AddArray("subMeshes",subMeshes.size());
			uint32_t subMeshIdx = 0;
			for(auto &subMesh : subMeshes)
			{
				auto udmSubMesh = udmSubMeshes[subMeshIdx++];
				udmSubMesh["referenceId"] = subMesh->GetReferenceId();
				udmSubMesh["pose"] = subMesh->GetPose();
				udmSubMesh["geometryType"] = magic_enum::enum_name(subMesh->GetGeometryType());
				udmSubMesh["vertexData"] = udm::compress_lz4_blob(subMesh->GetVertices());
				udmSubMesh["indexData"] = udm::compress_lz4_blob(subMesh->GetTriangles());

				auto udmUvSets = udmSubMesh["uvSets"];
				for(auto &pair : subMesh->GetUVSets())
					udmUvSets[pair.first] = udm::compress_lz4_blob(pair.second);

				auto &vertexWeights = subMesh->GetVertexWeights();
				if(!vertexWeights.empty())
					udmSubMesh["vertexWeights"] = udm::compress_lz4_blob(vertexWeights);
				
				auto &extBoneWeights = subMesh->GetExtendedVertexWeights();
				if(!extBoneWeights.empty())
					udmSubMesh["extendedVertexWeights"] = udm::compress_lz4_blob(extBoneWeights);

				auto &alphas = subMesh->GetAlphas();
				udmSubMesh["alphaCount"] = subMesh->GetAlphaCount();
				if(!alphas.empty())
					udmSubMesh["alphas"] = udm::compress_lz4_blob(alphas);
			}
		}
	}
#endif
}

bool Model::SaveLegacy(Game *game,const std::string &name,const std::string &rootPath) const
{
	auto fname = pragma::asset::get_normalized_path(name,pragma::asset::Type::Model);
	fname = rootPath +"models\\" +fname;
	FileManager::CreatePath(ufile::get_path_from_filename(fname).c_str());
	auto f = FileManager::OpenFile<VFilePtrReal>(fname.c_str(),"wb");
	if(f == nullptr)
		return false;
	auto &mdl = const_cast<Model&>(*this);
	auto &skeleton = mdl.GetSkeleton();
	auto &bones = skeleton.GetBones();
	auto &refPose = mdl.GetReference();
	auto &rootBones = skeleton.GetRootBones();
	auto &attachments = mdl.GetAttachments();
	auto &objectAttachments = mdl.GetObjectAttachments();
	auto &hitboxes = mdl.GetHitboxes();
	auto &meta = mdl.GetMetaInfo();
	auto flags = meta.flags;
	auto bStatic = umath::is_flag_set(flags,Model::Flags::Static);

	auto &texturePaths = meta.texturePaths;
	assert(texturePaths.size() <= std::numeric_limits<uint8_t>::max());

	f->Write("WMD",3);
	f->Write<uint16_t>(WMD_VERSION);
	f->Write<Flags>(flags);
	f->Write<Vector3>(mdl.GetEyeOffset());

	auto offIndex = f->Tell();
	f->Write<uint64_t>(0); // Offset to model data
	f->Write<uint64_t>(0); // Offset to model meshes
	f->Write<uint64_t>(0); // Offset to LOD data

	// Version 0x0004
	f->Write<uint64_t>(0); // Offset to bodygroups
	//

	// Version 38
	f->Write<uint64_t>(0); // Offset to joints
	//

	f->Write<uint64_t>(0); // Offset to collision mesh (0 if there is none)
	//auto bStatic = umath::is_flag_set(flags,Model::Flags::Static);
	if(!bStatic)
	{
		f->Write<uint64_t>(0); // Offset to bones
		f->Write<uint64_t>(0); // Offset to animations
		// Version 0x0015
		f->Write<uint64_t>(0ull); // Offset to vertex animations
		f->Write<uint64_t>(0ull); // Offset to flex controllers
		f->Write<uint64_t>(0ull); // Offset to flexes
		f->Write<uint64_t>(0ull); // Offset to phoneme map
		//

		// Version 0x0016
		f->Write<uint64_t>(0ull); // INDEX_OFFSET_IK_CONTROLLERS
		//

		// Version 28
		f->Write<uint64_t>(0ull); // INDEX_OFFSET_EYEBALLS
		//

		// Version 37
		f->Write<uint64_t>(0ull); // INDEX_OFFSET_FLEX_ANIMATIONS
		//
	}

	write_offset(f,offIndex +INDEX_OFFSET_MODEL_DATA *INDEX_OFFSET_INDEX_SIZE); // Model Data
	f->Write<uint8_t>(static_cast<uint8_t>(texturePaths.size()));
	for(auto &path : texturePaths)
		f->WriteString(path);

	if(!bStatic)
	{
		std::function<void(VFilePtrReal&,Bone&)> fWriteChildBones;
		fWriteChildBones = [&fWriteChildBones](VFilePtrReal &f,Bone &bone) {
			auto &children = bone.children;
			f->Write<uint32_t>(static_cast<uint32_t>(children.size()));
			for(auto &pair : children)
			{
				f->Write<uint32_t>(pair.first);
				fWriteChildBones(f,*pair.second);
			}
		};

		write_offset(f,offIndex +INDEX_OFFSET_BONES *INDEX_OFFSET_INDEX_SIZE); // Bones
		f->Write<uint32_t>(static_cast<uint32_t>(bones.size()));
		for(auto &bone : bones)
			f->WriteString(bone->name);
		for(auto &bone : bones)
		{
			auto *pos = refPose.GetBonePosition(bone->ID);
			auto *rot = refPose.GetBoneOrientation(bone->ID);
			f->Write<Quat>((rot != nullptr) ? *rot : uquat::identity());
			f->Write<Vector3>((pos != nullptr) ? *pos : Vector3{});
		}

		f->Write<uint32_t>(static_cast<uint32_t>(rootBones.size()));
		for(auto &pair : rootBones)
		{
			f->Write<uint32_t>(pair.first);
			auto &bone = *pair.second;

			fWriteChildBones(f,bone);
		}

		f->Write<uint32_t>(static_cast<uint32_t>(attachments.size()));
		for(auto &att : attachments)
		{
			f->WriteString(att.name);
			f->Write<uint32_t>(att.bone);
			f->Write<Vector3>(att.offset);
			f->Write<EulerAngles>(att.angles);
		}

		f->Write<uint32_t>(static_cast<uint32_t>(objectAttachments.size()));
		for(auto &objAtt : objectAttachments)
		{
			f->Write<ObjectAttachment::Type>(objAtt.type);
			f->WriteString(objAtt.name);
			f->WriteString(objAtt.attachment);
			f->Write<uint32_t>(objAtt.keyValues.size());
			for(auto &pair : objAtt.keyValues)
			{
				f->WriteString(pair.first);
				f->WriteString(pair.second);
			}
		}

		f->Write<uint32_t>(static_cast<uint32_t>(hitboxes.size()));
		for(auto &pair : hitboxes)
		{
			auto &hb = pair.second;
			f->Write<uint32_t>(pair.first);
			f->Write<uint32_t>(umath::to_integral(hb.group));
			f->Write<Vector3>(hb.min);
			f->Write<Vector3>(hb.max);
		}
	}

	auto &skins = mdl.GetTextureGroups();
	auto &textures = mdl.GetTextures();
	auto numBaseTextures = (skins.empty() == false) ? skins.front().textures.size() : 0;
	f->Write<uint16_t>(static_cast<uint16_t>(numBaseTextures));
	f->Write<uint16_t>(static_cast<uint16_t>(textures.size()));
	for(auto &tex : textures)
		f->WriteString(tex);
	f->Write<uint16_t>(static_cast<uint16_t>(skins.size()));
	for(auto &skin : skins)
	{
		assert(skin.textures.size() == skins.front().textures.size());
		if(skin.textures.size() != skins.front().textures.size())
		{
			f = nullptr;
			FileManager::RemoveFile(fname.c_str());
			throw std::logic_error("All skins have to contain same number of textures.");
		}
		for(auto &texId : skin.textures)
			f->Write<uint16_t>(static_cast<uint16_t>(texId));
	}

	Vector3 min,max;
	mdl.GetRenderBounds(min,max);
	f->Write<Vector3>(min);
	f->Write<Vector3>(max);

	auto &meshGroups = mdl.GetMeshGroups();
	write_offset(f,offIndex +INDEX_OFFSET_MODEL_MESHES *INDEX_OFFSET_INDEX_SIZE); // Meshes
	f->Write<uint32_t>(static_cast<uint32_t>(meshGroups.size()));
	for(auto &group : meshGroups)
	{
		f->WriteString(group->GetName());
		auto &meshes = group->GetMeshes();
		f->Write<uint32_t>(static_cast<uint32_t>(meshes.size()));
		for(auto &mesh : meshes)
		{
			f->Write<uint32_t>(mesh->GetReferenceId());

			auto &subMeshes = mesh->GetSubMeshes();
			f->Write<uint32_t>(static_cast<uint32_t>(subMeshes.size()));
			for(auto &subMesh : subMeshes)
			{
				// Version 26
				f->Write<umath::ScaledTransform>(subMesh->GetPose());
				//

				f->Write<uint16_t>(static_cast<uint16_t>(subMesh->GetSkinTextureIndex()));

				// Version 27
				f->Write<ModelSubMesh::GeometryType>(subMesh->GetGeometryType());
				//
				f->Write<uint32_t>(subMesh->GetReferenceId());

				auto &verts = subMesh->GetVertices();
				auto numVerts = verts.size();
				f->Write<uint64_t>(numVerts);
				for(auto &v : verts)
				{
					f->Write<Vector3>(v.position);
					f->Write<Vector3>(v.normal);
				}

				auto &uvSets = subMesh->GetUVSets();
				// Version 30
				auto numUvSets = umath::min(uvSets.size() +1,static_cast<size_t>(std::numeric_limits<uint8_t>::max()));
				f->Write<uint8_t>(numUvSets);
				f->WriteString(std::string{"base"});
				for(auto &v : verts)
					f->Write<Vector2>(v.uv);
				auto uvSetIdx = 1;
				for(auto &pair : uvSets)
				{
					f->WriteString(pair.first);
					auto &uvSet = pair.second;
					for(auto i=decltype(numVerts){0u};i<numVerts;++i)
					{
						auto uv = (i < uvSet.size()) ? uvSet.at(i) : Vector2{};
						f->Write<Vector2>(uv);
					}
					if(++uvSetIdx == numUvSets)
						break;
				}
				//

				auto &boneWeights = subMesh->GetVertexWeights();
				f->Write<uint64_t>(boneWeights.size());
				static_assert(sizeof(decltype(boneWeights.front())) == sizeof(Vector4) *2);
				f->Write(boneWeights.data(),boneWeights.size() *sizeof(decltype(boneWeights.front())));

				// Version 27
				auto &extBoneWeights = subMesh->GetExtendedVertexWeights();
				f->Write<uint64_t>(extBoneWeights.size());
				static_assert(sizeof(decltype(extBoneWeights.front())) == sizeof(Vector4) *2);
				f->Write(extBoneWeights.data(),extBoneWeights.size() *sizeof(decltype(extBoneWeights.front())));
				//

				// Version 30
				auto numAlphas = subMesh->GetAlphaCount();
				f->Write<uint8_t>(numAlphas);
				if(numAlphas > 0)
				{
					auto &alphas = subMesh->GetAlphas();
					for(auto i=decltype(numVerts){0u};i<numVerts;++i)
					{
						auto a = (i < alphas.size()) ? alphas.at(i) : Vector2{};
						f->Write<float>(a.x);
						if(numAlphas > 1)
							f->Write<float>(a.y);
					}
				}
				//

				auto &indices = subMesh->GetTriangles();
				f->Write<uint32_t>(indices.size());
				static_assert(std::is_same_v<std::remove_reference_t<decltype(indices.front())>,uint16_t>);
				f->Write(indices.data(),indices.size() *sizeof(decltype(indices.front())));
			}
		}
	}

	auto &baseMeshes = mdl.GetBaseMeshes();

	// Version 0x0004
	f->Write<uint16_t>(static_cast<uint16_t>(baseMeshes.size()));
	for(auto &meshId : baseMeshes)
		f->Write<uint32_t>(meshId);
	//
	
	write_offset(f,offIndex +INDEX_OFFSET_LOD_DATA *INDEX_OFFSET_INDEX_SIZE); // LOD data
	// Version 0x0004
	auto numLods = mdl.GetLODCount();

	auto &lodInfos = GetLODs();
	f->Write<uint8_t>(static_cast<uint8_t>(lodInfos.size()));
	for(auto &lodInfo : lodInfos)
	{
		f->Write<uint8_t>(static_cast<uint8_t>(lodInfo.lod));
		f->Write<float>(lodInfo.distance);
		f->Write<uint8_t>(static_cast<uint8_t>(lodInfo.meshReplacements.size()));
		for(auto &pair : lodInfo.meshReplacements)
		{
			f->Write<uint32_t>(pair.first);
			f->Write<uint32_t>(pair.second);
		}
	}
	//

	// Version 0x0004
	write_offset(f,offIndex +INDEX_OFFSET_BODYGROUPS *INDEX_OFFSET_INDEX_SIZE); // Bodygroup Data
	auto &bodyGroups = mdl.GetBodyGroups();
	f->Write<uint16_t>(static_cast<uint16_t>(bodyGroups.size()));
	for(auto &bg : bodyGroups)
	{
		f->WriteString(bg.name);
		f->Write<uint8_t>(static_cast<uint8_t>(bg.meshGroups.size()));
		for(auto &group : bg.meshGroups)
			f->Write<uint32_t>(group);
	}
	//

	// Version 38
	write_offset(f,offIndex +INDEX_OFFSET_JOINTS *INDEX_OFFSET_INDEX_SIZE);
	auto &joints = mdl.GetJoints();
	f->Write<uint32_t>(joints.size());
	for(auto &joint : joints)
	{
		f->Write<JointType>(joint.type);
		f->Write<BoneId>(joint.child);
		f->Write<BoneId>(joint.parent);
		f->Write<bool>(joint.collide);
		f->Write<uint8_t>(joint.args.size());
		for(auto &pair : joint.args)
		{
			f->WriteString(pair.first);
			f->WriteString(pair.second);
		}
	}
	//

	auto &collisionMeshes = mdl.GetCollisionMeshes();
	if(!collisionMeshes.empty())
	{
		write_offset(f,offIndex +INDEX_OFFSET_COLLISION_MESHES *INDEX_OFFSET_INDEX_SIZE);
		f->Write<float>(mdl.GetMass());
		f->Write<uint32_t>(collisionMeshes.size());

		auto &surfaceMaterials = game->GetSurfaceMaterials();
		for(auto i=decltype(collisionMeshes.size()){0};i<collisionMeshes.size();++i)
		{
			auto &mesh = collisionMeshes[i];
			auto flags = CollisionMeshLoadFlags::None;
			if(mesh->IsConvex())
				flags |= CollisionMeshLoadFlags::Convex;

			// Collect soft body information
			auto bSoftBody = mesh->IsSoftBody();
			auto *sbInfo = mesh->GetSoftBodyInfo();
			auto *sbMesh = mesh->GetSoftBodyMesh();
			auto *sbTriangles = mesh->GetSoftBodyTriangles();
			auto *sbAnchors = mesh->GetSoftBodyAnchors();
			bSoftBody = (bSoftBody && sbInfo != nullptr && sbMesh != nullptr && sbTriangles != nullptr && sbAnchors != nullptr) ? true : false;

			auto meshGroupId = std::numeric_limits<uint32_t>::max();
			auto meshId = std::numeric_limits<uint32_t>::max();
			auto subMeshId = std::numeric_limits<uint32_t>::max();
			ModelSubMesh *subMesh = nullptr;
			if(bSoftBody == true)
			{
				auto bFound = false;
				for(auto i=decltype(meshGroups.size()){0};i<meshGroups.size();++i)
				{
					auto &group = meshGroups.at(i);
					auto &meshes = group->GetMeshes();
					for(auto j=decltype(meshes.size()){0};j<meshes.size();++j)
					{
						auto &mesh = meshes.at(j);
						auto &subMeshes = mesh->GetSubMeshes();
						for(auto k=decltype(subMeshes.size()){0};k<subMeshes.size();++k)
						{
							subMesh = subMeshes.at(k).get();
							meshGroupId = i;
							meshId = j;
							subMeshId = k;
							bFound = true;
							goto endLoop;
						}
					}
				}
			endLoop:
				if(bFound == false)
					bSoftBody = false;
			}
			//
			if(bSoftBody)
				flags |= CollisionMeshLoadFlags::SoftBody;

			f->Write<CollisionMeshLoadFlags>(flags);
			f->Write<int32_t>(mesh->GetBoneParent());
			// Version 0x0002
			f->Write<Vector3>(mesh->GetOrigin());
			//
			// Version 0x0003
			auto &matSurface = surfaceMaterials[mesh->GetSurfaceMaterial()];
			f->WriteString(matSurface.GetIdentifier());
			//
			Vector3 min,max;
			mesh->GetAABB(&min,&max);

			f->Write<Vector3>(min);
			f->Write<Vector3>(max);
			auto &verts = mesh->GetVertices();
			f->Write<uint64_t>(verts.size());
			for(auto &v : verts)
				f->Write<Vector3>(v);

			// Version 0x0010
			auto &triangles = mesh->GetTriangles();
			f->Write<uint64_t>(triangles.size());
			f->Write(triangles.data(),sizeof(triangles.front()) *triangles.size());
			//

			// Version 0x0011
			f->Write<double>(mesh->GetVolume());
			f->Write<Vector3>(mesh->GetCenterOfMass());
			//

			// Version 0x0014
			if(bSoftBody == true)
			{
				f->Write<uint32_t>(meshGroupId);
				f->Write<uint32_t>(meshId);
				f->Write<uint32_t>(subMeshId);
				f->Write(sbInfo,sizeof(float) *19 +sizeof(uint32_t) *2);
				f->Write<uint32_t>(sbInfo->materialStiffnessCoefficient.size());
				for(auto &pair : sbInfo->materialStiffnessCoefficient)
				{
					f->Write<uint32_t>(pair.first);
					f->Write<float>(pair.second.linear);
					f->Write<float>(pair.second.angular);
					f->Write<float>(pair.second.volume);
				}

				f->Write<uint32_t>(sbTriangles->size());
				for(auto idx : *sbTriangles)
					f->Write<uint32_t>(idx);

				f->Write<uint32_t>(sbAnchors->size());
				for(auto &anchor : *sbAnchors)
					f->Write<CollisionMesh::SoftBodyAnchor>(anchor);
			}
			//
		}
	}
	if(bStatic == false)
	{
		auto &blendControllers = mdl.GetBlendControllers();
		f->Write<uint16_t>(static_cast<uint16_t>(blendControllers.size()));
		for(auto &bc : blendControllers)
		{
			f->WriteString(bc.name);
			f->Write<int32_t>(bc.min);
			f->Write<int32_t>(bc.max);
			f->Write<bool>(bc.loop);
		}

		// Version 0x0016
		write_offset(f,offIndex +INDEX_OFFSET_IK_CONTROLLERS *INDEX_OFFSET_INDEX_SIZE); // IK Controllers
		auto &ikControllers = mdl.GetIKControllers();
		f->Write<uint32_t>(ikControllers.size());
		for(auto &ikController : ikControllers)
		{
			f->WriteString(ikController->GetEffectorName());
			f->WriteString(ikController->GetType());
			f->Write<uint32_t>(ikController->GetChainLength());
			f->Write<uint32_t>(umath::to_integral(ikController->GetMethod()));

			auto &keyValues = ikController->GetKeyValues();
			f->Write<uint32_t>(keyValues.size());
			for(auto &pair : keyValues)
			{
				f->WriteString(pair.first);
				f->WriteString(pair.second);
			}
		}
		//

		write_offset(f,offIndex +INDEX_OFFSET_ANIMATIONS *INDEX_OFFSET_INDEX_SIZE); // Animations
		auto &anims = mdl.GetAnimations();
		f->Write<uint32_t>(static_cast<uint32_t>(anims.size()));
		for(auto i=decltype(anims.size()){0};i<anims.size();++i)
		{
			auto &anim = anims[i];
			f->WriteString(mdl.GetAnimationName(static_cast<uint32_t>(i)));
			anim->SaveLegacy(f);
		}

		// Version 0x0015
		write_offset(f,offIndex +INDEX_OFFSET_VERTEX_ANIMATIONS *INDEX_OFFSET_INDEX_SIZE); // Vertex animations
		auto &vertexAnims = GetVertexAnimations();
		auto &meshGroups = GetMeshGroups();
		f->Write<uint32_t>(vertexAnims.size());
		for(auto &va : vertexAnims)
		{
			f->WriteString(va->GetName());
			auto &meshAnims = va->GetMeshAnimations();

			auto offsetMeshAnimCount = f->Tell();
			auto meshAnimCount = 0u;
			f->Write<uint32_t>(meshAnims.size());
			for(auto &anim : meshAnims)
			{
				auto *mesh = anim->GetMesh();
				auto *subMesh = anim->GetSubMesh();
				if(mesh == nullptr || subMesh == nullptr)
					continue;
				auto meshGroupId = std::numeric_limits<uint32_t>::max();
				auto meshId = std::numeric_limits<uint32_t>::max();
				auto subMeshId = std::numeric_limits<uint32_t>::max();
				for(auto itMeshGroup=meshGroups.begin();itMeshGroup!=meshGroups.end();++itMeshGroup)
				{
					auto &meshGroup = *itMeshGroup;
					auto &meshes = meshGroup->GetMeshes();
					auto itMesh = std::find_if(meshes.begin(),meshes.end(),[mesh](const std::shared_ptr<ModelMesh> &other) {
						return (other.get() == mesh) ? true : false;
					});
					if(itMesh == meshes.end())
						continue;
					auto &mesh = *itMesh;
					auto &subMeshes = mesh->GetSubMeshes();
					meshGroupId = itMeshGroup -meshGroups.begin();
					meshId = itMesh -meshes.begin();
					auto itSubMesh = std::find_if(subMeshes.begin(),subMeshes.end(),[subMesh](const std::shared_ptr<ModelSubMesh> &other) {
						return (other.get() == subMesh) ? true : false;
					});
					if(itSubMesh == subMeshes.end())
						break;
					subMeshId = itSubMesh -subMeshes.begin();
					break;
				}
				if(subMeshId == std::numeric_limits<uint32_t>::max())
				{
					Con::cwar<<"WARNING: Invalid mesh reference in vertex animation '"<<va->GetName()<<"'! Skipping..."<<Con::endl;
					continue;
				}
				++meshAnimCount;
				f->Write<uint32_t>(meshGroupId);
				f->Write<uint32_t>(meshId);
				f->Write<uint32_t>(subMeshId);

				auto &frames = anim->GetFrames();
				f->Write<uint32_t>(frames.size());
				for(auto &frame : frames)
				{
					auto flags = frame->GetFlags();
					auto offsetToEndOfFrameOffset = f->Tell();
					f->Write<uint64_t>(0ull);
					f->Write<MeshVertexFrame::Flags>(flags);

					struct Attribute
					{
						Attribute(const std::string &name,const std::vector<std::array<uint16_t,4>> &vertexData)
							: name{name},vertexData{vertexData}
						{}
						std::string name;
						const std::vector<std::array<uint16_t,4>> &vertexData;
					};
					std::vector<Attribute> attributes {};
					attributes.push_back({"position",frame->GetVertices()});
					if(umath::is_flag_set(flags,MeshVertexFrame::Flags::HasNormals))
						attributes.push_back({"normal",frame->GetNormals()});
					std::set<uint16_t> usedVertIndices {};
					for(auto &attr : attributes)
					{
						auto vertIdx = 0u;
						auto &vdata = attr.vertexData.at(vertIdx);
						for(auto &vdata : attr.vertexData)
						{
							auto itUsed = std::find_if(vdata.begin(),vdata.end(),[](const uint16_t &v) {return v != 0;});
							if(itUsed != vdata.end())
								usedVertIndices.insert(vertIdx);
							++vertIdx;
						}
					}

					f->Write<uint16_t>(usedVertIndices.size());
					for(auto idx : usedVertIndices)
						f->Write<uint16_t>(idx);
					f->Write<uint16_t>(attributes.size());
					for(auto &attr : attributes)
					{
						f->WriteString(attr.name);
						for(auto idx : usedVertIndices)
							f->Write<std::array<uint16_t,4>>(attr.vertexData.at(idx));
					}
					write_offset(f,offsetToEndOfFrameOffset);
				}
			}

			// Write actual frame count
			auto cur = f->Tell();
			f->Seek(offsetMeshAnimCount);
			f->Write<uint32_t>(meshAnimCount);
			f->Seek(cur);
		}

		write_offset(f,offIndex +INDEX_OFFSET_FLEX_CONTROLLERS *INDEX_OFFSET_INDEX_SIZE); // Flex controllers
		auto &flexControllers = GetFlexControllers();
		f->Write<uint32_t>(flexControllers.size());
		for(auto &fc : flexControllers)
		{
			f->WriteString(fc.name);
			f->Write<float>(fc.min);
			f->Write<float>(fc.max);
		}

		write_offset(f,offIndex +INDEX_OFFSET_FLEXES *INDEX_OFFSET_INDEX_SIZE); // Flexes
		auto &flexes = GetFlexes();
		f->Write<uint32_t>(flexes.size());
		for(auto &flex : flexes)
		{
			f->WriteString(flex.GetName());

			auto *va = flex.GetVertexAnimation();
			if(va != nullptr)
			{
				auto &vertAnims = GetVertexAnimations();
				auto itVa = std::find(vertAnims.begin(),vertAnims.end(),va->shared_from_this());
				if(itVa == vertAnims.end())
					va = nullptr;
				else
					f->Write<uint32_t>(itVa -vertAnims.begin());
			}
			else
				f->Write<uint32_t>(std::numeric_limits<uint32_t>::max());
			f->Write<uint32_t>(flex.GetFrameIndex());

			auto &ops = flex.GetOperations();
			f->Write<uint32_t>(ops.size());
			for(auto &op : ops)
			{
				f->Write<uint32_t>(umath::to_integral(op.type));
				f->Write<float>(op.d.value);
			}
		}

		write_offset(f,offIndex +INDEX_OFFSET_PHONEMES *INDEX_OFFSET_INDEX_SIZE); // Phonemes
		auto &phonemeMap = GetPhonemeMap();
		f->Write<uint32_t>(phonemeMap.phonemes.size());
		for(auto &pair : phonemeMap.phonemes)
		{
			f->WriteString(pair.first);
			auto numControllers = pair.second.flexControllers.size();
			auto offsetNumControllers = f->Tell();
			f->Write<uint32_t>(numControllers);
			for(auto &pair : pair.second.flexControllers)
			{
				auto id = 0u;
				if(GetFlexControllerId(pair.first,id) == false)
				{
					--numControllers;
					continue;
				}
				f->Write<uint32_t>(id);
				f->Write<float>(pair.second);
			}
			auto offset = f->Tell();
			f->Seek(offsetNumControllers);
			f->Write<uint32_t>(numControllers);
			f->Seek(offset);
		}
		//

		// Eyeballs
		write_offset(f,offIndex +INDEX_OFFSET_EYEBALLS *INDEX_OFFSET_INDEX_SIZE);

		f->Write(GetMaxEyeDeflection());
		auto &eyeballs = GetEyeballs();
		f->Write<uint32_t>(eyeballs.size());
		for(auto &eyeball : eyeballs)
		{
			f->WriteString(eyeball.name);
			f->Write(reinterpret_cast<const uint8_t*>(&eyeball) +sizeof(std::string),sizeof(Eyeball) -sizeof(std::string));
		}
		//

		// Flex animations
		write_offset(f,offIndex +INDEX_OFFSET_FLEX_ANIMATIONS *INDEX_OFFSET_INDEX_SIZE);
		auto &flexAnims = mdl.GetFlexAnimations();
		auto &flexAnimNames = mdl.GetFlexAnimationNames();
		f->Write<uint32_t>(static_cast<uint32_t>(flexAnims.size()));
		for(auto i=decltype(flexAnims.size()){0};i<flexAnims.size();++i)
		{
			auto &anim = flexAnims[i];
			auto &animName = flexAnimNames[i];
			f->WriteString(animName);
			anim->Save(f);
		}
		//
	}

	auto &includes = meta.includes;
	f->Write<uint8_t>(static_cast<uint8_t>(includes.size()));
	for(auto &inc : includes)
		f->WriteString(inc);
	return true;
}
