// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.pp_bloom;

export import :entities.components.pp_base;
import :rendering;

export namespace pragma {
	class DLLCLIENT CRendererPpBloomComponent final : public CRendererPpBaseComponent {
	  public:
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		CRendererPpBloomComponent(ecs::BaseEntity &ent);

		void SetBlurRadius(uint32_t radius);
		void SetBlurSigma(double sigma);
		uint32_t GetBlurRadius() const;
		double GetBlurSigma() const;

		void SetBloomThreshold(float threshold);
		float GetBloomThreshold() const;

		void SetBlurAmount(int32_t blurAmount);
		int32_t GetBlurAmount() const;

		virtual void InitializeLuaObject(lua::State *l) override;
		virtual std::string GetIdentifier() const override { return "bloom"; }
		virtual uint32_t GetPostProcessingWeight() const override { return math::to_integral(CRendererComponent::StandardPostProcessingWeight::Bloom); }

		virtual void OnTick(double dt) override;
	  private:
		void SetPipelineDirty();
		virtual void DoRenderEffect(const rendering::DrawSceneInfo &drawSceneInfo) override;
		rendering::ControlledBlurSettings m_controlledBlurSettings;
		float m_bloomThreshold = 1.f;
	};
};
