// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.parent;

export import :entities.components.base;
export import :entities.components.base_child;

export namespace pragma {
	namespace parentComponent {
		REGISTER_COMPONENT_EVENT(EVENT_ON_CHILD_ADDED)
		REGISTER_COMPONENT_EVENT(EVENT_ON_CHILD_REMOVED)
	}
	class DLLNETWORK ParentComponent final : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		ParentComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua::State *lua) override;
		virtual void OnRemove() override;

		const std::vector<ComponentHandle<BaseChildComponent>> &GetChildren() const;
		std::vector<ComponentHandle<BaseChildComponent>> &GetChildren();
	  protected:
		friend BaseChildComponent;
		void AddChild(BaseChildComponent &ent);
		void RemoveChild(BaseChildComponent &ent);

		std::vector<ComponentHandle<BaseChildComponent>> m_children = {};
	};
	struct DLLNETWORK CEOnChildAdded : public ComponentEvent {
		CEOnChildAdded(BaseChildComponent &child);
		virtual void PushArguments(lua::State *l) override;
		BaseChildComponent &child;
	};
	using CEOnChildRemoved = CEOnChildAdded;
};
