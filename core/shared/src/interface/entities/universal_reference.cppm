// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.universal_reference;

export import :entities.base_entity_handle;
export import :entities.components.handle;
export import :entities.enums;
export import :entities.member_reference;
export import :types;

export {
	namespace pragma {
		using EntityIdentifier = std::variant<util::Uuid, std::string>;
		struct DLLNETWORK EntityURef {
			EntityURef(const EntityURef &other);
			EntityURef(EntityURef &&other);
			EntityURef() : EntityURef {util::Uuid {}} {}
			EntityURef(EntityIdentifier identifier);
			EntityURef(const ecs::BaseEntity &ent);
			EntityURef &operator=(const EntityURef &other);
			EntityURef &operator=(EntityURef &&other);
			ecs::BaseEntity *GetEntity(Game &game);
			const ecs::BaseEntity *GetEntity(Game &game) const { return const_cast<EntityURef *>(this)->GetEntity(game); }

			bool HasEntityReference() const;
			static void AttachEntityFilter(ecs::EntityIterator &it, const EntityIdentifier &identifier);
			const EntityIdentifier *GetIdentifier() const { return m_identifier.get(); }
			std::optional<util::Uuid> GetUuid() const;
			std::optional<std::string> GetClassOrName() const;
			std::string ToString() const;
		  protected:
			void ToStringArgs(std::stringstream &outSs) const;
			std::unique_ptr<EntityIdentifier> m_identifier = nullptr;
			EntityHandle m_hEntity;
		};

		struct DLLNETWORK EntityUComponentRef : EntityURef {
			EntityUComponentRef();
			EntityUComponentRef(EntityIdentifier identifier, ComponentId componentId);
			EntityUComponentRef(EntityIdentifier identifier, const std::string &componentName);
			EntityUComponentRef(const ecs::BaseEntity &ent, ComponentId componentId);
			EntityUComponentRef(const ecs::BaseEntity &ent, const std::string &componentName);
			EntityUComponentRef(const EntityUComponentRef &other);
			EntityUComponentRef(EntityUComponentRef &&other);
			EntityUComponentRef &operator=(const EntityUComponentRef &other);
			EntityUComponentRef &operator=(EntityUComponentRef &&other);
			BaseEntityComponent *GetComponent(Game &game);
			const BaseEntityComponent *GetComponent(Game &game) const { return const_cast<EntityUComponentRef *>(this)->GetComponent(game); }

			bool HasComponentReference() const;
			ComponentId GetComponentId() const { return m_componentId; }
			const std::string *GetComponentName() const { return m_componentName.get(); }
			std::string ToString() const;
		  protected:
			void ToStringArgs(std::stringstream &outSs) const;
			ComponentId m_componentId = INVALID_COMPONENT_ID;
			std::unique_ptr<std::string> m_componentName = nullptr;
			ComponentHandle<BaseEntityComponent> m_hComponent;
		};

		struct DLLNETWORK EntityUComponentMemberRef : EntityUComponentRef {
			EntityUComponentMemberRef();
			EntityUComponentMemberRef(EntityIdentifier identifier, ComponentId componentId, const std::string &memberName);
			EntityUComponentMemberRef(EntityIdentifier identifier, const std::string &componentName, const std::string &memberName);
			EntityUComponentMemberRef(const ecs::BaseEntity &ent, ComponentId componentId, const std::string &memberName);
			EntityUComponentMemberRef(const ecs::BaseEntity &ent, const std::string &componentName, const std::string &memberName);
			EntityUComponentMemberRef(const EntityUComponentMemberRef &) = default;
			EntityUComponentMemberRef(const std::string &path);
			const ComponentMemberInfo *GetMemberInfo(Game &game) const;
			ComponentMemberIndex GetMemberIndex() const;
			void UpdateMemberIndex(Game &game) const;
			const std::string &GetMemberName() const;

			EntityUComponentMemberRef &operator=(const EntityUComponentMemberRef &) = default;
			ComponentMemberReference &operator*() { return m_memberRef; }
			const ComponentMemberReference &operator*() const { return const_cast<EntityUComponentMemberRef *>(this)->operator*(); }
			ComponentMemberReference *operator->() { return &m_memberRef; }
			const ComponentMemberReference *operator->() const { return const_cast<EntityUComponentMemberRef *>(this)->operator->(); }

			bool HasMemberReference() const;
			std::string ToString() const;
		  protected:
			void ToStringArgs(std::stringstream &outSs) const;
			ComponentMemberReference m_memberRef;
		};

		//////////////////

		struct DLLNETWORK MultiEntityURef {
			MultiEntityURef() : MultiEntityURef {util::Uuid {}} {}
			MultiEntityURef(const MultiEntityURef &other);
			MultiEntityURef(MultiEntityURef &&other);
			MultiEntityURef(EntityIdentifier identifier);
			MultiEntityURef(const ecs::BaseEntity &ent);
			MultiEntityURef &operator=(const MultiEntityURef &other);
			MultiEntityURef &operator=(MultiEntityURef &&other);
			void FindEntities(Game &game, std::vector<ecs::BaseEntity *> &outEnts) const;

			bool HasEntityReference() const;
			const EntityIdentifier *GetIdentifier() const { return m_identifier.get(); }
			std::string ToString() const;
		  protected:
			void ToStringArgs(std::stringstream &outSs) const;
			std::unique_ptr<EntityIdentifier> m_identifier = nullptr;
		};

		struct DLLNETWORK MultiEntityUComponentRef : MultiEntityURef {
			MultiEntityUComponentRef();
			MultiEntityUComponentRef(EntityIdentifier identifier, ComponentId componentId);
			MultiEntityUComponentRef(EntityIdentifier identifier, const std::string &componentName);
			MultiEntityUComponentRef(const ecs::BaseEntity &ent, ComponentId componentId);
			MultiEntityUComponentRef(const ecs::BaseEntity &ent, const std::string &componentName);
			MultiEntityUComponentRef(const MultiEntityUComponentRef &other);
			MultiEntityUComponentRef(MultiEntityUComponentRef &&other);
			MultiEntityUComponentRef &operator=(const MultiEntityUComponentRef &other);
			MultiEntityUComponentRef &operator=(MultiEntityUComponentRef &&other);
			void FindComponents(Game &game, std::vector<BaseEntityComponent *> &outComponents) const;

			bool HasComponentReference() const;
			ComponentId GetComponentId() const { return m_componentId; }
			const std::string *GetComponentName() const { return m_componentName.get(); }
			std::string ToString() const;
		  protected:
			void ToStringArgs(std::stringstream &outSs) const;
			ComponentId m_componentId = INVALID_COMPONENT_ID;
			std::unique_ptr<std::string> m_componentName = nullptr;
			ComponentHandle<BaseEntityComponent> m_hComponent;
		};
	};

	std::ostream &operator<<(std::ostream &out, const pragma::EntityURef &ref);
	std::ostream &operator<<(std::ostream &out, const pragma::EntityUComponentRef &ref);
	std::ostream &operator<<(std::ostream &out, const pragma::EntityUComponentMemberRef &ref);
	std::ostream &operator<<(std::ostream &out, const pragma::MultiEntityURef &ref);
	std::ostream &operator<<(std::ostream &out, const pragma::MultiEntityUComponentRef &ref);
};
