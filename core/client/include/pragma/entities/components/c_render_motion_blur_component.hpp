/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_RENDER_MOTION_BLUR_COMPONENT_HPP__
#define __C_RENDER_MOTION_BLUR_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_entity_component.hpp>
#include <unordered_set>

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

	class DLLCLIENT CRenderMotionBlurComponent final
		: public BaseEntityComponent
	{
	public:
		CRenderMotionBlurComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;

		const std::shared_ptr<prosper::ISwapCommandBufferGroup> &GetSwapCommandBuffer() const;
		const std::shared_ptr<prosper::RenderTarget> &GetRenderTarget() const;
		void PPTest(const util::DrawSceneInfo &drawSceneInfo);
	private:
		void RecordVelocityPass(const util::DrawSceneInfo &drawSceneInfo);
		void ExecuteVelocityPass(const util::DrawSceneInfo &drawSceneInfo);
		util::WeakHandle<prosper::Shader> m_velocityShader {};
		std::shared_ptr<prosper::ISwapCommandBufferGroup> m_swapCmd = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_velocityTexDsg;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_motionBlurDataDsg;
		std::shared_ptr<prosper::IBuffer> m_motionBlurDataBuffer;
		std::shared_ptr<prosper::RenderTarget> m_renderTarget;
		MotionBlurTemporalData m_motionBlurData {};
	};
};

#endif
