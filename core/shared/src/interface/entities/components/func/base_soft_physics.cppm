// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.func.base_soft_physics;

export import :entities.components.base;
export import :physics.soft_body_info;

export namespace pragma {
	class DLLNETWORK BaseFuncSoftPhysicsComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		physics::PhysSoftBodyInfo *GetSoftBodyInfo();
	  protected:
		bool SetKeyValue(std::string key, std::string val);
		physics::PhysSoftBodyInfo m_softBodyInfo = {};
	};
};
