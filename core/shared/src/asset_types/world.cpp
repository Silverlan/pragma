/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/asset_types/world.hpp"
#include "pragma/level/level_info.hpp"
#include "pragma/model/model.h"
#include <util_image_buffer.hpp>

extern DLLNETWORK Engine *engine;

pragma::asset::WorldData::WorldData(NetworkState &nw) : m_nw {nw}
{
	SetMessageLogger(nullptr); // Don't output anything by default
}

pragma::asset::EntityData *pragma::asset::WorldData::FindWorld()
{
	auto it = std::find_if(m_entities.begin(), m_entities.end(), [](const std::shared_ptr<EntityData> &entData) { return entData->IsWorld(); });
	return (it != m_entities.end()) ? it->get() : nullptr;
}

void pragma::asset::WorldData::AddEntity(EntityData &ent, bool isWorld)
{
	if(m_entities.size() == m_entities.capacity())
		m_entities.reserve(m_entities.size() * 1.5f + 100);
	if(isWorld)
		m_entities.insert(m_entities.begin(), ent.shared_from_this()); // World always has to be first in the list!
	else
		m_entities.push_back(ent.shared_from_this());
	ent.m_mapIndex = m_entities.size();
}
void pragma::asset::WorldData::SetBSPTree(util::BSPTree &bspTree) { m_bspTree = bspTree.shared_from_this(); }
util::BSPTree *pragma::asset::WorldData::GetBSPTree() { return m_bspTree.get(); }

void pragma::asset::WorldData::SetLightMapAtlas(uimg::ImageBuffer &imgAtlas)
{
	m_lightMapAtlas = imgAtlas.shared_from_this();
	SetLightMapEnabled(true);
}
void pragma::asset::WorldData::SetLightMapEnabled(bool enabled) { m_lightMapAtlasEnabled = enabled; }
bool pragma::asset::WorldData::IsLegacyLightMapEnabled() const { return m_useLegacyLightmapDefinition; }
void pragma::asset::WorldData::SetLightMapIntensity(float intensity) { m_lightMapIntensity = intensity; }
void pragma::asset::WorldData::SetLightMapExposure(float exp) { m_lightMapExposure = exp; }
float pragma::asset::WorldData::GetLightMapIntensity() const { return m_lightMapIntensity; }
float pragma::asset::WorldData::GetLightMapExposure() const { return m_lightMapExposure; }

NetworkState &pragma::asset::WorldData::GetNetworkState() const { return m_nw; }

std::vector<uint16_t> &pragma::asset::WorldData::GetStaticPropLeaves() { return m_staticPropLeaves; }
const std::vector<std::shared_ptr<pragma::asset::EntityData>> &pragma::asset::WorldData::GetEntities() const { return m_entities; }
const std::vector<std::string> &pragma::asset::WorldData::GetMaterialTable() const { return const_cast<WorldData *>(this)->GetMaterialTable(); }
std::vector<std::string> &pragma::asset::WorldData::GetMaterialTable() { return m_materialTable; }
std::string pragma::asset::WorldData::GetLightmapAtlasTexturePath(const std::string &mapName) { return "maps/" + mapName + "/lightmap_atlas"; }
void pragma::asset::WorldData::SetMessageLogger(const std::function<void(const std::string &)> &msgLogger)
{
	m_messageLogger = msgLogger ? msgLogger : [](const std::string &) {};
}
std::shared_ptr<pragma::asset::WorldData> pragma::asset::WorldData::Create(NetworkState &nw) { return std::shared_ptr<WorldData> {new WorldData {nw}}; }

