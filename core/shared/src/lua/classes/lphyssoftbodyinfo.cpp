/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/lua/classes/lphyssoftbodyinfo.hpp"
#include "pragma/physics/physsoftbodyinfo.hpp"

void Lua::PhysSoftBodyInfo::register_class(lua_State *l,luabind::class_<::PhysSoftBodyInfo> &classDef)
{
	classDef.def_readwrite("poseMatchingCoefficient",&::PhysSoftBodyInfo::poseMatchingCoefficient);
	classDef.def_readwrite("anchorsHardness",&::PhysSoftBodyInfo::anchorsHardness);
	classDef.def_readwrite("dragCoefficient",&::PhysSoftBodyInfo::dragCoefficient);
	classDef.def_readwrite("rigidContactsHardness",&::PhysSoftBodyInfo::rigidContactsHardness);
	classDef.def_readwrite("softContactsHardness",&::PhysSoftBodyInfo::softContactsHardness);
	classDef.def_readwrite("liftCoefficient",&::PhysSoftBodyInfo::liftCoefficient);
	classDef.def_readwrite("kineticContactsHardness",&::PhysSoftBodyInfo::kineticContactsHardness);
	classDef.def_readwrite("dynamicFrictionCoefficient",&::PhysSoftBodyInfo::dynamicFrictionCoefficient);
	classDef.def_readwrite("dampingCoefficient",&::PhysSoftBodyInfo::dampingCoefficient);
	classDef.def_readwrite("volumeConversationCoefficient",&::PhysSoftBodyInfo::volumeConversationCoefficient);
	classDef.def_readwrite("softVsRigidImpulseSplitK",&::PhysSoftBodyInfo::softVsRigidImpulseSplitK);
	classDef.def_readwrite("softVsRigidImpulseSplitR",&::PhysSoftBodyInfo::softVsRigidImpulseSplitR);
	classDef.def_readwrite("softVsRigidImpulseSplitS",&::PhysSoftBodyInfo::softVsRigidImpulseSplitS);
	classDef.def_readwrite("softVsKineticHardness",&::PhysSoftBodyInfo::softVsKineticHardness);
	classDef.def_readwrite("softVsRigidHardness",&::PhysSoftBodyInfo::softVsRigidHardness);
	classDef.def_readwrite("softVsSoftHardness",&::PhysSoftBodyInfo::softVsSoftHardness);
	classDef.def_readwrite("pressureCoefficient",&::PhysSoftBodyInfo::pressureCoefficient);
	classDef.def_readwrite("velocitiesCorrectionFactor",&::PhysSoftBodyInfo::velocitiesCorrectionFactor);
	classDef.def_readwrite("bendingConstraintsDistance",&::PhysSoftBodyInfo::bendingConstraintsDistance);
	classDef.def_readwrite("clusterCount",&::PhysSoftBodyInfo::clusterCount);
	classDef.def_readwrite("maxClusterIterations",&::PhysSoftBodyInfo::maxClusterIterations);
	classDef.def("SetMaterialStiffnessCoefficients",static_cast<void(*)(lua_State*,::PhysSoftBodyInfo&,uint32_t,float,float,float)>([](lua_State *l,::PhysSoftBodyInfo &sbInfo,uint32_t matId,float linear,float angular,float volume) {
		auto it = sbInfo.materialStiffnessCoefficient.find(matId);
		if(it == sbInfo.materialStiffnessCoefficient.end())
			it = sbInfo.materialStiffnessCoefficient.insert(std::make_pair(matId,::PhysSoftBodyInfo::MaterialStiffnessCoefficient{})).first;
		it->second.linear = linear;
		it->second.angular = angular;
		it->second.volume = volume;
	}));
	classDef.def("GetMaterialStiffnessCoefficients",static_cast<void(*)(lua_State*,::PhysSoftBodyInfo&,uint32_t)>([](lua_State *l,::PhysSoftBodyInfo &sbInfo,uint32_t matId) {
		auto it = sbInfo.materialStiffnessCoefficient.find(matId);
		if(it == sbInfo.materialStiffnessCoefficient.end())
			return;
		Lua::PushNumber(l,it->second.linear);
		Lua::PushNumber(l,it->second.angular);
		Lua::PushNumber(l,it->second.volume);
	}));
}
