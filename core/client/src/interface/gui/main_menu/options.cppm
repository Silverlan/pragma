// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

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
		std::vector<pragma::gui::WIHandle> m_labels;
		pragma::gui::WIHandle m_hResolutionList;
		pragma::gui::WIHandle m_hVSync;
		pragma::gui::WIHandle m_hAntiAliasing;
		pragma::gui::WIHandle m_hSSAO;
		pragma::gui::WIHandle m_hTextureFiltering;
		pragma::gui::WIHandle m_hDOF;
		pragma::gui::WIHandle hWindowMode;
		pragma::gui::WIHandle m_hDisplay;
		pragma::gui::WIHandle m_hShadowRes;
		pragma::gui::WIHandle m_hTexQuality;
		pragma::gui::WIHandle m_hMdlQuality;
		pragma::gui::WIHandle m_hKeyBindings;
		pragma::gui::WIHandle m_hActive;
		pragma::gui::WIHandle m_hMessageBox;
		pragma::gui::WIHandle m_hButtonApply;
		pragma::gui::WIHandle m_hButtonReset;
		pragma::gui::WIHandle m_hGPUMemoryUsage = {};
		pragma::gui::WIHandle m_hLuaMemoryUsageGUI = {};
		pragma::gui::WIHandle m_hLuaMemoryUsageClient = {};
		pragma::gui::WIHandle m_hLuaMemoryUsageServer = {};

		pragma::gui::WIHandle m_hGeneralSettings;
		pragma::gui::WIHandle m_hVideoSettings;
		pragma::gui::WIHandle m_hAudioSettings;
		pragma::gui::WIHandle m_hSsao;
		pragma::gui::WIHandle m_hdrr;
		pragma::gui::WIHandle m_hBloom;
		pragma::gui::WIHandle m_hMotionBlur;
		pragma::gui::WIHandle m_hOcclusionCulling;
		pragma::gui::WIHandle m_hDoF;
		pragma::gui::WIHandle m_hPresentMode;
		pragma::gui::WIHandle m_hParticleQuality;
		pragma::gui::WIHandle m_hShaderQuality;
		pragma::gui::WIHandle m_hShadowQuality;
		pragma::gui::WIHandle m_hReflectionQuality;
		pragma::gui::WIHandle m_hDynamicShadows;
		pragma::gui::WIHandle m_hShadowUpdateFrequency;
		pragma::gui::WIHandle m_hPssmShadowUpdateFrequencyOffset;
		pragma::gui::WIHandle m_hPssmSplitCount;
		bool m_bInitialized = false;

		util::Clock::time_point m_tLastMemoryUsageUpdate = {};

		CallbackHandle m_joystickStateChanged = {};
		int m_yOffset;
		void Apply(pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods);
		void ResetDefaults(pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods);
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
