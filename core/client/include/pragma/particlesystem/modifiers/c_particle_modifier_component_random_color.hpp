#ifndef __C_PARTICLE_MODIFIER_COMPONENT_RANDOM_COLOR_HPP__
#define __C_PARTICLE_MODIFIER_COMPONENT_RANDOM_COLOR_HPP__

#include "pragma/particlesystem/modifiers/c_particle_modifier_component_random_variable.hpp"

class DLLCLIENT CParticleModifierComponentRandomColor
{
public:
	enum class ComponentFlags : uint8_t
	{
		None = 0,
		Red = 1u,
		Green = Red<<1,
		Blue = Green<<1,
		Alpha = Blue<<1,
		RGBA = Red | Green | Blue | Alpha
	};
	CParticleModifierComponentRandomColor(const std::string &suffix,const std::unordered_map<std::string,std::string> &values);
	void Initialize(const std::string &suffix,const std::unordered_map<std::string,std::string> &values);
	Color GetValue(CParticle &p) const;
	bool IsSet() const;
	bool IsSet(ComponentFlags components) const;
	ComponentFlags GetComponentFlags() const;
private:
	CParticleModifierComponentRandomVariable<std::uniform_int_distribution<int16_t>,int16_t> m_red;
	CParticleModifierComponentRandomVariable<std::uniform_int_distribution<int16_t>,int16_t> m_green;
	CParticleModifierComponentRandomVariable<std::uniform_int_distribution<int16_t>,int16_t> m_blue;
	CParticleModifierComponentRandomVariable<std::uniform_int_distribution<int16_t>,int16_t> m_alpha;
};
REGISTER_BASIC_BITWISE_OPERATORS(CParticleModifierComponentRandomColor::ComponentFlags);

#endif
