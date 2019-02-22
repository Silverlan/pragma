#include "stdafx_client.h"
#include "pragma/particlesystem/c_particle.h"
#include <mathutil/umath_random.hpp>

void CParticleSystemBaseKeyValues::RecordKeyValues(const std::unordered_map<std::string,std::string> &values)
{
	m_keyValues = std::make_unique<std::unordered_map<std::string,std::string>>(values);
}
bool CParticleSystemBaseKeyValues::IsRecordingKeyValues() const {return m_keyValues != nullptr;}
const std::unordered_map<std::string,std::string> *CParticleSystemBaseKeyValues::GetKeyValues() const {return m_keyValues.get();}

CParticle::CParticle()
	: m_index(0)
{Reset(0.f);}

CParticle::~CParticle()
{}

void CParticle::Reset(float t)
{
	m_radius = 0;
	m_rotation = 0;
	m_tLife = 0;
	m_color.r = 255;
	m_color.g = 255;
	m_color.b = 255;
	m_color.a = 255;
	m_camDist = -1;
	m_pos.x = 0;
	m_pos.y = 0;
	m_pos.z = 0;
	m_velocity.x = 0;
	m_velocity.y = 0;
	m_velocity.z = 0;
	m_tCreated = t;
	m_tAlive = 0.f;
	m_bIsAlive = false;
	m_frameOffset = 0.f;
	m_rot = {};
	m_origin = {};
	m_angularVelocity = {};
	m_seed = umath::random_int<decltype(m_seed)>(0u,std::numeric_limits<decltype(m_seed)>::max());
}

uint32_t CParticle::GetSeed() const {return m_seed;}
void CParticle::Die() {m_bDying = true; m_tDeath = m_tAlive;}
void CParticle::Resurrect() {m_bDying = false ;m_tDeath = 0.f;}
bool CParticle::IsDying() const {return m_bDying;}
float CParticle::GetDeathTime() const {return m_tDeath;}
float CParticle::GetLifeSpan() const {return GetTimeAlive() +GetLife();}
const Quat &CParticle::GetWorldRotation() const {return m_rot;}
void CParticle::SetWorldRotation(const Quat &rot) {m_rot = rot;}
const Vector3 &CParticle::GetOrigin() const {return m_origin;}
void CParticle::SetOrigin(const Vector3 &origin) {m_origin = origin;}

float CParticle::GetFrameOffset() const {return m_frameOffset;}
void CParticle::SetFrameOffset(float offset) {m_frameOffset = offset;}
void CParticle::SetIndex(uint32_t idx) {m_index = idx;}
uint32_t CParticle::GetIndex() const {return m_index;}
bool CParticle::IsAlive() const {return m_bIsAlive;}
void CParticle::SetAlive(bool b) {m_bIsAlive = b;}
bool CParticle::ShouldDraw() const {return (GetLife() > 0.f && GetRadius() > 0.f && m_color.a > 0) ? true : false;}
float CParticle::GetRotation() const {return m_rotation;}
void CParticle::SetRotation(float rotation) {m_rotation = rotation;}
float CParticle::GetTimeAlive() const {return m_tAlive;}
void CParticle::SetTimeAlive(float alive) {m_tAlive = alive;}
float CParticle::GetTimeCreated() const {return m_tCreated;}
void CParticle::SetTimeCreated(float time) {m_tCreated = time;}

const Vector3 &CParticle::GetPosition() const {return m_pos;}
const Vector3 &CParticle::GetVelocity() const {return m_velocity;}
const Vector3 &CParticle::GetAngularVelocity() const {return m_angularVelocity;}
float CParticle::GetLife() const {return m_tLife;}
void CParticle::SetLife(float life) {m_tLife = life;}
void CParticle::SetColor(const Color &col) {m_color = col;}
void CParticle::SetColor(uint8_t r,uint8_t g,uint8_t b,uint8_t a)
{
	m_color.r = r;
	m_color.g = g;
	m_color.b = b;
	m_color.a = a;
}
const Color &CParticle::GetColor() const {return m_color;}
Color &CParticle::GetColor() {return m_color;}
void CParticle::SetPosition(const Vector3 &pos) {m_pos = pos;}
void CParticle::SetVelocity(const Vector3 &vel) {m_velocity = vel;}
void CParticle::SetAngularVelocity(const Vector3 &vel) {m_angularVelocity = vel;}
void CParticle::SetRadius(float size) {m_radius = size;}
void CParticle::SetLength(float length)
{
	m_length = length;
	m_bHasLength = true;
}
float CParticle::GetRadius() const {return m_radius;}
float CParticle::GetLength() const {return m_bHasLength ? m_length : m_radius;}
float CParticle::GetExtent() const {return umath::sqrt(umath::pow2(m_radius) *2.f);}
void CParticle::SetCameraDistance(float dist) {m_camDist = dist;}
float CParticle::GetCameraDistance() const {return m_camDist;}