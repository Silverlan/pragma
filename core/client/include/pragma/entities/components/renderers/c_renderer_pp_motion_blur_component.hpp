/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_RENDERER_PP_MOTION_BLUR_COMPONENT_HPP__
#define __C_RENDERER_PP_MOTION_BLUR_COMPONENT_HPP__

#include "pragma/entities/components/renderers/c_renderer_pp_base_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include <image/prosper_render_target.hpp>
#include <wgui/types.hpp>
#include <mathutil/transform.hpp>

namespace pragma
{
	struct DLLCLIENT MotionBlurTemporalData
	{
		std::unordered_map<const BaseEntity*,Mat4> prevModelMatrices;
		std::unordered_map<const BaseEntity*,Mat4> curModelMatrices;
		umath::Transform prevCamPose {};
		umath::Transform curCamPose {};
		double lastTick = 0.0;
	};

#pragma pack(push,1)
	struct DLLCLIENT MotionBlurData
	{
		Vector4 linearCameraVelocity;
		Vector4 angularCameraVelocity;
	};
#pragma pack(pop)
	
	class DLLCLIENT CRendererPpMotionBlurComponent final
		: public CRendererPpBaseComponent
	{
	public:
		CRendererPpMotionBlurComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnRemove() override;
		virtual std::string GetIdentifier() const override {return "motion_blur";}
		virtual uint32_t GetPostProcessingWeight() const override {return umath::to_integral(CRendererComponent::StandardPostProcessingWeight::MotionBlur);}

		const std::shared_ptr<prosper::ISwapCommandBufferGroup> &GetSwapCommandBuffer() const;
		const std::shared_ptr<prosper::RenderTarget> &GetRenderTarget() const;
	private:
		virtual void DoRenderEffect(const util::DrawSceneInfo &drawSceneInfo) override;
		void RecordVelocityPass(const util::DrawSceneInfo &drawSceneInfo);
		void ExecuteVelocityPass(const util::DrawSceneInfo &drawSceneInfo);
		void RenderPostProcessing(const util::DrawSceneInfo &drawSceneInfo);
		void ReloadVelocityTexture();
		util::WeakHandle<prosper::Shader> m_velocityShader {};
		std::shared_ptr<prosper::ISwapCommandBufferGroup> m_swapCmd = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_velocityTexDsg;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_motionBlurDataDsg;
		std::shared_ptr<prosper::IBuffer> m_motionBlurDataBuffer;
		std::shared_ptr<prosper::RenderTarget> m_renderTarget;
		MotionBlurTemporalData m_motionBlurData {};
		WIHandle m_debugTex;
		bool m_valid = false;
	};
};

#endif
