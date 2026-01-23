// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.pp_fxaa;

export import :entities.components.pp_base;

export namespace pragma {
	class CRasterizationRendererComponent;
	class DLLCLIENT CRendererPpFxaaComponent final : public CRendererPpBaseComponent {
	  public:
		CRendererPpFxaaComponent(ecs::BaseEntity &ent);
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual std::string GetIdentifier() const override { return "fxaa"; }
		virtual uint32_t GetPostProcessingWeight() const override { return math::to_integral(CRendererComponent::StandardPostProcessingWeight::Fxaa); }
		virtual PostProcessingEffectData::Flags GetFlags() const override { return PostProcessingEffectData::Flags::ToneMapped; }
	  private:
		virtual void DoRenderEffect(const rendering::DrawSceneInfo &drawSceneInfo) override;
	};
};
