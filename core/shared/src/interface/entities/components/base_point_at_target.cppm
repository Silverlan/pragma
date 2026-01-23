// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_point_at_target;

export import :entities.components.base;
export import :entities.property;

export namespace pragma {
	class DLLNETWORK BasePointAtTargetComponent : public BaseEntityComponent {
	  public:
		virtual void Initialize() override;
		virtual void OnRemove() override;

		const PEntityProperty &GetPointAtTargetProperty() const;
		void SetPointAtTarget(ecs::BaseEntity &ent);
		void ClearPointAtTarget();
		ecs::BaseEntity *GetPointAtTarget() const;
	  protected:
		BasePointAtTargetComponent(ecs::BaseEntity &ent);
		void UpdatePose();
		virtual void OnEntitySpawn() override;
		virtual void SetPointAtTarget(ecs::BaseEntity *ent);

		std::string m_kvPointAtTargetName = "";
		PEntityProperty m_pointAtTarget;
		NetEventId m_netEvSetPointAtTarget = INVALID_NET_EVENT;
		CallbackHandle m_cbOnPoseChanged = {};
		CallbackHandle m_cbOnPoseChangedThis = {};
	};
};
