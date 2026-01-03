// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :scripting.lua.bindings.scene;

void pragma::scripting::lua_core::bindings::register_renderers(lua::State *l, luabind::module_ &entsMod)
{
	auto defRenderer = pragma::LuaCore::create_entity_component_class<CRendererComponent, BaseEntityComponent>("RendererComponent");
	defRenderer.def("GetWidth", &CRendererComponent::GetWidth);
	defRenderer.def("GetHeight", &CRendererComponent::GetHeight);
	defRenderer.def("InitializeRenderTarget", static_cast<void (*)(lua::State *, CRendererComponent &, CSceneComponent &, uint32_t, uint32_t, bool)>([](lua::State *l, CRendererComponent &renderer, CSceneComponent &scene, uint32_t width, uint32_t height, bool reload) {
		if(reload == false && width == renderer.GetWidth() && height == renderer.GetHeight())
			return;
		renderer.ReloadRenderTarget(scene, width, height);
	}));
	defRenderer.def("InitializeRenderTarget", static_cast<void (*)(lua::State *, CRendererComponent &, CSceneComponent &, uint32_t, uint32_t)>([](lua::State *l, CRendererComponent &renderer, CSceneComponent &scene, uint32_t width, uint32_t height) {
		if(width == renderer.GetWidth() && height == renderer.GetHeight())
			return;
		renderer.ReloadRenderTarget(scene, width, height);
	}));
	defRenderer.def("GetSceneTexture", &CRendererComponent::GetSceneTexture, luabind::shared_from_this_policy<0> {});
	defRenderer.def("GetPresentationTexture", &CRendererComponent::GetPresentationTexture, luabind::shared_from_this_policy<0> {});
	defRenderer.def("GetHDRPresentationTexture", &CRendererComponent::GetHDRPresentationTexture, luabind::shared_from_this_policy<0> {});
	entsMod[defRenderer];

	auto defRaster = pragma::LuaCore::create_entity_component_class<CRasterizationRendererComponent, BaseEntityComponent>("RasterizationRendererComponent");
	defRaster.add_static_constant("EVENT_ON_RECORD_PREPASS", cRasterizationRendererComponent::EVENT_ON_RECORD_PREPASS);
	defRaster.add_static_constant("EVENT_ON_RECORD_LIGHTING_PASS", cRasterizationRendererComponent::EVENT_ON_RECORD_LIGHTING_PASS);
	defRaster.add_static_constant("EVENT_PRE_EXECUTE_PREPASS", cRasterizationRendererComponent::EVENT_PRE_EXECUTE_PREPASS);
	defRaster.add_static_constant("EVENT_POST_EXECUTE_PREPASS", cRasterizationRendererComponent::EVENT_POST_EXECUTE_PREPASS);
	defRaster.add_static_constant("EVENT_PRE_EXECUTE_LIGHTING_PASS", cRasterizationRendererComponent::EVENT_PRE_EXECUTE_LIGHTING_PASS);
	defRaster.add_static_constant("EVENT_POST_EXECUTE_LIGHTING_PASS", cRasterizationRendererComponent::EVENT_POST_EXECUTE_LIGHTING_PASS);
	defRaster.add_static_constant("EVENT_PRE_PREPASS", cRasterizationRendererComponent::EVENT_PRE_PREPASS);
	defRaster.add_static_constant("EVENT_POST_PREPASS", cRasterizationRendererComponent::EVENT_POST_PREPASS);
	defRaster.add_static_constant("EVENT_PRE_LIGHTING_PASS", cRasterizationRendererComponent::EVENT_PRE_LIGHTING_PASS);
	defRaster.add_static_constant("EVENT_POST_LIGHTING_PASS", cRasterizationRendererComponent::EVENT_POST_LIGHTING_PASS);
	defRaster.def("GetPrepassDepthTexture", +[](lua::State *l, CRasterizationRendererComponent &renderer) -> std::shared_ptr<prosper::Texture> { return renderer.GetPrepass().textureDepth; });
	defRaster.def("GetPrepassNormalTexture", +[](lua::State *l, CRasterizationRendererComponent &renderer) -> std::shared_ptr<prosper::Texture> { return renderer.GetPrepass().textureNormals; });
	defRaster.def("GetRenderTarget", +[](lua::State *l, CRasterizationRendererComponent &renderer) -> std::shared_ptr<prosper::RenderTarget> { return renderer.GetHDRInfo().sceneRenderTarget; });
	defRaster.def("BeginRenderPass", +[](lua::State *l, CRasterizationRendererComponent &renderer, const rendering::DrawSceneInfo &drawSceneInfo) -> bool { return renderer.BeginRenderPass(drawSceneInfo); });
	defRaster.def("BeginRenderPass", +[](lua::State *l, CRasterizationRendererComponent &renderer, const rendering::DrawSceneInfo &drawSceneInfo, prosper::IRenderPass &rp) -> bool { return renderer.BeginRenderPass(drawSceneInfo, &rp); });
	defRaster.def("EndRenderPass", &CRasterizationRendererComponent::EndRenderPass);
	defRaster.def("GetPrepassShader", &CRasterizationRendererComponent::GetPrepassShader);
	defRaster.def("SetShaderOverride", &CRasterizationRendererComponent::SetShaderOverride);
	defRaster.def("ClearShaderOverride", &CRasterizationRendererComponent::ClearShaderOverride);
	defRaster.def("SetPrepassMode", &CRasterizationRendererComponent::SetPrepassMode);
	defRaster.def("GetPrepassMode", &CRasterizationRendererComponent::GetPrepassMode);
	defRaster.def("SetSSAOEnabled", &CRasterizationRendererComponent::SetSSAOEnabled);
	defRaster.def("IsSSAOEnabled", &CRasterizationRendererComponent::IsSSAOEnabled);
	defRaster.def("GetLightSourceDescriptorSet", static_cast<std::shared_ptr<prosper::IDescriptorSetGroup> (*)(lua::State *, CRasterizationRendererComponent &)>([](lua::State *l, CRasterizationRendererComponent &renderer) -> std::shared_ptr<prosper::IDescriptorSetGroup> {
		auto *ds = CShadowManagerComponent::GetShadowManager()->GetDescriptorSet();
		if(ds == nullptr)
			return nullptr;
		return ds->GetDescriptorSetGroup().shared_from_this();
	}));
	defRaster.def("GetPostPrepassDepthTexture",
	  static_cast<std::shared_ptr<prosper::Texture> (*)(lua::State *, CRasterizationRendererComponent &)>([](lua::State *l, CRasterizationRendererComponent &renderer) -> std::shared_ptr<prosper::Texture> { return renderer.GetPrepass().textureDepth; }));
	defRaster.def("GetPostProcessingDepthDescriptorSet",
	  static_cast<std::shared_ptr<prosper::IDescriptorSetGroup> (*)(lua::State *, CRasterizationRendererComponent &)>([](lua::State *l, CRasterizationRendererComponent &renderer) -> std::shared_ptr<prosper::IDescriptorSetGroup> { return renderer.GetHDRInfo().dsgDepthPostProcessing; }));
	defRaster.def("GetPostProcessingHDRColorDescriptorSet",
	  static_cast<std::shared_ptr<prosper::IDescriptorSetGroup> (*)(lua::State *, CRasterizationRendererComponent &)>([](lua::State *l, CRasterizationRendererComponent &renderer) -> std::shared_ptr<prosper::IDescriptorSetGroup> { return renderer.GetHDRInfo().dsgHDRPostProcessing; }));
#if 0
	defRaster.def("GetStagingRenderTarget",static_cast<void(*)(lua::State*,pragma::CRasterizationRendererComponent&)>([](lua::State *l,pragma::CRasterizationRendererComponent &renderer) {

		auto &rt = renderer.GetHDRInfo().hdrPostProcessingRenderTarget;
		if(rt == nullptr)
			return;
		Lua::Push(l,rt);
		}));
	defRaster.def("BlitStagingRenderTargetToMainRenderTarget",static_cast<void(*)(lua::State*,pragma::CRasterizationRendererComponent&,const pragma::rendering::DrawSceneInfo&)>([](lua::State *l,pragma::CRasterizationRendererComponent &renderer,const pragma::rendering::DrawSceneInfo &drawSceneInfo) {

		renderer.GetHDRInfo().BlitStagingRenderTargetToMainRenderTarget(drawSceneInfo);
	}));
#endif
	defRaster.def("GetBloomTexture", static_cast<std::shared_ptr<prosper::Texture> (*)(lua::State *, CRasterizationRendererComponent &)>([](lua::State *l, CRasterizationRendererComponent &renderer) -> std::shared_ptr<prosper::Texture> {
		auto &rt = renderer.GetHDRInfo().bloomBlurRenderTarget;
		if(rt == nullptr)
			return nullptr;
		return rt->GetTexture().shared_from_this();
	}));
#if 0
	defRaster.def("GetGlowTexture",static_cast<void(*)(lua::State*,pragma::CRasterizationRendererComponent&)>([](lua::State *l,pragma::CRasterizationRendererComponent &renderer) {

		auto &rt = renderer.GetGlowInfo().renderTarget;
		if(rt == nullptr)
			return;
		Lua::Push(l,rt->GetTexture().shared_from_this());
	}));
#endif
	defRaster.def("GetRenderTargetTextureDescriptorSet",
	  static_cast<std::shared_ptr<prosper::IDescriptorSetGroup> (*)(lua::State *, CRasterizationRendererComponent &)>([](lua::State *l, CRasterizationRendererComponent &renderer) -> std::shared_ptr<prosper::IDescriptorSetGroup> { return renderer.GetHDRInfo().dsgHDRPostProcessing; }));
	defRaster.def("ReloadPresentationRenderTarget", &CRasterizationRendererComponent::ReloadPresentationRenderTarget);
	defRaster.def("RecordPrepass", &CRasterizationRendererComponent::RecordPrepass);
	defRaster.def("RecordLightingPass", &CRasterizationRendererComponent::RecordLightingPass);
	defRaster.def("ExecutePrepass", &CRasterizationRendererComponent::ExecutePrepass);
	defRaster.def("ExecuteLightingPass", &CRasterizationRendererComponent::ExecuteLightingPass);
	defRaster.def("GetPrepassCommandBufferRecorder", &CRasterizationRendererComponent::GetPrepassCommandBufferRecorder, luabind::copy_policy<0> {});
	defRaster.def("GetShadowCommandBufferRecorder", &CRasterizationRendererComponent::GetShadowCommandBufferRecorder, luabind::copy_policy<0> {});
	defRaster.def("GetLightingPassCommandBufferRecorder", &CRasterizationRendererComponent::GetLightingPassCommandBufferRecorder, luabind::copy_policy<0> {});
	defRaster.def("UpdatePrepassRenderBuffers", &CRasterizationRendererComponent::UpdatePrepassRenderBuffers);
	defRaster.def("UpdateLightingPassRenderBuffers", &CRasterizationRendererComponent::UpdateLightingPassRenderBuffers);
	defRaster.add_static_constant("PREPASS_MODE_DISABLED", math::to_integral(CRasterizationRendererComponent::PrepassMode::NoPrepass));
	defRaster.add_static_constant("PREPASS_MODE_DEPTH_ONLY", math::to_integral(CRasterizationRendererComponent::PrepassMode::DepthOnly));
	defRaster.add_static_constant("PREPASS_MODE_EXTENDED", math::to_integral(CRasterizationRendererComponent::PrepassMode::Extended));
	entsMod[defRaster];

	auto defRaytracing = pragma::LuaCore::create_entity_component_class<CRaytracingComponent, BaseEntityComponent>("RaytracingRendererComponent");
	entsMod[defRaytracing];
}
