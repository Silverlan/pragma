/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include "pragma/lua/classes/lproperty.hpp"
#include "pragma/lua/classes/c_lcamera.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/renderers/raytracing_renderer.hpp"
#include "pragma/rendering/render_queue.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/entities/util/c_util_pbr_converter.hpp"
#include <pragma/lua/classes/lproperty_generic.hpp>
#include <pragma/lua/classes/ldef_vector.h>
#include <pragma/lua/classes/ldef_color.h>
#include <pragma/lua/classes/ldef_angle.h>
#include <pragma/lua/classes/ldef_quaternion.h>
#include <pragma/lua/policies/game_object_policy.hpp>
#include <pragma/lua/policies/optional_policy.hpp>
#include <pragma/lua/policies/shared_from_this_policy.hpp>
#include <pragma/lua/policies/pair_policy.hpp>
#include <pragma/model/model.h>
#include <pragma/physics/raytraces.h>
#include <pragma/lua/lentity_components_base_types.hpp>
#include <pragma/lua/lentity_components.hpp>
#include <pragma/lua/lua_entity_component.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_render_pass.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <pragma/physics/movetypes.h>
#include <pragma/lua/lua_call.hpp>
#include <luabind/copy_policy.hpp>
#include <image/prosper_render_target.hpp>
#include "pragma/entities/components/c_ai_component.hpp"
#include "pragma/entities/components/c_character_component.hpp"
#include "pragma/entities/components/c_color_component.hpp"
#include "pragma/entities/components/c_score_component.hpp"
#include "pragma/entities/components/c_flammable_component.hpp"
#include "pragma/entities/components/c_health_component.hpp"
#include "pragma/entities/components/c_name_component.hpp"
#include "pragma/entities/components/c_networked_component.hpp"
#include "pragma/entities/components/c_observable_component.hpp"
#include "pragma/entities/components/c_physics_component.hpp"
#include "pragma/entities/components/c_radius_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_sound_emitter_component.hpp"
#include "pragma/entities/components/c_toggle_component.hpp"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/entities/components/c_vehicle_component.hpp"
#include "pragma/entities/components/c_weapon_component.hpp"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/components/c_raytracing_component.hpp"
#include "pragma/entities/environment/audio/c_env_sound_dsp.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp_chorus.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp_distortion.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp_eaxreverb.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp_echo.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp_equalizer.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp_flanger.h"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/entities/environment/effects/c_env_explosion.h"
#include "pragma/entities/environment/effects/c_env_fire.h"
#include "pragma/entities/environment/c_env_decal.h"
#include "pragma/entities/environment/c_env_fog_controller.h"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/entities/environment/lights/c_env_light_directional.h"
#include "pragma/entities/environment/lights/c_env_light_point.h"
#include "pragma/entities/environment/lights/c_env_light_spot.h"
#include "pragma/entities/environment/lights/c_env_light_spot_vol.h"
#include "pragma/entities/environment/c_env_microphone.h"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include "pragma/entities/environment/c_env_quake.h"
#include "pragma/entities/environment/effects/c_env_smoke_trail.h"
#include "pragma/entities/environment/audio/c_env_sound.h"
#include "pragma/entities/environment/audio/c_env_soundscape.h"
#include "pragma/entities/environment/effects/c_env_sprite.h"
#include "pragma/entities/environment/c_env_weather.h"
#include "pragma/entities/environment/c_sky_camera.hpp"
#include "pragma/entities/environment/lights/c_env_shadow_csm.hpp"
#include "pragma/entities/components/c_time_scale_component.hpp"
#include "pragma/entities/environment/c_env_wind.hpp"
#include "pragma/entities/func/c_func_brush.h"
#include "pragma/entities/func/c_func_kinematic.hpp"
#include "pragma/entities/func/c_func_physics.h"
#include "pragma/entities/func/c_func_softphysics.hpp"
#include "pragma/entities/func/c_func_portal.h"
#include "pragma/entities/func/c_func_water.h"
#include "pragma/entities/func/c_funcbutton.h"
#include "pragma/entities/game/c_game_occlusion_culler.hpp"
#include "pragma/entities/c_bot.h"
#include "pragma/entities/c_listener.h"
#include "pragma/entities/c_viewbody.h"
#include "pragma/entities/c_viewmodel.h"
#include "pragma/entities/point/constraints/c_point_constraint_ballsocket.h"
#include "pragma/entities/point/constraints/c_point_constraint_dof.h"
#include "pragma/entities/point/constraints/c_point_constraint_conetwist.h"
#include "pragma/entities/point/constraints/c_point_constraint_fixed.h"
#include "pragma/entities/point/constraints/c_point_constraint_hinge.h"
#include "pragma/entities/point/constraints/c_point_constraint_slider.h"
#include "pragma/entities/point/c_point_rendertarget.h"
#include "pragma/entities/point/c_point_target.h"
#include "pragma/entities/prop/c_prop_base.hpp"
#include "pragma/entities/prop/c_prop_dynamic.hpp"
#include "pragma/entities/prop/c_prop_physics.h"
#include "pragma/entities/trigger/c_trigger_touch.h"
#include "pragma/entities/c_skybox.h"
#include "pragma/entities/c_flashlight.h"
#include "pragma/entities/components/c_shooter_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/entities/components/c_entity_component.hpp"
#include "pragma/entities/components/c_io_component.hpp"
#include "pragma/entities/components/c_time_scale_component.hpp"
#include "pragma/entities/components/c_attachable_component.hpp"
#include "pragma/entities/components/c_parent_component.hpp"
#include "pragma/entities/components/c_generic_component.hpp"
#include "pragma/entities/components/c_point_at_target_component.hpp"
#include "pragma/entities/components/c_gamemode_component.hpp"
#include "pragma/entities/environment/c_env_reflection_probe.hpp"
#include "pragma/entities/c_wheel.hpp"
#include "pragma/entities/environment/c_env_timescale.h"
#include "pragma/entities/util/c_util_pbr_converter.hpp"
#include "pragma/entities/environment/lights/c_env_shadow.hpp"
#include "pragma/entities/environment/audio/c_env_sound_probe.hpp"
#include "pragma/entities/components/c_weapon_component.hpp"
#include "pragma/entities/c_water_surface.hpp"
#include "pragma/entities/components/c_flex_component.hpp"
#include "pragma/entities/components/c_softbody_component.hpp"
#include "pragma/entities/components/c_vertex_animated_component.hpp"
#include "pragma/entities/components/c_debug_component.hpp"
#include "pragma/entities/components/c_bsp_leaf_component.hpp"
#include "pragma/entities/components/renderers/c_raytracing_renderer_component.hpp"
#include "pragma/entities/components/c_eye_component.hpp"
#include "pragma/entities/components/c_bsp_component.hpp"
#include "pragma/entities/components/c_light_map_component.hpp"
#include "pragma/entities/components/c_light_map_receiver_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/entities/c_filter_entity_class.h"
#include "pragma/entities/c_filter_entity_name.h"
#include "pragma/entities/components/c_ownable_component.hpp"

namespace Lua
{
	namespace PBRConverter
	{
		static void GenerateAmbientOcclusionMaps(lua_State *l,pragma::CPBRConverterComponent &hComponent,Model &mdl,uint32_t width,uint32_t height,uint32_t samples,bool rebuild)
		{
			hComponent.GenerateAmbientOcclusionMaps(mdl,width,height,samples,rebuild);
		}
		static void GenerateAmbientOcclusionMaps(lua_State *l,pragma::CPBRConverterComponent &hComponent,Model &mdl,uint32_t width,uint32_t height,uint32_t samples)
		{
			hComponent.GenerateAmbientOcclusionMaps(mdl,width,height,samples);
		}
		static void GenerateAmbientOcclusionMaps(lua_State *l,pragma::CPBRConverterComponent &hComponent,Model &mdl,uint32_t width,uint32_t height)
		{
			hComponent.GenerateAmbientOcclusionMaps(mdl,width,height);
		}
		static void GenerateAmbientOcclusionMaps(lua_State *l,pragma::CPBRConverterComponent &hComponent,Model &mdl)
		{
			hComponent.GenerateAmbientOcclusionMaps(mdl);
		}
		
