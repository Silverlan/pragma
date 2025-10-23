// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <string>

module pragma.shared;

import :entities.components.func.base_soft_physics;

using namespace pragma;

PhysSoftBodyInfo *BaseFuncSoftPhysicsComponent::GetSoftBodyInfo() { return &m_softBodyInfo; }

// TODO
/*PhysObj *BaseFuncSoftPhysicsComponent::InitializePhysics()
{
	if(m_bClientsidePhysics == true)
		return nullptr;
	return Entity::InitializePhysics(pragma::physics::PHYSICSTYPE::SOFTBODY);
}*/

void BaseFuncSoftPhysicsComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEvent(pragma::ecs::BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		return SetKeyValue(kvData.key, kvData.value) ? util::EventReply::Handled : util::EventReply::Unhandled;
	});
	auto &ent = GetEntity();
	ent.AddComponent("model");
	ent.AddComponent("name");
	auto whRenderComponent = ent.AddComponent("render");
	if(whRenderComponent.valid())
		static_cast<BaseRenderComponent *>(whRenderComponent.get())->SetCastShadows(true);
}

bool BaseFuncSoftPhysicsComponent::SetKeyValue(std::string key, std::string val)
{
	if(key == "pose_matching_coefficient")
		m_softBodyInfo.poseMatchingCoefficient = ustring::to_float(val);
	else if(key == "anchors_hardness")
		m_softBodyInfo.anchorsHardness = ustring::to_float(val);
	else if(key == "drag_coefficient")
		m_softBodyInfo.dragCoefficient = ustring::to_float(val);
	else if(key == "rigid_contacts_hardness")
		m_softBodyInfo.rigidContactsHardness = ustring::to_float(val);
	else if(key == "soft_contacts_hardness")
		m_softBodyInfo.softContactsHardness = ustring::to_float(val);
	else if(key == "lift_coefficient")
		m_softBodyInfo.liftCoefficient = ustring::to_float(val);
	else if(key == "kinetic_contacts_hardness")
		m_softBodyInfo.kineticContactsHardness = ustring::to_float(val);
	else if(key == "dynamic_friction_coefficient")
		m_softBodyInfo.dynamicFrictionCoefficient = ustring::to_float(val);
	else if(key == "damping_coefficient")
		m_softBodyInfo.dampingCoefficient = ustring::to_float(val);
	else if(key == "volume_conversation_coefficient")
		m_softBodyInfo.volumeConversationCoefficient = ustring::to_float(val);
	else if(key == "soft_vs_rigid_impulse_split_k")
		m_softBodyInfo.softVsRigidImpulseSplitK = ustring::to_float(val);
	else if(key == "soft_vs_rigid_impulse_split_r")
		m_softBodyInfo.softVsRigidImpulseSplitR = ustring::to_float(val);
	else if(key == "soft_vs_rigid_impulse_split_s")
		m_softBodyInfo.softVsRigidImpulseSplitS = ustring::to_float(val);
	else if(key == "soft_vs_kinetic_hardness")
		m_softBodyInfo.softVsKineticHardness = ustring::to_float(val);
	else if(key == "soft_vs_rigid_hardness")
		m_softBodyInfo.softVsRigidHardness = ustring::to_float(val);
	else if(key == "soft_vs_soft_hardness")
		m_softBodyInfo.softVsSoftHardness = ustring::to_float(val);
	else if(key == "pressure_coefficient")
		m_softBodyInfo.pressureCoefficient = ustring::to_float(val);
	else if(key == "velocities_correction_factor")
		m_softBodyInfo.velocitiesCorrectionFactor = ustring::to_float(val);
	else if(key == "bending_constraints_distance")
		m_softBodyInfo.bendingConstraintsDistance = ustring::to_float(val);
	else if(key == "cluster_count")
		m_softBodyInfo.clusterCount = ustring::to_int(val);
	else if(key == "max_cluster_iterations")
		m_softBodyInfo.maxClusterIterations = ustring::to_int(val);
	else
		return false;
	return true;
}
