// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/lua/ostream_operator_alias.hpp"

module pragma.client;

import :game;
import :scripting.lua.bindings.scene;

namespace Lua {
	namespace PBRConverter {
		static void GenerateAmbientOcclusionMaps(lua::State *l, pragma::CPBRConverterComponent &hComponent, pragma::asset::Model &mdl, uint32_t width, uint32_t height, uint32_t samples, bool rebuild) { hComponent.GenerateAmbientOcclusionMaps(mdl, width, height, samples, rebuild); }
		static void GenerateAmbientOcclusionMaps(lua::State *l, pragma::CPBRConverterComponent &hComponent, pragma::asset::Model &mdl, uint32_t width, uint32_t height, uint32_t samples) { hComponent.GenerateAmbientOcclusionMaps(mdl, width, height, samples); }
		static void GenerateAmbientOcclusionMaps(lua::State *l, pragma::CPBRConverterComponent &hComponent, pragma::asset::Model &mdl, uint32_t width, uint32_t height) { hComponent.GenerateAmbientOcclusionMaps(mdl, width, height); }
		static void GenerateAmbientOcclusionMaps(lua::State *l, pragma::CPBRConverterComponent &hComponent, pragma::asset::Model &mdl) { hComponent.GenerateAmbientOcclusionMaps(mdl); }

		static void GenerateAmbientOcclusionMaps(lua::State *l, pragma::CPBRConverterComponent &hComponent, pragma::ecs::BaseEntity &ent, uint32_t width, uint32_t height, uint32_t samples, bool rebuild) { hComponent.GenerateAmbientOcclusionMaps(ent, width, height, samples, rebuild); }
		static void GenerateAmbientOcclusionMaps(lua::State *l, pragma::CPBRConverterComponent &hComponent, pragma::ecs::BaseEntity &ent, uint32_t width, uint32_t height, uint32_t samples) { hComponent.GenerateAmbientOcclusionMaps(ent, width, height, samples); }
		static void GenerateAmbientOcclusionMaps(lua::State *l, pragma::CPBRConverterComponent &hComponent, pragma::ecs::BaseEntity &ent, uint32_t width, uint32_t height) { hComponent.GenerateAmbientOcclusionMaps(ent, width, height); }
		static void GenerateAmbientOcclusionMaps(lua::State *l, pragma::CPBRConverterComponent &hComponent, pragma::ecs::BaseEntity &ent) { hComponent.GenerateAmbientOcclusionMaps(ent); }
	};
	namespace Decal {
		static void create_from_projection(lua::State *l, pragma::CDecalComponent &hComponent, luabind::object tMeshes, const pragma::math::ScaledTransform &pose)
		{

			int32_t t = 2;
			CheckTable(l, t);
			std::vector<pragma::DecalProjector::MeshData> meshesDatas {};
			auto numMeshes = GetObjectLength(l, t);
			meshesDatas.reserve(numMeshes);
			for(auto i = decltype(numMeshes) {0u}; i < numMeshes; ++i) {
				meshesDatas.push_back({});
				auto &meshData = meshesDatas.back();
				PushInt(l, i + 1);   /* 1 */
				GetTableValue(l, t); /* 1 */

				auto tMeshData = GetStackTop(l);
				CheckTable(l, tMeshData);

				PushString(l, "pose");       /* 2 */
				GetTableValue(l, tMeshData); /* 2 */
				meshData.pose = Lua::Check<pragma::math::ScaledTransform>(l, -1);
				Pop(l, 1); /* 1 */

				PushString(l, "subMeshes");  /* 2 */
				GetTableValue(l, tMeshData); /* 2 */
				auto tSubMeshes = GetStackTop(l);
				CheckTable(l, tSubMeshes);
				auto numSubMeshes = GetObjectLength(l, tSubMeshes);
				meshData.subMeshes.reserve(numSubMeshes);
				for(auto j = decltype(numSubMeshes) {0u}; j < numSubMeshes; ++j) {
					PushInt(l, j + 1);            /* 1 */
					GetTableValue(l, tSubMeshes); /* 1 */
					auto &mesh = Lua::Check<pragma::geometry::ModelSubMesh>(l, -1);
					meshData.subMeshes.push_back(&mesh);
					Pop(l, 1); /* 0 */
				}
				Pop(l, 1); /* 1 */

				Pop(l, 1); /* 0 */
			}
			PushBool(l, hComponent.ApplyDecal(meshesDatas));
		}
		static void create_from_projection(lua::State *l, pragma::CDecalComponent &hComponent, luabind::object tMeshes) { create_from_projection(l, hComponent, tMeshes, {}); }
	};
};

static bool reflection_probe_capture_ibl_reflections_from_scene(lua::State *l, pragma::CReflectionProbeComponent &hRp, luabind::table<> tEnts, bool renderJob)
{

	std::vector<pragma::ecs::BaseEntity *> ents {};
	ents.reserve(Lua::GetObjectLength(l, 2));
	for(auto it = luabind::iterator {tEnts}, end = luabind::iterator {}; it != end; ++it) {
		auto val = luabind::object_cast_nothrow<EntityHandle>(*it, EntityHandle {});
		if(val.expired())
			return false;
		ents.push_back(val.get());
	}
	return hRp.CaptureIBLReflectionsFromScene(&ents, renderJob);
}
static bool reflection_probe_capture_ibl_reflections_from_scene(lua::State *l, pragma::CReflectionProbeComponent &hRp, luabind::table<> tEnts) { return reflection_probe_capture_ibl_reflections_from_scene(l, hRp, tEnts, false); }
static bool reflection_probe_capture_ibl_reflections_from_scene(lua::State *l, pragma::CReflectionProbeComponent &hRp, bool renderJob) { return hRp.CaptureIBLReflectionsFromScene(nullptr, renderJob); }
static bool reflection_probe_capture_ibl_reflections_from_scene(lua::State *l, pragma::CReflectionProbeComponent &hRp) { return hRp.CaptureIBLReflectionsFromScene(); }

