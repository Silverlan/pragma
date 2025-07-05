// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_RENDERER_PP_VOLUMETRIC_COMPONENT_HPP__
#define __C_RENDERER_PP_VOLUMETRIC_COMPONENT_HPP__

#include "pragma/entities/components/renderers/c_renderer_pp_base_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"

namespace pragma {
	class CRasterizationRendererComponent;
	class DLLCLIENT CRendererPpVolumetricComponent final : public CRendererPpBaseComponent {
	  public:
		CRendererPpVolumetricComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnEntitySpawn() override;
		virtual std::string GetIdentifier() const override { return "volumetric"; }
		virtual uint32_t GetPostProcessingWeight() const override { return umath::to_integral(CRendererComponent::StandardPostProcessingWeight::Fog) - 20'000; }
	  private:
		void ReloadRenderTarget();
		virtual void DoRenderEffect(const util::DrawSceneInfo &drawSceneInfo) override;
		std::shared_ptr<prosper::RenderTarget> m_renderTarget;
	};
};

#endif
