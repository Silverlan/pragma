/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_MOTION_BLUR_DATA_COMPONENT_HPP__
#define __C_MOTION_BLUR_DATA_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_entity_component.hpp>
#include <image/prosper_render_target.hpp>
#include <mathutil/transform.hpp>
#include <unordered_set>

namespace pragma {
	class ShaderVelocityBuffer;
#pragma pack(push, 1)
	struct DLLCLIENT MotionBlurCameraData {
		Vector4 linearCameraVelocity;
		Vector4 angularCameraVelocity;
	};
#pragma pack(pop)

	struct DLLCLIENT MotionBlurTemporalData {
		struct PoseData {
			Mat4 matrix;
			umath::Transform pose;
			std::shared_ptr<prosper::IBuffer> boneBuffer;
			std::shared_ptr<prosper::IDescriptorSetGroup> boneDsg;
		};
		std::unordered_map<const BaseEntity *, PoseData> prevModelMatrices;
		std::unordered_map<const BaseEntity *, PoseData> curModelMatrices;
		MotionBlurCameraData cameraData;
		double lastTick = 0.0;
	};

	DLLCLIENT pragma::ShaderVelocityBuffer *get_velocity_buffer_shader();
	class DLLCLIENT CMotionBlurDataComponent final : public BaseEntityComponent {
	  public:
		CMotionBlurDataComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;

		void UpdateEntityPoses();
		const MotionBlurTemporalData &GetMotionBlurData() const { return m_motionBlurData; }
		size_t GetLastUpdateIndex() const { return m_lastUpdateIndex; }
	  private:
		MotionBlurTemporalData m_motionBlurData {};
		size_t m_lastUpdateIndex = 0;
	};
};

#endif
