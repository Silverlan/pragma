// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/cxxmodules.hpp"
#include "pragma/entities/components/base_io_component.hpp"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/rendering/shaders/c_shader_lua.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include "pragma/rendering/shaders/particles/c_shader_particle_2d_base.hpp"
#include <pragma/model/model.h>
#include <pragma/model/modelmesh.h>
#include <pragma/networking/nwm_util.h>
#include "pragma/entities/baseentity_luaobject.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/baseentity_events.hpp>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/entities/environment/effects/particlesystemdata.h>
#include <udm.hpp>
#include "pragma/particlesystem/initializers/c_particle_initializer_lua.hpp"
#include <pragma/lua/lua_util_component.hpp>
#include <pragma/model/model.h>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <buffers/prosper_buffer.hpp>

module pragma.client.entities.components.particle_system;

import pragma.client.client_state;
import pragma.client.entities.components.animated;
import pragma.client.entities.components.io;
import pragma.client.entities.components.rasterization_renderer;
import pragma.client.entities.components.transform;
import pragma.client.game;
import pragma.client.particle_system;

using namespace pragma;

extern ClientState *client;

void ecs::CParticleSystemComponent::Initialize()
{
	BaseEnvParticleSystemComponent::Initialize();

	BindEventUnhandled(BaseToggleComponent::EVENT_ON_TURN_ON, [this](std::reference_wrapper<ComponentEvent> evData) { Start(); });
	BindEventUnhandled(BaseToggleComponent::EVENT_ON_TURN_OFF, [this](std::reference_wrapper<ComponentEvent> evData) { Stop(); });
	BindEvent(CIOComponent::EVENT_HANDLE_INPUT, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<pragma::CEInputData &>(evData.get());
		if(ustring::compare<std::string>(inputData.input, "setcontinuous", false)) {
			SetContinuous(util::to_boolean(inputData.data));
			return util::EventReply::Handled;
		}
		return util::EventReply::Unhandled;
	});
	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		return HandleKeyValue(kvData.key, kvData.value);
	});
	BindEvent(CAnimatedComponent::EVENT_SHOULD_UPDATE_BONES, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		static_cast<CEShouldUpdateBones &>(evData.get()).shouldUpdate = IsActive();
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent != nullptr) {
		auto &trC = *pTrComponent;
		FlagCallbackForRemoval(pTrComponent->AddEventCallback(CTransformComponent::EVENT_ON_POSE_CHANGED,
		                         [this, &trC](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
			                         if(IsActive() == false || umath::is_flag_set(static_cast<pragma::CEOnPoseChanged &>(evData.get()).changeFlags, pragma::TransformChangeFlags::PositionChanged) == false)
				                         return util::EventReply::Unhandled;
			                         for(auto it = m_childSystems.begin(); it != m_childSystems.end(); ++it) {
				                         auto &hChild = *it;
				                         if(hChild.child.valid()) {
					                         auto pTrComponent = hChild.child->GetEntity().GetTransformComponent();
					                         if(pTrComponent != nullptr)
						                         pTrComponent->SetPosition(trC.GetPosition());
				                         }
			                         }
			                         return util::EventReply::Unhandled;
		                         }),
		  CallbackType::Entity);
	}
}
void ecs::CParticleSystemComponent::OnEntitySpawn()
{
	CreateParticle();
	BaseEnvParticleSystemComponent::OnEntitySpawn();
}
util::EventReply ecs::CParticleSystemComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseEnvParticleSystemComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;

	return util::EventReply::Unhandled;
}
void ecs::CParticleSystemComponent::ReceiveData(NetPacket &packet)
{
	SetParticleFile(packet->ReadString());
	m_particleName = packet->ReadString();
}
void ecs::CParticleSystemComponent::SetParticleFile(const std::string &fileName)
{
	BaseEnvParticleSystemComponent::SetParticleFile(fileName);
	ecs::CParticleSystemComponent::Precache(fileName);
}
pragma::rendering::SceneRenderPass ecs::CParticleSystemComponent::GetSceneRenderPass() const { return m_renderPass; }
void ecs::CParticleSystemComponent::SetSceneRenderPass(pragma::rendering::SceneRenderPass pass) { m_renderPass = pass; }

void ecs::CParticleSystemComponent::CreateParticle()
{
	if(SetupParticleSystem(m_particleName) == false)
		return;
	/*if(m_hCbRenderCallback.IsValid())
		m_hCbRenderCallback.Remove();
	m_hCbRenderCallback = particle->AddRenderCallback([this]() {
		auto &ent = static_cast<CBaseEntity&>(GetEntity());
		auto pAttComponent = ent.GetComponent<CAttachableComponent>();
		if(pAttComponent.valid())
			pAttComponent->UpdateAttachmentOffset();
	});*/
}
void ecs::CParticleSystemComponent::SetRemoveOnComplete(bool b) { BaseEnvParticleSystemComponent::SetRemoveOnComplete(b); }

void ecs::CParticleSystemComponent::Clear()
{
	Stop();
	m_particleSystemName.clear();
	m_initializers.clear();
	m_operators.clear();
	m_renderers.clear();
	for(auto &hChild : m_childSystems) {
		if(hChild.child.expired())
			continue;
		hChild.child->GetEntity().Remove();
	}
	m_childSystems.clear();
	m_state = State::Initial;

	umath::set_flag(m_flags, Flags::Setup, false);
}

