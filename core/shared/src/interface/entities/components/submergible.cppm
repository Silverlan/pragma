// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.submergible;

export import :entities.components.base;

export namespace pragma {
	namespace submergibleComponent {
		STATIC_DLL_COMPAT ComponentEventId EVENT_ON_WATER_SUBMERGED;
		STATIC_DLL_COMPAT ComponentEventId EVENT_ON_WATER_EMERGED;
		STATIC_DLL_COMPAT ComponentEventId EVENT_ON_WATER_ENTERED;
		STATIC_DLL_COMPAT ComponentEventId EVENT_ON_WATER_EXITED;
	}
	class DLLNETWORK SubmergibleComponent final : public BaseEntityComponent {
	  public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		SubmergibleComponent(pragma::ecs::BaseEntity &ent);
		virtual void Initialize() override;

		bool IsSubmerged() const;
		bool IsFullySubmerged() const;
		bool IsInWater() const;
		float GetSubmergedFraction() const;
		void SetSubmergedFraction(pragma::ecs::BaseEntity &waterEntity, float fraction);

		pragma::ecs::BaseEntity *GetWaterEntity();
		const pragma::ecs::BaseEntity *GetWaterEntity() const;
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
