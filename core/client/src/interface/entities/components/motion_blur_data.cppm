// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_entity_component.hpp>
#include <image/prosper_render_target.hpp>
#include <mathutil/transform.hpp>
#include <unordered_set>

export module pragma.client.entities.components:motion_blur_data;

import pragma.client.rendering.shaders;

export namespace pragma {
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
