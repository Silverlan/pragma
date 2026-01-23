// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.motion_blur_data;

import :rendering.shaders;

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
			math::Transform pose;
			std::shared_ptr<prosper::IBuffer> boneBuffer;
			std::shared_ptr<prosper::IDescriptorSetGroup> boneDsg;
		};
		std::unordered_map<const ecs::BaseEntity *, PoseData> prevModelMatrices;
		std::unordered_map<const ecs::BaseEntity *, PoseData> curModelMatrices;
		MotionBlurCameraData cameraData;
		double lastTick = 0.0;
	};

	DLLCLIENT ShaderVelocityBuffer *get_velocity_buffer_shader();
	class DLLCLIENT CMotionBlurDataComponent final : public BaseEntityComponent {
	  public:
		CMotionBlurDataComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua::State *l) override;

		void UpdateEntityPoses();
		const MotionBlurTemporalData &GetMotionBlurData() const { return m_motionBlurData; }
		size_t GetLastUpdateIndex() const { return m_lastUpdateIndex; }
	  private:
		MotionBlurTemporalData m_motionBlurData {};
		size_t m_lastUpdateIndex = 0;
	};
};
