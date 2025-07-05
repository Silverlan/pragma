// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PARENT_COMPONENT_HPP__
#define __PARENT_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"

struct AttachmentData;
namespace pragma {
	class BaseChildComponent;
	class DLLNETWORK ParentComponent final : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_ON_CHILD_ADDED;
		static ComponentEventId EVENT_ON_CHILD_REMOVED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		ParentComponent(BaseEntity &ent);
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

#endif
