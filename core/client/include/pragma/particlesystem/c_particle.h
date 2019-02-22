#ifndef __C_PARTICLE_H__
#define __C_PARTICLE_H__

#include "pragma/clientdefinitions.h"
#include <mathutil/glmutil.h>
#include <mathutil/color.h>
#include <random>

#pragma warning(push)
#pragma warning(disable : 4251)
class DLLCLIENT CParticleSystemBaseKeyValues
{
public:
	CParticleSystemBaseKeyValues()=default;
	void RecordKeyValues(const std::unordered_map<std::string,std::string> &values);
	const std::unordered_map<std::string,std::string> *GetKeyValues() const;
	bool IsRecordingKeyValues() const;
private:
	std::unique_ptr<std::unordered_map<std::string,std::string>> m_keyValues;
};

class DLLCLIENT CParticle
{
private:
	Vector3 m_pos = {};
	Quat m_rot = uquat::identity(); // Only used by model renderer and physics
	// Optional relative origin, which is rotated by the particle's rotation, and added to its position before
	// being written to the render buffer (= Position modifier)
	Vector3 m_origin = {};
	Vector3 m_velocity = {};
	Vector3 m_angularVelocity = {};
	float m_radius = 0.f;
	float m_length = 0.f;
	bool m_bHasLength = false;
	float m_tLife = 0.f;
	float m_tAlive = 0.f;
	float m_tCreated = 0.f;
	float m_rotation = 0.f;
	Color m_color = {};
	float m_camDist = 0.f;
	bool m_bIsAlive = false;
	float m_frameOffset = 0.f;
	uint32_t m_index = 0u;
	uint32_t m_seed = 0u;
	mutable std::mt19937 m_mt;

	bool m_bDying = false;
	float m_tDeath = 0.f;
public:
	CParticle();
	~CParticle();

	uint32_t GetIndex() const;
	void SetIndex(uint32_t idx);
	bool IsAlive() const;
	void SetAlive(bool b);
	bool ShouldDraw() const;
	float GetRotation() const;
	void SetRotation(float rotation);
	const Vector3 &GetPosition() const;
	const Vector3 &GetVelocity() const;
	const Vector3 &GetAngularVelocity() const;
	float GetLife() const;
	void SetLife(float life);
	float GetTimeAlive() const;
	void SetTimeAlive(float alive);
	float GetTimeCreated() const;
	void SetTimeCreated(float time);
	void SetColor(const Color &col);
	void SetColor(uint8_t r,uint8_t g,uint8_t b,uint8_t a);
	void SetPosition(const Vector3 &pos);
	void SetVelocity(const Vector3 &vel);
	void SetAngularVelocity(const Vector3 &vel);
	const Color &GetColor() const;
	Color &GetColor();
	float GetRadius() const;
	float GetLength() const;
	float GetExtent() const;
	void SetRadius(float radius);
	void SetLength(float length);
	void SetCameraDistance(float dist);
	float GetCameraDistance() const;
	void Reset(float t);
	void SetFrameOffset(float offset);
	float GetFrameOffset() const;

	void Die();
	void Resurrect();
	bool IsDying() const;
	// Returns the time at which the particle has died
	float GetDeathTime() const;
	// Returns the total lifespan of the particle up to the point where it will die
	float GetLifeSpan() const;

	const Quat &GetWorldRotation() const;
	void SetWorldRotation(const Quat &rot);
	const Vector3 &GetOrigin() const;
	void SetOrigin(const Vector3 &origin);

	uint32_t GetSeed() const;
	// Generates a random int, but gurantees to always return the same int for a specific particle and a specific seed
	template<typename T,typename = std::enable_if_t<std::is_integral<T>::value>>
		T PseudoRandomInt(T min,T max,uint32_t seed=0u) const;
	template<typename T,typename = std::enable_if_t<std::is_integral<T>::value>>
		T PseudoRandomInt(const std::uniform_int_distribution<T> &dis,uint32_t seed=0u) const;
	// Generates a random real, but gurantees to always return the same real for a specific particle and a specific seed
	template<typename T,typename = std::enable_if_t<std::is_floating_point<T>::value>>
		T PseudoRandomReal(T min,T max,uint32_t seed=0u) const;
	template<typename T,typename = std::enable_if_t<std::is_floating_point<T>::value>>
		T PseudoRandomReal(const std::uniform_real_distribution<T> &dis,uint32_t seed=0u) const;
};
#pragma warning(pop)

template<typename T,typename = std::enable_if_t<std::is_integral<T>::value>>
	T CParticle::PseudoRandomInt(const std::uniform_int_distribution<T> &dis,uint32_t seed) const
{
	m_mt.seed(m_seed +seed);
	return const_cast<std::uniform_int_distribution<T>&>(dis)(m_mt);
}

template<typename T,typename>
	T CParticle::PseudoRandomInt(T min,T max,uint32_t seed) const
{
	return PseudoRandomInt(std::uniform_int_distribution<T>(min,max),seed);
}

template<typename T,typename = std::enable_if_t<std::is_floating_point<T>::value>>
	T CParticle::PseudoRandomReal(const std::uniform_real_distribution<T> &dis,uint32_t seed) const
{
	m_mt.seed(m_seed +seed);
	return const_cast<std::uniform_real_distribution<T>&>(dis)(m_mt);
}

template<typename T,typename>
	T CParticle::PseudoRandomReal(T min,T max,uint32_t seed) const
{
	return PseudoRandomReal(std::uniform_real_distribution<T>(min,max),seed);
}

inline bool operator<(const CParticle &a,const CParticle &b)
{return a.GetCameraDistance() > b.GetCameraDistance();}

#endif