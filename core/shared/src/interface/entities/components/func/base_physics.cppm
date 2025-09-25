// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/entities/func/basefuncsurfacematerial.hpp"

export module pragma.shared:entities.components.func.base_physics;

export namespace pragma {
	class DLLNETWORK BaseFuncPhysicsComponent : public BaseFuncSurfaceMaterialComponent {
	  public:
		using BaseFuncSurfaceMaterialComponent::BaseFuncSurfaceMaterialComponent;
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
	  protected:
		virtual PhysObj *InitializePhysics();

		float m_kvMass = 0.f;
		bool m_bClientsidePhysics = false;
	};
};