		static void GenerateAmbientOcclusionMaps(lua_State *l,pragma::CPBRConverterComponent &hComponent,BaseEntity &ent,uint32_t width,uint32_t height,uint32_t samples,bool rebuild)
		{
			hComponent.GenerateAmbientOcclusionMaps(ent,width,height,samples,rebuild);
		}
		static void GenerateAmbientOcclusionMaps(lua_State *l,pragma::CPBRConverterComponent &hComponent,BaseEntity &ent,uint32_t width,uint32_t height,uint32_t samples)
		{
			hComponent.GenerateAmbientOcclusionMaps(ent,width,height,samples);
		}
		static void GenerateAmbientOcclusionMaps(lua_State *l,pragma::CPBRConverterComponent &hComponent,BaseEntity &ent,uint32_t width,uint32_t height)
		{
			hComponent.GenerateAmbientOcclusionMaps(ent,width,height);
		}
		static void GenerateAmbientOcclusionMaps(lua_State *l,pragma::CPBRConverterComponent &hComponent,BaseEntity &ent)
		{
			hComponent.GenerateAmbientOcclusionMaps(ent);
		}
	};
	namespace ParticleSystem
	{
		static std::string get_key_value(lua_State *l,int32_t argIdx)
		{
			if(Lua::IsNumber(l,argIdx))
				return std::to_string(Lua::CheckNumber(l,argIdx));
			if(Lua::IsBool(l,argIdx))
				return Lua::CheckBool(l,argIdx) ? "1" : "0";
			if(Lua::IsVector4(l,argIdx))
			{
				auto &v = *Lua::CheckVector4(l,argIdx);
				return std::to_string(v.x) +" " +std::to_string(v.y) +" " +std::to_string(v.z) +" " +std::to_string(v.w);
			}
			if(Lua::IsVector(l,argIdx))
			{
				auto &v = *Lua::CheckVector(l,argIdx);
				return std::to_string(v.x) +" " +std::to_string(v.y) +" " +std::to_string(v.z);
			}
			if(Lua::IsVector2(l,argIdx))
			{
				auto &v = *Lua::CheckVector2(l,argIdx);
				return std::to_string(v.x) +" " +std::to_string(v.y);
			}
			if(Lua::IsVector4i(l,argIdx))
			{
				auto &v = *Lua::CheckVector4i(l,argIdx);
				return std::to_string(v.x) +" " +std::to_string(v.y) +" " +std::to_string(v.z) +" " +std::to_string(v.w);
			}
			if(Lua::IsVectori(l,argIdx))
			{
				auto &v = *Lua::CheckVectori(l,argIdx);
				return std::to_string(v.x) +" " +std::to_string(v.y) +" " +std::to_string(v.z);
			}
			if(Lua::IsVector2i(l,argIdx))
			{
				auto &v = *Lua::CheckVector2i(l,argIdx);
				return std::to_string(v.x) +" " +std::to_string(v.y);
			}
			if(Lua::IsColor(l,argIdx))
			{
				auto &c = *Lua::CheckColor(l,argIdx);
				return std::to_string(c.r) +" " +std::to_string(c.g) +" " +std::to_string(c.b) +" " +std::to_string(c.a);
			}
			if(Lua::IsEulerAngles(l,argIdx))
			{
				auto &ang = *Lua::CheckEulerAngles(l,argIdx);
				return std::to_string(ang.p) +" " +std::to_string(ang.y) +" " +std::to_string(ang.r);
			}
			if(Lua::IsQuaternion(l,argIdx))
			{
				auto &rot = *Lua::CheckQuaternion(l,argIdx);
				return std::to_string(rot.w) +" " +std::to_string(rot.x) +" " +std::to_string(rot.y) +" " +std::to_string(rot.z);
			}
			return Lua::CheckString(l,argIdx);
		}
	};
	namespace Decal
	{
		static void create_from_projection(lua_State *l,pragma::CDecalComponent &hComponent,luabind::object tMeshes,const umath::ScaledTransform &pose)
		{
			
			int32_t t = 2;
			Lua::CheckTable(l,t);
			std::vector<pragma::DecalProjector::MeshData> meshesDatas {};
			auto numMeshes = Lua::GetObjectLength(l,t);
			meshesDatas.reserve(numMeshes);
			for(auto i=decltype(numMeshes){0u};i<numMeshes;++i)
			{
				meshesDatas.push_back({});
				auto &meshData = meshesDatas.back();
				Lua::PushInt(l,i +1); /* 1 */
				Lua::GetTableValue(l,t); /* 1 */

				auto tMeshData = Lua::GetStackTop(l);
				Lua::CheckTable(l,tMeshData);

				Lua::PushString(l,"pose"); /* 2 */
				Lua::GetTableValue(l,tMeshData); /* 2 */
				meshData.pose = Lua::Check<umath::ScaledTransform>(l,-1);
				Lua::Pop(l,1); /* 1 */
				
				Lua::PushString(l,"subMeshes"); /* 2 */
				Lua::GetTableValue(l,tMeshData); /* 2 */
				auto tSubMeshes = Lua::GetStackTop(l);
				Lua::CheckTable(l,tSubMeshes);
				auto numSubMeshes = Lua::GetObjectLength(l,tSubMeshes);
				meshData.subMeshes.reserve(numSubMeshes);
				for(auto j=decltype(numSubMeshes){0u};j<numSubMeshes;++j)
				{
					Lua::PushInt(l,j +1); /* 1 */
					Lua::GetTableValue(l,tSubMeshes); /* 1 */
					auto &mesh = Lua::Check<ModelSubMesh>(l,-1);
					meshData.subMeshes.push_back(&mesh);
					Lua::Pop(l,1); /* 0 */
				}
				Lua::Pop(l,1); /* 1 */

				Lua::Pop(l,1); /* 0 */
			}
			Lua::PushBool(l,hComponent.ApplyDecal(meshesDatas));
		}
		static void create_from_projection(lua_State *l,pragma::CDecalComponent &hComponent,luabind::object tMeshes)
		{
			create_from_projection(l,hComponent,tMeshes,{});
		}
	};
};

static bool reflection_probe_capture_ibl_reflections_from_scene(lua_State *l,pragma::CReflectionProbeComponent &hRp,luabind::table<> tEnts,bool renderJob)
{
	
	std::vector<BaseEntity*> ents {};
	ents.reserve(Lua::GetObjectLength(l,2));
	for(auto it=luabind::iterator{tEnts},end=luabind::iterator{};it!=end;++it)
	{
		auto val = luabind::object_cast_nothrow<EntityHandle>(*it,EntityHandle{});
		if(val.expired())
			return false;
		ents.push_back(val.get());
	}
	return hRp.CaptureIBLReflectionsFromScene(&ents,renderJob);
}
static bool reflection_probe_capture_ibl_reflections_from_scene(lua_State *l,pragma::CReflectionProbeComponent &hRp,luabind::table<> tEnts)
{
	return reflection_probe_capture_ibl_reflections_from_scene(l,hRp,tEnts,false);
}
static bool reflection_probe_capture_ibl_reflections_from_scene(lua_State *l,pragma::CReflectionProbeComponent &hRp,bool renderJob)
{
	
	return hRp.CaptureIBLReflectionsFromScene(nullptr,renderJob);
}
static bool reflection_probe_capture_ibl_reflections_from_scene(lua_State *l,pragma::CReflectionProbeComponent &hRp)
{
	
	return hRp.CaptureIBLReflectionsFromScene();
}