static void bsp_register_class(lua::State *l, luabind::module_ &entsMod, luabind::class_<pragma::CWorldComponent, pragma::BaseWorldComponent> &defWorld)
{
	auto defBspTree = luabind::class_<pragma::util::BSPTree>("BSPTree");
	defBspTree.def("IsValid", static_cast<void (*)(lua::State *, pragma::util::BSPTree &)>([](lua::State *l, pragma::util::BSPTree &tree) { Lua::PushBool(l, tree.IsValid()); }));
	defBspTree.def("IsClusterVisible", static_cast<void (*)(lua::State *, pragma::util::BSPTree &, uint32_t, uint32_t)>([](lua::State *l, pragma::util::BSPTree &tree, uint32_t clusterSrc, uint32_t clusterDst) { Lua::PushBool(l, tree.IsClusterVisible(clusterSrc, clusterDst)); }));
	defBspTree.def("GetRootNode", static_cast<void (*)(lua::State *, pragma::util::BSPTree &)>([](lua::State *l, pragma::util::BSPTree &tree) {
		auto &node = tree.GetRootNode();
		Lua::Push(l, &node);
	}));
	defBspTree.def("GetNodes", static_cast<void (*)(lua::State *, pragma::util::BSPTree &)>([](lua::State *l, pragma::util::BSPTree &tree) {
		auto &nodes = tree.GetNodes();
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto &node : nodes) {
			Lua::PushInt(l, idx++);
			Lua::Push(l, node);
			Lua::SetTableValue(l, t);
		}
	}));
	defBspTree.def("GetClusterVisibility", static_cast<void (*)(lua::State *, pragma::util::BSPTree &)>([](lua::State *l, pragma::util::BSPTree &tree) {
		auto &clusterVisibility = tree.GetClusterVisibility();
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto vis : clusterVisibility) {
			Lua::PushInt(l, idx++);
			Lua::PushInt(l, vis);
			Lua::SetTableValue(l, t);
		}
	}));
	defBspTree.def("GetClusterCount", static_cast<void (*)(lua::State *, pragma::util::BSPTree &)>([](lua::State *l, pragma::util::BSPTree &tree) { Lua::PushInt(l, tree.GetClusterCount()); }));
	defBspTree.def("FindLeafNode", static_cast<void (*)(lua::State *, pragma::util::BSPTree &, const Vector3 &)>([](lua::State *l, pragma::util::BSPTree &tree, const Vector3 &origin) {
		auto *node = tree.FindLeafNode(origin);
		if(node == nullptr)
			return;
		Lua::Push(l, &node);
	}));
	defBspTree.def("FindLeafNodesInAABB", static_cast<luabind::object (*)(lua::State *, pragma::util::BSPTree &, const Vector3 &, const Vector3 &)>([](lua::State *l, pragma::util::BSPTree &tree, const Vector3 &min, const Vector3 &max) -> luabind::object {
		auto nodes = tree.FindLeafNodesInAabb(min, max);
		auto t = luabind::newtable(l);
		int32_t idx = 1;
		for(auto &n : nodes)
			t[idx++] = n;
		return t;
	}));

	auto defBspNode = luabind::class_<pragma::util::BSPTree::Node>("Node");
	defBspNode.def("GetIndex", static_cast<pragma::util::BSPTree::ChildIndex (*)(lua::State *, pragma::util::BSPTree::Node &)>([](lua::State *l, pragma::util::BSPTree::Node &node) -> pragma::util::BSPTree::ChildIndex { return node.index; }));
	defBspNode.def("IsLeaf", static_cast<void (*)(lua::State *, pragma::util::BSPTree::Node &)>([](lua::State *l, pragma::util::BSPTree::Node &node) { Lua::PushBool(l, node.leaf); }));
	defBspNode.def("GetBounds", static_cast<void (*)(lua::State *, pragma::util::BSPTree::Node &)>([](lua::State *l, pragma::util::BSPTree::Node &node) {
		Lua::Push<Vector3>(l, node.min);
		Lua::Push<Vector3>(l, node.max);
	}));
	defBspNode.def("GetChildren", static_cast<void (*)(lua::State *, pragma::util::BSPTree::Node &)>([](lua::State *l, pragma::util::BSPTree::Node &node) {
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto &child : node.children) {
			Lua::PushInt(l, idx++);
			Lua::Push(l, child);
		}
	}));
	defBspNode.def("GetCluster", static_cast<void (*)(lua::State *, pragma::util::BSPTree::Node &)>([](lua::State *l, pragma::util::BSPTree::Node &node) { Lua::PushInt(l, node.cluster); }));
	defBspNode.def("GetVisibleLeafAreaBounds", static_cast<void (*)(lua::State *, pragma::util::BSPTree::Node &)>([](lua::State *l, pragma::util::BSPTree::Node &node) {
		Lua::Push<Vector3>(l, node.minVisible);
		Lua::Push<Vector3>(l, node.maxVisible);
	}));
	defBspNode.def("GetInternalNodePlane", static_cast<void (*)(lua::State *, pragma::util::BSPTree::Node &)>([](lua::State *l, pragma::util::BSPTree::Node &node) { Lua::Push<pragma::math::Plane>(l, node.plane); }));
	defBspNode.def("GetInternalNodeFirstFaceIndex", static_cast<void (*)(lua::State *, pragma::util::BSPTree::Node &)>([](lua::State *l, pragma::util::BSPTree::Node &node) { Lua::PushInt(l, node.firstFace); }));
	defBspNode.def("GetInternalNodeFaceCount", static_cast<void (*)(lua::State *, pragma::util::BSPTree::Node &)>([](lua::State *l, pragma::util::BSPTree::Node &node) { Lua::PushInt(l, node.numFaces); }));
	defBspTree.scope[defBspNode];
	defWorld.scope[defBspTree];
}

