/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include <algorithm>
#include <sharedutils/util_weak_handle.hpp>
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/shaders/particles/c_shader_particle.hpp"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include <pragma/asset/util_asset.hpp>
#include <sprite_sheet_animation.hpp>
#include <buffers/prosper_dynamic_resizable_buffer.hpp>
#include <buffers/prosper_uniform_resizable_buffer.hpp>
#include <prosper_util.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <sharedutils/util_file.h>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/entities/environment/effects/particlesystemdata.h>
#include <datasystem_vector.h>

using namespace pragma;

// 10 MiB
#define PARTICLE_BUFFER_SIZE 10'485'760
#pragma optimize("",off)
decltype(CParticleSystemComponent::s_particleData) CParticleSystemComponent::s_particleData;
decltype(CParticleSystemComponent::s_precached) CParticleSystemComponent::s_precached;

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCENGINE CEngine *c_engine;

struct SpriteSheetTextureAnimationFrame
{
	Vector2 uvStart;
	Vector2 uvEnd;
};

Color CParticleSystemComponent::ParticleData::GetColor() const
{
	return Color{static_cast<int16_t>(color.at(0)),static_cast<int16_t>(color.at(1)),static_cast<int16_t>(color.at(2)),static_cast<int16_t>(color.at(3))};
}

CParticleSystemComponent::Node::Node(CBaseEntity *ent)
	: hEntity((ent != nullptr) ? ent->GetHandle() : EntityHandle{}),bEntity(true)
{}
CParticleSystemComponent::Node::Node(const Vector3 &pos)
	: position(pos),bEntity(false)
{}

CParticleSystemComponent *CParticleSystemComponent::Create(const std::string &fname,CParticleSystemComponent *parent,bool bRecordKeyValues,bool bAutoSpawn)
{
	auto *entChild = c_game->CreateEntity<CEnvParticleSystem>();
	if(entChild == nullptr)
		return nullptr;
	auto pParticleSysComponent = entChild->GetComponent<CParticleSystemComponent>();
	if(pParticleSysComponent.expired() || pParticleSysComponent->SetupParticleSystem(fname,parent,bRecordKeyValues) == false)
	{
		entChild->Remove();
		return nullptr;
	}
	if(bAutoSpawn)
		entChild->Spawn();
	return pParticleSysComponent.get();
}
CParticleSystemComponent *CParticleSystemComponent::Create(const std::unordered_map<std::string,std::string> &values,CParticleSystemComponent *parent,bool bRecordKeyValues,bool bAutoSpawn)
{
	auto *entChild = c_game->CreateEntity<CEnvParticleSystem>();
	if(entChild == nullptr)
		return nullptr;
	auto pParticleSysComponent = entChild->GetComponent<CParticleSystemComponent>();
	if(pParticleSysComponent.expired() || pParticleSysComponent->SetupParticleSystem(values,parent,bRecordKeyValues) == false)
	{
		entChild->Remove();
		return nullptr;
	}
	if(bAutoSpawn)
		entChild->Spawn();
	return pParticleSysComponent.get();
}
CParticleSystemComponent *CParticleSystemComponent::Create(CParticleSystemComponent *parent,bool bAutoSpawn)
{
	auto *entChild = c_game->CreateEntity<CEnvParticleSystem>();
	if(entChild == nullptr)
		return nullptr;
	auto pParticleSysComponent = entChild->GetComponent<CParticleSystemComponent>();
	if(pParticleSysComponent.expired() || pParticleSysComponent->SetupParticleSystem(parent) == false)
	{
		entChild->Remove();
		return nullptr;
	}
	if(bAutoSpawn)
		entChild->Spawn();
	return pParticleSysComponent.get();
}
Vector3 CParticleSystemComponent::GetParticlePosition(uint32_t ptIdx) const
{
	auto bufIdx = TranslateBufferIndex(ptIdx);
	auto &ptData = m_instanceData.at(bufIdx);
	auto pos = ptData.position;
	if(ShouldParticlesMoveWithEmitter())
		pos += GetEntity().GetPosition();
	return pos;
}
bool CParticleSystemComponent::IsStatic() const {return (m_operators.empty() && umath::is_flag_set(m_flags,Flags::HasMovingParticles) == false) ? true : false;}
bool CParticleSystemComponent::IsRendererBufferUpdateRequired() const {return umath::is_flag_set(m_flags,Flags::RendererBufferUpdateRequired);}

bool CParticleSystemComponent::IsParticleFilePrecached(const std::string &fname)
{
	auto fid = FileManager::GetCanonicalizedPath(fname);
	ustring::to_lower(fid);
	ufile::remove_extension_from_filename(fid);
	auto it = std::find(s_precached.begin(),s_precached.end(),fid);
	return (it != s_precached.end()) ? true : false;
}

std::optional<ParticleSystemFileHeader> CParticleSystemComponent::ReadHeader(NetworkState &nw,const std::string &fileName)
{
	auto ptPath = pragma::asset::find_file(nw,fileName,pragma::asset::Type::ParticleSystem);
	if(ptPath.has_value() == false)
		return {};
	auto fullPtPath = "particles/" +*ptPath;
	auto f = FileManager::OpenFile(fullPtPath.c_str(),"rb");
	if(f == nullptr)
		return {};
	return ReadHeader(f);
}

std::optional<ParticleSystemFileHeader> CParticleSystemComponent::ReadHeader(VFilePtr &f)
{
	std::array<int8_t,3> header = {
		static_cast<int8_t>(f->ReadChar()),
		static_cast<int8_t>(f->ReadChar()),
		static_cast<int8_t>(f->ReadChar())
	};
	if(header[0] != 'W' || header[1] != 'P' || header[2] != 'T')
		return {}; // Incorrect format
	ParticleSystemFileHeader fileHeader {};
	fileHeader.version = f->Read<uint32_t>();
	fileHeader.numParticles = f->Read<uint32_t>();
	auto numParticles = fileHeader.numParticles;
	fileHeader.particleSystemNames.resize(numParticles);
	fileHeader.particleSystemOffsets.resize(numParticles);
	for(auto i=decltype(numParticles){0};i<numParticles;++i)
	{
		fileHeader.particleSystemNames.at(i) = f->ReadString();
		fileHeader.particleSystemOffsets.at(i) = f->Read<uint64_t>();
	}
	return fileHeader;
}

static void to_cache_name(std::string &fname)
{
	fname = FileManager::GetCanonicalizedPath(fname);
	ustring::to_lower(fname);
	if(fname.length() >= 4 && fname.substr(fname.length() -4) == ".wpt")
		fname = fname.substr(0,fname.length() -4);
}
static std::unordered_map<std::string,std::vector<std::string>> s_particleFileToSystems {};
const std::vector<std::string> &CParticleSystemComponent::GetPrecachedParticleSystemFiles() {return s_precached;}
const std::unordered_map<std::string,std::unique_ptr<CParticleSystemData>> &CParticleSystemComponent::GetCachedParticleSystemData() {return s_particleData;}
std::optional<std::string> CParticleSystemComponent::FindParticleSystemFile(const std::string ptName)
{
	for(auto &pair : s_particleFileToSystems)
	{
		auto it = std::find_if(pair.second.begin(),pair.second.end(),[&ptName](const std::string &ptNameOther) {
			return ustring::compare(ptNameOther,ptName,false);
		});
		if(it == pair.second.end())
			continue;
		return pair.first;
	}
	return {};
}

bool CParticleSystemComponent::Precache(std::string fname,bool bReload)
{
	to_cache_name(fname);
	auto it = std::find(s_precached.begin(),s_precached.end(),fname);
	if(it != s_precached.end())
	{
		if(bReload == false)
			return true;
	}
	else
		s_precached.push_back(fname);
	auto path = "particles\\" +fname +".wpt";
	auto f = FileManager::OpenFile(path.c_str(),"rb");
	if(f == nullptr)
		return false;
	auto header = ReadHeader(f);
	if(header.has_value() == false)
		return false;
	auto &ptSystemNames = s_particleFileToSystems.insert(std::make_pair(fname,std::vector<std::string>{})).first->second;
	ptSystemNames.clear();
	auto numParticles = header->numParticles;
	ptSystemNames.reserve(numParticles);
	for(auto i=decltype(numParticles){0};i<numParticles;++i)
	{
		auto &name = header->particleSystemNames.at(i);
		ptSystemNames.push_back(name);
		auto offset = header->particleSystemOffsets.at(i);
		if(bReload == true || s_particleData.find(name) == s_particleData.end())
		{
			f->Seek(offset);

			auto data = std::make_unique<CParticleSystemData>();
			auto numSettings = f->Read<uint32_t>();
			auto &settings = data->settings;
			for(auto i=decltype(numSettings){0};i<numSettings;++i)
			{
				auto key = f->ReadString();
				auto val = f->ReadString();
				settings.insert(std::remove_reference<decltype(settings)>::type::value_type(key,val));
				if(key == "material")
					client->LoadMaterial(val.c_str());
			}
			std::array<std::vector<CParticleModifierData>*,3> params = {
				&data->initializers,
				&data->operators,
				&data->renderers
			};
			for(int32_t i=0;i<3;++i)
			{
				auto num = f->Read<uint32_t>();
				for(auto j=decltype(num){0};j<num;++j)
				{
					auto identifier = f->ReadString();
					auto modData = CParticleModifierData{identifier};
					auto numKeyValues = f->Read<uint32_t>();
					for(auto k=decltype(numKeyValues){0};k<numKeyValues;++k)
					{
						auto key = f->ReadString();
						auto val = f->ReadString();
						modData.settings.insert(decltype(modData.settings)::value_type(key,val));
					}
					params[i]->push_back(modData);
				}
			}
			auto &children = data->children;
			auto numChildren = f->Read<uint8_t>();
			for(auto i=decltype(numChildren){0};i<numChildren;++i)
			{
				children.push_back({});
				auto &child = children.back();
				child.childName = f->ReadString();
				if(header->version >= 2)
					child.delay = f->Read<float>();
			}
			s_particleData[name] = std::move(data);
		}
	}
	return true;
}

