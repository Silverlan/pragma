// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.flex_merge;

export import :entities.components.base;
export import :entities.components.base_flex;
export import :entities.universal_reference;
export import :model.animation.enums;
export import :model.animation.flex_animation;
export import :types;

export {
	namespace pragma {
		namespace flexMergeComponent {
			REGISTER_COMPONENT_EVENT(EVENT_ON_TARGET_CHANGED)
		}
		class DLLNETWORK FlexMergeComponent final : public BaseEntityComponent {
		  public:
			static bool can_merge(const asset::Model &mdl, const asset::Model &mdlParent);

			static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
			static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

			FlexMergeComponent(ecs::BaseEntity &ent);
			virtual void Initialize() override;
			virtual void OnRemove() override;
			virtual void OnTick(double tDelta) override;

			void SetTarget(const EntityURef &target);
			const EntityURef &GetTarget() const;

			virtual void InitializeLuaObject(lua::State *lua) override;
		  private:
			std::unordered_map<animation::FlexControllerId, animation::FlexControllerId> m_flexControllerMap;
			EntityURef m_target;
			ComponentHandle<BaseFlexComponent> m_flexC;
			ComponentHandle<BaseFlexComponent> m_flexCParent;
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
