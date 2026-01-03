// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :particle_system.particle;

void pragma::pts::CParticleSystemBaseKeyValues::RecordKeyValues(const std::unordered_map<std::string, std::string> &values) { m_keyValues = std::make_unique<std::unordered_map<std::string, std::string>>(values); }
bool pragma::pts::CParticleSystemBaseKeyValues::IsRecordingKeyValues() const { return m_keyValues != nullptr; }
const std::unordered_map<std::string, std::string> *pragma::pts::CParticleSystemBaseKeyValues::GetKeyValues() const { return m_keyValues.get(); }

pragma::pts::CParticle::CParticle() : m_index(0) { Reset(0.f); }

pragma::pts::CParticle::~CParticle() {}

void pragma::pts::CParticle::Reset(float t)
{
	m_radius = 0;
	m_length = 0.f;
	m_rotation = 0;
	m_rotationYaw = 0.f;
	m_tLife = 0;
	m_color.r = 255;
	m_color.g = 255;
	m_color.b = 255;
	m_color.a = 255;
	m_camDist = -1;
	m_pos.x = 0;
	m_pos.y = 0;
	m_pos.z = 0;
	m_sequence = 0;
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
	m_seed = pragma::math::random_int<decltype(m_seed)>(0u, std::numeric_limits<decltype(m_seed)>::max());
	m_bHasLength = false;
	m_bDying = false;
	m_tDeath = 0.f;
	m_prevPos = {};

	m_initialRadius = 0.f;
	m_initialLength = 0.f;
	m_initialRotation = 0.f;
	m_initialRotationYaw = 0.f;
	m_initialLife = 0.f;
	m_initialColor = {};
	m_initialFrameOffset = 0.f;
}

uint32_t pragma::pts::CParticle::GetSeed() const { return m_seed; }
void pragma::pts::CParticle::Die()
{
	m_bDying = true;
	m_tDeath = m_tAlive;
}
void pragma::pts::CParticle::Resurrect()
{
	m_bDying = false;
	m_tDeath = 0.f;
}
bool pragma::pts::CParticle::IsDying() const { return m_bDying; }
float pragma::pts::CParticle::GetDeathTime() const { return m_tDeath; }
float pragma::pts::CParticle::GetLifeSpan() const { return GetTimeAlive() + GetLife(); }
const Quat &pragma::pts::CParticle::GetWorldRotation() const { return m_rot; }
void pragma::pts::CParticle::SetWorldRotation(const Quat &rot) { m_rot = rot; }
const Vector3 &pragma::pts::CParticle::GetOrigin() const { return m_origin; }
void pragma::pts::CParticle::SetOrigin(const Vector3 &origin) { m_origin = origin; }

