// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.pp_fog;

export import :entities.components.pp_base;

export namespace pragma {
	class CRasterizationRendererComponent;
	class DLLCLIENT CRendererPpFogComponent final : public CRendererPpBaseComponent {
	  public:
		CRendererPpFogComponent(ecs::BaseEntity &ent);
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual std::string GetIdentifier() const override { return "fog"; }
		virtual uint32_t GetPostProcessingWeight() const override { return math::to_integral(CRendererComponent::StandardPostProcessingWeight::Fog); }
	  private:
		virtual void DoRenderEffect(const rendering::DrawSceneInfo &drawSceneInfo) override;
	};
};
