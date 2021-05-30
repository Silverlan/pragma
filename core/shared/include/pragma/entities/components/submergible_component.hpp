/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __SUBMERGIBLE_COMPONENT_HPP__
#define __SUBMERGIBLE_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include <sharedutils/property/util_property.hpp>

class DamageInfo;
namespace pragma
{
	class BaseFuncWaterComponent;
	class DLLNETWORK SubmergibleComponent final
		: public BaseEntityComponent
	{
	public:
		static ComponentEventId EVENT_ON_WATER_SUBMERGED;
		static ComponentEventId EVENT_ON_WATER_EMERGED;
		static ComponentEventId EVENT_ON_WATER_ENTERED;
		static ComponentEventId EVENT_ON_WATER_EXITED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);

		SubmergibleComponent(BaseEntity &ent);
		virtual void Initialize() override;

		bool IsSubmerged() const;
		bool IsFullySubmerged() const;
		bool IsInWater() const;
		float GetSubmergedFraction() const;
		void SetSubmergedFraction(BaseEntity &waterEntity,float fraction);

		BaseEntity *GetWaterEntity();
		const BaseEntity *GetWaterEntity() const;
		const util::PFloatProperty &GetSubmergedFractionProperty() const;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	protected:
		util::PFloatProperty m_submergedFraction;
		EntityHandle m_waterEntity = {};
		virtual void OnWaterEntered();
		virtual void OnWaterExited();
		virtual void OnWaterSubmerged();
		virtual void OnWaterEmerged();
	};
};

#endif