std::shared_ptr<Model> ecs::CParticleSystemComponent::GenerateModel(Game &game, const std::vector<const ecs::CParticleSystemComponent *> &particleSystems)
{
	auto *cam = static_cast<CGame&>(game).GetRenderCamera<pragma::CCameraComponent>();
	if(cam == nullptr)
		return nullptr;
	std::unordered_set<const ecs::CParticleSystemComponent *> particleSystemList {};
	for(auto *pts : particleSystems) {
		particleSystemList.insert(pts);
		for(auto &childData : pts->GetChildren()) {
			if(childData.child.expired())
				continue;
			particleSystemList.insert(childData.child.get());
		}
	}
	Vector3 camUpWs;
	Vector3 camRightWs;
	float ptNearZ, ptFarZ;
	auto nearZ = cam->GetNearZ();
	auto farZ = cam->GetFarZ();
	auto &posCam = cam->GetEntity().GetPosition();
	auto mdl = game.CreateModel();
	constexpr uint32_t numVerts = pragma::ShaderParticle2DBase::VERTEX_COUNT;
	uint32_t numTris = pragma::ShaderParticle2DBase::TRIANGLE_COUNT * 2;
	for(auto *pts : particleSystemList) {
		auto &renderers = pts->GetRenderers();
		if(renderers.empty())
			return nullptr;
		auto &renderer = *renderers.front();
		auto *pShader = dynamic_cast<pragma::ShaderParticle2DBase *>(renderer.GetShader());
		if(pShader == nullptr)
			return nullptr;
		auto *mat = pts->GetMaterial();
		if(mat == nullptr)
			continue;
		std::optional<uint32_t> skinTexIdx {};
		mdl->AddMaterial(0, mat, {}, &skinTexIdx);
		if(skinTexIdx.has_value() == false)
			continue;
		auto orientationType = pts->GetOrientationType();
		pShader->GetParticleSystemOrientationInfo(cam->GetProjectionMatrix() * cam->GetViewMatrix(), *CXXM_RCAST(const pragma::CParticleSystemComponent*, pts), CXXM_SCAST(pragma::ParticleOrientationType, orientationType), camUpWs, camRightWs, ptNearZ, ptFarZ, mat, nearZ, farZ);

		auto *spriteSheetAnim = pts->GetSpriteSheetAnimation();
		auto &particles = pts->GetRenderParticleData();
		auto &animData = pts->GetParticleAnimationData();
		auto numParticles = pts->GetRenderParticleCount();

		auto subMesh = game.CreateModelSubMesh();
		subMesh->SetSkinTextureIndex(*skinTexIdx);
		auto &verts = subMesh->GetVertices();
		auto numTrisSys = numParticles * numTris;
		auto numVertsSys = numParticles * numVerts;
		subMesh->ReserveIndices(numTrisSys * 3);
		verts.resize(numVertsSys);
		uint32_t vertOffset = 0;
		for(auto i = decltype(numParticles) {0u}; i < numParticles; ++i) {
			auto &pt = particles.at(i);
			auto ptIdx = pts->TranslateBufferIndex(i);
			auto pos = pts->GetParticlePosition(ptIdx);
			Vector2 uvStart {0.f, 0.f};
			Vector2 uvEnd {1.f, 1.f};
			if(pts->IsAnimated() && spriteSheetAnim && spriteSheetAnim->sequences.empty() == false) {
				auto &animData = pts->GetParticleAnimationData().at(i);
				auto &ptData = *const_cast<ecs::CParticleSystemComponent *>(pts)->GetParticle(ptIdx);
				auto seqIdx = ptData.GetSequence();
				assert(seqIdx < spriteSheetAnim->sequences.size());
				auto &seq = (seqIdx < spriteSheetAnim->sequences.size()) ? spriteSheetAnim->sequences.at(seqIdx) : spriteSheetAnim->sequences.back();
				auto frameIndex = (seqIdx < spriteSheetAnim->sequences.size()) ? seq.GetLocalFrameIndex(animData.frameIndex0) : 0;
				auto &frame = seq.frames.at(frameIndex);
				uvStart = frame.uvStart;
				uvEnd = frame.uvEnd;
			}
			for(auto vertIdx = decltype(numVerts) {0u}; vertIdx < numVerts; ++vertIdx) {
				auto vertPos = pShader->CalcVertexPosition(*CXXM_RCAST(const pragma::CParticleSystemComponent*, pts), pts->TranslateBufferIndex(i), vertIdx, posCam, camUpWs, camRightWs, nearZ, farZ);
				auto uv = pragma::ShaderParticle2DBase::GetVertexUV(vertIdx);
				auto &v = verts.at(vertOffset + vertIdx);
				v.position = vertPos;
				v.normal = Vector4 {-uvec::RIGHT, 1.f};
				// v.tangent = uvec::FORWARD;
				v.uv = uvStart + uv * (uvEnd - uvStart);
			}
			static_assert(pragma::ShaderParticle2DBase::TRIANGLE_COUNT == 2 && pragma::ShaderParticle2DBase::VERTEX_COUNT == 6);
			std::array<uint32_t, 12> indices = {0, 1, 2, 3, 4, 5,

			  // Back facing
			  0, 2, 1, 3, 5, 4};
			for(auto idx : indices)
				subMesh->AddIndex(vertOffset + idx);

			vertOffset += numVerts;
		}

		auto mesh = game.CreateModelMesh();
		mesh->AddSubMesh(subMesh);
		mdl->GetMeshGroup(0)->AddMesh(mesh);
	}
	return mdl;
}
std::shared_ptr<Model> ecs::CParticleSystemComponent::GenerateModel() const
{
	auto &game = static_cast<CGame &>(*GetEntity().GetNetworkState()->GetGameState());
	std::vector<const ecs::CParticleSystemComponent *> particleSystems {};
	particleSystems.push_back(this);
	return GenerateModel(game, particleSystems);
}

void ecs::CParticleSystemComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

bool ecs::CParticleSystemComponent::InitializeFromAssetData(const std::string &ptName, const ::udm::LinkedPropertyWrapper &udm, std::string &outErr)
{
	auto ptData = std::make_unique<CParticleSystemData>();
	auto res = LoadFromAssetData(*ptData, udm, outErr);
	if(res == false)
		return false;
	s_particleData[ptName] = std::move(ptData);
	return true;
}

bool ecs::CParticleSystemComponent::LoadFromAssetData(CParticleSystemData &ptData, const ::udm::LinkedPropertyWrapper &udm, std::string &outErr)
{
	udm["keyValues"](ptData.settings);

	auto itMat = ptData.settings.find("material");
	if(itMat != ptData.settings.end())
		client->LoadMaterial(itMat->second);

	auto readModifier = [&udm](const std::string &name, std::vector<CParticleModifierData> &modifiers) {
		auto udmModifiers = udm[name];
		auto numModifiers = udmModifiers.GetSize();
		modifiers.reserve(numModifiers);
		for(auto &udmModifier : udmModifiers) {
			std::string name;
			udmModifier["name"](name);
			if(name.empty())
				continue;
			modifiers.push_back(CParticleModifierData {name});
			udmModifier["keyValues"](modifiers.back().settings);
		}
	};
	readModifier("initializers", ptData.initializers);
	readModifier("operators", ptData.operators);
	readModifier("renderers", ptData.renderers);

	auto udmChildren = udm["children"];
	auto &children = ptData.children;
	auto numChildren = udmChildren.GetSize();
	children.reserve(numChildren);
	for(auto i = decltype(numChildren) {0u}; i < numChildren; ++i) {
		auto udmChild = udmChildren[i];
		std::string type;
		udmChild["type"](type);
		if(type.empty())
			continue;
		children.push_back({});
		auto &childData = children.back();
		childData.childName = type;
		udmChild["delay"](childData.delay);
	}
	return true;
}
bool ecs::CParticleSystemComponent::LoadFromAssetData(CParticleSystemData &ptData, const udm::AssetData &data, std::string &outErr)
{
	if(data.GetAssetType() != pragma::asset::PPTSYS_IDENTIFIER) {
		outErr = "Incorrect format!";
		return false;
	}

	auto version = data.GetAssetVersion();
	if(version < 1) {
		outErr = "Invalid version!";
		return false;
	}
	// if(version > FORMAT_VERSION)
	// 	return false;

	auto udm = *data;
	return LoadFromAssetData(ptData, udm, outErr);
}

