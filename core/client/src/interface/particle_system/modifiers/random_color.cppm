// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"

export module pragma.client:particle_system.modifier_random_color;

import :particle_system.modifier_random_variable;

export namespace pragma::pts {
	class DLLCLIENT CParticleModifierComponentRandomColor {
	  public:
		enum class ComponentFlags : uint8_t { None = 0, Red = 1u, Green = Red << 1, Blue = Green << 1, Alpha = Blue << 1, RGBA = Red | Green | Blue | Alpha };
		CParticleModifierComponentRandomColor() = default;
		CParticleModifierComponentRandomColor(const std::string &suffix, const std::unordered_map<std::string, std::string> &values);
		void Initialize(const std::string &suffix, const std::unordered_map<std::string, std::string> &values);
		Color GetValue(CParticle &p) const;
		bool IsSet() const;
		bool IsSet(ComponentFlags components) const;
		ComponentFlags GetComponentFlags() const;
	  private:
		CParticleModifierComponentRandomVariable<std::uniform_int_distribution<int16_t>, int16_t> m_red;
		CParticleModifierComponentRandomVariable<std::uniform_int_distribution<int16_t>, int16_t> m_green;
		CParticleModifierComponentRandomVariable<std::uniform_int_distribution<int16_t>, int16_t> m_blue;
		CParticleModifierComponentRandomVariable<std::uniform_int_distribution<int16_t>, int16_t> m_alpha;
	};
};
export {
	REGISTER_ENUM_FLAGS(pragma::pts::CParticleModifierComponentRandomColor::ComponentFlags)
}

pragma::pts::CParticleModifierComponentRandomColor::CParticleModifierComponentRandomColor(const std::string &suffix, const std::unordered_map<std::string, std::string> &values) { Initialize(suffix, values); }

void pragma::pts::CParticleModifierComponentRandomColor::Initialize(const std::string &suffix, const std::unordered_map<std::string, std::string> &values)
{
	auto strSuffix = suffix.empty() ? suffix : ("_" + suffix);
	auto idColor = "color" + strSuffix;
	auto idRed = "red" + strSuffix;
	auto idGreen = "green" + strSuffix;
	auto idBlue = "blue" + strSuffix;
	auto idAlpha = "alpha" + strSuffix;
	for(auto it = values.begin(); it != values.end(); it++) {
		auto key = it->first;
		string::to_lower(key);
		if(key == idColor) {
			auto color = Color(it->second);
			m_red.SetRange(color.r);
			m_green.SetRange(color.g);
			m_blue.SetRange(color.b);
			m_alpha.SetRange(color.a);
		}
		else if(key == idRed)
			m_red.SetRange(util::to_int(it->second));
		else if(key == idGreen)
			m_green.SetRange(util::to_int(it->second));
		else if(key == idBlue)
			m_blue.SetRange(util::to_int(it->second));
		else if(key == idAlpha)
			m_alpha.SetRange(util::to_int(it->second));
		else if(key == idColor + "_min") {
			auto color = Color(it->second);
			m_red.SetMin(color.r);
			m_green.SetMin(color.g);
			m_blue.SetMin(color.b);
			m_alpha.SetMin(color.a);
		}
		else if(key == idRed + "_min")
			m_red.SetMin(util::to_int(it->second));
		else if(key == idGreen + "_min")
			m_green.SetMin(util::to_int(it->second));
		else if(key == idBlue + "_min")
			m_blue.SetMin(util::to_int(it->second));
		else if(key == idAlpha + "_min")
			m_alpha.SetMin(util::to_int(it->second));
		else if(key == idColor + "_max") {
			auto color = Color(it->second);
			m_red.SetMax(color.r);
			m_green.SetMax(color.g);
			m_blue.SetMax(color.b);
			m_alpha.SetMax(color.a);
		}
		else if(key == idRed + "_max")
			m_red.SetMax(util::to_int(it->second));
		else if(key == idGreen + "_max")
			m_green.SetMax(util::to_int(it->second));
		else if(key == idBlue + "_max")
			m_blue.SetMax(util::to_int(it->second));
		else if(key == idAlpha + "_max")
			m_alpha.SetMax(util::to_int(it->second));
	}
}

Color pragma::pts::CParticleModifierComponentRandomColor::GetValue(CParticle &p) const { return Color(m_red.GetValue(p), m_green.GetValue(p), m_blue.GetValue(p), m_alpha.GetValue(p)); }

bool pragma::pts::CParticleModifierComponentRandomColor::IsSet() const { return m_red.IsSet() || m_green.IsSet() || m_blue.IsSet() || m_alpha.IsSet(); }

bool pragma::pts::CParticleModifierComponentRandomColor::IsSet(ComponentFlags components) const
{
	return ((components & ComponentFlags::Red) != ComponentFlags::None && m_red.IsSet()) || ((components & ComponentFlags::Green) != ComponentFlags::None && m_green.IsSet()) || ((components & ComponentFlags::Blue) != ComponentFlags::None && m_blue.IsSet())
	  || ((components & ComponentFlags::Alpha) != ComponentFlags::None && m_alpha.IsSet());
}

pragma::pts::CParticleModifierComponentRandomColor::ComponentFlags pragma::pts::CParticleModifierComponentRandomColor::GetComponentFlags() const
{
	auto componentFlags = ComponentFlags::None;
	if(m_red.IsSet())
		componentFlags |= ComponentFlags::Red;
	if(m_green.IsSet())
		componentFlags |= ComponentFlags::Green;
	if(m_blue.IsSet())
		componentFlags |= ComponentFlags::Blue;
	if(m_alpha.IsSet())
		componentFlags |= ComponentFlags::Alpha;
	return componentFlags;
}
