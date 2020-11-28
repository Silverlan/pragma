/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include "pragma/model/c_modelmesh.h"
#include <prosper_command_buffer.hpp>
#include <pragma/math/intersection.h>

void Lua::Render::register_class(lua_State *l,luabind::module_ &entsMod)
{
	auto defCRender = luabind::class_<CRenderHandle,BaseEntityComponentHandle>("RenderComponent");
	Lua::register_base_render_component_methods<luabind::class_<CRenderHandle,BaseEntityComponentHandle>,CRenderHandle>(l,defCRender);
	defCRender.def("GetTransformationMatrix",&Lua::Render::GetTransformationMatrix);
	defCRender.def("SetRenderMode",&Lua::Render::SetRenderMode);
	defCRender.def("GetRenderMode",&Lua::Render::GetRenderMode);
	defCRender.def("GetRenderModeProperty",static_cast<void(*)(lua_State*,CRenderHandle&)>([](lua_State *l,CRenderHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::Property::push(l,*hComponent->GetRenderModeProperty());
		}));
	defCRender.def("GetLocalRenderBounds",&Lua::Render::GetLocalRenderBounds);
	defCRender.def("GetLocalRenderSphereBounds",&Lua::Render::GetLocalRenderSphereBounds);
	defCRender.def("GetAbsoluteRenderBounds",&Lua::Render::GetAbsoluteRenderBounds);
	defCRender.def("GetAbsoluteRenderSphereBounds",&Lua::Render::GetAbsoluteRenderSphereBounds);
	defCRender.def("SetLocalRenderBounds",&Lua::Render::SetLocalRenderBounds);
	// defCRender.def("UpdateRenderBuffers",static_cast<void(*)(lua_State*,CRenderHandle&,std::shared_ptr<prosper::ICommandBuffer>&,CSceneHandle&,CCameraHandle&,bool)>(&Lua::Render::UpdateRenderBuffers));
	// defCRender.def("UpdateRenderBuffers",static_cast<void(*)(lua_State*,CRenderHandle&,std::shared_ptr<prosper::ICommandBuffer>&,CSceneHandle&,CCameraHandle&)>(&Lua::Render::UpdateRenderBuffers));
	defCRender.def("GetRenderBuffer",&Lua::Render::GetRenderBuffer);
	defCRender.def("GetBoneBuffer",&Lua::Render::GetBoneBuffer);
	defCRender.def("GetLODMeshes",static_cast<void(*)(lua_State*,CRenderHandle&)>([](lua_State *l,CRenderHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto &lodMeshes = hComponent->GetLODMeshes();
		auto t = Lua::CreateTable(l);
		int32_t idx = 1;
		for(auto &lodMesh : lodMeshes)
		{
			Lua::PushInt(l,idx++);
			Lua::Push(l,lodMesh);
			Lua::SetTableValue(l,t);
		}
		}));
	defCRender.def("SetExemptFromOcclusionCulling",static_cast<void(*)(lua_State*,CRenderHandle&,bool)>([](lua_State *l,CRenderHandle &hComponent,bool exempt) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetExemptFromOcclusionCulling(exempt);
		}));
	defCRender.def("IsExemptFromOcclusionCulling",static_cast<void(*)(lua_State*,CRenderHandle&)>([](lua_State *l,CRenderHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->IsExemptFromOcclusionCulling());
		}));
	defCRender.def("SetReceiveShadows",static_cast<void(*)(lua_State*,CRenderHandle&,bool)>([](lua_State *l,CRenderHandle &hComponent,bool enabled) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetReceiveShadows(enabled);
	}));
	defCRender.def("IsReceivingShadows",static_cast<void(*)(lua_State*,CRenderHandle&)>([](lua_State *l,CRenderHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->IsReceivingShadows());
	}));
	defCRender.def("SetRenderBufferDirty",static_cast<void(*)(lua_State*,CRenderHandle&)>([](lua_State *l,CRenderHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetRenderBufferDirty();
		}));
	defCRender.def("GetDepthBias",static_cast<void(*)(lua_State*,CRenderHandle&)>([](lua_State *l,CRenderHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		float constantFactor,biasClamp,slopeFactor;
		hComponent->GetDepthBias(constantFactor,biasClamp,slopeFactor);
		Lua::PushNumber(l,constantFactor);
		Lua::PushNumber(l,biasClamp);
		Lua::PushNumber(l,slopeFactor);
	}));
	defCRender.def("SetDepthBias",static_cast<void(*)(lua_State*,CRenderHandle&,float,float,float)>([](lua_State *l,CRenderHandle &hComponent,float constantFactor,float biasClamp,float slopeFactor) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetDepthBias(constantFactor,biasClamp,slopeFactor);
	}));
	defCRender.def("CalcRayIntersection",static_cast<void(*)(lua_State*,CRenderHandle&,const Vector3&,const Vector3&,bool)>(&Lua::Render::CalcRayIntersection));
	defCRender.def("CalcRayIntersection",static_cast<void(*)(lua_State*,CRenderHandle&,const Vector3&,const Vector3&)>(&Lua::Render::CalcRayIntersection));
	defCRender.def("SetDepthPassEnabled",static_cast<void(*)(lua_State*,CRenderHandle&,bool)>([](lua_State *l,CRenderHandle &hComponent,bool depthPassEnabled) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetDepthPassEnabled(depthPassEnabled);
	}));
	defCRender.def("IsDepthPassEnabled",static_cast<void(*)(lua_State*,CRenderHandle&)>([](lua_State *l,CRenderHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->IsDepthPassEnabled());
	}));
	defCRender.def("GetRenderClipPlane",static_cast<void(*)(lua_State*,CRenderHandle&)>([](lua_State *l,CRenderHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto *clipPlane = hComponent->GetRenderClipPlane();
		if(clipPlane == nullptr)
			return;
		Lua::Push(l,*clipPlane);
	}));
	defCRender.def("SetRenderClipPlane",static_cast<void(*)(lua_State*,CRenderHandle&,const Vector4&)>([](lua_State *l,CRenderHandle &hComponent,const Vector4 &clipPlane) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetRenderClipPlane(clipPlane);
	}));
	defCRender.def("ClearRenderClipPlane",static_cast<void(*)(lua_State*,CRenderHandle&)>([](lua_State *l,CRenderHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->ClearRenderClipPlane();
	}));
	defCRender.def("GetRenderPose",static_cast<void(*)(lua_State*,CRenderHandle&)>([](lua_State *l,CRenderHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::Push(l,hComponent->GetRenderPose());
	}));
	defCRender.def("SetRenderOffsetTransform",static_cast<void(*)(lua_State*,CRenderHandle&,const umath::ScaledTransform&)>([](lua_State *l,CRenderHandle &hComponent,const umath::ScaledTransform &pose) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetRenderOffsetTransform(pose);
	}));
	defCRender.def("ClearRenderOffsetTransform",static_cast<void(*)(lua_State*,CRenderHandle&)>([](lua_State *l,CRenderHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->ClearRenderOffsetTransform();
	}));
	defCRender.def("GetRenderOffsetTransform",static_cast<void(*)(lua_State*,CRenderHandle&)>([](lua_State *l,CRenderHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto *t = hComponent->GetRenderOffsetTransform();
		if(t == nullptr)
			return;
		Lua::Push(l,*t);
	}));
	// defCRender.add_static_constant("EVENT_ON_UPDATE_RENDER_DATA",pragma::CRenderComponent::EVENT_ON_UPDATE_RENDER_DATA);
	defCRender.add_static_constant("EVENT_ON_RENDER_BOUNDS_CHANGED",pragma::CRenderComponent::EVENT_ON_RENDER_BOUNDS_CHANGED);
	defCRender.add_static_constant("EVENT_ON_RENDER_BUFFERS_INITIALIZED",pragma::CRenderComponent::EVENT_ON_RENDER_BUFFERS_INITIALIZED);
	defCRender.add_static_constant("EVENT_SHOULD_DRAW",pragma::CRenderComponent::EVENT_SHOULD_DRAW);
	defCRender.add_static_constant("EVENT_SHOULD_DRAW_SHADOW",pragma::CRenderComponent::EVENT_SHOULD_DRAW_SHADOW);
	defCRender.add_static_constant("EVENT_ON_UPDATE_RENDER_MATRICES",pragma::CRenderComponent::EVENT_ON_UPDATE_RENDER_MATRICES);
	defCRender.add_static_constant("EVENT_UPDATE_INSTANTIABILITY",pragma::CRenderComponent::EVENT_UPDATE_INSTANTIABILITY);

	// Enums
	defCRender.add_static_constant("RENDERMODE_NONE",umath::to_integral(RenderMode::None));
	defCRender.add_static_constant("RENDERMODE_AUTO",umath::to_integral(RenderMode::Auto));
	defCRender.add_static_constant("RENDERMODE_WORLD",umath::to_integral(RenderMode::World));
	defCRender.add_static_constant("RENDERMODE_VIEW",umath::to_integral(RenderMode::View));
	defCRender.add_static_constant("RENDERMODE_SKYBOX",umath::to_integral(RenderMode::Skybox));
	defCRender.add_static_constant("RENDERMODE_WATER",umath::to_integral(RenderMode::Water));
	entsMod[defCRender];
}
void Lua::Render::CalcRayIntersection(lua_State *l,CRenderHandle &hComponent,const Vector3 &start,const Vector3 &dir,bool precise)
{
	pragma::Lua::check_component(l,hComponent);
	auto result = hComponent->CalcRayIntersection(start,dir,precise);
	if(result.has_value() == false)
	{
		Lua::PushInt(l,umath::to_integral(Intersection::Result::NoIntersection));
		return;
	}
	Lua::Push(l,umath::to_integral(result->result));

	auto t = Lua::CreateTable(l);

	Lua::PushString(l,"position"); /* 1 */
	Lua::Push<Vector3>(l,result->hitPos); /* 2 */
	Lua::SetTableValue(l,t); /* 0 */

	Lua::PushString(l,"distance"); /* 1 */
	Lua::PushNumber(l,result->hitValue); /* 2 */
	Lua::SetTableValue(l,t); /* 0 */

	if(precise)
	{
		Lua::PushString(l,"uv"); /* 1 */
		Lua::Push<Vector2>(l,Vector2{result->u,result->v}); /* 2 */
		Lua::SetTableValue(l,t); /* 0 */
		return;
	}
	
	Lua::PushString(l,"boneId"); /* 1 */
	Lua::PushInt(l,result->boneId); /* 2 */
	Lua::SetTableValue(l,t); /* 0 */
}
void Lua::Render::CalcRayIntersection(lua_State *l,CRenderHandle &hComponent,const Vector3 &start,const Vector3 &dir) {CalcRayIntersection(l,hComponent,start,dir,false);}
void Lua::Render::GetTransformationMatrix(lua_State *l,CRenderHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Mat4 mat = hEnt->GetTransformationMatrix();
	luabind::object(l,mat).push(l);
}

