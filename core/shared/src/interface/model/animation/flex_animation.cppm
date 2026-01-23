// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:model.animation.flex_animation;

export import :model.animation.enums;
export import pragma.udm;

export {
	class DLLNETWORK FlexAnimationFrame : public std::enable_shared_from_this<FlexAnimationFrame> {
	  public:
		FlexAnimationFrame() = default;
		FlexAnimationFrame(const FlexAnimationFrame &frame);
		std::vector<float> &GetValues() { return m_flexControllerValues; }
		const std::vector<float> &GetValues() const { return const_cast<FlexAnimationFrame *>(this)->GetValues(); }

		bool operator==(const FlexAnimationFrame &other) const;
		bool operator!=(const FlexAnimationFrame &other) const { return !operator==(other); }
	  private:
		std::vector<float> m_flexControllerValues;
	};

	class DLLNETWORK FlexAnimation : public std::enable_shared_from_this<FlexAnimation> {
	  public:
		static constexpr uint32_t FORMAT_VERSION = 1u;
		static constexpr auto PFLEXANIM_IDENTIFIER = "PFLEXANI";
		static std::shared_ptr<FlexAnimation> Load(ufile::IFile &f);
		static std::shared_ptr<FlexAnimation> Load(const udm::AssetData &data, std::string &outErr);
		FlexAnimation() = default;
		FlexAnimation(const FlexAnimation &other);
		std::vector<std::shared_ptr<FlexAnimationFrame>> &GetFrames() { return m_frames; }
		const std::vector<std::shared_ptr<FlexAnimationFrame>> &GetFrames() const { return const_cast<FlexAnimation *>(this)->GetFrames(); }
		std::vector<pragma::animation::FlexControllerId> &GetFlexControllerIds() { return m_flexControllerIds; }
		const std::vector<pragma::animation::FlexControllerId> &GetFlexControllerIds() const { return const_cast<FlexAnimation *>(this)->GetFlexControllerIds(); }
		uint32_t AddFlexControllerId(pragma::animation::FlexControllerId id);
		FlexAnimationFrame &AddFrame();
		void SetFlexControllerIds(std::vector<pragma::animation::FlexControllerId> &&ids);
		void SetFps(float fps) { m_fps = fps; }
		float GetFps() const { return m_fps; }

		bool Save(udm::AssetDataArg outData, std::string &outErr);
		bool SaveLegacy(std::shared_ptr<pragma::fs::VFilePtrInternalReal> &f);

		bool operator==(const FlexAnimation &other) const;
		bool operator!=(const FlexAnimation &other) const { return !operator==(other); }
	  private:
		bool LoadFromAssetData(const udm::AssetData &data, std::string &outErr);
		std::vector<std::shared_ptr<FlexAnimationFrame>> m_frames;
		std::vector<pragma::animation::FlexControllerId> m_flexControllerIds;
		float m_fps = 24.f;
	};
};
