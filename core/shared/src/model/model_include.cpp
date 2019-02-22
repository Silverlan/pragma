#include "stdafx_shared.h"
#include "pragma/model/model.h"
#include "pragma/model/modelmesh.h"
#include "pragma/physics/collisionmesh.h"

void Model::Merge(const Model &other,MergeFlags flags)
{
	std::vector<std::size_t> boneTranslations; // 'other' bone Id to 'this' bone Id
	auto bMerged = false;
	const auto fMergeSkeletons = [this,&other,&boneTranslations,&bMerged]() {
		if(bMerged == true)
			return;
		bMerged = true;
		auto &skeletonOther = other.GetSkeleton();
		auto &bonesOther = skeletonOther.GetBones();
		auto &skeleton = GetSkeleton();
		auto &bones = skeleton.GetBones();
		bones.reserve(bones.size() +bonesOther.size());
		boneTranslations.reserve(bones.size() +bonesOther.size());
		for(auto &boneOther : bonesOther)
		{
			auto it = std::find_if(bones.begin(),bones.end(),[&boneOther](const std::shared_ptr<Bone> &bone) {
				return (bone->name == boneOther->name) ? true : false;
			});
			if(it == bones.end())
			{
				auto boneId = skeleton.AddBone(new Bone());
				auto bone = skeleton.GetBone(boneId).lock();
				bone->name = boneOther->name;
				it = bones.end() -1;
			}
			boneTranslations.push_back(it -bones.begin());
		}
		// Determine new bone parents (Has to be done AFTER all bones have been processed!)
		for(auto idxOther=decltype(boneTranslations.size()){0};idxOther<boneTranslations.size();++idxOther)
		{
			auto &boneOther = bonesOther.at(idxOther);
			if(boneOther->parent.expired() == true)
				continue;
			auto idxThis = boneTranslations.at(idxOther);
			auto &boneThis = bones.at(idxThis);
			boneThis->parent = bones.at(boneTranslations.at(boneOther->parent.lock()->ID));
		}
	};

	if((flags &MergeFlags::Animations) != MergeFlags::None)
	{
		fMergeSkeletons();
		auto &anims = GetAnimations();
		auto &animsOther = other.GetAnimations();
		anims.reserve(anims.size() +animsOther.size());
		for(auto i=decltype(animsOther.size()){0};i<animsOther.size();++i)
		{
			auto &animOther = animsOther.at(i);
			auto animName = other.GetAnimationName(i);

			auto anim = std::make_shared<Animation>(*animOther,Animation::ShareMode::Frames | Animation::ShareMode::Events);
			auto &boneList = anim->GetBoneList();
			for(auto idx=decltype(boneList.size()){0};idx<boneList.size();++idx)
				anim->SetBoneId(idx,boneTranslations.at(boneList.at(idx)));

			auto it = m_animationIDs.find(animName);
			if(it != m_animationIDs.end())
				anims.at(it->second) = anim;
			else
			{
				anims.push_back(anim);
				m_animationIDs.insert(decltype(m_animationIDs)::value_type(animName,anims.size() -1));
			}
		}
	}
	if((flags &MergeFlags::Attachments) != MergeFlags::None)
	{
		fMergeSkeletons();
		auto &skeleton = GetSkeleton();
		auto &attachments = GetAttachments();
		auto &attachmentsOther = other.GetAttachments();
		auto &skeletonOther = other.GetSkeleton();
		auto &bonesOther = skeletonOther.GetBones();
		attachments.reserve(attachments.size() +attachmentsOther.size());
		for(auto &attOther : attachmentsOther)
		{
			auto it = std::find_if(attachments.begin(),attachments.end(),[&attOther](const Attachment &att) {
				return (att.name == attOther.name) ? true : false;
			});
			if(it != attachments.end())
			{
				auto &att = *it;
				att.angles = attOther.angles;
				att.offset = attOther.offset;
				att.bone = boneTranslations.at(attOther.bone);
				continue;
			}
			AddAttachment(attOther.name,boneTranslations.at(attOther.bone),attOther.offset,attOther.angles);
		}
	}
	if((flags &MergeFlags::BlendControllers) != MergeFlags::None)
	{
		auto &blendControllersOther = other.GetBlendControllers();
		auto &blendControllers = GetBlendControllers();
		blendControllers.reserve(blendControllers.size() +blendControllersOther.size());
		for(auto &blendController : blendControllersOther)
			blendControllers.push_back(blendController);
	}
	if((flags &MergeFlags::Hitboxes) != MergeFlags::None)
	{
		fMergeSkeletons();
		auto &hitboxesOther = other.GetHitboxes();
		auto &hitboxes = GetHitboxes();
		hitboxes.reserve(hitboxes.size() +hitboxesOther.size());
		for(auto &pair : hitboxesOther)
		{
			auto &hitboxOther = pair.second;
			auto boneId = boneTranslations.at(pair.first);
			Hitbox hb(hitboxOther.group,hitboxOther.min,hitboxOther.max);
			auto it = hitboxes.find(boneId);
			if(it != hitboxes.end())
				it->second = hb;
			else
				hitboxes.insert(std::make_pair(boneId,hb));
		}
	}
	if((flags &MergeFlags::Joints) != MergeFlags::None)
	{
		fMergeSkeletons();
		auto &jointsOther = other.GetJoints();
		auto &joints = GetJoints();
		joints.reserve(joints.size() +jointsOther.size());
		for(auto &jointOther : jointsOther)
		{
			joints.push_back(JointInfo(jointOther.type,boneTranslations.at(jointOther.src),boneTranslations.at(jointOther.dest)));
			auto &joint = joints.back();
			joint.args = jointOther.args;
			joint.collide = jointOther.collide;
		}
	}
	if((flags &MergeFlags::CollisionMeshes) != MergeFlags::None)
	{
		fMergeSkeletons();
		auto &collisionMeshesOther = other.GetCollisionMeshes();
		auto &collisionMeshes = GetCollisionMeshes();
		collisionMeshes.reserve(collisionMeshes.size() +collisionMeshesOther.size());
		for(auto &colMeshOther : collisionMeshesOther)
		{
			auto colMesh = CollisionMesh::Create(*colMeshOther);
			auto boneParent = colMesh->GetBoneParent();
			if(boneParent >= 0)
				boneParent = boneTranslations.at(boneParent);
			colMesh->SetBoneParent(boneParent);
			collisionMeshes.push_back(colMesh);
		}
	}
	if((flags &MergeFlags::Meshes) != MergeFlags::None)
	{
		fMergeSkeletons();
		auto &meshGroupsOther = other.GetMeshGroups();
		auto &baseMeshesOther = other.GetBaseMeshes();
		auto &meshGroups = other.GetMeshGroups();
		auto &baseMeshes = other.GetBaseMeshes();
		for(auto i=decltype(meshGroupsOther.size()){0};i<meshGroupsOther.size();++i)
		{
			auto &groupOther = meshGroupsOther.at(i);
			std::shared_ptr<ModelMeshGroup> group = nullptr;
			if(i >= meshGroups.size())
				group = AddMeshGroup(groupOther->GetName());
			else
				group = meshGroups.at(i);
			for(auto &meshOther : groupOther->GetMeshes())
			{
				auto mesh = meshOther->Copy();
				for(auto &subMesh : mesh->GetSubMeshes())
				{
					subMesh = subMesh->Copy();
					// TODO: Update vertex weights
				}
			}
		}
	}
}
