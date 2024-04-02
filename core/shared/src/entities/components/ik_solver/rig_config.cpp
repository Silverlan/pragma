/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/ik_solver/rig_config.hpp"
#include "pragma/logging.hpp"

const std::vector<std::string> &pragma::ik::RigConfig::get_supported_extensions()
{
	static std::vector<std::string> exts = {"pikr", "pikr_b"};
	return exts;
}
std::optional<pragma::ik::RigConfig> pragma::ik::RigConfig::load(const std::string &fileName)
{
	auto nFileName = filemanager::find_available_file(fileName, get_supported_extensions());
	std::shared_ptr<udm::Data> udmData = nullptr;
	try {
		udmData = udm::Data::Load(nFileName ? *nFileName : fileName);
	}
	catch(const udm::Exception &e) {
		return {};
	}
	if(!udmData)
		return {};
	auto data = udmData->GetAssetData().GetData();
	return load_from_udm_data(data);
}
std::optional<pragma::ik::RigConfig> pragma::ik::RigConfig::load_from_udm_data(udm::LinkedPropertyWrapper &prop)
{
	RigConfig rig {};
	for(auto &udmBone : prop["bones"]) {
		std::string name;
		bool locked = false;
		udmBone["name"](name);
		udmBone["locked"](locked);
		rig.AddBone(name);
		rig.SetBoneLocked(name, locked);
	}

	for(auto &udmControl : prop["controls"]) {
		std::string bone;
		auto type = RigConfigControl::Type::Drag;
		udmControl["bone"](bone);
		udm::to_enum_value<RigConfigControl::Type>(udmControl["type"], type);
		auto ctrl = rig.AddControl(bone, type);
		if(ctrl) {
			float maxForce;
			if(udmControl["maxForce"](maxForce))
				ctrl->maxForce = maxForce;

			float rigidity;
			if(udmControl["rigidity"](rigidity))
				ctrl->rigidity = rigidity;
		}
	}

	for(auto &udmConstraint : prop["constraints"]) {
		std::string bone0, bone1;
		auto type = RigConfigConstraint::Type::Fixed;
		udmConstraint["bone0"](bone0);
		udmConstraint["bone1"](bone1);
		udm::to_enum_value<RigConfigConstraint::Type>(udmConstraint["type"], type);
		PRigConfigConstraint constraint = nullptr;
		switch(type) {
		case RigConfigConstraint::Type::Fixed:
			constraint = rig.AddFixedConstraint(bone0, bone1);
			break;
		case RigConfigConstraint::Type::Hinge:
			{
				float minAngle = 0.f;
				float maxAngle = 0.f;
				udmConstraint["minAngle"](minAngle);
				udmConstraint["maxAngle"](maxAngle);
				constraint = rig.AddHingeConstraint(bone0, bone1, minAngle, maxAngle);
				break;
			}
		case RigConfigConstraint::Type::BallSocket:
			{
				EulerAngles minAngles {};
				EulerAngles maxAngles {};
				udmConstraint["minAngles"](minAngles);
				udmConstraint["maxAngles"](maxAngles);
				constraint = rig.AddBallSocketConstraint(bone0, bone1, minAngles, maxAngles);
				break;
			}
		}
		if(constraint) {
			udmConstraint["rigidity"](constraint->rigidity);
			udmConstraint["maxForce"](constraint->maxForce);

			udmConstraint["offsetPose"](constraint->offsetPose);
			udm::to_enum_value<pragma::SignedAxis>(udmConstraint["axis"], constraint->axis);
		}
	}

	for(auto &udmJoint : prop["joints"]) {
		std::string bone0, bone1;
		udmJoint["bone0"](bone0);
		udmJoint["bone1"](bone1);
		auto type = RigConfigJoint::Type::BallSocketJoint;
		udm::to_enum_value<RigConfigJoint::Type>(udmJoint["type"], type);
		PRigConfigJoint joint = nullptr;
		switch(type) {
		case RigConfigJoint::Type::BallSocketJoint:
			joint = rig.AddBallSocketJoint(bone0, bone1);
			break;
		case RigConfigJoint::Type::SwingLimit:
			{
				Vector3 axisA, axisB;
				float maxAngle = 0.f;
				udmJoint["axisA"](axisA);
				udmJoint["axisB"](axisB);
				udmJoint["maxAngle"](maxAngle);
				joint = rig.AddSwingLimit(bone0, bone1, axisA, axisB, maxAngle);
				break;
			}
		case RigConfigJoint::Type::TwistLimit:
			{
				Vector3 axisA, axisB;
				float maxAngle = 0.f;
				float rigidity = 1.f;
				udmJoint["axisA"](axisA);
				udmJoint["axisB"](axisB);
				udmJoint["maxAngle"](maxAngle);
				udmJoint["rigidity"](rigidity);
				joint = rig.AddTwistLimit(bone0, bone1, axisA, axisB, maxAngle, rigidity);
				break;
			}
		case RigConfigJoint::Type::SwivelHingeJoint:
			{
				Vector3 axisA, axisB;
				udmJoint["axisA"](axisA);
				udmJoint["axisB"](axisB);
				joint = rig.AddSwivelHingeJoint(bone0, bone1, axisA, axisB);
				break;
			}
		case RigConfigJoint::Type::TwistJoint:
			{
				Vector3 axisA, axisB;
				float rigidity = 1.f;
				udmJoint["axisA"](axisA);
				udmJoint["axisB"](axisB);
				udmJoint["rigidity"](rigidity);
				joint = rig.AddTwistJoint(bone0, bone1, axisA, axisB, rigidity);
				break;
			}
		}
	}
	return rig;
}

