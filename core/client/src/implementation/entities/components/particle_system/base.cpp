// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.particle_system;
import :client_state;
import :engine;
import :game;

using namespace pragma;

// 10 MiB
#define PARTICLE_BUFFER_SIZE 10'485'760

decltype(ecs::CParticleSystemComponent::s_particleData) ecs::CParticleSystemComponent::s_particleData;
decltype(ecs::CParticleSystemComponent::s_precached) ecs::CParticleSystemComponent::s_precached;

struct SpriteSheetTextureAnimationFrame {
	Vector2 uvStart;
	Vector2 uvEnd;
};

Color ecs::CParticleSystemComponent::ParticleData::GetColor() const { return Color {static_cast<int16_t>(color.at(0)), static_cast<int16_t>(color.at(1)), static_cast<int16_t>(color.at(2)), static_cast<int16_t>(color.at(3))}; }

ecs::CParticleSystemComponent::Node::Node(CBaseEntity *ent) : hEntity((ent != nullptr) ? ent->GetHandle() : EntityHandle {}), bEntity(true) {}
ecs::CParticleSystemComponent::Node::Node(const Vector3 &pos) : position(pos), bEntity(false) {}

ecs::CParticleSystemComponent *ecs::CParticleSystemComponent::Create(const std::string &fname, CParticleSystemComponent *parent, bool bRecordKeyValues, bool bAutoSpawn)
{
	auto *entChild = get_cgame()->CreateEntity<CEnvParticleSystem>();
	if(entChild == nullptr)
		return nullptr;
	auto pParticleSysComponent = entChild->GetComponent<CParticleSystemComponent>();
	if(pParticleSysComponent.expired() || pParticleSysComponent->SetupParticleSystem(fname, parent, bRecordKeyValues) == false) {
		entChild->Remove();
		return nullptr;
	}
	if(bAutoSpawn)
		entChild->Spawn();
	return pParticleSysComponent.get();
}
ecs::CParticleSystemComponent *ecs::CParticleSystemComponent::Create(const std::unordered_map<std::string, std::string> &values, CParticleSystemComponent *parent, bool bRecordKeyValues, bool bAutoSpawn)
{
	auto *entChild = get_cgame()->CreateEntity<CEnvParticleSystem>();
	if(entChild == nullptr)
		return nullptr;
	auto pParticleSysComponent = entChild->GetComponent<CParticleSystemComponent>();
	if(pParticleSysComponent.expired() || pParticleSysComponent->SetupParticleSystem(values, parent, bRecordKeyValues) == false) {
		entChild->Remove();
		return nullptr;
	}
	if(bAutoSpawn)
		entChild->Spawn();
	return pParticleSysComponent.get();
}
ecs::CParticleSystemComponent *ecs::CParticleSystemComponent::Create(CParticleSystemComponent *parent, bool bAutoSpawn)
{
	auto *entChild = get_cgame()->CreateEntity<CEnvParticleSystem>();
	if(entChild == nullptr)
		return nullptr;
	auto pParticleSysComponent = entChild->GetComponent<CParticleSystemComponent>();
	if(pParticleSysComponent.expired() || pParticleSysComponent->SetupParticleSystem(parent) == false) {
		entChild->Remove();
		return nullptr;
	}
	if(bAutoSpawn)
		entChild->Spawn();
	return pParticleSysComponent.get();
}
Vector3 ecs::CParticleSystemComponent::GetParticlePosition(uint32_t ptIdx) const
{
	auto bufIdx = TranslateBufferIndex(ptIdx);
	auto &ptData = m_instanceData.at(bufIdx);
	auto pos = ptData.position;
	if(ShouldParticlesMoveWithEmitter())
		pos += GetEntity().GetPosition();
	return pos;
}
bool ecs::CParticleSystemComponent::IsStatic() const { return (m_operators.empty() && math::is_flag_set(m_flags, Flags::HasMovingParticles) == false) ? true : false; }
bool ecs::CParticleSystemComponent::IsRendererBufferUpdateRequired() const { return math::is_flag_set(m_flags, Flags::RendererBufferUpdateRequired); }

bool ecs::CParticleSystemComponent::IsParticleFilePrecached(const std::string &fname)
{
	auto fid = fs::get_canonicalized_path(fname);
	string::to_lower(fid);
	ufile::remove_extension_from_filename(fid);
	auto it = std::find(s_precached.begin(), s_precached.end(), fid);
	return (it != s_precached.end()) ? true : false;
}

std::optional<ecs::ParticleSystemFileHeader> ecs::CParticleSystemComponent::ReadHeader(NetworkState &nw, const std::string &fileName)
{
	auto ptPath = pragma::asset::find_file(fileName, asset::Type::ParticleSystem);
	if(ptPath.has_value() == false)
		return {};
	auto fullPtPath = "particles/" + *ptPath;
	auto f = pragma::fs::open_file(fullPtPath.c_str(), fs::FileMode::Read | fs::FileMode::Binary);
	if(f == nullptr)
		return {};
	return ReadHeader(f);
}

std::optional<ecs::ParticleSystemFileHeader> ecs::CParticleSystemComponent::ReadHeader(fs::VFilePtr &f)
{
	std::array<int8_t, 3> header = {static_cast<int8_t>(f->ReadChar()), static_cast<int8_t>(f->ReadChar()), static_cast<int8_t>(f->ReadChar())};
	if(header[0] != 'W' || header[1] != 'P' || header[2] != 'T') {
		f->Seek(0);
		std::shared_ptr<udm::Data> data = nullptr;
		try {
			data = udm::Data::Load(f);
		}
		catch(const udm::Exception &e) {
			data = nullptr;
		}
		if(data) {
			auto assetData = data->GetAssetData().GetData()["particleSystemDefinitions"];
			if(assetData) {
				ParticleSystemFileHeader header {};
				for(auto udmDef : assetData.ElIt())
					header.particleSystemNames.push_back(std::string {udmDef.key});
				header.numParticles = header.particleSystemNames.size();
				return header;
			}
		}
		return {}; // Incorrect format
	}
	ParticleSystemFileHeader fileHeader {};
	fileHeader.version = f->Read<uint32_t>();
	fileHeader.numParticles = f->Read<uint32_t>();
	auto numParticles = fileHeader.numParticles;
	fileHeader.particleSystemNames.resize(numParticles);
	fileHeader.particleSystemOffsets.resize(numParticles);
	for(auto i = decltype(numParticles) {0}; i < numParticles; ++i) {
		fileHeader.particleSystemNames.at(i) = f->ReadString();
		fileHeader.particleSystemOffsets.at(i) = f->Read<uint64_t>();
	}
	return fileHeader;
}

static void to_cache_name(std::string &fname)
{
	fname = fs::get_canonicalized_path(fname);
	pragma::string::to_lower(fname);
	ufile::remove_extension_from_filename(fname, pragma::asset::get_supported_extensions(asset::Type::ParticleSystem));
}
static std::unordered_map<std::string, std::vector<std::string>> s_particleFileToSystems {};
const std::vector<std::string> &ecs::CParticleSystemComponent::GetPrecachedParticleSystemFiles() { return s_precached; }
const std::unordered_map<std::string, std::unique_ptr<asset::ParticleSystemData>> &ecs::CParticleSystemComponent::GetCachedParticleSystemData() { return s_particleData; }
std::optional<std::string> ecs::CParticleSystemComponent::FindParticleSystemFile(const std::string ptName)
{
	for(auto &pair : s_particleFileToSystems) {
		auto it = std::find_if(pair.second.begin(), pair.second.end(), [&ptName](const std::string &ptNameOther) { return string::compare(ptNameOther, ptName, false); });
		if(it == pair.second.end())
			continue;
		return pair.first;
	}
	return {};
}

bool ecs::CParticleSystemComponent::Precache(std::string fname, bool bReload)
{
	to_cache_name(fname);
	auto it = std::find(s_precached.begin(), s_precached.end(), fname);
	util::ScopeGuard sgCache;
	if(it != s_precached.end()) {
		if(bReload == false)
			return true;
	}
	else {
		sgCache = [&fname]() { s_precached.push_back(fname); };
	}

	std::string format;
	auto ptFname = pragma::asset::find_file(fname, asset::Type::ParticleSystem, &format);
	if(!ptFname.has_value())
		return false;
	if(format == asset::FORMAT_PARTICLE_SYSTEM_LEGACY) {
		sgCache.dismiss();
		return PrecacheLegacy(fname, bReload);
	}
	std::string err;
	auto udmData = util::load_udm_asset("particles/" + *ptFname, &err);
	if(udmData == nullptr)
		return false;
	auto &ptSystemNames = s_particleFileToSystems.insert(std::make_pair(fname, std::vector<std::string> {})).first->second;
	ptSystemNames.clear();

	auto &data = *udmData;
	/*if(data.GetAssetType() != pragma::asset::PPTSYS_COLLECTION_IDENTIFIER)
	{
		err = "Incorrect format!";
		return false;
	}

	auto version = data.GetAssetVersion();
	if(version < 1)
	{
		err = "Invalid version!";
		return false;
	}*/
	// if(version > FORMAT_VERSION)
	// 	return false;

	auto udm = *data.GetAssetData();
	auto udmParticleSystemDefinitions = udm["particleSystemDefinitions"];
	auto numParticles = udmParticleSystemDefinitions.GetChildCount();
	ptSystemNames.reserve(numParticles);
	s_particleData.reserve(s_particleData.size() + numParticles);
	uint32_t idx = 0;
	for(auto udmDef : udmParticleSystemDefinitions.ElIt()) {
		auto name = std::string {udmDef.key};
		ptSystemNames.push_back(name);
		auto data = std::make_unique<asset::ParticleSystemData>();
		auto result = LoadFromAssetData(*data, udm::AssetData {udmDef.property}, err);
		if(result == false)
			return false;
		s_particleData[name] = std::move(data);
		++idx;
	}
	return true;
}