void Lua::Render::SetRenderMode(lua_State *l,CRenderHandle &hEnt,unsigned int mode)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetRenderMode(RenderMode(mode));
}

void Lua::Render::GetRenderMode(lua_State *l,CRenderHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	RenderMode mode = hEnt->GetRenderMode();
	Lua::PushInt(l,static_cast<int>(mode));
}
void Lua::Render::GetLocalRenderBounds(lua_State *l,CRenderHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	auto &aabb = hEnt->GetLocalRenderBounds();
	Lua::Push<Vector3>(l,aabb.min);
	Lua::Push<Vector3>(l,aabb.max);
}
void Lua::Render::GetLocalRenderSphereBounds(lua_State *l,CRenderHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	auto &sphere = hEnt->GetLocalRenderSphere();
	Lua::Push<Vector3>(l,sphere.pos);
	Lua::PushNumber(l,sphere.radius);
}
void Lua::Render::GetAbsoluteRenderBounds(lua_State *l,CRenderHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	auto &aabb = hEnt->GetUpdatedAbsoluteRenderBounds();
	Lua::Push<Vector3>(l,aabb.min);
	Lua::Push<Vector3>(l,aabb.max);
}
void Lua::Render::GetAbsoluteRenderSphereBounds(lua_State *l,CRenderHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	auto &sphere = hEnt->GetUpdatedAbsoluteRenderSphere();
	Lua::Push<Vector3>(l,sphere.pos);
	Lua::PushNumber(l,sphere.radius);
}

