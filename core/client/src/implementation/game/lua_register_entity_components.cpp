// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/policies/default_parameter_policy.hpp"

#include "mathutil/transform.hpp"

#include "pragma/lua/luaapi.h"
#include "pragma/lua/ostream_operator_alias.hpp"
#include "mathutil/umath.h"

#include "stdafx_client.h"
#include <prosper_command_buffer.hpp>
#include <prosper_render_pass.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <image/prosper_render_target.hpp>
#include <util_image_buffer.hpp>

module pragma.client;

import :game;
import :scripting.lua.bindings.scene;

namespace Lua {
	namespace PBRConverter {
		static void GenerateAmbientOcclusionMaps(lua_State *l, pragma::CPBRConverterComponent &hComponent, ::Model &mdl, uint32_t width, uint32_t height, uint32_t samples, bool rebuild) { hComponent.GenerateAmbientOcclusionMaps(mdl, width, height, samples, rebuild); }
		static void GenerateAmbientOcclusionMaps(lua_State *l, pragma::CPBRConverterComponent &hComponent, ::Model &mdl, uint32_t width, uint32_t height, uint32_t samples) { hComponent.GenerateAmbientOcclusionMaps(mdl, width, height, samples); }
		static void GenerateAmbientOcclusionMaps(lua_State *l, pragma::CPBRConverterComponent &hComponent, ::Model &mdl, uint32_t width, uint32_t height) { hComponent.GenerateAmbientOcclusionMaps(mdl, width, height); }
		static void GenerateAmbientOcclusionMaps(lua_State *l, pragma::CPBRConverterComponent &hComponent, ::Model &mdl) { hComponent.GenerateAmbientOcclusionMaps(mdl); }

		static void GenerateAmbientOcclusionMaps(lua_State *l, pragma::CPBRConverterComponent &hComponent, BaseEntity &ent, uint32_t width, uint32_t height, uint32_t samples, bool rebuild) { hComponent.GenerateAmbientOcclusionMaps(ent, width, height, samples, rebuild); }
		static void GenerateAmbientOcclusionMaps(lua_State *l, pragma::CPBRConverterComponent &hComponent, BaseEntity &ent, uint32_t width, uint32_t height, uint32_t samples) { hComponent.GenerateAmbientOcclusionMaps(ent, width, height, samples); }
		static void GenerateAmbientOcclusionMaps(lua_State *l, pragma::CPBRConverterComponent &hComponent, BaseEntity &ent, uint32_t width, uint32_t height) { hComponent.GenerateAmbientOcclusionMaps(ent, width, height); }
		static void GenerateAmbientOcclusionMaps(lua_State *l, pragma::CPBRConverterComponent &hComponent, BaseEntity &ent) { hComponent.GenerateAmbientOcclusionMaps(ent); }
	};
	namespace Decal {
		static void create_from_projection(lua_State *l, pragma::CDecalComponent &hComponent, luabind::object tMeshes, const umath::ScaledTransform &pose)
		{

			int32_t t = 2;
			Lua::CheckTable(l, t);
			std::vector<pragma::DecalProjector::MeshData> meshesDatas {};
			auto numMeshes = Lua::GetObjectLength(l, t);
			meshesDatas.reserve(numMeshes);
			for(auto i = decltype(numMeshes) {0u}; i < numMeshes; ++i) {
				meshesDatas.push_back({});
				auto &meshData = meshesDatas.back();
				Lua::PushInt(l, i + 1);   /* 1 */
				Lua::GetTableValue(l, t); /* 1 */

				auto tMeshData = Lua::GetStackTop(l);
				Lua::CheckTable(l, tMeshData);

				Lua::PushString(l, "pose");       /* 2 */
				Lua::GetTableValue(l, tMeshData); /* 2 */
				meshData.pose = Lua::Check<umath::ScaledTransform>(l, -1);
				Lua::Pop(l, 1); /* 1 */

				Lua::PushString(l, "subMeshes");  /* 2 */
				Lua::GetTableValue(l, tMeshData); /* 2 */
				auto tSubMeshes = Lua::GetStackTop(l);
				Lua::CheckTable(l, tSubMeshes);
				auto numSubMeshes = Lua::GetObjectLength(l, tSubMeshes);
				meshData.subMeshes.reserve(numSubMeshes);
				for(auto j = decltype(numSubMeshes) {0u}; j < numSubMeshes; ++j) {
					Lua::PushInt(l, j + 1);            /* 1 */
					Lua::GetTableValue(l, tSubMeshes); /* 1 */
					auto &mesh = Lua::Check<::ModelSubMesh>(l, -1);
					meshData.subMeshes.push_back(&mesh);
					Lua::Pop(l, 1); /* 0 */
				}
				Lua::Pop(l, 1); /* 1 */

				Lua::Pop(l, 1); /* 0 */
			}
			Lua::PushBool(l, hComponent.ApplyDecal(meshesDatas));
		}
		static void create_from_projection(lua_State *l, pragma::CDecalComponent &hComponent, luabind::object tMeshes) { create_from_projection(l, hComponent, tMeshes, {}); }
	};
};

static bool reflection_probe_capture_ibl_reflections_from_scene(lua_State *l, pragma::CReflectionProbeComponent &hRp, luabind::table<> tEnts, bool renderJob)
{

	std::vector<BaseEntity *> ents {};
	ents.reserve(Lua::GetObjectLength(l, 2));
	for(auto it = luabind::iterator {tEnts}, end = luabind::iterator {}; it != end; ++it) {
		auto val = luabind::object_cast_nothrow<EntityHandle>(*it, EntityHandle {});
		if(val.expired())
			return false;
		ents.push_back(val.get());
	}
	return hRp.CaptureIBLReflectionsFromScene(&ents, renderJob);
}
static bool reflection_probe_capture_ibl_reflections_from_scene(lua_State *l, pragma::CReflectionProbeComponent &hRp, luabind::table<> tEnts) { return reflection_probe_capture_ibl_reflections_from_scene(l, hRp, tEnts, false); }
static bool reflection_probe_capture_ibl_reflections_from_scene(lua_State *l, pragma::CReflectionProbeComponent &hRp, bool renderJob) { return hRp.CaptureIBLReflectionsFromScene(nullptr, renderJob); }
static bool reflection_probe_capture_ibl_reflections_from_scene(lua_State *l, pragma::CReflectionProbeComponent &hRp) { return hRp.CaptureIBLReflectionsFromScene(); }

namespace pragma {
	template<typename T>
	    requires(std::is_same_v<T, pragma::CSoundDspChorusComponent> || std::is_same_v<T, pragma::CSoundDspDistortionComponent> || std::is_same_v<T, pragma::CSoundDspEAXReverbComponent> || std::is_same_v<T, pragma::CSoundDspEchoComponent>
	      || std::is_same_v<T, pragma::CSoundDspEqualizerComponent> || std::is_same_v<T, pragma::CSoundDspFlangerComponent> || std::is_same_v<T, pragma::CLightDirectionalComponent> || std::is_same_v<T, pragma::CLightPointComponent> || std::is_same_v<T, pragma::CLightSpotComponent>
	      || std::is_same_v<T, pragma::CLiquidComponent> || std::is_same_v<T, pragma::CWaterSurfaceComponent>)
	static std::ostream &operator<<(std::ostream &os, const T &component)
	{
		return ::operator<<(os, static_cast<const pragma::BaseEntityComponent &>(component));
	}
};

