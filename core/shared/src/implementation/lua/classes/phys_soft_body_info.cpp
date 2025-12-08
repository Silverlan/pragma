// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.classes.phys_soft_body_info;

void Lua::PhysSoftBodyInfo::register_class(lua::State *l, luabind::class_<pragma::physics::PhysSoftBodyInfo> &classDef)
{
	classDef.def_readwrite("poseMatchingCoefficient", &pragma::physics::PhysSoftBodyInfo::poseMatchingCoefficient);
	classDef.def_readwrite("anchorsHardness", &pragma::physics::PhysSoftBodyInfo::anchorsHardness);
	classDef.def_readwrite("dragCoefficient", &pragma::physics::PhysSoftBodyInfo::dragCoefficient);
	classDef.def_readwrite("rigidContactsHardness", &pragma::physics::PhysSoftBodyInfo::rigidContactsHardness);
	classDef.def_readwrite("softContactsHardness", &pragma::physics::PhysSoftBodyInfo::softContactsHardness);
	classDef.def_readwrite("liftCoefficient", &pragma::physics::PhysSoftBodyInfo::liftCoefficient);
	classDef.def_readwrite("kineticContactsHardness", &pragma::physics::PhysSoftBodyInfo::kineticContactsHardness);
	classDef.def_readwrite("dynamicFrictionCoefficient", &pragma::physics::PhysSoftBodyInfo::dynamicFrictionCoefficient);
	classDef.def_readwrite("dampingCoefficient", &pragma::physics::PhysSoftBodyInfo::dampingCoefficient);
	classDef.def_readwrite("volumeConversationCoefficient", &pragma::physics::PhysSoftBodyInfo::volumeConversationCoefficient);
	classDef.def_readwrite("softVsRigidImpulseSplitK", &pragma::physics::PhysSoftBodyInfo::softVsRigidImpulseSplitK);
	classDef.def_readwrite("softVsRigidImpulseSplitR", &pragma::physics::PhysSoftBodyInfo::softVsRigidImpulseSplitR);
	classDef.def_readwrite("softVsRigidImpulseSplitS", &pragma::physics::PhysSoftBodyInfo::softVsRigidImpulseSplitS);
	classDef.def_readwrite("softVsKineticHardness", &pragma::physics::PhysSoftBodyInfo::softVsKineticHardness);
	classDef.def_readwrite("softVsRigidHardness", &pragma::physics::PhysSoftBodyInfo::softVsRigidHardness);
	classDef.def_readwrite("softVsSoftHardness", &pragma::physics::PhysSoftBodyInfo::softVsSoftHardness);
	classDef.def_readwrite("pressureCoefficient", &pragma::physics::PhysSoftBodyInfo::pressureCoefficient);
	classDef.def_readwrite("velocitiesCorrectionFactor", &pragma::physics::PhysSoftBodyInfo::velocitiesCorrectionFactor);
	classDef.def_readwrite("bendingConstraintsDistance", &pragma::physics::PhysSoftBodyInfo::bendingConstraintsDistance);
	classDef.def_readwrite("clusterCount", &pragma::physics::PhysSoftBodyInfo::clusterCount);
	classDef.def_readwrite("maxClusterIterations", &pragma::physics::PhysSoftBodyInfo::maxClusterIterations);
	classDef.def("SetMaterialStiffnessCoefficients", static_cast<void (*)(lua::State *, pragma::physics::PhysSoftBodyInfo &, uint32_t, float, float, float)>([](lua::State *l, pragma::physics::PhysSoftBodyInfo &sbInfo, uint32_t matId, float linear, float angular, float volume) {
		auto it = sbInfo.materialStiffnessCoefficient.find(matId);
		if(it == sbInfo.materialStiffnessCoefficient.end())
			it = sbInfo.materialStiffnessCoefficient.insert(std::make_pair(matId, pragma::physics::PhysSoftBodyInfo::MaterialStiffnessCoefficient {})).first;
		it->second.linear = linear;
		it->second.angular = angular;
		it->second.volume = volume;
	}));
	classDef.def("GetMaterialStiffnessCoefficients", static_cast<luabind::optional<luabind::mult<float, float, float>> (*)(lua::State *, pragma::physics::PhysSoftBodyInfo &, uint32_t)>([](lua::State *l, pragma::physics::PhysSoftBodyInfo &sbInfo, uint32_t matId) -> luabind::optional<luabind::mult<float, float, float>> {
		auto it = sbInfo.materialStiffnessCoefficient.find(matId);
		if(it == sbInfo.materialStiffnessCoefficient.end())
			return nil;
		return luabind::mult<float, float, float> {l, it->second.linear, it->second.angular, it->second.volume};
	}));
}
