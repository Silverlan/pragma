// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

module pragma.shared;

import :model.model;

#define INDEX_OFFSET_INDEX_SIZE sizeof(uint64_t)
#define INDEX_OFFSET_MODEL_DATA 0
#define INDEX_OFFSET_MODEL_MESHES (INDEX_OFFSET_MODEL_DATA + 1)
#define INDEX_OFFSET_LOD_DATA (INDEX_OFFSET_MODEL_MESHES + 1)
#define INDEX_OFFSET_BODYGROUPS (INDEX_OFFSET_LOD_DATA + 1)
#define INDEX_OFFSET_JOINTS (INDEX_OFFSET_BODYGROUPS + 1)
#define INDEX_OFFSET_COLLISION_MESHES (INDEX_OFFSET_JOINTS + 1)
#define INDEX_OFFSET_BONES (INDEX_OFFSET_COLLISION_MESHES + 1)
#define INDEX_OFFSET_ANIMATIONS (INDEX_OFFSET_BONES + 1)
#define INDEX_OFFSET_VERTEX_ANIMATIONS (INDEX_OFFSET_ANIMATIONS + 1)
#define INDEX_OFFSET_FLEX_CONTROLLERS (INDEX_OFFSET_VERTEX_ANIMATIONS + 1)
#define INDEX_OFFSET_FLEXES (INDEX_OFFSET_FLEX_CONTROLLERS + 1)
#define INDEX_OFFSET_PHONEMES (INDEX_OFFSET_FLEXES + 1)
#define INDEX_OFFSET_IK_CONTROLLERS (INDEX_OFFSET_PHONEMES + 1)
#define INDEX_OFFSET_EYEBALLS (INDEX_OFFSET_IK_CONTROLLERS + 1)
#define INDEX_OFFSET_FLEX_ANIMATIONS (INDEX_OFFSET_EYEBALLS + 1)

static void write_offset(pragma::fs::VFilePtrReal f, uint64_t offIndex)
{
	auto cur = f->Tell();
	f->Seek(offIndex);
	f->Write<uint64_t>(cur);
	f->Seek(cur);
}

static void to_vertex_list(pragma::geometry::ModelMesh &mesh, std::vector<pragma::math::Vertex> &vertices, std::unordered_map<pragma::geometry::ModelSubMesh *, std::vector<uint32_t>> &vertexIds)
{
	vertices.reserve(mesh.GetVertexCount());
	for(auto &subMesh : mesh.GetSubMeshes()) {
		auto &verts = subMesh->GetVertices();
		auto itMesh = vertexIds.insert(std::remove_reference_t<decltype(vertexIds)>::value_type(subMesh.get(), {})).first;
		auto &vertIds = itMesh->second;
		vertIds.reserve(verts.size());
		for(auto &v : verts) {
			auto it = std::find(vertices.begin(), vertices.end(), v);
			if(it == vertices.end()) {
				vertices.push_back({v.position, v.uv, v.normal, v.tangent});
				it = vertices.end() - 1;
			}
			vertIds.push_back(static_cast<uint32_t>(it - vertices.begin()));
		}
	}
}

struct MeshBoneWeight {
	MeshBoneWeight(uint64_t vId, float w) : vertId(vId), weight(w) {}
	uint64_t vertId;
	float weight;
};
static void to_vertex_weight_list(pragma::geometry::ModelMesh &mesh, std::unordered_map<uint32_t, std::vector<MeshBoneWeight>> &boneWeights, const std::unordered_map<pragma::geometry::ModelSubMesh *, std::vector<uint32_t>> &vertexIds)
{
	// TODO: Has to be the same order as 'to_vertex_list'!!
	for(auto &subMesh : mesh.GetSubMeshes()) {
		auto &meshVerts = vertexIds.find(subMesh.get())->second;
		auto &weights = subMesh->GetVertexWeights();
		for(auto i = decltype(weights.size()) {0}; i < weights.size(); ++i) {
			auto vertId = meshVerts[i];
			auto &vertexWeight = weights[i];
			auto &weights = vertexWeight.weights;
			auto &boneId = vertexWeight.boneIds;
			for(uint8_t i = 0; i < 4; ++i) {
				if(boneId[i] == -1)
					continue;
				auto it = boneWeights.find(boneId[i]);
				if(it == boneWeights.end())
					it = boneWeights.insert(std::remove_reference_t<decltype(boneWeights)>::value_type(boneId[i], {})).first;
				it->second.push_back({vertId, weights[i]});
			}
		}
	}
}

std::shared_ptr<pragma::asset::Model> pragma::asset::Model::Copy(Game *game, CopyFlags copyFlags) const
{
	auto fCreateModel = static_cast<std::shared_ptr<Model> (Game::*)(bool) const>(&Game::CreateModel);
	auto mdl = (game->*fCreateModel)(false);
	if(mdl == nullptr)
		return nullptr;
	mdl->m_metaInfo = m_metaInfo;
	mdl->m_stateFlags = m_stateFlags;
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
	mdl->m_animations = m_animations;
	mdl->m_flexAnimations = m_flexAnimations;
	mdl->m_animationIDs = m_animationIDs;
	mdl->m_skeleton = std::make_unique<animation::Skeleton>(*m_skeleton);
	mdl->m_metaRig = m_metaRig ? std::make_unique<animation::MetaRig>(*m_metaRig) : nullptr;
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
		ikController = pragma::util::make_shared<physics::IKController>(*ikController);
	std::unordered_map<geometry::ModelMesh *, geometry::ModelMesh *> oldMeshToNewMesh;
	std::unordered_map<geometry::ModelSubMesh *, geometry::ModelSubMesh *> oldSubMeshToNewSubMesh;
	if((copyFlags & CopyFlags::CopyMeshesBit) != CopyFlags::None) {
		auto copyVertexData = math::is_flag_set(copyFlags, CopyFlags::CopyVertexData);
		for(auto &meshGroup : mdl->m_meshGroups) {
			auto newMeshGroup = ModelMeshGroup::Create(meshGroup->GetName());
			static_assert(ModelMeshGroup::layout_version == 1, "Update this function when making changes to this class!");
			newMeshGroup->GetMeshes() = meshGroup->GetMeshes();
			for(auto &mesh : newMeshGroup->GetMeshes()) {
				auto newMesh = mesh->Copy();
				oldMeshToNewMesh[mesh.get()] = newMesh.get();
				for(auto &subMesh : newMesh->GetSubMeshes()) {
					auto newSubMesh = subMesh->Copy(copyVertexData);
					if(math::is_flag_set(copyFlags, CopyFlags::CopyUniqueIdsBit))
						newSubMesh->SetUuid(subMesh->GetUuid());
					oldSubMeshToNewSubMesh[subMesh.get()] = newSubMesh.get();
					subMesh = newSubMesh;
				}
				mesh = newMesh;
			}
			meshGroup = newMeshGroup;
		}
	}
	if((copyFlags & CopyFlags::CopyAnimationsBit) != CopyFlags::None) {
		for(auto &anim : mdl->m_animations)
			anim = animation::Animation::Create(*anim, animation::Animation::ShareMode::None);
	}
	if((copyFlags & CopyFlags::CopyVertexAnimationsBit) != CopyFlags::None) {
		for(auto &vertexAnim : mdl->m_vertexAnimations) {
			vertexAnim = animation::VertexAnimation::Create(*vertexAnim);
			for(auto &meshAnim : vertexAnim->GetMeshAnimations()) {
				auto *mesh = meshAnim->GetMesh();
				auto *subMesh = meshAnim->GetSubMesh();
				if(mesh == nullptr || subMesh == nullptr)
					continue;
				auto itMesh = oldMeshToNewMesh.find(mesh);
				auto itSubMesh = oldSubMeshToNewSubMesh.find(subMesh);
				if(itMesh == oldMeshToNewMesh.end() || itSubMesh == oldSubMeshToNewSubMesh.end())
					continue;
				meshAnim->SetMesh(*itMesh->second, *itSubMesh->second);
			}
		}
	}
	if((copyFlags & CopyFlags::CopyCollisionMeshesBit) != CopyFlags::None) {
		for(auto &colMesh : mdl->m_collisionMeshes) {
			auto oldColMesh = colMesh;
			colMesh = physics::CollisionMesh::Create(*colMesh);
			if(math::is_flag_set(copyFlags, CopyFlags::CopyUniqueIdsBit))
				colMesh->SetUuid(oldColMesh->GetUuid());
			// TODO: Update shape?
		}
	}
	if((copyFlags & CopyFlags::CopyFlexAnimationsBit) != CopyFlags::None) {
		for(auto &flexAnim : mdl->m_flexAnimations)
			flexAnim = pragma::util::make_shared<FlexAnimation>(*flexAnim);
	}
	// TODO: Copy collision mesh soft body sub mesh reference

	// Copy extension data
	std::stringstream extStream {};
	ufile::OutStreamFile extStreamFileOut {std::move(extStream)};
	m_extensions->Write(extStreamFileOut);

	mdl->m_extensions = udm::Property::Create(udm::Type::Element);
	ufile::InStreamFile extStreamFileIn {std::move(extStreamFileOut.MoveStream())};
	mdl->m_extensions->Read(extStreamFileIn);
	//

	static_assert(layout_version == 1, "Update this function when making changes to this class!");
	return mdl;
}

