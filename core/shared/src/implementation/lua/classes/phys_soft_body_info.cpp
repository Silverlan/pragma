// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/luaapi.h"

module pragma.shared;

import :scripting.lua.classes.phys_soft_body_info;

void Lua::PhysSoftBodyInfo::register_class(lua_State *l, luabind::class_<::PhysSoftBodyInfo> &classDef)
{
	classDef.def_readwrite("poseMatchingCoefficient", &::PhysSoftBodyInfo::poseMatchingCoefficient);
	classDef.def_readwrite("anchorsHardness", &::PhysSoftBodyInfo::anchorsHardness);
	classDef.def_readwrite("dragCoefficient", &::PhysSoftBodyInfo::dragCoefficient);
	classDef.def_readwrite("rigidContactsHardness", &::PhysSoftBodyInfo::rigidContactsHardness);
	classDef.def_readwrite("softContactsHardness", &::PhysSoftBodyInfo::softContactsHardness);
	classDef.def_readwrite("liftCoefficient", &::PhysSoftBodyInfo::liftCoefficient);
	classDef.def_readwrite("kineticContactsHardness", &::PhysSoftBodyInfo::kineticContactsHardness);
	classDef.def_readwrite("dynamicFrictionCoefficient", &::PhysSoftBodyInfo::dynamicFrictionCoefficient);
	classDef.def_readwrite("dampingCoefficient", &::PhysSoftBodyInfo::dampingCoefficient);
	classDef.def_readwrite("volumeConversationCoefficient", &::PhysSoftBodyInfo::volumeConversationCoefficient);
	classDef.def_readwrite("softVsRigidImpulseSplitK", &::PhysSoftBodyInfo::softVsRigidImpulseSplitK);
	classDef.def_readwrite("softVsRigidImpulseSplitR", &::PhysSoftBodyInfo::softVsRigidImpulseSplitR);
	classDef.def_readwrite("softVsRigidImpulseSplitS", &::PhysSoftBodyInfo::softVsRigidImpulseSplitS);
	classDef.def_readwrite("softVsKineticHardness", &::PhysSoftBodyInfo::softVsKineticHardness);
	classDef.def_readwrite("softVsRigidHardness", &::PhysSoftBodyInfo::softVsRigidHardness);
	classDef.def_readwrite("softVsSoftHardness", &::PhysSoftBodyInfo::softVsSoftHardness);
	classDef.def_readwrite("pressureCoefficient", &::PhysSoftBodyInfo::pressureCoefficient);
	classDef.def_readwrite("velocitiesCorrectionFactor", &::PhysSoftBodyInfo::velocitiesCorrectionFactor);
	classDef.def_readwrite("bendingConstraintsDistance", &::PhysSoftBodyInfo::bendingConstraintsDistance);
	classDef.def_readwrite("clusterCount", &::PhysSoftBodyInfo::clusterCount);
	classDef.def_readwrite("maxClusterIterations", &::PhysSoftBodyInfo::maxClusterIterations);
	classDef.def("SetMaterialStiffnessCoefficients", static_cast<void (*)(lua_State *, ::PhysSoftBodyInfo &, uint32_t, float, float, float)>([](lua_State *l, ::PhysSoftBodyInfo &sbInfo, uint32_t matId, float linear, float angular, float volume) {
		auto it = sbInfo.materialStiffnessCoefficient.find(matId);
		if(it == sbInfo.materialStiffnessCoefficient.end())
			it = sbInfo.materialStiffnessCoefficient.insert(std::make_pair(matId, ::PhysSoftBodyInfo::MaterialStiffnessCoefficient {})).first;
		it->second.linear = linear;
		it->second.angular = angular;
		it->second.volume = volume;
	}));
	classDef.def("GetMaterialStiffnessCoefficients", static_cast<luabind::optional<luabind::mult<float, float, float>> (*)(lua_State *, ::PhysSoftBodyInfo &, uint32_t)>([](lua_State *l, ::PhysSoftBodyInfo &sbInfo, uint32_t matId) -> luabind::optional<luabind::mult<float, float, float>> {
		auto it = sbInfo.materialStiffnessCoefficient.find(matId);
		if(it == sbInfo.materialStiffnessCoefficient.end())
			return nil;
		return luabind::mult<float, float, float> {l, it->second.linear, it->second.angular, it->second.volume};
	}));
}
