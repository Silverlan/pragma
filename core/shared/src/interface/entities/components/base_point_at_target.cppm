// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"

export module pragma.shared:entities.components.base_point_at_target;

export import :entities.components.base;
export import :entities.property;

export namespace pragma {
	class DLLNETWORK BasePointAtTargetComponent : public BaseEntityComponent {
	  public:
		virtual void Initialize() override;
		virtual void OnRemove() override;

		const pragma::PEntityProperty &GetPointAtTargetProperty() const;
		void SetPointAtTarget(BaseEntity &ent);
		void ClearPointAtTarget();
		BaseEntity *GetPointAtTarget() const;
	  protected:
		BasePointAtTargetComponent(BaseEntity &ent);
		void UpdatePose();
		virtual void OnEntitySpawn() override;
		virtual void SetPointAtTarget(BaseEntity *ent);

		std::string m_kvPointAtTargetName = "";
		pragma::PEntityProperty m_pointAtTarget;
		pragma::NetEventId m_netEvSetPointAtTarget = pragma::INVALID_NET_EVENT;
		CallbackHandle m_cbOnPoseChanged = {};
		CallbackHandle m_cbOnPoseChangedThis = {};
	};
};