std::shared_ptr<pragma::asset::WorldData> pragma::asset::WorldData::load(NetworkState &nw, const std::string &fileName, std::string &outErr, EntityData::Flags entMask)
{
	auto nFileName = filemanager::find_available_file(fileName, get_supported_extensions());
	std::shared_ptr<udm::Data> udmData = nullptr;
	try {
		udmData = udm::Data::Load(nFileName ? *nFileName : fileName);
	}
	catch(const udm::Exception &e) {
		return {};
	}
	if(!udmData)
		return {};
	auto data = udmData->GetAssetData().GetData();
	auto worldData = load_from_udm_data(nw, data, outErr, entMask);
	if(!worldData)
		return {};
	worldData->SetVersion(udmData->GetAssetVersion());
	return worldData;
}
std::shared_ptr<pragma::asset::WorldData> pragma::asset::WorldData::load_from_udm_data(NetworkState &nw, udm::LinkedPropertyWrapper &prop, std::string &outErr, EntityData::Flags entMask)
{
	auto worldData = WorldData::Create(nw);
	if(!worldData->LoadFromAssetData(prop, entMask, outErr))
		return nullptr;
	return worldData;
}
const std::vector<std::string> &pragma::asset::WorldData::get_supported_extensions()
{
	static std::vector<std::string> exts = {PMAP_EXTENSION_ASCII, PMAP_EXTENSION_BINARY};
	return exts;
}

////////////

std::shared_ptr<pragma::asset::ComponentData> pragma::asset::ComponentData::Create() { return std::shared_ptr<ComponentData> {new ComponentData {}}; }

pragma::asset::ComponentData::ComponentData() : m_data {udm::Property::Create<udm::Element>()} {}
pragma::asset::ComponentData::Flags pragma::asset::ComponentData::GetFlags() const { return m_flags; }
void pragma::asset::ComponentData::SetFlags(Flags flags) { m_flags = flags; }

////////////

std::shared_ptr<pragma::asset::EntityData> pragma::asset::EntityData::Create() { return std::shared_ptr<EntityData> {new EntityData {}}; }
bool pragma::asset::EntityData::IsWorld() const { return m_className == "world"; }
bool pragma::asset::EntityData::IsSkybox() const { return m_className == "skybox"; }
bool pragma::asset::EntityData::IsClientSideOnly() const { return umath::is_flag_set(m_flags, Flags::ClientsideOnly); }
void pragma::asset::EntityData::SetClassName(const std::string &className)
{
	m_className = className;
	ustring::to_lower(m_className);
}
void pragma::asset::EntityData::SetKeyValue(const std::string &key, const std::string &value) { m_keyValues[key] = value; }
void pragma::asset::EntityData::AddOutput(const Output &output)
{
	if(m_outputs.size() == m_outputs.capacity())
		m_outputs.reserve(m_outputs.size() * 1.5f + 10);
	m_outputs.push_back(output);
}
void pragma::asset::EntityData::SetLeafData(uint32_t firstLeaf, uint32_t numLeaves)
{
	m_firstLeaf = firstLeaf;
	m_numLeaves = numLeaves;
}

