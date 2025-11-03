// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"

#include "pragma/lua/core.hpp"


export module pragma.shared:entities.components.parent;

export import :entities.components.base;
export import :entities.components.base_child;

export namespace pragma {
		namespace parentComponent {
		STATIC_DLL_COMPAT ComponentEventId EVENT_ON_CHILD_ADDED;
		STATIC_DLL_COMPAT ComponentEventId EVENT_ON_CHILD_REMOVED;
	}
class DLLNETWORK ParentComponent final : public BaseEntityComponent {
	  public:

		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		ParentComponent(pragma::ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *lua) override;
		virtual void OnRemove() override;

		const std::vector<pragma::ComponentHandle<BaseChildComponent>> &GetChildren() const;
		std::vector<pragma::ComponentHandle<BaseChildComponent>> &GetChildren();
	  protected:
		friend BaseChildComponent;
		void AddChild(BaseChildComponent &ent);
		void RemoveChild(BaseChildComponent &ent);

		std::vector<pragma::ComponentHandle<BaseChildComponent>> m_children = {};
	};
	struct DLLNETWORK CEOnChildAdded : public ComponentEvent {
		CEOnChildAdded(BaseChildComponent &child);
		virtual void PushArguments(lua_State *l) override;
		BaseChildComponent &child;
	};
	using CEOnChildRemoved = CEOnChildAdded;
};
