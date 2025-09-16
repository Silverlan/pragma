// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/c_enginedefinitions.h"
#include "pragma/lua/c_ldefinitions.h"
#include "pragma/model/modelmesh.h"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree.hpp"
#include "pragma/entities/components/c_scene_component.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/lua/policies/shared_from_this_policy.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include "luabind/copy_policy.hpp"
#include "prosper_render_pass.hpp"
#include "prosper_command_buffer.hpp"
#include "pragma/rendering/world_environment.hpp"
#include <image/prosper_texture.hpp>
#include "cmaterial.h"
#include <mathutil/glmutil.h>
#include <pragma/lua/classes/ldef_vector.h>
#include "pragma/lua/classes/ldef_plane.h"
#include "pragma/lua/lua_util_component.hpp"
#include "luasystem.h"
#include "pragma/lua/classes/c_lshader.h"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/model/c_modelmesh.h"
#include <pragma/lua/classes/ldef_entity.h>
#include <prosper_descriptor_set_group.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <sharedutils/util_shaderinfo.hpp>
#include "pragma/rendering/occlusion_culling/c_occlusion_octree.hpp"

export module pragma.client.scripting.lua.bindings.scene;

import pragma.client.entities.components;

export namespace pragma::scripting::lua::bindings {
	void register_renderers(lua_State *l, luabind::module_ &entsMod) {
		auto defRenderer = pragma::lua::create_entity_component_class<pragma::CRendererComponent, pragma::BaseEntityComponent>("RendererComponent");
		defRenderer.def("GetWidth", &pragma::CRendererComponent::GetWidth);
		defRenderer.def("GetHeight", &pragma::CRendererComponent::GetHeight);
		defRenderer.def("InitializeRenderTarget",
		static_cast<void (*)(lua_State *, pragma::CRendererComponent &, pragma::CSceneComponent &, uint32_t, uint32_t, bool)>([](lua_State *l, pragma::CRendererComponent &renderer, pragma::CSceneComponent &scene, uint32_t width, uint32_t height, bool reload) {
			if(reload == false && width == renderer.GetWidth() && height == renderer.GetHeight())
				return;
			renderer.ReloadRenderTarget(scene, width, height);
		}));
		defRenderer.def("InitializeRenderTarget", static_cast<void (*)(lua_State *, pragma::CRendererComponent &, pragma::CSceneComponent &, uint32_t, uint32_t)>([](lua_State *l, pragma::CRendererComponent &renderer, pragma::CSceneComponent &scene, uint32_t width, uint32_t height) {
			if(width == renderer.GetWidth() && height == renderer.GetHeight())
				return;
			renderer.ReloadRenderTarget(scene, width, height);
		}));
		defRenderer.def("GetSceneTexture", &pragma::CRendererComponent::GetSceneTexture, luabind::shared_from_this_policy<0> {});
		defRenderer.def("GetPresentationTexture", &pragma::CRendererComponent::GetPresentationTexture, luabind::shared_from_this_policy<0> {});
		defRenderer.def("GetHDRPresentationTexture", &pragma::CRendererComponent::GetHDRPresentationTexture, luabind::shared_from_this_policy<0> {});
		entsMod[defRenderer];

		auto defRaster = pragma::lua::create_entity_component_class<pragma::CRasterizationRendererComponent, pragma::BaseEntityComponent>("RasterizationRendererComponent");
		defRaster.add_static_constant("EVENT_ON_RECORD_PREPASS", pragma::CRasterizationRendererComponent::EVENT_ON_RECORD_PREPASS);
		defRaster.add_static_constant("EVENT_ON_RECORD_LIGHTING_PASS", pragma::CRasterizationRendererComponent::EVENT_ON_RECORD_LIGHTING_PASS);
		defRaster.add_static_constant("EVENT_PRE_EXECUTE_PREPASS", pragma::CRasterizationRendererComponent::EVENT_PRE_EXECUTE_PREPASS);
		defRaster.add_static_constant("EVENT_POST_EXECUTE_PREPASS", pragma::CRasterizationRendererComponent::EVENT_POST_EXECUTE_PREPASS);
		defRaster.add_static_constant("EVENT_PRE_EXECUTE_LIGHTING_PASS", pragma::CRasterizationRendererComponent::EVENT_PRE_EXECUTE_LIGHTING_PASS);
		defRaster.add_static_constant("EVENT_POST_EXECUTE_LIGHTING_PASS", pragma::CRasterizationRendererComponent::EVENT_POST_EXECUTE_LIGHTING_PASS);
		defRaster.add_static_constant("EVENT_PRE_PREPASS", pragma::CRasterizationRendererComponent::EVENT_PRE_PREPASS);
		defRaster.add_static_constant("EVENT_POST_PREPASS", pragma::CRasterizationRendererComponent::EVENT_POST_PREPASS);
		defRaster.add_static_constant("EVENT_PRE_LIGHTING_PASS", pragma::CRasterizationRendererComponent::EVENT_PRE_LIGHTING_PASS);
		defRaster.add_static_constant("EVENT_POST_LIGHTING_PASS", pragma::CRasterizationRendererComponent::EVENT_POST_LIGHTING_PASS);
		defRaster.def("GetPrepassDepthTexture", +[](lua_State *l, pragma::CRasterizationRendererComponent &renderer) -> std::shared_ptr<prosper::Texture> {
			return renderer.GetPrepass().textureDepth;
		});
		defRaster.def("GetPrepassNormalTexture", +[](lua_State *l, pragma::CRasterizationRendererComponent &renderer) -> std::shared_ptr<prosper::Texture> {
			return renderer.GetPrepass().textureNormals;
		});
		defRaster.def("GetRenderTarget", +[](lua_State *l, pragma::CRasterizationRendererComponent &renderer) -> std::shared_ptr<prosper::RenderTarget> {
			return renderer.GetHDRInfo().sceneRenderTarget;
		});
		defRaster.def("BeginRenderPass", +[](lua_State *l, pragma::CRasterizationRendererComponent &renderer, const ::util::DrawSceneInfo &drawSceneInfo) -> bool {
			return renderer.BeginRenderPass(drawSceneInfo);
		});
		defRaster.def("BeginRenderPass", +[](lua_State *l, pragma::CRasterizationRendererComponent &renderer, const ::util::DrawSceneInfo &drawSceneInfo, prosper::IRenderPass &rp) -> bool {
			return renderer.BeginRenderPass(drawSceneInfo, &rp);
		});
		defRaster.def("EndRenderPass", &pragma::CRasterizationRendererComponent::EndRenderPass);
		defRaster.def("GetPrepassShader", &pragma::CRasterizationRendererComponent::GetPrepassShader);
		defRaster.def("SetShaderOverride", &pragma::CRasterizationRendererComponent::SetShaderOverride);
		defRaster.def("ClearShaderOverride", &pragma::CRasterizationRendererComponent::ClearShaderOverride);
		defRaster.def("SetPrepassMode", &pragma::CRasterizationRendererComponent::SetPrepassMode);
		defRaster.def("GetPrepassMode", &pragma::CRasterizationRendererComponent::GetPrepassMode);
		defRaster.def("SetSSAOEnabled", &pragma::CRasterizationRendererComponent::SetSSAOEnabled);
		defRaster.def("IsSSAOEnabled", &pragma::CRasterizationRendererComponent::IsSSAOEnabled);
		defRaster.def("GetLightSourceDescriptorSet", static_cast<std::shared_ptr<prosper::IDescriptorSetGroup> (*)(lua_State *, pragma::CRasterizationRendererComponent &)>([](lua_State *l, pragma::CRasterizationRendererComponent &renderer) -> std::shared_ptr<prosper::IDescriptorSetGroup> {
			auto *ds = pragma::CShadowManagerComponent::GetShadowManager()->GetDescriptorSet();
			if(ds == nullptr)
				return nullptr;
			return ds->GetDescriptorSetGroup().shared_from_this();
		}));
		defRaster.def("GetPostPrepassDepthTexture",
		static_cast<std::shared_ptr<prosper::Texture> (*)(lua_State *, pragma::CRasterizationRendererComponent &)>([](lua_State *l, pragma::CRasterizationRendererComponent &renderer) -> std::shared_ptr<prosper::Texture> { return renderer.GetPrepass().textureDepth; }));
		defRaster.def("GetPostProcessingDepthDescriptorSet",
		static_cast<std::shared_ptr<prosper::IDescriptorSetGroup> (*)(lua_State *, pragma::CRasterizationRendererComponent &)>(
			[](lua_State *l, pragma::CRasterizationRendererComponent &renderer) -> std::shared_ptr<prosper::IDescriptorSetGroup> { return renderer.GetHDRInfo().dsgDepthPostProcessing; }));
		defRaster.def("GetPostProcessingHDRColorDescriptorSet",
		static_cast<std::shared_ptr<prosper::IDescriptorSetGroup> (*)(lua_State *, pragma::CRasterizationRendererComponent &)>(
			[](lua_State *l, pragma::CRasterizationRendererComponent &renderer) -> std::shared_ptr<prosper::IDescriptorSetGroup> { return renderer.GetHDRInfo().dsgHDRPostProcessing; }));
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
		defRaster.def("GetBloomTexture", static_cast<std::shared_ptr<prosper::Texture> (*)(lua_State *, pragma::CRasterizationRendererComponent &)>([](lua_State *l, pragma::CRasterizationRendererComponent &renderer) -> std::shared_ptr<prosper::Texture> {
			auto &rt = renderer.GetHDRInfo().bloomBlurRenderTarget;
			if(rt == nullptr)
				return nullptr;
			return rt->GetTexture().shared_from_this();
		}));
#if 0
		defRaster.def("GetGlowTexture",static_cast<void(*)(lua_State*,pragma::CRasterizationRendererComponent&)>([](lua_State *l,pragma::CRasterizationRendererComponent &renderer) {
			
			auto &rt = renderer.GetGlowInfo().renderTarget;
			if(rt == nullptr)
				return;
			Lua::Push(l,rt->GetTexture().shared_from_this());
		}));
#endif
		defRaster.def("GetRenderTargetTextureDescriptorSet",
		static_cast<std::shared_ptr<prosper::IDescriptorSetGroup> (*)(lua_State *, pragma::CRasterizationRendererComponent &)>(
			[](lua_State *l, pragma::CRasterizationRendererComponent &renderer) -> std::shared_ptr<prosper::IDescriptorSetGroup> { return renderer.GetHDRInfo().dsgHDRPostProcessing; }));
		defRaster.def("ReloadPresentationRenderTarget", &pragma::CRasterizationRendererComponent::ReloadPresentationRenderTarget);
		defRaster.def("RecordPrepass", &pragma::CRasterizationRendererComponent::RecordPrepass);
		defRaster.def("RecordLightingPass", &pragma::CRasterizationRendererComponent::RecordLightingPass);
		defRaster.def("ExecutePrepass", &pragma::CRasterizationRendererComponent::ExecutePrepass);
		defRaster.def("ExecuteLightingPass", &pragma::CRasterizationRendererComponent::ExecuteLightingPass);
		defRaster.def("GetPrepassCommandBufferRecorder", &pragma::CRasterizationRendererComponent::GetPrepassCommandBufferRecorder, luabind::copy_policy<0> {});
		defRaster.def("GetShadowCommandBufferRecorder", &pragma::CRasterizationRendererComponent::GetShadowCommandBufferRecorder, luabind::copy_policy<0> {});
		defRaster.def("GetLightingPassCommandBufferRecorder", &pragma::CRasterizationRendererComponent::GetLightingPassCommandBufferRecorder, luabind::copy_policy<0> {});
		defRaster.def("UpdatePrepassRenderBuffers", &pragma::CRasterizationRendererComponent::UpdatePrepassRenderBuffers);
		defRaster.def("UpdateLightingPassRenderBuffers", &pragma::CRasterizationRendererComponent::UpdateLightingPassRenderBuffers);
		defRaster.add_static_constant("PREPASS_MODE_DISABLED", umath::to_integral(pragma::CRasterizationRendererComponent::PrepassMode::NoPrepass));
		defRaster.add_static_constant("PREPASS_MODE_DEPTH_ONLY", umath::to_integral(pragma::CRasterizationRendererComponent::PrepassMode::DepthOnly));
		defRaster.add_static_constant("PREPASS_MODE_EXTENDED", umath::to_integral(pragma::CRasterizationRendererComponent::PrepassMode::Extended));
		entsMod[defRaster];

		auto defRaytracing = pragma::lua::create_entity_component_class<pragma::CRaytracingComponent, pragma::BaseEntityComponent>("RaytracingRendererComponent");
		entsMod[defRaytracing];
	}
}