const std::vector<CParticle> &CParticleSystemComponent::GetParticles() const {return m_particles;}
CParticle *CParticleSystemComponent::GetParticle(size_t idx)
{
	if(idx >= GetMaxParticleCount())
		return nullptr;
	return &m_particles[idx];
}

void CParticleSystemComponent::ClearCache()
{
	for(auto it=s_particleData.begin();it!=s_particleData.end();++it)
		it->second = nullptr;
}

bool CParticleSystemComponent::SetupParticleSystem(std::string fname,CParticleSystemComponent *parent,bool bRecordKeyValues)
{
	bRecordKeyValues = true; // TODO
	if(umath::is_flag_set(m_flags,Flags::Setup))
		return true;
	auto it = s_particleData.find(fname);
	if(it == s_particleData.end())
	{
		Con::cwar<<"WARNING: Attempted to create unknown particle system '"<<fname<<"'!"<<Con::endl;
		return false;
	}
	auto &data = it->second;
	auto r = SetupParticleSystem(data->settings,parent,bRecordKeyValues);
	if(r == false)
		return false;

	SetParticleSystemName(fname);

	// Children have to be initialized before operators (in case some operators need to access the children)
	for(auto &child : data->children)
	{
		// TODO: child.delay is currently ignored, implement it!
		auto *pt = Create(child.childName,this,bRecordKeyValues);
		if(pt != nullptr)
			pt->GetEntity().Spawn();
	}

	for(auto &modData : data->initializers)
		AddInitializer(modData.name,modData.settings);
	for(auto &modData : data->operators)
		AddOperator(modData.name,modData.settings);
	for(auto &modData : data->renderers)
		AddRenderer(modData.name,modData.settings);
	if(data->renderers.empty())
	{
		std::unordered_map<std::string,std::string> values {};
		AddRenderer("sprite",values); // Default Renderer
	}
	return r;
}

void CParticleSystemComponent::SetParticleSystemName(const std::string &name) {m_particleSystemName = name;}
const std::string &CParticleSystemComponent::GetParticleSystemName() const {return m_particleSystemName;}

bool CParticleSystemComponent::SetupParticleSystem(const std::unordered_map<std::string,std::string> &values,CParticleSystemComponent *parent,bool bRecordKeyValues)
{
	if(umath::is_flag_set(m_flags,Flags::Setup))
		return true;
	umath::set_flag(m_flags,Flags::Setup);

	for(auto &kv : values)
		GetEntity().SetKeyValue(kv.first,kv.second);
	if(bRecordKeyValues)
		RecordKeyValues(values);
	m_nodes.resize(m_maxNodes,{nullptr});
	m_tNextEmission = 0.f;
	if(parent != nullptr)
		SetParent(parent);
	// TODO: 'orientation_axis' / 'orientation'?
	return true;
}

bool CParticleSystemComponent::SetupParticleSystem(CParticleSystemComponent *parent)
{
	std::unordered_map<std::string,std::string> values;
	return SetupParticleSystem(values,parent);
}

///////////////////////////////////////////

decltype(CParticleSystemComponent::PARTICLE_DATA_SIZE) CParticleSystemComponent::PARTICLE_DATA_SIZE = sizeof(CParticleSystemComponent::ParticleData);
decltype(CParticleSystemComponent::VERTEX_COUNT) CParticleSystemComponent::VERTEX_COUNT = 6;
static std::shared_ptr<prosper::IDynamicResizableBuffer> s_particleBuffer = nullptr;
static std::shared_ptr<prosper::IDynamicResizableBuffer> s_animStartBuffer = nullptr;
static std::shared_ptr<prosper::IDynamicResizableBuffer> s_animBuffer = nullptr;
const auto PARTICLE_BUFFER_INSTANCE_SIZE = sizeof(CParticleSystemComponent::ParticleData);
const auto PARTICLE_ANIM_BUFFER_INSTANCE_SIZE = sizeof(Vector2) *2;
::util::EventReply CParticleSystemComponent::HandleKeyValue(const std::string &key,const std::string &value)
{
#pragma message ("TODO: Calculate max particles automatically!")
	if(ustring::compare(key,"maxparticles",false))
	{
		if(m_state != State::Initial)
			Con::cwar<<"WARNING: Attempted to change max particle count for particle system which has already been started! Ignoring..."<<Con::endl;
		else
			m_maxParticles = ::util::to_int(value);
	}
	else if(ustring::compare(key,"limit_particle_count"))
		m_particleLimit = ::util::to_int(value);
	else if(ustring::compare(key,"emission_rate"))
		m_emissionRate = ::util::to_int(value);
	else if(ustring::compare(key,"cast_shadows"))
		SetCastShadows(::util::to_boolean(value));
	else if(ustring::compare(key,"static_scale"))
		m_worldScale = ::util::to_float(value);
	else if(ustring::compare(key,"random_start_frame"))
		umath::set_flag(m_flags,Flags::RandomStartFrame,::util::to_boolean(value));
	else if(ustring::compare(key,"material"))
		SetMaterial(client->LoadMaterial(value));
	else if(ustring::compare(key,"radius"))
		SetRadius(::util::to_float(value));
	else if(ustring::compare(key,"extent"))
		SetExtent(::util::to_float(value));
	else if(ustring::compare(key,"sort_particles"))
		umath::set_flag(m_flags,Flags::SortParticles,::util::to_boolean(value));
	else if(ustring::compare(key,"orientation_type"))
		m_orientationType = static_cast<OrientationType>(::util::to_int(value));
	else if(ustring::compare(key,"color"))
		m_initialColor = Color(value);
	else if(ustring::compare(key,"loop"))
		SetContinuous(::util::to_boolean(value));
	else if(ustring::compare(key,"origin"))
		m_origin = uvec::create(value);
	else if(ustring::compare(key,"bloom_scale"))
	{
		auto bloomFactor = ::util::to_float(value);
		SetBloomColorFactor({bloomFactor,bloomFactor,bloomFactor,1.f});
	}
	else if(ustring::compare(key,"color_factor"))
		m_colorFactor = uvec::create_v4(value);
	else if(ustring::compare(key,"bloom_color_factor"))
		SetBloomColorFactor(uvec::create_v4(value));
	else if(ustring::compare(key,"max_node_count"))
		m_maxNodes = ::util::to_int(value);
	else if(ustring::compare(key,"lifetime"))
		m_lifeTime = ::util::to_float(value);
	else if(ustring::compare(key,"soft_particles"))
		SetSoftParticles(::util::to_boolean(value));
	else if(ustring::compare(key,"texture_scrolling_enabled"))
		SetTextureScrollingEnabled(::util::to_boolean(value));
	else if(ustring::compare(key,"world_rotation"))
	{
		std::array<float,4> values;
		ustring::string_to_array(value,values.data(),atof,values.size());
		m_particleRot.w = values.at(0);
		m_particleRot.x = values.at(1);
		m_particleRot.y = values.at(2);
		m_particleRot.z = values.at(3);
	}
	else if(ustring::compare(key,"alpha_mode"))
	{
		auto alphaMode = value;
		ustring::to_lower(alphaMode);
		if(alphaMode == "additive_by_color" || alphaMode == "additive_full")
			m_alphaMode = pragma::ParticleAlphaMode::AdditiveByColor;
		else if(alphaMode == "opaque")
			m_alphaMode = pragma::ParticleAlphaMode::Opaque;
		else if(alphaMode == "masked")
			m_alphaMode = pragma::ParticleAlphaMode::Masked;
		else if(alphaMode == "translucent")
			m_alphaMode = pragma::ParticleAlphaMode::Translucent;
		else if(alphaMode == "additive")
			m_alphaMode = pragma::ParticleAlphaMode::Additive;
		else if(alphaMode == "custom")
			m_alphaMode = pragma::ParticleAlphaMode::Custom;
	}
	else if(ustring::compare(key,"premultiply_alpha"))
		SetAlphaPremultiplied(::util::to_boolean(value));
	else if(ustring::compare(key,"angles"))
	{
		auto ang = EulerAngles(value);
		m_particleRot = uquat::create(ang);
	}
	else if(ustring::compare(key,"black_to_alpha"))
		m_alphaMode = pragma::ParticleAlphaMode::AdditiveByColor;
	else if(ustring::compare(key,"move_with_emitter"))
		umath::set_flag(m_flags,Flags::MoveWithEmitter,::util::to_boolean(value));
	else if(ustring::compare(key,"rotate_with_emitter"))
		umath::set_flag(m_flags,Flags::RotateWithEmitter,::util::to_boolean(value));
	else if(ustring::compare(key,"transform_with_emitter"))
		umath::set_flag(m_flags,Flags::MoveWithEmitter | Flags::RotateWithEmitter,::util::to_boolean(value));
	else if(ustring::compare(key,"auto_simulate"))
		SetAutoSimulate(::util::to_boolean(value));
	else if(ustring::compare(key,"bounding_box_min"))
		m_renderBounds.first = uvec::create(value);
	else if(ustring::compare(key,"bounding_box_max"))
		m_renderBounds.second = uvec::create(value);
	else
		return ::util::EventReply::Unhandled;
	return ::util::EventReply::Handled;
}

CParticleSystemComponent::~CParticleSystemComponent()
{
	Stop();
	for(auto &childInfo : m_childSystems)
	{
		if(childInfo.child.expired())
			continue;
		childInfo.child->GetEntity().RemoveSafely();
	}
}

