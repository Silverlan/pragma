/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#include "stdafx_server.h"
#include "pragma/lua/libraries/s_lutil.h"
#include <pragma/lua/classes/ldef_color.h>
#include <pragma/physics/raytraces.h>
#include <pragma/lua/libraries/lutil.h>
#include <pragma/util/giblet_create_info.hpp>
#include <pragma/util/bulletinfo.h>
#include <pragma/util/util_splash_damage_info.hpp>
#include <sharedutils/netpacket.hpp>
#include <pragma/networking/enums.hpp>

extern DLLSERVER ServerState *server;
extern DLLSERVER SGame *s_game;

int Lua::util::Server::fire_bullets(lua_State *l,const BulletInfo &bulletInfo)
{
	uint8_t tracerSettings = 0;
	if(bulletInfo.tracerRadius != BulletInfo::DEFAULT_TRACER_RADIUS)
		tracerSettings |= 1;

	if(bulletInfo.tracerColor != BulletInfo::DEFAULT_TRACER_COLOR)
		tracerSettings |= 2;

	if(bulletInfo.tracerLength != BulletInfo::DEFAULT_TRACER_LENGTH)
		tracerSettings |= 4;

	if(bulletInfo.tracerSpeed != BulletInfo::DEFAULT_TRACER_SPEED)
		tracerSettings |= 8;

	if(bulletInfo.tracerMaterial != BulletInfo::DEFAULT_TRACER_MATERIAL)
		tracerSettings |= 16;

	if(bulletInfo.tracerBloom != BulletInfo::DEFAULT_TRACER_BLOOM)
		tracerSettings |= 32;

	NetPacket packet;
	std::vector<Vector3> hitPositions;
	std::vector<Vector3> hitNormals;
	std::vector<int32_t> hitSurfaceMaterials;
	Vector3 start;
	uint32_t numTracer = 0;
	auto r = Lua::util::fire_bullets(l,[&hitPositions,&hitNormals,&hitSurfaceMaterials,&start,&numTracer](DamageInfo &dmg,TraceData&,TraceResult &result,uint32_t &tracerCount) {
		if(result.hitType != RayCastHitType::None)
		{
			hitPositions.push_back(result.position);
			hitNormals.push_back(result.normal);

			auto surfMatId = (result.collisionObj.IsValid()) ? result.collisionObj->GetSurfaceMaterial() : -1;
			hitSurfaceMaterials.push_back(surfMatId);
		}
		start = dmg.GetSource();
		numTracer = tracerCount;
	});
	auto numHits = umath::min(hitPositions.size(),static_cast<std::size_t>(255));
	packet->Write<uint8_t>(static_cast<uint8_t>(numTracer));
	packet->Write<uint8_t>(tracerSettings);
	if(tracerSettings &1)
		packet->Write<float>(bulletInfo.tracerRadius);
	if(tracerSettings &2)
	{
		packet->Write<uint8_t>(static_cast<uint8_t>(bulletInfo.tracerColor.r));
		packet->Write<uint8_t>(static_cast<uint8_t>(bulletInfo.tracerColor.g));
		packet->Write<uint8_t>(static_cast<uint8_t>(bulletInfo.tracerColor.b));
		packet->Write<uint8_t>(static_cast<uint8_t>(bulletInfo.tracerColor.a));
	}
	if(tracerSettings &4)
		packet->Write<float>(bulletInfo.tracerLength);
	if(tracerSettings &8)
		packet->Write<float>(bulletInfo.tracerSpeed);
	if(tracerSettings &16)
		packet->WriteString(bulletInfo.tracerMaterial);
	if(tracerSettings &32)
		packet->Write<float>(bulletInfo.tracerBloom);

	packet->Write<Vector3>(start);
	packet->Write<uint8_t>(static_cast<uint8_t>(numHits));
	for(auto i=decltype(numHits){0};i<numHits;++i)
	{
		auto &p = hitPositions[i];
		auto &n = hitNormals[i];
		auto surfaceMaterial = hitSurfaceMaterials[i];
		packet->Write<Vector3>(p);
		packet->Write<Vector3>(n);
		packet->Write<int32_t>(surfaceMaterial);
	}
	server->SendPacket("fire_bullet",packet,pragma::networking::Protocol::FastUnreliable);
	return r;
}

void Lua::util::Server::create_giblet(lua_State *l,const GibletCreateInfo &gibletInfo)
{
	s_game->CreateGiblet(gibletInfo);
}

void Lua::util::Server::create_explosion(lua_State *l,const ::util::SplashDamageInfo &splashDamageInfo)
{
	Lua::util::splash_damage(l,splashDamageInfo);
}
