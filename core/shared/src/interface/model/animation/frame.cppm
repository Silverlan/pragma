// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:model.animation.frame;

export import :game.coordinate_system;
export import :model.animation.skeleton;
export import :types;

export {
	struct DLLNETWORK FlexFrameData {
		std::vector<float> flexControllerWeights;
		std::vector<uint32_t> flexControllerIds;

		bool operator==(const FlexFrameData &other) const { return flexControllerWeights == other.flexControllerWeights && flexControllerIds == other.flexControllerIds; }
		bool operator!=(const FlexFrameData &other) const { return !operator==(other); }
	};

	class DLLNETWORK Frame : public std::enable_shared_from_this<Frame> {
	  public:
		static std::shared_ptr<Frame> Create(unsigned int numBones);
		static std::shared_ptr<Frame> Create(const Frame &other);

		void SetBonePosition(unsigned int boneID, const Vector3 &pos);
		void SetBoneOrientation(unsigned int boneID, const Quat &orientation);
		void SetBoneScale(uint32_t boneId, const Vector3 &scale);
		Vector3 *GetBonePosition(unsigned int boneID);
		Quat *GetBoneOrientation(unsigned int boneID);
		Vector3 *GetBoneScale(uint32_t boneId);
		const Vector3 *GetBonePosition(unsigned int boneID) const;
		const Quat *GetBoneOrientation(unsigned int boneID) const;
		const Vector3 *GetBoneScale(uint32_t boneId) const;
		bool GetBonePose(uint32_t boneId, pragma::math::ScaledTransform &outTransform) const;
		void SetBonePose(uint32_t boneId, const pragma::math::ScaledTransform &pose);
		void SetBonePose(uint32_t boneId, const pragma::math::Transform &pose);
		bool GetBoneMatrix(unsigned int boneID, Mat4 *mat);
		Vector2 *GetMoveOffset();
		void GetMoveOffset(float *x, float *z);
		void SetMoveOffset(float x, float z = 0);
		void SetMoveOffset(Vector2 move);
		void Localize(const pragma::animation::Animation &anim, const pragma::animation::Skeleton &skeleton);
		void Globalize(const pragma::animation::Animation &anim, const pragma::animation::Skeleton &skeleton);

		// These assume that the bones of the frame match the skeleton exactly
		void Localize(const pragma::animation::Skeleton &skeleton);
		void Globalize(const pragma::animation::Skeleton &skeleton);

		uint32_t GetBoneCount() const;
		void SetBoneCount(uint32_t numBones);
		std::pair<Vector3, Vector3> CalcRenderBounds(const pragma::animation::Animation &anim, const pragma::asset::Model &mdl) const;
		void Rotate(const Quat &rot);
		void Translate(const Vector3 &t);
		void Rotate(const pragma::animation::Skeleton &skeleton, const Quat &rot);
		void Translate(const pragma::animation::Skeleton &skeleton, const Vector3 &t);
		void Rotate(const pragma::animation::Animation &anim, const pragma::animation::Skeleton &skeleton, const Quat &rot);
		void Translate(const pragma::animation::Animation &anim, const pragma::animation::Skeleton &skeleton, const Vector3 &t);
		void Scale(const Vector3 &scale);
		void Mirror(pragma::Axis axis);

		const FlexFrameData &GetFlexFrameData() const;
		FlexFrameData &GetFlexFrameData();

		bool HasScaleTransforms() const;

		const std::vector<pragma::math::Transform> &GetBoneTransforms() const;
		const std::vector<Vector3> &GetBoneScales() const;
		std::vector<pragma::math::Transform> &GetBoneTransforms();
		std::vector<Vector3> &GetBoneScales();
		pragma::math::Transform *GetBoneTransform(uint32_t idx);
		const pragma::math::Transform *GetBoneTransform(uint32_t idx) const;
		void Validate();

		bool operator==(const Frame &other) const;
		bool operator!=(const Frame &other) const { return !operator==(other); }
	  private:
		Frame(unsigned int numBones);
		Frame(const Frame &other);
		std::vector<pragma::math::Transform> m_bones;
		std::vector<Vector3> m_scales;
		std::unique_ptr<Vector2> m_move;
		FlexFrameData m_flexFrameData {};
		std::vector<uint32_t> GetLocalRootBoneIds(const pragma::animation::Animation &anim, const pragma::animation::Skeleton &skeleton) const;

		void UpdateScales();
	};
};
