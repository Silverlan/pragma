/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ENTITY_UUID_REF_HPP__
#define __ENTITY_UUID_REF_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/baseentity_handle.h"
#include "pragma/types.hpp"
#include "pragma/entities/component_member_reference.hpp"
#include <sharedutils/util.h>
#include <variant>

class Game;
class EntityIterator;
namespace pragma {
	using EntityIdentifier = std::variant<util::Uuid, std::string>;
	struct DLLNETWORK EntityURef {
		EntityURef(const EntityURef &other);
		EntityURef(EntityURef &&other);
		EntityURef() : EntityURef {util::Uuid {}} {}
		EntityURef(EntityIdentifier identifier);
		EntityURef(const BaseEntity &ent);
		EntityURef &operator=(const EntityURef &other);
		EntityURef &operator=(EntityURef &&other);
		BaseEntity *GetEntity(Game &game);
		const BaseEntity *GetEntity(Game &game) const { return const_cast<EntityURef *>(this)->GetEntity(game); }

		bool HasEntityReference() const;
		static void AttachEntityFilter(EntityIterator &it, const EntityIdentifier &identifier);
		const EntityIdentifier *GetIdentifier() const { return m_identifier.get(); }
		std::optional<util::Uuid> GetUuid() const;
		std::optional<std::string> GetClassOrName() const;
	  protected:
		std::unique_ptr<EntityIdentifier> m_identifier = nullptr;
		EntityHandle m_hEntity;
	};

	struct DLLNETWORK EntityUComponentRef : EntityURef {
		EntityUComponentRef();
		EntityUComponentRef(EntityIdentifier identifier, ComponentId componentId);
		EntityUComponentRef(EntityIdentifier identifier, const std::string &componentName);
		EntityUComponentRef(const BaseEntity &ent, ComponentId componentId);
		EntityUComponentRef(const BaseEntity &ent, const std::string &componentName);
		EntityUComponentRef(const EntityUComponentRef &other);
		EntityUComponentRef(EntityUComponentRef &&other);
		EntityUComponentRef &operator=(const EntityUComponentRef &other);
		EntityUComponentRef &operator=(EntityUComponentRef &&other);
		BaseEntityComponent *GetComponent(Game &game);
		const BaseEntityComponent *GetComponent(Game &game) const { return const_cast<EntityUComponentRef *>(this)->GetComponent(game); }

		bool HasComponentReference() const;
		ComponentId GetComponentId() const { return m_componentId; }
		const std::string *GetComponentName() const { return m_componentName.get(); }
	  protected:
		ComponentId m_componentId = INVALID_COMPONENT_ID;
		std::unique_ptr<std::string> m_componentName = nullptr;
		ComponentHandle<BaseEntityComponent> m_hComponent;
	};

	struct DLLNETWORK EntityUComponentMemberRef : EntityUComponentRef {
		EntityUComponentMemberRef();
		EntityUComponentMemberRef(EntityIdentifier identifier, ComponentId componentId, const std::string &memberName);
		EntityUComponentMemberRef(EntityIdentifier identifier, const std::string &componentName, const std::string &memberName);
		EntityUComponentMemberRef(const BaseEntity &ent, ComponentId componentId, const std::string &memberName);
		EntityUComponentMemberRef(const BaseEntity &ent, const std::string &componentName, const std::string &memberName);
		EntityUComponentMemberRef(const EntityUComponentMemberRef &) = default;
		EntityUComponentMemberRef(const std::string &path);
		const ComponentMemberInfo *GetMemberInfo(Game &game) const;

		EntityUComponentMemberRef &operator=(const EntityUComponentMemberRef &) = default;
		ComponentMemberReference &operator*() { return m_memberRef; }
		const ComponentMemberReference &operator*() const { return const_cast<EntityUComponentMemberRef *>(this)->operator*(); }
		ComponentMemberReference *operator->() { return &m_memberRef; }
		const ComponentMemberReference *operator->() const { return const_cast<EntityUComponentMemberRef *>(this)->operator->(); }

		bool HasMemberReference() const;
	  protected:
		ComponentMemberReference m_memberRef;
	};

	//////////////////

	struct DLLNETWORK MultiEntityURef {
		MultiEntityURef() : MultiEntityURef {util::Uuid {}} {}
		MultiEntityURef(const MultiEntityURef &other);
		MultiEntityURef(MultiEntityURef &&other);
		MultiEntityURef(EntityIdentifier identifier);
		MultiEntityURef(const BaseEntity &ent);
		MultiEntityURef &operator=(const MultiEntityURef &other);
		MultiEntityURef &operator=(MultiEntityURef &&other);
		void FindEntities(Game &game, std::vector<BaseEntity *> &outEnts) const;

		bool HasEntityReference() const;
		const EntityIdentifier *GetIdentifier() const { return m_identifier.get(); }
	  protected:
		std::unique_ptr<EntityIdentifier> m_identifier = nullptr;
	};

	struct DLLNETWORK MultiEntityUComponentRef : MultiEntityURef {
		MultiEntityUComponentRef();
		MultiEntityUComponentRef(EntityIdentifier identifier, ComponentId componentId);
		MultiEntityUComponentRef(EntityIdentifier identifier, const std::string &componentName);
		MultiEntityUComponentRef(const BaseEntity &ent, ComponentId componentId);
		MultiEntityUComponentRef(const BaseEntity &ent, const std::string &componentName);
		MultiEntityUComponentRef(const MultiEntityUComponentRef &other);
		MultiEntityUComponentRef(MultiEntityUComponentRef &&other);
		MultiEntityUComponentRef &operator=(const MultiEntityUComponentRef &other);
		MultiEntityUComponentRef &operator=(MultiEntityUComponentRef &&other);
		void FindComponents(Game &game, std::vector<BaseEntityComponent *> &outComponents) const;

		bool HasComponentReference() const;
		ComponentId GetComponentId() const { return m_componentId; }
		const std::string *GetComponentName() const { return m_componentName.get(); }
	  protected:
		ComponentId m_componentId = INVALID_COMPONENT_ID;
		std::unique_ptr<std::string> m_componentName = nullptr;
		ComponentHandle<BaseEntityComponent> m_hComponent;
	};
};

#endif