static void register_particle_modifier(lua_State *l, pragma::LuaParticleModifierManager::Type type, const std::string &name, luabind::object oClass)
{
	Lua::CheckUserData(l, 2);
	auto &particleModMan = reinterpret_cast<LuaParticleModifierManager&>(c_game->GetLuaParticleModifierManager());
	if(particleModMan.RegisterModifier(type, name, oClass) == false)
		return;

	auto *map = GetParticleModifierMap();
	if(map == nullptr)
		return;
	switch(type) {
	case pragma::LuaParticleModifierManager::Type::Initializer:
		map->AddInitializer(name, [name](pragma::CParticleSystemComponent &psc, const std::unordered_map<std::string, std::string> &keyValues) -> std::unique_ptr<CParticleInitializer, void (*)(CParticleInitializer *)> {
			auto &particleModMan = reinterpret_cast<LuaParticleModifierManager&>(c_game->GetLuaParticleModifierManager());
			auto *modifier = dynamic_cast<CParticleInitializer *>(particleModMan.CreateModifier(name));
			if(modifier == nullptr)
				return std::unique_ptr<CParticleInitializer, void (*)(CParticleInitializer *)>(nullptr, [](CParticleInitializer *p) {});
			modifier->Initialize(psc, keyValues);
			modifier->SetName(name);
			return std::unique_ptr<CParticleInitializer, void (*)(CParticleInitializer *)>(modifier, [](CParticleInitializer *p) {}); // Externally owned (by Lua state), so no delete
		});
		break;
	case pragma::LuaParticleModifierManager::Type::Operator:
		map->AddOperator(name, [name](pragma::CParticleSystemComponent &psc, const std::unordered_map<std::string, std::string> &keyValues) -> std::unique_ptr<CParticleOperator, void (*)(CParticleOperator *)> {
			auto &particleModMan = reinterpret_cast<LuaParticleModifierManager&>(c_game->GetLuaParticleModifierManager());
			auto *modifier = dynamic_cast<CParticleOperator *>(particleModMan.CreateModifier(name));
			if(modifier == nullptr)
				return std::unique_ptr<CParticleOperator, void (*)(CParticleOperator *)>(nullptr, [](CParticleOperator *p) {});
			modifier->Initialize(psc, keyValues);
			modifier->SetName(name);
			return std::unique_ptr<CParticleOperator, void (*)(CParticleOperator *)>(modifier, [](CParticleOperator *p) {}); // Externally owned (by Lua state), so no delete
		});
		break;
	case pragma::LuaParticleModifierManager::Type::Renderer:
		map->AddRenderer(name, [name](pragma::CParticleSystemComponent &psc, const std::unordered_map<std::string, std::string> &keyValues) -> std::unique_ptr<CParticleRenderer, void (*)(CParticleRenderer *)> {
			auto &particleModMan = reinterpret_cast<LuaParticleModifierManager&>(c_game->GetLuaParticleModifierManager());
			auto *modifier = dynamic_cast<CParticleRenderer *>(particleModMan.CreateModifier(name));
			if(modifier == nullptr)
				return std::unique_ptr<CParticleRenderer, void (*)(CParticleRenderer *)>(nullptr, [](CParticleRenderer *p) {});
			modifier->Initialize(psc, keyValues);
			modifier->SetName(name);
			return std::unique_ptr<CParticleRenderer, void (*)(CParticleRenderer *)>(modifier, [](CParticleRenderer *p) {}); // Externally owned (by Lua state), so no delete
		});
		break;
	case pragma::LuaParticleModifierManager::Type::Emitter:
		// TODO
		break;
	}
}

static void push_particle_system_definition_data(lua_State *l, const CParticleSystemData &ptSysData)
{
	auto tPtSys = Lua::CreateTable(l);

	for(auto &pair : ptSysData.settings) {
		Lua::PushString(l, pair.first);
		Lua::PushString(l, pair.second);
		Lua::SetTableValue(l, tPtSys);
	}

	auto fPushOperators = [l, tPtSys](const std::string &type, const std::vector<CParticleModifierData> &data) {
		Lua::PushString(l, type);
		auto tInitializers = Lua::CreateTable(l);
		int32_t idx = 1;
		for(auto &initializer : data) {
			Lua::PushInt(l, idx++);
			auto tInitializer = Lua::CreateTable(l);

			Lua::PushString(l, "operatorType");
			Lua::PushString(l, initializer.name);
			Lua::SetTableValue(l, tInitializer);

			for(auto &pair : initializer.settings) {
				Lua::PushString(l, pair.first);
				Lua::PushString(l, pair.second);
				Lua::SetTableValue(l, tInitializer);
			}

			Lua::SetTableValue(l, tInitializers);
		}
		Lua::SetTableValue(l, tPtSys);
	};
	fPushOperators("initializers", ptSysData.initializers);
	fPushOperators("operators", ptSysData.operators);
	fPushOperators("renderers", ptSysData.renderers);

	Lua::PushString(l, "children");
	auto tChildren = Lua::CreateTable(l);
	int32_t idx = 1;
	for(auto &childData : ptSysData.children) {
		Lua::PushInt(l, idx++);
		auto tChild = Lua::CreateTable(l);

		Lua::PushString(l, "childName");
		Lua::PushString(l, childData.childName);
		Lua::SetTableValue(l, tChild);

		Lua::PushString(l, "delay");
		Lua::PushNumber(l, childData.delay);
		Lua::SetTableValue(l, tChild);

		Lua::SetTableValue(l, tChildren);
	}
	Lua::SetTableValue(l, tPtSys);
}

static void Stop(lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, bool bStopImmediately)
{
	if(bStopImmediately == true)
		hComponent.Stop();
	else
		hComponent.Die();
}

static std::string get_key_value(lua_State *l, const luabind::object &value)
{
	auto type = luabind::type(value);
	switch(type) {
	case LUA_TNUMBER:
		return std::to_string(luabind::object_cast<double>(value));
	case LUA_TBOOLEAN:
		return luabind::object_cast<bool>(value) ? "1" : "0";
	case LUA_TUSERDATA:
		{
			auto *v4 = luabind::object_cast_nothrow<Vector4 *>(value, static_cast<Vector4 *>(nullptr));
			if(v4)
				return std::to_string(v4->x) + " " + std::to_string(v4->y) + " " + std::to_string(v4->z) + " " + std::to_string(v4->w);

			auto *v3 = luabind::object_cast_nothrow<Vector3 *>(value, static_cast<Vector3 *>(nullptr));
			if(v3)
				return std::to_string(v3->x) + " " + std::to_string(v3->y) + " " + std::to_string(v3->z);

			auto *v2 = luabind::object_cast_nothrow<Vector2 *>(value, static_cast<Vector2 *>(nullptr));
			if(v2)
				return std::to_string(v2->x) + " " + std::to_string(v2->y);

			auto *v4i = luabind::object_cast_nothrow<Vector4i *>(value, static_cast<Vector4i *>(nullptr));
			if(v4i)
				return std::to_string(v4i->x) + " " + std::to_string(v4i->y) + " " + std::to_string(v4i->z) + " " + std::to_string(v4i->w);

			auto *v3i = luabind::object_cast_nothrow<Vector3i *>(value, static_cast<Vector3i *>(nullptr));
			if(v3i)
				return std::to_string(v3i->x) + " " + std::to_string(v3i->y) + " " + std::to_string(v3i->z);

			auto *v2i = luabind::object_cast_nothrow<Vector2i *>(value, static_cast<Vector2i *>(nullptr));
			if(v2i)
				return std::to_string(v2i->x) + " " + std::to_string(v2i->y);

			auto *col = luabind::object_cast_nothrow<Color *>(value, static_cast<Color *>(nullptr));
			if(col)
				return std::to_string(col->r) + " " + std::to_string(col->g) + " " + std::to_string(col->b) + " " + std::to_string(col->a);

			auto *ang = luabind::object_cast_nothrow<EulerAngles *>(value, static_cast<EulerAngles *>(nullptr));
			if(ang)
				return std::to_string(ang->p) + " " + std::to_string(ang->y) + " " + std::to_string(ang->r);

			auto *rot = luabind::object_cast_nothrow<Quat *>(value, static_cast<Quat *>(nullptr));
			if(rot)
				return std::to_string(rot->w) + " " + std::to_string(rot->x) + " " + std::to_string(rot->y) + " " + std::to_string(rot->z);
		}
	}
	return luabind::object_cast<std::string>(value);
}

std::unordered_map<std::string, std::string> pragma::ecs::get_particle_key_values(lua_State *l, const luabind::map<std::string, void> &keyValues)
{
	std::unordered_map<std::string, std::string> values;
	for(luabind::iterator i {keyValues}, end; i != end; ++i) {
		auto key = luabind::object_cast<std::string>(i.key());
		std::string val = get_key_value(l, *i);

		ustring::to_lower(key);
		values[key] = val;
	}
	return values;
}

static void register_particle_class(luabind::class_<pragma::ecs::CParticleSystemComponent, pragma::BaseEnvParticleSystemComponent> &defPtc)
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
static void register_modifier_class(luabind::class_<pragma::ecs::CParticleSystemComponent, pragma::BaseEnvParticleSystemComponent> &defPtc)
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