bool ecs::CParticleSystemComponent::PrecacheLegacy(std::string fname, bool bReload)
{
	to_cache_name(fname);
	auto it = std::find(s_precached.begin(), s_precached.end(), fname);
	if(it != s_precached.end()) {
		if(bReload == false)
			return true;
	}
	else
		s_precached.push_back(fname);

	auto path = "particles\\" + fname + ".wpt";
	auto f = pragma::fs::open_file(path.c_str(), fs::FileMode::Read | fs::FileMode::Binary);
	if(f == nullptr)
		return false;
	auto header = ReadHeader(f);
	if(header.has_value() == false)
		return false;
	auto &ptSystemNames = s_particleFileToSystems.insert(std::make_pair(fname, std::vector<std::string> {})).first->second;
	ptSystemNames.clear();
	auto numParticles = header->numParticles;
	ptSystemNames.reserve(numParticles);
	for(auto i = decltype(numParticles) {0}; i < numParticles; ++i) {
		auto &name = header->particleSystemNames.at(i);
		ptSystemNames.push_back(name);
		auto offset = header->particleSystemOffsets.at(i);
		if(bReload == true || s_particleData.find(name) == s_particleData.end()) {
			f->Seek(offset);

			auto data = std::make_unique<asset::ParticleSystemData>();
			auto numSettings = f->Read<uint32_t>();
			auto &settings = data->settings;
			for(auto i = decltype(numSettings) {0}; i < numSettings; ++i) {
				auto key = f->ReadString();
				auto val = f->ReadString();
				settings.insert(std::remove_reference<decltype(settings)>::type::value_type(key, val));
				if(key == "material")
					get_client_state()->LoadMaterial(val.c_str());
			}
			std::array<std::vector<asset::ParticleModifierData> *, 3> params = {&data->initializers, &data->operators, &data->renderers};
			for(int32_t i = 0; i < 3; ++i) {
				auto num = f->Read<uint32_t>();
				for(auto j = decltype(num) {0}; j < num; ++j) {
					auto identifier = f->ReadString();
					auto modData = asset::ParticleModifierData {identifier};
					auto numKeyValues = f->Read<uint32_t>();
					for(auto k = decltype(numKeyValues) {0}; k < numKeyValues; ++k) {
						auto key = f->ReadString();
						auto val = f->ReadString();
						modData.settings.insert(decltype(modData.settings)::value_type(key, val));
					}
					params[i]->push_back(modData);
				}
			}
			auto &children = data->children;
			auto numChildren = f->Read<uint8_t>();
			for(auto i = decltype(numChildren) {0}; i < numChildren; ++i) {
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

const std::vector<pts::CParticle> &ecs::CParticleSystemComponent::GetParticles() const { return m_particles; }
pts::CParticle *ecs::CParticleSystemComponent::GetParticle(size_t idx)
{
	if(idx >= GetMaxParticleCount())
		return nullptr;
	return &m_particles[idx];
}

void ecs::CParticleSystemComponent::ClearCache()
{
	for(auto it = s_particleData.begin(); it != s_particleData.end(); ++it)
		it->second = nullptr;
}

bool ecs::CParticleSystemComponent::SetupParticleSystem(std::string fname, CParticleSystemComponent *parent, bool bRecordKeyValues)
{
	bRecordKeyValues = true; // TODO
	if(math::is_flag_set(m_flags, Flags::Setup))
		return true;
	auto it = s_particleData.find(fname);
	if(it == s_particleData.end()) {
		Con::CWAR << "Attempted to create unknown particle system '" << fname << "'!" << Con::endl;
		return false;
	}
	auto &data = it->second;
	auto r = SetupParticleSystem(data->settings, parent, bRecordKeyValues);
	if(r == false)
		return false;

	SetParticleSystemName(fname);

	// Children have to be initialized before operators (in case some operators need to access the children)
	for(auto &child : data->children) {
		// TODO: child.delay is currently ignored, implement it!
		auto *pt = Create(child.childName, this, bRecordKeyValues);
		if(pt != nullptr)
			pt->GetEntity().Spawn();
	}

	for(auto &modData : data->initializers)
		AddInitializer(modData.name, modData.settings);
	for(auto &modData : data->operators)
		AddOperator(modData.name, modData.settings);
	for(auto &modData : data->renderers)
		AddRenderer(modData.name, modData.settings);
	if(data->renderers.empty()) {
		std::unordered_map<std::string, std::string> values {};
		AddRenderer("sprite", values); // Default Renderer
	}
	return r;
}

void ecs::CParticleSystemComponent::SetParticleSystemName(const std::string &name) { m_particleSystemName = name; }
const std::string &ecs::CParticleSystemComponent::GetParticleSystemName() const { return m_particleSystemName; }

bool ecs::CParticleSystemComponent::SetupParticleSystem(const std::unordered_map<std::string, std::string> &values, CParticleSystemComponent *parent, bool bRecordKeyValues)
{
	if(math::is_flag_set(m_flags, Flags::Setup))
		return true;
	math::set_flag(m_flags, Flags::Setup);

	for(auto &kv : values)
		GetEntity().SetKeyValue(kv.first, kv.second);
	if(bRecordKeyValues)
		RecordKeyValues(values);
	m_nodes.resize(m_maxNodes, {nullptr});
	m_tNextEmission = 0.f;
	if(parent != nullptr)
		SetParent(parent);
	// TODO: 'orientation_axis' / 'orientation'?
	return true;
}

bool ecs::CParticleSystemComponent::SetupParticleSystem(CParticleSystemComponent *parent)
{
	std::unordered_map<std::string, std::string> values;
	return SetupParticleSystem(values, parent);
}

///////////////////////////////////////////

decltype(ecs::CParticleSystemComponent::PARTICLE_DATA_SIZE) ecs::CParticleSystemComponent::PARTICLE_DATA_SIZE = sizeof(ParticleData);
decltype(ecs::CParticleSystemComponent::VERTEX_COUNT) ecs::CParticleSystemComponent::VERTEX_COUNT = 6;
static std::shared_ptr<prosper::IDynamicResizableBuffer> s_particleBuffer = nullptr;
static std::shared_ptr<prosper::IDynamicResizableBuffer> s_animStartBuffer = nullptr;
static std::shared_ptr<prosper::IDynamicResizableBuffer> s_animBuffer = nullptr;
const auto PARTICLE_BUFFER_INSTANCE_SIZE = sizeof(ecs::CParticleSystemComponent::ParticleData);
const auto PARTICLE_ANIM_BUFFER_INSTANCE_SIZE = sizeof(Vector2) * 2;
util::EventReply ecs::CParticleSystemComponent::HandleKeyValue(const std::string &key, const std::string &value)
{
#pragma message("TODO: Calculate max particles automatically!")
	if(pragma::string::compare<std::string>(key, "maxparticles", false)) {
		if(m_state != State::Initial)
			Con::CWAR << "Attempted to change max particle count for particle system which has already been started! Ignoring..." << Con::endl;
		else
			m_maxParticles = util::to_int(value);
	}
	else if(pragma::string::compare<std::string>(key, "limit_particle_count"))
		m_particleLimit = util::to_int(value);
	else if(pragma::string::compare<std::string>(key, "emission_rate"))
		m_emissionRate = util::to_int(value);
	else if(pragma::string::compare<std::string>(key, "cast_shadows"))
		SetCastShadows(util::to_boolean(value));
	else if(pragma::string::compare<std::string>(key, "static_scale"))
		m_worldScale = util::to_float(value);
	else if(pragma::string::compare<std::string>(key, "random_start_frame"))
		math::set_flag(m_flags, Flags::RandomStartFrame, util::to_boolean(value));
	else if(pragma::string::compare<std::string>(key, "material"))
		SetMaterial(get_client_state()->LoadMaterial(value));
	else if(pragma::string::compare<std::string>(key, "radius"))
		SetRadius(util::to_float(value));
	else if(pragma::string::compare<std::string>(key, "extent"))
		SetExtent(util::to_float(value));
	else if(pragma::string::compare<std::string>(key, "sort_particles"))
		math::set_flag(m_flags, Flags::SortParticles, util::to_boolean(value));
	else if(pragma::string::compare<std::string>(key, "orientation_type"))
		m_orientationType = static_cast<pts::ParticleOrientationType>(util::to_int(value));
	else if(pragma::string::compare<std::string>(key, "color"))
		m_initialColor = Color(value);
	else if(pragma::string::compare<std::string>(key, "loop"))
		SetContinuous(util::to_boolean(value));
	else if(pragma::string::compare<std::string>(key, "origin"))
		m_origin = uvec::create(value);
	else if(pragma::string::compare<std::string>(key, "bloom_scale")) {
		auto bloomFactor = util::to_float(value);
		SetBloomColorFactor({bloomFactor, bloomFactor, bloomFactor, 1.f});
	}
	else if(pragma::string::compare<std::string>(key, "color_factor"))
		m_colorFactor = uvec::create_v4(value);
	else if(pragma::string::compare<std::string>(key, "bloom_color_factor"))
		SetBloomColorFactor(uvec::create_v4(value));
	else if(pragma::string::compare<std::string>(key, "max_node_count"))
		m_maxNodes = util::to_int(value);
	else if(pragma::string::compare<std::string>(key, "lifetime"))
		m_lifeTime = util::to_float(value);
	else if(pragma::string::compare<std::string>(key, "soft_particles"))
		SetSoftParticles(util::to_boolean(value));
	else if(pragma::string::compare<std::string>(key, "texture_scrolling_enabled"))
		SetTextureScrollingEnabled(util::to_boolean(value));
	else if(pragma::string::compare<std::string>(key, "world_rotation")) {
		std::array<float, 4> values;
		string::string_to_array(value, values.data(), pragma::string::cstring_to_number<float>, values.size());
		m_particleRot.w = values.at(0);
		m_particleRot.x = values.at(1);
		m_particleRot.y = values.at(2);
		m_particleRot.z = values.at(3);
	}
	else if(pragma::string::compare<std::string>(key, "alpha_mode")) {
		auto alphaMode = value;
		pragma::string::to_lower(alphaMode);
		if(alphaMode == "additive_by_color" || alphaMode == "additive_full")
			m_alphaMode = rendering::ParticleAlphaMode::AdditiveByColor;
		else if(alphaMode == "opaque")
			m_alphaMode = rendering::ParticleAlphaMode::Opaque;
		else if(alphaMode == "masked")
			m_alphaMode = rendering::ParticleAlphaMode::Masked;
		else if(alphaMode == "translucent")
			m_alphaMode = rendering::ParticleAlphaMode::Translucent;
		else if(alphaMode == "additive")
			m_alphaMode = rendering::ParticleAlphaMode::Additive;
		else if(alphaMode == "custom")
			m_alphaMode = rendering::ParticleAlphaMode::Custom;
	}
	else if(pragma::string::compare<std::string>(key, "premultiply_alpha"))
		SetAlphaPremultiplied(util::to_boolean(value));
	else if(pragma::string::compare<std::string>(key, "angles")) {
		auto ang = EulerAngles(value);
		m_particleRot = uquat::create(ang);
	}
	else if(pragma::string::compare<std::string>(key, "black_to_alpha"))
		m_alphaMode = rendering::ParticleAlphaMode::AdditiveByColor;
	else if(pragma::string::compare<std::string>(key, "move_with_emitter"))
		math::set_flag(m_flags, Flags::MoveWithEmitter, util::to_boolean(value));
	else if(pragma::string::compare<std::string>(key, "rotate_with_emitter"))
		math::set_flag(m_flags, Flags::RotateWithEmitter, util::to_boolean(value));
	else if(pragma::string::compare<std::string>(key, "transform_with_emitter"))
		math::set_flag(m_flags, Flags::MoveWithEmitter | Flags::RotateWithEmitter, util::to_boolean(value));
	else if(pragma::string::compare<std::string>(key, "auto_simulate"))
		SetAutoSimulate(util::to_boolean(value));
	else if(pragma::string::compare<std::string>(key, "bounding_box_min"))
		m_renderBounds.first = uvec::create(value);
	else if(pragma::string::compare<std::string>(key, "bounding_box_max"))
		m_renderBounds.second = uvec::create(value);
	else
		return util::EventReply::Unhandled;
	return util::EventReply::Handled;
}

ecs::CParticleSystemComponent::~CParticleSystemComponent()
{
	Stop();
	for(auto &childInfo : m_childSystems) {
		if(childInfo.child.expired())
			continue;
		childInfo.child->GetEntity().RemoveSafely();
	}
}

const std::shared_ptr<prosper::IDynamicResizableBuffer> &ecs::CParticleSystemComponent::GetGlobalParticleBuffer() { return s_particleBuffer; }
const std::shared_ptr<prosper::IDynamicResizableBuffer> &ecs::CParticleSystemComponent::GetGlobalAnimationStartBuffer() { return s_animStartBuffer; }
const std::shared_ptr<prosper::IDynamicResizableBuffer> &ecs::CParticleSystemComponent::GetGlobalAnimationBuffer() { return s_animBuffer; }

ecs::CParticleSystemComponent::ControlPoint &ecs::CParticleSystemComponent::InitializeControlPoint(ControlPointIndex idx)
{
	if(idx >= m_controlPoints.size())
		m_controlPoints.resize(idx + 1);
	if(idx >= m_controlPointsPrev.size())
		m_controlPointsPrev.resize(idx + 1);
	return m_controlPoints.at(idx);
}
void ecs::CParticleSystemComponent::SetControlPointEntity(ControlPointIndex idx, CBaseEntity &ent)
{
	auto &cp = InitializeControlPoint(idx);
	cp.hEntity = ent.GetHandle();

	for(auto &childData : GetChildren()) {
		if(childData.child.expired())
			continue;
		childData.child->SetControlPointEntity(idx, ent);
	}
}
void ecs::CParticleSystemComponent::SetControlPointPosition(ControlPointIndex idx, const Vector3 &pos)
{
	auto optPose = GetControlPointPose(idx);
	auto pose = optPose.has_value() ? *optPose : math::Transform {};
	pose.SetOrigin(pos);
	SetControlPointPose(idx, pose);
}
void ecs::CParticleSystemComponent::SetControlPointRotation(ControlPointIndex idx, const Quat &rot)
{
	auto optPose = GetControlPointPose(idx);
	auto pose = optPose.has_value() ? *optPose : math::Transform {};
	pose.SetRotation(rot);
	SetControlPointPose(idx, pose);
}
void ecs::CParticleSystemComponent::SetControlPointPose(ControlPointIndex idx, const math::Transform &pose, float *optTimestamp)
{
	InitializeControlPoint(idx);
	auto t = optTimestamp ? *optTimestamp : m_simulationTime;
	if(t > m_controlPoints.at(idx).simTimestamp)
		m_controlPointsPrev.at(idx) = m_controlPoints.at(idx);
	auto &cp = m_controlPoints.at(idx);
	cp.simTimestamp = t;
	cp.pose = pose;

	for(auto &childData : GetChildren()) {
		if(childData.child.expired())
			continue;
		childData.child->SetControlPointPose(idx, pose, optTimestamp);
	}
}

ecs::CBaseEntity *ecs::CParticleSystemComponent::GetControlPointEntity(ControlPointIndex idx) const
{
	if(idx >= m_controlPoints.size())
		return nullptr;
	return static_cast<CBaseEntity *>(const_cast<BaseEntity *>(m_controlPoints.at(idx).hEntity.get()));
}
std::optional<math::Transform> ecs::CParticleSystemComponent::GetControlPointPose(ControlPointIndex idx, float *optOutTimestamp) const
{
	if(idx >= m_controlPoints.size())
		return {};
	auto pose = m_controlPoints.at(idx).pose;
	auto *ent = GetControlPointEntity(idx);
	if(ent) {
		auto &entPose = ent->GetPose();
		pose = entPose * pose;
	}
	if(optOutTimestamp)
		*optOutTimestamp = m_controlPoints.at(idx).simTimestamp;
	return pose;
}
std::optional<math::Transform> ecs::CParticleSystemComponent::GetPrevControlPointPose(ControlPointIndex idx, float *optOutTimestamp) const
{
	if(idx >= m_controlPointsPrev.size())
		return {};
	auto pose = m_controlPointsPrev.at(idx).pose;
	auto *ent = GetControlPointEntity(idx);
	if(ent) {
		auto &entPose = ent->GetPose();
		pose = entPose * pose;
	}
	if(optOutTimestamp)
		*optOutTimestamp = m_controlPointsPrev.at(idx).simTimestamp;
	return pose;
}
std::optional<math::Transform> ecs::CParticleSystemComponent::GetControlPointPose(ControlPointIndex idx, float t) const
{
	if(idx >= m_controlPoints.size())
		return {};
	auto &cp = m_controlPoints.at(idx);
	if(idx >= m_controlPointsPrev.size() || math::abs(cp.simTimestamp - m_controlPointsPrev.at(idx).simTimestamp) < 0.001f)
		return GetControlPointPose(idx);
	auto &cpPrev = m_controlPointsPrev.at(idx);
	t = (t - cpPrev.simTimestamp) / (cp.simTimestamp - cpPrev.simTimestamp);
	t = math::clamp(t, 0.f, 1.f);
	auto pose = *GetPrevControlPointPose(idx);
	pose.Interpolate(*GetControlPointPose(idx), t);
	return pose;
}

const std::vector<std::unique_ptr<pts::CParticleInitializer, void (*)(pts::CParticleInitializer *)>> &ecs::CParticleSystemComponent::GetInitializers() const { return const_cast<CParticleSystemComponent *>(this)->GetInitializers(); }
const std::vector<std::unique_ptr<pts::CParticleOperator, void (*)(pts::CParticleOperator *)>> &ecs::CParticleSystemComponent::GetOperators() const { return const_cast<CParticleSystemComponent *>(this)->GetOperators(); }
const std::vector<std::unique_ptr<pts::CParticleRenderer, void (*)(pts::CParticleRenderer *)>> &ecs::CParticleSystemComponent::GetRenderers() const { return const_cast<CParticleSystemComponent *>(this)->GetRenderers(); }
std::vector<std::unique_ptr<pts::CParticleInitializer, void (*)(pts::CParticleInitializer *)>> &ecs::CParticleSystemComponent::GetInitializers() { return m_initializers; }
std::vector<std::unique_ptr<pts::CParticleOperator, void (*)(pts::CParticleOperator *)>> &ecs::CParticleSystemComponent::GetOperators() { return m_operators; }
std::vector<std::unique_ptr<pts::CParticleRenderer, void (*)(pts::CParticleRenderer *)>> &ecs::CParticleSystemComponent::GetRenderers() { return m_renderers; }

void ecs::CParticleSystemComponent::SetContinuous(bool b)
{
	BaseEnvParticleSystemComponent::SetContinuous(b);
	if(b == false) {
		if(GetRemoveOnComplete() && m_state == State::Complete)
			GetEntity().RemoveSafely();
	}
}

bool ecs::CParticleSystemComponent::ShouldParticlesRotateWithEmitter() const { return math::is_flag_set(m_flags, Flags::RotateWithEmitter); }
bool ecs::CParticleSystemComponent::ShouldParticlesMoveWithEmitter() const { return math::is_flag_set(m_flags, Flags::MoveWithEmitter); }

void ecs::CParticleSystemComponent::SetAutoSimulate(bool b) { math::set_flag(m_flags, Flags::AutoSimulate, b); }
bool ecs::CParticleSystemComponent::ShouldAutoSimulate() const { return math::is_flag_set(m_flags, Flags::AutoSimulate); }

Vector3 ecs::CParticleSystemComponent::PointToParticleSpace(const Vector3 &p, bool bRotateWithEmitter) const
{
	auto r = p;
	if(bRotateWithEmitter == true) {
		auto pTrComponent = GetEntity().GetTransformComponent();
		if(pTrComponent != nullptr)
			uvec::rotate(&r, pTrComponent->GetRotation());
	}
	if(ShouldParticlesMoveWithEmitter()) {
		auto pTrComponent = GetEntity().GetTransformComponent();
		if(pTrComponent != nullptr)
			r += pTrComponent->GetPosition();
	}
	return r;
}
Vector3 ecs::CParticleSystemComponent::PointToParticleSpace(const Vector3 &p) const { return PointToParticleSpace(p, ShouldParticlesRotateWithEmitter()); }
Vector3 ecs::CParticleSystemComponent::DirectionToParticleSpace(const Vector3 &p, bool bRotateWithEmitter) const
{
	auto r = p;
	if(bRotateWithEmitter == true) {
		auto pTrComponent = GetEntity().GetTransformComponent();
		if(pTrComponent != nullptr)
			uvec::rotate(&r, pTrComponent->GetRotation());
	}
	return r;
}
Vector3 ecs::CParticleSystemComponent::DirectionToParticleSpace(const Vector3 &p) const { return DirectionToParticleSpace(p, ShouldParticlesRotateWithEmitter()); }

void ecs::CParticleSystemComponent::InitializeBuffers()
{
	if(s_particleBuffer == nullptr) {
		auto instanceCount = 32'768ull;
		auto maxInstanceCount = instanceCount * 40u;
		auto instanceSize = PARTICLE_BUFFER_INSTANCE_SIZE;
		prosper::util::BufferCreateInfo createInfo {};
		createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
		createInfo.size = instanceSize * maxInstanceCount;
		createInfo.usageFlags = prosper::BufferUsageFlags::VertexBufferBit | prosper::BufferUsageFlags::TransferDstBit | prosper::BufferUsageFlags::StorageBufferBit;
		s_particleBuffer = get_cengine()->GetRenderContext().CreateDynamicResizableBuffer(createInfo, instanceSize * maxInstanceCount, 0.05f);
		s_particleBuffer->SetDebugName("particle_instance_buf");
	}
	if(s_animStartBuffer == nullptr) {
		auto instanceCount = 524'288ull;
		auto maxInstanceCount = instanceCount * 5u;
		auto instanceSize = PARTICLE_ANIM_BUFFER_INSTANCE_SIZE;
		prosper::util::BufferCreateInfo createInfo {};
		createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
		createInfo.size = instanceSize * maxInstanceCount;
		createInfo.usageFlags = prosper::BufferUsageFlags::VertexBufferBit | prosper::BufferUsageFlags::TransferDstBit;
		s_animStartBuffer = get_cengine()->GetRenderContext().CreateDynamicResizableBuffer(createInfo, instanceSize * maxInstanceCount, 0.01f);
		s_animStartBuffer->SetDebugName("particle_anim_start_buf");
	}
	if(s_animBuffer == nullptr) {
		auto instanceCount = 4'096u;
		auto maxInstanceCount = instanceCount * 5u;
		auto instanceSize = sizeof(ParticleAnimationData);
		prosper::util::BufferCreateInfo createInfo {};
		createInfo.memoryFeatures = prosper::MemoryFeatureFlags::DeviceLocal;
		createInfo.size = instanceSize * maxInstanceCount;
		createInfo.usageFlags = prosper::BufferUsageFlags::StorageBufferBit | prosper::BufferUsageFlags::TransferDstBit;
		s_animBuffer = get_cengine()->GetRenderContext().CreateDynamicResizableBuffer(createInfo, instanceSize * maxInstanceCount, 0.01f);
		s_animBuffer->SetDebugName("particle_anim_data_buf");
	}
}
void ecs::CParticleSystemComponent::ClearBuffers()
{
	s_particleBuffer = nullptr;
	s_animStartBuffer = nullptr;
	s_animBuffer = nullptr;
}
float ecs::CParticleSystemComponent::GetStaticWorldScale() const { return m_worldScale; }
void ecs::CParticleSystemComponent::SetStaticWorldScale(float scale) { m_worldScale = scale; }
void ecs::CParticleSystemComponent::SetSoftParticles(bool bSmooth) { math::set_flag(m_flags, Flags::SoftParticles, bSmooth); }
bool ecs::CParticleSystemComponent::GetSoftParticles() const { return math::is_flag_set(m_flags, Flags::SoftParticles); }
void ecs::CParticleSystemComponent::SetSortParticles(bool sort) { math::set_flag(m_flags, Flags::SortParticles, sort); }
bool ecs::CParticleSystemComponent::GetSortParticles() const { return math::is_flag_set(m_flags, Flags::SortParticles); }
const Color &ecs::CParticleSystemComponent::GetInitialColor() const { return m_initialColor; }
void ecs::CParticleSystemComponent::SetInitialColor(const Color &col) { m_initialColor = col; }

void ecs::CParticleSystemComponent::SetCastShadows(bool b) { math::set_flag(m_flags, Flags::CastShadows, b); }
bool ecs::CParticleSystemComponent::GetCastShadows() const { return math::is_flag_set(m_flags, Flags::CastShadows); }
const Vector4 &ecs::CParticleSystemComponent::GetBloomColorFactor() const { return m_bloomColorFactor; }
void ecs::CParticleSystemComponent::SetBloomColorFactor(const Vector4 &colorFactor) { m_bloomColorFactor = colorFactor; }
std::optional<Vector4> ecs::CParticleSystemComponent::GetEffectiveBloomColorFactor() const
{
	auto colorFactor = GetBloomColorFactor();
	auto *mat = GetMaterial();
	if(mat) {
		Vector4 bloomColorFactor;
		if(mat->GetProperty("bloom_color_factor", &bloomColorFactor))
			colorFactor *= bloomColorFactor;
	}
	if(colorFactor.r == 0.f && colorFactor.g == 0.f && colorFactor.b == 0.f)
		return {};
	return colorFactor;
}
bool ecs::CParticleSystemComponent::IsBloomEnabled() const { return GetEffectiveBloomColorFactor().has_value(); }
const Vector4 &ecs::CParticleSystemComponent::GetColorFactor() const { return m_colorFactor; }
void ecs::CParticleSystemComponent::SetColorFactor(const Vector4 &colorFactor) { m_colorFactor = colorFactor; }
const std::pair<Vector3, Vector3> &ecs::CParticleSystemComponent::GetRenderBounds() const { return m_renderBounds; }
std::pair<Vector3, Vector3> ecs::CParticleSystemComponent::CalcRenderBounds() const
{
	std::pair<Vector3, Vector3> bounds {};
	bounds.first = uvec::max();
	bounds.second = uvec::min();

	uint32_t n = 0;
	for(auto i = decltype(m_numRenderParticles) {0u}; i < m_numRenderParticles; ++i) {
		auto &data = m_instanceData.at(i);
		auto &ptPos = data.position;
		if(data.color.at(3) == 0)
			continue;
		auto radius = data.radius;
		auto ptMin = ptPos - Vector3 {radius, radius, radius};
		auto ptMax = ptPos + Vector3 {radius, radius, radius};
		uvec::min(&bounds.first, ptMin);
		uvec::max(&bounds.second, ptMax);
		++n;
	}
	if(n == 0)
		return {};
	return bounds;
}

pts::ParticleOrientationType ecs::CParticleSystemComponent::GetOrientationType() const { return m_orientationType; }
void ecs::CParticleSystemComponent::SetOrientationType(pts::ParticleOrientationType type) { m_orientationType = type; }

void ecs::CParticleSystemComponent::SetRadius(float r)
{
	m_radius = r;
	m_extent = math::sqrt(math::pow2(r) * 2.f);
}
void ecs::CParticleSystemComponent::SetExtent(float ext)
{
	m_extent = ext;
	m_radius = math::sqrt(math::pow2(ext) / 2.f);
}
float ecs::CParticleSystemComponent::GetRadius() const { return m_radius; }
float ecs::CParticleSystemComponent::GetExtent() const { return m_extent; }

void ecs::CParticleSystemComponent::SetMaterial(material::Material *mat) { m_material = mat ? mat->GetHandle() : material::MaterialHandle {}; }
void ecs::CParticleSystemComponent::SetMaterial(const char *mat) { SetMaterial(get_client_state()->LoadMaterial(mat)); }
material::Material *ecs::CParticleSystemComponent::GetMaterial() const { return const_cast<material::Material *>(m_material.get()); }

pts::CParticleInitializer *ecs::CParticleSystemComponent::AddInitializer(std::string identifier, const std::unordered_map<std::string, std::string> &values)
{
	string::to_lower(identifier);
	auto &map = pts::get_particle_modifier_map();
	auto factory = map.FindInitializer(identifier);
	if(factory == nullptr) {
		Con::CWAR << "Attempted to create unknown particle initializer '" << identifier << "'! Ignoring..." << Con::endl;
		return nullptr;
	}
	auto initializer = factory(*this, values);
	if(initializer == nullptr)
		return nullptr;
	initializer->SetName(identifier);
	if(IsRecordingKeyValues())
		initializer->RecordKeyValues(values);
	m_initializers.push_back(std::move(initializer));
	return m_initializers.back().get();
}
pts::CParticleOperator *ecs::CParticleSystemComponent::AddOperator(std::string identifier, const std::unordered_map<std::string, std::string> &values)
{
	string::to_lower(identifier);
	auto &map = pts::get_particle_modifier_map();
	auto factory = map.FindOperator(identifier);
	if(factory == nullptr) {
		Con::CWAR << "Attempted to create unknown particle operator '" << identifier << "'! Ignoring..." << Con::endl;
		return nullptr;
	}
	auto op = factory(*this, values);
	if(op == nullptr)
		return nullptr;
	op->SetName(identifier);
	if(IsRecordingKeyValues())
		op->RecordKeyValues(values);
	m_operators.push_back(std::move(op));
	return m_operators.back().get();
}
pts::CParticleRenderer *ecs::CParticleSystemComponent::AddRenderer(std::string identifier, const std::unordered_map<std::string, std::string> &values)
{
	string::to_lower(identifier);
	auto &map = pts::get_particle_modifier_map();
	auto factory = map.FindRenderer(identifier);
	if(factory == nullptr) {
		Con::CWAR << "Attempted to create unknown particle renderer '" << identifier << "'! Ignoring..." << Con::endl;
		return nullptr;
	}
	auto op = factory(*this, values);
	if(op == nullptr)
		return nullptr;
	op->SetName(identifier);
	if(IsRecordingKeyValues())
		op->RecordKeyValues(values);
	m_renderers.push_back(std::move(op));
	return m_renderers.back().get();
}

void ecs::CParticleSystemComponent::RemoveInitializer(const std::string &name)
{
	auto it = std::find_if(m_initializers.begin(), m_initializers.end(), [&name](const std::unique_ptr<pts::CParticleInitializer, void (*)(pts::CParticleInitializer *)> &initializer) { return string::compare(initializer->GetName(), name, false); });
	if(it == m_initializers.end())
		return;
	m_initializers.erase(it);
}
void ecs::CParticleSystemComponent::RemoveOperator(const std::string &name)
{
	auto it = std::find_if(m_operators.begin(), m_operators.end(), [&name](const std::unique_ptr<pts::CParticleOperator, void (*)(pts::CParticleOperator *)> &initializer) { return string::compare(initializer->GetName(), name, false); });
	if(it == m_operators.end())
		return;
	m_operators.erase(it);
}
void ecs::CParticleSystemComponent::RemoveRenderer(const std::string &name)
{
	auto it = std::find_if(m_renderers.begin(), m_renderers.end(), [&name](const std::unique_ptr<pts::CParticleRenderer, void (*)(pts::CParticleRenderer *)> &initializer) { return string::compare(initializer->GetName(), name, false); });
	if(it == m_renderers.end())
		return;
	m_renderers.erase(it);
}
void ecs::CParticleSystemComponent::RemoveInitializersByType(const std::string &type)
{
	for(auto it = m_initializers.begin(); it != m_initializers.end();) {
		auto &initializer = *it;
		if(string::compare(initializer->GetType(), type, false) == false) {
			++it;
			continue;
		}
		it = m_initializers.erase(it);
	}
}
void ecs::CParticleSystemComponent::RemoveOperatorsByType(const std::string &type)
{
	for(auto it = m_operators.begin(); it != m_operators.end();) {
		auto &op = *it;
		if(string::compare(op->GetType(), type, false) == false) {
			++it;
			continue;
		}
		it = m_operators.erase(it);
	}
}
void ecs::CParticleSystemComponent::RemoveRenderersByType(const std::string &type)
{
	for(auto it = m_renderers.begin(); it != m_renderers.end();) {
		auto &renderer = *it;
		if(string::compare(renderer->GetType(), type, false) == false) {
			++it;
			continue;
		}
		it = m_renderers.erase(it);
	}
}

const ecs::CParticleSystemComponent *ecs::CParticleSystemComponent::GetParent() const { return const_cast<CParticleSystemComponent *>(this)->GetParent(); }
ecs::CParticleSystemComponent *ecs::CParticleSystemComponent::GetParent()
{
	if(m_hParent.expired())
		return nullptr;
	return m_hParent.get();
}

void ecs::CParticleSystemComponent::SetParent(CParticleSystemComponent *particle)
{
	if(m_hParent.valid()) {
		auto *parent = m_hParent.get();
		if(parent == particle)
			return;
		m_hParent = pragma::util::WeakHandle<CParticleSystemComponent> {};
		if(parent != nullptr)
			parent->RemoveChild(this);
	}
	if(particle == nullptr) {
		m_hParent = pragma::util::WeakHandle<CParticleSystemComponent> {};
		return;
	}
	m_hParent = pragma::util::WeakHandle<CParticleSystemComponent> {std::static_pointer_cast<CParticleSystemComponent>(particle->shared_from_this())};
	particle->AddChild(*this);
	auto pTrComponent = GetEntity().GetTransformComponent();
	auto pTrComponentPt = particle->GetEntity().GetTransformComponent();
	if(pTrComponent != nullptr && pTrComponentPt) {
		pTrComponent->SetPosition(pTrComponentPt->GetPosition());
		pTrComponent->SetRotation(pTrComponentPt->GetRotation());
	}
}

ecs::CParticleSystemComponent *ecs::CParticleSystemComponent::AddChild(const std::string &name)
{
	auto *pt = Create(name, this, IsRecordingKeyValues());
	if(pt == nullptr)
		return nullptr;
	pt->GetEntity().Spawn();
	AddChild(*pt);
	return pt;
}

void ecs::CParticleSystemComponent::AddChild(CParticleSystemComponent &particle, float delay)
{
	if(HasChild(particle))
		return;
	ChildData childData {};
	childData.child = pragma::util::WeakHandle<CParticleSystemComponent> {std::static_pointer_cast<CParticleSystemComponent>(particle.shared_from_this())};
	childData.delay = delay;
	m_childSystems.push_back(childData);
	particle.SetParent(this);
	particle.SetContinuous(IsContinuous());
	uint32_t cpIdx = 0;
	for(auto &cp : m_controlPoints) {
		if(cp.hEntity.valid())
			particle.SetControlPointEntity(cpIdx, static_cast<CBaseEntity &>(*cp.hEntity.get()));
		particle.SetControlPointPose(cpIdx, cp.pose);
		++cpIdx;
	}
}

void ecs::CParticleSystemComponent::RemoveChild(CParticleSystemComponent *particle)
{
	auto it = std::find_if(m_childSystems.begin(), m_childSystems.end(), [particle](const ChildData &childInfo) { return childInfo.child.get() == particle; });
	if(it == m_childSystems.end())
		return;
	auto *child = it->child.get();
	m_childSystems.erase(it);
	if(child)
		child->SetParent(nullptr);
}

bool ecs::CParticleSystemComponent::HasChild(CParticleSystemComponent &particle)
{
	auto it = std::find_if(m_childSystems.begin(), m_childSystems.end(), [&particle](const ChildData &hchild) { return (hchild.child.get() == &particle) ? true : false; });
	return (it != m_childSystems.end()) ? true : false;
}

const std::shared_ptr<prosper::IBuffer> &ecs::CParticleSystemComponent::GetParticleBuffer() const { return m_bufParticles; }
const std::shared_ptr<prosper::IBuffer> &ecs::CParticleSystemComponent::GetParticleAnimationBuffer() const { return m_bufParticleAnimData; }
const std::shared_ptr<prosper::IBuffer> &ecs::CParticleSystemComponent::GetSpriteSheetBuffer() const { return m_bufSpriteSheet; }
prosper::IDescriptorSet *ecs::CParticleSystemComponent::GetAnimationDescriptorSet() { return (m_descSetGroupAnimation != nullptr) ? m_descSetGroupAnimation->GetDescriptorSet() : nullptr; }
const std::shared_ptr<prosper::IDescriptorSetGroup> &ecs::CParticleSystemComponent::GetAnimationDescriptorSetGroup() const { return m_descSetGroupAnimation; }

bool ecs::CParticleSystemComponent::IsAnimated() const { return m_descSetGroupAnimation != nullptr; }

void ecs::CParticleSystemComponent::Start()
{
	if(!math::is_flag_set(m_flags, Flags::MaterialDescriptorSetInitialized)) {
		m_flags |= Flags::MaterialDescriptorSetInitialized;
		// Material descriptor set has to be initialized on main thread, before rendering
		auto &renderers = GetRenderers();
		auto *mat = static_cast<material::CMaterial *>(GetMaterial());
		if(mat && !renderers.empty()) {
			auto *texBase = dynamic_cast<ShaderTexturedBase *>(renderers.front()->GetShader());
			if(texBase) {
				if(!mat->GetDescriptorSetGroup(*dynamic_cast<prosper::Shader *>(texBase)))
					texBase->InitializeMaterialDescriptorSet(*mat, false);
			}
		}
	}

	CreateParticle();
	if(IsActiveOrPaused())
		Stop();
	math::remove_flag(m_flags, Flags::Dying);
	for(auto &pt : m_particles)
		pt.Resurrect();
	m_state = State::Active;
	m_tLastEmission = 0.0;
	m_tLifeTime = 0.0;
	m_tStartTime = get_cgame()->RealTime();
	m_currentParticleLimit = m_particleLimit;

	// Children have to be started before operators are initialized,
	// in case one of the operators needs to access a child
	for(auto &hChild : m_childSystems) {
		if(hChild.child.expired())
			continue;
		hChild.child->Start();
	}
	//

	std::sort(m_initializers.begin(), m_initializers.end(), [](const std::unique_ptr<pts::CParticleInitializer, void (*)(pts::CParticleInitializer *)> &a, const std::unique_ptr<pts::CParticleInitializer, void (*)(pts::CParticleInitializer *)> &b) { return a->GetPriority() > b->GetPriority(); });
	std::sort(m_operators.begin(), m_operators.end(), [](const std::unique_ptr<pts::CParticleOperator, void (*)(pts::CParticleOperator *)> &a, const std::unique_ptr<pts::CParticleOperator, void (*)(pts::CParticleOperator *)> &b) { return a->GetPriority() > b->GetPriority(); });
	std::sort(m_renderers.begin(), m_renderers.end(), [](const std::unique_ptr<pts::CParticleRenderer, void (*)(pts::CParticleRenderer *)> &a, const std::unique_ptr<pts::CParticleRenderer, void (*)(pts::CParticleRenderer *)> &b) { return a->GetPriority() > b->GetPriority(); });

	//
	for(auto &init : m_initializers)
		init->OnParticleSystemStarted();
	for(auto &op : m_operators)
		op->OnParticleSystemStarted(); // Operators have to be initialized before buffers are initialized
	for(auto &r : m_renderers)
		r->OnParticleSystemStarted();
	//

	if(m_maxParticles > 0) {
		m_bufParticleAnimData = nullptr;
		m_descSetGroupAnimation = nullptr;
		if(m_material) {
			if(IsTextureScrollingEnabled()) {
			}
			else if(ShaderParticle2DBase::DESCRIPTOR_SET_ANIMATION.IsValid()) {
				auto *spriteSheetAnim = static_cast<material::CMaterial *>(m_material.get())->GetSpriteSheetAnimation();
				m_spriteSheetAnimationData = spriteSheetAnim ? std::make_unique<material::SpriteSheetAnimation>(*spriteSheetAnim) : nullptr;

				if(!m_spriteSheetAnimationData) {
					if(m_material->HasPropertyBlock("animation")) {
						int32_t offset = 0;
						int32_t numFrames = 0;
						int32_t fps = 0;
						int32_t rows = 0;
						int32_t columns = 0;
						m_material->GetProperty("animation/offset", &offset);
						m_material->GetProperty("animation/frames", &numFrames);
						m_material->GetProperty("animation/fps", &fps);
						m_material->GetProperty("animation/rows", &rows);
						m_material->GetProperty("animation/columns", &columns);

						uint32_t w = 0;
						uint32_t h = 0;
						auto *albedoMap = m_material->GetAlbedoMap();
						if(albedoMap) {
							w = albedoMap->width;
							h = albedoMap->height;
						}

						m_spriteSheetAnimationData = std::make_unique<material::SpriteSheetAnimation>();
						m_spriteSheetAnimationData->sequences.push_back({});
						auto &seq = m_spriteSheetAnimationData->sequences.back();

						auto &frames = seq.frames;
						frames.reserve(numFrames);
						auto widthPerFrame = w / static_cast<float>(columns);
						auto heightPerFrame = h / static_cast<float>(rows);
						for(auto x = decltype(rows) {0u}; x < rows; ++x) {
							for(auto y = decltype(columns) {0u}; y < columns; ++y) {
								frames.push_back({});
								auto &frame = frames.back();
								frame.uvStart = Vector2 {y * widthPerFrame, x * heightPerFrame};
								frame.uvEnd = frame.uvStart + Vector2 {widthPerFrame, heightPerFrame};

								frame.uvStart /= Vector2 {w, h};
								frame.uvEnd /= Vector2 {w, h};

								frame.duration = 0.1f;
							}
						}
						m_spriteSheetAnimationData->UpdateLookupData();
					}
				}

				if(m_spriteSheetAnimationData) {
					uint32_t numFrames = 0;
					for(auto &seq : m_spriteSheetAnimationData->sequences)
						numFrames += seq.frames.size();

					std::vector<SpriteSheetTextureAnimationFrame> frames {};
					frames.resize(numFrames);
					uint32_t frameIndex = 0;
					for(auto &seq : m_spriteSheetAnimationData->sequences) {
						for(auto &frame : seq.frames) {
							auto &frameData = frames.at(frameIndex++);
							frameData.uvStart = frame.uvStart;
							frameData.uvEnd = frame.uvEnd;
						}
					}
					m_bufSpriteSheet = s_animBuffer->AllocateBuffer(frames.size() * sizeof(frames.front()), frames.data());

					m_descSetGroupAnimation = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(ShaderParticle2DBase::DESCRIPTOR_SET_ANIMATION);
					m_descSetGroupAnimation->GetDescriptorSet()->SetBindingUniformBuffer(*m_bufSpriteSheet, 0u);
				}
			}
		}

		m_bufParticles = s_particleBuffer->AllocateBuffer(m_maxParticles * PARTICLE_BUFFER_INSTANCE_SIZE);
		auto bAnimated = IsAnimated();
		if(bAnimated)
			m_bufParticleAnimData = s_animStartBuffer->AllocateBuffer(m_maxParticles * PARTICLE_ANIM_BUFFER_INSTANCE_SIZE);
		else
			m_bufParticleAnimData = nullptr;

		//if(m_maxParticles != m_maxParticlesCur)
		//{
		m_particles.resize(m_maxParticles);
		m_sortedParticleIndices.resize(m_particles.size());

		m_particleIndicesToBufferIndices.resize(m_particles.size());
		std::fill(m_particleIndicesToBufferIndices.begin(), m_particleIndicesToBufferIndices.end(), 0);
		m_bufferIndicesToParticleIndices.resize(m_particles.size());
		std::fill(m_bufferIndicesToParticleIndices.begin(), m_bufferIndicesToParticleIndices.end(), 0);

		for(auto i = decltype(m_maxParticles) {0}; i < m_maxParticles; ++i) {
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

void ecs::CParticleSystemComponent::Stop()
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
	for(auto &hChild : m_childSystems) {
		if(hChild.child.expired())
			continue;
		hChild.child->Stop();
	}
	m_tLifeTime = 0.0;
	OnComplete();
}

double ecs::CParticleSystemComponent::GetLifeTime() const { return m_tLifeTime; }
float ecs::CParticleSystemComponent::GetSimulationTime() const { return m_simulationTime; }

double ecs::CParticleSystemComponent::GetStartTime() const { return m_tStartTime; }

const std::vector<ecs::CParticleSystemComponent::ParticleData> &ecs::CParticleSystemComponent::GetRenderParticleData() const { return m_instanceData; }
const std::vector<ecs::CParticleSystemComponent::ParticleAnimationData> &ecs::CParticleSystemComponent::GetParticleAnimationData() const { return m_particleAnimData; }

bool ecs::CParticleSystemComponent::IsActive() const { return m_state == State::Active; }
bool ecs::CParticleSystemComponent::IsEmissionPaused() const { return m_state == State::Paused; }
bool ecs::CParticleSystemComponent::IsActiveOrPaused() const { return IsActive() || IsEmissionPaused(); }

uint32_t ecs::CParticleSystemComponent::GetParticleCount() const { return m_numParticles; }
uint32_t ecs::CParticleSystemComponent::GetRenderParticleCount() const { return m_numRenderParticles; }
uint32_t ecs::CParticleSystemComponent::GetMaxParticleCount() const { return m_maxParticles; }
void ecs::CParticleSystemComponent::SetMaxParticleCount(uint32_t count) { m_maxParticles = count; }

void ecs::CParticleSystemComponent::OnRemove()
{
	BaseEnvParticleSystemComponent::OnRemove();
	for(auto &hChild : m_childSystems) {
		if(hChild.child.expired())
			continue;
		hChild.child->GetEntity().RemoveSafely();
	}
}

bool ecs::CParticleSystemComponent::IsDying() const { return math::is_flag_set(m_flags, Flags::Dying); }

void ecs::CParticleSystemComponent::Die(float maxRemainingLifetime)
{
	math::add_flag(m_flags, Flags::Dying);
	for(auto &pt : m_particles) {
		pt.Die();
		if(pt.GetLife() > maxRemainingLifetime)
			pt.SetLife(maxRemainingLifetime);
	}
	for(auto &hChild : m_childSystems) {
		if(hChild.child.expired())
			hChild.child->Die();
	}
}

bool ecs::CParticleSystemComponent::FindFreeParticle(uint32_t *idx)
{
	if(math::is_flag_set(m_flags, Flags::Dying))
		return false;
	for(auto i = m_idxLast; i < m_maxParticlesCur; ++i) {
		auto &p = m_particles[i];
		if(p.GetLife() <= 0) {
			m_idxLast = i + 1;
			*idx = i;
			return true;
		}
	}
	if(IsContinuous() == false)
		return false;
	for(auto i = decltype(m_idxLast) {0}; i < m_idxLast; ++i) {
		auto &p = m_particles[i];
		if(p.GetLife() <= 0) {
			m_idxLast = i;
			*idx = i;
			return true;
		}
	}
	*idx = 0;
	return true;
}

void ecs::CParticleSystemComponent::SetNodeTarget(uint32_t node, CBaseEntity *ent)
{
	if(node == 0)
		return;
	--node;
	if(node >= m_nodes.size())
		return;
	m_nodes[node].hEntity = ent->GetHandle();
	m_nodes[node].bEntity = true;
}
void ecs::CParticleSystemComponent::SetNodeTarget(uint32_t node, const Vector3 &pos)
{
	if(node == 0)
		return;
	--node;
	if(node >= m_nodes.size())
		return;
	m_nodes[node].position = pos;
	m_nodes[node].bEntity = false;
}
uint32_t ecs::CParticleSystemComponent::GetNodeCount() const { return m_nodes.size() + 1; }
Vector3 ecs::CParticleSystemComponent::GetNodePosition(uint32_t node) const
{
	if(node == 0) {
		auto pTrComponent = GetEntity().GetTransformComponent();
		return pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3 {};
	}
	--node;
	if(node >= m_nodes.size() || (m_nodes[node].bEntity == true && !m_nodes[node].hEntity.valid()))
		return {0.f, 0.f, 0.f};
	if(m_nodes[node].bEntity == false)
		return m_nodes[node].position;
	auto pTrComponent = m_nodes[node].hEntity.get()->GetTransformComponent();
	return pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3 {};
}
ecs::CBaseEntity *ecs::CParticleSystemComponent::GetNodeTarget(uint32_t node) const
{
	if(node == 0)
		return nullptr;
	--node;
	if(node >= m_nodes.size())
		return nullptr;
	return static_cast<CBaseEntity *>(const_cast<BaseEntity *>(m_nodes[node].hEntity.get()));
}

CallbackHandle ecs::CParticleSystemComponent::AddRenderCallback(const std::function<void(void)> &cb)
{
	auto hCb = FunctionCallback<void>::Create(cb);
	AddRenderCallback(hCb);
	return hCb;
}
void ecs::CParticleSystemComponent::AddRenderCallback(const CallbackHandle &hCb) { m_renderCallbacks.push_back(hCb); }
rendering::ParticleAlphaMode ecs::CParticleSystemComponent::GetAlphaMode() const { return m_alphaMode; }
rendering::ParticleAlphaMode ecs::CParticleSystemComponent::GetEffectiveAlphaMode() const
{
	auto alphaMode = GetAlphaMode();
	if(alphaMode != rendering::ParticleAlphaMode::Additive)
		return alphaMode;
	auto *mat = GetMaterial();
	if(mat) {
		if(mat->GetProperty("additive", false))
			alphaMode = rendering::ParticleAlphaMode::AdditiveByColor;
	}
	return alphaMode;
}
void ecs::CParticleSystemComponent::SetAlphaMode(rendering::ParticleAlphaMode alphaMode) { m_alphaMode = alphaMode; }
void ecs::CParticleSystemComponent::SetTextureScrollingEnabled(bool b) { math::set_flag(m_flags, Flags::TextureScrollingEnabled, b); }
bool ecs::CParticleSystemComponent::IsTextureScrollingEnabled() const { return math::is_flag_set(m_flags, Flags::TextureScrollingEnabled); }

bool ecs::CParticleSystemComponent::IsAlphaPremultiplied() const { return math::is_flag_set(m_flags, Flags::PremultiplyAlpha); }
void ecs::CParticleSystemComponent::SetAlphaPremultiplied(bool b) { math::set_flag(m_flags, Flags::PremultiplyAlpha, b); }
uint32_t ecs::CParticleSystemComponent::GetEmissionRate() const { return m_emissionRate; }
void ecs::CParticleSystemComponent::SetEmissionRate(uint32_t emissionRate) { m_emissionRate = emissionRate; }
void ecs::CParticleSystemComponent::SetNextParticleEmissionCount(uint32_t count) { m_nextParticleEmissionCount = count; }
void ecs::CParticleSystemComponent::PauseEmission() { m_state = State::Paused; }
void ecs::CParticleSystemComponent::ResumeEmission()
{
	if(m_state != State::Paused)
		return;
	m_state = State::Active;
}
void ecs::CParticleSystemComponent::SetAlwaysSimulate(bool b) { math::set_flag(m_flags, Flags::AlwaysSimulate, b); }

void ecs::CParticleSystemComponent::RecordRender(prosper::ICommandBuffer &drawCmd, CSceneComponent &scene, const CRasterizationRendererComponent &renderer, pts::ParticleRenderFlags renderFlags)
{
	if(math::is_flag_set(renderFlags, pts::ParticleRenderFlags::Bloom) && IsBloomEnabled() == false)
		return;
	m_tLastEmission = get_cgame()->RealTime();
	if(IsActiveOrPaused() == false) {
		if(math::is_flag_set(m_flags, Flags::Dying))
			Stop();
		return;
	}
	auto numRenderParticles = GetRenderParticleCount();
	for(auto &hChild : m_childSystems) {
		if(hChild.child.expired() || hChild.child->IsActiveOrPaused() == false)
			continue;
		numRenderParticles += hChild.child->GetRenderParticleCount();
		hChild.child->RecordRender(drawCmd, scene, renderer, renderFlags);
	}
	if(numRenderParticles == 0) {
		if(math::is_flag_set(m_flags, Flags::Dying))
			Stop();
		return;
	}

	if(m_bufParticles != nullptr) {
		for(auto &r : m_renderers)
			r->RecordRender(drawCmd, scene, renderer, renderFlags);
	}
	math::set_flag(m_flags, Flags::RendererBufferUpdateRequired, false);
}

void ecs::CParticleSystemComponent::RecordRenderShadow(prosper::ICommandBuffer &drawCmd, CSceneComponent &scene, const CRasterizationRendererComponent &renderer, CLightComponent *light, uint32_t layerId)
{
	if(!IsActiveOrPaused() || m_numRenderParticles == 0)
		return;
	for(auto &hChild : m_childSystems) {
		if(hChild.child.valid() && hChild.child->IsActiveOrPaused())
			hChild.child->RecordRenderShadow(drawCmd, scene, renderer, light, layerId);
	}
	for(auto &r : m_renderers)
		r->RecordRenderShadow(drawCmd, scene, renderer, *light, layerId);
}

pts::CParticle &ecs::CParticleSystemComponent::CreateParticle(uint32_t idx, float timeCreated, float timeAlive)
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
	if(pragma::math::is_flag_set(m_flags,Flags::MoveWithEmitter) == false) // If the particle is moving with the emitter, the position is added elsewhere!
	{
		auto pTrComponent = GetEntity().GetTransformComponent();
		if(pTrComponent != nullptr)
			pos += pTrComponent->GetPosition();
	}
	particle.SetPosition(pos);
	auto rot = m_particleRot;
	if(pragma::math::is_flag_set(m_flags,Flags::RotateWithEmitter) == false)
	{
		auto pTrComponent = GetEntity().GetTransformComponent();
		if(pTrComponent != nullptr)
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
					auto block = std::static_pointer_cast<datasystem::Block>(anim);
					if(block->GetBool("start_random") == true || pragma::math::is_flag_set(m_flags,Flags::RandomStartFrame) == true)
					{
						auto frames = block->GetInt("frames");
						auto fps = block->GetInt("fps");
						auto frame = pragma::math::random(0,frames -1);
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
		op->Simulate(particle, 0.0); // TODO: Should we use a delta time here?
	return particle;
}

uint32_t ecs::CParticleSystemComponent::CreateParticles(uint32_t count, double tSimDelta, float tStart, float tDtPerParticle)
{
	auto bHasLimit = m_currentParticleLimit != std::numeric_limits<uint32_t>::max();
	if(bHasLimit)
		count = math::min(count, m_currentParticleLimit);
	auto t = tStart;
	for(auto i = decltype(count) {0}; i < count; ++i) {
		uint32_t idx;
		if(FindFreeParticle(&idx) == false) {
			if(bHasLimit)
				m_currentParticleLimit -= i;
			return i;
		}
		else {
			// Simulation time hasn't been updated yet, so we have to add the delta sim time here
			auto ptAge = GetSimulationTime() + tSimDelta - t;
			auto &pt = CreateParticle(idx, t, ptAge);
		}
		t += tDtPerParticle;
	}
	if(bHasLimit) {
		m_currentParticleLimit -= count;
		if(m_currentParticleLimit == 0u && IsContinuous() == true)
			m_currentParticleLimit = m_particleLimit;
	}
	return count;
}

void ecs::CParticleSystemComponent::OnComplete()
{
	if(m_bRemoveOnComplete == true)
		GetEntity().RemoveSafely();
}

material::SpriteSheetAnimation *ecs::CParticleSystemComponent::GetSpriteSheetAnimation() { return m_spriteSheetAnimationData.get(); }
const material::SpriteSheetAnimation *ecs::CParticleSystemComponent::GetSpriteSheetAnimation() const { return const_cast<CParticleSystemComponent *>(this)->GetSpriteSheetAnimation(); }

void ecs::CParticleSystemComponent::Simulate(double tDelta)
{
	auto *cam = get_cgame()->GetPrimaryCamera<CCameraComponent>();
	if(!IsActiveOrPaused() || cam == nullptr)
		return;
	util::ScopeGuard sg {[this, tDelta]() { m_simulationTime += tDelta; }}; // Increment simulation time once this tick is complete

	auto pTsComponent = GetEntity().GetTimeScaleComponent();
	if(pTsComponent.valid())
		tDelta *= pTsComponent->GetTimeScale();
	m_tLifeTime += tDelta;

	m_numParticles = 0;
	m_numRenderParticles = 0;
	for(auto i = decltype(m_maxParticlesCur) {0}; i < m_maxParticlesCur; ++i) {
		auto &p = m_particles[i];
		auto life = p.GetLife();
		if(life > 0.f) {
			life -= static_cast<float>(tDelta);
			p.SetLife(life);
			p.SetTimeAlive(p.GetTimeAlive() + static_cast<float>(tDelta));
			if(life > 0)
				m_numParticles++;
			else
				p.SetCameraDistance(-1);
		}
		if(life <= 0.f && p.IsAlive()) {
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
	if(math::is_flag_set(m_flags, Flags::MoveWithEmitter) || math::is_flag_set(m_flags, Flags::RotateWithEmitter)) {
		auto pAttComponent = GetEntity().GetComponent<CAttachmentComponent>();
		if(pAttComponent.valid())
			pAttComponent->UpdateAttachmentOffset();
	}

	auto bMoving
	  = (math::is_flag_set(m_flags, Flags::MoveWithEmitter) && GetEntity().HasStateFlag(BaseEntity::StateFlags::PositionChanged)) || (math::is_flag_set(m_flags, Flags::RotateWithEmitter) && GetEntity().HasStateFlag(BaseEntity::StateFlags::RotationChanged));
	math::set_flag(m_flags, Flags::HasMovingParticles, bMoving);
	auto &pose = GetEntity().GetPose();
	auto &posCam = cam->GetEntity().GetPosition();
	for(auto i = decltype(m_maxParticlesCur) {0}; i < m_maxParticlesCur; ++i) {
		auto &p = m_particles[i];
		if(p.GetLife() > 0.f) {
			// p.SetPrevPos(p.GetPosition());
			for(auto &op : m_operators)
				op->PreSimulate(p, tDelta);
			for(auto &op : m_operators)
				op->Simulate(p, tDelta);

			auto velAng = p.GetAngularVelocity() * static_cast<float>(tDelta);
			if(uvec::length_sqr(velAng) > 0.f) {
				// Update world rotation
				auto rotOld = p.GetWorldRotation();
				auto rotNew = glm::gtc::quat_cast(glm::gtx::eulerAngleYXZ(velAng.y, velAng.x, velAng.z)) * rotOld;
				p.SetWorldRotation(rotNew);
				if(rotOld.w != rotNew.w || rotOld.x != rotNew.x || rotOld.y != rotNew.y || rotOld.z != rotNew.z)
					math::set_flag(m_flags, Flags::HasMovingParticles, true);

				// Update sprite rotation
				auto rot = p.GetRotation();
				rot += math::rad_to_deg(velAng.y);
				p.SetRotation(rot);
			}

			auto pos = p.GetPosition();
			auto &vel = p.GetVelocity();
			if(uvec::length(vel) > 0.f) {
				auto velEffective = vel;
				if(math::is_flag_set(m_flags, Flags::RotateWithEmitter))
					uvec::rotate(&velEffective, pose.GetRotation());
				pos += velEffective * static_cast<float>(tDelta);
				p.SetPosition(pos);
				if(math::is_flag_set(m_flags, Flags::HasMovingParticles) == false && uvec::length_sqr(velEffective) > 0.f)
					math::set_flag(m_flags, Flags::HasMovingParticles, true);
			}
			p.SetCameraDistance(glm::gtx::length2(pos - posCam));
			for(auto &op : m_operators)
				op->PostSimulate(p, tDelta);
			//numRender++;
		}
	}
	//

	auto numFill = m_maxParticlesCur - m_numParticles;
	auto bEmissionPaused = IsEmissionPaused();
	if(numFill > 0) {
		int32_t numCreate = 0;
		auto tEmissionStart = 0.f;
		auto tEmissionRate = 0.f;
		if(m_nextParticleEmissionCount != std::numeric_limits<uint32_t>::max()) {
			if(bEmissionPaused == false) {
				numCreate = m_nextParticleEmissionCount;

				// TODO: Unsure about these
				tEmissionStart = m_simulationTime - tDelta;
				tEmissionRate = static_cast<float>(tDelta) / static_cast<float>(numCreate);
				//

				m_nextParticleEmissionCount = std::numeric_limits<uint32_t>::max();
				if(numCreate > 0u)
					m_tNextEmission -= static_cast<float>(tDelta);
			}
		}
		else {
			auto emissionRate = bEmissionPaused ? 0u : m_emissionRate;
			if(emissionRate > 0) {
				auto rate = 1.f / static_cast<float>(emissionRate);
				m_tNextEmission -= static_cast<float>(tDelta);
				if(m_tNextEmission <= 0.f) {
					tEmissionStart = (m_simulationTime + tDelta) + m_tNextEmission;
					tEmissionRate = rate;

					numCreate = math::floor(-m_tNextEmission / rate);
					if(numCreate == 0 && m_tLastEmission == 0.0 && rate > 0.f)
						++numCreate; // Make sure at least one particle is created right away when the particle system was started
					m_tNextEmission += numCreate * rate;
				}
			}
		}
		if(numCreate > numFill)
			numCreate = numFill;
		tEmissionStart = math::clamp(tEmissionStart, 0.f, m_simulationTime + static_cast<float>(tDelta));
		numCreate = CreateParticles(numCreate, tDelta, tEmissionStart, tEmissionRate);
		m_numParticles += numCreate;
	}

	auto bChildrenSimulated = false;
	const auto fSimulateChildren = [this, tDelta, &bChildrenSimulated]() {
		if(bChildrenSimulated)
			return;
		bChildrenSimulated = true;
		for(auto &hChild : m_childSystems) {
			if(hChild.child.valid() && hChild.child->IsActiveOrPaused())
				hChild.child->Simulate(tDelta);
		}
	};

	if(m_numParticles == 0 && IsContinuous() == false && m_tLastEmission != 0.0 && bEmissionPaused == false && (m_currentParticleLimit == 0u || m_currentParticleLimit == std::numeric_limits<uint32_t>::max())) {
		m_renderBounds = {{}, {}};
		auto bChildActive = false;
		for(auto &hChild : m_childSystems) {
			if(hChild.child.valid() && hChild.child->IsActiveOrPaused()) {
				bChildActive = true;
				break;
			}
		}
		if(bChildActive == false) {
			// TODO: In some cases particle systems are marked as completed, even though they're not actually complete yet.
			// This can happen if no particles are being emitted for some time. Find a better way to handle this!
			m_state = State::Complete;
			OnComplete();
			return;
		}
		fSimulateChildren();
		if(math::is_flag_set(m_flags, Flags::AlwaysSimulate) == false)
			return;
	}
	if(math::is_flag_set(m_flags, Flags::SortParticles))
		SortParticles(); // TODO Sort every frame?
	fSimulateChildren();

	auto constexpr enableDynamicBounds = false;
	auto bStatic = IsStatic();
	auto bUpdateBounds = (enableDynamicBounds && (bStatic == true || m_tLastEmission == 0.0 || m_maxParticlesCur != m_prevMaxParticlesCur)) ? true : false;
	if(bUpdateBounds == true) {
		m_renderBounds.first = uvec::PRM_MAX;
		m_renderBounds.second = uvec::PRM_MIN;
	}
	if(m_maxParticlesCur > 0) {
		// Call render callbacks; Last chance to update particle transforms and such
		for(auto it = m_renderCallbacks.begin(); it != m_renderCallbacks.end();) {
			auto &hCb = *it;
			if(hCb.IsValid() == false) {
				it = m_renderCallbacks.erase(it);
				continue;
			}
			hCb();
			++it;
		}
	}
	auto pTrComponent = GetEntity().GetTransformComponent();
	auto psPos = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3 {};
	auto alphaMode = GetEffectiveAlphaMode();
	for(auto i = decltype(m_maxParticlesCur) {0}; i < m_maxParticlesCur; ++i) {
		auto sortedIdx = m_sortedParticleIndices[i];
		auto &p = m_particles[sortedIdx];
		auto radius = p.GetRadius();
		if(p.ShouldDraw() == true) {
			auto &data = m_instanceData[m_numRenderParticles];
			auto &pos = p.GetPosition();
			auto &prevPos = p.GetPrevPos();
			auto &vCol = p.GetColor();
			if(math::is_flag_set(m_flags, Flags::PremultiplyAlpha))
				pragma::rendering::premultiply_alpha(vCol, alphaMode);
			auto &col = data.color;
			col = {static_cast<uint16_t>(vCol.x * 255.f), static_cast<uint16_t>(vCol.y * 255.f), static_cast<uint16_t>(vCol.z * 255.f), static_cast<uint16_t>(vCol.a * 255.f)};

			auto &rot = p.GetWorldRotation();
			auto origin = p.GetOrigin() * p.GetRadius(); // TODO: What is this for?
			uvec::rotate(&origin, rot);

			data.position = Vector3 {pos.x + origin.x, pos.y + origin.y, pos.z + origin.z};
			data.radius = radius;
			data.prevPos = Vector3 {prevPos.x + origin.x, prevPos.y + origin.y, prevPos.z + origin.z};
			data.age = p.GetTimeAlive();
			//if(pragma::math::is_flag_set(m_flags,Flags::MoveWithEmitter))
			{
				for(auto i = 0u; i < 3u; ++i)
					data.position[i] += psPos[i];
			}
			data.rotation = p.GetRotation();
			data.rotationYaw = math::float32_to_float16_glm(p.GetRotationYaw());
			data.length = math::float32_to_float16_glm(p.GetLength());
			m_particleIndicesToBufferIndices[sortedIdx] = m_numRenderParticles;
			m_bufferIndicesToParticleIndices[m_numRenderParticles] = sortedIdx;

			if(bUpdateBounds == true) {
				const Vector3 minExtents = {-1.f, -1.f, -1.f};
				const Vector3 maxExtents = {1.f, 1.f, 1.f};
				auto r = p.GetExtent();
				auto ptPos = Vector3 {data.position.x, data.position.y, data.position.z};
				auto minBounds = ptPos + minExtents * r;
				auto maxBounds = ptPos + maxExtents * r;

				if constexpr(enableDynamicBounds)
					uvec::to_min_max(m_renderBounds.first, m_renderBounds.second, minBounds, maxBounds);
			}
			if(m_particleAnimData.empty() == false && m_spriteSheetAnimationData) {
				auto seqIdx = p.GetSequence();
				auto &animData = m_particleAnimData.at(m_numRenderParticles);
				if(m_spriteSheetAnimationData && seqIdx < m_spriteSheetAnimationData->sequences.size()) {
					auto &seq = m_spriteSheetAnimationData->sequences.at(seqIdx);
					auto time = p.GetFrameOffset() * seq.GetDuration();
					uint32_t frameIdx0, frameIdx1;
					float interpFactor;
					if(seq.GetInterpolatedFrameData(time, frameIdx0, frameIdx1, interpFactor)) {
						animData.frameIndex0 = seq.GetAbsoluteFrameIndex(frameIdx0);
						animData.frameIndex1 = seq.GetAbsoluteFrameIndex(frameIdx1);
						animData.interpFactor = interpFactor;
					}
				}
			}
			++m_numRenderParticles;
		}
	}
	if constexpr(enableDynamicBounds) {
		if(m_numRenderParticles == 0)
			m_renderBounds = {{}, {}};
		for(auto &r : m_renderers) {
			auto rendererBounds = r->GetRenderBounds();
			uvec::to_min_max(m_renderBounds.first, m_renderBounds.second, rendererBounds.first, rendererBounds.second);
		}
	}
	auto &bufParticles = GetParticleBuffer();
	auto bUpdateBuffers = (bStatic == false || m_numRenderParticles != m_numPrevRenderParticles) ? true : false;
	m_numPrevRenderParticles = m_numRenderParticles;
	if(bufParticles != nullptr && bUpdateBuffers == true && m_numRenderParticles > 0u) {
		get_cengine()->GetRenderContext().ScheduleRecordUpdateBuffer(bufParticles, 0ull, m_numRenderParticles * sizeof(ParticleData), m_instanceData.data());
		math::set_flag(m_flags, Flags::RendererBufferUpdateRequired, true);
	}
	if(IsAnimated()) {
		auto &particleAnimBuffer = GetParticleAnimationBuffer();
		if(particleAnimBuffer != nullptr && m_numRenderParticles > 0u)
			get_cengine()->GetRenderContext().ScheduleRecordUpdateBuffer(particleAnimBuffer, 0ull, m_numRenderParticles * sizeof(ParticleAnimationData), m_particleAnimData.data());
	}
	for(auto &r : m_renderers)
		r->PostSimulate(tDelta);
}

const std::vector<ecs::CParticleSystemComponent::ChildData> &ecs::CParticleSystemComponent::GetChildren() const { return const_cast<CParticleSystemComponent *>(this)->GetChildren(); }
std::vector<ecs::CParticleSystemComponent::ChildData> &ecs::CParticleSystemComponent::GetChildren() { return m_childSystems; }

std::size_t ecs::CParticleSystemComponent::TranslateParticleIndex(std::size_t particleIdx) const
{
	if(particleIdx >= m_particleIndicesToBufferIndices.size())
		return std::numeric_limits<std::size_t>::max();
	return m_particleIndicesToBufferIndices[particleIdx];
}

std::size_t ecs::CParticleSystemComponent::TranslateBufferIndex(std::size_t particleIdx) const
{
	if(particleIdx >= m_bufferIndicesToParticleIndices.size())
		return std::numeric_limits<std::size_t>::max();
	return m_bufferIndicesToParticleIndices[particleIdx];
}

void ecs::CParticleSystemComponent::SortParticles()
{
	std::sort(m_sortedParticleIndices.begin(), m_sortedParticleIndices.end(), [this](std::size_t idx0, std::size_t idx1) { return m_particles[idx0] < m_particles[idx1]; });
}

void ecs::CParticleSystemComponent::OnParticleDestroyed(pts::CParticle &particle)
{
	for(auto &init : m_initializers)
		init->OnParticleDestroyed(particle);
	for(auto &op : m_operators)
		op->OnParticleDestroyed(particle);
	for(auto &r : m_renderers)
		r->OnParticleDestroyed(particle);
}