static void register_renderer_bindings(luabind::module_ &entsMod)
{
	auto defRenderer = luabind::class_<pragma::CRendererComponent,pragma::BaseEntityComponent>("RendererComponent");
	defRenderer.def("GetWidth",&pragma::CRendererComponent::GetWidth);
	defRenderer.def("GetHeight",&pragma::CRendererComponent::GetHeight);
	defRenderer.def("InitializeRenderTarget", static_cast<void(*)(lua_State*,pragma::CRendererComponent&,pragma::CSceneComponent&,uint32_t,uint32_t,bool)>([](lua_State *l,pragma::CRendererComponent &renderer,pragma::CSceneComponent &scene,uint32_t width,uint32_t height,bool reload) {
		
		
		if(reload == false && width == renderer.GetWidth() && height == renderer.GetHeight())
			return;
		renderer.ReloadRenderTarget(scene,width,height);
	}));
	defRenderer.def("InitializeRenderTarget", static_cast<void(*)(lua_State*,pragma::CRendererComponent&,pragma::CSceneComponent&,uint32_t,uint32_t)>([](lua_State *l,pragma::CRendererComponent &renderer,pragma::CSceneComponent &scene,uint32_t width,uint32_t height) {
		
		
		if(width == renderer.GetWidth() && height == renderer.GetHeight())
			return;
		renderer.ReloadRenderTarget(scene,width,height);
	}));
	defRenderer.def("GetSceneTexture",&pragma::CRendererComponent::GetSceneTexture,luabind::shared_from_this_policy<0>{});
	defRenderer.def("GetPresentationTexture",&pragma::CRendererComponent::GetPresentationTexture,luabind::shared_from_this_policy<0>{});
	defRenderer.def("GetHDRPresentationTexture",&pragma::CRendererComponent::GetHDRPresentationTexture,luabind::shared_from_this_policy<0>{});
	entsMod[defRenderer];

	auto defRaster = luabind::class_<pragma::CRasterizationRendererComponent,pragma::BaseEntityComponent>("RasterizationRendererComponent");
	defRaster.add_static_constant("EVENT_ON_RECORD_PREPASS",pragma::CRasterizationRendererComponent::EVENT_ON_RECORD_PREPASS);
	defRaster.add_static_constant("EVENT_ON_RECORD_LIGHTING_PASS",pragma::CRasterizationRendererComponent::EVENT_ON_RECORD_LIGHTING_PASS);
	defRaster.add_static_constant("EVENT_PRE_EXECUTE_PREPASS",pragma::CRasterizationRendererComponent::EVENT_PRE_EXECUTE_PREPASS);
	defRaster.add_static_constant("EVENT_POST_EXECUTE_PREPASS",pragma::CRasterizationRendererComponent::EVENT_POST_EXECUTE_PREPASS);
	defRaster.add_static_constant("EVENT_PRE_EXECUTE_LIGHTING_PASS",pragma::CRasterizationRendererComponent::EVENT_PRE_EXECUTE_LIGHTING_PASS);
	defRaster.add_static_constant("EVENT_POST_EXECUTE_LIGHTING_PASS",pragma::CRasterizationRendererComponent::EVENT_POST_EXECUTE_LIGHTING_PASS);
	defRaster.add_static_constant("EVENT_PRE_PREPASS",pragma::CRasterizationRendererComponent::EVENT_PRE_PREPASS);
	defRaster.add_static_constant("EVENT_POST_PREPASS",pragma::CRasterizationRendererComponent::EVENT_POST_PREPASS);
	defRaster.add_static_constant("EVENT_PRE_LIGHTING_PASS",pragma::CRasterizationRendererComponent::EVENT_PRE_LIGHTING_PASS);
	defRaster.add_static_constant("EVENT_POST_LIGHTING_PASS",pragma::CRasterizationRendererComponent::EVENT_POST_LIGHTING_PASS);
	defRaster.def("GetPrepassDepthTexture",&Lua::RasterizationRenderer::GetPrepassDepthTexture);
	defRaster.def("GetPrepassNormalTexture",&Lua::RasterizationRenderer::GetPrepassNormalTexture);
	defRaster.def("GetRenderTarget",&Lua::RasterizationRenderer::GetRenderTarget);
	defRaster.def("BeginRenderPass",static_cast<void(*)(lua_State*,pragma::CRasterizationRendererComponent&,const ::util::DrawSceneInfo&,prosper::IRenderPass&)>(&Lua::RasterizationRenderer::BeginRenderPass));
	defRaster.def("BeginRenderPass",static_cast<void(*)(lua_State*,pragma::CRasterizationRendererComponent&,const ::util::DrawSceneInfo&)>(&Lua::RasterizationRenderer::BeginRenderPass));
	defRaster.def("EndRenderPass",&pragma::CRasterizationRendererComponent::EndRenderPass);
	defRaster.def("GetPrepassShader",&Lua::RasterizationRenderer::GetPrepassShader);
	defRaster.def("SetShaderOverride",&pragma::CRasterizationRendererComponent::SetShaderOverride);
	defRaster.def("ClearShaderOverride",&pragma::CRasterizationRendererComponent::ClearShaderOverride);
	defRaster.def("SetPrepassMode",&pragma::CRasterizationRendererComponent::SetPrepassMode);
	defRaster.def("GetPrepassMode",&pragma::CRasterizationRendererComponent::GetPrepassMode);
	defRaster.def("SetSSAOEnabled",&pragma::CRasterizationRendererComponent::SetSSAOEnabled);
	defRaster.def("IsSSAOEnabled", &pragma::CRasterizationRendererComponent::IsSSAOEnabled);
	defRaster.def("GetLightSourceDescriptorSet", static_cast<void(*)(lua_State*, pragma::CRasterizationRendererComponent&)>([](lua_State *l,pragma::CRasterizationRendererComponent &renderer) {
		
		auto *ds = pragma::CShadowManagerComponent::GetShadowManager()->GetDescriptorSet();
		if(ds == nullptr)
			return;
		Lua::Push(l,ds->GetDescriptorSetGroup().shared_from_this());
	}));
	defRaster.def("GetPostPrepassDepthTexture", static_cast<void(*)(lua_State*, pragma::CRasterizationRendererComponent&)>([](lua_State *l,pragma::CRasterizationRendererComponent &renderer) {
		
		auto &depthTex = renderer.GetPrepass().textureDepth;
		if (depthTex == nullptr)
			return;
		Lua::Push(l,depthTex);
	}));
	defRaster.def("GetPostProcessingDepthDescriptorSet", static_cast<void(*)(lua_State*, pragma::CRasterizationRendererComponent&)>([](lua_State *l,pragma::CRasterizationRendererComponent &renderer) {
		
		auto &depthTex = renderer.GetHDRInfo().dsgDepthPostProcessing;
		if (depthTex == nullptr)
			return;
		Lua::Push(l,depthTex);
	}));
	defRaster.def("GetPostProcessingHDRColorDescriptorSet",static_cast<void(*)(lua_State*,pragma::CRasterizationRendererComponent&)>([](lua_State *l,pragma::CRasterizationRendererComponent &renderer) {
		
		auto &dsg = renderer.GetHDRInfo().dsgHDRPostProcessing;
		if(dsg == nullptr)
			return;
		Lua::Push(l,dsg);
	}));
#if 0
	defRaster.def("GetStagingRenderTarget",static_cast<void(*)(lua_State*,pragma::CRasterizationRendererComponent&)>([](lua_State *l,pragma::CRasterizationRendererComponent &renderer) {
		
		auto &rt = renderer.GetHDRInfo().hdrPostProcessingRenderTarget;
		if(rt == nullptr)
			return;
		Lua::Push(l,rt);
		}));
	defRaster.def("BlitStagingRenderTargetToMainRenderTarget",static_cast<void(*)(lua_State*,pragma::CRasterizationRendererComponent&,const util::DrawSceneInfo&)>([](lua_State *l,pragma::CRasterizationRendererComponent &renderer,const util::DrawSceneInfo &drawSceneInfo) {
		
		renderer.GetHDRInfo().BlitStagingRenderTargetToMainRenderTarget(drawSceneInfo);
	}));
#endif
	defRaster.def("GetBloomTexture",static_cast<void(*)(lua_State*,pragma::CRasterizationRendererComponent&)>([](lua_State *l,pragma::CRasterizationRendererComponent &renderer) {
		
		auto &rt = renderer.GetHDRInfo().bloomBlurRenderTarget;
		if(rt == nullptr)
			return;
		Lua::Push(l,rt->GetTexture().shared_from_this());
	}));
#if 0
	defRaster.def("GetGlowTexture",static_cast<void(*)(lua_State*,pragma::CRasterizationRendererComponent&)>([](lua_State *l,pragma::CRasterizationRendererComponent &renderer) {
		
		auto &rt = renderer.GetGlowInfo().renderTarget;
		if(rt == nullptr)
			return;
		Lua::Push(l,rt->GetTexture().shared_from_this());
	}));
#endif
	defRaster.def("GetRenderTargetTextureDescriptorSet",static_cast<void(*)(lua_State*,pragma::CRasterizationRendererComponent&)>([](lua_State *l,pragma::CRasterizationRendererComponent &renderer) {
		
		auto &dsg = renderer.GetHDRInfo().dsgHDRPostProcessing;
		if(dsg == nullptr)
			return;
		Lua::Push(l,dsg);
	}));
	defRaster.def("ReloadPresentationRenderTarget",&pragma::CRasterizationRendererComponent::ReloadPresentationRenderTarget);
	defRaster.def("ScheduleMeshForRendering",static_cast<void(*)(
		lua_State*,pragma::CRasterizationRendererComponent&,pragma::CSceneComponent&,uint32_t,pragma::ShaderGameWorldLightingPass&,Material&,BaseEntity&,ModelSubMesh&
	)>(&Lua::RasterizationRenderer::ScheduleMeshForRendering));
	defRaster.def("ScheduleMeshForRendering",static_cast<void(*)(
		lua_State*,pragma::CRasterizationRendererComponent&,pragma::CSceneComponent&,uint32_t,const std::string&,Material&,BaseEntity&,ModelSubMesh&
	)>(&Lua::RasterizationRenderer::ScheduleMeshForRendering));
	defRaster.def("ScheduleMeshForRendering",static_cast<void(*)(
		lua_State*,pragma::CRasterizationRendererComponent&,pragma::CSceneComponent&,uint32_t,::Material&,BaseEntity&,ModelSubMesh&
	)>(&Lua::RasterizationRenderer::ScheduleMeshForRendering));
	defRaster.def("RecordPrepass",&pragma::CRasterizationRendererComponent::RecordPrepass);
	defRaster.def("RecordLightingPass",&pragma::CRasterizationRendererComponent::RecordLightingPass);
	defRaster.def("ExecutePrepass",&pragma::CRasterizationRendererComponent::ExecutePrepass);
	defRaster.def("ExecuteLightingPass",&pragma::CRasterizationRendererComponent::ExecuteLightingPass);
	defRaster.def("GetPrepassCommandBufferRecorder",&pragma::CRasterizationRendererComponent::GetPrepassCommandBufferRecorder,luabind::copy_policy<0>{});
	defRaster.def("GetShadowCommandBufferRecorder",&pragma::CRasterizationRendererComponent::GetShadowCommandBufferRecorder,luabind::copy_policy<0>{});
	defRaster.def("GetLightingPassCommandBufferRecorder",&pragma::CRasterizationRendererComponent::GetLightingPassCommandBufferRecorder,luabind::copy_policy<0>{});
	defRaster.def("UpdatePrepassRenderBuffers",&pragma::CRasterizationRendererComponent::UpdatePrepassRenderBuffers);
	defRaster.def("UpdateLightingPassRenderBuffers",&pragma::CRasterizationRendererComponent::UpdateLightingPassRenderBuffers);
	defRaster.add_static_constant("PREPASS_MODE_DISABLED",umath::to_integral(pragma::CRasterizationRendererComponent::PrepassMode::NoPrepass));
	defRaster.add_static_constant("PREPASS_MODE_DEPTH_ONLY",umath::to_integral(pragma::CRasterizationRendererComponent::PrepassMode::DepthOnly));
	defRaster.add_static_constant("PREPASS_MODE_EXTENDED",umath::to_integral(pragma::CRasterizationRendererComponent::PrepassMode::Extended));
	entsMod[defRaster];

	auto defRaytracing = luabind::class_<pragma::CRaytracingComponent,pragma::BaseEntityComponent>("RaytracingRendererComponent");
	entsMod[defRaytracing];
}

