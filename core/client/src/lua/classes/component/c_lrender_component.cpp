/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/model/c_modelmesh.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/lua/converters/optional_converter_t.hpp>
#include <pragma/lua/lua_util_component.hpp>
#include <pragma/lua/lua_util_component_stream.hpp>
#include <prosper_command_buffer.hpp>
#include <buffers/prosper_swap_buffer.hpp>
#include <pragma/math/intersection.h>

void Lua::Render::register_class(lua_State *l, luabind::module_ &entsMod)
{
	auto defCRender = pragma::lua::create_entity_component_class<pragma::CRenderComponent, pragma::BaseRenderComponent>("RenderComponent");
	defCRender.def("GetTransformationMatrix", &Lua::Render::GetTransformationMatrix);
	defCRender.def("IsInPvs", static_cast<bool (pragma::CRenderComponent::*)(const Vector3 &, const pragma::CWorldComponent &) const>(&pragma::CRenderComponent::IsInPvs));
	defCRender.def("IsInPvs", static_cast<bool (pragma::CRenderComponent::*)(const Vector3 &) const>(&pragma::CRenderComponent::IsInPvs));
	defCRender.def("IsInRenderGroup", &pragma::CRenderComponent::IsInRenderGroup);
	defCRender.def("GetSceneRenderPass", &pragma::CRenderComponent::GetSceneRenderPass);
	defCRender.def("SetSceneRenderPass", &pragma::CRenderComponent::SetSceneRenderPass);
	defCRender.def(
	  "GetSceneRenderPassProperty", +[](lua_State *l, pragma::CRenderComponent &c) { Lua::Property::push(l, *c.GetSceneRenderPassProperty()); });
	defCRender.def("AddToRenderGroup", static_cast<bool (pragma::CRenderComponent::*)(const std::string &)>(&pragma::CRenderComponent::AddToRenderGroup));
	defCRender.def("AddToRenderGroup", static_cast<void (pragma::CRenderComponent::*)(pragma::rendering::RenderGroup)>(&pragma::CRenderComponent::AddToRenderGroup));
	defCRender.def("RemoveFromRenderGroup", static_cast<bool (pragma::CRenderComponent::*)(const std::string &)>(&pragma::CRenderComponent::RemoveFromRenderGroup));
	defCRender.def("RemoveFromRenderGroup", static_cast<void (pragma::CRenderComponent::*)(pragma::rendering::RenderGroup)>(&pragma::CRenderComponent::RemoveFromRenderGroup));
	defCRender.def("SetRenderGroups", &pragma::CRenderComponent::SetRenderGroups);
	defCRender.def("GetRenderGroups", &pragma::CRenderComponent::GetRenderGroups);
	defCRender.def(
	  "GetRenderGroupsProperty", +[](lua_State *l, pragma::CRenderComponent &c) { Lua::Property::push(l, *c.GetRenderGroupsProperty()); });
	defCRender.def("GetLocalRenderBounds", &Lua::Render::GetLocalRenderBounds);
	defCRender.def("GetLocalRenderSphereBounds", &Lua::Render::GetLocalRenderSphereBounds);
	defCRender.def("GetAbsoluteRenderBounds", &Lua::Render::GetAbsoluteRenderBounds);
	defCRender.def("GetAbsoluteRenderSphereBounds", &Lua::Render::GetAbsoluteRenderSphereBounds);
	defCRender.def("SetLocalRenderBounds", &Lua::Render::SetLocalRenderBounds);
	// defCRender.def("UpdateRenderBuffers",static_cast<void(*)(lua_State*,pragma::CRenderComponent&,std::shared_ptr<prosper::ICommandBuffer>&,CSceneHandle&,CCameraHandle&,bool)>(&Lua::Render::UpdateRenderBuffers));
	// defCRender.def("UpdateRenderBuffers",static_cast<void(*)(lua_State*,pragma::CRenderComponent&,std::shared_ptr<prosper::ICommandBuffer>&,CSceneHandle&,CCameraHandle&)>(&Lua::Render::UpdateRenderBuffers));
	defCRender.def("GetRenderBuffer", &Lua::Render::GetRenderBuffer);
	defCRender.def("GetBoneBuffer", &Lua::Render::GetBoneBuffer);
	defCRender.def("GetLODMeshes", static_cast<void (*)(lua_State *, pragma::CRenderComponent &)>([](lua_State *l, pragma::CRenderComponent &hComponent) {
		auto &lodMeshes = hComponent.GetLODMeshes();
		auto t = Lua::CreateTable(l);
		int32_t idx = 1;
		for(auto &lodMesh : lodMeshes) {
			Lua::PushInt(l, idx++);
			Lua::Push(l, lodMesh);
			Lua::SetTableValue(l, t);
		}
	}));
	defCRender.def("GetRenderMeshes", static_cast<void (*)(lua_State *, pragma::CRenderComponent &)>([](lua_State *l, pragma::CRenderComponent &hComponent) {
		auto &renderMeshes = hComponent.GetRenderMeshes();
		auto t = Lua::CreateTable(l);
		int32_t idx = 1;
		for(auto &mesh : renderMeshes) {
			Lua::PushInt(l, idx++);
			Lua::Push(l, mesh);
			Lua::SetTableValue(l, t);
		}
	}));
	defCRender.def("GetLodRenderMeshes", static_cast<void (*)(lua_State *, pragma::CRenderComponent &, uint32_t)>([](lua_State *l, pragma::CRenderComponent &hComponent, uint32_t lod) {
		auto &renderMeshes = hComponent.GetRenderMeshes();
		auto &renderMeshGroup = hComponent.GetLodRenderMeshGroup(lod);
		auto t = Lua::CreateTable(l);
		int32_t idx = 1;
		for(auto i = renderMeshGroup.first; i < renderMeshGroup.second; ++i) {
			Lua::PushInt(l, idx++);
			Lua::Push(l, renderMeshes[i]);
			Lua::SetTableValue(l, t);
		}
	}));
	defCRender.def("SetExemptFromOcclusionCulling", static_cast<void (*)(lua_State *, pragma::CRenderComponent &, bool)>([](lua_State *l, pragma::CRenderComponent &hComponent, bool exempt) { hComponent.SetExemptFromOcclusionCulling(exempt); }));
	defCRender.def("IsExemptFromOcclusionCulling", static_cast<void (*)(lua_State *, pragma::CRenderComponent &)>([](lua_State *l, pragma::CRenderComponent &hComponent) { Lua::PushBool(l, hComponent.IsExemptFromOcclusionCulling()); }));
	defCRender.def("SetReceiveShadows", static_cast<void (*)(lua_State *, pragma::CRenderComponent &, bool)>([](lua_State *l, pragma::CRenderComponent &hComponent, bool enabled) { hComponent.SetReceiveShadows(enabled); }));
	defCRender.def("IsReceivingShadows", static_cast<void (*)(lua_State *, pragma::CRenderComponent &)>([](lua_State *l, pragma::CRenderComponent &hComponent) { Lua::PushBool(l, hComponent.IsReceivingShadows()); }));
	defCRender.def("SetRenderBufferDirty", static_cast<void (*)(lua_State *, pragma::CRenderComponent &)>([](lua_State *l, pragma::CRenderComponent &hComponent) { hComponent.SetRenderBufferDirty(); }));
	/*defCRender.def("GetDepthBias",static_cast<void(*)(lua_State*,pragma::CRenderComponent&)>([](lua_State *l,pragma::CRenderComponent &hComponent) {
		
		float constantFactor,biasClamp,slopeFactor;
		hComponent.GetDepthBias(constantFactor,biasClamp,slopeFactor);
		Lua::PushNumber(l,constantFactor);
		Lua::PushNumber(l,biasClamp);
		Lua::PushNumber(l,slopeFactor);
	}));
	defCRender.def("SetDepthBias",static_cast<void(*)(lua_State*,pragma::CRenderComponent&,float,float,float)>([](lua_State *l,pragma::CRenderComponent &hComponent,float constantFactor,float biasClamp,float slopeFactor) {
		
		hComponent.SetDepthBias(constantFactor,biasClamp,slopeFactor);
	}));*/
	defCRender.def("CalcRayIntersection", static_cast<void (*)(lua_State *, pragma::CRenderComponent &, const Vector3 &, const Vector3 &, bool)>(&Lua::Render::CalcRayIntersection));
	defCRender.def("CalcRayIntersection", static_cast<void (*)(lua_State *, pragma::CRenderComponent &, const Vector3 &, const Vector3 &)>(&Lua::Render::CalcRayIntersection));
	defCRender.def("SetDepthPassEnabled", static_cast<void (*)(lua_State *, pragma::CRenderComponent &, bool)>([](lua_State *l, pragma::CRenderComponent &hComponent, bool depthPassEnabled) { hComponent.SetDepthPassEnabled(depthPassEnabled); }));
	defCRender.def("IsDepthPassEnabled", static_cast<void (*)(lua_State *, pragma::CRenderComponent &)>([](lua_State *l, pragma::CRenderComponent &hComponent) { Lua::PushBool(l, hComponent.IsDepthPassEnabled()); }));
	defCRender.def("GetRenderClipPlane", static_cast<void (*)(lua_State *, pragma::CRenderComponent &)>([](lua_State *l, pragma::CRenderComponent &hComponent) {
		auto *clipPlane = hComponent.GetRenderClipPlane();
		if(clipPlane == nullptr)
			return;
		Lua::Push(l, *clipPlane);
	}));
	defCRender.def("SetRenderClipPlane", static_cast<void (*)(lua_State *, pragma::CRenderComponent &, const Vector4 &)>([](lua_State *l, pragma::CRenderComponent &hComponent, const Vector4 &clipPlane) { hComponent.SetRenderClipPlane(clipPlane); }));
	defCRender.def("ClearRenderClipPlane", static_cast<void (*)(lua_State *, pragma::CRenderComponent &)>([](lua_State *l, pragma::CRenderComponent &hComponent) { hComponent.ClearRenderClipPlane(); }));
	defCRender.def("GetDepthBias", static_cast<void (*)(lua_State *, pragma::CRenderComponent &)>([](lua_State *l, pragma::CRenderComponent &hComponent) {
		auto *depthBias = hComponent.GetDepthBias();
		if(depthBias == nullptr)
			return;
		Lua::PushNumber(l, depthBias->x);
		Lua::PushNumber(l, depthBias->y);
	}));
	defCRender.def("SetDepthBias", static_cast<void (*)(lua_State *, pragma::CRenderComponent &, float, float)>([](lua_State *l, pragma::CRenderComponent &hComponent, float d, float delta) { hComponent.SetDepthBias(d, delta); }));
	defCRender.def("ClearDepthBias", static_cast<void (*)(lua_State *, pragma::CRenderComponent &)>([](lua_State *l, pragma::CRenderComponent &hComponent) { hComponent.ClearDepthBias(); }));
	defCRender.def("GetRenderPose", static_cast<void (*)(lua_State *, pragma::CRenderComponent &)>([](lua_State *l, pragma::CRenderComponent &hComponent) { Lua::Push(l, hComponent.GetRenderPose()); }));
	defCRender.def("SetRenderOffsetTransform", static_cast<void (*)(lua_State *, pragma::CRenderComponent &, const umath::ScaledTransform &)>([](lua_State *l, pragma::CRenderComponent &hComponent, const umath::ScaledTransform &pose) { hComponent.SetRenderOffsetTransform(pose); }));
	defCRender.def("ClearRenderOffsetTransform", static_cast<void (*)(lua_State *, pragma::CRenderComponent &)>([](lua_State *l, pragma::CRenderComponent &hComponent) { hComponent.ClearRenderOffsetTransform(); }));
	defCRender.def("GetRenderOffsetTransform", static_cast<void (*)(lua_State *, pragma::CRenderComponent &)>([](lua_State *l, pragma::CRenderComponent &hComponent) {
		auto *t = hComponent.GetRenderOffsetTransform();
		if(t == nullptr)
			return;
		Lua::Push(l, *t);
	}));
	defCRender.def("ShouldCastShadows", static_cast<bool (*)(lua_State *, pragma::CRenderComponent &)>([](lua_State *l, pragma::CRenderComponent &hComponent) -> bool { return hComponent.GetCastShadows(); }));
	defCRender.def("ShouldDraw", static_cast<bool (*)(lua_State *, pragma::CRenderComponent &)>([](lua_State *l, pragma::CRenderComponent &hComponent) -> bool { return hComponent.ShouldDraw(); }));
	defCRender.def("ShouldDrawShadow", static_cast<bool (*)(lua_State *, pragma::CRenderComponent &)>([](lua_State *l, pragma::CRenderComponent &hComponent) -> bool { return hComponent.ShouldDrawShadow(); }));
	defCRender.def("ClearBuffers", static_cast<void (*)(lua_State *, pragma::CRenderComponent &)>([](lua_State *l, pragma::CRenderComponent &hComponent) { hComponent.ClearRenderBuffers(); }));
	defCRender.def("SetTranslucencyPassDistanceOverride", &pragma::CRenderComponent::SetTranslucencyPassDistanceOverride);
	defCRender.def("ClearTranslucencyPassDistanceOverride", &pragma::CRenderComponent::ClearTranslucencyPassDistanceOverride);
	defCRender.def("GetTranslucencyPassDistanceOverrideSqr", &pragma::CRenderComponent::GetTranslucencyPassDistanceOverrideSqr);

	defCRender.def("SetHidden", &pragma::CRenderComponent::SetHidden);
	defCRender.def("IsHidden", &pragma::CRenderComponent::IsHidden);
	defCRender.def(
	  "SetVisible", +[](pragma::CRenderComponent &renderC, bool visible) { renderC.SetHidden(!visible); });
	defCRender.def("IsVisible", &pragma::CRenderComponent::IsVisible);
	defCRender.def("SetIgnoreAncestorVisibility", &pragma::CRenderComponent::SetIgnoreAncestorVisibility);
	defCRender.def("ShouldIgnoreAncestorVisibility", &pragma::CRenderComponent::ShouldIgnoreAncestorVisibility);

	// defCRender.add_static_constant("EVENT_ON_UPDATE_RENDER_DATA",pragma::CRenderComponent::EVENT_ON_UPDATE_RENDER_DATA);
	defCRender.add_static_constant("EVENT_ON_RENDER_BOUNDS_CHANGED", pragma::CRenderComponent::EVENT_ON_RENDER_BOUNDS_CHANGED);
	defCRender.add_static_constant("EVENT_ON_RENDER_MODE_CHANGED", pragma::CRenderComponent::EVENT_ON_RENDER_MODE_CHANGED);
	defCRender.add_static_constant("EVENT_ON_RENDER_BUFFERS_INITIALIZED", pragma::CRenderComponent::EVENT_ON_RENDER_BUFFERS_INITIALIZED);
	defCRender.add_static_constant("EVENT_SHOULD_DRAW", pragma::CRenderComponent::EVENT_SHOULD_DRAW);
	defCRender.add_static_constant("EVENT_SHOULD_DRAW_SHADOW", pragma::CRenderComponent::EVENT_SHOULD_DRAW_SHADOW);
	defCRender.add_static_constant("EVENT_ON_UPDATE_RENDER_MATRICES", pragma::CRenderComponent::EVENT_ON_UPDATE_RENDER_MATRICES);
	defCRender.add_static_constant("EVENT_UPDATE_INSTANTIABILITY", pragma::CRenderComponent::EVENT_UPDATE_INSTANTIABILITY);
	defCRender.add_static_constant("EVENT_ON_CLIP_PLANE_CHANGED", pragma::CRenderComponent::EVENT_ON_CLIP_PLANE_CHANGED);
	defCRender.add_static_constant("EVENT_ON_DEPTH_BIAS_CHANGED", pragma::CRenderComponent::EVENT_ON_DEPTH_BIAS_CHANGED);
	entsMod[defCRender];
}
void Lua::Render::CalcRayIntersection(lua_State *l, pragma::CRenderComponent &hComponent, const Vector3 &start, const Vector3 &dir, bool precise)
{

	auto result = hComponent.CalcRayIntersection(start, dir, precise);
	if(result.has_value() == false) {
		Lua::PushInt(l, umath::to_integral(umath::intersection::Result::NoIntersection));
		return;
	}
	Lua::Push(l, umath::to_integral(result->result));

	auto t = Lua::CreateTable(l);

	Lua::PushString(l, "position");        /* 1 */
	Lua::Push<Vector3>(l, result->hitPos); /* 2 */
	Lua::SetTableValue(l, t);              /* 0 */

	Lua::PushString(l, "distance");       /* 1 */
	Lua::PushNumber(l, result->hitValue); /* 2 */
	Lua::SetTableValue(l, t);             /* 0 */

	if(precise && result->precise) {
		Lua::PushString(l, "uv");                                                /* 1 */
		Lua::Push<Vector2>(l, Vector2 {result->precise->u, result->precise->v}); /* 2 */
		Lua::SetTableValue(l, t);                                                /* 0 */
		return;
	}

	Lua::PushString(l, "boneId");    /* 1 */
	Lua::PushInt(l, result->boneId); /* 2 */
	Lua::SetTableValue(l, t);        /* 0 */
}
void Lua::Render::CalcRayIntersection(lua_State *l, pragma::CRenderComponent &hComponent, const Vector3 &start, const Vector3 &dir) { CalcRayIntersection(l, hComponent, start, dir, false); }
void Lua::Render::GetTransformationMatrix(lua_State *l, pragma::CRenderComponent &hEnt)
{

	Mat4 mat = hEnt.GetTransformationMatrix();
	luabind::object(l, mat).push(l);
}