/*enum class FieldId : uint8_t
{
	Length,
	Life,
	Color,
	Sequence,

	Invalid = std::numeric_limits<uint8_t>::max()
};*/
const char *pragma::pts::CParticle::field_id_to_name(FieldId id)
{
	switch(id) {
	case FieldId::Pos:
		return "pos";
	case FieldId::Rot:
		return "rot";
	case FieldId::RotYaw:
		return "rot_yaw";
	case FieldId::Origin:
		return "origin";
	case FieldId::Velocity:
		return "velocity";
	case FieldId::AngularVelocity:
		return "angular_velocity";
	case FieldId::Radius:
		return "radius";
	case FieldId::Length:
		return "length";
	case FieldId::Color:
		return "color";
	case FieldId::Alpha:
		return "alpha";
	case FieldId::Sequence:
		return "sequence";
	case FieldId::CreationTime:
		return "creation_time";
	case FieldId::Life:
		return "life";
	}
	static_assert(math::to_integral(FieldId::Count) == 13);
	return "invalid";
}
pragma::pts::CParticle::FieldId pragma::pts::CParticle::name_to_field_id(const std::string &fieldName)
{
	if(pragma::string::compare<std::string>(fieldName, "pos", false))
		return FieldId::Pos;
	else if(pragma::string::compare<std::string>(fieldName, "rot", false))
		return FieldId::Rot;
	else if(pragma::string::compare<std::string>(fieldName, "rot_yaw", false))
		return FieldId::RotYaw;
	else if(pragma::string::compare<std::string>(fieldName, "origin", false))
		return FieldId::Origin;
	else if(pragma::string::compare<std::string>(fieldName, "velocity", false))
		return FieldId::Velocity;
	else if(pragma::string::compare<std::string>(fieldName, "angular_velocity", false))
		return FieldId::AngularVelocity;
	else if(pragma::string::compare<std::string>(fieldName, "radius", false))
		return FieldId::Radius;
	else if(pragma::string::compare<std::string>(fieldName, "length", false))
		return FieldId::Length;
	else if(pragma::string::compare<std::string>(fieldName, "color", false))
		return FieldId::Color;
	else if(pragma::string::compare<std::string>(fieldName, "alpha", false))
		return FieldId::Alpha;
	else if(pragma::string::compare<std::string>(fieldName, "sequence", false))
		return FieldId::Sequence;
	else if(pragma::string::compare<std::string>(fieldName, "life", false))
		return FieldId::Life;
	else if(pragma::string::compare<std::string>(fieldName, "creation_time", false))
		return FieldId::CreationTime;
	static_assert(math::to_integral(FieldId::Count) == 13);
	return FieldId::Invalid;
}
void pragma::pts::CParticle::SetField(FieldId fieldId, float value)
{
	switch(fieldId) {
	case FieldId::Rot:
		m_rotation = value;
		break;
	case FieldId::RotYaw:
		m_rotationYaw = value;
		break;
	case FieldId::Radius:
		m_radius = value;
		break;
	case FieldId::Length:
		m_length = value;
		break;
	case FieldId::Sequence:
		m_sequence = static_cast<int32_t>(value);
		break;
	case FieldId::Life:
		m_tLife = value;
		break;
	case FieldId::CreationTime:
		m_tCreated = value;
		break;
	case FieldId::Alpha:
		m_color.a = value;
		break;
	}
	static_assert(math::to_integral(FieldId::Count) == 13);
}
void pragma::pts::CParticle::SetField(FieldId fieldId, const Vector4 &value)
{
	switch(fieldId) {
	case FieldId::Pos:
		m_pos = value;
		break;
	case FieldId::Origin:
		m_origin = value;
		break;
	case FieldId::Velocity:
		m_velocity = value;
		break;
	case FieldId::AngularVelocity:
		m_angularVelocity = value;
		break;
	case FieldId::Color:
		m_color.r = value.r;
		m_color.g = value.g;
		m_color.b = value.b;
		break;
	}
	static_assert(math::to_integral(FieldId::Count) == 13);
}
bool pragma::pts::CParticle::GetField(FieldId fieldId, float &outValue) const
{
	switch(fieldId) {
	case FieldId::Rot:
		outValue = m_rotation;
		break;
	case FieldId::RotYaw:
		outValue = m_rotationYaw;
		break;
	case FieldId::Radius:
		outValue = m_radius;
		break;
	case FieldId::Length:
		outValue = m_length;
		break;
	case FieldId::Sequence:
		outValue = static_cast<float>(m_sequence);
		break;
	case FieldId::Life:
		outValue = m_tLife;
		break;
	case FieldId::CreationTime:
		outValue = m_tCreated;
		break;
	case FieldId::Alpha:
		outValue = m_color.a;
		break;
	default:
		return false;
	}
	static_assert(math::to_integral(FieldId::Count) == 13);
	return true;
}
bool pragma::pts::CParticle::GetField(FieldId fieldId, Vector4 &outValue) const
{
	switch(fieldId) {
	case FieldId::Pos:
		outValue = Vector4 {m_pos, 0.f};
		break;
	case FieldId::Origin:
		outValue = Vector4 {m_origin, 0.f};
		break;
	case FieldId::Velocity:
		outValue = Vector4 {m_velocity, 0.f};
		break;
	case FieldId::AngularVelocity:
		outValue = Vector4 {m_angularVelocity, 0.f};
		break;
	case FieldId::Color:
		outValue = m_color;
		break;
	default:
		return false;
	}
	static_assert(math::to_integral(FieldId::Count) == 13);
	return true;
}

