// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/entities/components/renderers/c_renderer_pp_base_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"

export module pragma.client.entities.components:pp_bloom;

import pragma.client.rendering;

export namespace pragma {
	class DLLCLIENT CRendererPpBloomComponent final : public CRendererPpBaseComponent {
	  public:
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		CRendererPpBloomComponent(BaseEntity &ent);

		void SetBlurRadius(uint32_t radius);
		void SetBlurSigma(double sigma);
		uint32_t GetBlurRadius() const;
		double GetBlurSigma() const;

		void SetBloomThreshold(float threshold);
		float GetBloomThreshold() const;

		void SetBlurAmount(int32_t blurAmount);
		int32_t GetBlurAmount() const;

		virtual void InitializeLuaObject(lua_State *l) override;
		virtual std::string GetIdentifier() const override { return "bloom"; }
		virtual uint32_t GetPostProcessingWeight() const override { return umath::to_integral(CRendererComponent::StandardPostProcessingWeight::Bloom); }

		virtual void OnTick(double dt) override;
	  private:
		void SetPipelineDirty();
		virtual void DoRenderEffect(const util::DrawSceneInfo &drawSceneInfo) override;
		ControlledBlurSettings m_controlledBlurSettings;
		float m_bloomThreshold = 1.f;
	};
};