void Lua::Render::GetLocalRenderBounds(lua_State *l, pragma::CRenderComponent &hEnt)
{

	auto &aabb = hEnt.GetLocalRenderBounds();
	Lua::Push<Vector3>(l, aabb.min);
	Lua::Push<Vector3>(l, aabb.max);
}
void Lua::Render::GetLocalRenderSphereBounds(lua_State *l, pragma::CRenderComponent &hEnt)
{

	auto &sphere = hEnt.GetLocalRenderSphere();
	Lua::Push<Vector3>(l, sphere.pos);
	Lua::PushNumber(l, sphere.radius);
}
void Lua::Render::GetAbsoluteRenderBounds(lua_State *l, pragma::CRenderComponent &hEnt)
{

	auto &aabb = hEnt.GetUpdatedAbsoluteRenderBounds();
	Lua::Push<Vector3>(l, aabb.min);
	Lua::Push<Vector3>(l, aabb.max);
}
void Lua::Render::GetAbsoluteRenderSphereBounds(lua_State *l, pragma::CRenderComponent &hEnt)
{

	auto &sphere = hEnt.GetUpdatedAbsoluteRenderSphere();
	Lua::Push<Vector3>(l, sphere.pos);
	Lua::PushNumber(l, sphere.radius);
}

void Lua::Render::SetLocalRenderBounds(lua_State *l, pragma::CRenderComponent &hEnt, Vector3 &min, Vector3 &max) { hEnt.SetLocalRenderBounds(min, max); }

