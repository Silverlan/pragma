// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :game.rig_config;

const std::vector<std::string> &pragma::ik::RigConfig::get_supported_extensions()
{
	static std::vector<std::string> exts = {PIKR_EXTENSION_ASCII, PIKR_EXTENSION_BINARY};
	return exts;
}
std::optional<pragma::ik::RigConfig> pragma::ik::RigConfig::load(const std::string &fileName)
{
	auto nFileName = fs::find_available_file(fileName, get_supported_extensions());
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
	if(!prop["bones"])
		return {};
	RigConfig rig {};
	prop["rootBone"] >> rig.m_rootBone;
	for(const auto &udmBone : prop["bones"]) {
		std::string name;
		bool locked = false;
		udmBone["name"] >> name;
		udmBone["locked"] >> locked;
		auto bone = rig.AddBone(name);
		rig.SetBoneLocked(name, locked);

		udmBone["radius"] >> bone->radius;
		udmBone["length"] >> bone->length;
		udmBone["ikPose"] >> bone->ikPose;
	}

	for(const auto &udmControl : prop["controls"]) {
		std::string bone;
		auto type = RigConfigControl::Type::Drag;
		udmControl["bone"] >> bone;
		udmControl["type"] >> type;
		auto ctrl = rig.AddControl(bone, type);
		if(ctrl) {
			udmControl["maxForce"] >> ctrl->maxForce;
			udmControl["rigidity"] >> ctrl->rigidity;

			std::string startBone;
			std::string endBone;
			udmControl["baseBone"] >> startBone;
			udmControl["effectorBone"] >> endBone;
			ctrl->poleTargetBaseBone = startBone;
			ctrl->poleTargetEffectorBone = endBone;

			udmControl["poleAngle"] >> ctrl->poleAngle;
			udmControl["position"] >> ctrl->initialPos;
			udmControl["rotation"] >> ctrl->initialRot;
		}
	}

	for(const auto &udmConstraint : prop["constraints"]) {
		std::string bone0, bone1;
		auto type = RigConfigConstraint::Type::Fixed;
		udmConstraint["bone0"] >> bone0;
		udmConstraint["bone1"] >> bone1;
		udmConstraint["type"] >> type;
		PRigConfigConstraint constraint = nullptr;
		switch(type) {
		case RigConfigConstraint::Type::Fixed:
			constraint = rig.AddFixedConstraint(bone0, bone1);
			break;
		case RigConfigConstraint::Type::Hinge:
			{
				float minAngle = 0.f;
				float maxAngle = 0.f;
				udmConstraint["minAngle"] >> minAngle;
				udmConstraint["maxAngle"] >> maxAngle;
				constraint = rig.AddHingeConstraint(bone0, bone1, minAngle, maxAngle);
				break;
			}
		case RigConfigConstraint::Type::BallSocket:
			{
				EulerAngles minAngles {};
				EulerAngles maxAngles {};
				udmConstraint["minAngles"] >> minAngles;
				udmConstraint["maxAngles"] >> maxAngles;
				constraint = rig.AddBallSocketConstraint(bone0, bone1, minAngles, maxAngles);
				break;
			}
		}
		if(constraint) {
			udmConstraint["rigidity"] >> constraint->rigidity;
			udmConstraint["maxForce"] >> constraint->maxForce;

			udmConstraint["offsetPose"] >> constraint->offsetPose;
			udmConstraint["axis"] >> constraint->axis;
		}
	}

	for(const auto &udmJoint : prop["joints"]) {
		std::string bone0, bone1;
		udmJoint["bone0"] >> bone0;
		udmJoint["bone1"] >> bone1;
		auto type = RigConfigJoint::Type::BallSocketJoint;
		udmJoint["type"] >> type;
		PRigConfigJoint joint = nullptr;
		switch(type) {
		case RigConfigJoint::Type::BallSocketJoint:
			{
				Vector3 anchorPosition;
				udmJoint["anchorPosition"] >> anchorPosition;
				joint = rig.AddBallSocketJoint(bone0, bone1, anchorPosition);
				break;
			}
		case RigConfigJoint::Type::SwingLimit:
			{
				Vector3 axisA, axisB;
				float maxAngle = 0.f;
				udmJoint["axisA"] >> axisA;
				udmJoint["axisB"] >> axisB;
				udmJoint["maxAngle"] >> maxAngle;
				joint = rig.AddSwingLimit(bone0, bone1, axisA, axisB, maxAngle);
				break;
			}
		case RigConfigJoint::Type::TwistLimit:
			{
				Vector3 axisA, axisB;
				float maxAngle = 0.f;
				float rigidity = 1.f;
				udmJoint["axisA"] >> axisA;
				udmJoint["axisB"] >> axisB;
				udmJoint["maxAngle"] >> maxAngle;
				udmJoint["rigidity"] >> rigidity;
				joint = rig.AddTwistLimit(bone0, bone1, axisA, axisB, maxAngle, rigidity);
				break;
			}
		case RigConfigJoint::Type::SwivelHingeJoint:
			{
				Vector3 axisA, axisB;
				udmJoint["axisA"] >> axisA;
				udmJoint["axisB"] >> axisB;
				joint = rig.AddSwivelHingeJoint(bone0, bone1, axisA, axisB);
				break;
			}
		case RigConfigJoint::Type::TwistJoint:
			{
				Vector3 axisA, axisB;
				float rigidity = 1.f;
				udmJoint["axisA"] >> axisA;
				udmJoint["axisB"] >> axisB;
				udmJoint["rigidity"] >> rigidity;
				joint = rig.AddTwistJoint(bone0, bone1, axisA, axisB, rigidity);
				break;
			}
		case RigConfigJoint::Type::AngularJoint:
			{
				float rigidity = 1.f;
				udmJoint["rigidity"] >> rigidity;
				joint = rig.AddAngularJoint(bone0, bone1, rigidity);
				break;
			}
		case RigConfigJoint::Type::DistanceJoint:
			{
				float rigidity = 1.f;
				udmJoint["rigidity"] >> rigidity;
				joint = rig.AddDistanceJoint(bone0, bone1, rigidity);
				break;
			}
		case RigConfigJoint::Type::ParentJoint:
			{
				joint = rig.AddParentJoint(bone0, bone1);
				break;
			}
		}
		static_assert(math::to_integral(RigConfigJoint::Type::Count) == 8u, "Update this list when new joint types are added!");

		if(joint)
			udmJoint["measurementAxisA"] >> joint->measurementAxisA;
	}
	return rig;
}

