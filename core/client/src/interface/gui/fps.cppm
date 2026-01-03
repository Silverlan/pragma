// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.fps;

export import pragma.gui;

export namespace pragma::gui::types {
	class DLLCLIENT WIFPS : public WIBase {
	  private:
		WIHandle m_text;
		uint32_t m_fpsLast;
		double m_tLastUpdate;
	  public:
		WIFPS();
		virtual ~WIFPS() override;
		virtual void Initialize() override;
		virtual void Think(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd) override;
	};
};