/*void Lua::Render::UpdateRenderBuffers(lua_State *l,pragma::CRenderComponent &hEnt,std::shared_ptr<prosper::ICommandBuffer> &drawCmd,CSceneHandle &hScene,CCameraHandle &hCam,bool bForceBufferUpdate)
{
	
	
	
	if(drawCmd->IsPrimary() == false)
		return;
	auto vp = hCam->GetProjectionMatrix() *hCam->GetViewMatrix();
	hEnt.UpdateRenderData(std::dynamic_pointer_cast<prosper::IPrimaryCommandBuffer>(drawCmd),*hScene,*hCam,vp,bForceBufferUpdate);
}
void Lua::Render::UpdateRenderBuffers(lua_State *l,pragma::CRenderComponent &hEnt,std::shared_ptr<prosper::ICommandBuffer> &drawCmd,CSceneHandle &hScene,CCameraHandle &hCam) {UpdateRenderBuffers(l,hEnt,drawCmd,hScene,hCam,false);}*/
void Lua::Render::GetRenderBuffer(lua_State *l, pragma::CRenderComponent &hEnt)
{
	auto *buf = hEnt.GetRenderBuffer();
	if(buf == nullptr)
		return;
	Lua::Push(l, buf->shared_from_this());
}
void Lua::Render::GetBoneBuffer(lua_State *l, pragma::CRenderComponent &hEnt)
{

	auto *pAnimComponent = static_cast<pragma::CAnimatedComponent *>(hEnt.GetEntity().GetAnimatedComponent().get());
	if(pAnimComponent == nullptr)
		return;
	auto *buf = pAnimComponent->GetBoneBuffer();
	if(!buf)
		return;
	Lua::Push(l, buf->shared_from_this());
}
