// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.detachable;

export import pragma.gui;

export namespace pragma::gui::types {
	class DLLCLIENT WIDetachable : public WIBase {
	  public:
		WIDetachable();
		virtual ~WIDetachable() override;
		virtual void OnRemove() override;
		void Detach();
		void Reattach();
		bool IsDetached() const;
	  protected:
		struct DetachedWindow {
			~DetachedWindow();
			void Clear();
			WIHandle reattachElement {};
			std::shared_ptr<prosper::Window> window;
			WIHandle detachedBg {};
			Vector2i origPos;
			Vector2i origSize;
			std::optional<std::array<float, 4>> origAnchor;
		};
		std::unique_ptr<DetachedWindow> m_detachedWindow = nullptr;
	};
}
