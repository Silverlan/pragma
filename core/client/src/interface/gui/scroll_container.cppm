// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"

export module pragma.client:gui.scroll_container;

export import pragma.gui;

export namespace pragma::gui::types {
	class DLLCLIENT WIScrollContainer : public WIBase {
	  public:
		enum class StateFlags : uint8_t { None = 0u, AutoStickToBottom = 1u, ContentsWidthFixed = AutoStickToBottom << 1u, ContentsHeightFixed = ContentsWidthFixed << 1u };
		WIScrollContainer();
		virtual ~WIScrollContainer() override;
		virtual void Initialize() override;
		virtual void SetSize(int x, int y) override;
		virtual util::EventReply ScrollCallback(Vector2 offset, bool offsetAsPixels = false) override;
		virtual void SizeToContents(bool x = true, bool y = true) override;
		WIScrollBar *GetHorizontalScrollBar();
		WIScrollBar *GetVerticalScrollBar();
		int GetScrollBarWidthV();
		int GetScrollBarHeightH();
		int GetContentWidth();
		int GetContentHeight();
		Vector2i GetContentSize();
		void SetScrollAmount(int32_t amX, int32_t amY);
		void SetScrollAmountX(int32_t amX);
		void SetScrollAmountY(int32_t amY);
		WIBase *GetWrapperElement();
		void ScrollToElement(WIBase &el);
		void ScrollToElementX(WIBase &el);
		void ScrollToElementY(WIBase &el);

		void ScrollToBottom();

		void SetAutoStickToBottom(bool autoStick);
		bool ShouldAutoStickToBottom() const;

		void SetContentsWidthFixed(bool fixed);
		void SetContentsHeightFixed(bool fixed);
		bool IsContentsWidthFixed() const;
		bool IsContentsHeightFixed() const;
	  protected:
		virtual void DoUpdate() override;
		StateFlags m_scFlags = StateFlags::None;
		WIHandle m_hScrollBarH = {};
		WIHandle m_hScrollBarV = {};
		WIHandle m_hWrapper = {};
		CallbackHandle m_hChildAdded = {};
		CallbackHandle m_hChildRemoved = {};
		Vector2i m_lastOffset = {};
		int32_t m_scrollAmountX = 64;
		int32_t m_scrollAmountY = 64;
		std::unordered_map<WIBase *, std::vector<CallbackHandle>> m_childCallbackHandles;
		virtual void OnChildAdded(WIBase *child) override;
		virtual void OnChildRemoved(WIBase *child) override;
		void OnHScrollOffsetChanged(unsigned int offset);
		void OnVScrollOffsetChanged(unsigned int offset);
		void OnWrapperChildAdded(WIBase *child);
		void OnWrapperChildRemoved(WIBase *child);
		static void OnChildReleased(WIBase *child);
		static void OnChildSetSize(WIBase *child);
	};
	using namespace pragma::math::scoped_enum::bitwise;
};

export {
	REGISTER_ENUM_FLAGS(pragma::gui::types::WIScrollContainer::StateFlags)
}