pragma::ik::RigConfig::RigConfig() {}

std::vector<pragma::ik::PRigConfigBone>::iterator pragma::ik::RigConfig::FindBoneIt(const std::string &name)
{
	return std::find_if(m_bones.begin(), m_bones.end(), [&name](const PRigConfigBone &bone) { return bone->name == name; });
}
const std::vector<pragma::ik::PRigConfigBone>::iterator pragma::ik::RigConfig::FindBoneIt(const std::string &name) const { return const_cast<RigConfig *>(this)->FindBoneIt(name); }

std::vector<pragma::ik::PRigConfigControl>::iterator pragma::ik::RigConfig::FindControlIt(const std::string &name)
{
	return std::find_if(m_controls.begin(), m_controls.end(), [&name](const PRigConfigControl &ctrl) { return ctrl->bone == name; });
}
const std::vector<pragma::ik::PRigConfigControl>::iterator pragma::ik::RigConfig::FindControlIt(const std::string &name) const { return const_cast<RigConfig *>(this)->FindControlIt(name); }

pragma::ik::PRigConfigBone pragma::ik::RigConfig::FindBone(const std::string &name)
{
	auto it = FindBoneIt(name);
	if(it == m_bones.end())
		return nullptr;
	return *it;
}

pragma::ik::PRigConfigBone pragma::ik::RigConfig::AddBone(const std::string &name)
{
	auto bone = FindBone(name);
	if(bone)
		return bone;
	m_bones.push_back(std::make_shared<RigConfigBone>());
	bone = m_bones.back();
	bone->name = name;
	return bone;
}

void pragma::ik::RigConfig::RemoveBone(const std::string &name)
{
	auto it = FindBoneIt(name);
	if(it == m_bones.end())
		return;
	RemoveConstraints((*it)->name);
	m_bones.erase(it);
}
bool pragma::ik::RigConfig::HasBone(const std::string &name) const { return FindBoneIt(name) != m_bones.end(); }
bool pragma::ik::RigConfig::IsBoneLocked(const std::string &name) const
{
	auto it = FindBoneIt(name);
	if(it == m_bones.end())
		return false;
	return (*it)->locked;
}
void pragma::ik::RigConfig::SetBoneLocked(const std::string &name, bool locked)
{
	auto it = FindBoneIt(name);
	if(it == m_bones.end())
		return;
	(*it)->locked = locked;
}

void pragma::ik::RigConfig::RemoveControl(const std::string &name)
{
	auto it = FindControlIt(name);
	if(it == m_controls.end())
		return;
	m_controls.erase(it);
}
bool pragma::ik::RigConfig::HasControl(const std::string &name) const { return FindControlIt(name) != m_controls.end(); }

pragma::ik::PRigConfigControl pragma::ik::RigConfig::AddControl(const std::string &bone, RigConfigControl::Type type, float rigidity)
{
	RemoveControl(bone);
	m_controls.push_back(std::make_shared<RigConfigControl>());
	auto &ctrl = m_controls.back();
	ctrl->bone = bone;
	ctrl->type = type;
	ctrl->rigidity = rigidity;
	return ctrl;
}

