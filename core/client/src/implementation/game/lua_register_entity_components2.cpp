// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

// --template-include-location

module pragma.client;

void RegisterLuaEntityComponents2_cl(lua::State *l, luabind::module_ &entsMod)
{
	auto defLiquidControl = pragma::LuaCore::create_entity_component_class<pragma::CLiquidControlComponent, pragma::BaseLiquidControlComponent>("LiquidControlComponent");
	entsMod[defLiquidControl];

	auto defLiquidSurfaceSimulation = pragma::LuaCore::create_entity_component_class<pragma::CLiquidSurfaceSimulationComponent, pragma::BaseLiquidSurfaceSimulationComponent>("LiquidSurfaceSimulationComponent");
	entsMod[defLiquidSurfaceSimulation];

	auto defBvh = pragma::LuaCore::create_entity_component_class<pragma::CBvhComponent, pragma::BaseBvhComponent>("BvhComponent");
	entsMod[defBvh];

	auto defAnimatedBvh = pragma::LuaCore::create_entity_component_class<pragma::CAnimatedBvhComponent, pragma::BaseEntityComponent>("AnimatedBvhComponent");
	defAnimatedBvh.def("SetUpdateLazily", &pragma::CAnimatedBvhComponent::SetUpdateLazily);
	defAnimatedBvh.def("RebuildAnimatedBvh", static_cast<void (pragma::CAnimatedBvhComponent::*)(bool)>(&pragma::CAnimatedBvhComponent::RebuildAnimatedBvh));
	entsMod[defAnimatedBvh];

	auto defStaticBvh = pragma::LuaCore::create_entity_component_class<pragma::CStaticBvhCacheComponent, pragma::BaseStaticBvhCacheComponent>("StaticBvhCacheComponent");
	entsMod[defStaticBvh];

	auto defStaticBvhUser = pragma::LuaCore::create_entity_component_class<pragma::CStaticBvhUserComponent, pragma::BaseStaticBvhUserComponent>("StaticBvhUserComponent");
	entsMod[defStaticBvhUser];

	auto defOpticalCamera = pragma::LuaCore::create_entity_component_class<pragma::COpticalCameraComponent, pragma::BaseEntityComponent>("OpticalCameraComponent");
	defOpticalCamera.def("SetDebugShowFocus", &pragma::COpticalCameraComponent::SetDebugShowFocus);
	defOpticalCamera.def("GetDebugShowFocus", &pragma::COpticalCameraComponent::GetDebugShowFocus);
	defOpticalCamera.def("SetVignetteEnabled", &pragma::COpticalCameraComponent::SetVignetteEnabled);
	defOpticalCamera.def("IsVignetteEnabled", &pragma::COpticalCameraComponent::IsVignetteEnabled);
	defOpticalCamera.def("SetPentagonBokehShape", &pragma::COpticalCameraComponent::SetPentagonBokehShape);
	defOpticalCamera.def("GetPentagonBokehShape", &pragma::COpticalCameraComponent::GetPentagonBokehShape);
	defOpticalCamera.def("SetDebugShowDepth", &pragma::COpticalCameraComponent::SetDebugShowDepth);
	defOpticalCamera.def("GetDebugShowDepth", &pragma::COpticalCameraComponent::GetDebugShowDepth);
	defOpticalCamera.def("SetFocalDistance", &pragma::COpticalCameraComponent::SetFocalDistance);
	defOpticalCamera.def("GetFocalDistance", &pragma::COpticalCameraComponent::GetFocalDistance);
	defOpticalCamera.def("SetFocalLength", &pragma::COpticalCameraComponent::SetFocalLength);
	defOpticalCamera.def("GetFocalLength", &pragma::COpticalCameraComponent::GetFocalLength);
	defOpticalCamera.def("SetFStop", &pragma::COpticalCameraComponent::SetFStop);
	defOpticalCamera.def("GetFStop", &pragma::COpticalCameraComponent::GetFStop);
	defOpticalCamera.def("SetApertureBokehRatio", &pragma::COpticalCameraComponent::SetApertureBokehRatio);
	defOpticalCamera.def("GetApertureBokehRatio", &pragma::COpticalCameraComponent::GetApertureBokehRatio);
	defOpticalCamera.def("SetApertureBladeCount", &pragma::COpticalCameraComponent::SetApertureBladeCount);
	defOpticalCamera.def("GetApertureBladeCount", &pragma::COpticalCameraComponent::GetApertureBladeCount);
	defOpticalCamera.def("SetApertureBladesRotation", &pragma::COpticalCameraComponent::SetApertureBladesRotation);
	defOpticalCamera.def("GetApertureBladesRotation", &pragma::COpticalCameraComponent::GetApertureBladesRotation);
	defOpticalCamera.def("SetSensorSize", &pragma::COpticalCameraComponent::SetSensorSize);
	defOpticalCamera.def("GetSensorSize", &pragma::COpticalCameraComponent::GetSensorSize);
	defOpticalCamera.def("SetRingCount", &pragma::COpticalCameraComponent::SetRingCount);
	defOpticalCamera.def("GetRingCount", &pragma::COpticalCameraComponent::GetRingCount);
	defOpticalCamera.def("SetRingSamples", &pragma::COpticalCameraComponent::SetRingSamples);
	defOpticalCamera.def("GetRingSamples", &pragma::COpticalCameraComponent::GetRingSamples);
	defOpticalCamera.def("SetCircleOfConfusionSize", &pragma::COpticalCameraComponent::SetCircleOfConfusionSize);
	defOpticalCamera.def("GetCircleOfConfusionSize", &pragma::COpticalCameraComponent::GetCircleOfConfusionSize);
	defOpticalCamera.def("SetMaxBlur", &pragma::COpticalCameraComponent::SetMaxBlur);
	defOpticalCamera.def("GetMaxBlur", &pragma::COpticalCameraComponent::GetMaxBlur);
	defOpticalCamera.def("SetDitherAmount", &pragma::COpticalCameraComponent::SetDitherAmount);
	defOpticalCamera.def("GetDitherAmount", &pragma::COpticalCameraComponent::GetDitherAmount);
	defOpticalCamera.def("SetVignettingInnerBorder", &pragma::COpticalCameraComponent::SetVignettingInnerBorder);
	defOpticalCamera.def("GetVignettingInnerBorder", &pragma::COpticalCameraComponent::GetVignettingInnerBorder);
	defOpticalCamera.def("SetVignettingOuterBorder", &pragma::COpticalCameraComponent::SetVignettingOuterBorder);
	defOpticalCamera.def("GetVignettingOuterBorder", &pragma::COpticalCameraComponent::GetVignettingOuterBorder);
	defOpticalCamera.def("SetPentagonShapeFeather", &pragma::COpticalCameraComponent::SetPentagonShapeFeather);
	defOpticalCamera.def("GetPentagonShapeFeather", &pragma::COpticalCameraComponent::GetPentagonShapeFeather);
	entsMod[defOpticalCamera];

	auto defHitboxBvh = pragma::LuaCore::create_entity_component_class<pragma::CHitboxBvhComponent, pragma::BaseEntityComponent>("HitboxBvhComponent");

	auto defDebugDrawInfo = luabind::class_<pragma::bvh::DebugDrawInfo>("DebugDrawInfo");
	defDebugDrawInfo.add_static_constant("FLAG_NONE", pragma::math::to_integral(pragma::bvh::DebugDrawInfo::Flags::None));
	defDebugDrawInfo.add_static_constant("FLAG_DRAW_TRAVERSED_NODES_BIT", pragma::math::to_integral(pragma::bvh::DebugDrawInfo::Flags::DrawTraversedNodesBit));
	defDebugDrawInfo.add_static_constant("FLAG_DRAW_TRAVERSED_LEAVES_BIT", pragma::math::to_integral(pragma::bvh::DebugDrawInfo::Flags::DrawTraversedLeavesBit));
	defDebugDrawInfo.add_static_constant("FLAG_DRAW_HIT_LEAVES_BIT", pragma::math::to_integral(pragma::bvh::DebugDrawInfo::Flags::DrawHitLeavesBit));
	defDebugDrawInfo.add_static_constant("FLAG_DRAW_TRAVERSED_MESHES_BIT", pragma::math::to_integral(pragma::bvh::DebugDrawInfo::Flags::DrawTraversedMeshesBit));
	defDebugDrawInfo.add_static_constant("FLAG_DRAW_HIT_MESHES_BIT", pragma::math::to_integral(pragma::bvh::DebugDrawInfo::Flags::DrawHitMeshesBit));
	defDebugDrawInfo.def(luabind::constructor<>());
	defDebugDrawInfo.def_readwrite("flags", &pragma::bvh::DebugDrawInfo::flags);
	defDebugDrawInfo.def_readwrite("pose", &pragma::bvh::DebugDrawInfo::basePose);
	defDebugDrawInfo.def_readwrite("duration", &pragma::bvh::DebugDrawInfo::duration);
	defHitboxBvh.scope[defDebugDrawInfo];

	defHitboxBvh.def("DebugDrawHitboxMeshes", &pragma::CHitboxBvhComponent::DebugDrawHitboxMeshes);
	defHitboxBvh.def(
	  "IntersectionTest", +[](pragma::CHitboxBvhComponent &bvhC, const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, const pragma::bvh::DebugDrawInfo &debugDrawInfo) -> std::optional<float> {
		  pragma::HitInfo hitInfo {};
		  if(!bvhC.IntersectionTest(origin, dir, minDist, maxDist, hitInfo, &debugDrawInfo))
			  return {};
		  return hitInfo.distance;
	  });

	entsMod[defHitboxBvh];

	auto defDebugHitbox = pragma::LuaCore::create_entity_component_class<pragma::CDebugHitboxComponent, pragma::BaseEntityComponent>("DebugHitboxComponent");
	defDebugHitbox.def("SetHitboxColor", &pragma::CDebugHitboxComponent::SetHitboxColor);
	entsMod[defDebugHitbox];

	auto defPpFog = pragma::LuaCore::create_entity_component_class<pragma::CRendererPpFogComponent, pragma::BaseEntityComponent>("RendererPpFogComponent");
	entsMod[defPpFog];

	auto defPpDof = pragma::LuaCore::create_entity_component_class<pragma::CRendererPpDoFComponent, pragma::BaseEntityComponent>("RendererPpDoFComponent");
	entsMod[defPpDof];

	auto defPpBloom = pragma::LuaCore::create_entity_component_class<pragma::CRendererPpBloomComponent, pragma::BaseEntityComponent>("RendererPpBloomComponent");
	defPpBloom.def("SetBlurRadius", &pragma::CRendererPpBloomComponent::SetBlurRadius);
	defPpBloom.def("SetBlurSigma", &pragma::CRendererPpBloomComponent::SetBlurSigma);
	defPpBloom.def("GetBlurRadius", &pragma::CRendererPpBloomComponent::GetBlurRadius);
	defPpBloom.def("GetBlurSigma", &pragma::CRendererPpBloomComponent::GetBlurSigma);
	defPpBloom.def("GetBloomThreshold", &pragma::CRendererPpBloomComponent::GetBloomThreshold);
	defPpBloom.def("SetBloomThreshold", &pragma::CRendererPpBloomComponent::SetBloomThreshold);
	defPpBloom.def("GetBlurAmount", &pragma::CRendererPpBloomComponent::GetBlurAmount);
	defPpBloom.def("SetBlurAmount", &pragma::CRendererPpBloomComponent::SetBlurAmount);
	entsMod[defPpBloom];

	auto defPpToneMapping = pragma::LuaCore::create_entity_component_class<pragma::CRendererPpToneMappingComponent, pragma::BaseEntityComponent>("RendererPpToneMappingComponent");
	defPpToneMapping.def("SetApplyToHdrImage", &pragma::CRendererPpToneMappingComponent::SetApplyToHdrImage);
	entsMod[defPpToneMapping];

	auto defPpFxaa = pragma::LuaCore::create_entity_component_class<pragma::CRendererPpFxaaComponent, pragma::BaseEntityComponent>("RendererPpFxaaComponent");
	entsMod[defPpFxaa];

	auto defPpMotionBlur = pragma::LuaCore::create_entity_component_class<pragma::CRendererPpMotionBlurComponent, pragma::BaseEntityComponent>("RendererPpMotionBlurComponent");
	defPpMotionBlur.add_static_constant("MOTION_BLUR_QUALITY_LOW", pragma::math::to_integral(pragma::MotionBlurQuality::Low));
	defPpMotionBlur.add_static_constant("MOTION_BLUR_QUALITY_MEDIUM", pragma::math::to_integral(pragma::MotionBlurQuality::Medium));
	defPpMotionBlur.add_static_constant("MOTION_BLUR_QUALITY_HIGH", pragma::math::to_integral(pragma::MotionBlurQuality::High));
	defPpMotionBlur.def("SetAutoUpdateMotionData", &pragma::CRendererPpMotionBlurComponent::SetAutoUpdateMotionData);
	defPpMotionBlur.def("SetMotionBlurIntensity", &pragma::CRendererPpMotionBlurComponent::SetMotionBlurIntensity);
	defPpMotionBlur.def("GetMotionBlurIntensity", &pragma::CRendererPpMotionBlurComponent::GetMotionBlurIntensity);
	defPpMotionBlur.def("SetMotionBlurQuality", &pragma::CRendererPpMotionBlurComponent::SetMotionBlurQuality);
	defPpMotionBlur.def("GetMotionBlurQuality", &pragma::CRendererPpMotionBlurComponent::GetMotionBlurQuality);
	defPpMotionBlur.def("UpdateMotionBlurData", &pragma::CRendererPpMotionBlurComponent::UpdateMotionBlurData);
	entsMod[defPpMotionBlur];

	auto defPpVol = pragma::LuaCore::create_entity_component_class<pragma::CRendererPpVolumetricComponent, pragma::BaseEntityComponent>("RendererPpVolumetricComponent");
	entsMod[defPpVol];

	auto defMotionBlurData = pragma::LuaCore::create_entity_component_class<pragma::CMotionBlurDataComponent, pragma::BaseEntityComponent>("MotionBlurDataComponent");
	defMotionBlurData.def("UpdatePoses", &pragma::CMotionBlurDataComponent::UpdateEntityPoses);
	entsMod[defMotionBlurData];

	pragma::get_cgame()->GetEntityComponentManager().RegisterLuaBindings(l, entsMod);

	// --template-component-register-location
}