CParticleSystemComponent::ControlPoint &CParticleSystemComponent::InitializeControlPoint(ControlPointIndex idx)
{
	if(idx >= m_controlPoints.size())
		m_controlPoints.resize(idx +1);
	if(idx >= m_controlPointsPrev.size())
		m_controlPointsPrev.resize(idx +1);
	return m_controlPoints.at(idx);
}
void CParticleSystemComponent::SetControlPointEntity(ControlPointIndex idx,CBaseEntity &ent)
{
	auto &cp = InitializeControlPoint(idx);
	cp.hEntity = ent.GetHandle();

	for(auto &childData : GetChildren())
	{
		if(childData.child.expired())
			continue;
		childData.child->SetControlPointEntity(idx,ent);
	}
}
void CParticleSystemComponent::SetControlPointPosition(ControlPointIndex idx,const Vector3 &pos)
{
	auto optPose = GetControlPointPose(idx);
	auto pose = optPose.has_value() ? *optPose : umath::Transform{};
	pose.SetOrigin(pos);
	SetControlPointPose(idx,pose);
}
void CParticleSystemComponent::SetControlPointRotation(ControlPointIndex idx,const Quat &rot)
{
	auto optPose = GetControlPointPose(idx);
	auto pose = optPose.has_value() ? *optPose : umath::Transform{};
	pose.SetRotation(rot);
	SetControlPointPose(idx,pose);
}
void CParticleSystemComponent::SetControlPointPose(ControlPointIndex idx,const umath::Transform &pose,float *optTimestamp)
{
	InitializeControlPoint(idx);
	auto t = optTimestamp ? *optTimestamp : m_simulationTime;
	if(t > m_controlPoints.at(idx).simTimestamp)
		m_controlPointsPrev.at(idx) = m_controlPoints.at(idx);
	auto &cp = m_controlPoints.at(idx);
	cp.simTimestamp = t;
	cp.pose = pose;

	for(auto &childData : GetChildren())
	{
		if(childData.child.expired())
			continue;
		childData.child->SetControlPointPose(idx,pose,optTimestamp);
	}
}

CBaseEntity *CParticleSystemComponent::GetControlPointEntity(ControlPointIndex idx) const
{
	if(idx >= m_controlPoints.size())
		return nullptr;
	return static_cast<CBaseEntity*>(m_controlPoints.at(idx).hEntity.get());
}
std::optional<umath::Transform> CParticleSystemComponent::GetControlPointPose(ControlPointIndex idx,float *optOutTimestamp) const
{
	if(idx >= m_controlPoints.size())
		return {};
	auto pose = m_controlPoints.at(idx).pose;
	auto *ent = GetControlPointEntity(idx);
	if(ent)
	{
		umath::Transform entPose;
		ent->GetPose(entPose);
		pose = entPose *pose;
	}
	if(optOutTimestamp)
		*optOutTimestamp = m_controlPoints.at(idx).simTimestamp;
	return pose;
}
std::optional<umath::Transform> CParticleSystemComponent::GetPrevControlPointPose(ControlPointIndex idx,float *optOutTimestamp) const
{
	if(idx >= m_controlPointsPrev.size())
		return {};
	auto pose = m_controlPointsPrev.at(idx).pose;
	auto *ent = GetControlPointEntity(idx);
	if(ent)
	{
		umath::Transform entPose;
		ent->GetPose(entPose);
		pose = entPose *pose;
	}
	if(optOutTimestamp)
		*optOutTimestamp = m_controlPointsPrev.at(idx).simTimestamp;
	return pose;
}
std::optional<umath::Transform> CParticleSystemComponent::GetControlPointPose(ControlPointIndex idx,float t) const
{
	if(idx >= m_controlPoints.size())
		return {};
	auto &cp = m_controlPoints.at(idx);
	if(idx >= m_controlPointsPrev.size() || umath::abs(cp.simTimestamp -m_controlPointsPrev.at(idx).simTimestamp) < 0.001f)
		return GetControlPointPose(idx);
	auto &cpPrev = m_controlPointsPrev.at(idx);
	t = (t -cpPrev.simTimestamp) /(cp.simTimestamp -cpPrev.simTimestamp);
	t = umath::clamp(t,0.f,1.f);
	auto pose = *GetPrevControlPointPose(idx);
	pose.Interpolate(*GetControlPointPose(idx),t);
	return pose;
}

const std::vector<std::unique_ptr<CParticleInitializer,void(*)(CParticleInitializer*)>> &CParticleSystemComponent::GetInitializers() const {return const_cast<CParticleSystemComponent*>(this)->GetInitializers();}
const std::vector<std::unique_ptr<CParticleOperator,void(*)(CParticleOperator*)>> &CParticleSystemComponent::GetOperators() const {return const_cast<CParticleSystemComponent*>(this)->GetOperators();}
const std::vector<std::unique_ptr<CParticleRenderer,void(*)(CParticleRenderer*)>> &CParticleSystemComponent::GetRenderers() const {return const_cast<CParticleSystemComponent*>(this)->GetRenderers();}
std::vector<std::unique_ptr<CParticleInitializer,void(*)(CParticleInitializer*)>> &CParticleSystemComponent::GetInitializers() {return m_initializers;}
std::vector<std::unique_ptr<CParticleOperator,void(*)(CParticleOperator*)>> &CParticleSystemComponent::GetOperators() {return m_operators;}
std::vector<std::unique_ptr<CParticleRenderer,void(*)(CParticleRenderer*)>> &CParticleSystemComponent::GetRenderers() {return m_renderers;}

void CParticleSystemComponent::SetContinuous(bool b)
{
	BaseEnvParticleSystemComponent::SetContinuous(b);
	if(b == false)
	{
		if(GetRemoveOnComplete() && m_state == State::Complete)
			GetEntity().RemoveSafely();
	}
}

bool CParticleSystemComponent::ShouldParticlesRotateWithEmitter() const {return umath::is_flag_set(m_flags,Flags::RotateWithEmitter);}
bool CParticleSystemComponent::ShouldParticlesMoveWithEmitter() const {return umath::is_flag_set(m_flags,Flags::MoveWithEmitter);}

void CParticleSystemComponent::SetAutoSimulate(bool b) {umath::set_flag(m_flags,Flags::AutoSimulate,b);}
bool CParticleSystemComponent::ShouldAutoSimulate() const {return umath::is_flag_set(m_flags,Flags::AutoSimulate);}

Vector3 CParticleSystemComponent::PointToParticleSpace(const Vector3 &p,bool bRotateWithEmitter) const
{
	auto r = p;
	if(bRotateWithEmitter == true)
	{
		auto pTrComponent = GetEntity().GetTransformComponent();
		if(pTrComponent.valid())
			uvec::rotate(&r,pTrComponent->GetOrientation());
	}
	if(ShouldParticlesMoveWithEmitter())
	{
		auto pTrComponent = GetEntity().GetTransformComponent();
		if(pTrComponent.valid())
			r += pTrComponent->GetPosition();
	}
	return r;
}
Vector3 CParticleSystemComponent::PointToParticleSpace(const Vector3 &p) const {return PointToParticleSpace(p,ShouldParticlesRotateWithEmitter());}
Vector3 CParticleSystemComponent::DirectionToParticleSpace(const Vector3 &p,bool bRotateWithEmitter) const
{
	auto r = p;
	if(bRotateWithEmitter == true)
	{
		auto pTrComponent = GetEntity().GetTransformComponent();
		if(pTrComponent.valid())
			uvec::rotate(&r,pTrComponent->GetOrientation());
	}
	return r;
}
Vector3 CParticleSystemComponent::DirectionToParticleSpace(const Vector3 &p) const {return DirectionToParticleSpace(p,ShouldParticlesRotateWithEmitter());}

void CParticleSystemComponent::InitializeBuffers()
{
	if(s_particleBuffer == nullptr)
	{
		auto instanceCount = 32'768ull;
		auto maxInstanceCount = instanceCount *40u;
		auto instanceSize = PARTICLE_BUFFER_INSTANCE_SIZE;
		prosper::util::BufferCreateInfo createInfo {};
		createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
		createInfo.size = instanceSize *maxInstanceCount;
		createInfo.usageFlags = prosper::BufferUsageFlags::VertexBufferBit | prosper::BufferUsageFlags::TransferDstBit;
		s_particleBuffer = c_engine->GetRenderContext().CreateDynamicResizableBuffer(createInfo,instanceSize *maxInstanceCount,0.05f);
		s_particleBuffer->SetDebugName("particle_instance_buf");
	}
	if(s_animStartBuffer == nullptr)
	{
		auto instanceCount = 524'288ull;
		auto maxInstanceCount = instanceCount *5u;
		auto instanceSize = PARTICLE_ANIM_BUFFER_INSTANCE_SIZE;
		prosper::util::BufferCreateInfo createInfo {};
		createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
		createInfo.size = instanceSize *maxInstanceCount;
		createInfo.usageFlags = prosper::BufferUsageFlags::VertexBufferBit | prosper::BufferUsageFlags::TransferDstBit;
		s_animStartBuffer = c_engine->GetRenderContext().CreateDynamicResizableBuffer(createInfo,instanceSize *maxInstanceCount,0.01f);
		s_animStartBuffer->SetDebugName("particle_anim_start_buf");
	}
	if(s_animBuffer == nullptr)
	{
		auto instanceCount = 4'096u;
		auto maxInstanceCount = instanceCount *5u;
		auto instanceSize = sizeof(ParticleAnimationData);
		prosper::util::BufferCreateInfo createInfo {};
		createInfo.memoryFeatures = prosper::MemoryFeatureFlags::DeviceLocal;
		createInfo.size = instanceSize *maxInstanceCount;
		createInfo.usageFlags = prosper::BufferUsageFlags::StorageBufferBit | prosper::BufferUsageFlags::TransferDstBit;
		s_animBuffer = c_engine->GetRenderContext().CreateDynamicResizableBuffer(createInfo,instanceSize *maxInstanceCount,0.01f);
		s_animBuffer->SetDebugName("particle_anim_data_buf");
	}
}
void CParticleSystemComponent::ClearBuffers()
{
	s_particleBuffer = nullptr;
	s_animStartBuffer = nullptr;
	s_animBuffer = nullptr;
}
float CParticleSystemComponent::GetStaticWorldScale() const {return m_worldScale;}
void CParticleSystemComponent::SetStaticWorldScale(float scale) {m_worldScale = scale;}
void CParticleSystemComponent::SetSoftParticles(bool bSmooth) {umath::set_flag(m_flags,Flags::SoftParticles,bSmooth);}
bool CParticleSystemComponent::GetSoftParticles() const {return umath::is_flag_set(m_flags,Flags::SoftParticles);}
void CParticleSystemComponent::SetSortParticles(bool sort) {umath::set_flag(m_flags,Flags::SortParticles,sort);}
bool CParticleSystemComponent::GetSortParticles() const {return umath::is_flag_set(m_flags,Flags::SortParticles);}
const Color &CParticleSystemComponent::GetInitialColor() const {return m_initialColor;}
void CParticleSystemComponent::SetInitialColor(const Color &col) {m_initialColor = col;}

