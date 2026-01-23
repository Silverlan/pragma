// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:entities.components.base_flex;

export import :entities.components.base;
export import :model.animation.enums;

export {
	namespace pragma {
		namespace asset {
			class Model;
		}
		namespace baseFlexComponent {
			REGISTER_COMPONENT_EVENT(EVENT_ON_FLEX_CONTROLLER_CHANGED)
		}
		class DLLNETWORK BaseFlexComponent : public BaseEntityComponent, public DynamicMemberRegister {
		  public:
			enum class StateFlags : uint8_t {
				None = 0u,
				EnableFlexControllerLimits = 1u,
				EnableFlexControllerUpdateListeners = EnableFlexControllerLimits << 1u,
			};

			static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
			static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
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
			void OnModelChanged(const std::shared_ptr<asset::Model> &model);
			virtual std::optional<ComponentMemberIndex> DoGetMemberIndex(const std::string &name) const override;
			BaseFlexComponent(ecs::BaseEntity &ent);
			float m_flexControllerScale = 1.f;
			StateFlags m_stateFlags = StateFlags::EnableFlexControllerLimits;
		};
		struct DLLNETWORK CEOnFlexControllerChanged : public ComponentEvent {
			CEOnFlexControllerChanged(animation::FlexControllerId flexControllerId, float value);
			virtual void PushArguments(lua::State *l) override;
			animation::FlexControllerId flexControllerId;
			float value;
		};
		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::BaseFlexComponent::StateFlags)
};