void pragma::pts::CParticle::SetSequence(uint32_t sequence) { m_sequence = sequence; }
uint32_t pragma::pts::CParticle::GetSequence() const { return m_sequence; }
float pragma::pts::CParticle::GetFrameOffset() const { return m_frameOffset; }
void pragma::pts::CParticle::SetFrameOffset(float offset) { m_frameOffset = offset; }
void pragma::pts::CParticle::SetIndex(uint32_t idx) { m_index = idx; }
uint32_t pragma::pts::CParticle::GetIndex() const { return m_index; }
bool pragma::pts::CParticle::IsAlive() const { return m_bIsAlive; }
void pragma::pts::CParticle::SetAlive(bool b) { m_bIsAlive = b; }
bool pragma::pts::CParticle::ShouldDraw() const { return (GetLife() > 0.f && GetRadius() > 0.f && m_color.a > 0) ? true : false; }
float pragma::pts::CParticle::GetRotation() const { return m_rotation; }
void pragma::pts::CParticle::SetRotation(float rotation) { m_rotation = rotation; }
float pragma::pts::CParticle::GetRotationYaw() const { return m_rotationYaw; }
void pragma::pts::CParticle::SetRotationYaw(float yaw) { m_rotationYaw = yaw; }
float pragma::pts::CParticle::GetTimeAlive() const { return m_tAlive; }
void pragma::pts::CParticle::SetTimeAlive(float alive) { m_tAlive = alive; }
float pragma::pts::CParticle::GetTimeCreated() const { return m_tCreated; }
void pragma::pts::CParticle::SetTimeCreated(float time) { m_tCreated = time; }

void pragma::pts::CParticle::SetPrevPos(const Vector3 &prevPos) { m_prevPos = prevPos; }
void pragma::pts::CParticle::PopulateInitialValues()
{
	m_initialRadius = m_radius;
	m_initialLength = m_length;
	m_initialRotation = m_rotation;
	m_initialLife = m_tLife;
	m_initialColor = m_color;
	m_initialFrameOffset = m_frameOffset;
}

const Vector3 &pragma::pts::CParticle::GetPrevPos() const { return m_prevPos; }
float pragma::pts::CParticle::GetInitialRadius() const { return m_initialRadius; }
float pragma::pts::CParticle::GetInitialLength() const { return m_initialLength; }
float pragma::pts::CParticle::GetInitialRotation() const { return m_initialRotation; }
float pragma::pts::CParticle::GetInitialLife() const { return m_initialLife; }
const Vector4 &pragma::pts::CParticle::GetInitialColor() const { return m_initialColor; }
float pragma::pts::CParticle::GetInitialFrameOffset() const { return m_initialFrameOffset; }

const Vector3 &pragma::pts::CParticle::GetPosition() const { return m_pos; }
const Vector3 &pragma::pts::CParticle::GetVelocity() const { return m_velocity; }
const Vector3 &pragma::pts::CParticle::GetAngularVelocity() const { return m_angularVelocity; }
float pragma::pts::CParticle::GetLife() const { return m_tLife; }
void pragma::pts::CParticle::SetLife(float life) { m_tLife = life; }
void pragma::pts::CParticle::SetColor(const Vector4 &col) { m_color = col; }
void pragma::pts::CParticle::SetColor(const Color &col) { SetColor(col.ToVector4()); }
const Vector4 &pragma::pts::CParticle::GetColor() const { return m_color; }
Vector4 &pragma::pts::CParticle::GetColor() { return m_color; }
void pragma::pts::CParticle::SetPosition(const Vector3 &pos) { m_pos = pos; }
void pragma::pts::CParticle::SetVelocity(const Vector3 &vel) { m_velocity = vel; }
void pragma::pts::CParticle::SetAngularVelocity(const Vector3 &vel) { m_angularVelocity = vel; }
void pragma::pts::CParticle::SetRadius(float size) { m_radius = size; }
void pragma::pts::CParticle::SetLength(float length)
{
	m_length = length;
	m_bHasLength = true;
}
float pragma::pts::CParticle::GetRadius() const { return m_radius; }
float pragma::pts::CParticle::GetLength() const { return m_bHasLength ? m_length : m_radius; }
float pragma::pts::CParticle::GetExtent() const { return math::sqrt(math::pow2(m_radius) * 2.f); }
void pragma::pts::CParticle::SetCameraDistance(float dist) { m_camDist = dist; }
float pragma::pts::CParticle::GetCameraDistance() const { return m_camDist; }