bool pragma::asset::Model::FindSubMeshIndex(const ModelMeshGroup *optMeshGroup, const geometry::ModelMesh *optMesh, const geometry::ModelSubMesh *optSubMesh, uint32_t &outGroupIdx, uint32_t &outMeshIdx, uint32_t &outSubMeshIdx) const
{
	auto &meshGroups = GetMeshGroups();
	uint32_t mgStart = 0;
	uint32_t mgEnd = meshGroups.size();
	if(optMeshGroup) {
		auto it = std::find_if(meshGroups.begin(), meshGroups.end(), [optMeshGroup](const std::shared_ptr<ModelMeshGroup> &mmg) { return mmg.get() == optMeshGroup; });
		if(it == meshGroups.end())
			return false;
		mgStart = (it - meshGroups.begin());
		mgEnd = mgStart + 1;
		if(!optMesh && !optSubMesh) {
			outGroupIdx = mgStart;
			return true;
		}
	}

	for(auto i = mgStart; i < mgEnd; ++i) {
		auto &mg = meshGroups[i];
		auto &meshes = mg->GetMeshes();
		uint32_t meshStart = 0;
		uint32_t meshEnd = meshes.size();
		if(optMesh) {
			auto it = std::find_if(meshes.begin(), meshes.end(), [optMesh](const std::shared_ptr<geometry::ModelMesh> &mesh) { return mesh.get() == optMesh; });
			if(it == meshes.end())
				continue;
			meshStart = (it - meshes.begin());
			meshEnd = meshStart + 1;
			if(!optSubMesh) {
				outGroupIdx = i;
				outMeshIdx = meshStart;
				return true;
			}
		}
		if(optSubMesh == nullptr)
			continue;
		for(auto j = meshStart; j < meshEnd; ++j) {
			auto &mesh = meshes[j];
			auto &subMeshes = mesh->GetSubMeshes();
			auto it = std::find_if(subMeshes.begin(), subMeshes.end(), [optSubMesh](const std::shared_ptr<geometry::ModelSubMesh> &subMesh) { return subMesh.get() == optSubMesh; });
			if(it == subMeshes.end())
				continue;
			outGroupIdx = i;
			outMeshIdx = j;
			outSubMeshIdx = (it - subMeshes.begin());
			return true;
		}
	}
	return false;
}

