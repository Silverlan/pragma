/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/c_lparticle_modifiers.hpp"
#include "pragma/particlesystem/c_particlemodifier.h"
#include "pragma/particlesystem/initializers/c_particle_initializer_lua.hpp"
#include "pragma/rendering/shaders/c_shader_lua.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/lua/converters/shader_converter_t.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <prosper_command_buffer.hpp>
#include <luasystem.h>

extern DLLCLIENT CEngine *c_engine;

CParticleModifierLua *pragma::LuaParticleModifierManager::CreateModifier(std::string className) const
{
	ustring::to_lower(className);
	auto it = m_modifiers.find(className);
	if(it == m_modifiers.end())
		return nullptr;
	auto &modInfo = it->second;
	auto &o = modInfo.luaClassObject;

	auto *l = o.interpreter();
	luabind::object r;
#ifndef LUABIND_NO_EXCEPTIONS
	try {
#endif
		r = o();
#ifndef LUABIND_NO_EXCEPTIONS
	}
	catch(luabind::error &) {
		::Lua::HandleLuaError(l);
		return nullptr;
	}
#endif
	if(!r) {
		Con::cwar << Con::PREFIX_CLIENT << "Unable to create lua particle modifier '" << className << "'!" << Con::endl;
		return nullptr;
	}

	CParticleModifierLua *modifier = nullptr;
	switch(modInfo.type) {
	case Type::Initializer:
		{
			auto *oModifier = luabind::object_cast_nothrow<CParticleInitializerLua *>(r, static_cast<CParticleInitializerLua *>(nullptr));
			modifier = dynamic_cast<CParticleModifierLua *>(oModifier);
			break;
		}
	case Type::Operator:
		{
			auto *oModifier = luabind::object_cast_nothrow<CParticleOperatorLua *>(r, static_cast<CParticleOperatorLua *>(nullptr));
			modifier = dynamic_cast<CParticleModifierLua *>(oModifier);
			break;
		}
	case Type::Renderer:
		{
			auto *oModifier = luabind::object_cast_nothrow<CParticleRendererLua *>(r, static_cast<CParticleRendererLua *>(nullptr));
			modifier = dynamic_cast<CParticleModifierLua *>(oModifier);
			break;
		}
	case Type::Emitter:
		break;
	}
	if(modifier == nullptr) {
		// TODO: Can we check this when the particle modifier is being registered?
		Con::cwar << Con::PREFIX_CLIENT << "Unable to create lua particle modifier '" << className << "': Lua class is not derived from valid particle modifier base!" << Con::endl;
		return nullptr;
	}
	r.push(l); /* 1 */
	auto idx = ::Lua::GetStackTop(l);

	modifier->Initialize(r);
	modifier->SetIdentifier(className);

	::Lua::Pop(l); /* 0 */
	return modifier;
}

bool pragma::LuaParticleModifierManager::RegisterModifier(Type type, std::string className, luabind::object &o)
{
	ustring::to_lower(className);
	auto itShader = m_modifiers.find(className);
	if(itShader != m_modifiers.end()) {
		Con::cwar << "Attempted to register particle modifier '" << className << "', which has already been registered previously! Ignoring..." << Con::endl;
		return false;
		//Con::cwar<<"Attempted to register particle modifier '"<<className<<"', which has already been registered previously! Overwriting previous definition..."<<Con::endl;
	}
	auto &pair = m_modifiers[className] = {};
	pair.luaClassObject = o;
	pair.type = type;
	return true;
}

luabind::object *pragma::LuaParticleModifierManager::GetClassObject(std::string className)
{
	ustring::to_lower(className);
	auto it = m_modifiers.find(className);
	if(it == m_modifiers.end())
		return nullptr;
	return &it->second.luaClassObject;
}

/////////////

