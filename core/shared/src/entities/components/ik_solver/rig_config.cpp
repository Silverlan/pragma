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
	static std::vector<std::string> exts = {"pikr","pikr_b"};
	return exts;
}
std::optional<pragma::ik::RigConfig> pragma::ik::RigConfig::load(const std::string &fileName)
{
	auto udmData = udm::Data::Load(fileName);
	if(!udmData)
		return {};
	auto data = udmData->GetAssetData().GetData();
	return load_from_udm_data(data);
}
std::optional<pragma::ik::RigConfig> pragma::ik::RigConfig::load_from_udm_data(udm::LinkedPropertyWrapper &prop)
{
	RigConfig rig {};
	for(auto &udmBone : prop["bones"])
	{
		std::string name;
		bool locked = false;
		udmBone["name"](name);
		udmBone["locked"](locked);
		rig.AddBone(name);
		rig.SetBoneLocked(name,locked);
	}

	for(auto &udmControl : prop["controls"])
	{
		std::string bone;
		auto type = RigConfigControl::Type::Drag;
		udmControl["bone"](bone);
		udm::to_enum_value<RigConfigControl::Type>(udmControl["type"],type);
		rig.AddControl(bone,type);
	}

	for(auto &udmConstraint : prop["constraints"])
	{
		std::string bone0,bone1;
		auto type = RigConfigConstraint::Type::Fixed;
		udmConstraint["bone0"](bone0);
		udmConstraint["bone1"](bone1);
		udm::to_enum_value<RigConfigConstraint::Type>(udmConstraint["type"],type);
		switch(type)
		{
		case RigConfigConstraint::Type::Fixed:
			rig.AddFixedConstraint(bone0,bone1);
			break;
		case RigConfigConstraint::Type::Hinge:
		{
			float minAngle = 0.f;
			float maxAngle = 0.f;
			udmConstraint["minAngle"](minAngle);
			udmConstraint["maxAngle"](maxAngle);
			rig.AddHingeConstraint(bone0,bone1,minAngle,maxAngle);
			break;
		}
		case RigConfigConstraint::Type::BallSocket:
		{
			EulerAngles minAngles {};
			EulerAngles maxAngles {};
			udmConstraint["minAngles"](minAngles);
			udmConstraint["maxAngles"](maxAngles);
			rig.AddBallSocketConstraint(bone0,bone1,minAngles,maxAngles);
			break;
		}
		}
	}
	return rig;
}

pragma::ik::RigConfig::RigConfig()
{}

std::vector<pragma::ik::RigConfigBone>::iterator pragma::ik::RigConfig::FindBone(const std::string &name)
{
	return std::find_if(m_bones.begin(),m_bones.end(),[&name](const RigConfigBone &bone) {return bone.name == name;});
}
const std::vector<pragma::ik::RigConfigBone>::iterator pragma::ik::RigConfig::FindBone(const std::string &name) const {return const_cast<RigConfig*>(this)->FindBone(name);}

std::vector<pragma::ik::RigConfigControl>::iterator pragma::ik::RigConfig::FindControl(const std::string &name)
{
	return std::find_if(m_controls.begin(),m_controls.end(),[&name](const RigConfigControl &ctrl) {return ctrl.bone == name;});
}
const std::vector<pragma::ik::RigConfigControl>::iterator pragma::ik::RigConfig::FindControl(const std::string &name) const {return const_cast<RigConfig*>(this)->FindControl(name);}

void pragma::ik::RigConfig::AddBone(const std::string &name)
{
	RemoveBone(name);
	m_bones.push_back({});
	auto &bone = m_bones.back();
	bone.name = name;
}

void pragma::ik::RigConfig::RemoveBone(const std::string &name)
{
	auto it = FindBone(name);
	if(it == m_bones.end())
		return;
	m_bones.erase(it);
}
bool pragma::ik::RigConfig::HasBone(const std::string &name) const {return FindBone(name) != m_bones.end();}
bool pragma::ik::RigConfig::IsBoneLocked(const std::string &name) const
{
	auto it = FindBone(name);
	if(it == m_bones.end())
		return false;
	return it->locked;
}
void pragma::ik::RigConfig::SetBoneLocked(const std::string &name,bool locked)
{
	auto it = FindBone(name);
	if(it == m_bones.end())
		return;
	it->locked = locked;
}

void pragma::ik::RigConfig::RemoveControl(const std::string &name)
{
	auto it = FindControl(name);
	if(it == m_controls.end())
		return;
	m_controls.erase(it);
}
bool pragma::ik::RigConfig::HasControl(const std::string &name) const {return FindControl(name) != m_controls.end();}

void pragma::ik::RigConfig::AddControl(const std::string &bone,RigConfigControl::Type type)
{
	RemoveControl(bone);
	m_controls.push_back({});
	auto &ctrl = m_controls.back();
	ctrl.bone = bone;
	ctrl.type = type;
}

void pragma::ik::RigConfig::RemoveConstraints(const std::string &bone0,const std::string &bone1)
{
	for(auto it=m_constraints.begin();it!=m_constraints.end();)
	{
		auto &c = *it;
		if((c.bone0 == bone0 and c.bone1 == bone1) || (c.bone0 == bone1 && c.bone1 == bone0))
			it = m_constraints.erase(it);
		else
			++it;
	}
}

