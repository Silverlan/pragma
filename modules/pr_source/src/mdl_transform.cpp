#include "mdl.h"
#include "mathutil/umat.h"
#include <mathutil/uvec.h>
#include <mathutil/eulerangles.h>
#include <pragma/model/modelmesh.h>
#include <pragma/model/model.h>

static void build_node_matrix(Frame &frame,uint32_t id,const std::shared_ptr<Bone> &bone,std::vector<Mat4> &matrices)
{
	auto &pos = *frame.GetBonePosition(id);
	auto &rot = *frame.GetBoneOrientation(id); // Actually containes angles (in radians) in xyz components
	EulerAngles ang(rot.x,rot.y,rot.z);
	//EulerAngles ang(umath::rad_to_deg(rot.x),umath::rad_to_deg(rot.y),umath::rad_to_deg(rot.z));

	auto m = import::mdl::util::euler_angles_to_matrix(ang);
	import::mdl::util::translate_matrix(m,pos);
	if(bone->parent.expired() == false)
		m = import::mdl::util::mul_matrix(m,matrices[bone->parent.lock()->ID]);
	matrices[id] = m;
	for(auto &pair : bone->children)
		build_node_matrix(frame,pair.first,pair.second,matrices);
}

static void transform_frame(Skeleton &skeleton,const std::shared_ptr<Animation> &anim,Frame &frame,bool bLocalize=true)
{
	auto numBones = frame.GetBoneCount();
	std::vector<Mat4> matrices(numBones);
	auto &rootBones = skeleton.GetRootBones();
	for(auto &pair : rootBones)
		build_node_matrix(frame,pair.first,pair.second,matrices);
	for(auto i=decltype(numBones){0};i<numBones;++i)
	{
		auto &m = matrices[i];

		auto &pos = *frame.GetBonePosition(i);
		auto &rot = *frame.GetBoneOrientation(i);
		pos = import::mdl::util::get_translation(m);
		auto y = pos.y;
		pos.y = pos.z;
		pos.z = -y;
		pos.x = -pos.x;

		rot = import::mdl::util::get_rotation(m);
		import::mdl::util::convert_rotation(rot);
	}
	if(bLocalize == true)
		frame.Localize(*anim,skeleton);
}

static void transform_animation(Skeleton &skeleton,const std::shared_ptr<Animation> &anim,bool bLocalize=true)
{
	for(auto &frame : anim->GetFrames())
		transform_frame(skeleton,anim,*frame,bLocalize);
}

void import::MdlInfo::ConvertTransforms(const std::vector<std::shared_ptr<ModelSubMesh>> &meshesSkip,Animation *reference)
{
	auto &skeleton = model.GetSkeleton();
	uint32_t count = 0;
	for(auto &anim : model.GetAnimations())//animations)
	{
		++count;
		if(anim.get() == reference)
			continue;
		transform_animation(skeleton,anim);

		// Fix rotations for some animations (Usually delta animations)
		// where root bones have no rotation (Unit quaternion) which results in incorrect
		// rotation transformation from import::mdl::util::convert_rotation.
		// This is an ugly hack and is only needed because there's probably
		// an error somewhere else in the code? Unsure.
		// Examples: antlion_guard.mdl ("charge_hit")
		// In some cases this results in errors, e.g. "layer_body_yaw" of fassassin.mdl
		if(anim->HasFlag(FAnim::Gesture) == true)
		{
			auto &rootBones = skeleton.GetRootBones();
			std::vector<uint32_t> boneIds;
			boneIds.reserve(rootBones.size());
			for(auto &pair : rootBones)
				boneIds.push_back(pair.first);
			auto bFirstFrame = true;
			for(auto &frame : anim->GetFrames())
			{
				for(auto it=boneIds.begin();it!=boneIds.end();)
				{
					auto &rot = *frame->GetBoneOrientation(*it);
					if(bFirstFrame == true && (umath::abs(rot.w) > 0.001f || umath::abs(rot.y) > 0.001f || umath::abs(rot.z) > 0.001f || umath::abs(rot.x -1.f) > 0.001f))
					{
						it = boneIds.erase(it);
						break;
					}
					rot = Quat(0,-1,0,0) *rot;
					++it;
				}
				bFirstFrame = false;
			}
		}
	}
	auto anim = model.GetAnimation(model.LookupAnimation("reference"));
	if(anim != nullptr)
		transform_animation(skeleton,anim,false); // Don't localize; Will be localized after reference pose has been assigned!


	for(auto &anim : model.GetAnimations())
	{
		if(anim->HasFlag(FAnim::Gesture))
			continue;
		if(anim.get() == reference)
			continue;
		for(auto &pair : skeleton.GetRootBones())
		{
			for(auto &frame : anim->GetFrames())
			{
				auto &pos = *frame->GetBonePosition(pair.first);
				auto &rot = *frame->GetBoneOrientation(pair.first);
				uvec::rotate(&pos,uquat::create(EulerAngles{0.f,90.f,0.f}));
				rot = uquat::create(EulerAngles{0.f,90.f,0.f}) *rot;
			}
		}
	}

	for(auto &meshGroup : model.GetMeshGroups())
	{
		for(auto &mesh : meshGroup->GetMeshes())
		{
			for(auto &subMesh : mesh->GetSubMeshes())
			{
				auto it = std::find(meshesSkip.begin(),meshesSkip.end(),subMesh);
				if(it != meshesSkip.end())
					continue;
				for(auto &v : subMesh->GetVertices())
				{
					auto y = v.position.y;
					v.position.y = v.position.z;
					v.position.z = -y;

					// Note: This works for regular props, but may not work properly with props that
					// have the static flag set. If so, add a separate case for static props!
					v.normal = {v.normal.x,v.normal.z,-v.normal.y};
				}
			}
		}
	}
}
