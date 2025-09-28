// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <sharedutils/property/util_property.hpp>

export module pragma.shared:entities.components.submergible;

export import :entities.components.base;

export namespace pragma {
	class DLLNETWORK SubmergibleComponent final : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_ON_WATER_SUBMERGED;
		static ComponentEventId EVENT_ON_WATER_EMERGED;
		static ComponentEventId EVENT_ON_WATER_ENTERED;
		static ComponentEventId EVENT_ON_WATER_EXITED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		SubmergibleComponent(BaseEntity &ent);
		virtual void Initialize() override;

		bool IsSubmerged() const;
		bool IsFullySubmerged() const;
		bool IsInWater() const;
		float GetSubmergedFraction() const;
		void SetSubmergedFraction(BaseEntity &waterEntity, float fraction);

		BaseEntity *GetWaterEntity();
		const BaseEntity *GetWaterEntity() const;
		const util::PFloatProperty &GetSubmergedFractionProperty() const;
		virtual void InitializeLuaObject(lua_State *l) override;
	  protected:
		util::PFloatProperty m_submergedFraction;
		EntityHandle m_waterEntity = {};
		virtual void OnWaterEntered();
		virtual void OnWaterExited();
		virtual void OnWaterSubmerged();
		virtual void OnWaterEmerged();
	};
};
