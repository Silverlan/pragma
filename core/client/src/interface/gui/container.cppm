// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.container;

export import pragma.gui;

export namespace pragma::gui::types {
	class DLLCLIENT WIContainer : public WIBase {
	  public:
		WIContainer();
		enum class DLLCLIENT Padding : uint32_t { Top = 0, Right, Bottom, Left };
		void SetPadding(int32_t padding);
		void SetPadding(int32_t top, int32_t right, int32_t bottom, int32_t left);
		void SetPadding(Padding paddingType, int32_t padding);
		void SetPaddingTop(int32_t top);
		void SetPaddingRight(int32_t right);
		void SetPaddingBottom(int32_t bottom);
		void SetPaddingLeft(int32_t left);
		const std::array<int32_t, 4> &GetPadding() const;
		int32_t GetPaddingTop() const;
		int32_t GetPaddingRight() const;
		int32_t GetPaddingBottom() const;
		int32_t GetPaddingLeft() const;
		int32_t GetPadding(Padding paddingType) const;
		virtual void SizeToContents(bool x = true, bool y = true) override;

		virtual void OnChildAdded(WIBase *child) override;
		virtual void OnChildRemoved(WIBase *child) override;
	  protected:
		virtual void DoUpdate() override;
		std::array<int32_t, 4> m_padding;
		std::unordered_map<WIBase *, std::array<CallbackHandle, 2>> m_childCallbacks = {};
	};
}