static void bsp_register_class(lua_State *l, luabind::module_ &entsMod, luabind::class_<pragma::CWorldComponent, pragma::BaseWorldComponent> &defWorld)
{
	auto defBspTree = luabind::class_<::util::BSPTree>("BSPTree");
	defBspTree.def("IsValid", static_cast<void (*)(lua_State *, ::util::BSPTree &)>([](lua_State *l, ::util::BSPTree &tree) { Lua::PushBool(l, tree.IsValid()); }));
	defBspTree.def("IsClusterVisible", static_cast<void (*)(lua_State *, ::util::BSPTree &, uint32_t, uint32_t)>([](lua_State *l, ::util::BSPTree &tree, uint32_t clusterSrc, uint32_t clusterDst) { Lua::PushBool(l, tree.IsClusterVisible(clusterSrc, clusterDst)); }));
	defBspTree.def("GetRootNode", static_cast<void (*)(lua_State *, ::util::BSPTree &)>([](lua_State *l, ::util::BSPTree &tree) {
		auto &node = tree.GetRootNode();
		Lua::Push(l, &node);
	}));
	defBspTree.def("GetNodes", static_cast<void (*)(lua_State *, ::util::BSPTree &)>([](lua_State *l, ::util::BSPTree &tree) {
		auto &nodes = tree.GetNodes();
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto &node : nodes) {
			Lua::PushInt(l, idx++);
			Lua::Push(l, node);
			Lua::SetTableValue(l, t);
		}
	}));
	defBspTree.def("GetClusterVisibility", static_cast<void (*)(lua_State *, ::util::BSPTree &)>([](lua_State *l, ::util::BSPTree &tree) {
		auto &clusterVisibility = tree.GetClusterVisibility();
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto vis : clusterVisibility) {
			Lua::PushInt(l, idx++);
			Lua::PushInt(l, vis);
			Lua::SetTableValue(l, t);
		}
	}));
	defBspTree.def("GetClusterCount", static_cast<void (*)(lua_State *, ::util::BSPTree &)>([](lua_State *l, ::util::BSPTree &tree) { Lua::PushInt(l, tree.GetClusterCount()); }));
	defBspTree.def("FindLeafNode", static_cast<void (*)(lua_State *, ::util::BSPTree &, const Vector3 &)>([](lua_State *l, ::util::BSPTree &tree, const Vector3 &origin) {
		auto *node = tree.FindLeafNode(origin);
		if(node == nullptr)
			return;
		Lua::Push(l, &node);
	}));
	defBspTree.def("FindLeafNodesInAABB", static_cast<luabind::object (*)(lua_State *, ::util::BSPTree &, const Vector3 &, const Vector3 &)>([](lua_State *l, ::util::BSPTree &tree, const Vector3 &min, const Vector3 &max) -> luabind::object {
		auto nodes = tree.FindLeafNodesInAabb(min, max);
		auto t = luabind::newtable(l);
		int32_t idx = 1;
		for(auto &n : nodes)
			t[idx++] = n;
		return t;
	}));

	auto defBspNode = luabind::class_<::util::BSPTree::Node>("Node");
	defBspNode.def("GetIndex", static_cast<::util::BSPTree::ChildIndex (*)(lua_State *, ::util::BSPTree::Node &)>([](lua_State *l, ::util::BSPTree::Node &node) -> ::util::BSPTree::ChildIndex { return node.index; }));
	defBspNode.def("IsLeaf", static_cast<void (*)(lua_State *, ::util::BSPTree::Node &)>([](lua_State *l, ::util::BSPTree::Node &node) { Lua::PushBool(l, node.leaf); }));
	defBspNode.def("GetBounds", static_cast<void (*)(lua_State *, ::util::BSPTree::Node &)>([](lua_State *l, ::util::BSPTree::Node &node) {
		Lua::Push<Vector3>(l, node.min);
		Lua::Push<Vector3>(l, node.max);
	}));
	defBspNode.def("GetChildren", static_cast<void (*)(lua_State *, ::util::BSPTree::Node &)>([](lua_State *l, ::util::BSPTree::Node &node) {
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto &child : node.children) {
			Lua::PushInt(l, idx++);
			Lua::Push(l, child);
		}
	}));
	defBspNode.def("GetCluster", static_cast<void (*)(lua_State *, ::util::BSPTree::Node &)>([](lua_State *l, ::util::BSPTree::Node &node) { Lua::PushInt(l, node.cluster); }));
	defBspNode.def("GetVisibleLeafAreaBounds", static_cast<void (*)(lua_State *, ::util::BSPTree::Node &)>([](lua_State *l, ::util::BSPTree::Node &node) {
		Lua::Push<Vector3>(l, node.minVisible);
		Lua::Push<Vector3>(l, node.maxVisible);
	}));
	defBspNode.def("GetInternalNodePlane", static_cast<void (*)(lua_State *, ::util::BSPTree::Node &)>([](lua_State *l, ::util::BSPTree::Node &node) { Lua::Push<umath::Plane>(l, node.plane); }));
	defBspNode.def("GetInternalNodeFirstFaceIndex", static_cast<void (*)(lua_State *, ::util::BSPTree::Node &)>([](lua_State *l, ::util::BSPTree::Node &node) { Lua::PushInt(l, node.firstFace); }));
	defBspNode.def("GetInternalNodeFaceCount", static_cast<void (*)(lua_State *, ::util::BSPTree::Node &)>([](lua_State *l, ::util::BSPTree::Node &node) { Lua::PushInt(l, node.numFaces); }));
	defBspTree.scope[defBspNode];
	defWorld.scope[defBspTree];
}

DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CGamemodeComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CGameComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CInputComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CColorComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CScoreComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CFlammableComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CHealthComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CNameComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CNetworkedComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CObservableComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CObserverComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, ecs::CShooterComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CPhysicsComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CRadiusComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CFieldAngleComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CWorldComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CEyeComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CSceneComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CToggleComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CTransformComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CWheelComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CSoundDspComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CSoundDspChorusComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CSoundDspDistortionComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CSoundDspEAXReverbComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CSoundDspEchoComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CSoundDspEqualizerComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CSoundDspFlangerComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CCameraComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, COcclusionCullerComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CDecalComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CExplosionComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CFireComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CFogControllerComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CLightComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CLightDirectionalComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CLightPointComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CLightSpotComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CLightSpotVolComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CMicrophoneComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CQuakeComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CSmokeTrailComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CSoundComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CSoundScapeComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CSpriteComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CEnvTimescaleComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CWindComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CFilterClassComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CFilterNameComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CBrushComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CKinematicComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CFuncPhysicsComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CFuncSoftPhysicsComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CSurfaceComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CLiquidSurfaceComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CLiquidVolumeComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CBuoyancyComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CLiquidComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CButtonComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CBotComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CPointConstraintBallSocketComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CPointConstraintConeTwistComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CPointConstraintDoFComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CPointConstraintFixedComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CPointConstraintHingeComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CPointConstraintSliderComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CPointTargetComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CPropComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CPropDynamicComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CPropPhysicsComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CTouchComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CSkyboxComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CFlashlightComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CEnvSoundProbeComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CWeatherComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CReflectionProbeComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CSkyCameraComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CPBRConverterComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CShadowComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CShadowCSMComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CShadowManagerComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CWaterSurfaceComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CListenerComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CViewBodyComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CViewModelComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CSoftBodyComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CRaytracingComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CBSPLeafComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CIOComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CTimeScaleComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CAttachmentComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CChildComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, COwnableComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CDebugTextComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CDebugPointComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CDebugLineComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CDebugBoxComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CDebugSphereComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CDebugConeComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CDebugCylinderComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CDebugPlaneComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CPointAtTargetComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CBSPComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, CGenericComponent);

