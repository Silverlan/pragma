// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.line_graph;

export import pragma.gui;

export namespace pragma::gui::types {
	class DLLCLIENT WILineGraph : public WIBase {
	  public:
		WILineGraph();
		virtual void Initialize() override;
		using WIBase::SetSize;
		void SetValue(uint32_t idx, float val);
		void SetSegmentCount(uint32_t count);
		uint32_t GetSegmentCount() const;
	  protected:
		WIHandle m_hOutline;
		std::vector<float> m_values;
		std::vector<WIHandle> m_lines;
		virtual void DoUpdate() override;
	    virtual void OnSizeChanged(const Vector2i &oldSize, ChangeSource changeSource) override;
		void UpdateGraph();
		void UpdateLines();
	};
};
