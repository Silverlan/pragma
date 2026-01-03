// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :scripting.lua.libraries.util;

import :game;
import :server_state;

luabind::object Lua::util::Server::fire_bullets(lua::State *l, const pragma::game::BulletInfo &bulletInfo) { return fire_bullets(l, bulletInfo, false); }
luabind::object Lua::util::Server::fire_bullets(lua::State *l, const pragma::game::BulletInfo &bulletInfo, bool hitReport)
{
	uint8_t tracerSettings = 0;
	if(bulletInfo.tracerRadius != pragma::game::bulletInfo::DEFAULT_TRACER_RADIUS)
		tracerSettings |= 1;

	if(bulletInfo.tracerColor != pragma::game::bulletInfo::DEFAULT_TRACER_COLOR)
		tracerSettings |= 2;

	if(bulletInfo.tracerLength != pragma::game::bulletInfo::DEFAULT_TRACER_LENGTH)
		tracerSettings |= 4;

	if(bulletInfo.tracerSpeed != pragma::game::bulletInfo::DEFAULT_TRACER_SPEED)
		tracerSettings |= 8;

	if(bulletInfo.tracerMaterial != pragma::game::bulletInfo::DEFAULT_TRACER_MATERIAL)
		tracerSettings |= 16;

	if(bulletInfo.tracerBloom != pragma::game::bulletInfo::DEFAULT_TRACER_BLOOM)
		tracerSettings |= 32;

	::NetPacket packet;
	std::vector<Vector3> hitPositions;
	std::vector<Vector3> hitNormals;
	std::vector<int32_t> hitSurfaceMaterials;
	Vector3 start;
	uint32_t numTracer = 0;
	auto r
	  = util::fire_bullets(l, const_cast<pragma::game::BulletInfo &>(bulletInfo), hitReport, [&hitPositions, &hitNormals, &hitSurfaceMaterials, &start, &numTracer](pragma::game::DamageInfo &dmg, pragma::physics::TraceData &, pragma::physics::TraceResult &result, uint32_t &tracerCount) {
		    if(result.hitType != pragma::physics::RayCastHitType::None) {
			    hitPositions.push_back(result.position);
			    hitNormals.push_back(result.normal);

			    auto surfMatId = (result.collisionObj.IsValid()) ? result.collisionObj->GetSurfaceMaterial() : -1;
			    hitSurfaceMaterials.push_back(surfMatId);
		    }
		    start = dmg.GetSource();
		    numTracer = tracerCount;
	    });
	auto numHits = pragma::math::min(hitPositions.size(), static_cast<std::size_t>(255));
	packet->Write<uint8_t>(static_cast<uint8_t>(numTracer));
	packet->Write<uint8_t>(tracerSettings);
	if(tracerSettings & 1)
		packet->Write<float>(bulletInfo.tracerRadius);
	if(tracerSettings & 2) {
		packet->Write<uint8_t>(static_cast<uint8_t>(bulletInfo.tracerColor.r));
		packet->Write<uint8_t>(static_cast<uint8_t>(bulletInfo.tracerColor.g));
		packet->Write<uint8_t>(static_cast<uint8_t>(bulletInfo.tracerColor.b));
		packet->Write<uint8_t>(static_cast<uint8_t>(bulletInfo.tracerColor.a));
	}
	if(tracerSettings & 4)
		packet->Write<float>(bulletInfo.tracerLength);
	if(tracerSettings & 8)
		packet->Write<float>(bulletInfo.tracerSpeed);
	if(tracerSettings & 16)
		packet->WriteString(bulletInfo.tracerMaterial);
	if(tracerSettings & 32)
		packet->Write<float>(bulletInfo.tracerBloom);

	packet->Write<Vector3>(start);
	packet->Write<uint8_t>(static_cast<uint8_t>(numHits));
	for(auto i = decltype(numHits) {0}; i < numHits; ++i) {
		auto &p = hitPositions[i];
		auto &n = hitNormals[i];
		auto surfaceMaterial = hitSurfaceMaterials[i];
		packet->Write<Vector3>(p);
		packet->Write<Vector3>(n);
		packet->Write<int32_t>(surfaceMaterial);
	}
	pragma::ServerState::Get()->SendPacket(pragma::networking::net_messages::client::FIRE_BULLET, packet, pragma::networking::Protocol::FastUnreliable);
	return r;
}

void Lua::util::Server::create_giblet(lua::State *l, const GibletCreateInfo &gibletInfo) { pragma::SGame::Get()->CreateGiblet(gibletInfo); }

void Lua::util::Server::create_explosion(lua::State *l, const pragma::util::SplashDamageInfo &splashDamageInfo) { splash_damage(l, splashDamageInfo); }
