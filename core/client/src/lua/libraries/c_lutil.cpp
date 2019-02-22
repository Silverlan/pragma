#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"
#include "pragma/lua/libraries/c_lutil.h"
#include "pragma/lua/classes/c_lcamera.h"
#include "pragma/lua/classes/c_ldef_camera.h"
#include "pragma/entities/point/c_point_target.h"
#include "luasystem.h"
#include "pragma/entities/components/c_render_component.hpp"
#include <pragma/lua/classes/ldef_color.h>
#include <pragma/lua/classes/ldef_vector.h>
#include <pragma/lua/classes/ldef_quaternion.h>
#include <pragma/lua/classes/ldef_entity.h>
#include <pragma/util/giblet_create_info.hpp>

extern DLLCLIENT CGame *c_game;

int Lua::util::Client::calc_world_direction_from_2d_coordinates(lua_State *l)
{
	int32_t arg = 1;
	if(!Lua::IsCamera(l,arg))
		return Lua::util::calc_world_direction_from_2d_coordinates(l);
	auto *hCam = Lua::CheckCamera(l,arg++);
	auto width = Lua::CheckInt(l,arg++);
	auto height = Lua::CheckInt(l,arg++);
	auto *uv = Lua::CheckVector2(l,arg++);
	auto *cam = hCam->get();
	auto dir = uvec::calc_world_direction_from_2d_coordinates(cam->GetFOVRad(),static_cast<float>(width),static_cast<float>(height),cam->GetForward(),cam->GetRight(),cam->GetUp(),*uv);
	Lua::Push<Vector3>(l,dir);
	return 1;
}

int Lua::util::Client::create_particle_tracer(lua_State *l)
{
	auto &start = *Lua::CheckVector(l,1);
	auto &end = *Lua::CheckVector(l,2);
	auto radius = Lua::IsSet(l,3) ? Lua::CheckNumber(l,3) : BulletInfo::DEFAULT_TRACER_RADIUS;
	const auto *col = Lua::IsSet(l,4) ? Lua::CheckColor(l,4) : &BulletInfo::DEFAULT_TRACER_COLOR;
	auto length = Lua::IsSet(l,5) ? Lua::CheckNumber(l,5) : BulletInfo::DEFAULT_TRACER_LENGTH;
	auto speed = Lua::IsSet(l,6) ? Lua::CheckNumber(l,6) : BulletInfo::DEFAULT_TRACER_SPEED;
	auto *mat = Lua::IsSet(l,7) ? Lua::CheckString(l,7) : BulletInfo::DEFAULT_TRACER_MATERIAL.c_str();
	auto bloomScale = Lua::IsSet(l,8) ? Lua::CheckNumber(l,8) : BulletInfo::DEFAULT_TRACER_BLOOM;

	auto *particle = c_game->CreateParticleTracer(start,end,static_cast<float>(radius),*col,static_cast<float>(length),static_cast<float>(speed),mat,static_cast<float>(bloomScale));
	if(particle == nullptr)
		return 0;
	particle->PushLuaObject(l);
	return 1;
}

int Lua::util::Client::create_muzzle_flash(lua_State *l)
{
	if(Lua::IsEntity(l,1) == true)
	{
		auto *ent = Lua::CheckEntity(l,1);
		int32_t attId = -1;
		std::string att {};
		if(Lua::IsNumber(l,2))
			attId = Lua::CheckInt(l,2);
		else
			att = Lua::CheckString(l,2);
		Vector3 relOffset {};
		auto relRot = uquat::identity();
		if(Lua::IsSet(l,3))
		{
			relOffset = *Lua::CheckVector(l,3);
			if(Lua::IsSet(l,4))
				relRot = *Lua::CheckQuaternion(l,4);
		}
		std::string particleName = "muzzleflash0" +std::to_string(umath::random(1,6));
		auto *pt = pragma::CParticleSystemComponent::Create(particleName);
		if(pt == nullptr)
			return 0;
		auto pRenderComponent = static_cast<CBaseEntity*>(ent)->GetRenderComponent();
		if(pRenderComponent.valid())
			pt->SetRenderMode(pRenderComponent->GetRenderMode());
		pt->GetEntity().SetKeyValue("transform_with_emitter","1");
		pt->SetRemoveOnComplete(true);
		auto &entPt = pt->GetEntity();
		pt->Start();
		auto pAttachableComponent = entPt.AddComponent<pragma::CAttachableComponent>();
		if(pAttachableComponent.valid())
		{
			AttachmentInfo attInfo {};
			attInfo.offset = relOffset;
			attInfo.rotation = relRot;
			attInfo.flags |= FAttachmentMode::UpdateEachFrame;
			if(att.empty() == false)
				pAttachableComponent->AttachToAttachment(ent,att,attInfo);
			else
				pAttachableComponent->AttachToAttachment(ent,attId,attInfo);
		}
		pt->PushLuaObject(l);
		return 1;
	}
	auto &pos = *Lua::CheckVector(l,1);
	auto &rot = *Lua::CheckQuaternion(l,2);
	std::string particleName = "muzzleflash0" +std::to_string(umath::random(1,6));
	auto *pt = pragma::CParticleSystemComponent::Create(particleName);
	if(pt == nullptr)
		return 0;
	auto pTrComponent = pt->GetEntity().GetTransformComponent();
	if(pTrComponent.valid())
	{
		pTrComponent->SetPosition(pos);
		pTrComponent->SetOrientation(rot);
	}
	pt->SetRemoveOnComplete(true);
	pt->Start();
	pt->PushLuaObject(l);
	return 1;
}

int Lua::util::Client::create_giblet(lua_State *l)
{
	auto *createInfo = Lua::CheckGibletCreateInfo(l,1);
	pragma::CParticleSystemComponent *particle = nullptr;
	c_game->CreateGiblet(*createInfo,&particle);
	if(particle == nullptr)
		return 0;
	particle->PushLuaObject(l);
	return 1;
}

