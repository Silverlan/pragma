// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.client:entities.components.pp_volumetric;

export import :entities.components.pp_base;

export namespace pragma {
	class CRasterizationRendererComponent;
	class DLLCLIENT CRendererPpVolumetricComponent final : public CRendererPpBaseComponent {
	  public:
		CRendererPpVolumetricComponent(pragma::ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual void OnEntitySpawn() override;
		virtual std::string GetIdentifier() const override { return "volumetric"; }
		virtual uint32_t GetPostProcessingWeight() const override { return umath::to_integral(CRendererComponent::StandardPostProcessingWeight::Fog) - 20'000; }
	  private:
		void ReloadRenderTarget();
		virtual void DoRenderEffect(const util::DrawSceneInfo &drawSceneInfo) override;
		std::shared_ptr<prosper::RenderTarget> m_renderTarget;
	};
};