void CParticleSystemComponent::SetCastShadows(bool b) {umath::set_flag(m_flags,Flags::CastShadows,b);}
bool CParticleSystemComponent::GetCastShadows() const {return umath::is_flag_set(m_flags,Flags::CastShadows);}
const Vector4 &CParticleSystemComponent::GetBloomColorFactor() const {return m_bloomColorFactor;}
void CParticleSystemComponent::SetBloomColorFactor(const Vector4 &colorFactor) {m_bloomColorFactor = colorFactor;}
std::optional<Vector4> CParticleSystemComponent::GetEffectiveBloomColorFactor() const
{
	auto colorFactor = GetBloomColorFactor();
	auto *mat = GetMaterial();
	if(mat)
	{
		auto &data = mat->GetDataBlock();
		auto &dColorFactor = data->GetValue("bloom_color_factor");
		if(dColorFactor != nullptr && typeid(*dColorFactor) == typeid(ds::Vector4))
		{
			auto &matColorFactor = static_cast<ds::Vector4*>(dColorFactor.get())->GetValue();
			colorFactor *= matColorFactor;
		}
	}
	if(colorFactor.r == 0.f && colorFactor.g == 0.f && colorFactor.b == 0.f)
		return {};
	return colorFactor;
}
bool CParticleSystemComponent::IsBloomEnabled() const {return GetEffectiveBloomColorFactor().has_value();}
const Vector4 &CParticleSystemComponent::GetColorFactor() const {return m_colorFactor;}
void CParticleSystemComponent::SetColorFactor(const Vector4 &colorFactor) {m_colorFactor = colorFactor;}
const std::pair<Vector3,Vector3> &CParticleSystemComponent::GetRenderBounds() const {return m_renderBounds;}
std::pair<Vector3,Vector3> CParticleSystemComponent::CalcRenderBounds() const
{
	std::pair<Vector3,Vector3> bounds {};
	bounds.first = uvec::max();
	bounds.second = uvec::min();

	uint32_t n = 0;
	for(auto i=decltype(m_numRenderParticles){0u};i<m_numRenderParticles;++i)
	{
		auto &data = m_instanceData.at(i);
		auto &ptPos = data.position;
		if(data.color.at(3) == 0)
			continue;
		auto radius = data.radius;
		auto ptMin = ptPos -Vector3{radius,radius,radius};
		auto ptMax = ptPos +Vector3{radius,radius,radius};
		uvec::min(&bounds.first,ptMin);
		uvec::max(&bounds.second,ptMax);
		++n;
	}
	if(n == 0)
		return {};
	return bounds;
}

CParticleSystemComponent::OrientationType CParticleSystemComponent::GetOrientationType() const {return m_orientationType;}
void CParticleSystemComponent::SetOrientationType(OrientationType type)
{
	m_orientationType = type;
}

void CParticleSystemComponent::SetRadius(float r)
{
	m_radius = r;
	m_extent = umath::sqrt(umath::pow2(r) *2.f);
}
void CParticleSystemComponent::SetExtent(float ext)
{
	m_extent = ext;
	m_radius = umath::sqrt(umath::pow2(ext) /2.f);
}
float CParticleSystemComponent::GetRadius() const {return m_radius;}
float CParticleSystemComponent::GetExtent() const {return m_extent;}

void CParticleSystemComponent::SetMaterial(Material *mat) {m_material = mat;}
void CParticleSystemComponent::SetMaterial(const char *mat) {SetMaterial(client->LoadMaterial(mat));}
Material *CParticleSystemComponent::GetMaterial() const {return m_material;}

CParticleInitializer *CParticleSystemComponent::AddInitializer(std::string identifier,const std::unordered_map<std::string,std::string> &values)
{
	ustring::to_lower(identifier);
	auto *map = GetParticleModifierMap();
	auto factory = map->FindInitializer(identifier);
	if(factory == nullptr)
	{
		Con::cwar<<"WARNING: Attempted to create unknown particle initializer '"<<identifier<<"'! Ignoring..."<<Con::endl;
		return nullptr;
	}
	auto initializer = factory(*this,values);
	if(initializer == nullptr)
		return nullptr;
	if(IsRecordingKeyValues())
		initializer->RecordKeyValues(values);
	m_initializers.push_back(std::move(initializer));
	return m_initializers.back().get();
}
CParticleOperator *CParticleSystemComponent::AddOperator(std::string identifier,const std::unordered_map<std::string,std::string> &values)
{
	ustring::to_lower(identifier);
	auto *map = GetParticleModifierMap();
	auto factory = map->FindOperator(identifier);
	if(factory == nullptr)
	{
		Con::cwar<<"WARNING: Attempted to create unknown particle operator '"<<identifier<<"'! Ignoring..."<<Con::endl;
		return nullptr;
	}
	auto op = factory(*this,values);
	if(op == nullptr)
		return nullptr;
	if(IsRecordingKeyValues())
		op->RecordKeyValues(values);
	m_operators.push_back(std::move(op));
	return m_operators.back().get();
}
CParticleRenderer *CParticleSystemComponent::AddRenderer(std::string identifier,const std::unordered_map<std::string,std::string> &values)
{
	ustring::to_lower(identifier);
	auto *map = GetParticleModifierMap();
	auto factory = map->FindRenderer(identifier);
	if(factory == nullptr)
	{
		Con::cwar<<"WARNING: Attempted to create unknown particle renderer '"<<identifier<<"'! Ignoring..."<<Con::endl;
		return nullptr;
	}
	auto op = factory(*this,values);
	if(op == nullptr)
		return nullptr;
	if(IsRecordingKeyValues())
		op->RecordKeyValues(values);
	m_renderers.push_back(std::move(op));
	return m_renderers.back().get();
}

void CParticleSystemComponent::RemoveInitializer(const std::string &name)
{
	auto it = std::find_if(m_initializers.begin(),m_initializers.end(),[&name](const std::unique_ptr<CParticleInitializer,void(*)(CParticleInitializer*)> &initializer) {
		return ustring::compare(initializer->GetName(),name,false);
	});
	if(it == m_initializers.end())
		return;
	m_initializers.erase(it);
}
void CParticleSystemComponent::RemoveOperator(const std::string &name)
{
	auto it = std::find_if(m_operators.begin(),m_operators.end(),[&name](const std::unique_ptr<CParticleOperator,void(*)(CParticleOperator*)> &initializer) {
		return ustring::compare(initializer->GetName(),name,false);
	});
	if(it == m_operators.end())
		return;
	m_operators.erase(it);
}
void CParticleSystemComponent::RemoveRenderer(const std::string &name)
{
	auto it = std::find_if(m_renderers.begin(),m_renderers.end(),[&name](const std::unique_ptr<CParticleRenderer,void(*)(CParticleRenderer*)> &initializer) {
		return ustring::compare(initializer->GetName(),name,false);
	});
	if(it == m_renderers.end())
		return;
	m_renderers.erase(it);
}
void CParticleSystemComponent::RemoveInitializersByType(const std::string &type)
{
	for(auto it=m_initializers.begin();it!=m_initializers.end();)
	{
		auto &initializer = *it;
		if(ustring::compare(initializer->GetType(),type,false) == false)
		{
			++it;
			continue;
		}
		it = m_initializers.erase(it);
	}
}
void CParticleSystemComponent::RemoveOperatorsByType(const std::string &type)
{
	for(auto it=m_operators.begin();it!=m_operators.end();)
	{
		auto &op = *it;
		if(ustring::compare(op->GetType(),type,false) == false)
		{
			++it;
			continue;
		}
		it = m_operators.erase(it);
	}
}
void CParticleSystemComponent::RemoveRenderersByType(const std::string &type)
{
	for(auto it=m_renderers.begin();it!=m_renderers.end();)
	{
		auto &renderer = *it;
		if(ustring::compare(renderer->GetType(),type,false) == false)
		{
			++it;
			continue;
		}
		it = m_renderers.erase(it);
	}
}

const CParticleSystemComponent *CParticleSystemComponent::GetParent() const {return const_cast<CParticleSystemComponent*>(this)->GetParent();}
CParticleSystemComponent *CParticleSystemComponent::GetParent()
{
	if(m_hParent.expired())
		return nullptr;
	return m_hParent.get();
}

