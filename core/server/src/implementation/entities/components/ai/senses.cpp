// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.ai;

import :game;
import :server_state;

using namespace pragma;

bool SAIComponent::IsInViewCone(ecs::BaseEntity *ent, float *dist)
{
	auto &entThis = GetEntity();
	auto charComponent = entThis.GetCharacterComponent();
	auto pTrComponent = ent->GetTransformComponent();
	if(charComponent.expired() || pTrComponent == nullptr)
		return false;
	auto dir = charComponent->GetViewForward();
	auto pos = charComponent->GetEyePosition();
	//auto dir = (charComponent != nullptr) ? charComponent->GetViewForward() : entThis.GetForward();
	//auto pos = (charComponent != nullptr) ? charComponent->GetEyePosition() : entThis.GetPosition();
	auto posEnt = pTrComponent->GetEyePosition();
	auto dirEnt = posEnt - pos;
	uvec::normalize(&dirEnt);
	auto dot = uvec::dot(dir, dirEnt);
	if(dot >= m_maxViewDot) {
		auto d = glm::distance(pos, posEnt);
		if(dist != nullptr)
			*dist = d;
		if(d <= m_maxViewDist) {
			auto data = charComponent->GetAimTraceData();
			data.SetTarget(posEnt);
			auto res = SGame::Get()->RayCast(data);
			if(res.hitType == physics::RayCastHitType::None || res.entity.get() == ent)
				return true;
		}
	}
	return false;
}

bool SAIComponent::CanSee() const { return (GetMaxViewDistance() > 0 && GetMaxViewAngle() > 0) ? true : false; }
void SAIComponent::SetHearingStrength(float strength) { m_hearingStrength = math::clamp(strength, 0.f, 1.f); }
float SAIComponent::GetHearingStrength() const { return m_hearingStrength; }
bool SAIComponent::CanHear() const { return (m_hearingStrength == 0.f) ? false : true; }

bool SAIComponent::OnSuspiciousSoundHeared(std::shared_ptr<audio::ALSound> &snd)
{
	CEOnSuspiciousSoundHeared evData {snd};
	return BroadcastEvent(sAIComponent::EVENT_ON_SUSPICIOUS_SOUND_HEARED, evData) == util::EventReply::Handled;
}

float SAIComponent::GetMaxViewDistance() const { return m_maxViewDist; }
void SAIComponent::SetMaxViewDistance(float dist) { m_maxViewDist = dist; }

float SAIComponent::GetMaxViewDotProduct() const { return m_maxViewDot; }
float SAIComponent::GetMaxViewAngle() const { return m_maxViewAngle; }
void SAIComponent::SetMaxViewAngle(float ang)
{
	m_maxViewAngle = ang;
	m_maxViewDot = 1.f - (ang / 180.f) * 2.f;
}

void SAIComponent::Listen(std::vector<TargetInfo> &targets)
{
	if(CanHear() == false)
		return;
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent == nullptr)
		return;
	auto hearingIntensity = 1.f - math::clamp(GetHearingStrength(), 0.f, 1.f);
	auto &pos = pTrComponent->GetPosition();
	auto &sounds = ServerState::Get()->GetSounds();
	auto &t = SGame::Get()->CurTime();
	for(auto &rsnd : sounds) {
		auto &snd = rsnd.get();
		if(snd.IsPlaying() == false || snd.IsRelative() == true)
			continue;
		auto *ent = snd.GetSource();
		if(ent != nullptr && (ent->IsPlayer() || ent->IsNPC()) && HasCharacterNoTargetEnabled(*ent) == false && IsEnemy(ent) == true) {
			auto intensity = snd.GetSoundIntensity(pos);
			if(intensity > 0.f && intensity >= hearingIntensity) // Sound is audible for this NPC
			{
				auto *fragment = GetMemory(ent);
				if(fragment == nullptr) {
					auto ptrSnd = snd.shared_from_this();
					if(OnSuspiciousSoundHeared(ptrSnd) == false) // Sound was emitted by entity we don't know yet; If OnSuspiciousSoundHeared returned false, use default behavior (Just add target to memory)
					{
						if((fragment = Memorize(ent, ai::Memory::MemoryType::Sound, snd.GetPosition(), {})) != nullptr)
							targets.push_back({ent, fragment->lastDistance});
					}
				}
				else if(fragment != nullptr) {
					fragment->lastHeared = static_cast<float>(SGame::Get()->CurTime());
					if(fragment->visible == false) // Don't bother updating if we can see the target.
					{
						auto bUpdatePos = ((t - fragment->lastSeen) >= AI_LISTEN_VISIBILITY_THRESHOLD) ? true : false; // We haven't seen the target in a while?
						if(bUpdatePos == false) {
							auto dLast = uvec::distance(pos, fragment->lastPosition);
							auto dSnd = uvec::distance(pos, snd.GetPosition());
							bUpdatePos = (dSnd < dLast ||                          // Sound position is closer than last known position of the target
							               dLast < AI_LISTEN_DISTANCE_THRESHOLD || // Last known position is very close, but target isn't visible -> Probably should update
							               HasReachedDestination() == true         // We don't have any path right now, no reason to sit idle
							               )
							  ? true
							  : false;
							if(bUpdatePos == true) // Distance to sound origin is closer than distance to previously known position of target.
								fragment->lastDistance = dSnd;
						}
						if(bUpdatePos == true) {
							fragment->lastPosition = snd.GetPosition(); // We'll assume the target is near the sound origin.
							fragment->lastVelocity = {};                // Clear all knowledge about last velocity
						}
					}
				}
			}
		}
	}
}