void RegisterLuaEntityComponents2_cl(lua::State *l, luabind::module_ &entsMod);
void pragma::CGame::RegisterLuaEntityComponents(luabind::module_ &entsMod)
{
	Game::RegisterLuaEntityComponents(entsMod);
	auto *l = GetLuaState();

	auto defCGamemode = pragma::LuaCore::create_entity_component_class<CGamemodeComponent, BaseGamemodeComponent>("GamemodeComponent");
	entsMod[defCGamemode];

	auto defCGame = pragma::LuaCore::create_entity_component_class<CGameComponent, BaseGameComponent>("GameComponent");
	entsMod[defCGame];

	auto defCInput = pragma::LuaCore::create_entity_component_class<CInputComponent, BaseEntityComponent>("InputComponent");
	defCInput.def("GetMouseDeltaX", &CInputComponent::GetMouseDeltaX);
	defCInput.def("GetMouseDeltaY", &CInputComponent::GetMouseDeltaY);
	entsMod[defCInput];

	auto defCColor = pragma::LuaCore::create_entity_component_class<CColorComponent, BaseColorComponent>("ColorComponent");
	entsMod[defCColor];

	auto defCScore = pragma::LuaCore::create_entity_component_class<CScoreComponent, BaseScoreComponent>("ScoreComponent");
	entsMod[defCScore];

	auto defCFlammable = pragma::LuaCore::create_entity_component_class<CFlammableComponent, BaseFlammableComponent>("FlammableComponent");
	entsMod[defCFlammable];

	auto defCHealth = pragma::LuaCore::create_entity_component_class<CHealthComponent, BaseHealthComponent>("HealthComponent");
	entsMod[defCHealth];

	auto defCName = pragma::LuaCore::create_entity_component_class<CNameComponent, BaseNameComponent>("NameComponent");
	entsMod[defCName];

	auto defCNetworked = pragma::LuaCore::create_entity_component_class<CNetworkedComponent, BaseNetworkedComponent>("NetworkedComponent");
	entsMod[defCNetworked];

	auto defCObservable = pragma::LuaCore::create_entity_component_class<CObservableComponent, BaseObservableComponent>("ObservableComponent");
	entsMod[defCObservable];

	auto defCObserver = pragma::LuaCore::create_entity_component_class<CObserverComponent, BaseObserverComponent>("ObserverComponent");
	entsMod[defCObserver];

	auto defCShooter = pragma::LuaCore::create_entity_component_class<ecs::CShooterComponent, ecs::BaseShooterComponent>("ShooterComponent");
	entsMod[defCShooter];

	auto defCPhysics = pragma::LuaCore::create_entity_component_class<CPhysicsComponent, BasePhysicsComponent>("PhysicsComponent");
	entsMod[defCPhysics];

	auto defCRadius = pragma::LuaCore::create_entity_component_class<CRadiusComponent, BaseRadiusComponent>("RadiusComponent");
	entsMod[defCRadius];

	auto defCFieldAngle = pragma::LuaCore::create_entity_component_class<CFieldAngleComponent, BaseFieldAngleComponent>("FieldAngleComponent");
	entsMod[defCFieldAngle];

	auto defCWorld = pragma::LuaCore::create_entity_component_class<CWorldComponent, BaseWorldComponent>("WorldComponent");
	defCWorld.def("GetBSPTree", &CWorldComponent::GetBSPTree);
	defCWorld.def("RebuildRenderQueues", &CWorldComponent::RebuildRenderQueues);

	auto defCEye = pragma::LuaCore::create_entity_component_class<CEyeComponent, BaseEntityComponent>("EyeComponent");
	defCEye.def("GetEyePose", &CEyeComponent::GetEyePose);
	defCEye.def("GetViewTarget", &CEyeComponent::GetViewTarget);
	defCEye.def("SetViewTarget", &CEyeComponent::SetViewTarget);
	defCEye.def("ClearViewTarget", &CEyeComponent::ClearViewTarget);
	defCEye.def("GetEyeShift", static_cast<std::optional<Vector3> (*)(lua::State *, CEyeComponent &, uint32_t)>([](lua::State *l, CEyeComponent &hEye, uint32_t eyeIndex) -> std::optional<Vector3> {
		auto *config = hEye.GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return {};
		return config->eyeShift;
	}));
	defCEye.def("SetEyeShift", static_cast<void (*)(lua::State *, CEyeComponent &, uint32_t, const Vector3 &)>([](lua::State *l, CEyeComponent &hEye, uint32_t eyeIndex, const Vector3 &eyeShift) {
		auto *config = hEye.GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return;
		config->eyeShift = eyeShift;
	}));
	defCEye.def("GetEyeJitter", static_cast<std::optional<Vector2> (*)(lua::State *, CEyeComponent &, uint32_t)>([](lua::State *l, CEyeComponent &hEye, uint32_t eyeIndex) -> std::optional<Vector2> {
		auto *config = hEye.GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return {};
		return config->jitter;
	}));
	defCEye.def("SetEyeJitter", static_cast<void (*)(lua::State *, CEyeComponent &, uint32_t, const Vector2 &)>([](lua::State *l, CEyeComponent &hEye, uint32_t eyeIndex, const Vector2 &eyeJitter) {
		auto *config = hEye.GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return;
		config->jitter = eyeJitter;
	}));
	defCEye.def("GetEyeSize", static_cast<std::optional<float> (*)(lua::State *, CEyeComponent &, uint32_t)>([](lua::State *l, CEyeComponent &hEye, uint32_t eyeIndex) -> std::optional<float> {
		auto *config = hEye.GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return {};
		return config->eyeSize;
	}));
	defCEye.def("SetEyeSize", static_cast<void (*)(lua::State *, CEyeComponent &, uint32_t, float)>([](lua::State *l, CEyeComponent &hEye, uint32_t eyeIndex, float eyeSize) {
		auto *config = hEye.GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return;
		config->eyeSize = eyeSize;
	}));
	defCEye.def("SetIrisDilation", static_cast<void (*)(lua::State *, CEyeComponent &, uint32_t, float)>([](lua::State *l, CEyeComponent &hEye, uint32_t eyeIndex, float dilation) {
		auto *config = hEye.GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return;
		config->dilation = dilation;
	}));
	defCEye.def("GetIrisDilation", static_cast<std::optional<float> (*)(lua::State *, CEyeComponent &, uint32_t)>([](lua::State *l, CEyeComponent &hEye, uint32_t eyeIndex) -> std::optional<float> {
		auto *config = hEye.GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return {};
		return config->dilation;
	}));
	defCEye.def("CalcEyeballPose", static_cast<std::pair<math::Transform, math::Transform> (*)(lua::State *, CEyeComponent &, uint32_t)>([](lua::State *l, CEyeComponent &hEye, uint32_t eyeIndex) -> std::pair<math::Transform, math::Transform> {
		math::Transform bonePose;
		auto pose = hEye.CalcEyeballPose(eyeIndex, &bonePose);
		return {pose, bonePose};
	}));
	defCEye.def(
	  "FindEyeballIndex", +[](lua::State *l, CEyeComponent &hEye, uint32_t skinMatIdx) -> std::optional<uint32_t> {
		  uint32_t eyeballIdx;
		  if(!hEye.FindEyeballIndex(skinMatIdx, eyeballIdx))
			  return {};
		  return eyeballIdx;
	  });
	defCEye.def(
	  "GetEyeballProjectionVectors", +[](lua::State *l, CEyeComponent &hEye, uint32_t eyeballIndex) -> std::optional<std::pair<Vector4, Vector4>> {
		  Vector4 projU, projV;
		  if(!hEye.GetEyeballProjectionVectors(eyeballIndex, projU, projV))
			  return {};
		  return std::pair<Vector4, Vector4> {projU, projV};
	  });
	defCEye.def("GetEyeballState", static_cast<CEyeComponent::EyeballState *(*)(lua::State *, CEyeComponent &, uint32_t)>([](lua::State *l, CEyeComponent &hEye, uint32_t eyeIndex) -> CEyeComponent::EyeballState * {
		auto *eyeballData = hEye.GetEyeballData(eyeIndex);
		if(eyeballData == nullptr)
			return nullptr;
		return &eyeballData->state;
	}));
	defCEye.def("SetBlinkDuration", &CEyeComponent::SetBlinkDuration);
	defCEye.def("GetBlinkDuration", &CEyeComponent::GetBlinkDuration);
	defCEye.def("SetBlinkingEnabled", &CEyeComponent::SetBlinkingEnabled);
	defCEye.def("IsBlinkingEnabled", &CEyeComponent::IsBlinkingEnabled);

	auto defEyeballState = luabind::class_<CEyeComponent::EyeballState>("EyeballState");
	defEyeballState.def_readwrite("origin", &CEyeComponent::EyeballState::origin);
	defEyeballState.def_readwrite("forward", &CEyeComponent::EyeballState::forward);
	defEyeballState.def_readwrite("right", &CEyeComponent::EyeballState::right);
	defEyeballState.def_readwrite("up", &CEyeComponent::EyeballState::up);
	defEyeballState.def_readwrite("irisProjectionU", &CEyeComponent::EyeballState::irisProjectionU);
	defEyeballState.def_readwrite("irisProjectionV", &CEyeComponent::EyeballState::irisProjectionV);
	defCEye.scope[defEyeballState];

	// defCEye.add_static_constant("EVENT_ON_EYEBALLS_UPDATED",pragma::CEyeComponent::EVENT_ON_EYEBALLS_UPDATED);
	// defCEye.add_static_constant("EVENT_ON_BLINK",pragma::CEyeComponent::EVENT_ON_BLINK);
	entsMod[defCEye];

	scripting::lua_core::bindings::register_renderers(l, entsMod);

	auto defCScene = pragma::LuaCore::create_entity_component_class<CSceneComponent, BaseEntityComponent>("SceneComponent");
	defCScene.add_static_constant("OCCLUSION_CULLING_METHOD_BRUTE_FORCE", math::to_integral(SceneRenderDesc::OcclusionCullingMethod::BruteForce));
	defCScene.add_static_constant("OCCLUSION_CULLING_METHOD_CHC_PLUSPLUS", math::to_integral(SceneRenderDesc::OcclusionCullingMethod::CHCPP));
	defCScene.add_static_constant("OCCLUSION_CULLING_METHOD_BSP", math::to_integral(SceneRenderDesc::OcclusionCullingMethod::BSP));
	defCScene.add_static_constant("OCCLUSION_CULLING_METHOD_OCTREE", math::to_integral(SceneRenderDesc::OcclusionCullingMethod::Octree));
	defCScene.add_static_constant("OCCLUSION_CULLING_METHOD_INERT", math::to_integral(SceneRenderDesc::OcclusionCullingMethod::Inert));
	defCScene.add_static_constant("EVENT_ON_ACTIVE_CAMERA_CHANGED", cSceneComponent::EVENT_ON_ACTIVE_CAMERA_CHANGED);
	defCScene.add_static_constant("EVENT_ON_RENDERER_CHANGED", cSceneComponent::EVENT_ON_RENDERER_CHANGED);
	defCScene.add_static_constant("DEBUG_MODE_NONE", math::to_integral(SceneDebugMode::None));
	defCScene.add_static_constant("DEBUG_MODE_AMBIENT_OCCLUSION", math::to_integral(SceneDebugMode::AmbientOcclusion));
	defCScene.add_static_constant("DEBUG_MODE_ALBEDO", math::to_integral(SceneDebugMode::Albedo));
	defCScene.add_static_constant("DEBUG_MODE_METALNESS", math::to_integral(SceneDebugMode::Metalness));
	defCScene.add_static_constant("DEBUG_MODE_ROUGHNESS", math::to_integral(SceneDebugMode::Roughness));
	defCScene.add_static_constant("DEBUG_MODE_DIFFUSE_LIGHTING", math::to_integral(SceneDebugMode::DiffuseLighting));
	defCScene.add_static_constant("DEBUG_MODE_NORMAL", math::to_integral(SceneDebugMode::Normal));
	defCScene.add_static_constant("DEBUG_MODE_NORMAL_MAP", math::to_integral(SceneDebugMode::NormalMap));
	defCScene.add_static_constant("DEBUG_MODE_REFLECTANCE", math::to_integral(SceneDebugMode::Reflectance));
	defCScene.add_static_constant("DEBUG_MODE_IBL_PREFILTER", math::to_integral(SceneDebugMode::IBLPrefilter));
	defCScene.add_static_constant("DEBUG_MODE_IBL_IRRADIANCE", math::to_integral(SceneDebugMode::IBLIrradiance));
	defCScene.add_static_constant("DEBUG_MODE_EMISSION", math::to_integral(SceneDebugMode::Emission));
	defCScene.def("GetActiveCamera", static_cast<ComponentHandle<CCameraComponent> &(CSceneComponent::*)()>(&CSceneComponent::GetActiveCamera));
	defCScene.def("SetActiveCamera", static_cast<void (CSceneComponent::*)(CCameraComponent &)>(&CSceneComponent::SetActiveCamera));
	defCScene.def("SetActiveCamera", static_cast<void (CSceneComponent::*)()>(&CSceneComponent::SetActiveCamera));
	defCScene.def("SetExclusionRenderMask", &CSceneComponent::SetExclusionRenderMask);
	defCScene.def("GetExclusionRenderMask", &CSceneComponent::GetExclusionRenderMask);
	defCScene.def("SetInclusionRenderMask", &CSceneComponent::SetInclusionRenderMask);
	defCScene.def("GetInclusionRenderMask", &CSceneComponent::GetInclusionRenderMask);
	defCScene.def("GetWidth", &CSceneComponent::GetWidth);
	defCScene.def("GetHeight", &CSceneComponent::GetHeight);
	defCScene.def("GetSize", static_cast<std::pair<uint32_t, uint32_t> (*)(const CSceneComponent &)>([](const CSceneComponent &scene) -> std::pair<uint32_t, uint32_t> { return {scene.GetWidth(), scene.GetHeight()}; }));
	defCScene.def("Resize", &CSceneComponent::Resize);
	// defCScene.def("BeginDraw",&pragma::CSceneComponent::BeginDraw);
	defCScene.def(
	  "UpdateBuffers", +[](lua::State *l, CSceneComponent &scene, prosper::ICommandBuffer &hCommandBuffer) {
		  if(hCommandBuffer.IsPrimary() == false)
			  return;
		  auto pCmdBuffer = std::dynamic_pointer_cast<prosper::IPrimaryCommandBuffer>(hCommandBuffer.shared_from_this());
		  scene.UpdateBuffers(pCmdBuffer);
	  });
	defCScene.def(
	  "GetWorldEnvironment", +[](lua::State *l, CSceneComponent &scene) -> std::shared_ptr<rendering::WorldEnvironment> {
		  auto *worldEnv = scene.GetWorldEnvironment();
		  if(worldEnv == nullptr)
			  return nullptr;
		  return worldEnv->shared_from_this();
	  });
	defCScene.def("SetWorldEnvironment", &CSceneComponent::SetWorldEnvironment);
	defCScene.def("ClearWorldEnvironment", &CSceneComponent::ClearWorldEnvironment);
	defCScene.def("InitializeRenderTarget", &CSceneComponent::ReloadRenderTarget);

	defCScene.def("GetIndex", static_cast<CSceneComponent::SceneIndex (CSceneComponent::*)() const>(&CSceneComponent::GetSceneIndex));
	defCScene.def("GetCameraDescriptorSet", static_cast<const std::shared_ptr<prosper::IDescriptorSetGroup> &(CSceneComponent::*)(prosper::PipelineBindPoint) const>(&CSceneComponent::GetCameraDescriptorSetGroup));
	defCScene.def("GetCameraDescriptorSet", static_cast<const std::shared_ptr<prosper::IDescriptorSetGroup> &(*)(const CSceneComponent &)>([](const CSceneComponent &scene) -> const std::shared_ptr<prosper::IDescriptorSetGroup> & {
		return scene.GetCameraDescriptorSetGroup(prosper::PipelineBindPoint::Graphics);
	}));
	defCScene.def("GetViewCameraDescriptorSet", &CSceneComponent::GetViewCameraDescriptorSet);
	defCScene.def("GetDebugMode", &CSceneComponent::GetDebugMode);
	defCScene.def("SetDebugMode", &CSceneComponent::SetDebugMode);
	defCScene.def("Link", &CSceneComponent::Link, luabind::default_parameter_policy<3, true> {});
	defCScene.def("Link", &CSceneComponent::Link);
	// defCScene.def("BuildRenderQueue",&pragma::CSceneComponent::BuildRenderQueue);
	defCScene.def("GetRenderer", static_cast<CRendererComponent *(CSceneComponent::*)()>(&CSceneComponent::GetRenderer<CRendererComponent>));
	defCScene.def("SetRenderer", &CSceneComponent::SetRenderer<CRendererComponent>);
	defCScene.def("GetSceneIndex", static_cast<CSceneComponent::SceneIndex (CSceneComponent::*)() const>(&CSceneComponent::GetSceneIndex));
	defCScene.def("SetParticleSystemColorFactor", &CSceneComponent::SetParticleSystemColorFactor);
	defCScene.def("GetParticleSystemColorFactor", &CSceneComponent::GetParticleSystemColorFactor, luabind::copy_policy<0> {});
	//defCScene.def("GetRenderParticleSystems",static_cast<std::vector<pragma::ecs::CParticleSystemComponent*>(*)(lua::State*,pragma::CSceneComponent&)>([](lua::State *l,pragma::CSceneComponent &scene) -> std::vector<pragma::ecs::CParticleSystemComponent*> {
	//	return scene.GetSceneRenderDesc().GetCulledParticles();
	//}));
	defCScene.def(
	  "GetRenderQueue", +[](lua::State *l, CSceneComponent &scene, rendering::SceneRenderPass renderMode, bool translucent) -> rendering::RenderQueue * {
		  auto *renderQueue = scene.GetSceneRenderDesc().GetRenderQueue(renderMode, translucent);
		  if(renderQueue == nullptr)
			  return nullptr;
		  return renderQueue;
	  });

	// Texture indices for scene render target
	defCScene.add_static_constant("RENDER_TARGET_TEXTURE_COLOR", 0u);
	defCScene.add_static_constant("RENDER_TARGET_TEXTURE_BLOOM", 1u);
	defCScene.add_static_constant("RENDER_TARGET_TEXTURE_DEPTH", 2u);

	auto defCreateInfo = luabind::class_<CSceneComponent::CreateInfo>("CreateInfo");
	defCreateInfo.def(luabind::constructor<>());
	defCreateInfo.def_readwrite("sampleCount", &CSceneComponent::CreateInfo::sampleCount);
	defCScene.scope[defCreateInfo];

	entsMod[defCScene];

	bsp_register_class(l, entsMod, defCWorld);
	entsMod[defCWorld];

	auto &componentManager = Engine::Get()->GetNetworkState(l)->GetGameState()->GetEntityComponentManager();

	auto defCToggle = pragma::LuaCore::create_entity_component_class<CToggleComponent, BaseToggleComponent>("ToggleComponent");
	entsMod[defCToggle];

	auto defCTransform = pragma::LuaCore::create_entity_component_class<CTransformComponent, BaseTransformComponent>("TransformComponent");
	defCTransform.add_static_constant("EVENT_ON_POSE_CHANGED", cTransformComponent::EVENT_ON_POSE_CHANGED);
	entsMod[defCTransform];

	auto defCWheel = pragma::LuaCore::create_entity_component_class<CWheelComponent, BaseWheelComponent>("WheelComponent");
	entsMod[defCWheel];

	auto defCSoundDsp = pragma::LuaCore::create_entity_component_class<CSoundDspComponent, BaseEnvSoundDspComponent>("SoundDspComponent");
	entsMod[defCSoundDsp];

	auto defCSoundDspChorus = pragma::LuaCore::create_entity_component_class<CSoundDspChorusComponent, BaseEnvSoundDspComponent>("SoundDspChorusComponent");
	entsMod[defCSoundDspChorus];

	auto defCSoundDspDistortion = pragma::LuaCore::create_entity_component_class<CSoundDspDistortionComponent, BaseEnvSoundDspComponent>("SoundDspDistortionComponent");
	entsMod[defCSoundDspDistortion];

	auto defCSoundDspEAXReverb = pragma::LuaCore::create_entity_component_class<CSoundDspEAXReverbComponent, BaseEnvSoundDspComponent>("SoundDspEAXReverbComponent");
	entsMod[defCSoundDspEAXReverb];

	auto defCSoundDspEcho = pragma::LuaCore::create_entity_component_class<CSoundDspEchoComponent, BaseEnvSoundDspComponent>("SoundDspEchoComponent");
	entsMod[defCSoundDspEcho];

	auto defCSoundDspEqualizer = pragma::LuaCore::create_entity_component_class<CSoundDspEqualizerComponent, BaseEnvSoundDspComponent>("SoundDspEqualizerComponent");
	entsMod[defCSoundDspEqualizer];

	auto defCSoundDspFlanger = pragma::LuaCore::create_entity_component_class<CSoundDspFlangerComponent, BaseEnvSoundDspComponent>("SoundDspFlangerComponent");
	entsMod[defCSoundDspFlanger];

	auto defCCamera = pragma::LuaCore::create_entity_component_class<CCameraComponent, BaseEnvCameraComponent>("CameraComponent");
	entsMod[defCCamera];

	auto defCOccl = pragma::LuaCore::create_entity_component_class<COcclusionCullerComponent, BaseEntityComponent>("OcclusionCullerComponent");
	entsMod[defCOccl];

	auto defCDecal = pragma::LuaCore::create_entity_component_class<CDecalComponent, BaseEnvDecalComponent>("DecalComponent");
	defCDecal.def("CreateFromProjection", static_cast<void (*)(lua::State *, CDecalComponent &, luabind::object, const math::ScaledTransform &)>(&Lua::Decal::create_from_projection));
	defCDecal.def("CreateFromProjection", static_cast<void (*)(lua::State *, CDecalComponent &, luabind::object)>(&Lua::Decal::create_from_projection));
	defCDecal.def("DebugDraw", &CDecalComponent::DebugDraw);
	defCDecal.def("ApplyDecal", static_cast<bool (CDecalComponent::*)()>(&CDecalComponent::ApplyDecal));
	entsMod[defCDecal];

	auto defCExplosion = pragma::LuaCore::create_entity_component_class<CExplosionComponent, BaseEnvExplosionComponent>("ExplosionComponent");
	entsMod[defCExplosion];

	auto defCFire = pragma::LuaCore::create_entity_component_class<CFireComponent, BaseEnvFireComponent>("FireComponent");
	entsMod[defCFire];

	auto defCFogController = pragma::LuaCore::create_entity_component_class<CFogControllerComponent, BaseEnvFogControllerComponent>("FogControllerComponent");
	entsMod[defCFogController];

	auto defCLight = pragma::LuaCore::create_entity_component_class<CLightComponent, BaseEnvLightComponent>("LightComponent");
	defCLight.def("SetShadowType", &CLightComponent::SetShadowType);
	defCLight.def("GetShadowType", &CLightComponent::GetShadowType);
	defCLight.def("UpdateBuffers", &CLightComponent::UpdateBuffers);
	defCLight.def("SetAddToGameScene", static_cast<void (*)(lua::State *, CLightComponent &, bool)>([](lua::State *l, CLightComponent &hComponent, bool b) { hComponent.SetStateFlag(CLightComponent::StateFlags::AddToGameScene, b); }));
	defCLight.def("SetMorphTargetsInShadowsEnabled", &CLightComponent::SetMorphTargetsInShadowsEnabled);
	defCLight.def("AreMorphTargetsInShadowsEnabled", &CLightComponent::AreMorphTargetsInShadowsEnabled);
	defCLight.add_static_constant("SHADOW_TYPE_NONE", math::to_integral(rendering::ShadowType::None));
	defCLight.add_static_constant("SHADOW_TYPE_STATIC_ONLY", math::to_integral(rendering::ShadowType::StaticOnly));
	defCLight.add_static_constant("SHADOW_TYPE_FULL", math::to_integral(rendering::ShadowType::Full));

	defCLight.add_static_constant("EVENT_SHOULD_PASS_ENTITY", cLightComponent::EVENT_SHOULD_PASS_ENTITY);
	defCLight.add_static_constant("EVENT_SHOULD_PASS_ENTITY_MESH", cLightComponent::EVENT_SHOULD_PASS_ENTITY_MESH);
	defCLight.add_static_constant("EVENT_SHOULD_PASS_MESH", cLightComponent::EVENT_SHOULD_PASS_MESH);
	defCLight.add_static_constant("EVENT_SHOULD_UPDATE_RENDER_PASS", cLightComponent::EVENT_SHOULD_UPDATE_RENDER_PASS);
	defCLight.add_static_constant("EVENT_GET_TRANSFORMATION_MATRIX", cLightComponent::EVENT_GET_TRANSFORMATION_MATRIX);
	defCLight.add_static_constant("EVENT_HANDLE_SHADOW_MAP", cLightComponent::EVENT_HANDLE_SHADOW_MAP);
	defCLight.add_static_constant("EVENT_ON_SHADOW_BUFFER_INITIALIZED", cLightComponent::EVENT_ON_SHADOW_BUFFER_INITIALIZED);
	entsMod[defCLight];

	auto defCLightDirectional = pragma::LuaCore::create_entity_component_class<CLightDirectionalComponent, BaseEnvLightDirectionalComponent>("LightDirectionalComponent");
	entsMod[defCLightDirectional];

	auto defCLightPoint = pragma::LuaCore::create_entity_component_class<CLightPointComponent, BaseEnvLightPointComponent>("LightPointComponent");
	entsMod[defCLightPoint];

	auto defCLightSpot = pragma::LuaCore::create_entity_component_class<CLightSpotComponent, BaseEnvLightSpotComponent>("LightSpotComponent");
	entsMod[defCLightSpot];

	auto defCLightSpotVol = pragma::LuaCore::create_entity_component_class<CLightSpotVolComponent, BaseEnvLightSpotVolComponent>("LightSpotVolComponent");
	entsMod[defCLightSpotVol];

	auto defCMicrophone = pragma::LuaCore::create_entity_component_class<CMicrophoneComponent, BaseEnvMicrophoneComponent>("MicrophoneComponent");
	entsMod[defCMicrophone];

	auto defCQuake = pragma::LuaCore::create_entity_component_class<CQuakeComponent, BaseEnvQuakeComponent>("QuakeComponent");
	entsMod[defCQuake];

	auto defCSmokeTrail = pragma::LuaCore::create_entity_component_class<CSmokeTrailComponent, BaseEnvSmokeTrailComponent>("SmokeTrailComponent");
	entsMod[defCSmokeTrail];

	auto defCSound = pragma::LuaCore::create_entity_component_class<CSoundComponent, BaseEnvSoundComponent>("SoundComponent");
	entsMod[defCSound];

	auto defCSoundScape = pragma::LuaCore::create_entity_component_class<CSoundScapeComponent, BaseEnvSoundScapeComponent>("SoundScapeComponent");
	entsMod[defCSoundScape];

	auto defCSprite = pragma::LuaCore::create_entity_component_class<CSpriteComponent, BaseEnvSpriteComponent>("SpriteComponent");
	defCSprite.def("StopAndRemoveEntity", &CSpriteComponent::StopAndRemoveEntity);
	entsMod[defCSprite];

	auto defCTimescale = pragma::LuaCore::create_entity_component_class<CEnvTimescaleComponent, BaseEnvTimescaleComponent>("EnvTimescaleComponent");
	entsMod[defCTimescale];

	auto defCWind = pragma::LuaCore::create_entity_component_class<CWindComponent, BaseEnvWindComponent>("WindComponent");
	entsMod[defCWind];

	auto defCFilterClass = pragma::LuaCore::create_entity_component_class<CFilterClassComponent, BaseFilterClassComponent>("FilterClassComponent");
	entsMod[defCFilterClass];

	auto defCFilterName = pragma::LuaCore::create_entity_component_class<CFilterNameComponent, BaseFilterNameComponent>("FilterNameComponent");
	entsMod[defCFilterName];

	auto defCBrush = pragma::LuaCore::create_entity_component_class<CBrushComponent, BaseFuncBrushComponent>("BrushComponent");
	entsMod[defCBrush];

	auto defCKinematic = pragma::LuaCore::create_entity_component_class<CKinematicComponent, BaseFuncKinematicComponent>("KinematicComponent");
	entsMod[defCKinematic];

	auto defCFuncPhysics = pragma::LuaCore::create_entity_component_class<CFuncPhysicsComponent, BaseFuncPhysicsComponent>("FuncPhysicsComponent");
	entsMod[defCFuncPhysics];

	auto defCFuncSoftPhysics = pragma::LuaCore::create_entity_component_class<CFuncSoftPhysicsComponent, BaseFuncSoftPhysicsComponent>("FuncSoftPhysicsComponent");
	entsMod[defCFuncSoftPhysics];

	auto defCSurface = pragma::LuaCore::create_entity_component_class<CSurfaceComponent, BaseSurfaceComponent>("SurfaceComponent");
	entsMod[defCSurface];

	// auto defCFuncPortal = pragma::LuaCore::create_entity_component_class<pragma::CFuncPortalComponent,pragma::BaseFuncPortalComponent>("FuncPortalComponent");
	// entsMod[defCFuncPortal];

	auto defCLiquidSurf = pragma::LuaCore::create_entity_component_class<CLiquidSurfaceComponent, BaseLiquidSurfaceComponent>("LiquidSurfaceComponent");
	defCLiquidSurf.def(
	  "Test", +[](CLiquidSurfaceComponent &c, prosper::Texture &tex) {
		  auto &scene = c.GetWaterScene();
		  scene.descSetGroupTexEffects->GetDescriptorSet()->SetBindingTexture(tex, 0);
		  scene.descSetGroupTexEffects->GetDescriptorSet()->Update();
	  });
	entsMod[defCLiquidSurf];

	auto defCLiquidVol = pragma::LuaCore::create_entity_component_class<CLiquidVolumeComponent, BaseLiquidVolumeComponent>("LiquidVolumeComponent");
	entsMod[defCLiquidVol];

	auto defCBuoyancy = pragma::LuaCore::create_entity_component_class<CBuoyancyComponent, BaseBuoyancyComponent>("BuoyancyComponent");
	entsMod[defCBuoyancy];

	auto defCWater = pragma::LuaCore::create_entity_component_class<CLiquidComponent, BaseFuncLiquidComponent>("LiquidComponent");
	entsMod[defCWater];

	auto defCButton = pragma::LuaCore::create_entity_component_class<CButtonComponent, BaseFuncButtonComponent>("ButtonComponent");
	entsMod[defCButton];

	auto defCBot = pragma::LuaCore::create_entity_component_class<CBotComponent, BaseBotComponent>("BotComponent");
	entsMod[defCBot];

	auto defCPointConstraintBallSocket = pragma::LuaCore::create_entity_component_class<CPointConstraintBallSocketComponent, BasePointConstraintBallSocketComponent>("PointConstraintBallSocketComponent");
	entsMod[defCPointConstraintBallSocket];

	auto defCPointConstraintConeTwist = pragma::LuaCore::create_entity_component_class<CPointConstraintConeTwistComponent, BasePointConstraintConeTwistComponent>("PointConstraintConeTwistComponent");
	entsMod[defCPointConstraintConeTwist];

	auto defCPointConstraintDoF = pragma::LuaCore::create_entity_component_class<CPointConstraintDoFComponent, BasePointConstraintDoFComponent>("PointConstraintDoFComponent");
	entsMod[defCPointConstraintDoF];

	auto defCPointConstraintFixed = pragma::LuaCore::create_entity_component_class<CPointConstraintFixedComponent, BasePointConstraintFixedComponent>("PointConstraintFixedComponent");
	entsMod[defCPointConstraintFixed];

	auto defCPointConstraintHinge = pragma::LuaCore::create_entity_component_class<CPointConstraintHingeComponent, BasePointConstraintHingeComponent>("PointConstraintHingeComponent");
	entsMod[defCPointConstraintHinge];

	auto defCPointConstraintSlider = pragma::LuaCore::create_entity_component_class<CPointConstraintSliderComponent, BasePointConstraintSliderComponent>("PointConstraintSliderComponent");
	entsMod[defCPointConstraintSlider];

	// auto defCRenderTarget = pragma::LuaCore::create_entity_component_class<pragma::CRenderTargetComponent,pragma::BasePointRenderTargetComponent>("RenderTargetComponent");
	// entsMod[defCRenderTarget];

	auto defCPointTarget = pragma::LuaCore::create_entity_component_class<CPointTargetComponent, BasePointTargetComponent>("PointTargetComponent");
	entsMod[defCPointTarget];

	auto defCProp = pragma::LuaCore::create_entity_component_class<CPropComponent, BasePropComponent>("PropComponent");
	entsMod[defCProp];

	auto defCPropDynamic = pragma::LuaCore::create_entity_component_class<CPropDynamicComponent, BasePropDynamicComponent>("PropDynamicComponent");
	entsMod[defCPropDynamic];

	auto defCPropPhysics = pragma::LuaCore::create_entity_component_class<CPropPhysicsComponent, BasePropPhysicsComponent>("PropPhysicsComponent");
	entsMod[defCPropPhysics];

	auto defCTouch = pragma::LuaCore::create_entity_component_class<CTouchComponent, BaseTouchComponent>("TouchComponent");
	entsMod[defCTouch];

	auto defCSkybox = pragma::LuaCore::create_entity_component_class<CSkyboxComponent, BaseSkyboxComponent>("SkyboxComponent");
	defCSkybox.def("SetSkyMaterial", &CSkyboxComponent::SetSkyMaterial);
	defCSkybox.def("SetSkyAngles", &CSkyboxComponent::SetSkyAngles);
	defCSkybox.def("GetSkyAngles", &CSkyboxComponent::GetSkyAngles);
	entsMod[defCSkybox];

	auto defCFlashlight = pragma::LuaCore::create_entity_component_class<CFlashlightComponent, BaseFlashlightComponent>("FlashlightComponent");
	entsMod[defCFlashlight];

	auto defCEnvSoundProbe = pragma::LuaCore::create_entity_component_class<CEnvSoundProbeComponent, BaseEntityComponent>("EnvSoundProbeComponent");
	entsMod[defCEnvSoundProbe];

	auto defCWeather = pragma::LuaCore::create_entity_component_class<CWeatherComponent, BaseEnvWeatherComponent>("WeatherComponent");
	entsMod[defCWeather];

	auto defCReflectionProbe = pragma::LuaCore::create_entity_component_class<CReflectionProbeComponent, BaseEntityComponent>("ReflectionProbeComponent");
	defCReflectionProbe.def("GetIBLStrength", &CReflectionProbeComponent::GetIBLStrength);
	defCReflectionProbe.def("SetIBLStrength", &CReflectionProbeComponent::SetIBLStrength);
	defCReflectionProbe.def("GetLocationIdentifier", &CReflectionProbeComponent::GetLocationIdentifier);
	defCReflectionProbe.def("GetIBLMaterialFilePath", &CReflectionProbeComponent::GetCubemapIBLMaterialFilePath);
	defCReflectionProbe.def("CaptureIBLReflectionsFromScene", static_cast<bool (*)(lua::State *, CReflectionProbeComponent &, luabind::table<>, bool)>(&reflection_probe_capture_ibl_reflections_from_scene));
	defCReflectionProbe.def("CaptureIBLReflectionsFromScene", static_cast<bool (*)(lua::State *, CReflectionProbeComponent &, luabind::table<>)>(&reflection_probe_capture_ibl_reflections_from_scene));
	defCReflectionProbe.def("CaptureIBLReflectionsFromScene", static_cast<bool (*)(lua::State *, CReflectionProbeComponent &, bool)>(&reflection_probe_capture_ibl_reflections_from_scene));
	defCReflectionProbe.def("CaptureIBLReflectionsFromScene", static_cast<bool (*)(lua::State *, CReflectionProbeComponent &)>(&reflection_probe_capture_ibl_reflections_from_scene));
	defCReflectionProbe.def("RequiresRebuild", &CReflectionProbeComponent::RequiresRebuild);
	defCReflectionProbe.def("GenerateFromEquirectangularImage", &CReflectionProbeComponent::GenerateFromEquirectangularImage);
	entsMod[defCReflectionProbe];

	auto defCSkyCamera = pragma::LuaCore::create_entity_component_class<CSkyCameraComponent, BaseEntityComponent>("SkyCameraComponent");
	entsMod[defCSkyCamera];

	auto defCPBRConverter = pragma::LuaCore::create_entity_component_class<CPBRConverterComponent, BaseEntityComponent>("PBRConverterComponent");
	defCPBRConverter.def("GenerateAmbientOcclusionMaps", static_cast<void (*)(lua::State *, CPBRConverterComponent &, asset::Model &, uint32_t, uint32_t, uint32_t, bool)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	defCPBRConverter.def("GenerateAmbientOcclusionMaps", static_cast<void (*)(lua::State *, CPBRConverterComponent &, asset::Model &, uint32_t, uint32_t, uint32_t)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	defCPBRConverter.def("GenerateAmbientOcclusionMaps", static_cast<void (*)(lua::State *, CPBRConverterComponent &, asset::Model &, uint32_t, uint32_t)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	defCPBRConverter.def("GenerateAmbientOcclusionMaps", static_cast<void (*)(lua::State *, CPBRConverterComponent &, asset::Model &)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	defCPBRConverter.def("GenerateAmbientOcclusionMaps", static_cast<void (*)(lua::State *, CPBRConverterComponent &, ecs::BaseEntity &, uint32_t, uint32_t, uint32_t, bool)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	defCPBRConverter.def("GenerateAmbientOcclusionMaps", static_cast<void (*)(lua::State *, CPBRConverterComponent &, ecs::BaseEntity &, uint32_t, uint32_t, uint32_t)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	defCPBRConverter.def("GenerateAmbientOcclusionMaps", static_cast<void (*)(lua::State *, CPBRConverterComponent &, ecs::BaseEntity &, uint32_t, uint32_t)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	defCPBRConverter.def("GenerateAmbientOcclusionMaps", static_cast<void (*)(lua::State *, CPBRConverterComponent &, ecs::BaseEntity &)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	entsMod[defCPBRConverter];

	auto defShadow = pragma::LuaCore::create_entity_component_class<CShadowComponent, BaseEntityComponent>("ShadowMapComponent");
	entsMod[defShadow];

	auto defShadowCsm = pragma::LuaCore::create_entity_component_class<CShadowCSMComponent, BaseEntityComponent>("CSMComponent");
	entsMod[defShadowCsm];

	auto defShadowManager = pragma::LuaCore::create_entity_component_class<CShadowManagerComponent, BaseEntityComponent>("ShadowManagerComponent");
	entsMod[defShadowManager];

	auto defCWaterSurface = pragma::LuaCore::create_entity_component_class<CWaterSurfaceComponent, BaseEntityComponent>("WaterSurfaceComponent");
	entsMod[defCWaterSurface];

	auto defCListener = pragma::LuaCore::create_entity_component_class<CListenerComponent, BaseEntityComponent>("ListenerComponent");
	entsMod[defCListener];

	auto defCViewBody = pragma::LuaCore::create_entity_component_class<CViewBodyComponent, BaseEntityComponent>("ViewBodyComponent");
	entsMod[defCViewBody];

	auto defCViewModel = pragma::LuaCore::create_entity_component_class<CViewModelComponent, BaseEntityComponent>("ViewModelComponent");
	defCViewModel.def("GetPlayer", &CViewModelComponent::GetPlayer);
	defCViewModel.def("GetWeapon", &CViewModelComponent::GetWeapon);
	entsMod[defCViewModel];

	auto defCSoftBody = pragma::LuaCore::create_entity_component_class<CSoftBodyComponent, BaseSoftBodyComponent>("SoftBodyComponent");
	entsMod[defCSoftBody];

	auto defCRaytracing = pragma::LuaCore::create_entity_component_class<CRaytracingComponent, BaseEntityComponent>("RaytracingComponent");
	entsMod[defCRaytracing];

	auto defCBSPLeaf = pragma::LuaCore::create_entity_component_class<CBSPLeafComponent, BaseEntityComponent>("BSPLeafComponent");
	entsMod[defCBSPLeaf];

	auto defCIo = pragma::LuaCore::create_entity_component_class<CIOComponent, BaseIOComponent>("IOComponent");
	entsMod[defCIo];

	auto defCTimeScale = pragma::LuaCore::create_entity_component_class<CTimeScaleComponent, BaseTimeScaleComponent>("TimeScaleComponent");
	entsMod[defCTimeScale];

	auto defCAttachable = pragma::LuaCore::create_entity_component_class<CAttachmentComponent, BaseAttachmentComponent>("AttachmentComponent");
	entsMod[defCAttachable];

	auto defCChild = pragma::LuaCore::create_entity_component_class<CChildComponent, BaseChildComponent>("ChildComponent");
	entsMod[defCChild];

	auto defCOwnable = pragma::LuaCore::create_entity_component_class<COwnableComponent, BaseOwnableComponent>("OwnableComponent");
	entsMod[defCOwnable];

	auto defCDebugText = pragma::LuaCore::create_entity_component_class<CDebugTextComponent, BaseDebugTextComponent>("DebugTextComponent");
	entsMod[defCDebugText];

	auto defCDebugPoint = pragma::LuaCore::create_entity_component_class<CDebugPointComponent, BaseDebugPointComponent>("DebugPointComponent");
	entsMod[defCDebugPoint];

	auto defCDebugLine = pragma::LuaCore::create_entity_component_class<CDebugLineComponent, BaseDebugLineComponent>("DebugLineComponent");
	entsMod[defCDebugLine];

	auto defCDebugBox = pragma::LuaCore::create_entity_component_class<CDebugBoxComponent, BaseDebugBoxComponent>("DebugBoxComponent");
	defCDebugBox.def("SetColorOverride", &CDebugBoxComponent::SetColorOverride);
	defCDebugBox.def("ClearColorOverride", &CDebugBoxComponent::ClearColorOverride);
	defCDebugBox.def("GetColorOverride", &CDebugBoxComponent::GetColorOverride);
	defCDebugBox.def("SetIgnoreDepthBuffer", &CDebugBoxComponent::SetIgnoreDepthBuffer);
	defCDebugBox.def("ShouldIgnoreDepthBuffer", &CDebugBoxComponent::ShouldIgnoreDepthBuffer);
	entsMod[defCDebugBox];

	auto defCDebugSphere = pragma::LuaCore::create_entity_component_class<CDebugSphereComponent, BaseDebugSphereComponent>("DebugSphereComponent");
	entsMod[defCDebugSphere];

	auto defCDebugCone = pragma::LuaCore::create_entity_component_class<CDebugConeComponent, BaseDebugConeComponent>("DebugConeComponent");
	entsMod[defCDebugCone];

	auto defCDebugCylinder = pragma::LuaCore::create_entity_component_class<CDebugCylinderComponent, BaseDebugCylinderComponent>("DebugCylinderComponent");
	entsMod[defCDebugCylinder];

	auto defCDebugPlane = pragma::LuaCore::create_entity_component_class<CDebugPlaneComponent, BaseDebugPlaneComponent>("DebugPlaneComponent");
	entsMod[defCDebugPlane];

	auto defCPointAtTarget = pragma::LuaCore::create_entity_component_class<CPointAtTargetComponent, BasePointAtTargetComponent>("PointAtTargetComponent");
	entsMod[defCPointAtTarget];

	auto defCBSP = pragma::LuaCore::create_entity_component_class<CBSPComponent, BaseEntityComponent>("BSPComponent");
	entsMod[defCBSP];

	auto defCGeneric = pragma::LuaCore::create_entity_component_class<CGenericComponent, BaseGenericComponent>("GenericComponent");
	entsMod[defCGeneric];

	RegisterLuaEntityComponents2_cl(l, entsMod);
}