bool pragma::asset::Model::LoadFromAssetData(Game &game, const udm::AssetData &data, std::string &outErr)
{
	if(data.GetAssetType() != PMDL_IDENTIFIER) {
		outErr = "Incorrect format!";
		return false;
	}

	const auto udm = *data;
	auto version = data.GetAssetVersion();
	if(version < 1) {
		outErr = "Invalid version!";
		return false;
	}
	// if(version > PMDL_VERSION)
	// 	return false;
	auto activity = udm["activity"];

	udm["materialPaths"].GetBlobData(GetTexturePaths());
	udm["materials"].GetBlobData(GetTextures());
	m_materials.resize(m_metaInfo.textures.size());
	udm["includeModels"].GetBlobData(GetMetaInfo().includes);
	udm["eyeOffset"](m_eyeOffset);
	udm["maxEyeDeflection"](m_maxEyeDeflection);
	udm["mass"](m_mass);

	auto udmLods = udm["lods"];
	m_lods.reserve(udmLods.GetSize());
	for(auto &udmLod : udmLods) {
		m_lods.push_back({});
		auto &lodInfo = m_lods.back();
		udmLod["distance"](lodInfo.distance);
		udmLod["lod"](lodInfo.lod);

		auto udmMeshGroupReplacements = udmLod["meshGroupReplacements"];
		for(auto &udmMeshGroupReplacement : udmMeshGroupReplacements) {
			uint32_t source = 0u;
			uint32_t target = 0u;
			udmMeshGroupReplacement["source"](source);
			udmMeshGroupReplacement["target"](target);
			lodInfo.meshReplacements[source] = target;
		}
	}

	udm["render"]["bounds"]["min"](m_renderMin);
	udm["render"]["bounds"]["max"](m_renderMax);

	auto readFlag = [this](auto udm, auto flag, const std::string &name, auto &outFlags) {
		auto udmFlags = udm["flags"];
		if(!udmFlags)
			return;
		math::set_flag(outFlags, flag, udmFlags[name](false));
	};
	auto &flags = GetMetaInfo().flags;
	readFlag(udm, Flags::Static, "static", flags);
	readFlag(udm, Flags::Inanimate, "inanimate", flags);
	readFlag(udm, Flags::DontPrecacheTextureGroups, "dontPrecacheSkins", flags);
	readFlag(udm, Flags::WorldGeometry, "worldGeometry", flags);
	readFlag(udm, Flags::GeneratedHitboxes, "generatedHitboxes", flags);
	readFlag(udm, Flags::GeneratedLODs, "generatedLODs", flags);
	readFlag(udm, Flags::GeneratedMetaRig, "generatedMetaRig", flags);
	readFlag(udm, Flags::GeneratedMetaBlendShapes, "generatedMetaBlendShapes", flags);
	static_assert(math::to_integral(Flags::Count) == 13, "Update this list when new flags have been added!");

	auto isStatic = math::is_flag_set(flags, Flags::Static);
	if(!isStatic) {
		auto udmSkeleton = udm["skeleton"];
		m_skeleton = animation::Skeleton::Load(udm::AssetData {udmSkeleton}, outErr);
		if(m_skeleton == nullptr) {
			outErr = "Failed to load skeleton: " + outErr;
			return false;
		}
		auto &ref = GetReference();
		auto &poses = m_skeleton->GetBonePoses();
		ref.SetBoneCount(poses.size());
		for(uint32_t boneId = 0u; auto &pose : poses)
			ref.SetBonePose(boneId++, pose);

		auto &attachments = GetAttachments();
		auto udmAttachments = udm["attachments"];
		auto numAttachments = udmAttachments.GetSize();
		attachments.resize(numAttachments);
		for(auto i = decltype(numAttachments) {0u}; i < numAttachments; ++i) {
			auto &att = attachments[i];
			math::Transform pose {};
			auto udmAttachment = udmAttachments[i];
			udmAttachment["name"](att.name);
			udmAttachment["bone"](att.bone);
			udmAttachment["pose"](pose);
			att.offset = pose.GetOrigin();
			att.angles = pose.GetRotation();
		}

		auto &objAttachments = GetObjectAttachments();
		auto udmObjAttachments = udm["objectAttachments"];
		auto numObjAttachments = udmObjAttachments.GetSize();
		objAttachments.resize(numObjAttachments);
		for(auto i = decltype(numObjAttachments) {0u}; i < numObjAttachments; ++i) {
			auto &objAtt = objAttachments[i];
			auto udmObjAttachment = udmObjAttachments[i];
			udmObjAttachment["name"](objAtt.name);

			int32_t attId = -1;
			udmObjAttachment["attachment"](attId);
			if(attId >= 0 && attId < attachments.size())
				objAtt.attachment = attachments[attId].name;
			udm::to_enum_value<ObjectAttachment::Type>(udmObjAttachment["type"], objAtt.type);

			udmObjAttachment["keyValues"](objAtt.keyValues);
		}

		auto &hitboxes = GetHitboxes();
		auto udmHitboxes = udm["hitboxes"];
		auto numHitboxes = udmHitboxes.GetSize();
		hitboxes.reserve(numHitboxes);
		for(auto i = decltype(numHitboxes) {0u}; i < numHitboxes; ++i) {
			auto udmHb = udmHitboxes[i];
			physics::Hitbox hb {};

			udm::to_enum_value<physics::HitGroup>(udmHb["hitGroup"], hb.group);
			udmHb["bounds"]["min"](hb.min);
			udmHb["bounds"]["max"](hb.max);

			auto boneId = std::numeric_limits<uint32_t>::max();
			udmHb["bone"](boneId);
			if(boneId != std::numeric_limits<uint32_t>::max())
				hitboxes[boneId] = hb;
		}

		auto udmMetaRig = udm["metaRig"];
		if(udmMetaRig) {
			m_metaRig = animation::MetaRig::Load(*m_skeleton, udm::AssetData {udmMetaRig}, outErr);
			if(!m_metaRig) {
				outErr = "Failed to load meta rig: " + outErr;
				return false;
			}
		}
	}

	// Bodygroups
	auto &bodyGroups = GetBodyGroups();
	auto udmBodyGroups = udm["bodyGroups"];
	auto numBodyGroups = udmBodyGroups.GetChildCount();
	bodyGroups.resize(numBodyGroups);
	uint32_t idx = 0;
	for(auto udmBodyGroup : udmBodyGroups.ElIt()) {
		auto &bg = bodyGroups[idx++];
		bg.name = udmBodyGroup.key;
		udmBodyGroup.property["meshGroups"](bg.meshGroups);
	}

	udm["baseMeshGroups"].GetBlobData(m_baseMeshes);

	// Material groups
	auto &texGroups = GetTextureGroups();
	auto udmTexGroups = udm["skins"];
	auto numTexGroups = udmTexGroups.GetSize();
	texGroups.resize(numTexGroups);
	for(auto i = decltype(texGroups.size()) {0u}; i < texGroups.size(); ++i) {
		auto &texGroup = texGroups[i];
		auto udmMaterials = udmTexGroups[i]["materials"];
		udmMaterials.GetBlobData(texGroup.textures);
	}

	// Collision meshes
	auto &colMeshes = GetCollisionMeshes();
	auto udmColMeshes = udm["collisionMeshes"];
	auto numColMeshes = udmColMeshes.GetSize();
	colMeshes.resize(numColMeshes);
	for(auto i = decltype(numColMeshes) {0u}; i < numColMeshes; ++i) {
		auto &colMesh = colMeshes[i];
		colMesh = physics::CollisionMesh::Load(game, *this, udm::AssetData {udmColMeshes[i]}, outErr);
		if(colMesh == nullptr) {
			outErr = "Failed to load collision mesh " + std::to_string(i) + ": " + outErr;
			return false;
		}
	}

	// Joints
	auto &joints = GetJoints();
	auto udmJoints = udm["joints"];
	auto numJoints = udmJoints.GetSize();
	joints.resize(numJoints);
	for(auto i = decltype(numJoints) {0u}; i < numJoints; ++i) {
		auto &joint = joints[i];
		auto udmJoint = udmJoints[i];

		udm::to_enum_value<physics::JointType>(udmJoint["type"], joint.type);
		udmJoint["parentBone"](joint.parent);
		udmJoint["childBone"](joint.child);
		udmJoint["enableCollisions"](joint.collide);
		udmJoint["args"](joint.args);
	}

	auto &meshGroups = GetMeshGroups();
	auto udmMeshGroups = udm["meshGroups"];
	meshGroups.resize(udmMeshGroups.GetChildCount());
	for(auto udmMeshGroup : udmMeshGroups.ElIt()) {
		auto meshGroup = ModelMeshGroup::Create(std::string {udmMeshGroup.key});
		uint32_t groupIdx = 0;
		udmMeshGroup.property["index"](groupIdx);
		if(groupIdx >= meshGroups.size()) {
			spdlog::warn("Invalid mesh group index: " + std::to_string(groupIdx));
			continue;
		}
		meshGroups[groupIdx] = meshGroup;
		auto udmMeshes = udmMeshGroup.property["meshes"];
		auto numMeshes = udmMeshes.GetSize();
		auto &meshes = meshGroup->GetMeshes();
		meshes.resize(numMeshes);
		for(auto meshIdx = decltype(numMeshes) {0u}; meshIdx < numMeshes; ++meshIdx) {
			auto &mesh = meshes[meshIdx];
			auto udmMesh = udmMeshes[meshIdx];
			mesh = pragma::util::make_shared<geometry::ModelMesh>();
			auto referenceId = std::numeric_limits<uint32_t>::max();
			udmMesh["referenceId"](referenceId);
			mesh->SetReferenceId(referenceId);
			auto udmSubMeshes = udmMesh["subMeshes"];
			auto numSubMeshes = udmSubMeshes.GetSize();
			auto &subMeshes = mesh->GetSubMeshes();
			subMeshes.resize(numSubMeshes);
			for(auto subMeshIdx = decltype(numSubMeshes) {0u}; subMeshIdx < numSubMeshes; ++subMeshIdx) {
				auto &subMesh = subMeshes[subMeshIdx];
				auto udmSubMesh = udmSubMeshes[subMeshIdx];
				subMesh = CreateSubMesh();
				subMesh->LoadFromAssetData(udm::AssetData {udmSubMesh}, outErr);
				if(subMesh == nullptr) {
					outErr = "Failed to load sub mesh " + std::to_string(subMeshIdx) + " of mesh " + std::to_string(meshIdx) + " of mesh group " + std::string {udmMeshGroup.key} + ": " + outErr;
					return false;
				}
				// subMesh->Update(ModelUpdateFlags::UpdateBuffers);
			}
		}
	}

	if(!isStatic) {
		auto &skeleton = GetSkeleton();
		auto &reference = GetReference();
		auto &animations = GetAnimations();
		auto udmAnimations = udm["animations"];
		auto numExpected = udmAnimations.GetChildCount();
		animations.resize(udmAnimations.GetChildCount());
		for(auto udmAnimation : udmAnimations.ElIt()) {
			auto anim = animation::Animation::Load(udm::AssetData {udmAnimation.property}, outErr, &skeleton, &reference);
			if(anim == nullptr) {
				outErr = "Failed to load animation " + std::string {udmAnimation.key} + ": " + outErr;
				return false;
			}
			uint32_t index = 0;
			udmAnimation.property["index"](index);
			m_animationIDs[std::string {udmAnimation.key}] = index;
			if(index >= animations.size())
				animations.resize(index + 1);
			animations[index] = anim;
		}
		if(animations.size() != numExpected) {
			for(auto &anim : animations) {
				if(anim)
					continue;
				anim = animation::Animation::Create(); // Create a dummy animation
			}
		}

		auto &blendControllers = GetBlendControllers();
		auto udmBlendControllers = udm["blendControllers"];
		auto numBlendControllers = udmBlendControllers.GetSize();
		blendControllers.resize(numBlendControllers);
		for(auto i = decltype(numBlendControllers) {0u}; i < numBlendControllers; ++i) {
			auto &bc = blendControllers[i];
			auto udmBc = udmBlendControllers[i];
			udmBc["name"](bc.name);
			udmBc["min"](bc.min);
			udmBc["max"](bc.max);
			udmBc["loop"](bc.loop);
		}

		auto &ikControllers = GetIKControllers();
		auto udmIkControllers = udm["ikControllers"];
		auto numIkControllers = udmIkControllers.GetSize();
		ikControllers.resize(numIkControllers);
		for(auto i = decltype(numIkControllers) {0u}; i < numIkControllers; ++i) {
			auto &ikc = ikControllers[i];
			auto udmIkController = udmIkControllers[i];

			std::string effectorName;
			std::string type;
			udmIkController["effectorName"](effectorName);
			udmIkController["type"](type);

			ikc->SetEffectorName(effectorName);
			ikc->SetType(type);
			uint32_t chainLength = 0;
			udmIkController["chainLength"](chainLength);
			ikc->SetChainLength(chainLength);
			auto method = ikc->GetMethod();
			udm::to_enum_value<physics::ik::Method>(udmIkController["method"], method);
			ikc->SetMethod(method);

			udmIkController["keyValues"](ikc->GetKeyValues());
		}

		auto &morphAnims = GetVertexAnimations();
		auto udmMorphTargetAnims = udm["morphTargetAnimations"];
		auto numMorphTargetAnims = udmMorphTargetAnims.GetChildCount();
		morphAnims.resize(numMorphTargetAnims);
		for(auto udmMorphTargetAnim : udmMorphTargetAnims.ElIt()) {
			udmMorphTargetAnim.property["index"](idx);
			morphAnims[idx] = animation::VertexAnimation::Load(*this, udm::AssetData {udmMorphTargetAnim.property}, outErr);
			if(morphAnims[idx] == nullptr) {
				outErr = "Failed to load vertex animation " + std::string {udmMorphTargetAnim.key} + ": " + outErr;
				return false;
			}
			++idx;
		}

		auto &flexes = GetFlexes();
		auto udmFlexes = udm["flexes"];
		flexes.resize(udmFlexes.GetChildCount());
		for(auto udmFlex : udmFlexes.ElIt()) {
			udmFlex.property["index"](idx);
			if(idx >= flexes.size()) {
				// outErr = "Flex index " + std::to_string(idx) + " out of range! (Number of flexes: " + std::to_string(flexes.size()) + ")";
				// return false;
				Con::CWAR << "Flex index " << std::to_string(idx) << " out of range! (Number of flexes: " << std::to_string(flexes.size()) << ")" << Con::endl;
				flexes.resize(idx + 1);
			}
			auto &flex = flexes[idx];
			flex.GetName() = udmFlex.key;

			auto morphTargetAnimation = std::numeric_limits<uint32_t>::max();
			udmFlex.property["morphTargetAnimation"](morphTargetAnimation);
			if(morphTargetAnimation < morphAnims.size()) {
				uint32_t frameIndex = 0;
				flex.SetVertexAnimation(*morphAnims[morphTargetAnimation], frameIndex);
			}

			auto &ops = flex.GetOperations();
			auto udmOps = udmFlex.property["operations"];
			auto numOps = udmOps.GetSize();
			ops.resize(numOps);
			for(auto i = decltype(numOps) {0u}; i < numOps; ++i) {
				auto &op = ops[i];
				auto udmOp = udmOps[i];
				udm::to_enum_value<animation::Flex::Operation::Type>(udmOp["type"], op.type);
				if(udmOp["value"])
					udmOp["value"](op.d.value);
				else if(udmOp["index"])
					udmOp["index"](op.d.index);
			}
		}

		auto &flexControllers = GetFlexControllers();
		auto udmFlexControllers = udm["flexControllers"];
		auto numFlexControllers = udmFlexControllers.GetChildCount();
		flexControllers.resize(numFlexControllers);
		for(auto udmFlexController : udmFlexControllers.ElIt()) {
			udmFlexController.property["index"](idx);
			auto &flexC = flexControllers[idx];
			flexC.name = udmFlexController.key;
			udmFlexController.property["min"](flexC.min);
			udmFlexController.property["max"](flexC.max);
		}

		auto &eyeballs = GetEyeballs();
		auto udmEyeballs = udm["eyeballs"];
		auto numEyeballs = udmEyeballs.GetChildCount();
		eyeballs.resize(numEyeballs);
		for(auto udmEyeball : udmEyeballs.ElIt()) {
			udmEyeball.property["index"](idx);
			auto &eyeball = eyeballs[idx];
			eyeball.name = udmEyeball.key;
			udmEyeball.property["bone"](eyeball.boneIndex);
			udmEyeball.property["origin"](eyeball.origin);
			udmEyeball.property["zOffset"](eyeball.zOffset);
			udmEyeball.property["radius"](eyeball.radius);
			udmEyeball.property["up"](eyeball.up);
			udmEyeball.property["forward"](eyeball.forward);
			udmEyeball.property["maxDilationFactor"](eyeball.maxDilationFactor);

			udmEyeball.property["iris"]["material"](eyeball.irisMaterialIndex);
			udmEyeball.property["iris"]["uvRadius"](eyeball.irisUvRadius);
			udmEyeball.property["iris"]["scale"](eyeball.irisScale);

			auto writeLid = [](udm::LinkedPropertyWrapperArg prop, const Eyeball::LidFlexDesc &lid) {
				prop["raiser"]["lidFlexIndex"] = lid.lidFlexIndex;

				prop["raiser"]["raiserFlexIndex"] = lid.raiserFlexIndex;
				prop["raiser"]["targetAngle"] = math::rad_to_deg(lid.raiserValue);

				prop["neutral"]["neutralFlexIndex"] = lid.neutralFlexIndex;
				prop["neutral"]["targetAngle"] = math::rad_to_deg(lid.neutralValue);

				prop["lowerer"]["lowererFlexIndex"] = lid.lowererFlexIndex;
				prop["lowerer"]["targetAngle"] = math::rad_to_deg(lid.lowererValue);
			};
			writeLid(udmEyeball.property["eyelids"]["upperLid"], eyeball.upperLid);
			writeLid(udmEyeball.property["eyelids"]["lowerLid"], eyeball.lowerLid);
		}

		auto &phonemeMap = GetPhonemeMap();
		auto udmPhonemes = udm["phonemes"];
		phonemeMap.phonemes.reserve(udmPhonemes.GetChildCount());
		for(auto udmPhoneme : udmPhonemes.ElIt()) {
			auto &phonemeInfo = phonemeMap.phonemes[std::string {udmPhoneme.key}] = {};
			udmPhoneme.property(phonemeInfo.flexControllers);
		}

		auto &flexAnims = GetFlexAnimations();
		auto &flexAnimNames = GetFlexAnimationNames();
		auto udmFlexAnims = udm["flexAnimations"];
		auto numFlexAnims = udmFlexAnims.GetChildCount();
		flexAnims.resize(numFlexAnims);
		flexAnimNames.resize(numFlexAnims);
		for(auto udmFlexAnim : udmFlexAnims.ElIt()) {
			udmFlexAnim.property["index"](idx);
			flexAnimNames[idx] = udmFlexAnim.key;
			flexAnims[idx] = FlexAnimation::Load(udm::AssetData {udmFlexAnim.property}, outErr);
			if(flexAnims[idx] == nullptr) {
				outErr = "Failed to load flex animation " + std::string {udmFlexAnim.key} + ": " + outErr;
				return false;
			}
		}
	}

	auto udmExtensions = udm["extensions"];
	if(udmExtensions)
		m_extensions = udmExtensions.ClaimOwnership();
	else
		m_extensions = udm::Property::Create(udm::Type::Element);
	return true;
}

