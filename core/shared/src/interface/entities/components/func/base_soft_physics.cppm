// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/physics/physsoftbodyinfo.hpp"

export module pragma.shared:entities.components.func.base_soft_physics;

export namespace pragma {
	class DLLNETWORK BaseFuncSoftPhysicsComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		PhysSoftBodyInfo *GetSoftBodyInfo();
	  protected:
		bool SetKeyValue(std::string key, std::string val);
		PhysSoftBodyInfo m_softBodyInfo = {};
	};
};
