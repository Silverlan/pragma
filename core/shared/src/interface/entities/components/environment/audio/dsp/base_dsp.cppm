// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:entities.components.environment.audio.dsp.base_dsp;

export import :entities.components.base;

export {
	namespace pragma {
		class DLLNETWORK BaseEnvSoundDspComponent : public BaseEntityComponent {
		  public:
			enum class DLLNETWORK SpawnFlags : uint32_t {
				None = 0,
				AffectRelative = 4,
				Effects = AffectRelative << 1,
				Music = Effects << 1,
				Voices = Music << 1,
				Weapons = Voices << 1,
				NPCs = Weapons << 1,
				Players = NPCs << 1,
				Vehicles = Players << 1,
				Physics = Vehicles << 2, // Shift by 2 bits, because 1024 is already taken by BaseToggle
				Environment = Physics << 1,
				GUI = Environment << 1,
				World = GUI << 1,
				All = World << 1,
				ApplyGlobally = All << 1
			};
			using BaseEntityComponent::BaseEntityComponent;
			virtual void Initialize() override;
			void SetDSPEffect(const std::string &identifier);
			void SetInnerRadius(float radius);
			void SetOuterRadius(float radius);
			bool Input(const std::string &input, ecs::BaseEntity *activator, ecs::BaseEntity *caller, std::string data);
			float GetGain() const;
			virtual void SetGain(float gain);
		  protected:
			virtual bool OnSetKeyValue(const std::string &key, const std::string &val);

			NetEventId m_netEvSetGain = INVALID_NET_EVENT;
			std::string m_kvDsp = "";
			float m_kvInnerRadius = 0.f;
			float m_kvOuterRadius = 0.f;
			float m_kvDspGain = 1.f;
		};
		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::BaseEnvSoundDspComponent::SpawnFlags);
};