void pragma::ik::RigConfig::RemoveJoints(const std::string &bone)
{
	for(auto it = m_joints.begin(); it != m_joints.end();) {
		auto &c = *it;
		if(c->bone0 == bone || c->bone1 == bone)
			it = m_joints.erase(it);
		else
			++it;
	}
}
void pragma::ik::RigConfig::RemoveJoints(const std::string &bone0, const std::string &bone1)
{
	for(auto it = m_joints.begin(); it != m_joints.end();) {
		auto &c = *it;
		if((c->bone0 == bone0 && c->bone1 == bone1) || (c->bone0 == bone1 && c->bone1 == bone0))
			it = m_joints.erase(it);
		else
			++it;
	}
}
void pragma::ik::RigConfig::RemoveJoint(const RigConfigJoint &joint)
{
	auto it = std::find_if(m_joints.begin(), m_joints.end(), [&joint](const PRigConfigJoint &jointOther) { return jointOther.get() == &joint; });
	if(it == m_joints.end())
		return;
	m_joints.erase(it);
}
pragma::ik::PRigConfigJoint pragma::ik::RigConfig::AddBallSocketJoint(const std::string &bone0, const std::string &bone1)
{
	m_joints.push_back(std::make_shared<RigConfigJoint>());
	auto &j = m_joints.back();
	j->bone0 = bone0;
	j->bone1 = bone1;
	j->type = RigConfigJoint::Type::BallSocketJoint;
	return j;
}
pragma::ik::PRigConfigJoint pragma::ik::RigConfig::AddSwingLimit(const std::string &bone0, const std::string &bone1, const Vector3 &axisA, const Vector3 &axisB, umath::Degree maxAngle)
{
	m_joints.push_back(std::make_shared<RigConfigJoint>());
	auto &j = m_joints.back();
	j->bone0 = bone0;
	j->bone1 = bone1;
	j->axisA = axisA;
	j->axisB = axisB;
	j->maxAngle = maxAngle;
	j->type = RigConfigJoint::Type::SwingLimit;
	return j;
}
pragma::ik::PRigConfigJoint pragma::ik::RigConfig::AddTwistLimit(const std::string &bone0, const std::string &bone1, const Vector3 &axisA, const Vector3 &axisB, umath::Degree maxAngle, float rigidity)
{
	m_joints.push_back(std::make_shared<RigConfigJoint>());
	auto &j = m_joints.back();
	j->bone0 = bone0;
	j->bone1 = bone1;
	j->axisA = axisA;
	j->axisB = axisB;
	j->maxAngle = maxAngle;
	j->rigidity = rigidity;
	j->type = RigConfigJoint::Type::TwistLimit;
	return j;
}
pragma::ik::PRigConfigJoint pragma::ik::RigConfig::AddSwivelHingeJoint(const std::string &bone0, const std::string &bone1, const Vector3 &axisA, const Vector3 &axisB)
{
	m_joints.push_back(std::make_shared<RigConfigJoint>());
	auto &j = m_joints.back();
	j->bone0 = bone0;
	j->bone1 = bone1;
	j->axisA = axisA;
	j->axisB = axisB;
	j->type = RigConfigJoint::Type::SwivelHingeJoint;
	return j;
}
pragma::ik::PRigConfigJoint pragma::ik::RigConfig::AddTwistJoint(const std::string &bone0, const std::string &bone1, const Vector3 &axisA, const Vector3 &axisB, float rigidity)
{
	m_joints.push_back(std::make_shared<RigConfigJoint>());
	auto &j = m_joints.back();
	j->bone0 = bone0;
	j->bone1 = bone1;
	j->axisA = axisA;
	j->axisB = axisB;
	j->rigidity = rigidity;
	j->type = RigConfigJoint::Type::TwistJoint;
	return j;
}

void pragma::ik::RigConfig::RemoveConstraints(const std::string &bone)
{
	for(auto it = m_constraints.begin(); it != m_constraints.end();) {
		auto &c = *it;
		if(c->bone0 == bone || c->bone1 == bone)
			it = m_constraints.erase(it);
		else
			++it;
	}
}

void pragma::ik::RigConfig::RemoveConstraints(const std::string &bone0, const std::string &bone1)
{
	for(auto it = m_constraints.begin(); it != m_constraints.end();) {
		auto &c = *it;
		if((c->bone0 == bone0 && c->bone1 == bone1) || (c->bone0 == bone1 && c->bone1 == bone0))
			it = m_constraints.erase(it);
		else
			++it;
	}
}

void pragma::ik::RigConfig::RemoveConstraint(const RigConfigConstraint &constraint)
{
	auto it = std::find_if(m_constraints.begin(), m_constraints.end(), [&constraint](const PRigConfigConstraint &constraintOther) { return constraintOther.get() == &constraint; });
	if(it == m_constraints.end())
		return;
	m_constraints.erase(it);
}
void pragma::ik::RigConfig::RemoveControl(const RigConfigControl &control)
{
	auto it = std::find_if(m_controls.begin(), m_controls.end(), [&control](const PRigConfigControl &configOther) { return configOther.get() == &control; });
	if(it == m_controls.end())
		return;
	m_controls.erase(it);
}
void pragma::ik::RigConfig::RemoveBone(const RigConfigBone &bone)
{
	auto it = std::find_if(m_bones.begin(), m_bones.end(), [&bone](const PRigConfigBone &boneOther) { return boneOther.get() == &bone; });
	if(it == m_bones.end())
		return;
	RemoveConstraints((*it)->name);
	m_bones.erase(it);
}

