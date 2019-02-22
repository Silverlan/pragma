#include "stdafx_client.h"
#include "pragma/particlesystem/modifiers/c_particle_modifier_component_random_color.hpp"

CParticleModifierComponentRandomColor::CParticleModifierComponentRandomColor(const std::string &suffix,const std::unordered_map<std::string,std::string> &values)
{
	Initialize(suffix,values);
}

void CParticleModifierComponentRandomColor::Initialize(const std::string &suffix,const std::unordered_map<std::string,std::string> &values)
{
	auto strSuffix = suffix.empty() ? suffix : ("_" +suffix);
	auto idColor = "color" +strSuffix;
	auto idRed = "red" +strSuffix;
	auto idGreen = "green" +strSuffix;
	auto idBlue = "blue" +strSuffix;
	auto idAlpha = "alpha" +strSuffix;
	for(auto it=values.begin();it!=values.end();it++)
	{
		auto key = it->first;
		ustring::to_lower(key);
		if(key == idColor)
		{
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
		else if(key == idColor +"_min")
		{
			auto color = Color(it->second);
			m_red.SetMin(color.r);
			m_green.SetMin(color.g);
			m_blue.SetMin(color.b);
			m_alpha.SetMin(color.a);
		}
		else if(key == idRed +"_min")
			m_red.SetMin(util::to_int(it->second));
		else if(key == idGreen +"_min")
			m_green.SetMin(util::to_int(it->second));
		else if(key == idBlue +"_min")
			m_blue.SetMin(util::to_int(it->second));
		else if(key == idAlpha +"_min")
			m_alpha.SetMin(util::to_int(it->second));
		else if(key == idColor +"_max")
		{
			auto color = Color(it->second);
			m_red.SetMax(color.r);
			m_green.SetMax(color.g);
			m_blue.SetMax(color.b);
			m_alpha.SetMax(color.a);
		}
		else if(key == idRed +"_max")
			m_red.SetMax(util::to_int(it->second));
		else if(key == idGreen +"_max")
			m_green.SetMax(util::to_int(it->second));
		else if(key == idBlue +"_max")
			m_blue.SetMax(util::to_int(it->second));
		else if(key == idAlpha +"_max")
			m_alpha.SetMax(util::to_int(it->second));
	}
}

Color CParticleModifierComponentRandomColor::GetValue(CParticle &p) const
{
	return Color(
		m_red.GetValue(p),
		m_green.GetValue(p),
		m_blue.GetValue(p),
		m_alpha.GetValue(p)
	);
}

bool CParticleModifierComponentRandomColor::IsSet() const
{
	return m_red.IsSet() || m_green.IsSet() || m_blue.IsSet() || m_alpha.IsSet();
}

bool CParticleModifierComponentRandomColor::IsSet(ComponentFlags components) const
{
	return ((components &ComponentFlags::Red) != ComponentFlags::None && m_red.IsSet()) ||
		((components &ComponentFlags::Green) != ComponentFlags::None && m_green.IsSet()) ||
		((components &ComponentFlags::Blue) != ComponentFlags::None && m_blue.IsSet()) ||
		((components &ComponentFlags::Alpha) != ComponentFlags::None && m_alpha.IsSet());
}

CParticleModifierComponentRandomColor::ComponentFlags CParticleModifierComponentRandomColor::GetComponentFlags() const
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
