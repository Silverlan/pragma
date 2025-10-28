// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <image/prosper_render_target.hpp>
#include <mathutil/transform.hpp>
#include "mathutil/umath.h"
#include "pragma/lua/core.hpp"

#define MOTION_BLUR_DEBUG_ELEMENT_ENABLED 0

export module pragma.client:entities.components.pp_motion_blur;
import :entities.components.pp_base;
import :entities.components.motion_blur_data;

export namespace pragma {
	enum class MotionBlurQuality : uint32_t { Low = 0, Medium, High };

	class DLLCLIENT CRendererPpMotionBlurComponent final : public CRendererPpBaseComponent {
	  public:
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		CRendererPpMotionBlurComponent(pragma::ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnRemove() override;
		virtual std::string GetIdentifier() const override { return "motion_blur"; }
		virtual uint32_t GetPostProcessingWeight() const override { return umath::to_integral(CRendererComponent::StandardPostProcessingWeight::MotionBlur); }

		void SetAutoUpdateMotionData(bool updateMotionPerFrame);
		void UpdateMotionBlurData();

		void SetMotionBlurIntensity(float intensity);
		float GetMotionBlurIntensity() const;

		void SetMotionBlurQuality(MotionBlurQuality quality);
		MotionBlurQuality GetMotionBlurQuality() const;

		const std::shared_ptr<prosper::ISwapCommandBufferGroup> &GetSwapCommandBuffer() const;
		const std::shared_ptr<prosper::RenderTarget> &GetRenderTarget() const;
	  private:
		virtual void DoRenderEffect(const util::DrawSceneInfo &drawSceneInfo) override;
		void DoUpdatePoses(const CMotionBlurDataComponent &motionBlurDataC, const MotionBlurTemporalData &motionBlurData, prosper::IPrimaryCommandBuffer &cmd);
		void RecordVelocityPass(const util::DrawSceneInfo &drawSceneInfo);
		void ExecuteVelocityPass(const util::DrawSceneInfo &drawSceneInfo);
		void RenderPostProcessing(const util::DrawSceneInfo &drawSceneInfo);
		void ReloadVelocityTexture();
		std::shared_ptr<prosper::ISwapCommandBufferGroup> m_swapCmd = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_velocityTexDsg;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_motionBlurDataDsg;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_genericBoneDsg;
		std::shared_ptr<prosper::IBuffer> m_motionBlurDataBuffer;
		std::shared_ptr<prosper::RenderTarget> m_renderTarget;
#if MOTION_BLUR_DEBUG_ELEMENT_ENABLED == 1
		WIHandle m_debugTex;
#endif
		float m_motionBlurIntensityFactor = 4.f;
		MotionBlurQuality m_motionBlurQuality = MotionBlurQuality::Low;
		bool m_valid = false;
		bool m_autoUpdateMotionData = true;
		bool m_motionDataUpdateRequired = false;
		size_t m_lastMotionDataBufferUpdateIndex = std::numeric_limits<size_t>::max();
	};
};