void CParticleSystemComponent::SetParent(CParticleSystemComponent *particle)
{
	if(m_hParent.valid())
	{
		auto *parent = m_hParent.get();
		if(parent == particle)
			return;
		m_hParent = ::util::WeakHandle<CParticleSystemComponent>{};
		if(parent != nullptr)
			parent->RemoveChild(this);
	}
	if(particle == nullptr)
	{
		m_hParent = ::util::WeakHandle<CParticleSystemComponent>{};
		return;
	}
	m_hParent = ::util::WeakHandle<CParticleSystemComponent>{std::static_pointer_cast<CParticleSystemComponent>(particle->shared_from_this())};
	particle->AddChild(*this);
	auto pTrComponent = GetEntity().GetTransformComponent();
	auto pTrComponentPt = particle->GetEntity().GetTransformComponent();
	if(pTrComponent.valid() && pTrComponentPt.valid())
	{
		pTrComponent->SetPosition(pTrComponentPt->GetPosition());
		pTrComponent->SetOrientation(pTrComponentPt->GetOrientation());
	}
}

CParticleSystemComponent *CParticleSystemComponent::AddChild(const std::string &name)
{
	auto *pt = Create(name,this,IsRecordingKeyValues());
	if(pt == nullptr)
		return nullptr;
	pt->GetEntity().Spawn();
	AddChild(*pt);
	return pt;
}

void CParticleSystemComponent::AddChild(CParticleSystemComponent &particle,float delay)
{
	if(HasChild(particle))
		return;
	ChildData childData {};
	childData.child = ::util::WeakHandle<CParticleSystemComponent>{std::static_pointer_cast<CParticleSystemComponent>(particle.shared_from_this())};
	childData.delay = delay;
	m_childSystems.push_back(childData);
	particle.SetParent(this);
	particle.SetContinuous(IsContinuous());
	uint32_t cpIdx = 0;
	for(auto &cp : m_controlPoints)
	{
		if(cp.hEntity.IsValid())
			particle.SetControlPointEntity(cpIdx,static_cast<CBaseEntity&>(*cp.hEntity.get()));
		particle.SetControlPointPose(cpIdx,cp.pose);
		++cpIdx;
	}
}

void CParticleSystemComponent::RemoveChild(CParticleSystemComponent *particle)
{
	auto it = std::find_if(m_childSystems.begin(),m_childSystems.end(),[particle](const ChildData &childInfo) {
		return childInfo.child.get() == particle;
	});
	if(it == m_childSystems.end())
		return;
	auto *child = it->child.get();
	m_childSystems.erase(it);
	if(child)
		child->SetParent(nullptr);
}

bool CParticleSystemComponent::HasChild(CParticleSystemComponent &particle)
{
	auto it = std::find_if(m_childSystems.begin(),m_childSystems.end(),[&particle](const ChildData &hchild) {
		return (hchild.child.get() == &particle) ? true : false;
	});
	return (it != m_childSystems.end()) ? true : false;
}

const std::shared_ptr<prosper::IBuffer> &CParticleSystemComponent::GetParticleBuffer() const {return m_bufParticles;}
const std::shared_ptr<prosper::IBuffer> &CParticleSystemComponent::GetParticleAnimationBuffer() const {return m_bufParticleAnimData;}
const std::shared_ptr<prosper::IBuffer> &CParticleSystemComponent::GetSpriteSheetBuffer() const {return m_bufSpriteSheet;}
prosper::IDescriptorSet *CParticleSystemComponent::GetAnimationDescriptorSet() {return (m_descSetGroupAnimation != nullptr) ? m_descSetGroupAnimation->GetDescriptorSet() : nullptr;}
const std::shared_ptr<prosper::IDescriptorSetGroup> &CParticleSystemComponent::GetAnimationDescriptorSetGroup() const {return m_descSetGroupAnimation;}

bool CParticleSystemComponent::IsAnimated() const {return m_descSetGroupAnimation != nullptr;}

void CParticleSystemComponent::Start()
{
	CreateParticle();
	if(IsActiveOrPaused())
		Stop();
	umath::remove_flag(m_flags,Flags::Dying);
	for(auto &pt : m_particles)
		pt.Resurrect();
	m_state = State::Active;
	m_tLastEmission = 0.0;
	m_tLifeTime = 0.0;
	m_tStartTime = c_game->RealTime();
	m_currentParticleLimit = m_particleLimit;

	// Children have to be started before operators are initialized,
	// in case one of the operators needs to access a child
	for(auto &hChild : m_childSystems)
	{
		if(hChild.child.expired())
			continue;
		hChild.child->Start();
	}
	//

	std::sort(m_initializers.begin(),m_initializers.end(),[](const std::unique_ptr<CParticleInitializer,void(*)(CParticleInitializer*)> &a,const std::unique_ptr<CParticleInitializer,void(*)(CParticleInitializer*)> &b) {
		return a->GetPriority() > b->GetPriority();
	});
	std::sort(m_operators.begin(),m_operators.end(),[](const std::unique_ptr<CParticleOperator,void(*)(CParticleOperator*)> &a,const std::unique_ptr<CParticleOperator,void(*)(CParticleOperator*)> &b) {
		return a->GetPriority() > b->GetPriority();
	});
	std::sort(m_renderers.begin(),m_renderers.end(),[](const std::unique_ptr<CParticleRenderer,void(*)(CParticleRenderer*)> &a,const std::unique_ptr<CParticleRenderer,void(*)(CParticleRenderer*)> &b) {
		return a->GetPriority() > b->GetPriority();
	});

	//
	for(auto &init : m_initializers)
		init->OnParticleSystemStarted();
	for(auto &op : m_operators)
		op->OnParticleSystemStarted(); // Operators have to be initialized before buffers are initialized
	for(auto &r : m_renderers)
		r->OnParticleSystemStarted();
	//

	if(m_maxParticles > 0)
	{
		m_bufParticleAnimData = nullptr;
		m_descSetGroupAnimation = nullptr;
		if(m_material != nullptr)
		{
			if(IsTextureScrollingEnabled())
			{

			}
			else if(pragma::ShaderParticle2DBase::DESCRIPTOR_SET_ANIMATION.IsValid())
			{
				auto *spriteSheetAnim = static_cast<CMaterial*>(m_material)->GetSpriteSheetAnimation();
				m_spriteSheetAnimationData = spriteSheetAnim ? std::make_unique<SpriteSheetAnimation>(*spriteSheetAnim) : nullptr;

				if(m_spriteSheetAnimationData)
				{
					uint32_t numFrames = 0;
					for(auto &seq : spriteSheetAnim->sequences)
						numFrames += seq.frames.size();

					std::vector<SpriteSheetTextureAnimationFrame> frames {};
					frames.resize(numFrames);
					uint32_t frameIndex = 0;
					for(auto &seq : spriteSheetAnim->sequences)
					{
						for(auto &frame : seq.frames)
						{
							auto &frameData = frames.at(frameIndex++);
							frameData.uvStart = frame.uvStart;
							frameData.uvEnd = frame.uvEnd;
						}
					}
					m_bufSpriteSheet = s_animBuffer->AllocateBuffer(frames.size() *sizeof(frames.front()),frames.data());

					m_descSetGroupAnimation = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderParticle2DBase::DESCRIPTOR_SET_ANIMATION);
					m_descSetGroupAnimation->GetDescriptorSet()->SetBindingUniformBuffer(
						*m_bufSpriteSheet,0u
					);
				}

				auto &data = m_material->GetDataBlock();
				if(data != nullptr)
				{
					auto &anim = data->GetValue("animation");
					if(anim)
					{
						if(anim->IsBlock())
						{
							// TODO: Re-implement this and translate it to SpriteSheetAnimation data
#if 0
							auto &animBlock = *std::static_pointer_cast<ds::Block>(anim);
							m_animData = std::make_unique<AnimationData>();

							animBlock.GetInt("offset",&m_animData->offset);
							animBlock.GetInt("frames",&m_animData->frames);
							animBlock.GetInt("fps",&m_animData->fps);
							animBlock.GetInt("rows",&m_animData->rows);
							animBlock.GetInt("columns",&m_animData->columns);
							m_bufAnim = s_animBuffer->AllocateBuffer(m_animData.get());

							m_descSetGroupAnimation = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderParticle2DBase::DESCRIPTOR_SET_ANIMATION);
							m_descSetGroupAnimation->GetDescriptorSet()->SetBindingUniformBuffer(
								*m_bufAnim,0u
							);
#endif
						}
					}
				}
			}
		}
		
		m_bufParticles = s_particleBuffer->AllocateBuffer(m_maxParticles *PARTICLE_BUFFER_INSTANCE_SIZE);
		auto bAnimated = IsAnimated();
		if(bAnimated)
			m_bufParticleAnimData = s_animStartBuffer->AllocateBuffer(m_maxParticles *PARTICLE_ANIM_BUFFER_INSTANCE_SIZE);
		else
			m_bufParticleAnimData = nullptr;

		//if(m_maxParticles != m_maxParticlesCur)
		//{
			m_particles.resize(m_maxParticles);
			m_sortedParticleIndices.resize(m_particles.size());

			m_particleIndicesToBufferIndices.resize(m_particles.size());
			std::fill(m_particleIndicesToBufferIndices.begin(),m_particleIndicesToBufferIndices.end(),0);
			m_bufferIndicesToParticleIndices.resize(m_particles.size());
			std::fill(m_bufferIndicesToParticleIndices.begin(),m_bufferIndicesToParticleIndices.end(),0);

			for(auto i=decltype(m_maxParticles){0};i<m_maxParticles;++i)
			{
				m_particles[i].SetIndex(i);
				m_sortedParticleIndices[i] = i;
			}
			m_instanceData.resize(m_maxParticles);
			if(bAnimated)
				m_particleAnimData.resize(m_maxParticles);
		//}
		m_maxParticlesCur = m_maxParticles;
	}
}