pragma::ik::PRigConfigConstraint pragma::ik::RigConfig::AddFixedConstraint(const std::string &bone0, const std::string &bone1)
{
	m_constraints.push_back(std::make_shared<RigConfigConstraint>());
	auto &c = m_constraints.back();
	c->bone0 = bone0;
	c->bone1 = bone1;
	c->type = RigConfigConstraint::Type::Fixed;
	return c;
}
pragma::ik::PRigConfigConstraint pragma::ik::RigConfig::AddHingeConstraint(const std::string &bone0, const std::string &bone1, umath::Degree minAngle, umath::Degree maxAngle, const Quat &offsetRotation)
{
	m_constraints.push_back(std::make_shared<RigConfigConstraint>());
	auto &c = m_constraints.back();
	c->bone0 = bone0;
	c->bone1 = bone1;
	c->type = RigConfigConstraint::Type::Hinge;
	c->minLimits.p = minAngle;
	c->maxLimits.p = maxAngle;
	c->offsetPose.SetRotation(offsetRotation);
	return c;
}
pragma::ik::PRigConfigConstraint pragma::ik::RigConfig::AddBallSocketConstraint(const std::string &bone0, const std::string &bone1, const EulerAngles &minAngles, const EulerAngles &maxAngles, SignedAxis axis)
{
	m_constraints.push_back(std::make_shared<RigConfigConstraint>());
	auto &c = m_constraints.back();
	c->bone0 = bone0;
	c->bone1 = bone1;
	c->type = RigConfigConstraint::Type::BallSocket;
	c->minLimits = minAngles;
	c->maxLimits = maxAngles;
	c->axis = axis;
	return c;
}

void pragma::ik::RigConfig::DebugPrint() const
{
	auto el = ::udm::Property::Create(::udm::Type::Element);
	udm::LinkedPropertyWrapper prop {*el};
	ToUdmData(prop);
	std::stringstream ss;
	el->GetValue<udm::Element>().ToAscii(udm::AsciiSaveFlags::None, ss);
	Con::cout << ss.str() << Con::endl;
}

void pragma::ik::RigConfig::ToUdmData(udm::LinkedPropertyWrapper &udmData) const
{
	udm::LinkedPropertyWrapper udmBones;
	if(udmData["bones"])
		udmBones = udmData["bones"];
	else
		udmBones = udmData.AddArray("bones", 0, ::udm::Type::Element);
	udmBones.Resize(m_bones.size());
	for(auto i = decltype(m_bones.size()) {0u}; i < m_bones.size(); ++i) {
		auto &boneData = m_bones[i];
		auto udmBone = udmBones[i];
		udmBone["name"] = boneData->name;
		udmBone["locked"] = boneData->locked;
	}

	udm::LinkedPropertyWrapper udmControls;
	if(udmData["controls"])
		udmControls = udmData["controls"];
	else
		udmControls = udmData.AddArray("controls", 0, ::udm::Type::Element);
	udmControls.Resize(m_controls.size());
	for(auto i = decltype(m_controls.size()) {0u}; i < m_controls.size(); ++i) {
		auto &ctrlData = m_controls[i];
		auto udmControl = udmControls[i];
		udmControl["bone"] = ctrlData->bone;
		udmControl["type"] = udm::enum_to_string(ctrlData->type);
		udmControl["maxForce"] = ctrlData->maxForce;
		udmControl["rigidity"] = ctrlData->rigidity;
	}

	udm::LinkedPropertyWrapper udmConstraints;
	if(udmData["constraints"])
		udmConstraints = udmData["constraints"];
	else
		udmConstraints = udmData.AddArray("constraints", 0, ::udm::Type::Element);
	udmConstraints.Resize(m_constraints.size());
	for(auto i = decltype(m_constraints.size()) {0u}; i < m_constraints.size(); ++i) {
		auto &constraintData = m_constraints[i];
		auto udmConstraint = udmConstraints[i];
		udmConstraint["bone0"] = constraintData->bone0;
		udmConstraint["bone1"] = constraintData->bone1;
		udmConstraint["type"] = udm::enum_to_string(constraintData->type);

		udmConstraint["rigidity"] = constraintData->rigidity;
		udmConstraint["maxForce"] = constraintData->maxForce;

		udmConstraint["axis"] = udm::enum_to_string(constraintData->axis);
		udmConstraint["offsetPose"] = constraintData->offsetPose;

		switch(constraintData->type) {
		case RigConfigConstraint::Type::Fixed:
			break;
		case RigConfigConstraint::Type::Hinge:
			udmConstraint["minAngle"] = constraintData->minLimits.p;
			udmConstraint["maxAngle"] = constraintData->maxLimits.p;
			break;
		case RigConfigConstraint::Type::BallSocket:
			udmConstraint["minAngles"] = constraintData->minLimits;
			udmConstraint["maxAngles"] = constraintData->maxLimits;
			break;
		}
	}

	udm::LinkedPropertyWrapper udmJoints;
	if(udmData["joints"])
		udmJoints = udmData["joints"];
	else
		udmJoints = udmData.AddArray("joints", 0, ::udm::Type::Element);
	udmJoints.Resize(m_joints.size());
	for(auto i = decltype(m_joints.size()) {0u}; i < m_joints.size(); ++i) {
		auto &jointData = m_joints[i];
		auto udmJoint = udmJoints[i];
		udmJoint["bone0"] = jointData->bone0;
		udmJoint["bone1"] = jointData->bone1;
		udmJoint["type"] = udm::enum_to_string(jointData->type);

		udmJoint["rigidity"] = jointData->rigidity;
		udmJoint["maxAngle"] = jointData->maxAngle;

		udmJoint["axisA"] = jointData->axisA;
		udmJoint["axisB"] = jointData->axisB;
	}
}