bool pragma::asset::Model::Save(Game &game, const std::string &fileName, std::string &outErr)
{
	auto udmData = udm::Data::Create();
	auto result = Save(game, udmData->GetAssetData(), outErr);
	if(result == false)
		return false;
	fs::create_path(ufile::get_path_from_filename(fileName));
	auto writeFileName = fileName;
	ufile::remove_extension_from_filename(writeFileName, get_supported_extensions(Type::Model));
	writeFileName += '.' + std::string {FORMAT_MODEL_BINARY};
	auto f = fs::open_file<fs::VFilePtrReal>(writeFileName, fs::FileMode::Write | fs::FileMode::Binary);
	if(f == nullptr) {
		outErr = "Unable to open file '" + writeFileName + "'!";
		return false;
	}
	result = udmData->Save(f);
	if(result == false) {
		outErr = "Unable to save UDM data!";
		return false;
	}
	get_engine()->PollResourceWatchers();
	return true;
}
bool pragma::asset::Model::Save(Game &game, std::string &outErr)
{
	auto mdlName = GetName();
	ufile::remove_extension_from_filename(mdlName, get_supported_extensions(Type::Model));
	mdlName += '.' + std::string {FORMAT_MODEL_BINARY};

	std::string absFileName;
	auto result = fs::find_absolute_path("models/" + mdlName, absFileName);
	if(result == false)
		absFileName = "models/" + mdlName;
	else {
		auto path = util::Path::CreateFile(absFileName);
		path.MakeRelative(fs::get_program_write_path());
		absFileName = path.GetString();
	}
	return Save(game, absFileName, outErr);
}

