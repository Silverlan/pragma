// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.func.base_physics;

export import :entities.components.func.base_surface_material;
export import :physics;

export namespace pragma {
	class DLLNETWORK BaseFuncPhysicsComponent : public BaseFuncSurfaceMaterialComponent {
	  public:
		using BaseFuncSurfaceMaterialComponent::BaseFuncSurfaceMaterialComponent;
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
	  protected:
		virtual physics::PhysObj *InitializePhysics();

		float m_kvMass = 0.f;
		bool m_bClientsidePhysics = false;
	};
};