void Lua::ParticleSystemModifier::register_particle_class(luabind::class_<pragma::CParticleSystemComponent, pragma::BaseEnvParticleSystemComponent> &defPtc)
{
	auto defPt = luabind::class_<::CParticle>("Particle");
	defPt.add_static_constant("FIELD_ID_POS", umath::to_integral(CParticle::FieldId::Pos));
	defPt.add_static_constant("FIELD_ID_ROT", umath::to_integral(CParticle::FieldId::Rot));
	defPt.add_static_constant("FIELD_ID_ROT_YAW", umath::to_integral(CParticle::FieldId::RotYaw));
	defPt.add_static_constant("FIELD_ID_ORIGIN", umath::to_integral(CParticle::FieldId::Origin));
	defPt.add_static_constant("FIELD_ID_VELOCITY", umath::to_integral(CParticle::FieldId::Velocity));
	defPt.add_static_constant("FIELD_ID_ANGULAR_VELOCITY", umath::to_integral(CParticle::FieldId::AngularVelocity));
	defPt.add_static_constant("FIELD_ID_RADIUS", umath::to_integral(CParticle::FieldId::Radius));
	defPt.add_static_constant("FIELD_ID_LENGTH", umath::to_integral(CParticle::FieldId::Length));
	defPt.add_static_constant("FIELD_ID_LIFE", umath::to_integral(CParticle::FieldId::Life));
	defPt.add_static_constant("FIELD_ID_COLOR", umath::to_integral(CParticle::FieldId::Color));
	defPt.add_static_constant("FIELD_ID_ALPHA", umath::to_integral(CParticle::FieldId::Alpha));
	defPt.add_static_constant("FIELD_ID_SEQUENCE", umath::to_integral(CParticle::FieldId::Sequence));
	defPt.add_static_constant("FIELD_ID_CREATION_TIME", umath::to_integral(CParticle::FieldId::CreationTime));
	defPt.add_static_constant("FIELD_ID_INVALID", umath::to_integral(CParticle::FieldId::Invalid));
	defPt.add_static_constant("FIELD_ID_COUNT", umath::to_integral(CParticle::FieldId::Count));
	static_assert(umath::to_integral(CParticle::FieldId::Count) == 13);
	defPt.scope[luabind::def("field_id_to_name", static_cast<void (*)(lua_State *, uint32_t)>([](lua_State *l, uint32_t id) {
		auto name = ::CParticle::field_id_to_name(static_cast<CParticle::FieldId>(id));
		Lua::PushString(l, name);
	}))];
	defPt.scope[luabind::def("name_to_field_id", static_cast<void (*)(lua_State *, const std::string &)>([](lua_State *l, const std::string &name) {
		auto id = ::CParticle::name_to_field_id(name);
		Lua::PushInt(l, umath::to_integral(id));
	}))];
	defPt.def("SetField", static_cast<void (*)(lua_State *, ::CParticle &, uint32_t, const Vector4 &)>([](lua_State *l, ::CParticle &pt, uint32_t fieldId, const Vector4 &value) { pt.SetField(static_cast<CParticle::FieldId>(fieldId), value); }));
	defPt.def("SetField", static_cast<void (*)(lua_State *, ::CParticle &, uint32_t, float)>([](lua_State *l, ::CParticle &pt, uint32_t fieldId, float value) { pt.SetField(static_cast<CParticle::FieldId>(fieldId), value); }));
	defPt.def("GetField", static_cast<void (*)(lua_State *, ::CParticle &, uint32_t)>([](lua_State *l, ::CParticle &pt, uint32_t fieldId) {
		float value;
		if(pt.GetField(static_cast<CParticle::FieldId>(fieldId), value) == false) {
			Vector4 vValue;
			if(pt.GetField(static_cast<CParticle::FieldId>(fieldId), vValue) == true) {
				Lua::Push<Vector4>(l, vValue);
				return;
			}
			return;
		}
		Lua::PushNumber(l, value);
	}));
	defPt.def("GetIndex", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::PushInt(l, pt.GetIndex()); }));
	defPt.def("IsAlive", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::PushBool(l, pt.IsAlive()); }));
	defPt.def("SetAlive", static_cast<void (*)(lua_State *, ::CParticle &, bool)>([](lua_State *l, ::CParticle &pt, bool alive) { pt.SetAlive(alive); }));
	defPt.def("ShouldDraw", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::PushBool(l, pt.ShouldDraw()); }));
	defPt.def("GetRotation", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::PushNumber(l, pt.GetRotation()); }));
	defPt.def("SetRotation", static_cast<void (*)(lua_State *, ::CParticle &, float)>([](lua_State *l, ::CParticle &pt, float rot) { pt.SetRotation(rot); }));
	defPt.def("GetRotationYaw", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::PushNumber(l, pt.GetRotationYaw()); }));
	defPt.def("SetRotationYaw", static_cast<void (*)(lua_State *, ::CParticle &, float)>([](lua_State *l, ::CParticle &pt, float rot) { pt.SetRotationYaw(rot); }));
	defPt.def("GetPosition", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::Push<Vector3>(l, pt.GetPosition()); }));
	defPt.def("SetPreviousPosition", static_cast<void (*)(lua_State *, ::CParticle &, const Vector3 &)>([](lua_State *l, ::CParticle &pt, const Vector3 &prevPos) { pt.SetPrevPos(prevPos); }));
	defPt.def("GetPreviousPosition", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::Push<Vector3>(l, pt.GetPrevPos()); }));
	defPt.def("GetVelocity", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::Push<Vector3>(l, pt.GetVelocity()); }));
	defPt.def("GetAngularVelocity", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::Push<Vector3>(l, pt.GetAngularVelocity()); }));
	defPt.def("GetColor", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::Push<Vector4>(l, pt.GetColor()); }));
	defPt.def("GetAlpha", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::PushNumber(l, pt.GetColor().a); }));
	defPt.def("GetLife", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::PushNumber(l, pt.GetLife()); }));
	defPt.def("SetLife", static_cast<void (*)(lua_State *, ::CParticle &, float)>([](lua_State *l, ::CParticle &pt, float life) { pt.SetLife(life); }));
	defPt.def("GetTimeAlive", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::PushNumber(l, pt.GetTimeAlive()); }));
	defPt.def("SetTimeAlive", static_cast<void (*)(lua_State *, ::CParticle &, float)>([](lua_State *l, ::CParticle &pt, float life) { pt.SetTimeAlive(life); }));
	defPt.def("GetTimeCreated", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::PushNumber(l, pt.GetTimeCreated()); }));
	defPt.def("SetTimeCreated", static_cast<void (*)(lua_State *, ::CParticle &, float)>([](lua_State *l, ::CParticle &pt, float time) { pt.SetTimeCreated(time); }));
	defPt.def("SetColor", static_cast<void (*)(lua_State *, ::CParticle &, const Vector4 &)>([](lua_State *l, ::CParticle &pt, const Vector4 &color) { pt.SetColor(color); }));
	defPt.def("SetAlpha", static_cast<void (*)(lua_State *, ::CParticle &, float)>([](lua_State *l, ::CParticle &pt, float a) {
		auto &col = pt.GetColor();
		col.a = a;
	}));
	defPt.def("SetPosition", static_cast<void (*)(lua_State *, ::CParticle &, const Vector3 &)>([](lua_State *l, ::CParticle &pt, const Vector3 &pos) { pt.SetPosition(pos); }));
	defPt.def("SetVelocity", static_cast<void (*)(lua_State *, ::CParticle &, const Vector3 &)>([](lua_State *l, ::CParticle &pt, const Vector3 &vel) { pt.SetVelocity(vel); }));
	defPt.def("SetAngularVelocity", static_cast<void (*)(lua_State *, ::CParticle &, const Vector3 &)>([](lua_State *l, ::CParticle &pt, const Vector3 &vel) { pt.SetAngularVelocity(vel); }));
	defPt.def("GetRadius", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::PushNumber(l, pt.GetRadius()); }));
	defPt.def("GetLength", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::PushNumber(l, pt.GetLength()); }));
	defPt.def("GetExtent", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::PushNumber(l, pt.GetExtent()); }));
	defPt.def("SetRadius", static_cast<void (*)(lua_State *, ::CParticle &, float)>([](lua_State *l, ::CParticle &pt, float radius) { pt.SetRadius(radius); }));
	defPt.def("SetLength", static_cast<void (*)(lua_State *, ::CParticle &, float)>([](lua_State *l, ::CParticle &pt, float len) { pt.SetLength(len); }));
	defPt.def("SetCameraDistance", static_cast<void (*)(lua_State *, ::CParticle &, float)>([](lua_State *l, ::CParticle &pt, float dist) { pt.SetCameraDistance(dist); }));
	defPt.def("GetCameraDistance", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::PushNumber(l, pt.GetCameraDistance()); }));
	defPt.def("Reset", static_cast<void (*)(lua_State *, ::CParticle &, float)>([](lua_State *l, ::CParticle &pt, float t) { pt.Reset(t); }));
	defPt.def("SetAnimationFrameOffset", static_cast<void (*)(lua_State *, ::CParticle &, float)>([](lua_State *l, ::CParticle &pt, float offset) { pt.SetFrameOffset(offset); }));
	defPt.def("GetAnimationFrameOffset", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::PushNumber(l, pt.GetFrameOffset()); }));
	defPt.def("Die", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { pt.Die(); }));
	defPt.def("Resurrect", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { pt.Resurrect(); }));
	defPt.def("IsDying", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::PushBool(l, pt.IsDying()); }));
	defPt.def("GetDeathTime", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::PushNumber(l, pt.GetDeathTime()); }));
	defPt.def("GetLifeSpan", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::PushNumber(l, pt.GetLifeSpan()); }));
	defPt.def("GetWorldRotation", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::Push<Quat>(l, pt.GetWorldRotation()); }));
	defPt.def("SetWorldRotation", static_cast<void (*)(lua_State *, ::CParticle &, const Quat &)>([](lua_State *l, ::CParticle &pt, const Quat &rot) { pt.SetWorldRotation(rot); }));
	defPt.def("GetOrigin", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::Push<Vector3>(l, pt.GetOrigin()); }));
	defPt.def("SetOrigin", static_cast<void (*)(lua_State *, ::CParticle &, const Vector3 &)>([](lua_State *l, ::CParticle &pt, const Vector3 &origin) { pt.SetOrigin(origin); }));
	defPt.def("GetSeed", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::PushNumber(l, pt.GetSeed()); }));
	defPt.def("GetSequence", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::PushInt(l, pt.GetSequence()); }));
	defPt.def("SetSequence", static_cast<void (*)(lua_State *, ::CParticle &, uint32_t)>([](lua_State *l, ::CParticle &pt, uint32_t sequence) { pt.SetSequence(sequence); }));
	defPt.def("CalcRandomInt", static_cast<void (*)(lua_State *, ::CParticle &, int32_t, int32_t)>([](lua_State *l, ::CParticle &pt, int32_t min, int32_t max) { Lua::PushInt(l, pt.PseudoRandomInt(min, max)); }));
	defPt.def("CalcRandomInt", static_cast<void (*)(lua_State *, ::CParticle &, int32_t, int32_t, uint32_t)>([](lua_State *l, ::CParticle &pt, int32_t min, int32_t max, uint32_t seed) { Lua::PushInt(l, pt.PseudoRandomInt(min, max, seed)); }));
	defPt.def("CalcRandomFloat", static_cast<void (*)(lua_State *, ::CParticle &, float, float)>([](lua_State *l, ::CParticle &pt, float min, float max) { Lua::PushNumber(l, pt.PseudoRandomReal(min, max)); }));
	defPt.def("CalcRandomFloat", static_cast<void (*)(lua_State *, ::CParticle &, float, float, uint32_t)>([](lua_State *l, ::CParticle &pt, float min, float max, uint32_t seed) { Lua::PushNumber(l, pt.PseudoRandomReal(min, max, seed)); }));
	defPt.def("CalcRandomFloatExp", static_cast<void (*)(lua_State *, ::CParticle &, float, float, float)>([](lua_State *l, ::CParticle &pt, float min, float max, float exp) { Lua::PushNumber(l, pt.PseudoRandomRealExp(min, max, exp)); }));
	defPt.def("CalcRandomFloatExp", static_cast<void (*)(lua_State *, ::CParticle &, float, float, float, uint32_t)>([](lua_State *l, ::CParticle &pt, float min, float max, float exp, uint32_t seed) { Lua::PushNumber(l, pt.PseudoRandomRealExp(min, max, exp, seed)); }));

	defPt.def("GetInitialRadius", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::PushNumber(l, pt.GetInitialRadius()); }));
	defPt.def("GetInitialLength", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::PushNumber(l, pt.GetInitialLength()); }));
	defPt.def("GetInitialRotation", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::PushNumber(l, pt.GetInitialRotation()); }));
	defPt.def("GetInitialLife", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::PushNumber(l, pt.GetInitialLife()); }));
	defPt.def("GetInitialColor", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::Push<Vector4>(l, pt.GetInitialColor()); }));
	defPt.def("GetInitialAlpha", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::PushNumber(l, pt.GetInitialColor().a); }));
	defPt.def("GetInitialAnimationFrameOffset", static_cast<void (*)(lua_State *, ::CParticle &)>([](lua_State *l, ::CParticle &pt) { Lua::PushNumber(l, pt.GetInitialFrameOffset()); }));
	defPtc.scope[defPt];
}
void Lua::ParticleSystemModifier::register_modifier_class(luabind::class_<pragma::CParticleSystemComponent, pragma::BaseEnvParticleSystemComponent> &defPtc)
{
	auto defPtModifier = luabind::class_<::CParticleModifier>("ParticleModifier");
	defPtModifier.def("GetName", static_cast<void (*)(lua_State *, ::CParticleModifier &)>([](lua_State *l, ::CParticleModifier &ptm) { Lua::PushString(l, ptm.GetName()); }));
	defPtModifier.def("GetType", static_cast<void (*)(lua_State *, ::CParticleModifier &)>([](lua_State *l, ::CParticleModifier &ptm) { Lua::PushString(l, ptm.GetType()); }));
	defPtModifier.def("SetPriority", static_cast<void (*)(lua_State *, ::CParticleModifier &, int32_t)>([](lua_State *l, ::CParticleModifier &ptm, int32_t priority) { ptm.SetPriority(priority); }));
	defPtModifier.def("GetPriority", static_cast<void (*)(lua_State *, ::CParticleModifier &)>([](lua_State *l, ::CParticleModifier &ptm) { Lua::PushInt(l, ptm.GetPriority()); }));
	defPtModifier.def("GetParticleSystem", static_cast<void (*)(lua_State *, ::CParticleModifier &)>([](lua_State *l, ::CParticleModifier &ptm) { ptm.GetParticleSystem().PushLuaObject(l); }));
	defPtModifier.def("SetKeyValue", static_cast<void (*)(lua_State *, ::CParticleModifier &, const std::string &, const std::string &)>([](lua_State *l, ::CParticleModifier &ptm, const std::string &key, const std::string &value) {
		auto *keyValues = const_cast<std::unordered_map<std::string, std::string> *>(ptm.GetKeyValues());
		if(keyValues == nullptr)
			return;
		(*keyValues)[key] = value;
	}));
	defPtModifier.def("GetKeyValue", static_cast<void (*)(lua_State *, ::CParticleModifier &, const std::string &)>([](lua_State *l, ::CParticleModifier &ptm, const std::string &key) {
		auto *keyValues = ptm.GetKeyValues();
		if(keyValues == nullptr)
			return;
		auto it = keyValues->find(key);
		Lua::PushString(l, (it != keyValues->end()) ? it->second : "");
	}));
	defPtModifier.def("GetKeyValues", static_cast<void (*)(lua_State *, ::CParticleModifier &)>([](lua_State *l, ::CParticleModifier &ptm) {
		auto t = Lua::CreateTable(l);
		auto *keyValues = ptm.GetKeyValues();
		if(keyValues == nullptr)
			return;
		for(auto &pair : *keyValues) {
			Lua::PushString(l, pair.first);
			Lua::PushString(l, pair.second);
			Lua::SetTableValue(l, t);
		}
	}));
	defPtc.scope[defPtModifier];

	auto defPtInitializer = luabind::class_<::CParticleInitializer, ::CParticleModifier>("ParticleInitializer");
	defPtc.scope[defPtInitializer];

	auto defPtOperator = luabind::class_<::CParticleOperator, ::CParticleModifier>("ParticleOperator");
	defPtOperator.def("CalcStrength", static_cast<void (*)(lua_State *, ::CParticleOperator &, float)>([](lua_State *l, ::CParticleOperator &op, float curTime) {
		auto strength = op.CalcStrength(curTime);
		Lua::PushNumber(l, strength);
	}));
	defPtc.scope[defPtOperator];

	auto defPtRenderer = luabind::class_<::CParticleRenderer, ::CParticleModifier>("ParticleRenderer");
	defPtc.scope[defPtRenderer];

	auto defPtInitializerBase = luabind::class_<CParticleInitializerLua, luabind::bases<::CParticleInitializer, ::CParticleModifier>>("BaseInitializer");
	defPtInitializerBase.def(luabind::constructor<>());
	defPtInitializerBase.def("Initialize", &CParticleInitializerLua::Lua_Initialize, &CParticleInitializerLua::Lua_default_Initialize);
	defPtInitializerBase.def("OnParticleSystemStarted", &CParticleInitializerLua::Lua_OnParticleSystemStarted, &CParticleInitializerLua::Lua_default_OnParticleSystemStarted);
	defPtInitializerBase.def("OnParticleSystemStopped", &CParticleInitializerLua::Lua_OnParticleSystemStopped, &CParticleInitializerLua::Lua_default_OnParticleSystemStopped);
	defPtInitializerBase.def("OnParticleCreated", &CParticleInitializerLua::Lua_OnParticleCreated, &CParticleInitializerLua::Lua_default_OnParticleCreated);
	defPtInitializerBase.def("OnParticleDestroyed", &CParticleInitializerLua::Lua_OnParticleDestroyed, &CParticleInitializerLua::Lua_default_OnParticleDestroyed);
	defPtc.scope[defPtInitializerBase];

	auto defPtOperatorBase = luabind::class_<CParticleOperatorLua, luabind::bases<::CParticleOperator, ::CParticleModifier>>("BaseOperator");
	defPtOperatorBase.def(luabind::constructor<>());
	defPtOperatorBase.def("Initialize", &CParticleOperatorLua::Lua_Initialize, &CParticleOperatorLua::Lua_default_Initialize);
	defPtOperatorBase.def("OnParticleSystemStarted", &CParticleOperatorLua::Lua_OnParticleSystemStarted, &CParticleOperatorLua::Lua_default_OnParticleSystemStarted);
	defPtOperatorBase.def("OnParticleSystemStopped", &CParticleOperatorLua::Lua_OnParticleSystemStopped, &CParticleOperatorLua::Lua_default_OnParticleSystemStopped);
	defPtOperatorBase.def("OnParticleCreated", &CParticleOperatorLua::Lua_OnParticleCreated, &CParticleOperatorLua::Lua_default_OnParticleCreated);
	defPtOperatorBase.def("OnParticleDestroyed", &CParticleOperatorLua::Lua_OnParticleDestroyed, &CParticleOperatorLua::Lua_default_OnParticleDestroyed);
	defPtOperatorBase.def("Simulate", &CParticleOperatorLua::Lua_Simulate, &CParticleOperatorLua::Lua_default_Simulate);
	defPtc.scope[defPtOperatorBase];

	auto defPtRendererBase = luabind::class_<CParticleRendererLua, luabind::bases<::CParticleRenderer, ::CParticleModifier>>("BaseRenderer");
	defPtRendererBase.def(luabind::constructor<>());
	defPtRendererBase.def("Initialize", &CParticleRendererLua::Lua_Initialize, &CParticleRendererLua::Lua_default_Initialize);
	defPtRendererBase.def("OnParticleSystemStarted", &CParticleRendererLua::Lua_OnParticleSystemStarted, &CParticleRendererLua::Lua_default_OnParticleSystemStarted);
	defPtRendererBase.def("OnParticleSystemStopped", &CParticleRendererLua::Lua_OnParticleSystemStopped, &CParticleRendererLua::Lua_default_OnParticleSystemStopped);
	defPtRendererBase.def("OnParticleCreated", &CParticleRendererLua::Lua_OnParticleCreated, &CParticleRendererLua::Lua_default_OnParticleCreated);
	defPtRendererBase.def("OnParticleDestroyed", &CParticleRendererLua::Lua_OnParticleDestroyed, &CParticleRendererLua::Lua_default_OnParticleDestroyed);
	defPtRendererBase.def("Render", &CParticleRendererLua::Lua_Render, &CParticleRendererLua::Lua_default_Render);
	defPtRendererBase.def("SetShader", static_cast<void (*)(lua_State *, ::CParticleRendererLua &, ::pragma::LuaShaderWrapperParticle2D &)>([](lua_State *l, ::CParticleRendererLua &renderer, ::pragma::LuaShaderWrapperParticle2D &shader) {
		renderer.SetShader(&static_cast<pragma::LShaderParticle2D &>(shader.GetShader()));
	}));
	defPtRendererBase.def("GetShader", static_cast<void (*)(lua_State *, ::CParticleRendererLua &)>([](lua_State *l, ::CParticleRendererLua &renderer) {
		auto *shader = dynamic_cast<pragma::LShaderParticle2D *>(renderer.GetShader());
		if(shader == nullptr)
			return;
		auto *wrapper = dynamic_cast<pragma::LuaShaderWrapperParticle2D *>(shader->GetWrapper());
		if(wrapper) {
			wrapper->GetLuaObject().push(l);
			return;
		}
		Lua::Push(l, shader);
	}));
	defPtc.scope[defPtRendererBase];
}
