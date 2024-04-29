/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan */

#ifndef __FLEX_MERGE_COMPONENT_HPP__
#define __FLEX_MERGE_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"

class Model;
namespace pragma {
	class BaseFlexComponent;
	class DLLNETWORK FlexMergeComponent final : public BaseEntityComponent {
	  public:
		static bool can_merge(const Model &mdl, const Model &mdlParent);
		static ComponentEventId EVENT_ON_TARGET_CHANGED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		FlexMergeComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void OnRemove() override;
		virtual void OnTick(double tDelta) override;

		void SetTarget(const pragma::EntityURef &target);
		const pragma::EntityURef &GetTarget() const;

		virtual void InitializeLuaObject(lua_State *lua) override;
	  private:
		std::unordered_map<animation::FlexControllerId, animation::FlexControllerId> m_flexControllerMap;
		pragma::EntityURef m_target;
		pragma::ComponentHandle<BaseFlexComponent> m_flexC;
		pragma::ComponentHandle<BaseFlexComponent> m_flexCParent;
		CallbackHandle m_cbOnFlexControllerChanged;
		CallbackHandle m_cbOnFlexControllerComponentRemoved;
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;
		void SetTargetDirty(bool updateImmediately = true);
		void UpdateFlexControllerMappings();
		void ApplyFlexController(animation::FlexControllerId flexCId, float value);
		void MergeFlexControllers();
	};
};

#endif
