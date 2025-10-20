// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"

export module pragma.shared:entities.components.flex_merge;

export import :entities.components.base;
export import :entities.components.base_flex;
export import :entities.universal_reference;
export import :model.animation.enums;
export import :model.animation.flex_animation;
export import :types;

export {
	namespace pragma {
		class DLLNETWORK FlexMergeComponent final : public BaseEntityComponent {
		  public:
			static bool can_merge(const pragma::Model &mdl, const pragma::Model &mdlParent);
			static ComponentEventId EVENT_ON_TARGET_CHANGED;
			static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
			static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

			FlexMergeComponent(pragma::ecs::BaseEntity &ent);
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
};