void CParticleSystemComponent::Stop()
{
	if(!IsActiveOrPaused())
		return;
	m_state = State::Complete;
	for(auto &init : m_initializers)
		init->OnParticleSystemStopped();
	for(auto &op : m_operators)
		op->OnParticleSystemStopped();
	for(auto &r : m_renderers)
		r->OnParticleSystemStopped();
	m_numParticles = 0;
	m_numRenderParticles = 0;
	m_maxParticlesCur = 0;
	m_prevMaxParticlesCur = 0;
	m_simulationTime = 0.f;
	m_tNextEmission = 0.f;
	m_tLastEmission = 0.f;
	m_tStartTime = 0.0;
	m_currentParticleLimit = std::numeric_limits<uint32_t>::max();

	m_particles.clear();
	m_sortedParticleIndices.clear();
	m_instanceData.clear();
	m_particleIndicesToBufferIndices.clear();
	m_bufferIndicesToParticleIndices.clear();
	m_particleAnimData.clear();
	m_bufParticles = nullptr;
	m_bufParticleAnimData = nullptr;
	for(auto &hChild : m_childSystems)
	{
		if(hChild.child.expired())
			continue;
		hChild.child->Stop();
	}
	m_tLifeTime = 0.0;
	OnComplete();
}

double CParticleSystemComponent::GetLifeTime() const {return m_tLifeTime;}
float CParticleSystemComponent::GetSimulationTime() const {return m_simulationTime;}

double CParticleSystemComponent::GetStartTime() const {return m_tStartTime;}

const std::vector<CParticleSystemComponent::ParticleData> &CParticleSystemComponent::GetRenderParticleData() const {return m_instanceData;}
const std::vector<CParticleSystemComponent::ParticleAnimationData> &CParticleSystemComponent::GetParticleAnimationData() const {return m_particleAnimData;}

bool CParticleSystemComponent::IsActive() const {return m_state == State::Active;}
bool CParticleSystemComponent::IsEmissionPaused() const {return m_state == State::Paused;}
bool CParticleSystemComponent::IsActiveOrPaused() const {return IsActive() || IsEmissionPaused();}

uint32_t CParticleSystemComponent::GetParticleCount() const {return m_numParticles;}
uint32_t CParticleSystemComponent::GetRenderParticleCount() const {return m_numRenderParticles;}
uint32_t CParticleSystemComponent::GetMaxParticleCount() const {return m_maxParticles;}

void CParticleSystemComponent::OnRemove()
{
	BaseEnvParticleSystemComponent::OnRemove();
	for(auto &hChild : m_childSystems)
	{
		if(hChild.child.expired())
			continue;
		hChild.child->GetEntity().RemoveSafely();
	}
}

bool CParticleSystemComponent::IsDying() const {return umath::is_flag_set(m_flags,Flags::Dying);}

void CParticleSystemComponent::Die(float maxRemainingLifetime)
{
	umath::add_flag(m_flags,Flags::Dying);
	for(auto &pt : m_particles)
	{
		pt.Die();
		if(pt.GetLife() > maxRemainingLifetime)
			pt.SetLife(maxRemainingLifetime);
	}
	for(auto &hChild : m_childSystems)
	{
		if(hChild.child.expired())
			hChild.child->Die();
	}
}

bool CParticleSystemComponent::FindFreeParticle(uint32_t *idx)
{
	if(umath::is_flag_set(m_flags,Flags::Dying))
		return false;
	for(auto i=m_idxLast;i<m_maxParticlesCur;++i)
	{
		auto &p = m_particles[i];
		if(p.GetLife() <= 0)
		{
			m_idxLast = i +1;
			*idx = i;
			return true;
		}
	}
	if(IsContinuous() == false)
		return false;
	for(auto i=decltype(m_idxLast){0};i<m_idxLast;++i)
	{
		auto &p = m_particles[i];
		if(p.GetLife() <= 0)
		{
			m_idxLast = i;
			*idx = i;
			return true;
		}
	}
	*idx = 0;
	return true;
}

void CParticleSystemComponent::SetNodeTarget(uint32_t node,CBaseEntity *ent)
{
	if(node == 0)
		return;
	--node;
	if(node >= m_nodes.size())
		return;
	m_nodes[node].hEntity = ent->GetHandle();
	m_nodes[node].bEntity = true;
}
void CParticleSystemComponent::SetNodeTarget(uint32_t node,const Vector3 &pos)
{
	if(node == 0)
		return;
	--node;
	if(node >= m_nodes.size())
		return;
	m_nodes[node].position = pos;
	m_nodes[node].bEntity = false;
}
uint32_t CParticleSystemComponent::GetNodeCount() const {return m_nodes.size() +1;}
Vector3 CParticleSystemComponent::GetNodePosition(uint32_t node) const
{
	if(node == 0)
	{
		auto pTrComponent = GetEntity().GetTransformComponent();
		return pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
	}
	--node;
	if(node >= m_nodes.size() || (m_nodes[node].bEntity == true && !m_nodes[node].hEntity.IsValid()))
		return {0.f,0.f,0.f};
	if(m_nodes[node].bEntity == false)
		return m_nodes[node].position;
	auto pTrComponent = m_nodes[node].hEntity.get()->GetTransformComponent();
	return pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
}
CBaseEntity *CParticleSystemComponent::GetNodeTarget(uint32_t node) const
{
	if(node == 0)
		return nullptr;
	--node;
	if(node >= m_nodes.size())
		return nullptr;
	return static_cast<CBaseEntity*>(m_nodes[node].hEntity.get());
}

CallbackHandle CParticleSystemComponent::AddRenderCallback(const std::function<void(void)> &cb)
{
	auto hCb = FunctionCallback<void>::Create(cb);
	AddRenderCallback(hCb);
	return hCb;
}
void CParticleSystemComponent::AddRenderCallback(const CallbackHandle &hCb) {m_renderCallbacks.push_back(hCb);}
pragma::ParticleAlphaMode CParticleSystemComponent::GetAlphaMode() const {return m_alphaMode;}
pragma::ParticleAlphaMode CParticleSystemComponent::GetEffectiveAlphaMode() const
{
	auto alphaMode = GetAlphaMode();
	if(alphaMode != ParticleAlphaMode::Additive)
		return alphaMode;
	auto *mat = GetMaterial();
	if(mat)
	{
		auto &data = mat->GetDataBlock();
		if(data->GetBool("additive"))
			alphaMode = ParticleAlphaMode::AdditiveByColor;
	}
	return alphaMode;
}
void CParticleSystemComponent::SetAlphaMode(pragma::ParticleAlphaMode alphaMode) {m_alphaMode = alphaMode;}
void CParticleSystemComponent::SetTextureScrollingEnabled(bool b) {umath::set_flag(m_flags,Flags::TextureScrollingEnabled,b);}
bool CParticleSystemComponent::IsTextureScrollingEnabled() const {return umath::is_flag_set(m_flags,Flags::TextureScrollingEnabled);}

bool CParticleSystemComponent::IsAlphaPremultiplied() const {return umath::is_flag_set(m_flags,Flags::PremultiplyAlpha);}
void CParticleSystemComponent::SetAlphaPremultiplied(bool b) {umath::set_flag(m_flags,Flags::PremultiplyAlpha,b);}
uint32_t CParticleSystemComponent::GetEmissionRate() const {return m_emissionRate;}
void CParticleSystemComponent::SetEmissionRate(uint32_t emissionRate) {m_emissionRate = emissionRate;}
void CParticleSystemComponent::SetNextParticleEmissionCount(uint32_t count) {m_nextParticleEmissionCount = count;}
void CParticleSystemComponent::PauseEmission() {m_state = State::Paused;}
void CParticleSystemComponent::ResumeEmission()
{
	if(m_state != State::Paused)
		return;
	m_state = State::Active;
}
void CParticleSystemComponent::SetAlwaysSimulate(bool b) {umath::set_flag(m_flags,Flags::AlwaysSimulate,b);}

void CParticleSystemComponent::Render(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,ParticleRenderFlags renderFlags)
{
	if(umath::is_flag_set(renderFlags,ParticleRenderFlags::Bloom) && IsBloomEnabled() == false)
		return;
	m_tLastEmission = c_game->RealTime();
	if(IsActiveOrPaused() == false)
	{
		if(umath::is_flag_set(m_flags,Flags::Dying))
			Stop();
		return;
	}
	auto numRenderParticles = GetRenderParticleCount();
	for(auto &hChild : m_childSystems)
	{
		if(hChild.child.expired() || hChild.child->IsActiveOrPaused() == false)
			continue;
		numRenderParticles += hChild.child->GetRenderParticleCount();
		hChild.child->Render(drawCmd,renderer,renderFlags);
	}
	if(numRenderParticles == 0)
	{
		if(umath::is_flag_set(m_flags,Flags::Dying))
			Stop();
		return;
	}

	if(m_bufParticles != nullptr)
	{
		for(auto &r : m_renderers)
			r->Render(drawCmd,renderer,renderFlags);
	}
	umath::set_flag(m_flags,Flags::RendererBufferUpdateRequired,false);
}

