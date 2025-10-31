// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <mathutil/umath_random.hpp>
#include "mathutil/umath.h"


export module pragma.client:particle_system.modifier_random_variable;

import :particle_system.particle;
import pragma.shared;

export {
	template<class TUniformDis, typename T>
	class DLLCLIENT CParticleModifierComponentRandomVariable {
	public:
		CParticleModifierComponentRandomVariable() = default;
		CParticleModifierComponentRandomVariable(const std::string &identifier, const std::unordered_map<std::string, std::string> &values);
		void Initialize(const std::string &identifier, const std::unordered_map<std::string, std::string> &values);
		template<typename U = T>
		typename std::enable_if<std::is_integral<T>::value, U>::type GetValue(CParticle &p) const;
		template<typename U = T>
		typename std::enable_if<std::is_floating_point<T>::value, U>::type GetValue(CParticle &p) const;

		void SetRange(T min, T max);
		void SetRange(T val);
		void SetMin(T val);
		void SetMax(T val);
		T GetMin() const;
		T GetMax() const;
		bool IsSet() const;
	private:
		uint32_t m_iSeed = umath::random_int(0u, std::numeric_limits<uint32_t>::max());
		TUniformDis m_value = TUniformDis(T(0), T(0));
		bool m_bIsSet = false;
	};
};

template<class TUniformDis, typename T>
CParticleModifierComponentRandomVariable<TUniformDis, T>::CParticleModifierComponentRandomVariable(const std::string &identifier, const std::unordered_map<std::string, std::string> &values)
{
	Initialize(identifier, values);
}
template<class TUniformDis, typename T>
void CParticleModifierComponentRandomVariable<TUniformDis, T>::Initialize(const std::string &identifier, const std::unordered_map<std::string, std::string> &values)
{
	auto it = values.find(identifier);
	if(it != values.end()) {
		m_bIsSet = true;
		T min, max;
		// Dirty hack
		if(std::is_floating_point<T>::value)
			util::to_random_float(it->second, reinterpret_cast<float &>(min), reinterpret_cast<float &>(max));
		else
			util::to_random_int(it->second, reinterpret_cast<int32_t &>(min), reinterpret_cast<int32_t &>(max));
		m_value = TUniformDis(min, max);
	}
}
template<class TUniformDis, typename T>
template<typename U>
typename std::enable_if<std::is_integral<T>::value, U>::type CParticleModifierComponentRandomVariable<TUniformDis, T>::GetValue(CParticle &p) const
{
	return p.PseudoRandomInt(m_value, m_iSeed);
}
template<class TUniformDis, typename T>
template<typename U>
typename std::enable_if<std::is_floating_point<T>::value, U>::type CParticleModifierComponentRandomVariable<TUniformDis, T>::GetValue(CParticle &p) const
{
	return p.PseudoRandomReal(m_value, m_iSeed);
}
template<class TUniformDis, typename T>
bool CParticleModifierComponentRandomVariable<TUniformDis, T>::IsSet() const
{
	return m_bIsSet;
}

template<class TUniformDis, typename T>
void CParticleModifierComponentRandomVariable<TUniformDis, T>::SetRange(T min, T max)
{
	m_value = TUniformDis(min, max);
	m_bIsSet = true;
}

template<class TUniformDis, typename T>
void CParticleModifierComponentRandomVariable<TUniformDis, T>::SetRange(T val)
{
	SetRange(val, val);
}

template<class TUniformDis, typename T>
void CParticleModifierComponentRandomVariable<TUniformDis, T>::SetMin(T val)
{
	m_value = TUniformDis(val, m_value.max());
	m_bIsSet = true;
}

template<class TUniformDis, typename T>
void CParticleModifierComponentRandomVariable<TUniformDis, T>::SetMax(T val)
{
	m_value = TUniformDis(m_value.min(), val);
	m_bIsSet = true;
}

template<class TUniformDis, typename T>
T CParticleModifierComponentRandomVariable<TUniformDis, T>::GetMin() const
{
	return m_value.min();
}

template<class TUniformDis, typename T>
T CParticleModifierComponentRandomVariable<TUniformDis, T>::GetMax() const
{
	return m_value.max();
}
