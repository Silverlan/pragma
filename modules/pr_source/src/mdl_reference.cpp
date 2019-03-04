#include "mdl.h"
#include <pragma/model/model.h>

static void to_local_bone_system(const SMDModel::Frame &frame,const SMDModel::Node &node,Vector3 &pos,Quat &rot)
{
	auto parentId = node.parent;
	if(parentId != -1)
	{
		auto &tParent = frame.transforms[parentId];
		pos -= tParent.position;
		auto inv = uquat::get_inverse(tParent.rotation);
		uvec::rotate(&pos,inv);

		rot = inv *rot;
	}
}

void import::MdlInfo::GenerateReference(const std::vector<std::shared_ptr<import::mdl::Bone>> &bones)
{
	/*auto animId = model.LookupAnimation("reference");
	if(animId == -1)
	{
		model.AddAnimation("reference",std::make_shared<Animation>());
		animId = model.LookupAnimation("reference");
	}

	auto anim = model.GetAnimation(animId);
	auto *frame = anim->GetFrame(0);
	if(frame == nullptr)
	{
		frame = new Frame(bones.size());
		anim->AddFrame(frame);
	}

	auto &skeleton = *model.GetSkeleton();
	for(auto i=decltype(bones.size()){0};i<bones.size();++i)
	{
		auto &pos = bones[i]->GetPos();
		auto &rot = bones[i]->GetAngles();
		//to_local_bone_system(smdFrame,node,pos,rot);

		frame->SetBonePosition(i,pos);
		frame->SetBoneOrientation(i,rot);

		auto m = glm::toMat4(t.rotation);
		m = glm::translate(m,t.position);
		mdl.SetBindPoseBoneMatrix(boneId,glm::inverse(m));
	}
	auto *refFrame = new Frame(*frame);
	frame->Localize(anim.get(),mdl.GetSkeleton());
	mdl.SetReference(refFrame);*/
}