void CParticleSystemComponent::RenderShadow(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,pragma::CLightComponent *light,uint32_t layerId)
{
	if(!IsActiveOrPaused() || m_numRenderParticles == 0)
		return;
	for(auto &hChild : m_childSystems)
	{
		if(hChild.child.valid() && hChild.child->IsActiveOrPaused())
			hChild.child->RenderShadow(drawCmd,renderer,light,layerId);
	}
	for(auto &r : m_renderers)
		r->RenderShadow(drawCmd,renderer,*light,layerId);
}

CParticle &CParticleSystemComponent::CreateParticle(uint32_t idx,float timeCreated,float timeAlive)
{
	auto &particle = m_particles[idx];
	if(particle.IsAlive())
		OnParticleDestroyed(particle);
	particle.Reset(timeCreated);
	particle.SetAlive(true);
	particle.SetTimeAlive(timeAlive);
	particle.SetColor(m_initialColor);
	particle.SetLife(m_lifeTime);
	particle.SetRadius(m_radius);
#if 0
	auto pos = m_origin;
	if(umath::is_flag_set(m_flags,Flags::MoveWithEmitter) == false) // If the particle is moving with the emitter, the position is added elsewhere!
	{
		auto pTrComponent = GetEntity().GetTransformComponent();
		if(pTrComponent.valid())
			pos += pTrComponent->GetPosition();
	}
	particle.SetPosition(pos);
	auto rot = m_particleRot;
	if(umath::is_flag_set(m_flags,Flags::RotateWithEmitter) == false)
	{
		auto pTrComponent = GetEntity().GetTransformComponent();
		if(pTrComponent.valid())
			rot = pTrComponent->GetOrientation() *rot;
	}
	particle.SetWorldRotation(rot);
#endif
#if 0
	// TODO
	if(IsAnimated() == true && IsTextureScrollingEnabled() == false)
	{
		if(m_material != nullptr)
		{
			auto &data = m_material->GetDataBlock();
			if(data != nullptr)
			{
				auto &anim = data->GetValue("animation");
				if(anim != nullptr && anim->IsBlock())
				{
					auto block = std::static_pointer_cast<ds::Block>(anim);
					if(block->GetBool("start_random") == true || umath::is_flag_set(m_flags,Flags::RandomStartFrame) == true)
					{
						auto frames = block->GetInt("frames");
						auto fps = block->GetInt("fps");
						auto frame = umath::random(0,frames -1);
						auto offset = 0.f;
						if(fps > 0)
							offset = static_cast<float>(frame) /static_cast<float>(fps);
						else
							offset = static_cast<float>(frame) /static_cast<float>(frames);

						particle.SetFrameOffset(offset);
					}
				}
			}
		}
	}
#endif
	for(auto &init : m_initializers)
		init->OnParticleCreated(particle);
	for(auto &op : m_operators)
		op->OnParticleCreated(particle);
	for(auto &r : m_renderers)
		r->OnParticleCreated(particle);
	particle.PopulateInitialValues();

	for(auto &op : m_operators)
		op->Simulate(particle,0.0); // TODO: Should we use a delta time here?
	return particle;
}

uint32_t CParticleSystemComponent::CreateParticles(uint32_t count,double tSimDelta,float tStart,float tDtPerParticle)
{
	auto bHasLimit = m_currentParticleLimit != std::numeric_limits<uint32_t>::max();
	if(bHasLimit)
		count = umath::min(count,m_currentParticleLimit);
	auto t = tStart;
	for(auto i=decltype(count){0};i<count;++i)
	{
		uint32_t idx;
		if(FindFreeParticle(&idx) == false)
		{
			if(bHasLimit)
				m_currentParticleLimit -= i;
			return i;
		}
		else
		{
			// Simulation time hasn't been updated yet, so we have to add the delta sim time here
			auto ptAge = GetSimulationTime() +tSimDelta -t;
			auto &pt = CreateParticle(idx,t,ptAge);
		}
		t += tDtPerParticle;
	}
	if(bHasLimit)
	{
		m_currentParticleLimit -= count;
		if(m_currentParticleLimit == 0u && IsContinuous() == true)
			m_currentParticleLimit = m_particleLimit;
	}
	return count;
}

void CParticleSystemComponent::OnComplete()
{
	if(m_bRemoveOnComplete == true)
		GetEntity().RemoveSafely();
}

SpriteSheetAnimation *CParticleSystemComponent::GetSpriteSheetAnimation() {return m_spriteSheetAnimationData.get();}
const SpriteSheetAnimation *CParticleSystemComponent::GetSpriteSheetAnimation() const {return const_cast<CParticleSystemComponent*>(this)->GetSpriteSheetAnimation();}