pragma::ik::RigConfig::RigConfig() {}

std::vector<pragma::ik::PRigConfigBone>::iterator pragma::ik::RigConfig::FindBoneIt(const GString &name)
{
	return std::find_if(m_bones.begin(), m_bones.end(), [&name](const PRigConfigBone &bone) { return bone->name == name; });
}
const std::vector<pragma::ik::PRigConfigBone>::iterator pragma::ik::RigConfig::FindBoneIt(const GString &name) const { return const_cast<RigConfig *>(this)->FindBoneIt(name); }

std::vector<pragma::ik::PRigConfigControl>::iterator pragma::ik::RigConfig::FindControlIt(const GString &name)
{
	return std::find_if(m_controls.begin(), m_controls.end(), [&name](const PRigConfigControl &ctrl) { return ctrl->bone == name; });
}
const std::vector<pragma::ik::PRigConfigControl>::iterator pragma::ik::RigConfig::FindControlIt(const GString &name) const { return const_cast<RigConfig *>(this)->FindControlIt(name); }

pragma::ik::PRigConfigBone pragma::ik::RigConfig::FindBone(const GString &name)
{
	auto it = FindBoneIt(name);
	if(it == m_bones.end())
		return nullptr;
	return *it;
}

pragma::ik::PRigConfigBone pragma::ik::RigConfig::AddBone(const GString &name)
{
	auto bone = FindBone(name);
	if(bone)
		return bone;
	m_bones.push_back(pragma::util::make_shared<RigConfigBone>());
	bone = m_bones.back();
	bone->name = name;
	return bone;
}

void pragma::ik::RigConfig::RemoveBone(const GString &name)
{
	auto it = FindBoneIt(name);
	if(it == m_bones.end())
		return;
	RemoveConstraints((*it)->name);
	m_bones.erase(it);
}
bool pragma::ik::RigConfig::HasBone(const GString &name) const { return FindBoneIt(name) != m_bones.end(); }
bool pragma::ik::RigConfig::IsBoneLocked(const GString &name) const
{
	auto it = FindBoneIt(name);
	if(it == m_bones.end())
		return false;
	return (*it)->locked;
}
void pragma::ik::RigConfig::SetBoneLocked(const GString &name, bool locked)
{
	auto it = FindBoneIt(name);
	if(it == m_bones.end())
		return;
	(*it)->locked = locked;
}

