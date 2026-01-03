// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.progress_bar;

export import pragma.gui;

export namespace pragma::gui::types {
	class DLLCLIENT WIProgressBar : public WIBase {
	  public:
		WIProgressBar();
		virtual ~WIProgressBar() override;
		virtual void Initialize() override;
		virtual void SetSize(int x, int y) override;
		void SetProgress(float progress);
		float GetProgress() const;
		const util::PFloatProperty &GetProgressProperty() const;
		void SetValue(float v);
		void SetRange(float min, float max, float stepSize = 1.f);
		std::array<float, 3> GetRange() const;
		void SetOptions(const std::vector<std::string> &options);
		void AddOption(const std::string &option);
		float GetValue() const;
		void GetValue(std::string &str);
		void SetPostFix(const std::string &postfix);
		void SetValueTranslator(const std::function<std::string(float)> &translator);
		void SetLabelVisible(bool visible);
	  private:
		WIHandle m_hProgress;
		WIHandle m_hLabel;
		WIHandle m_hLabel2;
		float m_min;
		float m_max;
		float m_stepSize;
		util::PFloatProperty m_progress = nullptr;
		std::string m_postfix;
		std::vector<std::string> m_options;
		std::function<std::string(float)> m_valueTranslator;
		UInt32 m_numDecimals;
		float UpdateProgress(float progress);
		void UpdateText();
		void UpdateTextPosition();
		void UpdateOptions();
		void OnProgressChanged(float oldValue, float value);
		void SetRange(float min, float max, float stepSize, bool bEraseOptions);
	};
};