void pragma::ik::RigConfig::AddFixedConstraint(const std::string &bone0,const std::string &bone1)
{
	m_constraints.push_back({});
	auto &c = m_constraints.back();
	c.bone0 = bone0;
	c.bone1 = bone1;
	c.type = RigConfigConstraint::Type::Fixed;
}
static void clamp_angles(float &min,float &max)
{
	// If the span range is too small it can cause instability,
	// so we'll force a minimum span angle
	constexpr umath::Degree minSpan = 0.5f;
	if(umath::abs(max -min) < minSpan)
	{
		auto baseAngle = (min +max) /2.f;
		min = baseAngle -minSpan;
		max = baseAngle +minSpan;
	}
}
void pragma::ik::RigConfig::AddHingeConstraint(const std::string &bone0,const std::string &bone1,umath::Degree minAngle,umath::Degree maxAngle)
{
	clamp_angles(minAngle,maxAngle);
	m_constraints.push_back({});
	auto &c = m_constraints.back();
	c.bone0 = bone0;
	c.bone1 = bone1;
	c.type = RigConfigConstraint::Type::Hinge;
	c.minLimits.p = minAngle;
	c.maxLimits.p = maxAngle;
}
void pragma::ik::RigConfig::AddBallSocketConstraint(const std::string &bone0,const std::string &bone1,const EulerAngles &minAngles,const EulerAngles &maxAngles)
{
	m_constraints.push_back({});
	auto &c = m_constraints.back();
	c.bone0 = bone0;
	c.bone1 = bone1;
	c.type = RigConfigConstraint::Type::BallSocket;
	c.minLimits = minAngles;
	c.maxLimits = maxAngles;
	for(uint8_t i=0;i<3;++i)
		clamp_angles(c.minLimits[i],c.maxLimits[i]);
}

void pragma::ik::RigConfig::DebugPrint() const
{
	auto el = ::udm::Property::Create(::udm::Type::Element);
	udm::LinkedPropertyWrapper prop {*el};
	ToUdmData(prop);
	std::stringstream ss;
	el->GetValue<udm::Element>().ToAscii(udm::AsciiSaveFlags::None,ss);
	Con::cout<<ss.str()<<Con::endl;
}

void pragma::ik::RigConfig::ToUdmData(udm::LinkedPropertyWrapper &udmData) const
{
	udm::LinkedPropertyWrapper udmBones;
	if(udmData["bones"])
		udmBones = udmData["bones"];
	else
		udmBones = udmData.AddArray("bones",0,::udm::Type::Element);
	udmBones.Resize(m_bones.size());
	for(auto i=decltype(m_bones.size()){0u};i<m_bones.size();++i)
	{
		auto &boneData = m_bones[i];
		auto udmBone = udmBones[i];
		udmBone["name"] = boneData.name;
		udmBone["locked"] = boneData.locked;
	}
	
	udm::LinkedPropertyWrapper udmControls;
	if(udmData["controls"])
		udmControls = udmData["controls"];
	else
		udmControls = udmData.AddArray("controls",0,::udm::Type::Element);
	udmControls.Resize(m_controls.size());
	for(auto i=decltype(m_controls.size()){0u};i<m_controls.size();++i)
	{
		auto &ctrlData = m_controls[i];
		auto udmBone = udmControls[i];
		udmBone["bone"] = ctrlData.bone;
		udmBone["type"] = udm::enum_to_string(ctrlData.type);
	}
	
	udm::LinkedPropertyWrapper udmConstraints;
	if(udmData["constraints"])
		udmConstraints = udmData["constraints"];
	else
		udmConstraints = udmData.AddArray("constraints",0,::udm::Type::Element);
	udmConstraints.Resize(m_constraints.size());
	for(auto i=decltype(m_constraints.size()){0u};i<m_constraints.size();++i)
	{
		auto &constraintData = m_constraints[i];
		auto udmBone = udmConstraints[i];
		udmBone["bone0"] = constraintData.bone0;
		udmBone["bone1"] = constraintData.bone1;
		udmBone["type"] = udm::enum_to_string(constraintData.type);
		switch(constraintData.type)
		{
		case RigConfigConstraint::Type::Fixed:
			break;
		case RigConfigConstraint::Type::Hinge:
			udmBone["minAngle"] = constraintData.minLimits.p;
			udmBone["maxAngle"] = constraintData.maxLimits.p;
			break;
		case RigConfigConstraint::Type::BallSocket:
			udmBone["minAngles"] = constraintData.minLimits;
			udmBone["maxAngles"] = constraintData.maxLimits;
			break;
		}
	}
}

bool pragma::ik::RigConfig::Save(const std::string &fileName)
{
	auto filePath = util::Path::CreateFile(fileName);
	auto udmData = udm::Data::Create("PIKC",1);
	if(!udmData)
	{
		spdlog::error("Failed to save ik rig '{}'.",fileName);
		return false;
	}

	auto assetData = udmData->GetAssetData().GetData();
	ToUdmData(assetData);

	if(filemanager::create_path(filePath.GetString()) == false)
	{
		spdlog::error("Failed to create path '{}' for ik rig.",filePath.GetString());
		return false;
	}

	auto f = filemanager::open_file(filePath.GetString(),filemanager::FileMode::Write);
	if(!f)
	{
		spdlog::error("Failed to open ik rig file '{}' for saving.",filePath.GetString());
		return false;
	}

	auto res = udmData->SaveAscii(f);
	f = nullptr;
	if(res == false)
	{
		spdlog::error("Failed to save ik rig '{}'.",filePath.GetString());
		return false;
	}
	return true;
}
