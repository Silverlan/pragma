// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:entities.components.base_io;

export import :entities.components.base;
import :entities.output;

export {
	namespace pragma {
		struct DLLNETWORK CEInputData : public ComponentEvent {
			CEInputData(const std::string &input, ecs::BaseEntity *activator, ecs::BaseEntity *caller, const std::string &data);
			virtual void PushArguments(lua::State *l) override;
			const std::string &input;
			ecs::BaseEntity *const activator;
			ecs::BaseEntity *const caller;
			const std::string &data;
		};
		namespace baseIOComponent {
			REGISTER_COMPONENT_EVENT(EVENT_HANDLE_INPUT)
		}
		class DLLNETWORK BaseIOComponent : public BaseEntityComponent {
		  public:
			enum class IoFlags : uint32_t {
				None = 0u,
				ForceDelayedFire = 1u // Never triggers the input/output immediately, even if the delay is set to 0
			};

			static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
			virtual void Initialize() override;

			void StoreOutput(std::string name, std::string entities, std::string input, std::string param, float delay = 0.f, int times = -1);
			void StoreOutput(std::string name, std::string info);
			void TriggerOutput(std::string name, ecs::BaseEntity *activator, IoFlags flags = IoFlags::None);

			virtual void Input(std::string input, ecs::BaseEntity *activator, ecs::BaseEntity *caller, std::string data);
			void Input(const std::string input, ecs::BaseEntity *activator = nullptr, ecs::BaseEntity *caller = nullptr);
			virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		  protected:
			virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
			BaseIOComponent(ecs::BaseEntity &ent);
			bool FireSingleOutput(Output &output, ecs::BaseEntity *activator, IoFlags flags = IoFlags::None);

			std::unordered_map<std::string, std::vector<Output>> m_outputs;
		};
		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::BaseIOComponent::IoFlags)
};