bool pragma::ik::RigConfig::Save(const std::string &fileName)
{
	auto filePath = util::Path::CreateFile(fileName);
	auto udmData = udm::Data::Create("PIKC", 1);
	if(!udmData) {
		spdlog::error("Failed to save ik rig '{}'.", fileName);
		return false;
	}

	auto assetData = udmData->GetAssetData().GetData();
	ToUdmData(assetData);

	std::string path {filePath.GetPath()};
	if(filemanager::create_path(path) == false) {
		spdlog::error("Failed to create path '{}' for ik rig.", filePath.GetString());
		return false;
	}

	auto f = filemanager::open_file(filePath.GetString(), filemanager::FileMode::Write);
	if(!f) {
		spdlog::error("Failed to open ik rig file '{}' for saving.", filePath.GetString());
		return false;
	}

	auto res = udmData->SaveAscii(f);
	f = nullptr;
	if(res == false) {
		spdlog::error("Failed to save ik rig '{}'.", filePath.GetString());
		return false;
	}
	return true;
}

std::ostream &operator<<(std::ostream &out, const pragma::ik::RigConfig &config)
{
	out << "RigConfig[bones:" << config.GetBones().size() << "][controls:" << config.GetControls().size() << "][constraints:" << config.GetConstraints().size() << "]";
	return out;
}

std::ostream &operator<<(std::ostream &out, const pragma::ik::RigConfigBone &bone)
{
	out << "RigConfigBone[name:" << bone.name << "][locked:" << (bone.locked ? "1" : "0") << "]";
	return out;
}
std::ostream &operator<<(std::ostream &out, const pragma::ik::RigConfigControl &control)
{
	out << "RigConfigControl[type:" << magic_enum::enum_name(control.type) << "][bone:" << control.bone << "]";
	return out;
}
std::ostream &operator<<(std::ostream &out, const pragma::ik::RigConfigConstraint &constraint)
{
	out << "RigConfigConstraint[type:" << magic_enum::enum_name(constraint.type) << "]";
	out << "[bone0:" << constraint.bone0 << "]";
	out << "[bone1:" << constraint.bone1 << "]";
	out << "[axis:" << magic_enum::enum_name(constraint.axis) << "]";
	out << "[minLimits:" << constraint.minLimits.p << "," << constraint.minLimits.y << "," << constraint.minLimits.r << "]";
	out << "[maxLimits:" << constraint.maxLimits.p << "," << constraint.maxLimits.y << "," << constraint.maxLimits.r << "]";

	auto &pos = constraint.offsetPose.GetOrigin();
	EulerAngles ang {constraint.offsetPose.GetRotation()};
	auto &scale = constraint.offsetPose.GetScale();
	out << "[offsetPose:(" << pos.x << "," << pos.y << "," << pos.z << ")(" << ang.p << "," << ang.y << "," << ang.r << ")(" << scale.x << "," << scale.y << "," << scale.z << ")]";
	return out;
}
