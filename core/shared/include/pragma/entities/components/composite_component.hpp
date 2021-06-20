/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __COMPOSITE_COMPONENT_HPP__
#define __COMPOSITE_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/baseentity_handle.h"

namespace pragma
{
	class DLLNETWORK CompositeComponent final
		: public BaseEntityComponent
	{
	public:
		static ComponentEventId EVENT_ON_ENTITY_ADDED;
		static ComponentEventId EVENT_ON_ENTITY_REMOVED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);

		CompositeComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void OnRemove() override;

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm,uint32_t version) override;

		void AddEntity(BaseEntity &ent);
		void RemoveEntity(BaseEntity &ent);
		const std::vector<EntityHandle> &GetEntities() const;
		void ClearEntities(bool safely=true);

		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	protected:
		std::vector<EntityHandle>::const_iterator FindEntity(BaseEntity &ent) const;
		std::vector<EntityHandle> m_ents;
	};

	struct DLLNETWORK CECompositeEntityChanged
		: public ComponentEvent
	{
		CECompositeEntityChanged(BaseEntity &ent);
		virtual void PushArguments(lua_State *l) override;
		BaseEntity &ent;
	};
};

#endif