void CGame::RegisterLuaEntityComponents(luabind::module_ &entsMod)
{
	Game::RegisterLuaEntityComponents(entsMod);
	auto *l = GetLuaState();
	Lua::register_cl_ai_component(l,entsMod);
	Lua::register_cl_character_component(l,entsMod);
	Lua::register_cl_player_component(l,entsMod);
	Lua::register_cl_vehicle_component(l,entsMod);
	Lua::register_cl_weapon_component(l,entsMod);

	auto defCGamemode = luabind::class_<pragma::CGamemodeComponent,pragma::BaseGamemodeComponent>("GamemodeComponent");
	entsMod[defCGamemode];

	auto defCColor = luabind::class_<pragma::CColorComponent,pragma::BaseColorComponent>("ColorComponent");
	entsMod[defCColor];

	auto defCScore = luabind::class_<pragma::CScoreComponent,pragma::BaseScoreComponent>("ScoreComponent");
	entsMod[defCScore];

	auto defCFlammable = luabind::class_<pragma::CFlammableComponent,pragma::BaseFlammableComponent>("FlammableComponent");
	entsMod[defCFlammable];

	auto defCHealth = luabind::class_<pragma::CHealthComponent,pragma::BaseHealthComponent>("HealthComponent");
	entsMod[defCHealth];

	auto defCName = luabind::class_<pragma::CNameComponent,pragma::BaseNameComponent>("NameComponent");
	entsMod[defCName];

	auto defCNetworked = luabind::class_<pragma::CNetworkedComponent,pragma::BaseNetworkedComponent>("NetworkedComponent");
	entsMod[defCNetworked];

	auto defCObservable = luabind::class_<pragma::CObservableComponent,pragma::BaseObservableComponent>("ObservableComponent");
	entsMod[defCObservable];

	auto defCShooter = luabind::class_<pragma::CShooterComponent,pragma::BaseShooterComponent>("ShooterComponent");
	entsMod[defCShooter];

	auto defCPhysics = luabind::class_<pragma::CPhysicsComponent,pragma::BasePhysicsComponent>("PhysicsComponent");
	entsMod[defCPhysics];

	auto defCRadius = luabind::class_<pragma::CRadiusComponent,pragma::BaseRadiusComponent>("RadiusComponent");
	entsMod[defCRadius];

	auto defCWorld = luabind::class_<pragma::CWorldComponent,pragma::BaseWorldComponent>("WorldComponent");
	defCWorld.def("GetBSPTree",&pragma::CWorldComponent::GetBSPTree,luabind::optional_policy<0>{});

	auto defCEye = luabind::class_<pragma::CEyeComponent,pragma::BaseEntityComponent>("EyeComponent");
	defCEye.def("GetEyePose",&pragma::CEyeComponent::GetEyePose,luabind::optional_policy<0>{});
	defCEye.def("GetViewTarget",&pragma::CEyeComponent::GetViewTarget);
	defCEye.def("SetViewTarget",&pragma::CEyeComponent::SetViewTarget);
	defCEye.def("ClearViewTarget",&pragma::CEyeComponent::ClearViewTarget);
	defCEye.def("GetEyeShift",static_cast<void(*)(lua_State*,pragma::CEyeComponent&,uint32_t)>([](lua_State *l,pragma::CEyeComponent &hEye,uint32_t eyeIndex) {
		
		auto *config = hEye.GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return;
		Lua::Push<Vector3>(l,config->eyeShift);
		}));
	defCEye.def("SetEyeShift",static_cast<void(*)(lua_State*,pragma::CEyeComponent&,uint32_t,const Vector3&)>([](lua_State *l,pragma::CEyeComponent &hEye,uint32_t eyeIndex,const Vector3 &eyeShift) {
		
		auto *config = hEye.GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return;
		config->eyeShift = eyeShift;
		}));
	defCEye.def("GetEyeJitter",static_cast<void(*)(lua_State*,pragma::CEyeComponent&,uint32_t)>([](lua_State *l,pragma::CEyeComponent &hEye,uint32_t eyeIndex) {
		
		auto *config = hEye.GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return;
		Lua::Push<Vector2>(l,config->jitter);
		}));
	defCEye.def("SetEyeJitter",static_cast<void(*)(lua_State*,pragma::CEyeComponent&,uint32_t,const Vector2&)>([](lua_State *l,pragma::CEyeComponent &hEye,uint32_t eyeIndex,const Vector2 &eyeJitter) {
		
		auto *config = hEye.GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return;
		config->jitter = eyeJitter;
		}));
	defCEye.def("GetEyeSize",static_cast<void(*)(lua_State*,pragma::CEyeComponent&,uint32_t)>([](lua_State *l,pragma::CEyeComponent &hEye,uint32_t eyeIndex) {
		
		auto *config = hEye.GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return;
		Lua::PushNumber(l,config->eyeSize);
		}));
	defCEye.def("SetEyeSize",static_cast<void(*)(lua_State*,pragma::CEyeComponent&,uint32_t,float)>([](lua_State *l,pragma::CEyeComponent &hEye,uint32_t eyeIndex,float eyeSize) {
		
		auto *config = hEye.GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return;
		config->eyeSize = eyeSize;
		}));
	defCEye.def("SetIrisDilation",static_cast<void(*)(lua_State*,pragma::CEyeComponent&,uint32_t,float)>([](lua_State *l,pragma::CEyeComponent &hEye,uint32_t eyeIndex,float dilation) {
		
		auto *config = hEye.GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return;
		config->dilation = dilation;
		}));
	defCEye.def("GetIrisDilation",static_cast<void(*)(lua_State*,pragma::CEyeComponent&,uint32_t)>([](lua_State *l,pragma::CEyeComponent &hEye,uint32_t eyeIndex) {
		
		auto *config = hEye.GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return;
		Lua::PushNumber(l,config->dilation);
		}));
	defCEye.def("CalcEyeballPose",static_cast<void(*)(lua_State*,pragma::CEyeComponent&,uint32_t)>([](lua_State *l,pragma::CEyeComponent &hEye,uint32_t eyeIndex) {
		
		umath::Transform bonePose;
		auto pose = hEye.CalcEyeballPose(eyeIndex,&bonePose);
		Lua::Push(l,pose);
		Lua::Push(l,bonePose);
	}));
	defCEye.def("GetEyeballState",static_cast<void(*)(lua_State*,pragma::CEyeComponent&,uint32_t)>([](lua_State *l,pragma::CEyeComponent &hEye,uint32_t eyeIndex) {
		
		auto *eyeballData = hEye.GetEyeballData(eyeIndex);
		if(eyeballData == nullptr)
			return;
		auto &eyeballState = eyeballData->state;
		Lua::Push<pragma::CEyeComponent::EyeballState*>(l,&eyeballState);
		}));
	defCEye.def("SetBlinkDuration",&pragma::CEyeComponent::SetBlinkDuration);
	defCEye.def("GetBlinkDuration",&pragma::CEyeComponent::GetBlinkDuration);
	defCEye.def("SetBlinkingEnabled",&pragma::CEyeComponent::SetBlinkingEnabled);
	defCEye.def("IsBlinkingEnabled",&pragma::CEyeComponent::IsBlinkingEnabled);

	auto defEyeballState = luabind::class_<pragma::CEyeComponent::EyeballState>("EyeballState");
	defEyeballState.def_readwrite("origin",&pragma::CEyeComponent::EyeballState::origin);
	defEyeballState.def_readwrite("forward",&pragma::CEyeComponent::EyeballState::forward);
	defEyeballState.def_readwrite("right",&pragma::CEyeComponent::EyeballState::right);
	defEyeballState.def_readwrite("up",&pragma::CEyeComponent::EyeballState::up);
	defEyeballState.def_readwrite("irisProjectionU",&pragma::CEyeComponent::EyeballState::irisProjectionU);
	defEyeballState.def_readwrite("irisProjectionV",&pragma::CEyeComponent::EyeballState::irisProjectionV);
	defCEye.scope[defEyeballState];

	// defCEye.add_static_constant("EVENT_ON_EYEBALLS_UPDATED",pragma::CEyeComponent::EVENT_ON_EYEBALLS_UPDATED);
	// defCEye.add_static_constant("EVENT_ON_BLINK",pragma::CEyeComponent::EVENT_ON_BLINK);
	entsMod[defCEye];

	register_renderer_bindings(entsMod);

	auto defCScene = luabind::class_<pragma::CSceneComponent,pragma::BaseEntityComponent>("SceneComponent");
	defCScene.add_static_constant("OCCLUSION_CULLING_METHOD_BRUTE_FORCE",umath::to_integral(SceneRenderDesc::OcclusionCullingMethod::BruteForce));
	defCScene.add_static_constant("OCCLUSION_CULLING_METHOD_CHC_PLUSPLUS",umath::to_integral(SceneRenderDesc::OcclusionCullingMethod::CHCPP));
	defCScene.add_static_constant("OCCLUSION_CULLING_METHOD_BSP",umath::to_integral(SceneRenderDesc::OcclusionCullingMethod::BSP));
	defCScene.add_static_constant("OCCLUSION_CULLING_METHOD_OCTREE",umath::to_integral(SceneRenderDesc::OcclusionCullingMethod::Octree));
	defCScene.add_static_constant("OCCLUSION_CULLING_METHOD_INERT",umath::to_integral(SceneRenderDesc::OcclusionCullingMethod::Inert));
	defCScene.add_static_constant("EVENT_ON_ACTIVE_CAMERA_CHANGED",pragma::CSceneComponent::CSceneComponent::EVENT_ON_ACTIVE_CAMERA_CHANGED);
	defCScene.add_static_constant("DEBUG_MODE_NONE",umath::to_integral(pragma::SceneDebugMode::None));
	defCScene.add_static_constant("DEBUG_MODE_AMBIENT_OCCLUSION",umath::to_integral(pragma::SceneDebugMode::AmbientOcclusion));
	defCScene.add_static_constant("DEBUG_MODE_ALBEDO",umath::to_integral(pragma::SceneDebugMode::Albedo));
	defCScene.add_static_constant("DEBUG_MODE_METALNESS",umath::to_integral(pragma::SceneDebugMode::Metalness));
	defCScene.add_static_constant("DEBUG_MODE_ROUGHNESS",umath::to_integral(pragma::SceneDebugMode::Roughness));
	defCScene.add_static_constant("DEBUG_MODE_DIFFUSE_LIGHTING",umath::to_integral(pragma::SceneDebugMode::DiffuseLighting));
	defCScene.add_static_constant("DEBUG_MODE_NORMAL",umath::to_integral(pragma::SceneDebugMode::Normal));
	defCScene.add_static_constant("DEBUG_MODE_NORMAL_MAP",umath::to_integral(pragma::SceneDebugMode::NormalMap));
	defCScene.add_static_constant("DEBUG_MODE_REFLECTANCE",umath::to_integral(pragma::SceneDebugMode::Reflectance));
	defCScene.add_static_constant("DEBUG_MODE_IBL_PREFILTER",umath::to_integral(pragma::SceneDebugMode::IBLPrefilter));
	defCScene.add_static_constant("DEBUG_MODE_IBL_IRRADIANCE",umath::to_integral(pragma::SceneDebugMode::IBLIrradiance));
	defCScene.add_static_constant("DEBUG_MODE_EMISSION",umath::to_integral(pragma::SceneDebugMode::Emission));
	defCScene.def("GetActiveCamera",static_cast<pragma::ComponentHandle<pragma::CCameraComponent>&(pragma::CSceneComponent::*)()>(&pragma::CSceneComponent::GetActiveCamera),luabind::game_object_policy<0>{});
	defCScene.def("SetActiveCamera",static_cast<void(pragma::CSceneComponent::*)(pragma::CCameraComponent&)>(&pragma::CSceneComponent::SetActiveCamera));
	defCScene.def("SetActiveCamera",static_cast<void(pragma::CSceneComponent::*)()>(&pragma::CSceneComponent::SetActiveCamera));
	defCScene.def("SetOcclusionCullingMethod",static_cast<void(*)(lua_State*,pragma::CSceneComponent&,uint32_t)>([](lua_State *l,pragma::CSceneComponent &scene,uint32_t method) {
		scene.GetSceneRenderDesc().SetOcclusionCullingMethod(static_cast<SceneRenderDesc::OcclusionCullingMethod>(method));
	}));
	defCScene.def("GetWidth",&pragma::CSceneComponent::GetWidth);
	defCScene.def("GetHeight",&pragma::CSceneComponent::GetHeight);
	defCScene.def("GetSize",static_cast<std::pair<uint32_t,uint32_t>(*)(const pragma::CSceneComponent&)>([](const pragma::CSceneComponent &scene) -> std::pair<uint32_t,uint32_t> {
		return {scene.GetWidth(),scene.GetHeight()};
	}),luabind::pair_policy<0>{});
	defCScene.def("Resize",&pragma::CSceneComponent::Resize);
	// defCScene.def("BeginDraw",&pragma::CSceneComponent::BeginDraw);
	defCScene.def("UpdateBuffers",&Lua::Scene::UpdateBuffers);
	defCScene.def("GetWorldEnvironment",&Lua::Scene::GetWorldEnvironment);
	defCScene.def("SetWorldEnvironment",&pragma::CSceneComponent::SetWorldEnvironment);
	defCScene.def("ClearWorldEnvironment",&pragma::CSceneComponent::ClearWorldEnvironment);
	defCScene.def("InitializeRenderTarget",&pragma::CSceneComponent::ReloadRenderTarget);

	defCScene.def("GetIndex",static_cast<pragma::CSceneComponent::SceneIndex(pragma::CSceneComponent::*)() const>(&pragma::CSceneComponent::GetSceneIndex));
	defCScene.def("GetCameraDescriptorSet",static_cast<const std::shared_ptr<prosper::IDescriptorSetGroup>&(pragma::CSceneComponent::*)(prosper::PipelineBindPoint) const>(&pragma::CSceneComponent::GetCameraDescriptorSetGroup));
	defCScene.def("GetCameraDescriptorSet",static_cast<const std::shared_ptr<prosper::IDescriptorSetGroup>&(*)(const pragma::CSceneComponent&)>([](const pragma::CSceneComponent &scene) -> const std::shared_ptr<prosper::IDescriptorSetGroup>& {
		return scene.GetCameraDescriptorSetGroup(prosper::PipelineBindPoint::Graphics);
	}));
	defCScene.def("GetViewCameraDescriptorSet",&pragma::CSceneComponent::GetViewCameraDescriptorSet);
	defCScene.def("GetDebugMode",&pragma::CSceneComponent::GetDebugMode);
	defCScene.def("SetDebugMode",&pragma::CSceneComponent::SetDebugMode);
	defCScene.def("Link",&pragma::CSceneComponent::Link);
	defCScene.def("Link",&pragma::CSceneComponent::Link);
	// defCScene.def("BuildRenderQueue",&pragma::CSceneComponent::BuildRenderQueue);
	defCScene.def("RenderPrepass",&Lua::Scene::RenderPrepass);
	defCScene.def("Render",static_cast<void(*)(lua_State*,pragma::CSceneComponent&,::util::DrawSceneInfo&,RenderMode,RenderFlags)>(Lua::Scene::Render));
	defCScene.def("Render",static_cast<void(*)(lua_State*,pragma::CSceneComponent&,::util::DrawSceneInfo&,RenderMode)>(Lua::Scene::Render));
	defCScene.def("GetRenderer",static_cast<pragma::CRendererComponent*(pragma::CSceneComponent::*)()>(&pragma::CSceneComponent::GetRenderer),luabind::game_object_policy<0>{});
	defCScene.def("SetRenderer",&pragma::CSceneComponent::SetRenderer);
	defCScene.def("GetSceneIndex",static_cast<pragma::CSceneComponent::SceneIndex(pragma::CSceneComponent::*)() const>(&pragma::CSceneComponent::GetSceneIndex));
	defCScene.def("SetParticleSystemColorFactor",&pragma::CSceneComponent::SetParticleSystemColorFactor);
	defCScene.def("GetParticleSystemColorFactor",&pragma::CSceneComponent::GetParticleSystemColorFactor);
	defCScene.def("GetRenderParticleSystems",static_cast<void(*)(lua_State*,pragma::CSceneComponent&)>([](lua_State *l,pragma::CSceneComponent &scene) {
		auto &particleSystems = scene.GetSceneRenderDesc().GetCulledParticles();
		auto t = Lua::CreateTable(l);
		int32_t idx = 1;
		for(auto &pts : particleSystems)
		{
			if(pts == nullptr)
				continue;
			Lua::PushInt(l,idx++);
			pts->PushLuaObject(l);
			Lua::SetTableValue(l,t);
		}
	}));
	defCScene.def("GetRenderQueue",static_cast<void(*)(lua_State*,pragma::CSceneComponent&,RenderMode,bool)>([](lua_State *l,pragma::CSceneComponent &scene,RenderMode renderMode,bool translucent) {
		auto *renderQueue = scene.GetSceneRenderDesc().GetRenderQueue(renderMode,translucent);
		if(renderQueue == nullptr)
			return;
		Lua::Push<pragma::rendering::RenderQueue*>(l,renderQueue);
	}));

	// Texture indices for scene render target
	defCScene.add_static_constant("RENDER_TARGET_TEXTURE_COLOR",0u);
	defCScene.add_static_constant("RENDER_TARGET_TEXTURE_BLOOM",1u);
	defCScene.add_static_constant("RENDER_TARGET_TEXTURE_DEPTH",2u);

	auto defCreateInfo = luabind::class_<pragma::CSceneComponent::CreateInfo>("CreateInfo");
	defCreateInfo.def(luabind::constructor<>());
	defCreateInfo.def_readwrite("sampleCount",&pragma::CSceneComponent::CreateInfo::sampleCount);
	defCScene.scope[defCreateInfo];

	entsMod[defCScene];

	Lua::Render::register_class(l,entsMod);
	Lua::ModelDef::register_class(l,entsMod);
	Lua::Animated::register_class(l,entsMod);
	Lua::Flex::register_class(l,entsMod);
	Lua::BSP::register_class(l,entsMod,defCWorld);
	Lua::Lightmap::register_class(l,entsMod);
	Lua::VertexAnimated::register_class(l,entsMod);
	Lua::SoundEmitter::register_class(l,entsMod);
	entsMod[defCWorld];

	auto &componentManager = engine->GetNetworkState(l)->GetGameState()->GetEntityComponentManager();

	auto defCToggle = luabind::class_<pragma::CToggleComponent,pragma::BaseToggleComponent>("ToggleComponent");
	entsMod[defCToggle];

	auto defCTransform = luabind::class_<pragma::CTransformComponent,pragma::BaseTransformComponent>("TransformComponent");
	entsMod[defCTransform];

	auto defCWheel = luabind::class_<pragma::CWheelComponent,pragma::BaseWheelComponent>("WheelComponent");
	entsMod[defCWheel];

	auto defCSoundDsp = luabind::class_<pragma::CSoundDspComponent,pragma::BaseEnvSoundDspComponent>("SoundDspComponent");
	entsMod[defCSoundDsp];

	auto defCSoundDspChorus = luabind::class_<pragma::CSoundDspChorusComponent,pragma::BaseEnvSoundDspComponent>("SoundDspChorusComponent");
	entsMod[defCSoundDspChorus];

	auto defCSoundDspDistortion = luabind::class_<pragma::CSoundDspDistortionComponent,pragma::BaseEnvSoundDspComponent>("SoundDspDistortionComponent");
	entsMod[defCSoundDspDistortion];

	auto defCSoundDspEAXReverb = luabind::class_<pragma::CSoundDspEAXReverbComponent,pragma::BaseEnvSoundDspComponent>("SoundDspEAXReverbComponent");
	entsMod[defCSoundDspEAXReverb];

	auto defCSoundDspEcho = luabind::class_<pragma::CSoundDspEchoComponent,pragma::BaseEnvSoundDspComponent>("SoundDspEchoComponent");
	entsMod[defCSoundDspEcho];

	auto defCSoundDspEqualizer = luabind::class_<pragma::CSoundDspEqualizerComponent,pragma::BaseEnvSoundDspComponent>("SoundDspEqualizerComponent");
	entsMod[defCSoundDspEqualizer];

	auto defCSoundDspFlanger = luabind::class_<pragma::CSoundDspFlangerComponent,pragma::BaseEnvSoundDspComponent>("SoundDspFlangerComponent");
	entsMod[defCSoundDspFlanger];

	auto defCCamera = luabind::class_<pragma::CCameraComponent,pragma::BaseEnvCameraComponent>("CameraComponent");
	entsMod[defCCamera];

	auto defCOccl = luabind::class_<pragma::COcclusionCullerComponent,pragma::BaseEntityComponent>("OcclusionCullerComponent");
	entsMod[defCOccl];

	auto defCDecal = luabind::class_<pragma::CDecalComponent,pragma::BaseEnvDecalComponent>("DecalComponent");
	defCDecal.def("CreateFromProjection",static_cast<void(*)(lua_State*,pragma::CDecalComponent&,luabind::object,const umath::ScaledTransform&)>(&Lua::Decal::create_from_projection));
	defCDecal.def("CreateFromProjection",static_cast<void(*)(lua_State*,pragma::CDecalComponent&,luabind::object)>(&Lua::Decal::create_from_projection));
	defCDecal.def("DebugDraw",static_cast<void(*)(lua_State*,pragma::CDecalComponent&,float)>([](lua_State *l,pragma::CDecalComponent &hEnt,float duration) {
		hEnt.GetProjector().DebugDraw(duration);
	}));
	entsMod[defCDecal];

	auto defCExplosion = luabind::class_<pragma::CExplosionComponent,pragma::BaseEnvExplosionComponent>("ExplosionComponent");
	entsMod[defCExplosion];

	auto defCFire = luabind::class_<pragma::CFireComponent,pragma::BaseEnvFireComponent>("FireComponent");
	entsMod[defCFire];

	auto defCFogController = luabind::class_<pragma::CFogControllerComponent,pragma::BaseEnvFogControllerComponent>("FogControllerComponent");
	entsMod[defCFogController];

	auto defCLight = luabind::class_<pragma::CLightComponent,pragma::BaseEnvLightComponent>("LightComponent");
	defCLight.def("SetShadowType",&pragma::CLightComponent::SetShadowType);
	defCLight.def("GetShadowType",&pragma::CLightComponent::GetShadowType);
	defCLight.def("UpdateBuffers",&pragma::CLightComponent::UpdateBuffers);
	defCLight.def("SetAddToGameScene",static_cast<void(*)(lua_State*,pragma::CLightComponent&,bool)>([](lua_State *l,pragma::CLightComponent &hComponent,bool b) {
		hComponent.SetStateFlag(pragma::CLightComponent::StateFlags::AddToGameScene,b);
	}));
	defCLight.def("SetMorphTargetsInShadowsEnabled",&pragma::CLightComponent::SetMorphTargetsInShadowsEnabled);
	defCLight.def("AreMorphTargetsInShadowsEnabled",&pragma::CLightComponent::AreMorphTargetsInShadowsEnabled);
	defCLight.add_static_constant("SHADOW_TYPE_NONE",umath::to_integral(ShadowType::None));
	defCLight.add_static_constant("SHADOW_TYPE_STATIC_ONLY",umath::to_integral(ShadowType::StaticOnly));
	defCLight.add_static_constant("SHADOW_TYPE_FULL",umath::to_integral(ShadowType::Full));

	defCLight.add_static_constant("EVENT_SHOULD_PASS_ENTITY",pragma::CLightComponent::EVENT_SHOULD_PASS_ENTITY);
	defCLight.add_static_constant("EVENT_SHOULD_PASS_ENTITY_MESH",pragma::CLightComponent::EVENT_SHOULD_PASS_ENTITY_MESH);
	defCLight.add_static_constant("EVENT_SHOULD_PASS_MESH",pragma::CLightComponent::EVENT_SHOULD_PASS_MESH);
	defCLight.add_static_constant("EVENT_SHOULD_UPDATE_RENDER_PASS",pragma::CLightComponent::EVENT_SHOULD_UPDATE_RENDER_PASS);
	defCLight.add_static_constant("EVENT_GET_TRANSFORMATION_MATRIX",pragma::CLightComponent::EVENT_GET_TRANSFORMATION_MATRIX);
	defCLight.add_static_constant("EVENT_HANDLE_SHADOW_MAP",pragma::CLightComponent::EVENT_HANDLE_SHADOW_MAP);
	defCLight.add_static_constant("EVENT_ON_SHADOW_BUFFER_INITIALIZED",pragma::CLightComponent::EVENT_ON_SHADOW_BUFFER_INITIALIZED);
	entsMod[defCLight];

	auto defCLightDirectional = luabind::class_<pragma::CLightDirectionalComponent,pragma::BaseEnvLightDirectionalComponent>("LightDirectionalComponent");
	entsMod[defCLightDirectional];

	auto defCLightPoint = luabind::class_<pragma::CLightPointComponent,pragma::BaseEnvLightPointComponent>("LightPointComponent");
	entsMod[defCLightPoint];

	auto defCLightSpot = luabind::class_<pragma::CLightSpotComponent,pragma::BaseEnvLightSpotComponent>("LightSpotComponent");
	entsMod[defCLightSpot];

	auto defCLightSpotVol = luabind::class_<pragma::CLightSpotVolComponent,pragma::BaseEnvLightSpotVolComponent>("LightSpotVolComponent");
	entsMod[defCLightSpotVol];

	auto defCMicrophone = luabind::class_<pragma::CMicrophoneComponent,pragma::BaseEnvMicrophoneComponent>("MicrophoneComponent");
	entsMod[defCMicrophone];

	Lua::ParticleSystem::register_class(l,entsMod);

	auto defCQuake = luabind::class_<pragma::CQuakeComponent,pragma::BaseEnvQuakeComponent>("QuakeComponent");
	entsMod[defCQuake];

	auto defCSmokeTrail = luabind::class_<pragma::CSmokeTrailComponent,pragma::BaseEnvSmokeTrailComponent>("SmokeTrailComponent");
	entsMod[defCSmokeTrail];

	auto defCSound = luabind::class_<pragma::CSoundComponent,pragma::BaseEnvSoundComponent>("SoundComponent");
	entsMod[defCSound];

	auto defCSoundScape = luabind::class_<pragma::CSoundScapeComponent,pragma::BaseEnvSoundScapeComponent>("SoundScapeComponent");
	entsMod[defCSoundScape];

	auto defCSprite = luabind::class_<pragma::CSpriteComponent,pragma::BaseEnvSpriteComponent>("SpriteComponent");
	defCSprite.def("StopAndRemoveEntity",&pragma::CSpriteComponent::StopAndRemoveEntity);
	entsMod[defCSprite];

	auto defCTimescale = luabind::class_<pragma::CEnvTimescaleComponent,pragma::BaseEnvTimescaleComponent>("EnvTimescaleComponent");
	entsMod[defCTimescale];

	auto defCWind = luabind::class_<pragma::CWindComponent,pragma::BaseEnvWindComponent>("WindComponent");
	entsMod[defCWind];

	auto defCFilterClass = luabind::class_<pragma::CFilterClassComponent,pragma::BaseFilterClassComponent>("FilterClassComponent");
	entsMod[defCFilterClass];

	auto defCFilterName = luabind::class_<pragma::CFilterNameComponent,pragma::BaseFilterNameComponent>("FilterNameComponent");
	entsMod[defCFilterName];

	auto defCBrush = luabind::class_<pragma::CBrushComponent,pragma::BaseFuncBrushComponent>("BrushComponent");
	entsMod[defCBrush];

	auto defCKinematic = luabind::class_<pragma::CKinematicComponent,pragma::BaseFuncKinematicComponent>("KinematicComponent");
	entsMod[defCKinematic];

	auto defCFuncPhysics = luabind::class_<pragma::CFuncPhysicsComponent,pragma::BaseFuncPhysicsComponent>("FuncPhysicsComponent");
	entsMod[defCFuncPhysics];

	auto defCFuncSoftPhysics = luabind::class_<pragma::CFuncSoftPhysicsComponent,pragma::BaseFuncSoftPhysicsComponent>("FuncSoftPhysicsComponent");
	entsMod[defCFuncSoftPhysics];

	auto defCFuncPortal = luabind::class_<pragma::CFuncPortalComponent,pragma::BaseFuncPortalComponent>("FuncPortalComponent");
	entsMod[defCFuncPortal];

	auto defCWater = luabind::class_<pragma::CWaterComponent,pragma::BaseFuncWaterComponent>("WaterComponent");
	defCWater.def("GetReflectionScene",static_cast<void(*)(lua_State*,pragma::CWaterComponent&)>([](lua_State *l,pragma::CWaterComponent &hEnt) {
		
		if(hEnt.IsWaterSceneValid() == false)
			return;
		hEnt.GetWaterScene().sceneReflection->GetLuaObject().push(l);
	}));
	defCWater.def("GetWaterSceneTexture",static_cast<void(*)(lua_State*,pragma::CWaterComponent&)>([](lua_State *l,pragma::CWaterComponent &hEnt) {
		
		if(hEnt.IsWaterSceneValid() == false)
			return;
		Lua::Push<std::shared_ptr<prosper::Texture>>(l,hEnt.GetWaterScene().texScene);
	}));
	defCWater.def("GetWaterSceneDepthTexture",static_cast<void(*)(lua_State*,pragma::CWaterComponent&)>([](lua_State *l,pragma::CWaterComponent &hEnt) {
		
		if(hEnt.IsWaterSceneValid() == false)
			return;
		Lua::Push<std::shared_ptr<prosper::Texture>>(l,hEnt.GetWaterScene().texSceneDepth);
	}));
	entsMod[defCWater];

	auto defCButton = luabind::class_<pragma::CButtonComponent,pragma::BaseFuncButtonComponent>("ButtonComponent");
	entsMod[defCButton];

	auto defCBot = luabind::class_<pragma::CBotComponent,pragma::BaseBotComponent>("BotComponent");
	entsMod[defCBot];

	auto defCPointConstraintBallSocket = luabind::class_<pragma::CPointConstraintBallSocketComponent,pragma::BasePointConstraintBallSocketComponent>("PointConstraintBallSocketComponent");
	entsMod[defCPointConstraintBallSocket];

	auto defCPointConstraintConeTwist = luabind::class_<pragma::CPointConstraintConeTwistComponent,pragma::BasePointConstraintConeTwistComponent>("PointConstraintConeTwistComponent");
	entsMod[defCPointConstraintConeTwist];

	auto defCPointConstraintDoF = luabind::class_<pragma::CPointConstraintDoFComponent,pragma::BasePointConstraintDoFComponent>("PointConstraintDoFComponent");
	entsMod[defCPointConstraintDoF];

	auto defCPointConstraintFixed = luabind::class_<pragma::CPointConstraintFixedComponent,pragma::BasePointConstraintFixedComponent>("PointConstraintFixedComponent");
	entsMod[defCPointConstraintFixed];

	auto defCPointConstraintHinge = luabind::class_<pragma::CPointConstraintHingeComponent,pragma::BasePointConstraintHingeComponent>("PointConstraintHingeComponent");
	entsMod[defCPointConstraintHinge];

	auto defCPointConstraintSlider = luabind::class_<pragma::CPointConstraintSliderComponent,pragma::BasePointConstraintSliderComponent>("PointConstraintSliderComponent");
	entsMod[defCPointConstraintSlider];

	auto defCRenderTarget = luabind::class_<pragma::CRenderTargetComponent,pragma::BasePointRenderTargetComponent>("RenderTargetComponent");
	entsMod[defCRenderTarget];

	auto defCPointTarget = luabind::class_<pragma::CPointTargetComponent,pragma::BasePointTargetComponent>("PointTargetComponent");
	entsMod[defCPointTarget];

	auto defCProp = luabind::class_<pragma::CPropComponent,pragma::BasePropComponent>("PropComponent");
	entsMod[defCProp];

	auto defCPropDynamic = luabind::class_<pragma::CPropDynamicComponent,pragma::BasePropDynamicComponent>("PropDynamicComponent");
	entsMod[defCPropDynamic];

	auto defCPropPhysics = luabind::class_<pragma::CPropPhysicsComponent,pragma::BasePropPhysicsComponent>("PropPhysicsComponent");
	entsMod[defCPropPhysics];

	auto defCTouch = luabind::class_<pragma::CTouchComponent,pragma::BaseTouchComponent>("TouchComponent");
	entsMod[defCTouch];

	auto defCSkybox = luabind::class_<pragma::CSkyboxComponent,pragma::BaseSkyboxComponent>("SkyboxComponent");
	entsMod[defCSkybox];

	auto defCFlashlight = luabind::class_<pragma::CFlashlightComponent,pragma::BaseFlashlightComponent>("FlashlightComponent");
	entsMod[defCFlashlight];

	auto defCEnvSoundProbe = luabind::class_<pragma::CEnvSoundProbeComponent,pragma::BaseEntityComponent>("EnvSoundProbeComponent");
	entsMod[defCEnvSoundProbe];

	auto defCWeather = luabind::class_<pragma::CWeatherComponent,pragma::BaseEnvWeatherComponent>("WeatherComponent");
	entsMod[defCWeather];

	auto defCReflectionProbe = luabind::class_<pragma::CReflectionProbeComponent,pragma::BaseEntityComponent>("ReflectionProbeComponent");
	defCReflectionProbe.def("GetIBLStrength",&pragma::CReflectionProbeComponent::GetIBLStrength);
	defCReflectionProbe.def("SetIBLStrength",&pragma::CReflectionProbeComponent::SetIBLStrength);
	defCReflectionProbe.def("GetIBLMaterialFilePath",&pragma::CReflectionProbeComponent::GetCubemapIBLMaterialFilePath);
	defCReflectionProbe.def("CaptureIBLReflectionsFromScene",static_cast<bool(*)(lua_State*,pragma::CReflectionProbeComponent&,luabind::table<>,bool)>(&reflection_probe_capture_ibl_reflections_from_scene));
	defCReflectionProbe.def("CaptureIBLReflectionsFromScene",static_cast<bool(*)(lua_State*,pragma::CReflectionProbeComponent&,luabind::table<>)>(&reflection_probe_capture_ibl_reflections_from_scene));
	defCReflectionProbe.def("CaptureIBLReflectionsFromScene",static_cast<bool(*)(lua_State*,pragma::CReflectionProbeComponent&,bool)>(&reflection_probe_capture_ibl_reflections_from_scene));
	defCReflectionProbe.def("CaptureIBLReflectionsFromScene",static_cast<bool(*)(lua_State*,pragma::CReflectionProbeComponent&)>(&reflection_probe_capture_ibl_reflections_from_scene));
	defCReflectionProbe.def("RequiresRebuild",&pragma::CReflectionProbeComponent::RequiresRebuild);
	entsMod[defCReflectionProbe];

	auto defCSkyCamera = luabind::class_<pragma::CSkyCameraComponent,pragma::BaseEntityComponent>("SkyCameraComponent");
	entsMod[defCSkyCamera];

	auto defCPBRConverter = luabind::class_<pragma::CPBRConverterComponent,pragma::BaseEntityComponent>("PBRConverterComponent");
	defCPBRConverter.def("GenerateAmbientOcclusionMaps",static_cast<void(*)(lua_State*,pragma::CPBRConverterComponent&,Model&,uint32_t,uint32_t,uint32_t,bool)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	defCPBRConverter.def("GenerateAmbientOcclusionMaps",static_cast<void(*)(lua_State*,pragma::CPBRConverterComponent&,Model&,uint32_t,uint32_t,uint32_t)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	defCPBRConverter.def("GenerateAmbientOcclusionMaps",static_cast<void(*)(lua_State*,pragma::CPBRConverterComponent&,Model&,uint32_t,uint32_t)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	defCPBRConverter.def("GenerateAmbientOcclusionMaps",static_cast<void(*)(lua_State*,pragma::CPBRConverterComponent&,Model&)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	defCPBRConverter.def("GenerateAmbientOcclusionMaps",static_cast<void(*)(lua_State*,pragma::CPBRConverterComponent&,BaseEntity&,uint32_t,uint32_t,uint32_t,bool)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	defCPBRConverter.def("GenerateAmbientOcclusionMaps",static_cast<void(*)(lua_State*,pragma::CPBRConverterComponent&,BaseEntity&,uint32_t,uint32_t,uint32_t)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	defCPBRConverter.def("GenerateAmbientOcclusionMaps",static_cast<void(*)(lua_State*,pragma::CPBRConverterComponent&,BaseEntity&,uint32_t,uint32_t)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	defCPBRConverter.def("GenerateAmbientOcclusionMaps",static_cast<void(*)(lua_State*,pragma::CPBRConverterComponent&,BaseEntity&)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	entsMod[defCPBRConverter];

	auto defShadow = luabind::class_<pragma::CShadowComponent,pragma::BaseEntityComponent>("ShadowMapComponent");
	entsMod[defShadow];

	auto defShadowCsm = luabind::class_<pragma::CShadowCSMComponent,pragma::BaseEntityComponent>("CSMComponent");
	entsMod[defShadowCsm];

	auto defShadowManager = luabind::class_<pragma::CShadowManagerComponent,pragma::BaseEntityComponent>("ShadowManagerComponent");
	entsMod[defShadowManager];

	auto defCWaterSurface = luabind::class_<pragma::CWaterSurfaceComponent,pragma::BaseEntityComponent>("WaterSurfaceComponent");
	entsMod[defCWaterSurface];

	auto defCListener = luabind::class_<pragma::CListenerComponent,pragma::BaseEntityComponent>("ListenerComponent");
	entsMod[defCListener];

	auto defCViewBody = luabind::class_<pragma::CViewBodyComponent,pragma::BaseEntityComponent>("ViewBodyComponent");
	entsMod[defCViewBody];

	auto defCViewModel = luabind::class_<pragma::CViewModelComponent,pragma::BaseEntityComponent>("ViewModelComponent");
	defCViewModel.def("GetPlayer",&pragma::CViewModelComponent::GetPlayer,luabind::game_object_policy<0>{});
	defCViewModel.def("GetWeapon",&pragma::CViewModelComponent::GetWeapon,luabind::game_object_policy<0>{});
	entsMod[defCViewModel];

	auto defCSoftBody = luabind::class_<pragma::CSoftBodyComponent,pragma::BaseSoftBodyComponent>("SoftBodyComponent");
	entsMod[defCSoftBody];

	auto defCRaytracing = luabind::class_<pragma::CRaytracingComponent,pragma::BaseEntityComponent>("RaytracingComponent");
	entsMod[defCRaytracing];

	auto defCBSPLeaf = luabind::class_<pragma::CBSPLeafComponent,pragma::BaseEntityComponent>("BSPLeafComponent");
	entsMod[defCBSPLeaf];

	auto defCIo = luabind::class_<pragma::CIOComponent,pragma::BaseIOComponent>("IOComponent");
	entsMod[defCIo];

	auto defCTimeScale = luabind::class_<pragma::CTimeScaleComponent,pragma::BaseTimeScaleComponent>("TimeScaleComponent");
	entsMod[defCTimeScale];

	auto defCAttachable = luabind::class_<pragma::CAttachableComponent,pragma::BaseAttachableComponent>("AttachableComponent");
	entsMod[defCAttachable];

	auto defCParent = luabind::class_<pragma::CParentComponent,pragma::BaseParentComponent>("ParentComponent");
	entsMod[defCParent];
	
	auto defCOwnable = luabind::class_<pragma::COwnableComponent,pragma::BaseOwnableComponent>("OwnableComponent");
	entsMod[defCOwnable];

	auto defCDebugText = luabind::class_<pragma::CDebugTextComponent,pragma::BaseDebugTextComponent>("DebugTextComponent");
	entsMod[defCDebugText];

	auto defCDebugPoint = luabind::class_<pragma::CDebugPointComponent,pragma::BaseDebugPointComponent>("DebugPointComponent");
	entsMod[defCDebugPoint];

	auto defCDebugLine = luabind::class_<pragma::CDebugLineComponent,pragma::BaseDebugLineComponent>("DebugLineComponent");
	entsMod[defCDebugLine];

	auto defCDebugBox = luabind::class_<pragma::CDebugBoxComponent,pragma::BaseDebugBoxComponent>("DebugBoxComponent");
	entsMod[defCDebugBox];

	auto defCDebugSphere = luabind::class_<pragma::CDebugSphereComponent,pragma::BaseDebugSphereComponent>("DebugSphereComponent");
	entsMod[defCDebugSphere];

	auto defCDebugCone = luabind::class_<pragma::CDebugConeComponent,pragma::BaseDebugConeComponent>("DebugConeComponent");
	entsMod[defCDebugCone];

	auto defCDebugCylinder = luabind::class_<pragma::CDebugCylinderComponent,pragma::BaseDebugCylinderComponent>("DebugCylinderComponent");
	entsMod[defCDebugCylinder];

	auto defCDebugPlane = luabind::class_<pragma::CDebugPlaneComponent,pragma::BaseDebugPlaneComponent>("DebugPlaneComponent");
	entsMod[defCDebugPlane];

	auto defCPointAtTarget = luabind::class_<pragma::CPointAtTargetComponent,pragma::BasePointAtTargetComponent>("PointAtTargetComponent");
	entsMod[defCPointAtTarget];

	auto defCBSP = luabind::class_<pragma::CBSPComponent,pragma::BaseEntityComponent>("BSPComponent");
	entsMod[defCBSP];

	auto defCGeneric = luabind::class_<pragma::CGenericComponent,pragma::BaseGenericComponent>("GenericComponent");
	entsMod[defCGeneric];
}

//////////////

void Lua::Flex::GetFlexController(lua_State *l,pragma::CFlexComponent &hEnt,uint32_t flexId)
{
	
	auto val = 0.f;
	if(hEnt.GetFlexController(flexId,val) == false)
		return;
	Lua::PushNumber(l,val);
}
void Lua::Flex::GetFlexController(lua_State *l,pragma::CFlexComponent &hEnt,const std::string &flexController)
{
	
	auto flexId = 0u;
	auto mdlComponent = hEnt.GetEntity().GetModelComponent();
	if(!mdlComponent || mdlComponent->LookupFlexController(flexController,flexId) == false)
		return;
	auto val = 0.f;
	if(hEnt.GetFlexController(flexId,val) == false)
		return;
	Lua::PushNumber(l,val);
}
void Lua::Flex::CalcFlexValue(lua_State *l,pragma::CFlexComponent &hEnt,uint32_t flexId)
{
	
	auto val = 0.f;
	if(hEnt.CalcFlexValue(flexId,val) == false)
		return;
	Lua::PushNumber(l,val);
}

//////////////

void Lua::SoundEmitter::CreateSound(lua_State *l,pragma::CSoundEmitterComponent &hEnt,std::string sndname,uint32_t soundType,bool bTransmit)
{
	
	auto snd = hEnt.CreateSound(sndname,static_cast<ALSoundType>(soundType));
	if(snd == nullptr)
		return;
	luabind::object(l,snd).push(l);
}
void Lua::SoundEmitter::EmitSound(lua_State *l,pragma::CSoundEmitterComponent &hEnt,std::string sndname,uint32_t soundType,float gain,float pitch,bool bTransmit)
{
	
	auto snd = hEnt.EmitSound(sndname,static_cast<ALSoundType>(soundType),gain,pitch);
	if(snd == nullptr)
		return;
	luabind::object(l,snd).push(l);
}

//////////////

void Lua::ParticleSystem::Stop(lua_State *l,pragma::CParticleSystemComponent &hComponent,bool bStopImmediately)
{
	
	if(bStopImmediately == true)
		hComponent.Stop();
	else
		hComponent.Die();
}
void Lua::ParticleSystem::AddInitializer(lua_State *l,pragma::CParticleSystemComponent &hComponent,std::string name,luabind::object o)
{
	auto t = Lua::GetStackTop(l);
	std::unordered_map<std::string,std::string> values;
	Lua::CheckTable(l,t);
	
	auto ot = luabind::object{luabind::from_stack{l,t}};
	for(luabind::iterator i{ot},end;i!=end;++i)
	{
		auto key = luabind::object_cast<std::string>(i.key());
		auto valRef = *i;
		valRef.push(l);
		auto idx = Lua::GetStackTop(l);
		std::string val = Lua::ParticleSystem::get_key_value(l,idx);
		Lua::Pop(l,1);

		ustring::to_lower(key);
		values[key] = val;
	}

	auto *initializer = hComponent.AddInitializer(name,values);
	if(initializer == nullptr)
		return;
	Lua::Push(l,initializer);
}
void Lua::ParticleSystem::AddOperator(lua_State *l,pragma::CParticleSystemComponent &hComponent,std::string name,luabind::object o)
{
	auto t = Lua::GetStackTop(l);
	std::unordered_map<std::string,std::string> values;
	Lua::CheckTable(l,t);

	auto ot = luabind::object{luabind::from_stack{l,t}};
	for(luabind::iterator i{ot},end;i!=end;++i)
	{
		auto key = luabind::object_cast<std::string>(i.key());
		auto valRef = *i;
		valRef.push(l);
		auto idx = Lua::GetStackTop(l);
		std::string val = Lua::ParticleSystem::get_key_value(l,idx);
		Lua::Pop(l,1);

		ustring::to_lower(key);
		values[key] = val;
	}

	auto *op = hComponent.AddOperator(name,values);
	if(op == nullptr)
		return;
	Lua::Push(l,op);
}
void Lua::ParticleSystem::AddRenderer(lua_State *l,pragma::CParticleSystemComponent &hComponent,std::string name,luabind::object o)
{
	auto t = Lua::GetStackTop(l);
	std::unordered_map<std::string,std::string> values;
	Lua::CheckTable(l,t);

	auto ot = luabind::object{luabind::from_stack{l,t}};
	for(luabind::iterator i{ot},end;i!=end;++i)
	{
		auto key = luabind::object_cast<std::string>(i.key());
		auto valRef = *i;
		valRef.push(l);
		auto idx = Lua::GetStackTop(l);
		std::string val = Lua::ParticleSystem::get_key_value(l,idx);
		Lua::Pop(l,1);

		ustring::to_lower(key);
		values[key] = val;
	}

	auto *renderer = hComponent.AddRenderer(name,values);
	if(renderer == nullptr)
		return;
	Lua::Push(l,renderer);
}
