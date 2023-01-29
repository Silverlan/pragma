/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __COMPOSITE_COMPONENT_HPP__
#define __COMPOSITE_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/baseentity_handle.h"

namespace pragma {
	class CompositeComponent;
	class DLLNETWORK CompositeGroup {
	  public:
		CompositeGroup(CompositeComponent &compositeC, const std::string &name);
		~CompositeGroup();
		CompositeGroup(const CompositeGroup &) = delete;
		CompositeGroup(CompositeGroup &&) = delete;
		CompositeGroup &operator=(const CompositeGroup &) = delete;
		CompositeGroup &operator=(CompositeGroup &&) = delete;
		void AddEntity(BaseEntity &ent);
		void RemoveEntity(BaseEntity &ent);
		const std::string &GetGroupName() const { return m_groupName; }
		void SetGroupName(const std::string &name) { m_groupName = name; }
		std::vector<EntityHandle> &GetEntities() { return m_ents; }
		const std::vector<EntityHandle> &GetEntities() const { return const_cast<CompositeGroup *>(this)->GetEntities(); }
		std::vector<std::unique_ptr<CompositeGroup>> &GetChildGroups() { return m_childGroups; }
		const std::vector<std::unique_ptr<CompositeGroup>> &GetChildGroups() const { return const_cast<CompositeGroup *>(this)->GetChildGroups(); }
		CompositeGroup &AddChildGroup(const std::string &groupName);
		CompositeGroup *FindChildGroup(const std::string &name);
		void ClearEntities(bool safely = true);
	  private:
		std::vector<EntityHandle>::const_iterator FindEntity(BaseEntity &ent) const;
		std::string m_groupName;
		std::vector<EntityHandle> m_ents;
		std::vector<std::unique_ptr<CompositeGroup>> m_childGroups;
		CompositeGroup *m_parent = nullptr;
		CompositeComponent *m_compositeComponent = nullptr;
	};

	class DLLNETWORK CompositeComponent final : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_ON_ENTITY_ADDED;
		static ComponentEventId EVENT_ON_ENTITY_REMOVED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		CompositeComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void OnRemove() override;

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;

		CompositeGroup &GetRootCompositeGroup() { return *m_rootGroup; }
		const CompositeGroup &GetRootCompositeGroup() const { return const_cast<CompositeComponent *>(this)->GetRootCompositeGroup(); }
		void ClearEntities(bool safely = true);

		virtual void InitializeLuaObject(lua_State *l) override;
	  protected:
		std::unique_ptr<CompositeGroup> m_rootGroup = nullptr;
	};

	struct DLLNETWORK CECompositeEntityChanged : public ComponentEvent {
		CECompositeEntityChanged(CompositeGroup &group, BaseEntity &ent);
		virtual void PushArguments(lua_State *l) override;
		CompositeGroup &group;
		BaseEntity &ent;
	};
};

#endif
