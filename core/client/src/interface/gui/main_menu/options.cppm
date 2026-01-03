// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.main_menu_options;

import :gui.checkbox;
export import :gui.main_menu_base;

export namespace pragma::gui::types {
	class DLLCLIENT WIMainMenuOptions : public WIMainMenuBase {
	  public:
		WIMainMenuOptions();
		virtual ~WIMainMenuOptions() override;
		virtual void Initialize() override;
		virtual void Think(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd) override;
	  protected:
		std::vector<WIHandle> m_labels;
		WIHandle m_hResolutionList;
		WIHandle m_hVSync;
		WIHandle m_hAntiAliasing;
		WIHandle m_hSSAO;
		WIHandle m_hTextureFiltering;
		WIHandle m_hDOF;
		WIHandle hWindowMode;
		WIHandle m_hDisplay;
		WIHandle m_hShadowRes;
		WIHandle m_hTexQuality;
		WIHandle m_hMdlQuality;
		WIHandle m_hKeyBindings;
		WIHandle m_hActive;
		WIHandle m_hMessageBox;
		WIHandle m_hButtonApply;
		WIHandle m_hButtonReset;
		WIHandle m_hGPUMemoryUsage = {};
		WIHandle m_hLuaMemoryUsageGUI = {};
		WIHandle m_hLuaMemoryUsageClient = {};
		WIHandle m_hLuaMemoryUsageServer = {};

		WIHandle m_hGeneralSettings;
		WIHandle m_hVideoSettings;
		WIHandle m_hAudioSettings;
		WIHandle m_hSsao;
		WIHandle m_hdrr;
		WIHandle m_hBloom;
		WIHandle m_hMotionBlur;
		WIHandle m_hOcclusionCulling;
		WIHandle m_hDoF;
		WIHandle m_hPresentMode;
		WIHandle m_hParticleQuality;
		WIHandle m_hShaderQuality;
		WIHandle m_hShadowQuality;
		WIHandle m_hReflectionQuality;
		WIHandle m_hDynamicShadows;
		WIHandle m_hShadowUpdateFrequency;
		WIHandle m_hPssmShadowUpdateFrequencyOffset;
		WIHandle m_hPssmSplitCount;
		bool m_bInitialized = false;

		util::Clock::time_point m_tLastMemoryUsageUpdate = {};

		CallbackHandle m_joystickStateChanged = {};
		int m_yOffset;
		void Apply(platform::MouseButton button, platform::KeyState state, platform::Modifier mods);
		void ResetDefaults(platform::MouseButton button, platform::KeyState state, platform::Modifier mods);
		void ApplyOptions();
		void ApplyWindowSize();
		void CloseMessageBox();

		void CreateLabel(std::string text);
		WIDropDownMenu *CreateDropDownMenu(std::string text);
		WICheckbox *CreateCheckbox(std::string text);
		void InitializeGeneralSettings();
		void InitializeVideoSettings();
		void InitializeAudioSettings();
		void InitializeControlSettings();
		void SetActiveMenu(WIHandle &hMenu);
		void ShowGeneralSettings();
		void ShowVideoSettings();
		void ShowAudioSettings();
		void ShowControlSettings();
		virtual void InitializeOptionsList(WIOptionsList *pList) override;
		virtual void OnVisibilityChanged(bool bVisible) override;
		using WIMainMenuBase::InitializeOptionsList;

		void UpdateMemoryUsage();
	};
};
