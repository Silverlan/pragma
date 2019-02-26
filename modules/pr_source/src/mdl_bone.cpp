#include "mdl_bone.h"
#include "mdl_animation.h"

void import::mdl::Bone::BuildHierarchy(std::vector<std::shared_ptr<Bone>> &bones)
{
	for(auto &bone : bones)
	{
		if(bone->m_parentId != -1)
		{
			auto &parent = bones[bone->m_parentId];
			bone->m_parent = parent;
			parent->m_children.push_back(bone);
		}
	}
}

void import::mdl::Bone::TransformLocalTransformsToGlobal(const std::vector<std::shared_ptr<Bone>> &bones)
{
	std::function<void(const std::vector<std::shared_ptr<mdl::Bone>>&,const Vector3&,const Quat&)> transform_local_transforms_to_global = nullptr;
	transform_local_transforms_to_global = [&transform_local_transforms_to_global](const std::vector<std::shared_ptr<mdl::Bone>> &bones,const Vector3 &origin,const Quat &originRot) {
		for(auto &bone : bones)
		{
			auto pos = bone->GetPos();
			uvec::rotate(&pos,originRot);
			pos += origin;
			bone->SetPos(pos);

			auto rot = bone->GetRot();//import::mdl::euler_angles_to_quaternion(bone->GetAngles());//bone->GetRot();
			rot = originRot *rot;

			bone->SetRot(rot);

			transform_local_transforms_to_global(bone->GetChildren(),pos,rot);
		}
	};
	transform_local_transforms_to_global(bones,{},{});
}

///////////////////////////////

import::mdl::Bone::Bone(uint32_t id,const VFilePtr &f)
	: m_id(id)
{
	auto offset = f->Tell();

	auto stdBone = f->Read<mdl::mstudiobone_t>();
	m_pos = stdBone.pos;
	m_rot = stdBone.quat;
	m_angles = stdBone.rot;
	m_flags = stdBone.flags;

	m_parentId = stdBone.parent;
	m_posScale = stdBone.posscale;
	m_rotScale = stdBone.rotscale;
	m_poseToBone = stdBone.poseToBone;

	f->Seek(offset +stdBone.surfacepropidx);
	m_surfaceProp = f->ReadString();

	f->Seek(offset +stdBone.sznameindex);
	m_name = f->ReadString();

	f->Seek(offset +sizeof(mdl::mstudiobone_t));
}

const Mat3x4 import::mdl::Bone::GetPoseToBone() const {return m_poseToBone;}
void import::mdl::Bone::SetPos(const Vector3 &pos) {m_pos = pos;}
void import::mdl::Bone::SetRot(const Quat &rot) {m_rot = rot;}

uint32_t import::mdl::Bone::GetID() const {return m_id;}
const Vector3 &import::mdl::Bone::GetPosScale() const {return m_posScale;}
const Vector3 &import::mdl::Bone::GetRotScale() const {return m_rotScale;}
const Vector3 &import::mdl::Bone::GetPos() const {return m_pos;}
const Quat &import::mdl::Bone::GetRot() const {return m_rot;}
const Vector3 &import::mdl::Bone::GetAngles() const {return m_angles;}
int32_t import::mdl::Bone::GetFlags() const {return m_flags;}
const std::string &import::mdl::Bone::GetName() const {return m_name;}
const std::string &import::mdl::Bone::GetSurfaceProp() const {return m_surfaceProp;}
const std::shared_ptr<import::mdl::Bone> &import::mdl::Bone::GetParent() const {return m_parent;}
const std::vector<std::shared_ptr<import::mdl::Bone>> &import::mdl::Bone::GetChildren() const {return m_children;}

void import::mdl::Bone::DebugPrint(const std::string &t)
{
	std::cout<<t<<"Name: "<<m_name<<std::endl;
	std::cout<<t<<"Pos: ("<<m_pos.x<<","<<m_pos.y<<","<<m_pos.z<<")"<<std::endl;
	std::cout<<t<<"Bone: ("<<m_rot.w<<","<<m_rot.x<<","<<m_rot.y<<","<<m_rot.z<<")"<<std::endl;
	std::cout<<t<<"Flags: "<<m_flags<<std::endl;
	std::cout<<t<<"Surface Prop: "<<m_surfaceProp<<std::endl;
}
