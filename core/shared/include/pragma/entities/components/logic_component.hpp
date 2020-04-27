/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __LOGIC_COMPONENT_HPP__
#define __LOGIC_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"

namespace pragma
{
	struct DLLNETWORK CEOnTick
		: public ComponentEvent
	{
		CEOnTick(double dt);
		virtual void PushArguments(lua_State *l) override;
		double deltaTime;
	};
	class DLLNETWORK LogicComponent final
		: public BaseEntityComponent
	{
	public:
		static ComponentEventId EVENT_ON_TICK;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);
		enum class TickPolicy : uint8_t
		{
			Never = 0u,
			WhenVisible,
			Always
		};

		LogicComponent(BaseEntity &ent);
		virtual void Initialize() override;

		virtual void Think(double tDelta);
		virtual void PostThink();

		double GetNextThink() const;
		void SetNextThink(double t);

		double LastThink() const;
		double DeltaTime() const;

		virtual void Save(DataStream &ds) override;
		virtual void Load(DataStream &ds,uint32_t version) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual void OnEntitySpawn() override;
	protected:
		TickPolicy m_tickPolicy = TickPolicy::WhenVisible;
		double m_lastThink = 0.0;
		double m_tNextThink = 0.0;
	};
};

#endif