uint32_t pragma::asset::EntityData::GetMapIndex() const { return m_mapIndex; }
const std::string &pragma::asset::EntityData::GetClassName() const { return m_className; }
pragma::asset::EntityData::Flags pragma::asset::EntityData::GetFlags() const { return m_flags; }
void pragma::asset::EntityData::SetFlags(Flags flags) { m_flags = flags; }
std::shared_ptr<pragma::asset::ComponentData> pragma::asset::EntityData::AddComponent(const std::string &name)
{
	auto it = m_components.find(name);
	if(it != m_components.end())
		return it->second;
	auto component = ComponentData::Create();
	m_components[name] = component;
	return component;
}
const std::unordered_map<std::string, std::shared_ptr<pragma::asset::ComponentData>> &pragma::asset::EntityData::GetComponents() const { return const_cast<EntityData *>(this)->GetComponents(); }
std::unordered_map<std::string, std::shared_ptr<pragma::asset::ComponentData>> &pragma::asset::EntityData::GetComponents() { return m_components; }
const std::unordered_map<std::string, std::string> &pragma::asset::EntityData::GetKeyValues() const { return const_cast<EntityData *>(this)->GetKeyValues(); }
std::unordered_map<std::string, std::string> &pragma::asset::EntityData::GetKeyValues() { return m_keyValues; }
const std::vector<pragma::asset::Output> &pragma::asset::EntityData::GetOutputs() const { return const_cast<EntityData *>(this)->GetOutputs(); }
std::vector<pragma::asset::Output> &pragma::asset::EntityData::GetOutputs() { return m_outputs; }
const std::vector<uint16_t> &pragma::asset::EntityData::GetLeaves() const { return const_cast<EntityData *>(this)->GetLeaves(); }
std::vector<uint16_t> &pragma::asset::EntityData::GetLeaves() { return m_leaves; }
std::optional<std::string> pragma::asset::EntityData::GetKeyValue(const std::string &key) const
{
	auto it = m_keyValues.find(key);
	return (it != m_keyValues.end()) ? it->second : std::optional<std::string> {};
}
std::string pragma::asset::EntityData::GetKeyValue(const std::string &key, const std::string &def) const
{
	auto val = GetKeyValue(key);
	return val.has_value() ? *val : def;
}

umath::ScaledTransform pragma::asset::EntityData::GetEffectivePose() const
{
	umath::ScaledTransform pose {};
	if(m_pose)
		pose = *m_pose;
	auto kvOrigin = GetKeyValue("origin");
	if(kvOrigin)
		pose.SetOrigin(uvec::create(*kvOrigin));

	auto kvAngles = GetKeyValue("angles");
	if(kvAngles)
		pose.SetRotation(uquat::create(EulerAngles {*kvAngles}));

	auto kvScale = GetKeyValue("scale");
	if(kvScale)
		pose.SetScale(uvec::create(*kvScale));
	return pose;
}

const std::optional<umath::ScaledTransform> &pragma::asset::EntityData::GetPose() const { return m_pose; }
void pragma::asset::EntityData::SetPose(const umath::ScaledTransform &pose) { m_pose = pose; }
void pragma::asset::EntityData::ClearPose() { m_pose = {}; }

void pragma::asset::EntityData::GetLeafData(uint32_t &outFirstLeaf, uint32_t &outNumLeaves) const
{
	outFirstLeaf = m_firstLeaf;
	outNumLeaves = m_numLeaves;
}

////////////

std::ostream &operator<<(std::ostream &out, const pragma::asset::Output &output)
{
	out << "Output[Name:" << output.name << "][Target:" << output.target << "][Input" << output.input << "][Param:" << output.param << "][Delay:" << output.delay << "][Times:" << output.times << "]";
	return out;
}
std::ostream &operator<<(std::ostream &out, const pragma::asset::ComponentData &componentData)
{
	out << "ComponentData[Flags:" << magic_enum::flags::enum_name(componentData.GetFlags()) << "]";
	return out;
}
std::ostream &operator<<(std::ostream &out, const pragma::asset::EntityData &entityData)
{
	auto uuid = entityData.GetKeyValue("uuid");
	out << "EntityData[Class:" << entityData.GetClassName() << "]";
	if(uuid)
		out << "[Uuid:" << *uuid << "]";
	out << "[MapIndex:" << entityData.GetMapIndex() << "] [Flags:" << magic_enum::flags::enum_name(entityData.GetFlags()) << "] ";
	auto &pose = entityData.GetPose();
	if(pose) {
		auto &pos = pose->GetOrigin();
		auto ang = pose->GetAngles();
		out << "[Pos:" << pos.x << " " << pos.y << " " << pos.z << "]";
		out << "[Ang:" << ang.p << " " << ang.y << " " << ang.r << "]";
	}
	return out;
}
std::ostream &operator<<(std::ostream &out, const pragma::asset::WorldData &worldData)
{
	out << "WorldData";
	return out;
}