void pragma::ik::RigConfig::RemoveControl(const GString &name)
{
	auto it = FindControlIt(name);
	if(it == m_controls.end())
		return;
	m_controls.erase(it);
}
bool pragma::ik::RigConfig::HasControl(const GString &name) const { return FindControlIt(name) != m_controls.end(); }

pragma::ik::PRigConfigControl pragma::ik::RigConfig::AddControl(const GString &bone, RigConfigControl::Type type, float rigidity)
{
	RemoveControl(bone);
	m_controls.push_back(pragma::util::make_shared<RigConfigControl>());
	auto &ctrl = m_controls.back();
	ctrl->bone = bone;
	ctrl->type = type;
	ctrl->rigidity = rigidity;
	return ctrl;
}

void pragma::ik::RigConfig::RemoveJoints(const GString &bone)
{
	for(auto it = m_joints.begin(); it != m_joints.end();) {
		auto &c = *it;
		if(c->bone0 == bone || c->bone1 == bone)
			it = m_joints.erase(it);
		else
			++it;
	}
}
void pragma::ik::RigConfig::RemoveJoints(const GString &bone0, const GString &bone1)
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
pragma::ik::PRigConfigJoint pragma::ik::RigConfig::AddBallSocketJoint(const GString &bone0, const GString &bone1, const Vector3 &anchorPosition, float rigidity)
{
	m_joints.push_back(pragma::util::make_shared<RigConfigJoint>());
	auto &j = m_joints.back();
	j->bone0 = bone0;
	j->bone1 = bone1;
	j->type = RigConfigJoint::Type::BallSocketJoint;
	j->rigidity = rigidity;
	j->anchorPosition = anchorPosition;
	return j;
}
pragma::ik::PRigConfigJoint pragma::ik::RigConfig::AddSwingLimit(const GString &bone0, const GString &bone1, const Vector3 &axisA, const Vector3 &axisB, math::Degree maxAngle, float rigidity)
{
	m_joints.push_back(pragma::util::make_shared<RigConfigJoint>());
	auto &j = m_joints.back();
	j->bone0 = bone0;
	j->bone1 = bone1;
	j->axisA = axisA;
	j->axisB = axisB;
	j->maxAngle = maxAngle;
	j->type = RigConfigJoint::Type::SwingLimit;
	j->rigidity = rigidity;
	return j;
}
pragma::ik::PRigConfigJoint pragma::ik::RigConfig::AddTwistLimit(const GString &bone0, const GString &bone1, const Vector3 &axisA, const Vector3 &axisB, math::Degree maxAngle, float rigidity, const std::optional<Vector3> &measurementAxisA)
{
	m_joints.push_back(pragma::util::make_shared<RigConfigJoint>());
	auto &j = m_joints.back();
	j->bone0 = bone0;
	j->bone1 = bone1;
	j->axisA = axisA;
	j->axisB = axisB;
	j->maxAngle = maxAngle;
	j->rigidity = rigidity;
	j->type = RigConfigJoint::Type::TwistLimit;
	j->measurementAxisA = measurementAxisA;
	return j;
}
pragma::ik::PRigConfigJoint pragma::ik::RigConfig::AddSwivelHingeJoint(const GString &bone0, const GString &bone1, const Vector3 &axisA, const Vector3 &axisB, float rigidity)
{
	m_joints.push_back(pragma::util::make_shared<RigConfigJoint>());
	auto &j = m_joints.back();
	j->bone0 = bone0;
	j->bone1 = bone1;
	j->axisA = axisA;
	j->axisB = axisB;
	j->rigidity = rigidity;
	j->type = RigConfigJoint::Type::SwivelHingeJoint;
	return j;
}
pragma::ik::PRigConfigJoint pragma::ik::RigConfig::AddTwistJoint(const GString &bone0, const GString &bone1, const Vector3 &axisA, const Vector3 &axisB, float rigidity)
{
	m_joints.push_back(pragma::util::make_shared<RigConfigJoint>());
	auto &j = m_joints.back();
	j->bone0 = bone0;
	j->bone1 = bone1;
	j->axisA = axisA;
	j->axisB = axisB;
	j->rigidity = rigidity;
	j->type = RigConfigJoint::Type::TwistJoint;
	return j;
}
pragma::ik::PRigConfigJoint pragma::ik::RigConfig::AddDistanceJoint(const GString &bone0, const GString &bone1, float rigidity)
{
	m_joints.push_back(pragma::util::make_shared<RigConfigJoint>());
	auto &j = m_joints.back();
	j->bone0 = bone0;
	j->bone1 = bone1;
	j->rigidity = rigidity;
	j->type = RigConfigJoint::Type::DistanceJoint;
	return j;
}
pragma::ik::PRigConfigJoint pragma::ik::RigConfig::AddAngularJoint(const GString &bone0, const GString &bone1, float rigidity)
{
	m_joints.push_back(pragma::util::make_shared<RigConfigJoint>());
	auto &j = m_joints.back();
	j->bone0 = bone0;
	j->bone1 = bone1;
	j->rigidity = rigidity;
	j->type = RigConfigJoint::Type::AngularJoint;
	return j;
}
pragma::ik::PRigConfigJoint pragma::ik::RigConfig::AddParentJoint(const GString &bone0, const GString &bone1)
{
	m_joints.push_back(pragma::util::make_shared<RigConfigJoint>());
	auto &j = m_joints.back();
	j->bone0 = bone0;
	j->bone1 = bone1;
	j->type = RigConfigJoint::Type::ParentJoint;
	return j;
}