void ecs::CParticleSystemComponent::RegisterLuaBindings(lua_State *l, luabind::module_ &modEnts)
{
	BaseEnvParticleSystemComponent::RegisterLuaBindings(l, modEnts);
	auto defCParticleSystem = pragma::lua::create_entity_component_class<pragma::ecs::CParticleSystemComponent, pragma::BaseEnvParticleSystemComponent>("ParticleSystemComponent");

	defCParticleSystem.add_static_constant("SF_PARTICLE_SYSTEM_CONTINUOUS", SF_PARTICLE_SYSTEM_CONTINUOUS);

	defCParticleSystem.add_static_constant("RENDER_FLAG_NONE", umath::to_integral(pragma::ParticleRenderFlags::None));
	defCParticleSystem.add_static_constant("RENDER_FLAG_BIT_BLOOM", umath::to_integral(pragma::ParticleRenderFlags::Bloom));
	defCParticleSystem.add_static_constant("RENDER_FLAG_BIT_DEPTH_ONLY", umath::to_integral(pragma::ParticleRenderFlags::DepthOnly));

	defCParticleSystem.add_static_constant("FLAG_NONE", umath::to_integral(pragma::ecs::CParticleSystemComponent::Flags::None));
	defCParticleSystem.add_static_constant("FLAG_BIT_SOFT_PARTICLES", umath::to_integral(pragma::ecs::CParticleSystemComponent::Flags::SoftParticles));
	defCParticleSystem.add_static_constant("FLAG_BIT_TEXTURE_SCROLLING_ENABLED", umath::to_integral(pragma::ecs::CParticleSystemComponent::Flags::TextureScrollingEnabled));
	defCParticleSystem.add_static_constant("FLAG_BIT_RENDERER_BUFFER_UPDATE_REQUIRED", umath::to_integral(pragma::ecs::CParticleSystemComponent::Flags::RendererBufferUpdateRequired));
	defCParticleSystem.add_static_constant("FLAG_BIT_HAS_MOVING_PARTICLES", umath::to_integral(pragma::ecs::CParticleSystemComponent::Flags::HasMovingParticles));
	defCParticleSystem.add_static_constant("FLAG_BIT_MOVE_WITH_EMITTER", umath::to_integral(pragma::ecs::CParticleSystemComponent::Flags::MoveWithEmitter));
	defCParticleSystem.add_static_constant("FLAG_BIT_ROTATE_WITH_EMITTER", umath::to_integral(pragma::ecs::CParticleSystemComponent::Flags::RotateWithEmitter));
	defCParticleSystem.add_static_constant("FLAG_BIT_SORT_PARTICLES", umath::to_integral(pragma::ecs::CParticleSystemComponent::Flags::SortParticles));
	defCParticleSystem.add_static_constant("FLAG_BIT_DYING", umath::to_integral(pragma::ecs::CParticleSystemComponent::Flags::Dying));
	defCParticleSystem.add_static_constant("FLAG_BIT_RANDOM_START_FRAME", umath::to_integral(pragma::ecs::CParticleSystemComponent::Flags::RandomStartFrame));
	defCParticleSystem.add_static_constant("FLAG_BIT_PREMULTIPLY_ALPHA", umath::to_integral(pragma::ecs::CParticleSystemComponent::Flags::PremultiplyAlpha));
	defCParticleSystem.add_static_constant("FLAG_BIT_ALWAYS_SIMULATE", umath::to_integral(pragma::ecs::CParticleSystemComponent::Flags::AlwaysSimulate));
	defCParticleSystem.add_static_constant("FLAG_BIT_CAST_SHADOWS", umath::to_integral(pragma::ecs::CParticleSystemComponent::Flags::CastShadows));
	defCParticleSystem.add_static_constant("FLAG_BIT_SETUP", umath::to_integral(pragma::ecs::CParticleSystemComponent::Flags::Setup));
	defCParticleSystem.add_static_constant("FLAG_BIT_AUTO_SIMULATE", umath::to_integral(pragma::ecs::CParticleSystemComponent::Flags::AutoSimulate));
	defCParticleSystem.add_static_constant("FLAG_BIT_MATERIAL_DESCRIPT_SET_INITIALIZED", umath::to_integral(pragma::ecs::CParticleSystemComponent::Flags::MaterialDescriptorSetInitialized));

	defCParticleSystem.def("Start", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { hComponent.Start(); }));
	defCParticleSystem.def("Stop", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { ::Stop(l, hComponent, false); }));
	defCParticleSystem.def("Stop", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, bool)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, bool bStopImmediately) { ::Stop(l, hComponent, bStopImmediately); }));
	defCParticleSystem.def("Die", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { hComponent.Die(); }));
	defCParticleSystem.def("Die", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, float)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, float t) { hComponent.Die(t); }));
	defCParticleSystem.def("GetFlags", &pragma::ecs::CParticleSystemComponent::GetFlags);
	defCParticleSystem.def("SetFlags", &pragma::ecs::CParticleSystemComponent::SetFlags);
	defCParticleSystem.def("GetMaxNodes", &pragma::ecs::CParticleSystemComponent::GetMaxNodes);
	defCParticleSystem.def("SetMaxNodes", &pragma::ecs::CParticleSystemComponent::SetMaxNodes);
	defCParticleSystem.def("AddInitializer", +[](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, std::string name, luabind::object o) {
		return hComponent.AddInitializer(name, pragma::get_particle_key_values(l, o));
	});
	defCParticleSystem.def("AddOperator", +[](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, std::string name, luabind::object o) {
		return hComponent.AddOperator(name, pragma::get_particle_key_values(l, o));
	});
	defCParticleSystem.def("AddRenderer", +[](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, std::string name, luabind::object o) {
		return hComponent.AddRenderer(name, pragma::get_particle_key_values(l, o));
	});
	defCParticleSystem.def("RemoveInitializer", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, const std::string &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, const std::string &name) { hComponent.RemoveInitializer(name); }));
	defCParticleSystem.def("RemoveOperator", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, const std::string &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, const std::string &name) { hComponent.RemoveOperator(name); }));
	defCParticleSystem.def("RemoveRenderer", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, const std::string &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, const std::string &name) { hComponent.RemoveRenderer(name); }));
	defCParticleSystem.def("RemoveInitializerByType", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, const std::string &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, const std::string &name) { hComponent.RemoveInitializersByType(name); }));
	defCParticleSystem.def("RemoveOperatorByType", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, const std::string &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, const std::string &name) { hComponent.RemoveOperatorsByType(name); }));
	defCParticleSystem.def("RemoveRendererByType", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, const std::string &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, const std::string &name) { hComponent.RemoveRenderersByType(name); }));
	defCParticleSystem.def("GetInitializers", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) {
		auto t = Lua::CreateTable(l);
		auto &initializers = hComponent.GetInitializers();
		uint32_t idx = 1;
		for(auto &initializer : initializers) {
			Lua::PushInt(l, idx++);
			auto *luaInit = dynamic_cast<CParticleInitializerLua *>(initializer.get());
			if(luaInit)
				luaInit->GetLuaObject().push(l);
			else
				Lua::Push(l, initializer.get());
			Lua::SetTableValue(l, t);
		}
	}));
	defCParticleSystem.def("GetOperators", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) {
		auto t = Lua::CreateTable(l);
		auto &operators = hComponent.GetOperators();
		uint32_t idx = 1;
		for(auto &op : operators) {
			Lua::PushInt(l, idx++);
			auto *luaOp = dynamic_cast<CParticleOperatorLua *>(op.get());
			if(luaOp)
				luaOp->GetLuaObject().push(l);
			else
				Lua::Push(l, op.get());
			Lua::SetTableValue(l, t);
		}
	}));
	defCParticleSystem.def("GetRenderers", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) {
		auto t = Lua::CreateTable(l);
		auto &renderers = hComponent.GetRenderers();
		uint32_t idx = 1;
		for(auto &renderer : renderers) {
			Lua::PushInt(l, idx++);
			auto *luaRenderer = dynamic_cast<CParticleRendererLua *>(renderer.get());
			if(luaRenderer)
				luaRenderer->GetLuaObject().push(l);
			else
				Lua::Push(l, renderer.get());
			Lua::SetTableValue(l, t);
		}
	}));
	defCParticleSystem.def("FindInitializer", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, const std::string &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, const std::string &name) {
		auto &initializers = hComponent.GetInitializers();
		auto it = std::find_if(initializers.begin(), initializers.end(), [&name](const std::unique_ptr<CParticleInitializer, void (*)(CParticleInitializer *)> &initializer) { return ustring::compare(name, initializer->GetName(), false); });
		if(it == initializers.end())
			return;
		Lua::Push(l, it->get());
	}));
	defCParticleSystem.def("FindOperator", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, const std::string &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, const std::string &name) {
		auto &operators = hComponent.GetOperators();
		auto it = std::find_if(operators.begin(), operators.end(), [&name](const std::unique_ptr<CParticleOperator, void (*)(CParticleOperator *)> &op) { return ustring::compare(name, op->GetName(), false); });
		if(it == operators.end())
			return;
		Lua::Push(l, it->get());
	}));
	defCParticleSystem.def("FindRenderer", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, const std::string &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, const std::string &name) {
		auto &renderers = hComponent.GetRenderers();
		auto it = std::find_if(renderers.begin(), renderers.end(), [&name](const std::unique_ptr<CParticleRenderer, void (*)(CParticleRenderer *)> &renderer) { return ustring::compare(name, renderer->GetName(), false); });
		if(it == renderers.end())
			return;
		Lua::Push(l, it->get());
	}));
	defCParticleSystem.def("FindInitializerByType", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, const std::string &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, const std::string &name) {
		auto &initializers = hComponent.GetInitializers();
		auto it = std::find_if(initializers.begin(), initializers.end(), [&name](const std::unique_ptr<CParticleInitializer, void (*)(CParticleInitializer *)> &initializer) { return ustring::compare(name, initializer->GetType(), false); });
		if(it == initializers.end())
			return;
		Lua::Push(l, it->get());
	}));
	defCParticleSystem.def("FindOperatorByType", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, const std::string &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, const std::string &name) {
		auto &operators = hComponent.GetOperators();
		auto it = std::find_if(operators.begin(), operators.end(), [&name](const std::unique_ptr<CParticleOperator, void (*)(CParticleOperator *)> &op) { return ustring::compare(name, op->GetType(), false); });
		if(it == operators.end())
			return;
		Lua::Push(l, it->get());
	}));
	defCParticleSystem.def("FindRendererByType", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, const std::string &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, const std::string &name) {
		auto &renderers = hComponent.GetRenderers();
		auto it = std::find_if(renderers.begin(), renderers.end(), [&name](const std::unique_ptr<CParticleRenderer, void (*)(CParticleRenderer *)> &renderer) { return ustring::compare(name, renderer->GetType(), false); });
		if(it == renderers.end())
			return;
		Lua::Push(l, it->get());
	}));
	defCParticleSystem.def("AddChild", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, const std::string &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, const std::string &name) {
		auto *pt = hComponent.AddChild(name);
		if(pt == nullptr)
			return;
		Lua::Push(l, pt->GetHandle());
	}));
	defCParticleSystem.def("AddChild", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, pragma::ecs::CParticleSystemComponent &hChild) { hComponent.AddChild(hChild); }));
	defCParticleSystem.def("SetNodeTarget",
	  static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, uint32_t, BaseEntity &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, uint32_t nodeId, BaseEntity &ent) { hComponent.SetNodeTarget(nodeId, static_cast<CBaseEntity *>(&ent)); }));
	defCParticleSystem.def("SetNodeTarget", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, uint32_t, const Vector3 &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, uint32_t nodeId, const Vector3 &pos) { hComponent.SetNodeTarget(nodeId, pos); }));
	defCParticleSystem.def("GetNodeCount", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::PushInt(l, hComponent.GetNodeCount()); }));
	defCParticleSystem.def("GetNodePosition", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, uint32_t)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, uint32_t nodeId) { Lua::Push<Vector3>(l, hComponent.GetNodePosition(nodeId)); }));
	defCParticleSystem.def("GetNodeTarget", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, uint32_t)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, uint32_t nodeId) {
		auto *ent = hComponent.GetNodeTarget(nodeId);
		if(ent == nullptr)
			return;
		ent->GetLuaObject().push(l);
	}));
	defCParticleSystem.def("SetRemoveOnComplete", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, bool)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, bool b) { hComponent.SetRemoveOnComplete(b); }));
	defCParticleSystem.def("GetRenderBounds", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) {
		auto &bounds = hComponent.GetRenderBounds();
		Lua::Push<Vector3>(l, bounds.first);
		Lua::Push<Vector3>(l, bounds.second);
	}));
	defCParticleSystem.def("CalcRenderBounds", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) {
		auto bounds = hComponent.CalcRenderBounds();
		Lua::Push<Vector3>(l, bounds.first);
		Lua::Push<Vector3>(l, bounds.second);
	}));
	defCParticleSystem.def("SetRadius", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, float)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, float radius) { hComponent.SetRadius(radius); }));
	defCParticleSystem.def("GetRadius", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::PushNumber(l, hComponent.GetRadius()); }));
	defCParticleSystem.def("GetSimulationTime", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::PushNumber(l, hComponent.GetSimulationTime()); }));
	defCParticleSystem.def("SetExtent", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, float)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, float extent) { hComponent.SetExtent(extent); }));
	defCParticleSystem.def("GetExtent", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::PushNumber(l, hComponent.GetExtent()); }));
	defCParticleSystem.def("SetMaterial", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, const std::string &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, const std::string &name) { hComponent.SetMaterial(name.c_str()); }));
	defCParticleSystem.def("SetMaterial", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, Material *)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, Material *mat) { hComponent.SetMaterial(mat); }));
	defCParticleSystem.def("GetMaterial", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) {
		auto *mat = hComponent.GetMaterial();
		if(mat == nullptr)
			return;
		Lua::Push<Material *>(l, mat);
	}));
	defCParticleSystem.def("SetOrientationType", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, uint32_t)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, uint32_t orientationType) {
		hComponent.SetOrientationType(static_cast<pragma::ecs::ParticleOrientationType>(orientationType));
	}));
	defCParticleSystem.def("GetOrientationType", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::PushInt(l, umath::to_integral(hComponent.GetOrientationType())); }));
	defCParticleSystem.def("IsContinuous", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::PushBool(l, hComponent.IsContinuous()); }));
	defCParticleSystem.def("SetContinuous", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, bool)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, bool b) { hComponent.SetContinuous(b); }));
	defCParticleSystem.def("GetRemoveOnComplete", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::PushBool(l, hComponent.GetRemoveOnComplete()); }));
	defCParticleSystem.def("GetCastShadows", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::PushBool(l, hComponent.GetCastShadows()); }));
	defCParticleSystem.def("SetCastShadows", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, bool)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, bool b) { hComponent.SetCastShadows(b); }));
	defCParticleSystem.def("SetBloomColorFactor", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, const Vector4 &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, const Vector4 &factor) { hComponent.SetBloomColorFactor(factor); }));
	defCParticleSystem.def("GetBloomColorFactor", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::Push<Vector4>(l, hComponent.GetBloomColorFactor()); }));
	defCParticleSystem.def("GetEffectiveBloomColorFactor", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) {
		auto bloomCol = hComponent.GetEffectiveBloomColorFactor();
		if(bloomCol.has_value() == false)
			return;
		Lua::Push<Vector4>(l, *bloomCol);
	}));
	defCParticleSystem.def("IsBloomEnabled", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::PushBool(l, hComponent.IsBloomEnabled()); }));
	defCParticleSystem.def("SetColorFactor", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, const Vector4 &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, const Vector4 &factor) { hComponent.SetColorFactor(factor); }));
	defCParticleSystem.def("GetColorFactor", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::Push<Vector4>(l, hComponent.GetColorFactor()); }));
	defCParticleSystem.def("GetParticleCount", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::PushInt(l, hComponent.GetParticleCount()); }));
	defCParticleSystem.def("GetParticle", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, uint32_t)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, uint32_t idx) {
		auto *pt = hComponent.GetParticle(idx);
		if(pt == nullptr)
			return;
		Lua::Push(l, pt);
	}));
	defCParticleSystem.def("GetParticles", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) {
		auto &particles = hComponent.GetParticles();
		auto t = Lua::CreateTable(l);
		uint32_t ptIdx = 1u;
		for(auto &pt : particles) {
			Lua::PushInt(l, ptIdx++);
			Lua::Push<CParticle *>(l, const_cast<CParticle *>(&pt));
			Lua::SetTableValue(l, t);
		}
	}));
	defCParticleSystem.def("GetMaxParticleCount", &pragma::ecs::CParticleSystemComponent::GetMaxParticleCount);
	defCParticleSystem.def("SetMaxParticleCount", &pragma::ecs::CParticleSystemComponent::SetMaxParticleCount);
	defCParticleSystem.def("IsActive", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::PushBool(l, hComponent.IsActive()); }));
	defCParticleSystem.def("GetSceneRenderPass", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::PushInt(l, hComponent.GetSceneRenderPass()); }));
	defCParticleSystem.def("SetSceneRenderPass",
	  static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, uint32_t)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, uint32_t renderMode) { hComponent.SetSceneRenderPass(static_cast<pragma::rendering::SceneRenderPass>(renderMode)); }));
	defCParticleSystem.def("SetName", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, const std::string &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, const std::string &name) { hComponent.SetParticleSystemName(name); }));
	defCParticleSystem.def("GetName", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::PushString(l, hComponent.GetParticleSystemName()); }));
	defCParticleSystem.def("IsStatic", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::PushBool(l, hComponent.IsStatic()); }));
	defCParticleSystem.def("IsDying", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::PushBool(l, hComponent.IsDying()); }));
	defCParticleSystem.def("GetChildren", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) {
		auto &children = hComponent.GetChildren();
		auto t = Lua::CreateTable(l);
		auto idx = 1u;
		for(auto &hChild : children) {
			if(hChild.child.expired())
				continue;
			Lua::PushInt(l, idx++);
			Lua::Push(l, hChild.child.get());
			Lua::SetTableValue(l, t);
		}
	}));
	defCParticleSystem.def("GetAlphaMode", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::PushInt(l, umath::to_integral(hComponent.GetAlphaMode())); }));
	defCParticleSystem.def("GetEffectiveAlphaMode", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::PushInt(l, umath::to_integral(hComponent.GetEffectiveAlphaMode())); }));
	defCParticleSystem.def("SetAlphaMode", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, uint32_t)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, uint32_t alphaMode) { hComponent.SetAlphaMode(static_cast<pragma::ParticleAlphaMode>(alphaMode)); }));
	defCParticleSystem.def("GetEmissionRate", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::PushInt(l, hComponent.GetEmissionRate()); }));
	defCParticleSystem.def("SetEmissionRate", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, uint32_t)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, uint32_t emissionRate) { hComponent.SetEmissionRate(emissionRate); }));
	defCParticleSystem.def("SetNextParticleEmissionCount", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, uint32_t)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, uint32_t count) { hComponent.SetNextParticleEmissionCount(count); }));
	defCParticleSystem.def("PauseEmission", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { hComponent.PauseEmission(); }));
	defCParticleSystem.def("ResumeEmission", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { hComponent.ResumeEmission(); }));
	defCParticleSystem.def("GetLifeTime", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::PushNumber(l, hComponent.GetLifeTime()); }));
	defCParticleSystem.def("GetStartTime", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::PushNumber(l, hComponent.GetStartTime()); }));
	defCParticleSystem.def("GetSoftParticles", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::PushBool(l, hComponent.GetSoftParticles()); }));
	defCParticleSystem.def("SetSoftParticles", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, bool)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, bool bSoft) { hComponent.SetSoftParticles(bSoft); }));
	defCParticleSystem.def("GetSortParticles", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::PushBool(l, hComponent.GetSortParticles()); }));
	defCParticleSystem.def("SetSortParticles", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, bool)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, bool bSort) { hComponent.SetSortParticles(bSort); }));
	defCParticleSystem.def("GetInitialColor", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::Push<Color>(l, hComponent.GetInitialColor()); }));
	defCParticleSystem.def("SetInitialColor", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, const Color &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, const Color &col) { hComponent.SetInitialColor(col); }));
	defCParticleSystem.def("Simulate", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, float)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, float tDelta) { hComponent.Simulate(tDelta); }));