bool pragma::asset::Model::Save(Game &game, udm::AssetDataArg outData, std::string &outErr)
{
	outData.SetAssetType(PMDL_IDENTIFIER);
	outData.SetAssetVersion(PMDL_VERSION);
	auto udm = *outData;

	udm["materialPaths"] = GetTexturePaths();
	udm["materials"] = m_metaInfo.textures;
	udm["includeModels"] = GetMetaInfo().includes;
	udm["eyeOffset"] = GetEyeOffset();
	udm["maxEyeDeflection"] = m_maxEyeDeflection;
	udm["mass"] = m_mass;

	auto udmLods = udm.AddArray("lods", m_lods.size());
	uint32_t idx = 0;
	for(auto &lodInfo : m_lods) {
		auto udmLod = udmLods[idx++];
		udmLod["distance"] = lodInfo.distance;
		udmLod["lod"] = lodInfo.lod;
		auto udmMeshGroupReplacements = udmLod.AddArray("meshGroupReplacements", lodInfo.meshReplacements.size());
		uint32_t idxMg = 0;
		for(auto &pair : lodInfo.meshReplacements) {
			auto udmMeshGroupReplacement = udmMeshGroupReplacements[idxMg++];
			udmMeshGroupReplacement["source"] = pair.first;
			udmMeshGroupReplacement["target"] = pair.second;
		}
	}

	Vector3 min, max;
	GetRenderBounds(min, max);
	udm["render"]["bounds"]["min"] = min;
	udm["render"]["bounds"]["max"] = max;

	auto flags = GetMetaInfo().flags;
	auto writeFlag = [](auto udm, auto flag, const std::string &name, auto flags) {
		if(math::is_flag_set(flags, flag) == false)
			return;
		udm["flags"][name] = true;
	};
	auto writeModelFlag = [&udm, flags, &writeFlag](Flags flag, const std::string &name) { writeFlag(udm, flag, name, flags); };
	writeModelFlag(Flags::Static, "static");
	writeModelFlag(Flags::Inanimate, "inanimate");
	writeModelFlag(Flags::DontPrecacheTextureGroups, "dontPrecacheSkins");
	writeModelFlag(Flags::WorldGeometry, "worldGeometry");
	writeModelFlag(Flags::GeneratedHitboxes, "generatedHitboxes");
	writeModelFlag(Flags::GeneratedLODs, "generatedLODs");
	writeModelFlag(Flags::GeneratedMetaRig, "generatedMetaRig");
	writeModelFlag(Flags::GeneratedMetaBlendShapes, "generatedMetaBlendShapes");
	static_assert(math::to_integral(Flags::Count) == 13, "Update this list when new flags have been added!");

	auto isStatic = math::is_flag_set(flags, Flags::Static);
	if(!isStatic) {
		auto udmSkeleton = udm["skeleton"];
		auto &skeleton = GetSkeleton();
		auto &reference = GetReference();
		auto &skeletonPoses = skeleton.GetBonePoses();
		skeletonPoses.resize(reference.GetBoneTransforms().size());
		for(uint32_t boneId = 0u; auto &pose : reference.GetBoneTransforms()) {
			auto *pscale = reference.GetBoneScale(boneId);
			math::ScaledTransform scaledPose {pose};
			if(pscale)
				scaledPose.SetScale(*pscale);
			skeletonPoses[boneId] = scaledPose;
			++boneId;
		}

		if(skeleton.Save(udm::AssetData {udmSkeleton}, outErr) == false)
			return false;

		auto &attachments = GetAttachments();
		auto udmAttachments = udm.AddArray("attachments", attachments.size());
		for(auto i = decltype(attachments.size()) {0u}; i < attachments.size(); ++i) {
			auto &att = attachments[i];
			auto udmAtt = udmAttachments[i];
			math::Transform transform {att.offset, uquat::create(att.angles)};
			udmAtt["name"] = att.name;
			udmAtt["bone"] = att.bone;
			udmAtt["pose"] = transform;
		}

		auto &objAttachments = GetObjectAttachments();
		auto udmObjAttachments = udm.AddArray("objectAttachments", objAttachments.size());
		for(auto i = decltype(objAttachments.size()) {0u}; i < objAttachments.size(); ++i) {
			auto &objAtt = objAttachments[i];
			auto udmObjAtt = udmObjAttachments[i];
			udmObjAtt["name"] = objAtt.name;
			udmObjAtt["attachment"] = LookupAttachment(objAtt.attachment);
			udmObjAtt["type"] = udm::enum_to_string(objAtt.type);
			udmObjAtt["keyValues"] = objAtt.keyValues;
		}

		auto &hitboxes = GetHitboxes();
		auto udmHitboxes = udm.AddArray("hitboxes", hitboxes.size());
		uint32_t hbIdx = 0;
		for(auto &pair : hitboxes) {
			auto &hb = pair.second;
			auto udmHb = udmHitboxes[hbIdx++];
			udmHb["hitGroup"] = udm::enum_to_string(hb.group);
			udmHb["bounds"]["min"] = hb.min;
			udmHb["bounds"]["max"] = hb.max;
			udmHb["bone"] = pair.first;
		}

		if(m_metaRig) {
			auto udmMetaRig = udm["metaRig"];
			if(!m_metaRig->Save(*m_skeleton, udm::AssetData {udmMetaRig}, outErr))
				return false;
		}
	}

	auto &bodyGroups = GetBodyGroups();
	auto udmBodyGroups = udm.Add("bodyGroups");
	for(auto &bg : bodyGroups) {
		auto udmBodyGroup = udmBodyGroups.Add(bg.name);
		udmBodyGroup["meshGroups"] = bg.meshGroups;
	}

	udm["baseMeshGroups"] = m_baseMeshes;

	auto &texGroups = GetTextureGroups();
	auto udmTexGroups = udm.AddArray("skins", texGroups.size());
	for(auto i = decltype(texGroups.size()) {0u}; i < texGroups.size(); ++i) {
		auto &texGroup = texGroups[i];
		udmTexGroups[i]["materials"] = texGroup.textures;
	}

	auto &colMeshes = GetCollisionMeshes();
	auto udmColMeshes = udm.AddArray("collisionMeshes", colMeshes.size());
	for(auto i = decltype(colMeshes.size()) {0u}; i < colMeshes.size(); ++i) {
		auto &colMesh = colMeshes[i];
		if(colMesh->Save(game, *this, udm::AssetData {udmColMeshes[i]}, outErr) == false)
			return false;
	}

	// Joints
	auto &joints = GetJoints();
	if(!joints.empty()) {
		auto udmJoints = udm.AddArray("joints", joints.size());
		uint32_t jointIdx = 0;
		for(auto &joint : joints) {
			auto udmJoint = udmJoints[jointIdx++];
			udmJoint["type"] = udm::enum_to_string(joint.type);
			udmJoint["parentBone"] = joint.parent;
			udmJoint["childBone"] = joint.child;
			udmJoint["enableCollisions"] = joint.collide;
			udmJoint["args"] = joint.args;
		}
	}

	if(!isStatic) {
		auto &animations = GetAnimations();
		if(!animations.empty()) {
			auto &ref = GetReference();
			auto udmAnimations = udm["animations"];
			for(auto i = decltype(animations.size()) {0u}; i < animations.size(); ++i) {
				auto &anim = animations[i];
				auto animName = GetAnimationName(i);
				if(animName.empty()) {
					uint32_t j = 0;
					do
						animName = "unnamed" + std::to_string(j++);
					while(udmAnimations[animName] || m_animationIDs.find(animName) != m_animationIDs.end());
				}
				auto udmAnim = udmAnimations[animName];
				udmAnim["index"] = static_cast<uint32_t>(i);
				auto enableOptimizations = (animName != "reference"); // Never enable optimization for reference pose
				if(anim->Save(udm::AssetData {udmAnim}, outErr, &ref, enableOptimizations) == false)
					return false;
			}
		}

		auto &blendControllers = GetBlendControllers();
		if(!blendControllers.empty()) {
			auto udmBlendControllers = udm.AddArray("blendControllers", blendControllers.size());
			uint32_t bcIdx = 0;
			for(auto &bc : blendControllers) {
				auto udmBc = udmBlendControllers[bcIdx++];
				udmBc["name"] = bc.name;
				udmBc["min"] = bc.min;
				udmBc["max"] = bc.max;
				udmBc["loop"] = bc.loop;
			}
		}

		auto &ikControllers = GetIKControllers();
		if(!ikControllers.empty()) {
			auto udmIkControllers = udm.AddArray("ikControllers", ikControllers.size());
			uint32_t ikControllerIdx = 0;
			for(auto &ikc : ikControllers) {
				auto udmIkController = udmIkControllers[ikControllerIdx++];
				udmIkController["effectorName"] = ikc->GetEffectorName();
				udmIkController["type"] = ikc->GetType();
				udmIkController["chainLength"] = ikc->GetChainLength();
				udmIkController["method"] = udm::enum_to_string(ikc->GetMethod());
				udmIkController["keyValues"] = ikc->GetKeyValues();
			}
		}

		auto &morphAnims = GetVertexAnimations();
		if(!morphAnims.empty()) {
			auto udmMorphAnims = udm["morphTargetAnimations"];
			for(auto i = decltype(morphAnims.size()) {0u}; i < morphAnims.size(); ++i) {
				auto &va = morphAnims[i];
				auto udmMa = udmMorphAnims[va->GetName()];
				udmMa["index"] = static_cast<uint32_t>(i);
				if(va->Save(*this, udm::AssetData {udmMa}, outErr) == false)
					return false;
			}

			auto &flexes = GetFlexes();
			auto udmFlexes = udm["flexes"];
			for(uint32_t flexIdx = 0u; flexIdx < flexes.size(); ++flexIdx) {
				auto &flex = flexes[flexIdx];
				auto &flexName = flex.GetName();
				if(udmFlexes[flexName]) {
					outErr = "Duplicate flex with name '" + flexName + "'!";
					return false;
				}
				auto udmFlex = udmFlexes[flexName];
				udmFlex["index"] = flexIdx;

				auto *va = flex.GetVertexAnimation();
				if(va != nullptr) {
					auto &vertAnims = GetVertexAnimations();
					auto itVa = std::find(vertAnims.begin(), vertAnims.end(), va->shared_from_this());
					if(itVa != vertAnims.end())
						udmFlex["morphTargetAnimation"] = static_cast<uint32_t>(itVa - vertAnims.begin());
				}
				if(!udmFlex["morphTargetAnimation"])
					udmFlex.Add("morphTargetAnimation", udm::Type::Nil);
				udmFlex["frame"] = flex.GetFrameIndex();

				auto &ops = flex.GetOperations();
				auto udmOps = udmFlex.AddArray("operations", ops.size());
				uint32_t opIdx = 0u;
				for(auto &op : ops) {
					auto udmOp = udmOps[opIdx++];
					udmOp["type"] = udm::enum_to_string(op.type);
					auto valueType = animation::Flex::Operation::GetOperationValueType(op.type);
					switch(valueType) {
					case animation::Flex::Operation::ValueType::Index:
						udmOp["index"] = op.d.index;
						break;
					case animation::Flex::Operation::ValueType::Value:
						udmOp["value"] = op.d.value;
						break;
					case animation::Flex::Operation::ValueType::None:
						break;
					}
				}
			}

			auto &flexControllers = GetFlexControllers();
			auto udmFlexControllers = udm["flexControllers"];
			uint32_t flexCIdx = 0;
			for(auto &flexC : flexControllers) {
				auto udmFlexC = udmFlexControllers[flexC.name];
				udmFlexC["index"] = flexCIdx++;
				udmFlexC["min"] = flexC.min;
				udmFlexC["max"] = flexC.max;
			}

			auto &phonemeMap = GetPhonemeMap();
			auto udmPhonemes = udm["phonemes"];
			for(auto &pairPhoneme : phonemeMap.phonemes)
				udmPhonemes[pairPhoneme.first] = pairPhoneme.second.flexControllers;

			auto &flexAnims = GetFlexAnimations();
			auto &flexAnimNames = GetFlexAnimationNames();
			auto udmFlexAnims = udm["flexAnimations"];
			for(uint32_t i = decltype(flexAnims.size()) {0u}; i < flexAnims.size(); ++i) {
				auto &flexAnim = flexAnims[i];
				auto udmFlexAnim = udmFlexAnims[flexAnimNames[i]];
				udmFlexAnim["index"] = i;
				if(flexAnim->Save(udm::AssetData {udmFlexAnim}, outErr) == false)
					return false;
			}
		}

		auto &eyeballs = GetEyeballs();
		auto udmEyeballs = udm["eyeballs"];
		uint32_t eyeballIdx = 0;
		for(auto &eyeball : eyeballs) {
			std::string name = eyeball.name;
			if(name.empty()) {
				name = "eyeball" + std::to_string(eyeballIdx);
				Con::CWAR << "Eyeball with no name found, assigning name '" << name << "'" << Con::endl;
			}
			auto udmEyeball = udmEyeballs[name];
			udmEyeball["index"] = eyeballIdx++;
			udmEyeball["bone"] = eyeball.boneIndex;
			udmEyeball["origin"] = eyeball.origin;
			udmEyeball["zOffset"] = eyeball.zOffset;
			udmEyeball["radius"] = eyeball.radius;
			udmEyeball["up"] = eyeball.up;
			udmEyeball["forward"] = eyeball.forward;
			udmEyeball["maxDilationFactor"] = eyeball.maxDilationFactor;

			udmEyeball["iris"]["material"] = eyeball.irisMaterialIndex;
			udmEyeball["iris"]["uvRadius"] = eyeball.irisUvRadius;
			udmEyeball["iris"]["scale"] = eyeball.irisScale;

			auto readLid = [](udm::LinkedPropertyWrapperArg prop, Eyeball::LidFlexDesc &lid) {
				prop["raiser"]["lidFlexIndex"](lid.lidFlexIndex);

				prop["raiser"]["raiserFlexIndex"](lid.raiserFlexIndex);
				prop["raiser"]["targetAngle"](lid.raiserValue);
				lid.raiserValue = math::deg_to_rad(lid.raiserValue);

				prop["neutral"]["neutralFlexIndex"](lid.neutralFlexIndex);
				prop["neutral"]["targetAngle"](lid.neutralValue);
				lid.neutralValue = math::deg_to_rad(lid.neutralValue);

				prop["lowerer"]["lowererFlexIndex"](lid.lowererFlexIndex);
				prop["lowerer"]["targetAngle"](lid.lowererValue);
				lid.lowererValue = math::deg_to_rad(lid.lowererValue);
			};
			readLid(udmEyeball["eyelids"]["upperLid"], eyeball.upperLid);
			readLid(udmEyeball["eyelids"]["lowerLid"], eyeball.lowerLid);
		}
	}

	auto &meshGroups = GetMeshGroups();
	auto udmMeshGroups = udm.Add("meshGroups");
	for(auto i = decltype(meshGroups.size()) {0u}; i < meshGroups.size(); ++i) {
		auto &meshGroup = meshGroups[i];
		auto udmMeshGroup = udmMeshGroups[meshGroup->GetName()];
		udmMeshGroup["index"] = static_cast<uint32_t>(i);
		auto &meshes = meshGroup->GetMeshes();
		auto udmMeshes = udmMeshGroup.AddArray("meshes", meshes.size());
		uint32_t meshIdx = 0;
		for(auto &mesh : meshes) {
			auto udmMesh = udmMeshes[meshIdx++];
			udmMesh["referenceId"] = mesh->GetReferenceId();
			auto &subMeshes = mesh->GetSubMeshes();
			auto udmSubMeshes = udmMesh.AddArray("subMeshes", subMeshes.size());
			uint32_t subMeshIdx = 0;
			for(auto &subMesh : subMeshes) {
				if(subMesh->Save(udm::AssetData {udmSubMeshes[subMeshIdx++]}, outErr) == false)
					return false;
			}
		}
	}

	udm["extensions"] = m_extensions;
	return true;
}

