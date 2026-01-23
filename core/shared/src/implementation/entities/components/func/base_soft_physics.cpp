// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.func.base_soft_physics;

using namespace pragma;

physics::PhysSoftBodyInfo *BaseFuncSoftPhysicsComponent::GetSoftBodyInfo() { return &m_softBodyInfo; }

// TODO
/*PhysObj *BaseFuncSoftPhysicsComponent::InitializePhysics()
{
	if(m_bClientsidePhysics == true)
		return nullptr;
	return Entity::InitializePhysics(pragma::physics::PhysicsType::SoftBody);
}*/

void BaseFuncSoftPhysicsComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
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
		m_softBodyInfo.poseMatchingCoefficient = string::to_float(val);
	else if(key == "anchors_hardness")
		m_softBodyInfo.anchorsHardness = string::to_float(val);
	else if(key == "drag_coefficient")
		m_softBodyInfo.dragCoefficient = string::to_float(val);
	else if(key == "rigid_contacts_hardness")
		m_softBodyInfo.rigidContactsHardness = string::to_float(val);
	else if(key == "soft_contacts_hardness")
		m_softBodyInfo.softContactsHardness = string::to_float(val);
	else if(key == "lift_coefficient")
		m_softBodyInfo.liftCoefficient = string::to_float(val);
	else if(key == "kinetic_contacts_hardness")
		m_softBodyInfo.kineticContactsHardness = string::to_float(val);
	else if(key == "dynamic_friction_coefficient")
		m_softBodyInfo.dynamicFrictionCoefficient = string::to_float(val);
	else if(key == "damping_coefficient")
		m_softBodyInfo.dampingCoefficient = string::to_float(val);
	else if(key == "volume_conversation_coefficient")
		m_softBodyInfo.volumeConversationCoefficient = string::to_float(val);
	else if(key == "soft_vs_rigid_impulse_split_k")
		m_softBodyInfo.softVsRigidImpulseSplitK = string::to_float(val);
	else if(key == "soft_vs_rigid_impulse_split_r")
		m_softBodyInfo.softVsRigidImpulseSplitR = string::to_float(val);
	else if(key == "soft_vs_rigid_impulse_split_s")
		m_softBodyInfo.softVsRigidImpulseSplitS = string::to_float(val);
	else if(key == "soft_vs_kinetic_hardness")
		m_softBodyInfo.softVsKineticHardness = string::to_float(val);
	else if(key == "soft_vs_rigid_hardness")
		m_softBodyInfo.softVsRigidHardness = string::to_float(val);
	else if(key == "soft_vs_soft_hardness")
		m_softBodyInfo.softVsSoftHardness = string::to_float(val);
	else if(key == "pressure_coefficient")
		m_softBodyInfo.pressureCoefficient = string::to_float(val);
	else if(key == "velocities_correction_factor")
		m_softBodyInfo.velocitiesCorrectionFactor = string::to_float(val);
	else if(key == "bending_constraints_distance")
		m_softBodyInfo.bendingConstraintsDistance = string::to_float(val);
	else if(key == "cluster_count")
		m_softBodyInfo.clusterCount = string::to_int(val);
	else if(key == "max_cluster_iterations")
		m_softBodyInfo.maxClusterIterations = string::to_int(val);
	else
		return false;
	return true;
}
