// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"

export module pragma.client:gui.image_slide_show;

import pragma.gui;

export {
	class DLLCLIENT WIImageSlideShow : public WIBase {
	private:
		struct PreloadImage {
			PreloadImage();
			Int32 image;
			std::shared_ptr<msys::Texture> texture;
			bool ready;
			bool loading;
		};
		WIHandle m_hImgPrev;
		WIHandle m_hImgNext;
		std::shared_ptr<prosper::Texture> m_lastTexture = nullptr;
		std::shared_ptr<prosper::BlurSet> m_blurSet = nullptr;
		int m_currentImg;
		ChronoTime m_tLastFade;
		std::vector<std::string> m_files;
		std::deque<size_t> m_randomShuffle;

		PreloadImage m_imgPreload;
		virtual void DoUpdate() override;
		void PreloadNextImage(Int32 img);
		void PreloadNextRandomShuffle();
		void DisplayPreloadedImage();
		void DisplayNextImage();
	public:
		WIImageSlideShow();
		virtual void Initialize() override;
		virtual void SetSize(int x, int y) override;
		virtual void Think(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd) override;
		virtual void SetColor(float r, float g, float b, float a = 1.f) override;
		void SetImages(const std::vector<std::string> &images);
	};
};