bool pragma::asset::Model::SaveLegacy(Game *game, const std::string &name, const std::string &rootPath) const
{
	auto fname = get_normalized_path(name, Type::Model) + '.' + FORMAT_MODEL_LEGACY;
	fname = rootPath + "models\\" + fname;
	fs::create_path(ufile::get_path_from_filename(fname));
	auto f = fs::open_file<fs::VFilePtrReal>(fname, fs::FileMode::Write | fs::FileMode::Binary);
	if(f == nullptr)
		return false;
	auto &mdl = const_cast<Model &>(*this);
	auto &skeleton = mdl.GetSkeleton();
	auto &bones = skeleton.GetBones();
	auto &refPose = mdl.GetReference();
	auto &rootBones = skeleton.GetRootBones();
	auto &attachments = mdl.GetAttachments();
	auto &objectAttachments = mdl.GetObjectAttachments();
	auto &hitboxes = mdl.GetHitboxes();
	auto &meta = mdl.GetMetaInfo();
	auto flags = meta.flags;
	auto bStatic = math::is_flag_set(flags, Flags::Static);

	auto &texturePaths = meta.texturePaths;
	assert(texturePaths.size() <= std::numeric_limits<uint8_t>::max());

	f->Write("WMD", 3);
	f->Write<uint16_t>(38 /* version */);
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
	//auto bStatic = pragma::math::is_flag_set(flags,Model::Flags::Static);
	if(!bStatic) {
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

	write_offset(f, offIndex + INDEX_OFFSET_MODEL_DATA * INDEX_OFFSET_INDEX_SIZE); // Model Data
	f->Write<uint8_t>(static_cast<uint8_t>(texturePaths.size()));
	for(auto &path : texturePaths)
		f->WriteString(path);

	if(!bStatic) {
		std::function<void(fs::VFilePtrReal &, animation::Bone &)> fWriteChildBones;
		fWriteChildBones = [&fWriteChildBones](fs::VFilePtrReal &f, animation::Bone &bone) {
			auto &children = bone.children;
			f->Write<uint32_t>(static_cast<uint32_t>(children.size()));
			for(auto &pair : children) {
				f->Write<uint32_t>(pair.first);
				fWriteChildBones(f, *pair.second);
			}
		};

		write_offset(f, offIndex + INDEX_OFFSET_BONES * INDEX_OFFSET_INDEX_SIZE); // Bones
		f->Write<uint32_t>(static_cast<uint32_t>(bones.size()));
		for(auto &bone : bones) {
			std::string name = bone->name;
			f->WriteString(name);
		}
		for(auto &bone : bones) {
			auto *pos = refPose.GetBonePosition(bone->ID);
			auto *rot = refPose.GetBoneOrientation(bone->ID);
			f->Write<Quat>((rot != nullptr) ? *rot : uquat::identity());
			f->Write<Vector3>((pos != nullptr) ? *pos : Vector3 {});
		}

		f->Write<uint32_t>(static_cast<uint32_t>(rootBones.size()));
		for(auto &pair : rootBones) {
			f->Write<uint32_t>(pair.first);
			auto &bone = *pair.second;

			fWriteChildBones(f, bone);
		}

		f->Write<uint32_t>(static_cast<uint32_t>(attachments.size()));
		for(auto &att : attachments) {
			f->WriteString(att.name);
			f->Write<uint32_t>(att.bone);
			f->Write<Vector3>(att.offset);
			f->Write<EulerAngles>(att.angles);
		}

		f->Write<uint32_t>(static_cast<uint32_t>(objectAttachments.size()));
		for(auto &objAtt : objectAttachments) {
			f->Write<ObjectAttachment::Type>(objAtt.type);
			f->WriteString(objAtt.name);
			f->WriteString(objAtt.attachment);
			f->Write<uint32_t>(objAtt.keyValues.size());
			for(auto &pair : objAtt.keyValues) {
				f->WriteString(pair.first);
				f->WriteString(pair.second);
			}
		}

		f->Write<uint32_t>(static_cast<uint32_t>(hitboxes.size()));
		for(auto &pair : hitboxes) {
			auto &hb = pair.second;
			f->Write<uint32_t>(pair.first);
			f->Write<uint32_t>(math::to_integral(hb.group));
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
	for(auto &skin : skins) {
		assert(skin.textures.size() == skins.front().textures.size());
		if(skin.textures.size() != skins.front().textures.size()) {
			f = nullptr;
			fs::remove_file(fname);
			throw std::logic_error("All skins have to contain same number of textures.");
		}
		for(auto &texId : skin.textures)
			f->Write<uint16_t>(static_cast<uint16_t>(texId));
	}

	Vector3 min, max;
	mdl.GetRenderBounds(min, max);
	f->Write<Vector3>(min);
	f->Write<Vector3>(max);

	auto &meshGroups = mdl.GetMeshGroups();
	write_offset(f, offIndex + INDEX_OFFSET_MODEL_MESHES * INDEX_OFFSET_INDEX_SIZE); // Meshes
	f->Write<uint32_t>(static_cast<uint32_t>(meshGroups.size()));
	for(auto &group : meshGroups) {
		f->WriteString(group->GetName());
		auto &meshes = group->GetMeshes();
		f->Write<uint32_t>(static_cast<uint32_t>(meshes.size()));
		for(auto &mesh : meshes) {
			f->Write<uint32_t>(mesh->GetReferenceId());

			auto &subMeshes = mesh->GetSubMeshes();
			f->Write<uint32_t>(static_cast<uint32_t>(subMeshes.size()));
			for(auto &subMesh : subMeshes) {
				// Version 26
				f->Write<math::ScaledTransform>(subMesh->GetPose());
				//

				f->Write<uint16_t>(static_cast<uint16_t>(subMesh->GetSkinTextureIndex()));

				// Version 27
				f->Write<geometry::ModelSubMesh::GeometryType>(subMesh->GetGeometryType());
				//
				f->Write<uint32_t>(subMesh->GetReferenceId());

				auto &verts = subMesh->GetVertices();
				auto numVerts = verts.size();
				f->Write<uint64_t>(numVerts);
				for(auto &v : verts) {
					f->Write<Vector3>(v.position);
					f->Write<Vector3>(v.normal);
				}

				auto &uvSets = subMesh->GetUVSets();
				// Version 30
				auto numUvSets = math::min(uvSets.size() + 1, static_cast<size_t>(std::numeric_limits<uint8_t>::max()));
				f->Write<uint8_t>(numUvSets);
				f->WriteString(std::string {"base"});
				for(auto &v : verts)
					f->Write<Vector2>(v.uv);
				auto uvSetIdx = 1;
				for(auto &pair : uvSets) {
					f->WriteString(pair.first);
					auto &uvSet = pair.second;
					for(auto i = decltype(numVerts) {0u}; i < numVerts; ++i) {
						auto uv = (i < uvSet.size()) ? uvSet.at(i) : Vector2 {};
						f->Write<Vector2>(uv);
					}
					if(++uvSetIdx == numUvSets)
						break;
				}
				//

				auto &boneWeights = subMesh->GetVertexWeights();
				f->Write<uint64_t>(boneWeights.size());
				static_assert(sizeof(decltype(boneWeights.front())) == sizeof(Vector4) * 2);
				f->Write(boneWeights.data(), boneWeights.size() * sizeof(decltype(boneWeights.front())));

				// Version 27
				auto &extBoneWeights = subMesh->GetExtendedVertexWeights();
				f->Write<uint64_t>(extBoneWeights.size());
				static_assert(sizeof(decltype(extBoneWeights.front())) == sizeof(Vector4) * 2);
				f->Write(extBoneWeights.data(), extBoneWeights.size() * sizeof(decltype(extBoneWeights.front())));
				//

				// Version 30
				auto numAlphas = subMesh->GetAlphaCount();
				f->Write<uint8_t>(numAlphas);
				if(numAlphas > 0) {
					auto &alphas = subMesh->GetAlphas();
					for(auto i = decltype(numVerts) {0u}; i < numVerts; ++i) {
						auto a = (i < alphas.size()) ? alphas.at(i) : Vector2 {};
						f->Write<float>(a.x);
						if(numAlphas > 1)
							f->Write<float>(a.y);
					}
				}
				//

				if(subMesh->GetIndexType() != geometry::IndexType::UInt16)
					return false;
				auto &indexData = subMesh->GetIndexData();
				f->Write<uint32_t>(subMesh->GetIndexCount());
				f->Write(indexData.data(), indexData.size());
			}
		}
	}

	auto &baseMeshes = mdl.GetBaseMeshes();

	// Version 0x0004
	f->Write<uint16_t>(static_cast<uint16_t>(baseMeshes.size()));
	for(auto &meshId : baseMeshes)
		f->Write<uint32_t>(meshId);
	//

	write_offset(f, offIndex + INDEX_OFFSET_LOD_DATA * INDEX_OFFSET_INDEX_SIZE); // LOD data
	// Version 0x0004
	auto numLods = mdl.GetLODCount();

	auto &lodInfos = GetLODs();
	f->Write<uint8_t>(static_cast<uint8_t>(lodInfos.size()));
	for(auto &lodInfo : lodInfos) {
		f->Write<uint8_t>(static_cast<uint8_t>(lodInfo.lod));
		f->Write<float>(lodInfo.distance);
		f->Write<uint8_t>(static_cast<uint8_t>(lodInfo.meshReplacements.size()));
		for(auto &pair : lodInfo.meshReplacements) {
			f->Write<uint32_t>(pair.first);
			f->Write<uint32_t>(pair.second);
		}
	}
	//

	// Version 0x0004
	write_offset(f, offIndex + INDEX_OFFSET_BODYGROUPS * INDEX_OFFSET_INDEX_SIZE); // Bodygroup Data
	auto &bodyGroups = mdl.GetBodyGroups();
	f->Write<uint16_t>(static_cast<uint16_t>(bodyGroups.size()));
	for(auto &bg : bodyGroups) {
		f->WriteString(bg.name);
		f->Write<uint8_t>(static_cast<uint8_t>(bg.meshGroups.size()));
		for(auto &group : bg.meshGroups)
			f->Write<uint32_t>(group);
	}
	//

	// Version 38
	write_offset(f, offIndex + INDEX_OFFSET_JOINTS * INDEX_OFFSET_INDEX_SIZE);
	auto &joints = mdl.GetJoints();
	f->Write<uint32_t>(joints.size());
	for(auto &joint : joints) {
		f->Write<physics::JointType>(joint.type);
		f->Write<animation::BoneId>(joint.child);
		f->Write<animation::BoneId>(joint.parent);
		f->Write<bool>(joint.collide);
		f->Write<uint8_t>(joint.args.size());
		for(auto &pair : joint.args) {
			f->WriteString(pair.first);
			f->WriteString(pair.second);
		}
	}
	//

	auto &collisionMeshes = mdl.GetCollisionMeshes();
	if(!collisionMeshes.empty()) {
		write_offset(f, offIndex + INDEX_OFFSET_COLLISION_MESHES * INDEX_OFFSET_INDEX_SIZE);
		f->Write<float>(mdl.GetMass());
		f->Write<uint32_t>(collisionMeshes.size());

		auto *surfaceMaterials = game->GetSurfaceMaterials();
		for(auto i = decltype(collisionMeshes.size()) {0}; i < collisionMeshes.size(); ++i) {
			auto &mesh = collisionMeshes[i];
			uint64_t flags = 0;
			if(mesh->IsConvex())
				flags |= 2;

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
			geometry::ModelSubMesh *subMesh = nullptr;
			if(bSoftBody == true) {
				auto bFound = false;
				for(auto i = decltype(meshGroups.size()) {0}; i < meshGroups.size(); ++i) {
					auto &group = meshGroups.at(i);
					auto &meshes = group->GetMeshes();
					for(auto j = decltype(meshes.size()) {0}; j < meshes.size(); ++j) {
						auto &mesh = meshes.at(j);
						auto &subMeshes = mesh->GetSubMeshes();
						for(auto k = decltype(subMeshes.size()) {0}; k < subMeshes.size(); ++k) {
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
				flags |= 1;

			f->Write<uint64_t>(flags);
			f->Write<int32_t>(mesh->GetBoneParent());
			// Version 0x0002
			f->Write<Vector3>(mesh->GetOrigin());
			//
			// Version 0x0003
			if(surfaceMaterials) {
				auto &matSurface = (*surfaceMaterials)[mesh->GetSurfaceMaterial()];
				f->WriteString(matSurface.GetIdentifier());
			}
			else
				f->WriteString("");
			//
			Vector3 min, max;
			mesh->GetAABB(&min, &max);

			f->Write<Vector3>(min);
			f->Write<Vector3>(max);
			auto &verts = mesh->GetVertices();
			f->Write<uint64_t>(verts.size());
			for(auto &v : verts)
				f->Write<Vector3>(v);

			// Version 0x0010
			auto &triangles = mesh->GetTriangles();
			f->Write<uint64_t>(triangles.size());
			f->Write(triangles.data(), sizeof(triangles.front()) * triangles.size());
			//

			// Version 0x0011
			f->Write<double>(mesh->GetVolume());
			f->Write<Vector3>(mesh->GetCenterOfMass());
			//

			// Version 0x0014
			if(bSoftBody == true) {
				f->Write<uint32_t>(meshGroupId);
				f->Write<uint32_t>(meshId);
				f->Write<uint32_t>(subMeshId);
				f->Write(sbInfo, sizeof(float) * 19 + sizeof(uint32_t) * 2);
				f->Write<uint32_t>(sbInfo->materialStiffnessCoefficient.size());
				for(auto &pair : sbInfo->materialStiffnessCoefficient) {
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
					f->Write<physics::CollisionMesh::SoftBodyAnchor>(anchor);
			}
			//
		}
	}
	if(bStatic == false) {
		auto &blendControllers = mdl.GetBlendControllers();
		f->Write<uint16_t>(static_cast<uint16_t>(blendControllers.size()));
		for(auto &bc : blendControllers) {
			f->WriteString(bc.name);
			f->Write<int32_t>(bc.min);
			f->Write<int32_t>(bc.max);
			f->Write<bool>(bc.loop);
		}

		// Version 0x0016
		write_offset(f, offIndex + INDEX_OFFSET_IK_CONTROLLERS * INDEX_OFFSET_INDEX_SIZE); // IK Controllers
		auto &ikControllers = mdl.GetIKControllers();
		f->Write<uint32_t>(ikControllers.size());
		for(auto &ikController : ikControllers) {
			f->WriteString(ikController->GetEffectorName());
			f->WriteString(ikController->GetType());
			f->Write<uint32_t>(ikController->GetChainLength());
			f->Write<uint32_t>(math::to_integral(ikController->GetMethod()));

			auto &keyValues = ikController->GetKeyValues();
			f->Write<uint32_t>(keyValues.size());
			for(auto &pair : keyValues) {
				f->WriteString(pair.first);
				f->WriteString(pair.second);
			}
		}
		//

		write_offset(f, offIndex + INDEX_OFFSET_ANIMATIONS * INDEX_OFFSET_INDEX_SIZE); // Animations
		auto &anims = mdl.GetAnimations();
		f->Write<uint32_t>(static_cast<uint32_t>(anims.size()));
		for(auto i = decltype(anims.size()) {0}; i < anims.size(); ++i) {
			auto &anim = anims[i];
			f->WriteString(mdl.GetAnimationName(static_cast<uint32_t>(i)));
			anim->SaveLegacy(f);
		}

		// Version 0x0015
		write_offset(f, offIndex + INDEX_OFFSET_VERTEX_ANIMATIONS * INDEX_OFFSET_INDEX_SIZE); // Vertex animations
		auto &vertexAnims = GetVertexAnimations();
		auto &meshGroups = GetMeshGroups();
		f->Write<uint32_t>(vertexAnims.size());
		for(auto &va : vertexAnims) {
			f->WriteString(va->GetName());
			auto &meshAnims = va->GetMeshAnimations();

			auto offsetMeshAnimCount = f->Tell();
			auto meshAnimCount = 0u;
			f->Write<uint32_t>(meshAnims.size());
			for(auto &anim : meshAnims) {
				auto *mesh = anim->GetMesh();
				auto *subMesh = anim->GetSubMesh();
				if(mesh == nullptr || subMesh == nullptr)
					continue;
				auto meshGroupId = std::numeric_limits<uint32_t>::max();
				auto meshId = std::numeric_limits<uint32_t>::max();
				auto subMeshId = std::numeric_limits<uint32_t>::max();
				for(auto itMeshGroup = meshGroups.begin(); itMeshGroup != meshGroups.end(); ++itMeshGroup) {
					auto &meshGroup = *itMeshGroup;
					auto &meshes = meshGroup->GetMeshes();
					auto itMesh = std::find_if(meshes.begin(), meshes.end(), [mesh](const std::shared_ptr<geometry::ModelMesh> &other) { return (other.get() == mesh) ? true : false; });
					if(itMesh == meshes.end())
						continue;
					auto &mesh = *itMesh;
					auto &subMeshes = mesh->GetSubMeshes();
					meshGroupId = itMeshGroup - meshGroups.begin();
					meshId = itMesh - meshes.begin();
					auto itSubMesh = std::find_if(subMeshes.begin(), subMeshes.end(), [subMesh](const std::shared_ptr<geometry::ModelSubMesh> &other) { return (other.get() == subMesh) ? true : false; });
					if(itSubMesh == subMeshes.end())
						break;
					subMeshId = itSubMesh - subMeshes.begin();
					break;
				}
				if(subMeshId == std::numeric_limits<uint32_t>::max()) {
					Con::CWAR << "Invalid mesh reference in vertex animation '" << va->GetName() << "'! Skipping..." << Con::endl;
					continue;
				}
				++meshAnimCount;
				f->Write<uint32_t>(meshGroupId);
				f->Write<uint32_t>(meshId);
				f->Write<uint32_t>(subMeshId);

				auto &frames = anim->GetFrames();
				f->Write<uint32_t>(frames.size());
				for(auto &frame : frames) {
					auto flags = frame->GetFlags();
					auto offsetToEndOfFrameOffset = f->Tell();
					f->Write<uint64_t>(0ull);
					f->Write<animation::MeshVertexFrame::Flags>(flags);

					struct Attribute {
						Attribute(const std::string &name, const std::vector<std::array<uint16_t, 4>> &vertexData) : name {name}, vertexData {vertexData} {}
						std::string name;
						const std::vector<std::array<uint16_t, 4>> &vertexData;
					};
					std::vector<Attribute> attributes {};
					attributes.push_back({"position", frame->GetVertices()});
					if(math::is_flag_set(flags, animation::MeshVertexFrame::Flags::HasNormals))
						attributes.push_back({"normal", frame->GetNormals()});
					std::set<uint16_t> usedVertIndices {};
					for(auto &attr : attributes) {
						auto vertIdx = 0u;
						auto &vdata = attr.vertexData.at(vertIdx);
						for(auto &vdata : attr.vertexData) {
							auto itUsed = std::find_if(vdata.begin(), vdata.end(), [](const uint16_t &v) { return v != 0; });
							if(itUsed != vdata.end())
								usedVertIndices.insert(vertIdx);
							++vertIdx;
						}
					}

					f->Write<uint16_t>(usedVertIndices.size());
					for(auto idx : usedVertIndices)
						f->Write<uint16_t>(idx);
					f->Write<uint16_t>(attributes.size());
					for(auto &attr : attributes) {
						f->WriteString(attr.name);
						for(auto idx : usedVertIndices)
							f->Write<std::array<uint16_t, 4>>(attr.vertexData.at(idx));
					}
					write_offset(f, offsetToEndOfFrameOffset);
				}
			}

			// Write actual frame count
			auto cur = f->Tell();
			f->Seek(offsetMeshAnimCount);
			f->Write<uint32_t>(meshAnimCount);
			f->Seek(cur);
		}

		write_offset(f, offIndex + INDEX_OFFSET_FLEX_CONTROLLERS * INDEX_OFFSET_INDEX_SIZE); // Flex controllers
		auto &flexControllers = GetFlexControllers();
		f->Write<uint32_t>(flexControllers.size());
		for(auto &fc : flexControllers) {
			f->WriteString(fc.name);
			f->Write<float>(fc.min);
			f->Write<float>(fc.max);
		}

		write_offset(f, offIndex + INDEX_OFFSET_FLEXES * INDEX_OFFSET_INDEX_SIZE); // Flexes
		auto &flexes = GetFlexes();
		f->Write<uint32_t>(flexes.size());
		for(auto &flex : flexes) {
			f->WriteString(flex.GetName());

			auto *va = flex.GetVertexAnimation();
			if(va != nullptr) {
				auto &vertAnims = GetVertexAnimations();
				auto itVa = std::find(vertAnims.begin(), vertAnims.end(), va->shared_from_this());
				if(itVa == vertAnims.end())
					va = nullptr;
				else
					f->Write<uint32_t>(itVa - vertAnims.begin());
			}
			else
				f->Write<uint32_t>(std::numeric_limits<uint32_t>::max());
			f->Write<uint32_t>(flex.GetFrameIndex());

			auto &ops = flex.GetOperations();
			f->Write<uint32_t>(ops.size());
			for(auto &op : ops) {
				f->Write<uint32_t>(math::to_integral(op.type));
				f->Write<float>(op.d.value);
			}
		}

		write_offset(f, offIndex + INDEX_OFFSET_PHONEMES * INDEX_OFFSET_INDEX_SIZE); // Phonemes
		auto &phonemeMap = GetPhonemeMap();
		f->Write<uint32_t>(phonemeMap.phonemes.size());
		for(auto &pair : phonemeMap.phonemes) {
			f->WriteString(pair.first);
			auto numControllers = pair.second.flexControllers.size();
			auto offsetNumControllers = f->Tell();
			f->Write<uint32_t>(numControllers);
			for(auto &pair : pair.second.flexControllers) {
				auto id = 0u;
				if(GetFlexControllerId(pair.first, id) == false) {
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
		write_offset(f, offIndex + INDEX_OFFSET_EYEBALLS * INDEX_OFFSET_INDEX_SIZE);

		f->Write(GetMaxEyeDeflection());
		auto &eyeballs = GetEyeballs();
		f->Write<uint32_t>(eyeballs.size());
		for(auto &eyeball : eyeballs) {
			f->WriteString(eyeball.name);
			f->Write(reinterpret_cast<const uint8_t *>(&eyeball) + sizeof(std::string), sizeof(Eyeball) - sizeof(std::string));
		}
		//

		// Flex animations
		write_offset(f, offIndex + INDEX_OFFSET_FLEX_ANIMATIONS * INDEX_OFFSET_INDEX_SIZE);
		auto &flexAnims = mdl.GetFlexAnimations();
		auto &flexAnimNames = mdl.GetFlexAnimationNames();
		f->Write<uint32_t>(static_cast<uint32_t>(flexAnims.size()));
		for(auto i = decltype(flexAnims.size()) {0}; i < flexAnims.size(); ++i) {
			auto &anim = flexAnims[i];
			auto &animName = flexAnimNames[i];
			f->WriteString(animName);
			anim->SaveLegacy(f);
		}
		//
	}

	auto &includes = meta.includes;
	f->Write<uint8_t>(static_cast<uint8_t>(includes.size()));
	for(auto &inc : includes)
		f->WriteString(inc);
	return true;
}