void pragma::ik::RigConfig::RemoveConstraints(const GString &bone)
{
	for(auto it = m_constraints.begin(); it != m_constraints.end();) {
		auto &c = *it;
		if(c->bone0 == bone || c->bone1 == bone)
			it = m_constraints.erase(it);
		else
			++it;
	}
}

void pragma::ik::RigConfig::RemoveConstraints(const GString &bone0, const GString &bone1)
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

pragma::ik::PRigConfigConstraint pragma::ik::RigConfig::AddFixedConstraint(const GString &bone0, const GString &bone1)
{
	m_constraints.push_back(pragma::util::make_shared<RigConfigConstraint>());
	auto &c = m_constraints.back();
	c->bone0 = bone0;
	c->bone1 = bone1;
	c->type = RigConfigConstraint::Type::Fixed;
	return c;
}
pragma::ik::PRigConfigConstraint pragma::ik::RigConfig::AddHingeConstraint(const GString &bone0, const GString &bone1, math::Degree minAngle, math::Degree maxAngle, const Quat &offsetRotation)
{
	m_constraints.push_back(pragma::util::make_shared<RigConfigConstraint>());
	auto &c = m_constraints.back();
	c->bone0 = bone0;
	c->bone1 = bone1;
	c->type = RigConfigConstraint::Type::Hinge;
	c->minLimits.p = minAngle;
	c->maxLimits.p = maxAngle;
	c->offsetPose.SetRotation(offsetRotation);
	return c;
}
pragma::ik::PRigConfigConstraint pragma::ik::RigConfig::AddBallSocketConstraint(const GString &bone0, const GString &bone1, const EulerAngles &minAngles, const EulerAngles &maxAngles, SignedAxis axis)
{
	m_constraints.push_back(pragma::util::make_shared<RigConfigConstraint>());
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
	auto el = udm::Property::Create(udm::Type::Element);
	udm::LinkedPropertyWrapper prop {*el};
	ToUdmData(prop);
	std::stringstream ss;
	el->GetValue<udm::Element>().ToAscii(udm::AsciiSaveFlags::Default, ss);
	Con::COUT << ss.str() << Con::endl;
}

