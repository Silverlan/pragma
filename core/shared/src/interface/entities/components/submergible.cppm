// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.submergible;

export import :entities.components.base;

export namespace pragma {
	namespace submergibleComponent {
		REGISTER_COMPONENT_EVENT(EVENT_ON_WATER_SUBMERGED)
		REGISTER_COMPONENT_EVENT(EVENT_ON_WATER_EMERGED)
		REGISTER_COMPONENT_EVENT(EVENT_ON_WATER_ENTERED)
		REGISTER_COMPONENT_EVENT(EVENT_ON_WATER_EXITED)
	}
	class DLLNETWORK SubmergibleComponent final : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		SubmergibleComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;

		bool IsSubmerged() const;
		bool IsFullySubmerged() const;
		bool IsInWater() const;
		float GetSubmergedFraction() const;
		void SetSubmergedFraction(ecs::BaseEntity &waterEntity, float fraction);

		ecs::BaseEntity *GetWaterEntity();
		const ecs::BaseEntity *GetWaterEntity() const;
		const util::PFloatProperty &GetSubmergedFractionProperty() const;
		virtual void InitializeLuaObject(lua::State *l) override;
	  protected:
		util::PFloatProperty m_submergedFraction;
		EntityHandle m_waterEntity = {};
		virtual void OnWaterEntered();
		virtual void OnWaterExited();
		virtual void OnWaterSubmerged();
		virtual void OnWaterEmerged();
	};
};