#if 0
	defCParticleSystem.def("Render",static_cast<void(*)(lua_State*,pragma::ecs::CParticleSystemComponent&,std::shared_ptr<prosper::ICommandBuffer>&,pragma::CSceneComponent&,pragma::CRasterizationRendererComponent&,uint32_t)>([](lua_State *l,pragma::ecs::CParticleSystemComponent &hComponent,std::shared_ptr<prosper::ICommandBuffer> &drawCmd,pragma::CSceneComponent &scene,pragma::CRasterizationRendererComponent &renderer,uint32_t renderFlags) {
		if(drawCmd->IsPrimary() == false)
			return;
		hComponent.Render(std::dynamic_pointer_cast<prosper::IPrimaryCommandBuffer>(drawCmd),scene,renderer,static_cast<pragma::ParticleRenderFlags>(renderFlags));
		}));
#endif
	defCParticleSystem.def("GetRenderParticleCount", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::PushInt(l, hComponent.GetRenderParticleCount()); }));
	defCParticleSystem.def("IsActiveOrPaused", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::PushBool(l, hComponent.IsActiveOrPaused()); }));
	defCParticleSystem.def("GetParticleBufferIndexFromParticleIndex",
	  static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, uint32_t)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, uint32_t ptIdx) { Lua::PushInt(l, hComponent.TranslateParticleIndex(ptIdx)); }));
	defCParticleSystem.def("GetParticleIndexFromParticleBufferIndex",
	  static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, uint32_t)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, uint32_t ptBufIdx) { Lua::PushInt(l, hComponent.TranslateBufferIndex(ptBufIdx)); }));
	defCParticleSystem.def("IsEmissionPaused", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::PushBool(l, hComponent.IsEmissionPaused()); }));
	defCParticleSystem.def("SetParent",
	  static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, pragma::ecs::CParticleSystemComponent &hParent) { hComponent.SetParent(&hParent); }));
	defCParticleSystem.def("RemoveChild",
	  static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, pragma::ecs::CParticleSystemComponent &hChild) { hComponent.RemoveChild(&hChild); }));
	defCParticleSystem.def("HasChild",
	  static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, pragma::ecs::CParticleSystemComponent &hChild) { Lua::PushBool(l, hComponent.HasChild(hChild)); }));
	defCParticleSystem.def("GetParent", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) {
		auto *hParent = hComponent.GetParent();
		if(hParent == nullptr)
			return;
		Lua::Push(l, hParent->GetHandle());
	}));
	defCParticleSystem.def("GetParticleBuffer", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) {
		auto &particleBuffer = hComponent.GetParticleBuffer();
		if(particleBuffer == nullptr)
			return;
		Lua::Push(l, particleBuffer);
	}));
	defCParticleSystem.def("GetParticleAnimationBuffer", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) {
		auto &animBuffer = hComponent.GetParticleAnimationBuffer();
		if(animBuffer == nullptr)
			return;
		Lua::Push(l, animBuffer);
	}));
	defCParticleSystem.def("GetAnimationSpriteSheetBuffer", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) {
		auto &spriteSheetBuffer = hComponent.GetSpriteSheetBuffer();
		if(spriteSheetBuffer == nullptr)
			return;
		Lua::Push(l, spriteSheetBuffer);
	}));
	defCParticleSystem.def("GetSpriteSheetAnimation", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) {
		auto *spriteSheetBuffer = hComponent.GetSpriteSheetAnimation();
		if(spriteSheetBuffer == nullptr)
			return;
		Lua::Push(l, spriteSheetBuffer);
	}));
	defCParticleSystem.def("GetAnimationDescriptorSet", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) {
		auto &animDescSetGroup = hComponent.GetAnimationDescriptorSetGroup();
		if(animDescSetGroup == nullptr)
			return;
		Lua::Push(l, animDescSetGroup);
	}));
	defCParticleSystem.def("IsAnimated", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::PushBool(l, hComponent.IsAnimated()); }));
	defCParticleSystem.def("ShouldAutoSimulate", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) { Lua::PushBool(l, hComponent.ShouldAutoSimulate()); }));
	defCParticleSystem.def("SetAutoSimulate", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, bool)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, bool autoSimulate) { hComponent.SetAutoSimulate(autoSimulate); }));
	defCParticleSystem.def("SetAutoSimulate", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, bool)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, bool autoSimulate) { hComponent.SetAutoSimulate(autoSimulate); }));
	defCParticleSystem.def("InitializeFromParticleSystemDefinition",
	  static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, const std::string &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, const std::string &name) { Lua::PushBool(l, hComponent.SetupParticleSystem(name)); }));
	defCParticleSystem.def("SetControlPointEntity",
	  static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, uint32_t, BaseEntity &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, uint32_t cpIdx, BaseEntity &ent) { hComponent.SetControlPointEntity(cpIdx, static_cast<CBaseEntity &>(ent)); }));
	defCParticleSystem.def("SetControlPointPosition",
	  static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, uint32_t, const Vector3 &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, uint32_t cpIdx, const Vector3 &pos) { hComponent.SetControlPointPosition(cpIdx, pos); }));
	defCParticleSystem.def("SetControlPointRotation",
	  static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, uint32_t, const Quat &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, uint32_t cpIdx, const Quat &rot) { hComponent.SetControlPointRotation(cpIdx, rot); }));
	defCParticleSystem.def("SetControlPointPose",
	  static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, uint32_t, const umath::Transform &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, uint32_t cpIdx, const umath::Transform &pose) { hComponent.SetControlPointPose(cpIdx, pose); }));
	defCParticleSystem.def("SetControlPointPose",
	  static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, uint32_t, const umath::Transform &, float)>(
	    [](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, uint32_t cpIdx, const umath::Transform &pose, float timeStamp) { hComponent.SetControlPointPose(cpIdx, pose, &timeStamp); }));
	defCParticleSystem.def("GetControlPointEntity", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, uint32_t)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, uint32_t cpIdx) {
		auto *ent = hComponent.GetControlPointEntity(cpIdx);
		if(ent == nullptr)
			return;
		ent->GetLuaObject().push(l);
	}));
	defCParticleSystem.def("GetPrevControlPointPose", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, uint32_t)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, uint32_t cpIdx) {
		float timeStamp;
		auto pose = hComponent.GetPrevControlPointPose(cpIdx, &timeStamp);
		if(pose.has_value() == false)
			return;
		Lua::Push(l, *pose);
		Lua::PushNumber(l, timeStamp);
	}));
	defCParticleSystem.def("GetControlPointPose", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, uint32_t)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, uint32_t cpIdx) {
		float timeStamp;
		auto pose = hComponent.GetControlPointPose(cpIdx, &timeStamp);
		if(pose.has_value() == false)
			return;
		Lua::Push(l, *pose);
		Lua::PushNumber(l, timeStamp);
	}));
	defCParticleSystem.def("GetControlPointPose", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &, uint32_t, float)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent, uint32_t cpIdx, float timeStamp) {
		auto pose = hComponent.GetControlPointPose(cpIdx, timeStamp);
		if(pose.has_value() == false)
			return;
		Lua::Push(l, *pose);
	}));
	defCParticleSystem.def("GenerateModel", static_cast<void (*)(lua_State *, pragma::ecs::CParticleSystemComponent &)>([](lua_State *l, pragma::ecs::CParticleSystemComponent &hComponent) {
		auto mdl = hComponent.GenerateModel();
		if(mdl == nullptr)
			return;
		Lua::Push(l, mdl);
	}));
	defCParticleSystem.def("Clear", &pragma::ecs::CParticleSystemComponent::Clear);
