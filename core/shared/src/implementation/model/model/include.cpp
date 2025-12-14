// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :model.model;

static void subtract_frame(Frame &frame, const Frame &frameToSubtract)
{
	auto numBones = frameToSubtract.GetBoneCount(); // TODO
	for(auto i = decltype(numBones) {0u}; i < numBones; ++i) {
		auto &pos = *frame.GetBonePosition(i);
		pos -= *frameToSubtract.GetBonePosition(i);
		auto inv = uquat::get_inverse(*frameToSubtract.GetBoneOrientation(i));
		uvec::rotate(&pos, inv);
		auto &rot = *frame.GetBoneOrientation(i);
		rot = inv * rot;
	}
}

static void add_frame(Frame &frame, const Frame &frameToAdd)
{
	auto numBones = frame.GetBoneCount(); // TODO
	for(auto i = decltype(numBones) {0u}; i < numBones; ++i) {
		auto &rotAdd = *frameToAdd.GetBoneOrientation(i);
		auto &rot = *frame.GetBoneOrientation(i);
		rot = rotAdd * rot;

		auto &pos = *frame.GetBonePosition(i);
		uvec::rotate(&pos, rotAdd);
		pos += *frameToAdd.GetBonePosition(i);
	}
}

void pragma::asset::Model::Merge(const Model &other, MergeFlags flags)
{
	spdlog::info("Merging model '{}' into '{}'...", other.GetName(), GetName());
	std::vector<std::size_t> boneTranslations; // 'other' bone Id to 'this' bone Id
	auto bMerged = false;
	const auto fMergeSkeletons = [this, &other, &boneTranslations, &bMerged]() {
		if(bMerged == true)
			return;
		bMerged = true;
		auto &skeletonOther = other.GetSkeleton();
		auto &bonesOther = skeletonOther.GetBones();
		auto &skeleton = GetSkeleton();
		auto &bones = skeleton.GetBones();
		bones.reserve(bones.size() + bonesOther.size());
		boneTranslations.reserve(bones.size() + bonesOther.size());

		std::vector<uint32_t> newBoneIndicesToOtherBoneIndices {};
		newBoneIndicesToOtherBoneIndices.reserve(bonesOther.size());
		uint32_t otherBoneIndex = 0u;
		auto numOldBones = skeleton.GetBoneCount();
		for(auto &boneOther : bonesOther) {
			auto it = std::find_if(bones.begin(), bones.end(), [&boneOther](const std::shared_ptr<animation::Bone> &bone) { return (bone->name == boneOther->name) ? true : false; });
			if(it == bones.end()) {
				auto boneId = skeleton.AddBone(new animation::Bone());
				newBoneIndicesToOtherBoneIndices.push_back(otherBoneIndex);
				auto bone = skeleton.GetBone(boneId).lock();
				bone->name = boneOther->name;
				it = bones.end() - 1;
			}
			boneTranslations.push_back(it - bones.begin());
			++otherBoneIndex;
		}
		// Determine new bone parents (Has to be done AFTER all bones have been processed!)
		for(auto idxOther = decltype(boneTranslations.size()) {0}; idxOther < boneTranslations.size(); ++idxOther) {
			auto &boneOther = bonesOther.at(idxOther);
			if(boneOther->parent.expired() == true)
				continue;
			auto idxThis = boneTranslations.at(idxOther);
			auto &boneThis = bones.at(idxThis);
			auto &parent = bones.at(boneTranslations.at(boneOther->parent.lock()->ID));
			boneThis->parent = parent;
			parent->children.insert(std::make_pair(idxThis, boneThis));
		}

		// Copy reference pose transforms of new bones from other model to this model
		auto numNewBones = skeleton.GetBoneCount();
		auto &reference = GetReference();
		if(numNewBones > numOldBones) {
			auto diffBoneCount = numNewBones - numOldBones;
			reference.SetBoneCount(numNewBones);
			auto &referenceOther = other.GetReference();
			for(auto i = decltype(newBoneIndicesToOtherBoneIndices.size()) {0u}; i < newBoneIndicesToOtherBoneIndices.size(); ++i) {
				auto boneIdx = numOldBones + i;
				auto otherBoneIdx = newBoneIndicesToOtherBoneIndices.at(i);
				math::ScaledTransform t;
				if(referenceOther.GetBonePose(otherBoneIdx, t))
					reference.SetBonePose(boneIdx, t);
			}

			// Do the same for the reference animation
			auto animReference = GetAnimation(LookupAnimation("reference"));
			auto animReferenceOther = const_cast<Model &>(other).GetAnimation(other.LookupAnimation("reference"));
			if(animReference && animReference->GetFrameCount() == 1 && animReferenceOther && animReferenceOther->GetFrameCount() == 1) {
				auto frameRef = animReference->GetFrame(0);
				auto frameRefOther = animReferenceOther->GetFrame(0);
				animReference->ReserveBoneIds(numNewBones);
				for(auto i = decltype(diffBoneCount) {0u}; i < diffBoneCount; ++i)
					animReference->AddBoneId(numOldBones + i);
				frameRef->SetBoneCount(numNewBones);
				for(auto i = decltype(newBoneIndicesToOtherBoneIndices.size()) {0u}; i < newBoneIndicesToOtherBoneIndices.size(); ++i) {
					auto boneIdx = numOldBones + i;
					auto otherBoneIdx = newBoneIndicesToOtherBoneIndices.at(i);
					math::ScaledTransform t;
					if(frameRefOther->GetBonePose(otherBoneIdx, t))
						frameRef->SetBonePose(boneIdx, t);
				}
			}
		}
	};

	if((flags & MergeFlags::BlendControllers) != MergeFlags::None) {
		auto &blendControllersOther = other.GetBlendControllers();
		auto &blendControllers = GetBlendControllers();
		blendControllers.reserve(blendControllers.size() + blendControllersOther.size());
		for(auto &blendController : blendControllersOther) {
			auto it = std::find_if(blendControllers.begin(), blendControllers.end(), [&blendController](const BlendController &bc) { return string::compare(bc.name, blendController.name, false); });
			if(it != blendControllers.end())
				continue;
			blendControllers.push_back(blendController);
		}
	}

	auto &reference = GetReference();
	auto &skeleton = GetSkeleton();
	auto refAnim = GetAnimation(LookupAnimation("reference"));
	if(refAnim) {
		auto newRefAnimFrame = Frame::Create(reference);
		newRefAnimFrame->Localize(skeleton);
		std::vector<uint16_t> boneList {};
		auto numBones = skeleton.GetBoneCount();
		boneList.reserve(numBones);
		for(auto i = decltype(numBones) {0u}; i < numBones; ++i)
			boneList.push_back(i);
		refAnim->SetBoneList(boneList);
		refAnim->GetFrames().clear();
		refAnim->AddFrame(newRefAnimFrame);
	}
	auto refFrame = refAnim ? refAnim->GetFrame(0) : nullptr;
	auto &bones = skeleton.GetBones();
	if((flags & MergeFlags::Animations) != MergeFlags::None) {
		fMergeSkeletons();
		auto &anims = GetAnimations();
		auto &animsOther = other.GetAnimations();
		anims.reserve(anims.size() + animsOther.size());
		auto animOffset = anims.size();
		for(auto i = decltype(animsOther.size()) {0}; i < animsOther.size(); ++i) {
			auto &animOther = animsOther.at(i);
			auto animName = other.GetAnimationName(i);

			auto shareMode = animation::Animation::ShareMode::Events;
			if(other.GetSkeleton().GetBoneCount() >= skeleton.GetBoneCount()) // TODO: Check this properly! The included model has to have all bones of this model (it may also have more, but not fewer!)
			{
				// If the number of bones of the included model is less than this model, we have to make a copy of all animation frames and fill up the missing bones - Otherwise there may be animation issues!
				// TODO: Make missing bones always use transformations of reference animation, then this won't be necessary anymore!
				shareMode |= animation::Animation::ShareMode::Frames;
			}
			auto anim = animation::Animation::Create(*animOther, shareMode);
			auto &boneList = anim->GetBoneList();
			for(auto idx = decltype(boneList.size()) {0}; idx < boneList.size(); ++idx)
				anim->SetBoneId(idx, boneTranslations.at(boneList.at(idx)));

			if(anim->HasFlag(FAnim::Autoplay))
				anim->SetFlags(anim->GetFlags() & ~FAnim::Autoplay); // TODO: Autoplay gesture animations cause issues in some cases (e.g. gman.wmd). Re-enable this once the issues have been taken care of!
			auto it = m_animationIDs.find(animName);
			if(it != m_animationIDs.end())
				; //anims.at(it->second) = anim;
			else {
				anims.push_back(anim);
				m_animationIDs.insert(decltype(m_animationIDs)::value_type(animName, anims.size() - 1));

				if(refFrame != nullptr && anim->HasFlag(FAnim::Gesture) == false) {
					auto boneList = anim->GetBoneList();
					auto numBones = skeleton.GetBoneCount();
					boneList.reserve(numBones);
					for(auto i = boneList.size(); i < numBones; ++i)
						boneList.push_back(i);
					anim->SetBoneList(boneList);

					// Fill up all bone transforms for this animation that
					// might be missing but needed for the reference pose.
					for(auto &frame : anim->GetFrames()) {
						auto numBonesFrame = frame->GetBoneCount();
						frame->SetBoneCount(numBones);
						for(auto boneId = numBonesFrame; boneId < (numBones - 1); ++boneId) {
							auto &bonePos = *refFrame->GetBonePosition(boneId);
							auto &boneRot = *refFrame->GetBoneOrientation(boneId);
							auto *boneScale = refFrame->GetBoneScale(boneId);
							frame->SetBonePosition(boneId, bonePos);
							frame->SetBoneOrientation(boneId, boneRot);
							if(boneScale != nullptr)
								frame->SetBoneScale(boneId, *boneScale);
						}
					}
				}
			}
		}

		// Update blend controllers
		std::unordered_map<std::string, uint32_t> *animIds = nullptr;
		GetAnimations(&animIds);
		for(auto i = animOffset; i < anims.size(); ++i) {
			auto &anim = anims.at(i);
			auto *animBc = anim->GetBlendController();
			if(animBc) {
				auto *bc = other.GetBlendController(animBc->controller);
				if(bc == nullptr) {
					anim->ClearBlendController();
					spdlog::warn("Animation {} of model {} has invalid blend controller! Skipping...", GetAnimationName(i), other.GetName());
				}
				else {
					auto bcId = LookupBlendController(bc->name);
					if(bcId != -1) {
						animBc->controller = bcId;
						for(auto &transition : animBc->transitions) {
							auto animName = other.GetAnimationName(transition.animation);
							auto it = animIds->find(animName);
							if(it != animIds->end())
								transition.animation = it->second;
							else {
								spdlog::warn("Blend controller '{}' of animation '{}' of model '{}' has invalid animation transition reference! Skipping...", bc->name, GetAnimationName(i), other.GetName());
								transition.animation = -1;
							}
						}
					}
					else
						spdlog::warn("Blend controller '{}' not found in model '{}'! Skipping...", bc->name, GetName());
				}
			}
		}
	}

	if((flags & MergeFlags::Attachments) != MergeFlags::None) {
		fMergeSkeletons();
		auto &skeleton = GetSkeleton();
		auto &attachments = GetAttachments();
		auto &attachmentsOther = other.GetAttachments();
		auto &skeletonOther = other.GetSkeleton();
		auto &bonesOther = skeletonOther.GetBones();
		attachments.reserve(attachments.size() + attachmentsOther.size());
		for(auto &attOther : attachmentsOther) {
			auto it = std::find_if(attachments.begin(), attachments.end(), [&attOther](const Attachment &att) { return (att.name == attOther.name) ? true : false; });
			if(it != attachments.end()) {
				auto &att = *it;
				att.angles = attOther.angles;
				att.offset = attOther.offset;
				att.bone = boneTranslations.at(attOther.bone);
				continue;
			}
			AddAttachment(attOther.name, boneTranslations.at(attOther.bone), attOther.offset, attOther.angles);
		}
	}
	if((flags & MergeFlags::Hitboxes) != MergeFlags::None) {
		fMergeSkeletons();
		auto &hitboxesOther = other.GetHitboxes();
		auto &hitboxes = GetHitboxes();
		hitboxes.reserve(hitboxes.size() + hitboxesOther.size());
		for(auto &pair : hitboxesOther) {
			auto &hitboxOther = pair.second;
			auto boneId = boneTranslations.at(pair.first);
			physics::Hitbox hb(hitboxOther.group, hitboxOther.min, hitboxOther.max);
			auto it = hitboxes.find(boneId);
			if(it != hitboxes.end())
				; // it->second = hb;
			else
				hitboxes.insert(std::make_pair(boneId, hb));
		}
	}
	if((flags & MergeFlags::Joints) != MergeFlags::None) {
		fMergeSkeletons();
		auto &jointsOther = other.GetJoints();
		auto &joints = GetJoints();
		joints.reserve(joints.size() + jointsOther.size());
		for(auto &jointOther : jointsOther) {
			joints.push_back(physics::JointInfo(jointOther.type, boneTranslations.at(jointOther.parent), boneTranslations.at(jointOther.child)));
			auto &joint = joints.back();
			joint.args = jointOther.args;
			joint.collide = jointOther.collide;
		}
	}
	if((flags & MergeFlags::CollisionMeshes) != MergeFlags::None) {
		fMergeSkeletons();
		auto &collisionMeshesOther = other.GetCollisionMeshes();
		auto &collisionMeshes = GetCollisionMeshes();
		collisionMeshes.reserve(collisionMeshes.size() + collisionMeshesOther.size());
		for(auto &colMeshOther : collisionMeshesOther) {
			auto colMesh = physics::CollisionMesh::Create(*colMeshOther);
			auto boneParent = colMesh->GetBoneParent();
			if(boneParent >= 0)
				boneParent = boneTranslations.at(boneParent);
			colMesh->SetBoneParent(boneParent);
			collisionMeshes.push_back(colMesh);
		}
	}
	if((flags & MergeFlags::Meshes) != MergeFlags::None) {
		fMergeSkeletons();

		auto &mats = GetMaterials();
		auto &matsOther = other.GetMaterials();
		mats.reserve(mats.size() + matsOther.size());

		auto &matPaths = m_metaInfo.texturePaths;
		auto &matPathsOther = other.m_metaInfo.texturePaths;
		matPaths.reserve(matPaths.size() + matPathsOther.size());
		for(auto &path : matPathsOther)
			AddTexturePath(path);

		auto &textures = m_metaInfo.textures;
		auto &texturesOther = other.m_metaInfo.textures;
		textures.reserve(textures.size() + texturesOther.size());
		std::unordered_map<uint32_t, uint32_t> otherMatIdxToThisMatIdx {};
		for(auto i = decltype(texturesOther.size()) {0u}; i < texturesOther.size(); ++i) {
			auto *matOther = (i < other.m_materials.size()) ? const_cast<material::Material *>(other.m_materials.at(i).get()) : nullptr;
			auto idx = AddTexture(texturesOther.at(i), matOther);
			otherMatIdxToThisMatIdx.insert(std::make_pair(i, idx));
		}

		auto &meshGroupsOther = other.GetMeshGroups();
		auto &baseMeshesOther = other.GetBaseMeshes();
		auto &meshGroups = GetMeshGroups();
		auto &baseMeshes = GetBaseMeshes();
		for(auto i = decltype(meshGroupsOther.size()) {0}; i < meshGroupsOther.size(); ++i) {
			auto &groupOther = meshGroupsOther.at(i);
			std::shared_ptr<ModelMeshGroup> group = nullptr;
			if(i >= meshGroups.size())
				group = AddMeshGroup(groupOther->GetName());
			else
				group = meshGroups.at(i);
			for(auto &meshOther : groupOther->GetMeshes()) {
				auto mesh = meshOther->Copy();
				for(auto &subMesh : mesh->GetSubMeshes()) {
					subMesh = subMesh->Copy();
					subMesh->SetSkinTextureIndex(otherMatIdxToThisMatIdx[subMesh->GetSkinTextureIndex()]);
					// TODO: Update vertex weights
				}
				group->AddMesh(mesh);
			}
		}
	}
}