void pragma::ik::RigConfig::ToUdmData(udm::LinkedPropertyWrapper &udmData) const
{
	udmData["rootBone"] << m_rootBone;

	udm::LinkedPropertyWrapper udmBones;
	if(udmData["bones"])
		udmBones = udmData["bones"];
	else
		udmBones = udmData.AddArray("bones", 0, udm::Type::Element);
	udmBones.Resize(m_bones.size());
	for(auto i = decltype(m_bones.size()) {0u}; i < m_bones.size(); ++i) {
		auto &boneData = m_bones[i];
		auto udmBone = udmBones[i];
		udmBone["name"] << boneData->name.c_str();
		udmBone["locked"] << boneData->locked;
		udmBone["radius"] << boneData->radius;
		udmBone["length"] << boneData->length;
		udmBone["ikPose"] << boneData->ikPose;
	}

	udm::LinkedPropertyWrapper udmControls;
	if(udmData["controls"])
		udmControls = udmData["controls"];
	else
		udmControls = udmData.AddArray("controls", 0, udm::Type::Element);
	udmControls.Resize(m_controls.size());
	for(auto i = decltype(m_controls.size()) {0u}; i < m_controls.size(); ++i) {
		auto &ctrlData = m_controls[i];
		auto udmControl = udmControls[i];
		udmControl["bone"] << ctrlData->bone.c_str();
		udmControl["type"] << udm::enum_to_string(ctrlData->type);
		udmControl["maxForce"] << ctrlData->maxForce;
		udmControl["rigidity"] << ctrlData->rigidity;

		if(ctrlData->type == RigConfigControl::Type::PoleTarget) {
			if(!ctrlData->poleTargetBaseBone.empty())
				udmControl["baseBone"] << ctrlData->poleTargetBaseBone.c_str();
			if(!ctrlData->poleTargetEffectorBone.empty())
				udmControl["effectorBone"] << ctrlData->poleTargetEffectorBone.c_str();

			udmControl["poleAngle"] << ctrlData->poleAngle;
			udmControl["position"] << ctrlData->initialPos;
			udmControl["rotation"] << ctrlData->initialRot;
		}
	}

	udm::LinkedPropertyWrapper udmConstraints;
	if(udmData["constraints"])
		udmConstraints = udmData["constraints"];
	else
		udmConstraints = udmData.AddArray("constraints", 0, udm::Type::Element);
	udmConstraints.Resize(m_constraints.size());
	for(auto i = decltype(m_constraints.size()) {0u}; i < m_constraints.size(); ++i) {
		auto &constraintData = m_constraints[i];
		auto udmConstraint = udmConstraints[i];
		udmConstraint["bone0"] << constraintData->bone0.c_str();
		udmConstraint["bone1"] << constraintData->bone1.c_str();
		udmConstraint["type"] << constraintData->type;

		udmConstraint["rigidity"] << constraintData->rigidity;
		udmConstraint["maxForce"] << constraintData->maxForce;

		udmConstraint["axis"] << constraintData->axis;
		udmConstraint["offsetPose"] << constraintData->offsetPose;

		switch(constraintData->type) {
		case RigConfigConstraint::Type::Fixed:
			break;
		case RigConfigConstraint::Type::Hinge:
			udmConstraint["minAngle"] << constraintData->minLimits.p;
			udmConstraint["maxAngle"] << constraintData->maxLimits.p;
			break;
		case RigConfigConstraint::Type::BallSocket:
			udmConstraint["minAngles"] << constraintData->minLimits;
			udmConstraint["maxAngles"] << constraintData->maxLimits;
			break;
		}
	}

	udm::LinkedPropertyWrapper udmJoints;
	if(udmData["joints"])
		udmJoints = udmData["joints"];
	else
		udmJoints = udmData.AddArray("joints", 0, udm::Type::Element);
	udmJoints.Resize(m_joints.size());
	for(auto i = decltype(m_joints.size()) {0u}; i < m_joints.size(); ++i) {
		auto &jointData = m_joints[i];
		auto udmJoint = udmJoints[i];
		udmJoint["bone0"] << jointData->bone0.c_str();
		udmJoint["bone1"] << jointData->bone1.c_str();
		udmJoint["type"] << jointData->type;

		udmJoint["rigidity"] << jointData->rigidity;
		udmJoint["maxAngle"] << jointData->maxAngle;

		udmJoint["axisA"] << jointData->axisA;
		udmJoint["axisB"] << jointData->axisB;
		udmJoint["measurementAxisA"] << jointData->measurementAxisA;
		udmJoint["anchorPosition"] << jointData->anchorPosition;
	}
}

float pragma::ik::RigConfig::CalcScaleFactor() const
{
	Vector3 min {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
	Vector3 max {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()};
	size_t n = 0;
	for(auto &joint : m_joints) {
		if(!joint->anchorPosition)
			continue;
		for(uint8_t i = 0; i < 3; ++i) {
			min[i] = math::min(min[i], (*joint->anchorPosition)[i]);
			max[i] = math::max(max[i], (*joint->anchorPosition)[i]);
		}
		++n;
	}
	if(n == 0)
		return 1.f;
	auto dim = math::max(math::abs(max.x - min.x), math::abs(max.y - min.y), math::abs(max.z - min.z));
	dim /= REFERENCE_HUMAN_UNIT_SIZE;
	if(dim < 0.0001f)
		return 1.f;
	return dim;
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
	if(fs::create_path(path) == false) {
		spdlog::error("Failed to create path '{}' for ik rig.", filePath.GetString());
		return false;
	}

	auto f = fs::open_file(filePath.GetString(), fs::FileMode::Write);
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
	if(control.type == pragma::ik::RigConfigControl::Type::PoleTarget)
		out << "[Start:" << control.poleTargetBaseBone << "][End:" << control.poleTargetEffectorBone << "][PoleAngle:" << control.poleAngle << "]";
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