void RegisterLuaEntityComponents2_cl(lua_State *l, luabind::module_ &entsMod);
void CGame::RegisterLuaEntityComponents(luabind::module_ &entsMod)
{
	Game::RegisterLuaEntityComponents(entsMod);
	auto *l = GetLuaState();

	auto defCGamemode = pragma::lua::create_entity_component_class<pragma::CGamemodeComponent, pragma::BaseGamemodeComponent>("GamemodeComponent");
	entsMod[defCGamemode];

	auto defCGame = pragma::lua::create_entity_component_class<pragma::CGameComponent, pragma::BaseGameComponent>("GameComponent");
	entsMod[defCGame];

	auto defCInput = pragma::lua::create_entity_component_class<pragma::CInputComponent, pragma::BaseEntityComponent>("InputComponent");
	defCInput.def("GetMouseDeltaX", &pragma::CInputComponent::GetMouseDeltaX);
	defCInput.def("GetMouseDeltaY", &pragma::CInputComponent::GetMouseDeltaY);
	entsMod[defCInput];

	auto defCColor = pragma::lua::create_entity_component_class<pragma::CColorComponent, pragma::BaseColorComponent>("ColorComponent");
	entsMod[defCColor];

	auto defCScore = pragma::lua::create_entity_component_class<pragma::CScoreComponent, pragma::BaseScoreComponent>("ScoreComponent");
	entsMod[defCScore];

	auto defCFlammable = pragma::lua::create_entity_component_class<pragma::CFlammableComponent, pragma::BaseFlammableComponent>("FlammableComponent");
	entsMod[defCFlammable];

	auto defCHealth = pragma::lua::create_entity_component_class<pragma::CHealthComponent, pragma::BaseHealthComponent>("HealthComponent");
	entsMod[defCHealth];

	auto defCName = pragma::lua::create_entity_component_class<pragma::CNameComponent, pragma::BaseNameComponent>("NameComponent");
	entsMod[defCName];

	auto defCNetworked = pragma::lua::create_entity_component_class<pragma::CNetworkedComponent, pragma::BaseNetworkedComponent>("NetworkedComponent");
	entsMod[defCNetworked];

	auto defCObservable = pragma::lua::create_entity_component_class<pragma::CObservableComponent, pragma::BaseObservableComponent>("ObservableComponent");
	entsMod[defCObservable];

	auto defCObserver = pragma::lua::create_entity_component_class<pragma::CObserverComponent, pragma::BaseObserverComponent>("ObserverComponent");
	entsMod[defCObserver];

	auto defCShooter = pragma::lua::create_entity_component_class<pragma::ecs::CShooterComponent, pragma::ecs::BaseShooterComponent>("ShooterComponent");
	entsMod[defCShooter];

	auto defCPhysics = pragma::lua::create_entity_component_class<pragma::CPhysicsComponent, pragma::BasePhysicsComponent>("PhysicsComponent");
	entsMod[defCPhysics];

	auto defCRadius = pragma::lua::create_entity_component_class<pragma::CRadiusComponent, pragma::BaseRadiusComponent>("RadiusComponent");
	entsMod[defCRadius];

	auto defCFieldAngle = pragma::lua::create_entity_component_class<pragma::CFieldAngleComponent, pragma::BaseFieldAngleComponent>("FieldAngleComponent");
	entsMod[defCFieldAngle];

	auto defCWorld = pragma::lua::create_entity_component_class<pragma::CWorldComponent, pragma::BaseWorldComponent>("WorldComponent");
	defCWorld.def("GetBSPTree", &pragma::CWorldComponent::GetBSPTree);
	defCWorld.def("RebuildRenderQueues", &pragma::CWorldComponent::RebuildRenderQueues);

	auto defCEye = pragma::lua::create_entity_component_class<pragma::CEyeComponent, pragma::BaseEntityComponent>("EyeComponent");
	defCEye.def("GetEyePose", &pragma::CEyeComponent::GetEyePose);
	defCEye.def("GetViewTarget", &pragma::CEyeComponent::GetViewTarget);
	defCEye.def("SetViewTarget", &pragma::CEyeComponent::SetViewTarget);
	defCEye.def("ClearViewTarget", &pragma::CEyeComponent::ClearViewTarget);
	defCEye.def("GetEyeShift", static_cast<std::optional<Vector3> (*)(lua_State *, pragma::CEyeComponent &, uint32_t)>([](lua_State *l, pragma::CEyeComponent &hEye, uint32_t eyeIndex) -> std::optional<Vector3> {
		auto *config = hEye.GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return {};
		return config->eyeShift;
	}));
	defCEye.def("SetEyeShift", static_cast<void (*)(lua_State *, pragma::CEyeComponent &, uint32_t, const Vector3 &)>([](lua_State *l, pragma::CEyeComponent &hEye, uint32_t eyeIndex, const Vector3 &eyeShift) {
		auto *config = hEye.GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return;
		config->eyeShift = eyeShift;
	}));
	defCEye.def("GetEyeJitter", static_cast<std::optional<Vector2> (*)(lua_State *, pragma::CEyeComponent &, uint32_t)>([](lua_State *l, pragma::CEyeComponent &hEye, uint32_t eyeIndex) -> std::optional<Vector2> {
		auto *config = hEye.GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return {};
		return config->jitter;
	}));
	defCEye.def("SetEyeJitter", static_cast<void (*)(lua_State *, pragma::CEyeComponent &, uint32_t, const Vector2 &)>([](lua_State *l, pragma::CEyeComponent &hEye, uint32_t eyeIndex, const Vector2 &eyeJitter) {
		auto *config = hEye.GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return;
		config->jitter = eyeJitter;
	}));
	defCEye.def("GetEyeSize", static_cast<std::optional<float> (*)(lua_State *, pragma::CEyeComponent &, uint32_t)>([](lua_State *l, pragma::CEyeComponent &hEye, uint32_t eyeIndex) -> std::optional<float> {
		auto *config = hEye.GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return {};
		return config->eyeSize;
	}));
	defCEye.def("SetEyeSize", static_cast<void (*)(lua_State *, pragma::CEyeComponent &, uint32_t, float)>([](lua_State *l, pragma::CEyeComponent &hEye, uint32_t eyeIndex, float eyeSize) {
		auto *config = hEye.GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return;
		config->eyeSize = eyeSize;
	}));
	defCEye.def("SetIrisDilation", static_cast<void (*)(lua_State *, pragma::CEyeComponent &, uint32_t, float)>([](lua_State *l, pragma::CEyeComponent &hEye, uint32_t eyeIndex, float dilation) {
		auto *config = hEye.GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return;
		config->dilation = dilation;
	}));
	defCEye.def("GetIrisDilation", static_cast<std::optional<float> (*)(lua_State *, pragma::CEyeComponent &, uint32_t)>([](lua_State *l, pragma::CEyeComponent &hEye, uint32_t eyeIndex) -> std::optional<float> {
		auto *config = hEye.GetEyeballConfig(eyeIndex);
		if(config == nullptr)
			return {};
		return config->dilation;
	}));
	defCEye.def("CalcEyeballPose", static_cast<std::pair<umath::Transform, umath::Transform> (*)(lua_State *, pragma::CEyeComponent &, uint32_t)>([](lua_State *l, pragma::CEyeComponent &hEye, uint32_t eyeIndex) -> std::pair<umath::Transform, umath::Transform> {
		umath::Transform bonePose;
		auto pose = hEye.CalcEyeballPose(eyeIndex, &bonePose);
		return {pose, bonePose};
	}));
	defCEye.def(
	  "FindEyeballIndex", +[](lua_State *l, pragma::CEyeComponent &hEye, uint32_t skinMatIdx) -> std::optional<uint32_t> {
		  uint32_t eyeballIdx;
		  if(!hEye.FindEyeballIndex(skinMatIdx, eyeballIdx))
			  return {};
		  return eyeballIdx;
	  });
	defCEye.def(
	  "GetEyeballProjectionVectors", +[](lua_State *l, pragma::CEyeComponent &hEye, uint32_t eyeballIndex) -> std::optional<std::pair<Vector4, Vector4>> {
		  Vector4 projU, projV;
		  if(!hEye.GetEyeballProjectionVectors(eyeballIndex, projU, projV))
			  return {};
		  return std::pair<Vector4, Vector4> {projU, projV};
	  });
	defCEye.def("GetEyeballState", static_cast<pragma::CEyeComponent::EyeballState *(*)(lua_State *, pragma::CEyeComponent &, uint32_t)>([](lua_State *l, pragma::CEyeComponent &hEye, uint32_t eyeIndex) -> pragma::CEyeComponent::EyeballState * {
		auto *eyeballData = hEye.GetEyeballData(eyeIndex);
		if(eyeballData == nullptr)
			return nullptr;
		return &eyeballData->state;
	}));
	defCEye.def("SetBlinkDuration", &pragma::CEyeComponent::SetBlinkDuration);
	defCEye.def("GetBlinkDuration", &pragma::CEyeComponent::GetBlinkDuration);
	defCEye.def("SetBlinkingEnabled", &pragma::CEyeComponent::SetBlinkingEnabled);
	defCEye.def("IsBlinkingEnabled", &pragma::CEyeComponent::IsBlinkingEnabled);

	auto defEyeballState = luabind::class_<pragma::CEyeComponent::EyeballState>("EyeballState");
	defEyeballState.def_readwrite("origin", &pragma::CEyeComponent::EyeballState::origin);
	defEyeballState.def_readwrite("forward", &pragma::CEyeComponent::EyeballState::forward);
	defEyeballState.def_readwrite("right", &pragma::CEyeComponent::EyeballState::right);
	defEyeballState.def_readwrite("up", &pragma::CEyeComponent::EyeballState::up);
	defEyeballState.def_readwrite("irisProjectionU", &pragma::CEyeComponent::EyeballState::irisProjectionU);
	defEyeballState.def_readwrite("irisProjectionV", &pragma::CEyeComponent::EyeballState::irisProjectionV);
	defCEye.scope[defEyeballState];

	// defCEye.add_static_constant("EVENT_ON_EYEBALLS_UPDATED",pragma::CEyeComponent::EVENT_ON_EYEBALLS_UPDATED);
	// defCEye.add_static_constant("EVENT_ON_BLINK",pragma::CEyeComponent::EVENT_ON_BLINK);
	entsMod[defCEye];

	pragma::scripting::lua::bindings::register_renderers(l, entsMod);

	auto defCScene = pragma::lua::create_entity_component_class<pragma::CSceneComponent, pragma::BaseEntityComponent>("SceneComponent");
	defCScene.add_static_constant("OCCLUSION_CULLING_METHOD_BRUTE_FORCE", umath::to_integral(SceneRenderDesc::OcclusionCullingMethod::BruteForce));
	defCScene.add_static_constant("OCCLUSION_CULLING_METHOD_CHC_PLUSPLUS", umath::to_integral(SceneRenderDesc::OcclusionCullingMethod::CHCPP));
	defCScene.add_static_constant("OCCLUSION_CULLING_METHOD_BSP", umath::to_integral(SceneRenderDesc::OcclusionCullingMethod::BSP));
	defCScene.add_static_constant("OCCLUSION_CULLING_METHOD_OCTREE", umath::to_integral(SceneRenderDesc::OcclusionCullingMethod::Octree));
	defCScene.add_static_constant("OCCLUSION_CULLING_METHOD_INERT", umath::to_integral(SceneRenderDesc::OcclusionCullingMethod::Inert));
	defCScene.add_static_constant("EVENT_ON_ACTIVE_CAMERA_CHANGED", pragma::CSceneComponent::EVENT_ON_ACTIVE_CAMERA_CHANGED);
	defCScene.add_static_constant("EVENT_ON_RENDERER_CHANGED", pragma::CSceneComponent::EVENT_ON_RENDERER_CHANGED);
	defCScene.add_static_constant("DEBUG_MODE_NONE", umath::to_integral(pragma::SceneDebugMode::None));
	defCScene.add_static_constant("DEBUG_MODE_AMBIENT_OCCLUSION", umath::to_integral(pragma::SceneDebugMode::AmbientOcclusion));
	defCScene.add_static_constant("DEBUG_MODE_ALBEDO", umath::to_integral(pragma::SceneDebugMode::Albedo));
	defCScene.add_static_constant("DEBUG_MODE_METALNESS", umath::to_integral(pragma::SceneDebugMode::Metalness));
	defCScene.add_static_constant("DEBUG_MODE_ROUGHNESS", umath::to_integral(pragma::SceneDebugMode::Roughness));
	defCScene.add_static_constant("DEBUG_MODE_DIFFUSE_LIGHTING", umath::to_integral(pragma::SceneDebugMode::DiffuseLighting));
	defCScene.add_static_constant("DEBUG_MODE_NORMAL", umath::to_integral(pragma::SceneDebugMode::Normal));
	defCScene.add_static_constant("DEBUG_MODE_NORMAL_MAP", umath::to_integral(pragma::SceneDebugMode::NormalMap));
	defCScene.add_static_constant("DEBUG_MODE_REFLECTANCE", umath::to_integral(pragma::SceneDebugMode::Reflectance));
	defCScene.add_static_constant("DEBUG_MODE_IBL_PREFILTER", umath::to_integral(pragma::SceneDebugMode::IBLPrefilter));
	defCScene.add_static_constant("DEBUG_MODE_IBL_IRRADIANCE", umath::to_integral(pragma::SceneDebugMode::IBLIrradiance));
	defCScene.add_static_constant("DEBUG_MODE_EMISSION", umath::to_integral(pragma::SceneDebugMode::Emission));
	defCScene.def("GetActiveCamera", static_cast<pragma::ComponentHandle<pragma::CCameraComponent> &(pragma::CSceneComponent::*)()>(&pragma::CSceneComponent::GetActiveCamera));
	defCScene.def("SetActiveCamera", static_cast<void (pragma::CSceneComponent::*)(pragma::CCameraComponent &)>(&pragma::CSceneComponent::SetActiveCamera));
	defCScene.def("SetActiveCamera", static_cast<void (pragma::CSceneComponent::*)()>(&pragma::CSceneComponent::SetActiveCamera));
	defCScene.def("SetExclusionRenderMask", &pragma::CSceneComponent::SetExclusionRenderMask);
	defCScene.def("GetExclusionRenderMask", &pragma::CSceneComponent::GetExclusionRenderMask);
	defCScene.def("SetInclusionRenderMask", &pragma::CSceneComponent::SetInclusionRenderMask);
	defCScene.def("GetInclusionRenderMask", &pragma::CSceneComponent::GetInclusionRenderMask);
	defCScene.def("GetWidth", &pragma::CSceneComponent::GetWidth);
	defCScene.def("GetHeight", &pragma::CSceneComponent::GetHeight);
	defCScene.def("GetSize", static_cast<std::pair<uint32_t, uint32_t> (*)(const pragma::CSceneComponent &)>([](const pragma::CSceneComponent &scene) -> std::pair<uint32_t, uint32_t> { return {scene.GetWidth(), scene.GetHeight()}; }));
	defCScene.def("Resize", &pragma::CSceneComponent::Resize);
	// defCScene.def("BeginDraw",&pragma::CSceneComponent::BeginDraw);
	defCScene.def("UpdateBuffers", +[](lua_State *l, pragma::CSceneComponent &scene, prosper::ICommandBuffer &hCommandBuffer) {
		if(hCommandBuffer.IsPrimary() == false)
			return;
		auto pCmdBuffer = std::dynamic_pointer_cast<prosper::IPrimaryCommandBuffer>(hCommandBuffer.shared_from_this());
		scene.UpdateBuffers(pCmdBuffer);
	});
	defCScene.def("GetWorldEnvironment", +[](lua_State *l, pragma::CSceneComponent &scene) -> std::shared_ptr<WorldEnvironment> {
		auto *worldEnv = scene.GetWorldEnvironment();
		if(worldEnv == nullptr)
			return nullptr;
		return worldEnv->shared_from_this();
	});
	defCScene.def("SetWorldEnvironment", &pragma::CSceneComponent::SetWorldEnvironment);
	defCScene.def("ClearWorldEnvironment", &pragma::CSceneComponent::ClearWorldEnvironment);
	defCScene.def("InitializeRenderTarget", &pragma::CSceneComponent::ReloadRenderTarget);

	defCScene.def("GetIndex", static_cast<pragma::CSceneComponent::SceneIndex (pragma::CSceneComponent::*)() const>(&pragma::CSceneComponent::GetSceneIndex));
	defCScene.def("GetCameraDescriptorSet", static_cast<const std::shared_ptr<prosper::IDescriptorSetGroup> &(pragma::CSceneComponent::*)(prosper::PipelineBindPoint) const>(&pragma::CSceneComponent::GetCameraDescriptorSetGroup));
	defCScene.def("GetCameraDescriptorSet", static_cast<const std::shared_ptr<prosper::IDescriptorSetGroup> &(*)(const pragma::CSceneComponent &)>([](const pragma::CSceneComponent &scene) -> const std::shared_ptr<prosper::IDescriptorSetGroup> & {
		return scene.GetCameraDescriptorSetGroup(prosper::PipelineBindPoint::Graphics);
	}));
	defCScene.def("GetViewCameraDescriptorSet", &pragma::CSceneComponent::GetViewCameraDescriptorSet);
	defCScene.def("GetDebugMode", &pragma::CSceneComponent::GetDebugMode);
	defCScene.def("SetDebugMode", &pragma::CSceneComponent::SetDebugMode);
	defCScene.def("Link", &pragma::CSceneComponent::Link, luabind::default_parameter_policy<3, true> {});
	defCScene.def("Link", &pragma::CSceneComponent::Link);
	// defCScene.def("BuildRenderQueue",&pragma::CSceneComponent::BuildRenderQueue);
	defCScene.def("GetRenderer", static_cast<pragma::CRendererComponent *(pragma::CSceneComponent::*)()>(&pragma::CSceneComponent::GetRenderer<pragma::CRendererComponent>));
	defCScene.def("SetRenderer", &pragma::CSceneComponent::SetRenderer<pragma::CRendererComponent>);
	defCScene.def("GetSceneIndex", static_cast<pragma::CSceneComponent::SceneIndex (pragma::CSceneComponent::*)() const>(&pragma::CSceneComponent::GetSceneIndex));
	defCScene.def("SetParticleSystemColorFactor", &pragma::CSceneComponent::SetParticleSystemColorFactor);
	defCScene.def("GetParticleSystemColorFactor", &pragma::CSceneComponent::GetParticleSystemColorFactor, luabind::copy_policy<0> {});
	//defCScene.def("GetRenderParticleSystems",static_cast<std::vector<pragma::ecs::CParticleSystemComponent*>(*)(lua_State*,pragma::CSceneComponent&)>([](lua_State *l,pragma::CSceneComponent &scene) -> std::vector<pragma::ecs::CParticleSystemComponent*> {
	//	return scene.GetSceneRenderDesc().GetCulledParticles();
	//}));
	defCScene.def(
	  "GetRenderQueue", +[](lua_State *l, pragma::CSceneComponent &scene, pragma::rendering::SceneRenderPass renderMode, bool translucent) -> pragma::rendering::RenderQueue * {
		  auto *renderQueue = scene.GetSceneRenderDesc().GetRenderQueue(renderMode, translucent);
		  if(renderQueue == nullptr)
			  return nullptr;
		  return renderQueue;
	  });

	// Texture indices for scene render target
	defCScene.add_static_constant("RENDER_TARGET_TEXTURE_COLOR", 0u);
	defCScene.add_static_constant("RENDER_TARGET_TEXTURE_BLOOM", 1u);
	defCScene.add_static_constant("RENDER_TARGET_TEXTURE_DEPTH", 2u);

	auto defCreateInfo = luabind::class_<pragma::CSceneComponent::CreateInfo>("CreateInfo");
	defCreateInfo.def(luabind::constructor<>());
	defCreateInfo.def_readwrite("sampleCount", &pragma::CSceneComponent::CreateInfo::sampleCount);
	defCScene.scope[defCreateInfo];

	entsMod[defCScene];

	bsp_register_class(l, entsMod, defCWorld);
	entsMod[defCWorld];

	auto &componentManager = Engine::Get()->GetNetworkState(l)->GetGameState()->GetEntityComponentManager();

	auto defCToggle = pragma::lua::create_entity_component_class<pragma::CToggleComponent, pragma::BaseToggleComponent>("ToggleComponent");
	entsMod[defCToggle];

	auto defCTransform = pragma::lua::create_entity_component_class<pragma::CTransformComponent, pragma::BaseTransformComponent>("TransformComponent");
	defCTransform.add_static_constant("EVENT_ON_POSE_CHANGED", pragma::CTransformComponent::EVENT_ON_POSE_CHANGED);
	entsMod[defCTransform];

	auto defCWheel = pragma::lua::create_entity_component_class<pragma::CWheelComponent, pragma::BaseWheelComponent>("WheelComponent");
	entsMod[defCWheel];

	auto defCSoundDsp = pragma::lua::create_entity_component_class<pragma::CSoundDspComponent, pragma::BaseEnvSoundDspComponent>("SoundDspComponent");
	entsMod[defCSoundDsp];

	auto defCSoundDspChorus = pragma::lua::create_entity_component_class<pragma::CSoundDspChorusComponent, pragma::BaseEnvSoundDspComponent>("SoundDspChorusComponent");
	entsMod[defCSoundDspChorus];

	auto defCSoundDspDistortion = pragma::lua::create_entity_component_class<pragma::CSoundDspDistortionComponent, pragma::BaseEnvSoundDspComponent>("SoundDspDistortionComponent");
	entsMod[defCSoundDspDistortion];

	auto defCSoundDspEAXReverb = pragma::lua::create_entity_component_class<pragma::CSoundDspEAXReverbComponent, pragma::BaseEnvSoundDspComponent>("SoundDspEAXReverbComponent");
	entsMod[defCSoundDspEAXReverb];

	auto defCSoundDspEcho = pragma::lua::create_entity_component_class<pragma::CSoundDspEchoComponent, pragma::BaseEnvSoundDspComponent>("SoundDspEchoComponent");
	entsMod[defCSoundDspEcho];

	auto defCSoundDspEqualizer = pragma::lua::create_entity_component_class<pragma::CSoundDspEqualizerComponent, pragma::BaseEnvSoundDspComponent>("SoundDspEqualizerComponent");
	entsMod[defCSoundDspEqualizer];

	auto defCSoundDspFlanger = pragma::lua::create_entity_component_class<pragma::CSoundDspFlangerComponent, pragma::BaseEnvSoundDspComponent>("SoundDspFlangerComponent");
	entsMod[defCSoundDspFlanger];

	auto defCCamera = pragma::lua::create_entity_component_class<pragma::CCameraComponent, pragma::BaseEnvCameraComponent>("CameraComponent");
	entsMod[defCCamera];

	auto defCOccl = pragma::lua::create_entity_component_class<pragma::COcclusionCullerComponent, pragma::BaseEntityComponent>("OcclusionCullerComponent");
	entsMod[defCOccl];

	auto defCDecal = pragma::lua::create_entity_component_class<pragma::CDecalComponent, pragma::BaseEnvDecalComponent>("DecalComponent");
	defCDecal.def("CreateFromProjection", static_cast<void (*)(lua_State *, pragma::CDecalComponent &, luabind::object, const umath::ScaledTransform &)>(&Lua::Decal::create_from_projection));
	defCDecal.def("CreateFromProjection", static_cast<void (*)(lua_State *, pragma::CDecalComponent &, luabind::object)>(&Lua::Decal::create_from_projection));
	defCDecal.def("DebugDraw", &pragma::CDecalComponent::DebugDraw);
	defCDecal.def("ApplyDecal", static_cast<bool (pragma::CDecalComponent::*)()>(&pragma::CDecalComponent::ApplyDecal));
	entsMod[defCDecal];

	auto defCExplosion = pragma::lua::create_entity_component_class<pragma::CExplosionComponent, pragma::BaseEnvExplosionComponent>("ExplosionComponent");
	entsMod[defCExplosion];

	auto defCFire = pragma::lua::create_entity_component_class<pragma::CFireComponent, pragma::BaseEnvFireComponent>("FireComponent");
	entsMod[defCFire];

	auto defCFogController = pragma::lua::create_entity_component_class<pragma::CFogControllerComponent, pragma::BaseEnvFogControllerComponent>("FogControllerComponent");
	entsMod[defCFogController];

	auto defCLight = pragma::lua::create_entity_component_class<pragma::CLightComponent, pragma::BaseEnvLightComponent>("LightComponent");
	defCLight.def("SetShadowType", &pragma::CLightComponent::SetShadowType);
	defCLight.def("GetShadowType", &pragma::CLightComponent::GetShadowType);
	defCLight.def("UpdateBuffers", &pragma::CLightComponent::UpdateBuffers);
	defCLight.def("SetAddToGameScene", static_cast<void (*)(lua_State *, pragma::CLightComponent &, bool)>([](lua_State *l, pragma::CLightComponent &hComponent, bool b) { hComponent.SetStateFlag(pragma::CLightComponent::StateFlags::AddToGameScene, b); }));
	defCLight.def("SetMorphTargetsInShadowsEnabled", &pragma::CLightComponent::SetMorphTargetsInShadowsEnabled);
	defCLight.def("AreMorphTargetsInShadowsEnabled", &pragma::CLightComponent::AreMorphTargetsInShadowsEnabled);
	defCLight.add_static_constant("SHADOW_TYPE_NONE", umath::to_integral(ShadowType::None));
	defCLight.add_static_constant("SHADOW_TYPE_STATIC_ONLY", umath::to_integral(ShadowType::StaticOnly));
	defCLight.add_static_constant("SHADOW_TYPE_FULL", umath::to_integral(ShadowType::Full));

	defCLight.add_static_constant("EVENT_SHOULD_PASS_ENTITY", pragma::CLightComponent::EVENT_SHOULD_PASS_ENTITY);
	defCLight.add_static_constant("EVENT_SHOULD_PASS_ENTITY_MESH", pragma::CLightComponent::EVENT_SHOULD_PASS_ENTITY_MESH);
	defCLight.add_static_constant("EVENT_SHOULD_PASS_MESH", pragma::CLightComponent::EVENT_SHOULD_PASS_MESH);
	defCLight.add_static_constant("EVENT_SHOULD_UPDATE_RENDER_PASS", pragma::CLightComponent::EVENT_SHOULD_UPDATE_RENDER_PASS);
	defCLight.add_static_constant("EVENT_GET_TRANSFORMATION_MATRIX", pragma::CLightComponent::EVENT_GET_TRANSFORMATION_MATRIX);
	defCLight.add_static_constant("EVENT_HANDLE_SHADOW_MAP", pragma::CLightComponent::EVENT_HANDLE_SHADOW_MAP);
	defCLight.add_static_constant("EVENT_ON_SHADOW_BUFFER_INITIALIZED", pragma::CLightComponent::EVENT_ON_SHADOW_BUFFER_INITIALIZED);
	entsMod[defCLight];

	auto defCLightDirectional = pragma::lua::create_entity_component_class<pragma::CLightDirectionalComponent, pragma::BaseEnvLightDirectionalComponent>("LightDirectionalComponent");
	entsMod[defCLightDirectional];

	auto defCLightPoint = pragma::lua::create_entity_component_class<pragma::CLightPointComponent, pragma::BaseEnvLightPointComponent>("LightPointComponent");
	entsMod[defCLightPoint];

	auto defCLightSpot = pragma::lua::create_entity_component_class<pragma::CLightSpotComponent, pragma::BaseEnvLightSpotComponent>("LightSpotComponent");
	entsMod[defCLightSpot];

	auto defCLightSpotVol = pragma::lua::create_entity_component_class<pragma::CLightSpotVolComponent, pragma::BaseEnvLightSpotVolComponent>("LightSpotVolComponent");
	entsMod[defCLightSpotVol];

	auto defCMicrophone = pragma::lua::create_entity_component_class<pragma::CMicrophoneComponent, pragma::BaseEnvMicrophoneComponent>("MicrophoneComponent");
	entsMod[defCMicrophone];

	auto defCQuake = pragma::lua::create_entity_component_class<pragma::CQuakeComponent, pragma::BaseEnvQuakeComponent>("QuakeComponent");
	entsMod[defCQuake];

	auto defCSmokeTrail = pragma::lua::create_entity_component_class<pragma::CSmokeTrailComponent, pragma::BaseEnvSmokeTrailComponent>("SmokeTrailComponent");
	entsMod[defCSmokeTrail];

	auto defCSound = pragma::lua::create_entity_component_class<pragma::CSoundComponent, pragma::BaseEnvSoundComponent>("SoundComponent");
	entsMod[defCSound];

	auto defCSoundScape = pragma::lua::create_entity_component_class<pragma::CSoundScapeComponent, pragma::BaseEnvSoundScapeComponent>("SoundScapeComponent");
	entsMod[defCSoundScape];

	auto defCSprite = pragma::lua::create_entity_component_class<pragma::CSpriteComponent, pragma::BaseEnvSpriteComponent>("SpriteComponent");
	defCSprite.def("StopAndRemoveEntity", &pragma::CSpriteComponent::StopAndRemoveEntity);
	entsMod[defCSprite];

	auto defCTimescale = pragma::lua::create_entity_component_class<pragma::CEnvTimescaleComponent, pragma::BaseEnvTimescaleComponent>("EnvTimescaleComponent");
	entsMod[defCTimescale];

	auto defCWind = pragma::lua::create_entity_component_class<pragma::CWindComponent, pragma::BaseEnvWindComponent>("WindComponent");
	entsMod[defCWind];

	auto defCFilterClass = pragma::lua::create_entity_component_class<pragma::CFilterClassComponent, pragma::BaseFilterClassComponent>("FilterClassComponent");
	entsMod[defCFilterClass];

	auto defCFilterName = pragma::lua::create_entity_component_class<pragma::CFilterNameComponent, pragma::BaseFilterNameComponent>("FilterNameComponent");
	entsMod[defCFilterName];

	auto defCBrush = pragma::lua::create_entity_component_class<pragma::CBrushComponent, pragma::BaseFuncBrushComponent>("BrushComponent");
	entsMod[defCBrush];

	auto defCKinematic = pragma::lua::create_entity_component_class<pragma::CKinematicComponent, pragma::BaseFuncKinematicComponent>("KinematicComponent");
	entsMod[defCKinematic];

	auto defCFuncPhysics = pragma::lua::create_entity_component_class<pragma::CFuncPhysicsComponent, pragma::BaseFuncPhysicsComponent>("FuncPhysicsComponent");
	entsMod[defCFuncPhysics];

	auto defCFuncSoftPhysics = pragma::lua::create_entity_component_class<pragma::CFuncSoftPhysicsComponent, pragma::BaseFuncSoftPhysicsComponent>("FuncSoftPhysicsComponent");
	entsMod[defCFuncSoftPhysics];

	auto defCSurface = pragma::lua::create_entity_component_class<pragma::CSurfaceComponent, pragma::BaseSurfaceComponent>("SurfaceComponent");
	entsMod[defCSurface];

	// auto defCFuncPortal = pragma::lua::create_entity_component_class<pragma::CFuncPortalComponent,pragma::BaseFuncPortalComponent>("FuncPortalComponent");
	// entsMod[defCFuncPortal];

	auto defCLiquidSurf = pragma::lua::create_entity_component_class<pragma::CLiquidSurfaceComponent, pragma::BaseLiquidSurfaceComponent>("LiquidSurfaceComponent");
	defCLiquidSurf.def(
	  "Test", +[](pragma::CLiquidSurfaceComponent &c, prosper::Texture &tex) {
		  auto &scene = c.GetWaterScene();
		  scene.descSetGroupTexEffects->GetDescriptorSet()->SetBindingTexture(tex, 0);
		  scene.descSetGroupTexEffects->GetDescriptorSet()->Update();
	  });
	entsMod[defCLiquidSurf];

	auto defCLiquidVol = pragma::lua::create_entity_component_class<pragma::CLiquidVolumeComponent, pragma::BaseLiquidVolumeComponent>("LiquidVolumeComponent");
	entsMod[defCLiquidVol];

	auto defCBuoyancy = pragma::lua::create_entity_component_class<pragma::CBuoyancyComponent, pragma::BaseBuoyancyComponent>("BuoyancyComponent");
	entsMod[defCBuoyancy];

	auto defCWater = pragma::lua::create_entity_component_class<pragma::CLiquidComponent, pragma::BaseFuncLiquidComponent>("LiquidComponent");
	entsMod[defCWater];

	auto defCButton = pragma::lua::create_entity_component_class<pragma::CButtonComponent, pragma::BaseFuncButtonComponent>("ButtonComponent");
	entsMod[defCButton];

	auto defCBot = pragma::lua::create_entity_component_class<pragma::CBotComponent, pragma::BaseBotComponent>("BotComponent");
	entsMod[defCBot];

	auto defCPointConstraintBallSocket = pragma::lua::create_entity_component_class<pragma::CPointConstraintBallSocketComponent, pragma::BasePointConstraintBallSocketComponent>("PointConstraintBallSocketComponent");
	entsMod[defCPointConstraintBallSocket];

	auto defCPointConstraintConeTwist = pragma::lua::create_entity_component_class<pragma::CPointConstraintConeTwistComponent, pragma::BasePointConstraintConeTwistComponent>("PointConstraintConeTwistComponent");
	entsMod[defCPointConstraintConeTwist];

	auto defCPointConstraintDoF = pragma::lua::create_entity_component_class<pragma::CPointConstraintDoFComponent, pragma::BasePointConstraintDoFComponent>("PointConstraintDoFComponent");
	entsMod[defCPointConstraintDoF];

	auto defCPointConstraintFixed = pragma::lua::create_entity_component_class<pragma::CPointConstraintFixedComponent, pragma::BasePointConstraintFixedComponent>("PointConstraintFixedComponent");
	entsMod[defCPointConstraintFixed];

	auto defCPointConstraintHinge = pragma::lua::create_entity_component_class<pragma::CPointConstraintHingeComponent, pragma::BasePointConstraintHingeComponent>("PointConstraintHingeComponent");
	entsMod[defCPointConstraintHinge];

	auto defCPointConstraintSlider = pragma::lua::create_entity_component_class<pragma::CPointConstraintSliderComponent, pragma::BasePointConstraintSliderComponent>("PointConstraintSliderComponent");
	entsMod[defCPointConstraintSlider];

	// auto defCRenderTarget = pragma::lua::create_entity_component_class<pragma::CRenderTargetComponent,pragma::BasePointRenderTargetComponent>("RenderTargetComponent");
	// entsMod[defCRenderTarget];

	auto defCPointTarget = pragma::lua::create_entity_component_class<pragma::CPointTargetComponent, pragma::BasePointTargetComponent>("PointTargetComponent");
	entsMod[defCPointTarget];

	auto defCProp = pragma::lua::create_entity_component_class<pragma::CPropComponent, pragma::BasePropComponent>("PropComponent");
	entsMod[defCProp];

	auto defCPropDynamic = pragma::lua::create_entity_component_class<pragma::CPropDynamicComponent, pragma::BasePropDynamicComponent>("PropDynamicComponent");
	entsMod[defCPropDynamic];

	auto defCPropPhysics = pragma::lua::create_entity_component_class<pragma::CPropPhysicsComponent, pragma::BasePropPhysicsComponent>("PropPhysicsComponent");
	entsMod[defCPropPhysics];

	auto defCTouch = pragma::lua::create_entity_component_class<pragma::CTouchComponent, pragma::BaseTouchComponent>("TouchComponent");
	entsMod[defCTouch];

	auto defCSkybox = pragma::lua::create_entity_component_class<pragma::CSkyboxComponent, pragma::BaseSkyboxComponent>("SkyboxComponent");
	defCSkybox.def("SetSkyMaterial", &pragma::CSkyboxComponent::SetSkyMaterial);
	defCSkybox.def("SetSkyAngles", &pragma::CSkyboxComponent::SetSkyAngles);
	defCSkybox.def("GetSkyAngles", &pragma::CSkyboxComponent::GetSkyAngles);
	entsMod[defCSkybox];

	auto defCFlashlight = pragma::lua::create_entity_component_class<pragma::CFlashlightComponent, pragma::BaseFlashlightComponent>("FlashlightComponent");
	entsMod[defCFlashlight];

	auto defCEnvSoundProbe = pragma::lua::create_entity_component_class<pragma::CEnvSoundProbeComponent, pragma::BaseEntityComponent>("EnvSoundProbeComponent");
	entsMod[defCEnvSoundProbe];

	auto defCWeather = pragma::lua::create_entity_component_class<pragma::CWeatherComponent, pragma::BaseEnvWeatherComponent>("WeatherComponent");
	entsMod[defCWeather];

	auto defCReflectionProbe = pragma::lua::create_entity_component_class<pragma::CReflectionProbeComponent, pragma::BaseEntityComponent>("ReflectionProbeComponent");
	defCReflectionProbe.def("GetIBLStrength", &pragma::CReflectionProbeComponent::GetIBLStrength);
	defCReflectionProbe.def("SetIBLStrength", &pragma::CReflectionProbeComponent::SetIBLStrength);
	defCReflectionProbe.def("GetLocationIdentifier", &pragma::CReflectionProbeComponent::GetLocationIdentifier);
	defCReflectionProbe.def("GetIBLMaterialFilePath", &pragma::CReflectionProbeComponent::GetCubemapIBLMaterialFilePath);
	defCReflectionProbe.def("CaptureIBLReflectionsFromScene", static_cast<bool (*)(lua_State *, pragma::CReflectionProbeComponent &, luabind::table<>, bool)>(&reflection_probe_capture_ibl_reflections_from_scene));
	defCReflectionProbe.def("CaptureIBLReflectionsFromScene", static_cast<bool (*)(lua_State *, pragma::CReflectionProbeComponent &, luabind::table<>)>(&reflection_probe_capture_ibl_reflections_from_scene));
	defCReflectionProbe.def("CaptureIBLReflectionsFromScene", static_cast<bool (*)(lua_State *, pragma::CReflectionProbeComponent &, bool)>(&reflection_probe_capture_ibl_reflections_from_scene));
	defCReflectionProbe.def("CaptureIBLReflectionsFromScene", static_cast<bool (*)(lua_State *, pragma::CReflectionProbeComponent &)>(&reflection_probe_capture_ibl_reflections_from_scene));
	defCReflectionProbe.def("RequiresRebuild", &pragma::CReflectionProbeComponent::RequiresRebuild);
	defCReflectionProbe.def("GenerateFromEquirectangularImage", &pragma::CReflectionProbeComponent::GenerateFromEquirectangularImage);
	entsMod[defCReflectionProbe];

	auto defCSkyCamera = pragma::lua::create_entity_component_class<pragma::CSkyCameraComponent, pragma::BaseEntityComponent>("SkyCameraComponent");
	entsMod[defCSkyCamera];

	auto defCPBRConverter = pragma::lua::create_entity_component_class<pragma::CPBRConverterComponent, pragma::BaseEntityComponent>("PBRConverterComponent");
	defCPBRConverter.def("GenerateAmbientOcclusionMaps", static_cast<void (*)(lua_State *, pragma::CPBRConverterComponent &, ::Model &, uint32_t, uint32_t, uint32_t, bool)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	defCPBRConverter.def("GenerateAmbientOcclusionMaps", static_cast<void (*)(lua_State *, pragma::CPBRConverterComponent &, ::Model &, uint32_t, uint32_t, uint32_t)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	defCPBRConverter.def("GenerateAmbientOcclusionMaps", static_cast<void (*)(lua_State *, pragma::CPBRConverterComponent &, ::Model &, uint32_t, uint32_t)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	defCPBRConverter.def("GenerateAmbientOcclusionMaps", static_cast<void (*)(lua_State *, pragma::CPBRConverterComponent &, ::Model &)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	defCPBRConverter.def("GenerateAmbientOcclusionMaps", static_cast<void (*)(lua_State *, pragma::CPBRConverterComponent &, BaseEntity &, uint32_t, uint32_t, uint32_t, bool)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	defCPBRConverter.def("GenerateAmbientOcclusionMaps", static_cast<void (*)(lua_State *, pragma::CPBRConverterComponent &, BaseEntity &, uint32_t, uint32_t, uint32_t)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	defCPBRConverter.def("GenerateAmbientOcclusionMaps", static_cast<void (*)(lua_State *, pragma::CPBRConverterComponent &, BaseEntity &, uint32_t, uint32_t)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	defCPBRConverter.def("GenerateAmbientOcclusionMaps", static_cast<void (*)(lua_State *, pragma::CPBRConverterComponent &, BaseEntity &)>(Lua::PBRConverter::GenerateAmbientOcclusionMaps));
	entsMod[defCPBRConverter];

	auto defShadow = pragma::lua::create_entity_component_class<pragma::CShadowComponent, pragma::BaseEntityComponent>("ShadowMapComponent");
	entsMod[defShadow];

	auto defShadowCsm = pragma::lua::create_entity_component_class<pragma::CShadowCSMComponent, pragma::BaseEntityComponent>("CSMComponent");
	entsMod[defShadowCsm];

	auto defShadowManager = pragma::lua::create_entity_component_class<pragma::CShadowManagerComponent, pragma::BaseEntityComponent>("ShadowManagerComponent");
	entsMod[defShadowManager];

	auto defCWaterSurface = pragma::lua::create_entity_component_class<pragma::CWaterSurfaceComponent, pragma::BaseEntityComponent>("WaterSurfaceComponent");
	entsMod[defCWaterSurface];

	auto defCListener = pragma::lua::create_entity_component_class<pragma::CListenerComponent, pragma::BaseEntityComponent>("ListenerComponent");
	entsMod[defCListener];

	auto defCViewBody = pragma::lua::create_entity_component_class<pragma::CViewBodyComponent, pragma::BaseEntityComponent>("ViewBodyComponent");
	entsMod[defCViewBody];

	auto defCViewModel = pragma::lua::create_entity_component_class<pragma::CViewModelComponent, pragma::BaseEntityComponent>("ViewModelComponent");
	defCViewModel.def("GetPlayer", &pragma::CViewModelComponent::GetPlayer);
	defCViewModel.def("GetWeapon", &pragma::CViewModelComponent::GetWeapon);
	entsMod[defCViewModel];

	auto defCSoftBody = pragma::lua::create_entity_component_class<pragma::CSoftBodyComponent, pragma::BaseSoftBodyComponent>("SoftBodyComponent");
	entsMod[defCSoftBody];

	auto defCRaytracing = pragma::lua::create_entity_component_class<pragma::CRaytracingComponent, pragma::BaseEntityComponent>("RaytracingComponent");
	entsMod[defCRaytracing];

	auto defCBSPLeaf = pragma::lua::create_entity_component_class<pragma::CBSPLeafComponent, pragma::BaseEntityComponent>("BSPLeafComponent");
	entsMod[defCBSPLeaf];

	auto defCIo = pragma::lua::create_entity_component_class<pragma::CIOComponent, pragma::BaseIOComponent>("IOComponent");
	entsMod[defCIo];

	auto defCTimeScale = pragma::lua::create_entity_component_class<pragma::CTimeScaleComponent, pragma::BaseTimeScaleComponent>("TimeScaleComponent");
	entsMod[defCTimeScale];

	auto defCAttachable = pragma::lua::create_entity_component_class<pragma::CAttachmentComponent, pragma::BaseAttachmentComponent>("AttachmentComponent");
	entsMod[defCAttachable];

	auto defCChild = pragma::lua::create_entity_component_class<pragma::CChildComponent, pragma::BaseChildComponent>("ChildComponent");
	entsMod[defCChild];

	auto defCOwnable = pragma::lua::create_entity_component_class<pragma::COwnableComponent, pragma::BaseOwnableComponent>("OwnableComponent");
	entsMod[defCOwnable];

	auto defCDebugText = pragma::lua::create_entity_component_class<pragma::CDebugTextComponent, pragma::BaseDebugTextComponent>("DebugTextComponent");
	entsMod[defCDebugText];

	auto defCDebugPoint = pragma::lua::create_entity_component_class<pragma::CDebugPointComponent, pragma::BaseDebugPointComponent>("DebugPointComponent");
	entsMod[defCDebugPoint];

	auto defCDebugLine = pragma::lua::create_entity_component_class<pragma::CDebugLineComponent, pragma::BaseDebugLineComponent>("DebugLineComponent");
	entsMod[defCDebugLine];

	auto defCDebugBox = pragma::lua::create_entity_component_class<pragma::CDebugBoxComponent, pragma::BaseDebugBoxComponent>("DebugBoxComponent");
	defCDebugBox.def("SetColorOverride", &pragma::CDebugBoxComponent::SetColorOverride);
	defCDebugBox.def("ClearColorOverride", &pragma::CDebugBoxComponent::ClearColorOverride);
	defCDebugBox.def("GetColorOverride", &pragma::CDebugBoxComponent::GetColorOverride);
	defCDebugBox.def("SetIgnoreDepthBuffer", &pragma::CDebugBoxComponent::SetIgnoreDepthBuffer);
	defCDebugBox.def("ShouldIgnoreDepthBuffer", &pragma::CDebugBoxComponent::ShouldIgnoreDepthBuffer);
	entsMod[defCDebugBox];

	auto defCDebugSphere = pragma::lua::create_entity_component_class<pragma::CDebugSphereComponent, pragma::BaseDebugSphereComponent>("DebugSphereComponent");
	entsMod[defCDebugSphere];

	auto defCDebugCone = pragma::lua::create_entity_component_class<pragma::CDebugConeComponent, pragma::BaseDebugConeComponent>("DebugConeComponent");
	entsMod[defCDebugCone];

	auto defCDebugCylinder = pragma::lua::create_entity_component_class<pragma::CDebugCylinderComponent, pragma::BaseDebugCylinderComponent>("DebugCylinderComponent");
	entsMod[defCDebugCylinder];

	auto defCDebugPlane = pragma::lua::create_entity_component_class<pragma::CDebugPlaneComponent, pragma::BaseDebugPlaneComponent>("DebugPlaneComponent");
	entsMod[defCDebugPlane];

	auto defCPointAtTarget = pragma::lua::create_entity_component_class<pragma::CPointAtTargetComponent, pragma::BasePointAtTargetComponent>("PointAtTargetComponent");
	entsMod[defCPointAtTarget];

	auto defCBSP = pragma::lua::create_entity_component_class<pragma::CBSPComponent, pragma::BaseEntityComponent>("BSPComponent");
	entsMod[defCBSP];

	auto defCGeneric = pragma::lua::create_entity_component_class<pragma::CGenericComponent, pragma::BaseGenericComponent>("GenericComponent");
	entsMod[defCGeneric];

	RegisterLuaEntityComponents2_cl(l, entsMod);
}
