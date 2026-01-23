// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:physics.soft_body_info;

export import std.compat;

export namespace pragma::physics {
#pragma pack(push, 1)
	struct DLLNETWORK PhysSoftBodyInfo {
		struct MaterialStiffnessCoefficient {
			MaterialStiffnessCoefficient() = default;
			MaterialStiffnessCoefficient(float a, float l, float v) : MaterialStiffnessCoefficient()
			{
				angular = a;
				linear = l;
				volume = v;
			}
			float angular = 1.f;
			float linear = 1.f;
			float volume = 1.f;

			bool operator==(const MaterialStiffnessCoefficient &other) const { return angular == other.angular && linear == other.linear && volume == other.volume; }
			bool operator!=(const MaterialStiffnessCoefficient &other) const { return !operator==(other); }
		};
		float poseMatchingCoefficient = 0.5f;
		float anchorsHardness = 0.6999f;
		float dragCoefficient = 0.f;
		float rigidContactsHardness = 1.f;
		float softContactsHardness = 1.f;
		float liftCoefficient = 0.f;
		float kineticContactsHardness = 0.1f;
		float dynamicFrictionCoefficient = 0.2f;
		float dampingCoefficient = 0.f;
		float volumeConversationCoefficient = 0.f;
		float softVsRigidImpulseSplitK = 0.5f;
		float softVsRigidImpulseSplitR = 0.5f;
		float softVsRigidImpulseSplitS = 0.5f;
		float softVsKineticHardness = 1.f;
		float softVsRigidHardness = 0.1f;
		float softVsSoftHardness = 0.5f;
		float pressureCoefficient = 0.f;
		float velocitiesCorrectionFactor = 1.f;

		float bendingConstraintsDistance = 0.2f;
		uint32_t clusterCount = 0u;
		uint32_t maxClusterIterations = 8192u;
		std::unordered_map<uint32_t, MaterialStiffnessCoefficient> materialStiffnessCoefficient;

		bool operator==(const PhysSoftBodyInfo &other) const;
		bool operator!=(const PhysSoftBodyInfo &other) const { return !operator==(other); }
	};
#pragma pack(pop)
};