void Lua::Render::SetLocalRenderBounds(lua_State *l,CRenderHandle &hEnt,Vector3 &min,Vector3 &max)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetLocalRenderBounds(min,max);
}

/*void Lua::Render::UpdateRenderBuffers(lua_State *l,CRenderHandle &hEnt,std::shared_ptr<prosper::ICommandBuffer> &drawCmd,CSceneHandle &hScene,CCameraHandle &hCam,bool bForceBufferUpdate)
{
	pragma::Lua::check_component(l,hEnt);
	pragma::Lua::check_component(l,hScene);
	pragma::Lua::check_component(l,hCam);
	if(drawCmd->IsPrimary() == false)
		return;
	auto vp = hCam->GetProjectionMatrix() *hCam->GetViewMatrix();
	hEnt->UpdateRenderData(std::dynamic_pointer_cast<prosper::IPrimaryCommandBuffer>(drawCmd),*hScene,*hCam,vp,bForceBufferUpdate);
}
void Lua::Render::UpdateRenderBuffers(lua_State *l,CRenderHandle &hEnt,std::shared_ptr<prosper::ICommandBuffer> &drawCmd,CSceneHandle &hScene,CCameraHandle &hCam) {UpdateRenderBuffers(l,hEnt,drawCmd,hScene,hCam,false);}*/
void Lua::Render::GetRenderBuffer(lua_State *l,CRenderHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	auto &buf = hEnt->GetRenderBuffer();
	if(buf == nullptr)
		return;
	Lua::Push(l,buf);
}
void Lua::Render::GetBoneBuffer(lua_State *l,CRenderHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	auto *pAnimComponent = static_cast<pragma::CAnimatedComponent*>(hEnt->GetEntity().GetAnimatedComponent().get());
	if(pAnimComponent == nullptr)
		return;
	auto buf = pAnimComponent->GetBoneBuffer();
	if(buf.expired())
		return;
	Lua::Push(l,buf.lock());
}
