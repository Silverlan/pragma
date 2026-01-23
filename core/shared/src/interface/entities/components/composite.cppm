// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.composite;

export import :entities.components.base;

export namespace pragma::ecs {
	class CompositeComponent;
	class DLLNETWORK CompositeGroup {
	  public:
		using UuidHash = size_t;
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
		std::unordered_map<UuidHash, EntityHandle> &GetEntities() { return m_ents; }
		const std::unordered_map<UuidHash, EntityHandle> &GetEntities() const { return const_cast<CompositeGroup *>(this)->GetEntities(); }
		std::vector<std::unique_ptr<CompositeGroup>> &GetChildGroups() { return m_childGroups; }
		const std::vector<std::unique_ptr<CompositeGroup>> &GetChildGroups() const { return const_cast<CompositeGroup *>(this)->GetChildGroups(); }
		CompositeGroup &AddChildGroup(const std::string &groupName);
		CompositeGroup *FindChildGroup(const std::string &name);
		void ClearEntities(bool safely = true);
	  private:
		std::unordered_map<UuidHash, EntityHandle>::const_iterator FindEntity(BaseEntity &ent) const;
		std::string m_groupName;
		std::unordered_map<UuidHash, EntityHandle> m_ents;
		std::vector<std::unique_ptr<CompositeGroup>> m_childGroups;
		CompositeGroup *m_parent = nullptr;
		CompositeComponent *m_compositeComponent = nullptr;
	};

	namespace compositeComponent {
		REGISTER_COMPONENT_EVENT(EVENT_ON_ENTITY_ADDED)
		REGISTER_COMPONENT_EVENT(EVENT_ON_ENTITY_REMOVED)
	};
	class DLLNETWORK CompositeComponent final : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		CompositeComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void OnRemove() override;

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;

		CompositeGroup &GetRootCompositeGroup() { return *m_rootGroup; }
		const CompositeGroup &GetRootCompositeGroup() const { return const_cast<CompositeComponent *>(this)->GetRootCompositeGroup(); }
		void ClearEntities(bool safely = true);

		virtual void InitializeLuaObject(lua::State *l) override;
	  protected:
		std::unique_ptr<CompositeGroup> m_rootGroup = nullptr;
	};

	namespace events {
		struct DLLNETWORK CECompositeEntityChanged : public ComponentEvent {
			CECompositeEntityChanged(CompositeGroup &group, BaseEntity &ent);
			virtual void PushArguments(lua::State *l) override;
			CompositeGroup &group;
			BaseEntity &ent;
		};
	};
};