#if 0
	defCParticleSystem.def("GetAnimationFrameCount",static_cast<void(*)(lua_State*,pragma::ecs::CParticleSystemComponent&)>([](lua_State *l,pragma::ecs::CParticleSystemComponent &hComponent) {
		
		auto *animData = hComponent.GetRenderParticleAnimationStartData();
		if(animData == nullptr)
			return;
		Lua::PushInt(l,animData->frames);
	}));
	defCParticleSystem.def("GetAnimationFPS",static_cast<void(*)(lua_State*,pragma::ecs::CParticleSystemComponent&)>([](lua_State *l,pragma::ecs::CParticleSystemComponent &hComponent) {
		
		auto *animData = hComponent.GetAnimationData();
		if(animData == nullptr)
			return;
		Lua::PushInt(l,animData->fps);
	}));
#endif
	defCParticleSystem.add_static_constant("ORIENTATION_TYPE_ALIGNED", umath::to_integral(pragma::ecs::ParticleOrientationType::Aligned));
	defCParticleSystem.add_static_constant("ORIENTATION_TYPE_UPRIGHT", umath::to_integral(pragma::ecs::ParticleOrientationType::Upright));
	defCParticleSystem.add_static_constant("ORIENTATION_TYPE_STATIC", umath::to_integral(pragma::ecs::ParticleOrientationType::Static));
	defCParticleSystem.add_static_constant("ORIENTATION_TYPE_WORLD", umath::to_integral(pragma::ecs::ParticleOrientationType::World));
	defCParticleSystem.add_static_constant("ORIENTATION_TYPE_BILLBOARD", umath::to_integral(pragma::ecs::ParticleOrientationType::Billboard));

	defCParticleSystem.add_static_constant("ALPHA_MODE_ADDITIVE", umath::to_integral(pragma::ParticleAlphaMode::Additive));
	defCParticleSystem.add_static_constant("ALPHA_MODE_ADDITIVE_BY_COLOR", umath::to_integral(pragma::ParticleAlphaMode::AdditiveByColor));
	defCParticleSystem.add_static_constant("ALPHA_MODE_OPAQUE", umath::to_integral(pragma::ParticleAlphaMode::Opaque));
	defCParticleSystem.add_static_constant("ALPHA_MODE_MASKED", umath::to_integral(pragma::ParticleAlphaMode::Masked));
	defCParticleSystem.add_static_constant("ALPHA_MODE_TRANSLUCENT", umath::to_integral(pragma::ParticleAlphaMode::Translucent));
	defCParticleSystem.add_static_constant("ALPHA_MODE_PREMULTIPLIED", umath::to_integral(pragma::ParticleAlphaMode::Premultiplied));
	defCParticleSystem.add_static_constant("ALPHA_MODE_COUNT", umath::to_integral(pragma::ParticleAlphaMode::Count));
	register_particle_class(defCParticleSystem);
	register_modifier_class(defCParticleSystem);
	defCParticleSystem.scope[luabind::def("find_particle_system_file", static_cast<void (*)(lua_State *, const std::string &)>([](lua_State *l, const std::string &ptSystemName) {
		std::string ptName = Lua::CheckString(l, 1);
		auto ptFileName = pragma::ecs::CParticleSystemComponent::FindParticleSystemFile(ptName);
		if(ptFileName.has_value() == false)
			return;
		Lua::PushString(l, "particles/" + *ptFileName + ".wpt");
	}))];
	defCParticleSystem.scope[luabind::def("get_particle_system_definitions", static_cast<void (*)(lua_State *)>([](lua_State *l) {
		auto &ptSystemCache = pragma::ecs::CParticleSystemComponent::GetCachedParticleSystemData();
		auto t = Lua::CreateTable(l);
		for(auto &pair : ptSystemCache) {
			Lua::PushString(l, pair.first);
			push_particle_system_definition_data(l, *pair.second);
			Lua::SetTableValue(l, t);
		}
	}))];
	defCParticleSystem.scope[luabind::def("get_particle_system_definition", static_cast<void (*)(lua_State *, const std::string &)>([](lua_State *l, const std::string &ptSystemName) {
		auto lPtSystemName = ptSystemName;
		ustring::to_lower(lPtSystemName);
		auto &ptSystemCache = pragma::ecs::CParticleSystemComponent::GetCachedParticleSystemData();
		auto it = ptSystemCache.find(lPtSystemName);
		if(it == ptSystemCache.end())
			return;
		push_particle_system_definition_data(l, *it->second);
	}))];
	defCParticleSystem.scope[luabind::def("generate_model", static_cast<void (*)(lua_State *, luabind::object)>([](lua_State *l, luabind::object o) {
		int32_t t = 1;
		Lua::CheckTable(l, t);
		auto n = Lua::GetObjectLength(l, t);
		std::vector<const pragma::ecs::CParticleSystemComponent *> particleSystems {};
		particleSystems.reserve(n);
		for(auto i = decltype(n) {0u}; i < n; ++i) {
			Lua::PushInt(l, i + 1);
			Lua::GetTableValue(l, t);
			auto &hPtC = Lua::Check<pragma::ecs::CParticleSystemComponent>(l, -1);

			particleSystems.push_back(&hPtC);

			Lua::Pop(l, 1);
		}
		auto mdl = pragma::ecs::CParticleSystemComponent::GenerateModel(static_cast<CGame &>(*Engine::Get()->GetNetworkState(l)->GetGameState()), particleSystems);
		if(mdl == nullptr)
			return;
		Lua::Push(l, mdl);
	}))];
	defCParticleSystem.scope[luabind::def("read_header_data", static_cast<void (*)(lua_State *, const std::string &)>([](lua_State *l, const std::string &name) {
		auto fileHeader = pragma::ecs::CParticleSystemComponent::ReadHeader(*Engine::Get()->GetNetworkState(l), name);
		if(fileHeader.has_value() == false)
			return;
		auto t = Lua::CreateTable(l);

		Lua::PushString(l, "version");
		Lua::PushInt(l, fileHeader->version);
		Lua::SetTableValue(l, t);

		Lua::PushString(l, "numParticles");
		Lua::PushInt(l, fileHeader->numParticles);
		Lua::SetTableValue(l, t);

		Lua::PushString(l, "particleSystemNames");
		auto tNames = Lua::CreateTable(l);
		for(auto i = decltype(fileHeader->particleSystemNames.size()) {0}; i < fileHeader->particleSystemNames.size(); ++i) {
			auto &name = fileHeader->particleSystemNames.at(i);
			Lua::PushInt(l, i + 1);
			Lua::PushString(l, name);
			Lua::SetTableValue(l, tNames);
		}
		Lua::SetTableValue(l, t);

		Lua::PushString(l, "particleSystemOffsets");
		auto tOffsets = Lua::CreateTable(l);
		for(auto i = decltype(fileHeader->particleSystemOffsets.size()) {0}; i < fileHeader->particleSystemOffsets.size(); ++i) {
			auto offset = fileHeader->particleSystemOffsets.at(i);
			Lua::PushInt(l, i + 1);
			Lua::PushInt(l, offset);
			Lua::SetTableValue(l, tOffsets);
		}
		Lua::SetTableValue(l, t);
	}))];
	defCParticleSystem.scope[luabind::def("register_initializer",
	  static_cast<void (*)(lua_State *, const std::string &, luabind::object)>([](lua_State *l, const std::string &name, luabind::object oClass) { register_particle_modifier(l, pragma::LuaParticleModifierManager::Type::Initializer, name, oClass); }))];
	defCParticleSystem
	  .scope[luabind::def("register_operator", static_cast<void (*)(lua_State *, const std::string &, luabind::object)>([](lua_State *l, const std::string &name, luabind::object oClass) { register_particle_modifier(l, pragma::LuaParticleModifierManager::Type::Operator, name, oClass); }))];
	defCParticleSystem
	  .scope[luabind::def("register_renderer", static_cast<void (*)(lua_State *, const std::string &, luabind::object)>([](lua_State *l, const std::string &name, luabind::object oClass) { register_particle_modifier(l, pragma::LuaParticleModifierManager::Type::Renderer, name, oClass); }))];
	defCParticleSystem
	  .scope[luabind::def("register_emitter", static_cast<void (*)(lua_State *, const std::string &, luabind::object)>([](lua_State *l, const std::string &name, luabind::object oClass) { register_particle_modifier(l, pragma::LuaParticleModifierManager::Type::Emitter, name, oClass); }))];

	defCParticleSystem.scope[luabind::def("get_registered_initializers", static_cast<void (*)(lua_State *)>([](lua_State *l) {
		auto t = Lua::CreateTable(l);
		auto *map = GetParticleModifierMap();
		if(map == nullptr)
			return;
		uint32_t idx = 1;
		for(auto &pair : map->GetInitializers()) {
			Lua::PushInt(l, idx++);
			Lua::PushString(l, pair.first);
			Lua::SetTableValue(l, t);
		}
	}))];
	defCParticleSystem.scope[luabind::def("get_registered_operators", static_cast<void (*)(lua_State *)>([](lua_State *l) {
		auto t = Lua::CreateTable(l);
		auto *map = GetParticleModifierMap();
		if(map == nullptr)
			return;
		uint32_t idx = 1;
		for(auto &pair : map->GetOperators()) {
			Lua::PushInt(l, idx++);
			Lua::PushString(l, pair.first);
			Lua::SetTableValue(l, t);
		}
	}))];
	defCParticleSystem.scope[luabind::def("get_registered_renderers", static_cast<void (*)(lua_State *)>([](lua_State *l) {
		auto t = Lua::CreateTable(l);
		auto *map = GetParticleModifierMap();
		if(map == nullptr)
			return;
		uint32_t idx = 1;
		for(auto &pair : map->GetRenderers()) {
			Lua::PushInt(l, idx++);
			Lua::PushString(l, pair.first);
			Lua::SetTableValue(l, t);
		}
	}))];
#if 0
	// TODO
	defCParticleSystem.scope[luabind::def("get_registered_emitters",static_cast<void(*)(lua_State*,const std::string&,luabind::object)>([](lua_State *l,const std::string &name,luabind::object oClass) {
		auto t = Lua::CreateTable(l);
		auto *map = GetParticleModifierMap();
		if(map == nullptr)
			return;
		uint32_t idx = 1;
		for(auto &pair : map->GetEmitters())
		{
			Lua::PushInt(l,idx++);
			Lua::PushString(l,pair.first);
			Lua::SetTableValue(l,t);
		}
	}))];
#endif
	modEnts[defCParticleSystem];
}

///////////////

void ecs::CEnvParticleSystem::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<ecs::CParticleSystemComponent>();
}
