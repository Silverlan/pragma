/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/environment/audio/env_sound.h"
#include "pragma/audio/alsound_type.h"
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/entities/components/base_io_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/asset/util_asset.hpp"

using namespace pragma;

void BaseEnvSoundComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = BaseEnvSoundComponent;
	{
		using TSound = std::string;
		auto memberInfo = create_component_member_info<T, TSound, [](const ComponentMemberInfo &info, T &component, const TSound &mdl) { component.SetSoundSource(mdl); }, static_cast<const TSound &(T::*)() const>(&T::GetSoundSource)>("sound", "", AttributeSpecializationType::File);
		auto &metaData = memberInfo.AddMetaData();
		metaData["assetType"] = "sound";
		metaData["rootPath"] = util::Path::CreatePath(pragma::asset::get_asset_root_directory(pragma::asset::Type::Sound)).GetString();
		metaData["extensions"] = pragma::asset::get_supported_extensions(pragma::asset::Type::Sound, pragma::asset::FormatType::All);
		metaData["stripRootPath"] = true;
		metaData["stripExtension"] = true;
		registerMember(std::move(memberInfo));
	}

	{
		auto memberInfo = create_component_member_info<T, float, static_cast<void (T::*)(float)>(&T::SetPitch), static_cast<float (T::*)() const>(&T::GetPitch)>("pitch", 1.f);
		memberInfo.SetMin(0.f);
		memberInfo.SetMax(1.f);
		registerMember(std::move(memberInfo));
	}

	{
		auto memberInfo = create_component_member_info<T, float, static_cast<void (T::*)(float)>(&T::SetGain), static_cast<float (T::*)() const>(&T::GetGain)>("volume", 1.f);
		memberInfo.SetMin(0.f);
		memberInfo.SetMax(1.f);
		registerMember(std::move(memberInfo));
	}

	{
		auto memberInfo = create_component_member_info<T, float, static_cast<void (T::*)(float)>(&T::SetMaxDistance), static_cast<float (T::*)() const>(&T::GetMaxDistance)>("maxDist", 1024.f, AttributeSpecializationType::Distance);
		memberInfo.SetMin(0.f);
		memberInfo.SetMax(8'192.f);
		registerMember(std::move(memberInfo));
	}

	{
		auto memberInfo = create_component_member_info<T, float, static_cast<void (T::*)(float)>(&T::SetReferenceDistance), static_cast<float (T::*)() const>(&T::GetReferenceDistance)>("refDist", 1.f, AttributeSpecializationType::Distance);
		memberInfo.SetMin(0.f);
		memberInfo.SetMax(8'192.f);
		registerMember(std::move(memberInfo));
	}

	{
		auto memberInfo = create_component_member_info<T, float, static_cast<void (T::*)(float)>(&T::SetRolloffFactor), static_cast<float (T::*)() const>(&T::GetRolloffFactor)>("rolloffFactor", 1.f);
		memberInfo.SetMin(0.f);
		memberInfo.SetMax(1.f);
		registerMember(std::move(memberInfo));
	}
}
void BaseEnvSoundComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(ustring::compare<std::string>(kvData.key, "sound", false))
			m_kvSoundName = kvData.value;
		else if(ustring::compare<std::string>(kvData.key, "pitch", false))
			m_kvPitch = util::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "gain", false))
			m_kvGain = util::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "rolloff", false))
			m_kvRolloff = util::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "min_gain", false))
			m_kvMinGain = util::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "max_gain", false))
			m_kvMaxGain = util::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "inner_cone", false))
			m_kvInnerCone = util::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "outer_cone", false))
			m_kvOuterCone = util::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "offset", false))
			m_kvOffset = util::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "reference_dist", false))
			m_kvReferenceDist = util::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "max_dist", false))
			m_kvMaxDist = util::to_float(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEvent(BaseIOComponent::EVENT_HANDLE_INPUT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData &>(evData.get());
		ALSound *snd = (m_sound != NULL) ? m_sound.get() : NULL;
		if(ustring::compare<std::string>(inputData.input, "play", false))
			if(snd != NULL)
				snd->Play();
			else if(ustring::compare<std::string>(inputData.input, "stop", false))
				if(snd != NULL)
					snd->Stop();
				else if(ustring::compare<std::string>(inputData.input, "toggle", false)) {
					if(snd != NULL) {
						if(snd->IsPlaying())
							snd->Pause();
						else
							snd->Play();
					}
				}
				else if(ustring::compare<std::string>(inputData.input, "fadein", false))
					if(snd != NULL)
						snd->FadeIn(util::to_float(inputData.data));
					else if(ustring::compare<std::string>(inputData.input, "fadeout", false))
						if(snd != NULL)
							snd->FadeOut(util::to_float(inputData.data));
						else if(ustring::compare<std::string>(inputData.input, "rewind", false))
							if(snd != NULL)
								snd->Rewind();
							else if(ustring::compare<std::string>(inputData.input, "pause", false))
								if(snd != NULL)
									snd->Pause();
								else if(ustring::compare<std::string>(inputData.input, "setpitch", false))
									if(snd != NULL)
										snd->SetPitch(util::to_float(inputData.data));
									else if(ustring::compare<std::string>(inputData.input, "setlooping", false))
										if(snd != NULL)
											snd->SetLooping(util::to_boolean(inputData.data));
										else if(ustring::compare<std::string>(inputData.input, "setgain", false))
											if(snd != NULL)
												snd->SetGain(util::to_float(inputData.data));
											else if(ustring::compare<std::string>(inputData.input, "setrelativetolistener", false))
												if(snd != NULL)
													snd->SetRelative(util::to_boolean(inputData.data));
												else if(ustring::compare<std::string>(inputData.input, "setoffset", false))
													if(snd != NULL)
														snd->SetOffset(util::to_float(inputData.data));
													else if(ustring::compare<std::string>(inputData.input, "setsecoffset", false))
														if(snd != NULL)
															snd->SetTimeOffset(util::to_float(inputData.data));
														else if(ustring::compare<std::string>(inputData.input, "setrollofffactor", false))
															if(snd != NULL)
																snd->SetRolloffFactor(util::to_float(inputData.data));
															else if(ustring::compare<std::string>(inputData.input, "setmaxdistance", false))
																if(snd != NULL)
																	snd->SetMaxDistance(util::to_float(inputData.data));
																else if(ustring::compare<std::string>(inputData.input, "setmingain", false))
																	if(snd != NULL)
																		snd->SetMinGain(util::to_float(inputData.data));
																	else if(ustring::compare<std::string>(inputData.input, "setmaxgain", false))
																		if(snd != NULL)
																			snd->SetMaxGain(util::to_float(inputData.data));
																		else if(ustring::compare<std::string>(inputData.input, "setconeinnerangle", false))
																			if(snd != NULL)
																				snd->SetInnerConeAngle(util::to_float(inputData.data));
																			else if(ustring::compare<std::string>(inputData.input, "setconeouterangle", false))
																				if(snd != NULL)
																					snd->SetOuterConeAngle(util::to_float(inputData.data));
																				else
																					return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	GetEntity().AddComponent("io");
}

void BaseEnvSoundComponent::OnRemove()
{
	Stop();
	BaseEntityComponent::OnRemove();
}

float BaseEnvSoundComponent::GetPitch() const
{
	if(m_sound)
		return m_sound->GetPitch();
	return m_kvPitch;
}
float BaseEnvSoundComponent::GetGain() const
{
	if(m_sound)
		return m_sound->GetGain();
	return m_kvGain;
}
float BaseEnvSoundComponent::GetRolloffFactor() const
{
	if(m_sound)
		return m_sound->GetRolloffFactor();
	return m_kvRolloff;
}
float BaseEnvSoundComponent::GetMinGain() const
{
	if(m_sound)
		return m_sound->GetMinGain();
	return m_kvMinGain;
}
float BaseEnvSoundComponent::GetMaxGain() const
{
	if(m_sound)
		return m_sound->GetMaxGain();
	return m_kvMaxGain;
}
float BaseEnvSoundComponent::GetInnerConeAngle() const
{
	if(m_sound)
		return m_sound->GetInnerConeAngle();
	return m_kvInnerCone;
}
float BaseEnvSoundComponent::GetOuterConeAngle() const
{
	if(m_sound)
		return m_sound->GetOuterConeAngle();
	return m_kvOuterCone;
}
float BaseEnvSoundComponent::GetReferenceDistance() const
{
	if(m_sound)
		return m_sound->GetReferenceDistance();
	return m_kvReferenceDist;
}
float BaseEnvSoundComponent::GetMaxDistance() const
{
	if(m_sound)
		return m_sound->GetMaxDistance();
	return m_kvMaxDist;
}

const std::string &BaseEnvSoundComponent::GetSoundSource() const { return m_kvSoundName; }
void BaseEnvSoundComponent::SetSoundSource(const std::string &sndName) { m_kvSoundName = sndName; }
void BaseEnvSoundComponent::SetPitch(float pitch)
{
	m_kvPitch = pitch;
	if(m_sound != nullptr)
		m_sound->SetPitch(pitch);
}
void BaseEnvSoundComponent::SetGain(float gain)
{
	m_kvGain = gain;
	if(m_sound != nullptr)
		m_sound->SetGain(gain);
}
void BaseEnvSoundComponent::SetRolloffFactor(float rolloffFactor)
{
	m_kvRolloff = rolloffFactor;
	if(m_sound != nullptr)
		m_sound->SetRolloffFactor(rolloffFactor);
}
void BaseEnvSoundComponent::SetMinGain(float minGain)
{
	m_kvMinGain = minGain;
	if(m_sound != nullptr)
		m_sound->SetMinGain(minGain);
}
void BaseEnvSoundComponent::SetMaxGain(float maxGain)
{
	m_kvMaxGain = maxGain;
	if(m_sound != nullptr)
		m_sound->SetMaxGain(maxGain);
}
void BaseEnvSoundComponent::SetInnerConeAngle(float angle)
{
	m_kvInnerCone = angle;
	if(m_sound != nullptr)
		m_sound->SetInnerConeAngle(angle);
}
void BaseEnvSoundComponent::SetOuterConeAngle(float angle)
{
	m_kvOuterCone = angle;
	if(m_sound != nullptr)
		m_sound->SetOuterConeAngle(angle);
}
void BaseEnvSoundComponent::SetOffset(float offset)
{
	m_kvOffset = offset;
	if(m_sound != nullptr)
		m_sound->SetOffset(offset);
}
void BaseEnvSoundComponent::SetTimeOffset(float offsetInSeconds)
{
	if(m_sound)
		m_sound->SetTimeOffset(offsetInSeconds);
}
float BaseEnvSoundComponent::GetOffset() const
{
	if(m_sound)
		return m_sound->GetOffset();
	return m_kvOffset;
}
float BaseEnvSoundComponent::GetTimeOffset() const
{
	if(m_sound)
		return m_sound->GetTimeOffset();
	return 0.f;
}
void BaseEnvSoundComponent::SetReferenceDistance(float referenceDist)
{
	m_kvReferenceDist = referenceDist;
	if(m_sound != nullptr)
		m_sound->SetReferenceDistance(referenceDist);
}
void BaseEnvSoundComponent::SetMaxDistance(float maxDist)
{
	m_kvMaxDist = maxDist;
	if(m_sound != nullptr)
		m_sound->SetMaxDistance(maxDist);
}
void BaseEnvSoundComponent::SetRelativeToListener(bool bRelative)
{
	auto &ent = GetEntity();
	auto spawnFlags = ent.GetSpawnFlags();
	if(bRelative)
		ent.SetSpawnFlags(spawnFlags | umath::to_integral(SpawnFlags::PlayEverywhere));
	else
		ent.SetSpawnFlags(spawnFlags & ~umath::to_integral(SpawnFlags::PlayEverywhere));
	if(m_sound != nullptr)
		m_sound->SetRelative(bRelative);
}
void BaseEnvSoundComponent::SetPlayOnSpawn(bool bPlayOnSpawn)
{
	auto &ent = GetEntity();
	auto spawnFlags = ent.GetSpawnFlags();
	if(bPlayOnSpawn)
		ent.SetSpawnFlags(spawnFlags | umath::to_integral(SpawnFlags::PlayOnSpawn));
	else
		ent.SetSpawnFlags(spawnFlags & ~umath::to_integral(SpawnFlags::PlayOnSpawn));
}
void BaseEnvSoundComponent::SetLooping(bool bLoop)
{
	auto &ent = GetEntity();
	auto spawnFlags = ent.GetSpawnFlags();
	if(bLoop)
		ent.SetSpawnFlags(spawnFlags | umath::to_integral(SpawnFlags::IsLooped));
	else
		ent.SetSpawnFlags(spawnFlags & ~umath::to_integral(SpawnFlags::IsLooped));
	if(m_sound != nullptr)
		m_sound->SetLooping(bLoop);
}
void BaseEnvSoundComponent::SetSoundType(ALSoundType types)
{
	m_soundTypes = types;
	GetEntity().SetSpawnFlags(GetEntity().GetSpawnFlags() & ~umath::to_integral(SpawnFlags::AllTypes));
	if(m_sound != nullptr)
		m_sound->SetType(types);
}
void BaseEnvSoundComponent::InitializeSound()
{
	if(m_kvSoundName.empty() || (m_sound != nullptr && m_sound->IsPlaying()))
		return;
	auto &ent = GetEntity();
	auto *nw = ent.GetNetworkState();
	auto spawnFlags = static_cast<SpawnFlags>(ent.GetSpawnFlags());
	auto mode = ALChannel::Auto;
	auto createFlags = ALCreateFlags::None;
	if((spawnFlags & SpawnFlags::PlayEverywhere) == SpawnFlags::None) {
		mode = ALChannel::Mono;
		createFlags = ALCreateFlags::Mono;
	}
	nw->PrecacheSound(m_kvSoundName, mode);

	auto type = m_soundTypes;
	const std::unordered_map<SpawnFlags, ALSoundType> types = {{SpawnFlags::Effect, ALSoundType::Effect}, {SpawnFlags::Music, ALSoundType::Music}, {SpawnFlags::Voice, ALSoundType::Voice}, {SpawnFlags::Weapon, ALSoundType::Weapon}, {SpawnFlags::NPC, ALSoundType::NPC},
	  {SpawnFlags::Player, ALSoundType::Player}, {SpawnFlags::Vehicle, ALSoundType::Vehicle}, {SpawnFlags::Physics, ALSoundType::Physics}, {SpawnFlags::Environment, ALSoundType::Environment}, {SpawnFlags::GUI, ALSoundType::GUI}};
	for(auto &pair : types) {
		if((spawnFlags & pair.first) != SpawnFlags::None)
			type |= pair.second;
	}

	m_sound = nw->CreateSound(m_kvSoundName, type, createFlags);

	auto *snd = m_sound.get();
	if(snd != NULL) {
		snd->SetSource(&ent);

		snd->AddCallback("OnStateChanged", FunctionCallback<void, ALState, ALState>::Create(std::bind(&BaseEnvSoundComponent::InjectStateChange, this, std::placeholders::_1, std::placeholders::_2)));
		if((spawnFlags & SpawnFlags::IsLooped) != SpawnFlags::None)
			snd->SetLooping(true);
		if((spawnFlags & SpawnFlags::PlayEverywhere) != SpawnFlags::None) {
			m_kvMaxDist = std::numeric_limits<float>::max();
			snd->SetRelative(true);
			snd->SetPosition(Vector3(0, 0, 0));
			snd->SetVelocity(Vector3(0, 0, 0));
			snd->SetDirection(Vector3(0, 0, 1));
		}

		snd->SetPitch(m_kvPitch);
		snd->SetGain(m_kvGain);
		snd->SetRolloffFactor(m_kvRolloff);
		snd->SetMinGain(m_kvMinGain);
		snd->SetMaxGain(m_kvMaxGain);
		snd->SetInnerConeAngle(m_kvInnerCone);
		snd->SetOuterConeAngle(m_kvOuterCone);
		snd->SetOffset(m_kvOffset);
		snd->SetReferenceDistance(m_kvReferenceDist);
		snd->SetMaxDistance(m_kvMaxDist);
		OnSoundCreated(*snd);
	}
}
void BaseEnvSoundComponent::Play()
{
	if(m_sound == nullptr)
		return;
	m_sound->Play();
}
void BaseEnvSoundComponent::Stop()
{
	if(m_sound == nullptr)
		return;
	m_sound->Stop();
}
void BaseEnvSoundComponent::Pause()
{
	if(m_sound == nullptr)
		return;
	m_sound->Pause();
}
bool BaseEnvSoundComponent::IsPlaying() const { return (m_sound != nullptr) ? m_sound->IsPlaying() : false; }
bool BaseEnvSoundComponent::IsPaused() const { return (m_sound != nullptr) ? m_sound->IsPaused() : true; }
const std::shared_ptr<ALSound> &BaseEnvSoundComponent::GetSound() const { return m_sound; }

void BaseEnvSoundComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	InitializeSound();
	if((static_cast<SpawnFlags>(GetEntity().GetSpawnFlags()) & SpawnFlags::PlayOnSpawn) != SpawnFlags::None)
		Play();
}

void BaseEnvSoundComponent::InjectStateChange(ALState oldState, ALState newState) {}
void BaseEnvSoundComponent::OnSoundCreated(ALSound &snd) {}
