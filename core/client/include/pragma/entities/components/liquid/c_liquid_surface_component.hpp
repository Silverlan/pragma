/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_LIQUID_SURFACE_COMPONENT_HPP__
#define __C_LIQUID_SURFACE_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include <pragma/types.hpp>
#include <pragma/entities/components/liquid/base_liquid_surface_component.hpp>
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include <image/prosper_texture.hpp>
#include <buffers/prosper_buffer.hpp>
#include <mathutil/umath_geometry.hpp>

namespace pragma {
	class CSceneComponent;
	struct DLLCLIENT WaterScene {
		~WaterScene();
		util::TWeakSharedHandle<pragma::CSceneComponent> sceneReflection = {};
		std::shared_ptr<prosper::IBuffer> settingsBuffer = nullptr;

		// Fog
		std::shared_ptr<prosper::IBuffer> fogBuffer = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> fogDescSetGroup = nullptr;

		std::shared_ptr<prosper::IDescriptorSetGroup> descSetGroupTexEffects = nullptr;

		std::vector<CallbackHandle> hPostProcessing = {};
		CallbackHandle hComponentCreationCallback = {};

		std::shared_ptr<prosper::Texture> texScene = nullptr;
		std::shared_ptr<prosper::Texture> texSceneDepth = nullptr;

		util::PFloatProperty waterScale = nullptr;
		util::PFloatProperty waveStrength = nullptr;
		util::PFloatProperty waveSpeed = nullptr;
		util::PFloatProperty reflectiveIntensity = nullptr;
	};

	class CSurfaceComponent;
	class DLLCLIENT CLiquidSurfaceComponent final : public BaseLiquidSurfaceComponent, public CBaseNetComponent {
	  public:
		CLiquidSurfaceComponent(BaseEntity &ent) : BaseLiquidSurfaceComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnRemove() override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void ReceiveData(NetPacket &packet) override;

		void ClearWaterScene();
		const WaterScene &GetWaterScene() const;
		prosper::IDescriptorSet *GetEffectDescriptorSet() const;
		CMaterial *GetWaterMaterial() const;
		void InitializeWaterScene(const Vector3 &refPos, const Vector3 &planeNormal, const Vector3 &waterAabbMin, const Vector3 &waterAabbMax);
		void InitializeWaterScene(const WaterScene &scene);
		bool IsWaterSceneValid() const;
	  private:
		virtual void OnEntitySpawn() override;
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		void RenderPostProcessingOverlay(const util::DrawSceneInfo &drawSceneInfo);
		void InitializeRenderData();
		void InitializeRenderer(pragma::CRendererComponent &component);

		EntityHandle m_hEntUnderwater {};
		pragma::ComponentHandle<CSurfaceComponent> m_surfaceComponent;
		std::unique_ptr<WaterScene> m_waterScene = nullptr;
		util::WeakHandle<prosper::Shader> m_shaderPpWater {};
		umath::geometry::PlaneSide m_curCameraSurfaceSide = umath::geometry::PlaneSide::OnPlane;
		std::pair<Vector3, Vector3> m_waterAabbBounds = {};
		bool m_renderDataInitialized = false;
	};
};

#endif
