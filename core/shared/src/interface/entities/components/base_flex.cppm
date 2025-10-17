// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <string>

export module pragma.shared:entities.components.base_flex;

export import :entities.components.base;
export import :model.animation.enums;

export {
	class Model;
	namespace pragma {
		class DLLNETWORK BaseFlexComponent : public BaseEntityComponent, public DynamicMemberRegister {
		public:
			enum class StateFlags : uint8_t {
				None = 0u,
				EnableFlexControllerLimits = 1u,
				EnableFlexControllerUpdateListeners = EnableFlexControllerLimits << 1u,
			};
			static ComponentEventId EVENT_ON_FLEX_CONTROLLER_CHANGED;
			static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
			static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
			virtual void Initialize() override;

			void SetFlexControllerUpdateListenersEnabled(bool enabled);
			bool AreFlexControllerUpdateListenersEnabled() const;

			// Flex Controllers
			void SetFlexController(const std::string &name, float val, float duration = 0.f, bool clampToLimits = true);
			virtual void SetFlexController(uint32_t flexId, float val, float duration = 0.f, bool clampToLimits = true) {}; // TODO /* =0 */
			float GetFlexController(uint32_t flexId) const;
			float GetFlexController(const std::string &flexController) const;
			virtual bool GetFlexController(uint32_t flexId, float &val) const
			{
				val = 0.f;
				return true;
			}; // TODO /* =0 */
			bool GetScaledFlexController(uint32_t flexId, float &val) const;

			void SetFlexControllerScale(float scale);
			float GetFlexControllerScale() const;

			void SetFlexControllerLimitsEnabled(bool enabled);
			bool AreFlexControllerLimitsEnabled() const;

			virtual const ComponentMemberInfo *GetMemberInfo(ComponentMemberIndex idx) const override;
		protected:
			void OnModelChanged(const std::shared_ptr<Model> &model);
			virtual std::optional<ComponentMemberIndex> DoGetMemberIndex(const std::string &name) const override;
			BaseFlexComponent(BaseEntity &ent);
			float m_flexControllerScale = 1.f;
			StateFlags m_stateFlags = StateFlags::EnableFlexControllerLimits;
		};
		struct DLLNETWORK CEOnFlexControllerChanged : public ComponentEvent {
			CEOnFlexControllerChanged(pragma::animation::FlexControllerId flexControllerId, float value);
			virtual void PushArguments(lua_State *l) override;
			pragma::animation::FlexControllerId flexControllerId;
			float value;
		};
	};
	REGISTER_BASIC_BITWISE_OPERATORS(pragma::BaseFlexComponent::StateFlags)
};
