// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.slider;

export import :gui.progress_bar;

export namespace pragma::gui::types {
	class DLLCLIENT WISlider : public WIProgressBar {
	  public:
		WISlider();
		virtual ~WISlider() override;
		virtual void Initialize() override;
		virtual util::EventReply MouseCallback(platform::MouseButton button, platform::KeyState state, platform::Modifier mods) override;
		virtual void Think(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd) override;

		bool IsBeingDragged() const;
	  private:
		bool m_bMoveSlider;
	};
};