void CParticleSystemComponent::Simulate(double tDelta)
{
	auto *cam = c_game->GetPrimaryCamera();
	if(!IsActiveOrPaused() || cam == nullptr)
		return;
	ScopeGuard sg {[this,tDelta]() {m_simulationTime += tDelta;}}; // Increment simulation time once this tick is complete

	auto pTsComponent = GetEntity().GetTimeScaleComponent();
	if(pTsComponent.valid())
		tDelta *= pTsComponent->GetTimeScale();
	m_tLifeTime += tDelta;

	m_numParticles = 0;
	m_numRenderParticles = 0;
	for(auto i=decltype(m_maxParticlesCur){0};i<m_maxParticlesCur;++i)
	{
		auto &p = m_particles[i];
		auto life = p.GetLife();
		if(life > 0.f)
		{
			life -= static_cast<float>(tDelta);
			p.SetLife(life);
			p.SetTimeAlive(p.GetTimeAlive() +static_cast<float>(tDelta));
			if(life > 0)
				m_numParticles++;
			else
				p.SetCameraDistance(-1);
		}
		if(life <= 0.f && p.IsAlive())
		{
			OnParticleDestroyed(p);
			p.SetAlive(false);
		}
	}

	// Simulate particle operators
	// This has to be before particles are created, otherwise operators like
	// "emission_rate_random" will not work properly!
	for(auto &op : m_operators)
		op->Simulate(tDelta);
	//unsigned int numRender = 0;
	if(umath::is_flag_set(m_flags,Flags::MoveWithEmitter) || umath::is_flag_set(m_flags,Flags::RotateWithEmitter))
	{
		auto pAttComponent = GetEntity().GetComponent<pragma::CAttachableComponent>();
		if(pAttComponent.valid())
			pAttComponent->UpdateAttachmentOffset();
	}

	auto bMoving = (umath::is_flag_set(m_flags,Flags::MoveWithEmitter) && GetEntity().HasStateFlag(BaseEntity::StateFlags::PositionChanged))
		|| (umath::is_flag_set(m_flags,Flags::RotateWithEmitter) && GetEntity().HasStateFlag(BaseEntity::StateFlags::RotationChanged));
	umath::set_flag(m_flags,Flags::HasMovingParticles,bMoving);
	umath::Transform pose;
	GetEntity().GetPose(pose);
	auto &posCam = cam->GetEntity().GetPosition();
	for(auto i=decltype(m_maxParticlesCur){0};i<m_maxParticlesCur;++i)
	{
		auto &p = m_particles[i];
		if(p.GetLife() > 0.f)
		{
			// p.SetPrevPos(p.GetPosition());
			for(auto &op : m_operators)
				op->PreSimulate(p,tDelta);
			for(auto &op : m_operators)
				op->Simulate(p,tDelta);

			auto velAng = p.GetAngularVelocity() *static_cast<float>(tDelta);
			if(uvec::length_sqr(velAng) > 0.f)
			{
				// Update world rotation
				auto rotOld = p.GetWorldRotation();
				auto rotNew = glm::quat_cast(glm::eulerAngleYXZ(velAng.y,velAng.x,velAng.z)) *rotOld;
				p.SetWorldRotation(rotNew);
				if(rotOld.w != rotNew.w || rotOld.x != rotNew.x || rotOld.y != rotNew.y || rotOld.z != rotNew.z)
					umath::set_flag(m_flags,Flags::HasMovingParticles,true);

				// Update sprite rotation
				auto rot = p.GetRotation();
				rot += umath::rad_to_deg(velAng.y);
				p.SetRotation(rot);
			}

			auto pos = p.GetPosition();
			auto &vel = p.GetVelocity();
			if(uvec::length(vel) > 0.f)
			{
				auto velEffective = vel;
				if(umath::is_flag_set(m_flags,Flags::RotateWithEmitter))
					uvec::rotate(&velEffective,pose.GetRotation());
				pos += velEffective *static_cast<float>(tDelta);
				p.SetPosition(pos);
				if(umath::is_flag_set(m_flags,Flags::HasMovingParticles) == false && uvec::length_sqr(velEffective) > 0.f)
					umath::set_flag(m_flags,Flags::HasMovingParticles,true);
			}
			p.SetCameraDistance(glm::length2(pos -posCam));
			for(auto &op : m_operators)
				op->PostSimulate(p,tDelta);
			//numRender++;
		}
	}
	//

	auto numFill = m_maxParticlesCur -m_numParticles;
	auto bEmissionPaused = IsEmissionPaused();
	if(numFill > 0)
	{
		int32_t numCreate = 0;
		auto tEmissionStart = 0.f;
		auto tEmissionRate = 0.f;
		if(m_nextParticleEmissionCount != std::numeric_limits<uint32_t>::max())
		{
			if(bEmissionPaused == false)
			{
				numCreate = m_nextParticleEmissionCount;

				// TODO: Unsure about these
				tEmissionStart = m_simulationTime -tDelta;
				tEmissionRate = static_cast<float>(tDelta) /static_cast<float>(numCreate);
				//

				m_nextParticleEmissionCount = std::numeric_limits<uint32_t>::max();
				if(numCreate > 0u)
					m_tNextEmission -= static_cast<float>(tDelta);
			}
		}
		else
		{
			auto emissionRate = bEmissionPaused ? 0u : m_emissionRate;
			if(emissionRate > 0)
			{
				auto rate = 1.f /static_cast<float>(emissionRate);
				m_tNextEmission -= static_cast<float>(tDelta);
				if(m_tNextEmission <= 0.f)
				{
					tEmissionStart = (m_simulationTime +tDelta) +m_tNextEmission;
					tEmissionRate = rate;

					numCreate = umath::floor(-m_tNextEmission /rate);
					if(numCreate == 0 && m_tLastEmission == 0.0 && rate > 0.f)
						++numCreate; // Make sure at least one particle is created right away when the particle system was started
					m_tNextEmission += numCreate *rate;
				}
			}
		}
		if(numCreate > numFill)
			numCreate = numFill;
		tEmissionStart = umath::clamp(tEmissionStart,0.f,m_simulationTime +static_cast<float>(tDelta));
		numCreate = CreateParticles(numCreate,tDelta,tEmissionStart,tEmissionRate);
		m_numParticles += numCreate;
	}

	auto bChildrenSimulated = false;
	const auto fSimulateChildren = [this,tDelta,&bChildrenSimulated]() {
		if(bChildrenSimulated)
			return;
		bChildrenSimulated = true;
		for(auto &hChild : m_childSystems)
		{
			if(hChild.child.valid() && hChild.child->IsActiveOrPaused())
				hChild.child->Simulate(tDelta);
		}
	};

	if(
		m_numParticles == 0 && IsContinuous() == false && m_tLastEmission != 0.0 && bEmissionPaused == false && 
		(m_currentParticleLimit == 0u || m_currentParticleLimit == std::numeric_limits<uint32_t>::max())
	)
	{
		m_renderBounds = {{},{}};
		auto bChildActive = false;
		for(auto &hChild : m_childSystems)
		{
			if(hChild.child.valid() && hChild.child->IsActiveOrPaused())
			{
				bChildActive = true;
				break;
			}
		}
		if(bChildActive == false)
		{
			// TODO: In some cases particle systems are marked as completed, even though they're not actually complete yet.
			// This can happen if no particles are being emitted for some time. Find a better way to handle this!
			m_state = State::Complete;
			OnComplete();
			return;
		}
		fSimulateChildren();
		if(umath::is_flag_set(m_flags,Flags::AlwaysSimulate) == false)
			return;
	}
	if(umath::is_flag_set(m_flags,Flags::SortParticles))
		SortParticles(); // TODO Sort every frame?
	fSimulateChildren();

	auto constexpr enableDynamicBounds = false;
	auto bStatic = IsStatic();
	auto bUpdateBounds = (enableDynamicBounds && (bStatic == true || m_tLastEmission == 0.0 || m_maxParticlesCur != m_prevMaxParticlesCur)) ? true : false;
	if(bUpdateBounds == true)
	{
		m_renderBounds.first = uvec::MAX;
		m_renderBounds.second = uvec::MIN;
	}
	if(m_maxParticlesCur > 0)
	{
		// Call render callbacks; Last chance to update particle transforms and such
		for(auto it=m_renderCallbacks.begin();it!=m_renderCallbacks.end();)
		{
			auto &hCb = *it;
			if(hCb.IsValid() == false)
			{
				it = m_renderCallbacks.erase(it);
				continue;
			}
			hCb();
			++it;
		}
	}
	auto pTrComponent = GetEntity().GetTransformComponent();
	auto psPos = pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
	auto alphaMode = GetEffectiveAlphaMode();
	for(auto i=decltype(m_maxParticlesCur){0};i<m_maxParticlesCur;++i)
	{
		auto sortedIdx = m_sortedParticleIndices[i];
		auto &p = m_particles[sortedIdx];
		auto radius = p.GetRadius();
		if(p.ShouldDraw() == true)
		{
			auto &data = m_instanceData[m_numRenderParticles];
			auto &pos = p.GetPosition();
			auto &prevPos = p.GetPrevPos();
			auto &vCol = p.GetColor();
			if(umath::is_flag_set(m_flags,Flags::PremultiplyAlpha))
				pragma::premultiply_alpha(vCol,alphaMode);
			auto &col = data.color;
			col = {static_cast<uint16_t>(vCol.x *255.f),static_cast<uint16_t>(vCol.y *255.f),static_cast<uint16_t>(vCol.z *255.f),static_cast<uint16_t>(vCol.a *255.f)};

			auto &rot = p.GetWorldRotation();
			auto origin = p.GetOrigin() *p.GetRadius(); // TODO: What is this for?
			uvec::rotate(&origin,rot);

			data.position = Vector3{pos.x +origin.x,pos.y +origin.y,pos.z +origin.z};
			data.radius = radius;
			data.prevPos = Vector3{prevPos.x +origin.x,prevPos.y +origin.y,prevPos.z +origin.z};
			data.age = p.GetTimeAlive();
			if(umath::is_flag_set(m_flags,Flags::MoveWithEmitter))
			{
				for(auto i=0u;i<3u;++i)
					data.position[i] += psPos[i];
			}
			data.rotation = p.GetRotation();
			data.rotationYaw = umath::float32_to_float16_glm(p.GetRotationYaw());
			data.length = umath::float32_to_float16_glm(p.GetLength());
			m_particleIndicesToBufferIndices[sortedIdx] = m_numRenderParticles;
			m_bufferIndicesToParticleIndices[m_numRenderParticles] = sortedIdx;

			if(bUpdateBounds == true)
			{
				const Vector3 minExtents = {-1.f,-1.f,-1.f};
				const Vector3 maxExtents = {1.f,1.f,1.f};
				auto r = p.GetExtent();
				auto ptPos = Vector3{data.position.x,data.position.y,data.position.z};
				auto minBounds = ptPos +minExtents *r;
				auto maxBounds = ptPos +maxExtents *r;

				if constexpr(enableDynamicBounds)
					uvec::to_min_max(m_renderBounds.first,m_renderBounds.second,minBounds,maxBounds);
			}
			if(m_particleAnimData.empty() == false && m_spriteSheetAnimationData)
			{
				auto seqIdx = p.GetSequence();
				auto &animData = m_particleAnimData.at(m_numRenderParticles);
				if(m_spriteSheetAnimationData && seqIdx < m_spriteSheetAnimationData->sequences.size())
				{
					auto &seq = m_spriteSheetAnimationData->sequences.at(seqIdx);
					auto time = p.GetFrameOffset() *seq.GetDuration();
					uint32_t frameIdx0,frameIdx1;
					float interpFactor;
					if(seq.GetInterpolatedFrameData(time,frameIdx0,frameIdx1,interpFactor))
					{
						animData.frameIndex0 = seq.GetAbsoluteFrameIndex(frameIdx0);
						animData.frameIndex1 = seq.GetAbsoluteFrameIndex(frameIdx1);
						animData.interpFactor = interpFactor;
					}
				}
			}
			++m_numRenderParticles;
		}
	}
	if constexpr(enableDynamicBounds)
	{
		if(m_numRenderParticles == 0)
			m_renderBounds = {{},{}};
		for(auto &r : m_renderers)
		{
			auto rendererBounds = r->GetRenderBounds();
			uvec::to_min_max(m_renderBounds.first,m_renderBounds.second,rendererBounds.first,rendererBounds.second);
		}
	}
	auto &bufParticles = GetParticleBuffer();
	auto bUpdateBuffers = (bStatic == false || m_numRenderParticles != m_numPrevRenderParticles) ? true : false;
	m_numPrevRenderParticles = m_numRenderParticles;
	if(bufParticles != nullptr && bUpdateBuffers == true && m_numRenderParticles > 0u)
	{
		c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(bufParticles,0ull,m_numRenderParticles *sizeof(ParticleData),m_instanceData.data());
		umath::set_flag(m_flags,Flags::RendererBufferUpdateRequired,true);
	}
	if(IsAnimated())
	{
		auto &particleAnimBuffer = GetParticleAnimationBuffer();
		if(particleAnimBuffer != nullptr && m_numRenderParticles > 0u)
			c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(particleAnimBuffer,0ull,m_numRenderParticles *sizeof(ParticleAnimationData),m_particleAnimData.data());
	}
	for(auto &r : m_renderers)
		r->PostSimulate(tDelta);
}

const std::vector<CParticleSystemComponent::ChildData> &CParticleSystemComponent::GetChildren() const {return const_cast<CParticleSystemComponent*>(this)->GetChildren();}
std::vector<CParticleSystemComponent::ChildData> &CParticleSystemComponent::GetChildren() {return m_childSystems;}

std::size_t CParticleSystemComponent::TranslateParticleIndex(std::size_t particleIdx) const
{
	if(particleIdx >= m_particleIndicesToBufferIndices.size())
		return std::numeric_limits<std::size_t>::max();
	return m_particleIndicesToBufferIndices[particleIdx];
}

std::size_t CParticleSystemComponent::TranslateBufferIndex(std::size_t particleIdx) const
{
	if(particleIdx >= m_bufferIndicesToParticleIndices.size())
		return std::numeric_limits<std::size_t>::max();
	return m_bufferIndicesToParticleIndices[particleIdx];
}

void CParticleSystemComponent::SortParticles()
{
	std::sort(m_sortedParticleIndices.begin(),m_sortedParticleIndices.end(),[this](std::size_t idx0,std::size_t idx1) {
		return m_particles[idx0] < m_particles[idx1];
	});
}

void CParticleSystemComponent::OnParticleDestroyed(CParticle &particle)
{
	for(auto &init : m_initializers)
		init->OnParticleDestroyed(particle);
	for(auto &op : m_operators)
		op->OnParticleDestroyed(particle);
	for(auto &r : m_renderers)
		r->OnParticleDestroyed(particle);
}
#pragma optimize("",on)
