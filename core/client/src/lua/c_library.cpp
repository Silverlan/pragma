#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/lua/libraries/c_ldebugoverlay.h"
#include "pragma/lua/libraries/c_lgui.h"
#include "pragma/lua/libraries/c_lphysx.h"
#include "pragma/lua/libraries/c_lsound.h"
#include "pragma/lua/classes/c_lshader.h"
#include "pragma/lua/libraries/c_lutil.h"
#include "pragma/lua/libraries/c_linput.h"
#include "pragma/audio/c_laleffect.h"
#include "pragma/audio/c_lalsound.hpp"
#include "pragma/gui/wiluabase.h"
#include "pragma/lua/classes/c_lwibase.h"
#include "pragma/lua/classes/c_ldef_wgui.h"
#include "pragma/ai/c_lai.hpp"
#include <alsoundsystem.hpp>
#include <luainterface.hpp>

static void register_gui(Lua::Interface &lua)
{
	auto *l = lua.GetState();
	lua.RegisterLibrary("gui",{
		{"create",Lua::gui::create},
		{"create_label",Lua::gui::create_label},
		{"register",Lua::gui::register_element},
		{"get_base_element",Lua::gui::get_base_element},
		{"get_focused_element",Lua::gui::get_focused_element},
		{"register_skin",Lua::gui::register_skin},
		{"set_skin",Lua::gui::set_skin},
		{"skin_exists",Lua::gui::skin_exists},
		{"get_cursor",Lua::gui::get_cursor},
		{"set_cursor",Lua::gui::set_cursor},
		{"get_cursor_input_mode",Lua::gui::get_cursor_input_mode},
		{"set_cursor_input_mode",Lua::gui::set_cursor_input_mode},
		{"get_window_size",Lua::gui::get_window_size},
		{"inject_mouse_input",Lua::gui::inject_mouse_input},
		{"inject_keyboard_input",Lua::gui::inject_keyboard_input},
		{"inject_char_input",Lua::gui::inject_char_input},
		{"inject_scroll_input",Lua::gui::inject_scroll_input}
	});
	auto guiMod = luabind::module(l,"gui");

	//
	auto wiElementClassDef = luabind::class_<WIHandle>("Element");
	Lua::WIBase::register_class(wiElementClassDef);
	guiMod[wiElementClassDef];

	// Custom Classes
	auto wiBaseWIElement = luabind::class_<WILuaHandle COMMA WILuaWrapper COMMA luabind::bases<WIHandle>>("Base");
	wiBaseWIElement.def(luabind::constructor<>());
	wiBaseWIElement.def("OnInitialize",&WILuaWrapper::OnInitialize,&WILuaWrapper::default_OnInitialize);
	wiBaseWIElement.def("OnThink",&WILuaWrapper::OnThink,&WILuaWrapper::default_OnThink);
	wiBaseWIElement.def("MouseCallback",&WILuaWrapper::MouseCallback,&WILuaWrapper::default_MouseCallback);
	wiBaseWIElement.def("KeyboardCallback",&WILuaWrapper::KeyboardCallback,&WILuaWrapper::default_KeyboardCallback);
	wiBaseWIElement.def("CharCallback",&WILuaWrapper::CharCallback,&WILuaWrapper::default_CharCallback);
	wiBaseWIElement.def("ScrollCallback",&WILuaWrapper::ScrollCallback,&WILuaWrapper::default_ScrollCallback);
	wiBaseWIElement.def("OnVisibilityChanged",&WILuaWrapper::OnSetVisible,&WILuaWrapper::default_OnSetVisible);
	wiBaseWIElement.def("OnSizeChanged",&WILuaWrapper::OnSetSize,&WILuaWrapper::default_OnSetSize);
	wiBaseWIElement.def("OnColorChanged",&WILuaWrapper::OnSetColor,&WILuaWrapper::default_OnSetColor);
	wiBaseWIElement.def("OnAlphaChanged",&WILuaWrapper::OnSetAlpha,&WILuaWrapper::default_OnSetAlpha);
	wiBaseWIElement.def("OnDraw",&WILuaWrapper::Render,&WILuaWrapper::default_Render);
	wiBaseWIElement.def("OnCursorEntered",&WILuaWrapper::OnCursorEntered,&WILuaWrapper::default_OnCursorEntered);
	wiBaseWIElement.def("OnCursorExited",&WILuaWrapper::OnCursorExited,&WILuaWrapper::default_OnCursorExited);
	wiBaseWIElement.def("OnFocusGained",&WILuaWrapper::OnFocusGained,&WILuaWrapper::default_OnFocusGained);
	wiBaseWIElement.def("OnFocusKilled",&WILuaWrapper::OnFocusKilled,&WILuaWrapper::default_OnFocusKilled);
	wiBaseWIElement.def("OnRemove",&WILuaWrapper::OnRemove,&WILuaWrapper::default_OnRemove);
	guiMod[wiBaseWIElement];
	//

	// Class specific handles have to also be defined in CGame::InitializeGUIElement and WGUIHandleFactory!
	auto wiShapeClassDef = luabind::class_<WIShapeHandle COMMA WIHandle>("Shape");
	Lua::WIShape::register_class(wiShapeClassDef);
	guiMod[wiShapeClassDef];

	auto wiButtonClassDef = luabind::class_<WIButtonHandle COMMA WIHandle>("Button");
	Lua::WIButton::register_class(wiButtonClassDef);
	guiMod[wiButtonClassDef];

	auto wiTexturedShapeClassDef = luabind::class_<WITexturedShapeHandle COMMA luabind::bases<WIShapeHandle COMMA WIHandle>>("TexturedShape");
	Lua::WITexturedShape::register_class(wiTexturedShapeClassDef);
	guiMod[wiTexturedShapeClassDef];

	auto wiIconClassDef = luabind::class_<WIIconHandle COMMA luabind::bases<WITexturedShapeHandle COMMA WIShapeHandle COMMA WIHandle>>("Icon");
	Lua::WIIcon::register_class(wiIconClassDef);
	guiMod[wiIconClassDef];

	auto wiSilkIconClassDef = luabind::class_<WISilkIconHandle COMMA luabind::bases<WIIconHandle COMMA WITexturedShapeHandle COMMA WIShapeHandle COMMA WIHandle>>("SilkIcon");
	Lua::WISilkIcon::register_class(wiSilkIconClassDef);
	guiMod[wiSilkIconClassDef];

	auto wiArrowClassDef = luabind::class_<WIArrowHandle COMMA luabind::bases<WIShapeHandle COMMA WIHandle>>("Arrow");
	Lua::WIArrow::register_class(wiArrowClassDef);
	guiMod[wiArrowClassDef];

	auto wiCheckboxClassDef = luabind::class_<WICheckboxHandle COMMA luabind::bases<WIShapeHandle COMMA WIHandle>>("Checkbox");
	Lua::WICheckbox::register_class(wiCheckboxClassDef);
	guiMod[wiCheckboxClassDef];

	auto wiTransformableClassDef = luabind::class_<WITransformableHandle COMMA WIHandle>("Transformable");
	Lua::WITransformable::register_class(wiTransformableClassDef);
	guiMod[wiTransformableClassDef];

	auto wiProgressBarClassDef = luabind::class_<WIProgressBarHandle COMMA WIHandle>("ProgressBar");
	Lua::WIProgressBar::register_class(wiProgressBarClassDef);
	guiMod[wiProgressBarClassDef];

	//auto wiSliderClassDef = luabind::class_<WISliderHandle COMMA luabind::bases<WIProgressBarHandle COMMA WIHandle>>("WISlider");
	//Lua::WISlider::register_class(wiSliderClassDef);
	//lua_bind(wiSliderClassDef);

	auto wiContainerClassDef = luabind::class_<WIContainerHandle COMMA WIHandle>("Container");
	Lua::WIContainer::register_class(wiContainerClassDef);
	guiMod[wiContainerClassDef];

	auto wiTableClassDef = luabind::class_<WITableHandle COMMA luabind::bases<WIContainerHandle COMMA WIHandle>>("Table");
	Lua::WITable::register_class(wiTableClassDef);

	auto wiTableRowClassDef = luabind::class_<WITableRowHandle COMMA luabind::bases<WIContainerHandle COMMA WIHandle>>("Row");
	Lua::WITableRow::register_class(wiTableRowClassDef);

	auto wiTableCellClassDef = luabind::class_<WITableCellHandle COMMA luabind::bases<WIContainerHandle COMMA WIHandle>>("Cell");
	Lua::WITableCell::register_class(wiTableCellClassDef);
	wiTableClassDef.scope[wiTableCellClassDef];
	wiTableClassDef.scope[wiTableRowClassDef];
	guiMod[wiTableClassDef];

	auto wiGridPanelClassDef = luabind::class_<WIGridPanelHandle COMMA luabind::bases<WITableHandle COMMA WIHandle>>("GridPanel");
	Lua::WIGridPanel::register_class(wiGridPanelClassDef);
	guiMod[wiGridPanelClassDef];

	auto wiTreeListClassDef = luabind::class_<WITreeListHandle COMMA luabind::bases<WITableHandle COMMA WIHandle>>("TreeList");
	Lua::WITreeList::register_class(wiTreeListClassDef);

	auto wiTreeListElementClassDef = luabind::class_<WITreeListElementHandle COMMA luabind::bases<WITableRowHandle COMMA WIHandle>>("Element");
	Lua::WITreeListElement::register_class(wiTreeListElementClassDef);
	wiTreeListClassDef.scope[wiTreeListElementClassDef];
	guiMod[wiTreeListClassDef];

	auto wiFrameClassDef = luabind::class_<WIFrameHandle COMMA luabind::bases<WITransformableHandle COMMA WIHandle>>("Frame");
	Lua::WIFrame::register_class(wiFrameClassDef);
	guiMod[wiFrameClassDef];

	auto wiTextClassDef = luabind::class_<WITextHandle COMMA WIHandle>("Text");
	Lua::WIText::register_class(wiTextClassDef);
	guiMod[wiTextClassDef];

	auto wiTextEntryClassDef = luabind::class_<WITextEntryHandle COMMA WIHandle>("TextEntry");
	Lua::WITextEntry::register_class(wiTextEntryClassDef);
	guiMod[wiTextEntryClassDef];

	auto wiOutlinedRectClassDef = luabind::class_<WIOutlinedRectHandle COMMA WIHandle>("OutlinedRect");
	Lua::WIOutlinedRect::register_class(wiOutlinedRectClassDef);

	auto wiLineClassDef = luabind::class_<WILineHandle COMMA WIHandle>("Line");
	Lua::WILine::register_class(wiLineClassDef);

	auto wiRoundedRectClassDef = luabind::class_<WIRoundedRectHandle COMMA luabind::bases<WIShapeHandle COMMA WIHandle>>("RoundedRect");
	Lua::WIRoundedRect::register_class(wiRoundedRectClassDef);
	guiMod[wiRoundedRectClassDef];

	auto wiRoundedTexturedRect = luabind::class_<WIRoundedTexturedRectHandle COMMA luabind::bases<WITexturedShapeHandle COMMA WIShapeHandle COMMA WIHandle>>("RoundedTexturedRect");
	Lua::WIRoundedTexturedRect::register_class(wiRoundedTexturedRect);
	guiMod[wiRoundedTexturedRect];

	auto wiScrollBarClassDef = luabind::class_<WIScrollBarHandle COMMA WIHandle>("ScrollBar");
	Lua::WIScrollBar::register_class(wiScrollBarClassDef);

	auto wiNumericEntryClassDef = luabind::class_<WINumericEntryHandle COMMA luabind::bases<WITextEntryHandle COMMA WIHandle>>("NumericTextEntry");
	Lua::WINumericEntry::register_class(wiNumericEntryClassDef);
	guiMod[wiNumericEntryClassDef];

	auto wiDropDownMenuClassDef = luabind::class_<WIDropDownMenuHandle COMMA luabind::bases<WITextEntryHandle COMMA WIHandle>>("DropDownMenu");
	Lua::WIDropDownMenu::register_class(wiDropDownMenuClassDef);
	guiMod[wiDropDownMenuClassDef];

	guiMod[wiOutlinedRectClassDef];
	guiMod[wiLineClassDef];
	guiMod[wiScrollBarClassDef];
}

void ClientState::RegisterSharedLuaLibraries(Lua::Interface &lua,bool bGUI)
{
	register_gui(lua);

	Lua::RegisterLibrary(lua.GetState(),"input",{
		{"get_mouse_button_state",Lua::input::get_mouse_button_state},
		{"get_key_state",Lua::input::get_key_state},
		{"add_callback",Lua::input::add_callback},
		{"get_cursor_pos",Lua::input::get_cursor_pos},
		{"set_cursor_pos",Lua::input::set_cursor_pos},
		{"get_controller_count",Lua::input::get_controller_count},
		{"get_controller_name",Lua::input::get_controller_name},
		{"get_controller_axes",Lua::input::get_joystick_axes},
		{"get_controller_buttons",Lua::input::get_joystick_buttons},

		{"key_to_string",Lua::input::key_to_string},
		{"key_to_text",Lua::input::key_to_text},
		{"string_to_key",Lua::input::string_to_key},
		{"get_mapped_keys",Lua::input::get_mapped_keys}
	});

	Lua::RegisterLibrary(lua.GetState(),"sound",{
		{"create",Lua::sound::create},
		LUA_LIB_SOUND_SHARED,
		{"create_dsp_effect",Lua::sound::register_aux_effect},
		{"get_dsp_effect",Lua::sound::get_aux_effect},
		{"set_distance_model",&Lua::sound::set_distance_model},
		{"get_distance_model",&Lua::sound::get_distance_model},
		{"is_supported",&Lua::sound::is_supported},
		{"get_doppler_factor",&Lua::sound::get_doppler_factor},
		{"set_doppler_factor",&Lua::sound::set_doppler_factor},
		{"get_speed_of_sound",&Lua::sound::get_speed_of_sound},
		{"set_speed_of_sound",&Lua::sound::set_speed_of_sound},
		{"get_device_name",&Lua::sound::get_device_name},
		{"add_global_effect",&Lua::sound::add_global_effect},
		{"remove_global_effect",&Lua::sound::remove_global_effect},
		{"set_global_effect_parameters",&Lua::sound::set_global_effect_parameters}
	});
	Lua::RegisterLibraryEnums(lua.GetState(),"sound",{
		{"GLOBAL_EFFECT_FLAG_NONE",umath::to_integral(al::SoundSystem::GlobalEffectFlag::None)},
		{"GLOBAL_EFFECT_FLAG_BIT_RELATIVE",umath::to_integral(al::SoundSystem::GlobalEffectFlag::RelativeSounds)},
		{"GLOBAL_EFFECT_FLAG_BIT_WORLD",umath::to_integral(al::SoundSystem::GlobalEffectFlag::WorldSounds)},
		{"GLOBAL_EFFECT_FLAG_ALL",umath::to_integral(al::SoundSystem::GlobalEffectFlag::All)},
		
		{"DISTANCE_MODEL_NONE",umath::to_integral(al::DistanceModel::None)},
		{"DISTANCE_MODEL_INVERSE_CLAMPED",umath::to_integral(al::DistanceModel::InverseClamped)},
		{"DISTANCE_MODEL_LINEAR_CLAMPED",umath::to_integral(al::DistanceModel::LinearClamped)},
		{"DISTANCE_MODEL_EXPONENT_CLAMPED",umath::to_integral(al::DistanceModel::ExponentClamped)},
		{"DISTANCE_MODEL_INVERSE",umath::to_integral(al::DistanceModel::Inverse)},
		{"DISTANCE_MODEL_LINEAR",umath::to_integral(al::DistanceModel::Linear)},
		{"DISTANCE_MODEL_EXPONENT",umath::to_integral(al::DistanceModel::Exponent)}
	});
	Lua::sound::register_enums(lua.GetState());

	auto classDefAlEffect = luabind::class_<al::PEffect>("Effect");
	classDefAlEffect.def("SetProperties",static_cast<void(*)(lua_State*,al::PEffect&,const al::EfxEaxReverbProperties&)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties",static_cast<void(*)(lua_State*,al::PEffect&,const al::EfxChorusProperties&)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties",static_cast<void(*)(lua_State*,al::PEffect&,const al::EfxDistortionProperties&)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties",static_cast<void(*)(lua_State*,al::PEffect&,const al::EfxEchoProperties&)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties",static_cast<void(*)(lua_State*,al::PEffect&,const al::EfxFlangerProperties&)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties",static_cast<void(*)(lua_State*,al::PEffect&,const al::EfxFrequencyShifterProperties&)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties",static_cast<void(*)(lua_State*,al::PEffect&,const al::EfxVocalMorpherProperties&)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties",static_cast<void(*)(lua_State*,al::PEffect&,const al::EfxPitchShifterProperties&)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties",static_cast<void(*)(lua_State*,al::PEffect&,const al::EfxRingModulatorProperties&)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties",static_cast<void(*)(lua_State*,al::PEffect&,const al::EfxAutoWahProperties&)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties",static_cast<void(*)(lua_State*,al::PEffect&,const al::EfxCompressor&)>(&Lua::ALEffect::SetProperties));
	classDefAlEffect.def("SetProperties",static_cast<void(*)(lua_State*,al::PEffect&,const al::EfxEqualizer&)>(&Lua::ALEffect::SetProperties));

	auto classDefEaxReverb = luabind::class_<al::EfxEaxReverbProperties>("ReverbProperties");
	classDefEaxReverb.def(luabind::constructor<>());
	classDefEaxReverb.def_readwrite("density",&al::EfxEaxReverbProperties::flDensity);
	classDefEaxReverb.def_readwrite("diffusion",&al::EfxEaxReverbProperties::flDiffusion);
	classDefEaxReverb.def_readwrite("gain",&al::EfxEaxReverbProperties::flGain);
	classDefEaxReverb.def_readwrite("gainHF",&al::EfxEaxReverbProperties::flGainHF);
	classDefEaxReverb.def_readwrite("gainLF",&al::EfxEaxReverbProperties::flGainLF);
	classDefEaxReverb.def_readwrite("decayTime",&al::EfxEaxReverbProperties::flDecayTime);
	classDefEaxReverb.def_readwrite("decayHFRatio",&al::EfxEaxReverbProperties::flDecayHFRatio);
	classDefEaxReverb.def_readwrite("decayLFRatio",&al::EfxEaxReverbProperties::flDecayLFRatio);
	classDefEaxReverb.def_readwrite("reflectionsGain",&al::EfxEaxReverbProperties::flReflectionsGain);
	classDefEaxReverb.def_readwrite("reflectionsDelay",&al::EfxEaxReverbProperties::flReflectionsDelay);
	classDefEaxReverb.def_readwrite("reflectionsPan",reinterpret_cast<Vector3 al::EfxEaxReverbProperties::*>(&al::EfxEaxReverbProperties::flReflectionsPan));
	classDefEaxReverb.def_readwrite("lateReverbGain",&al::EfxEaxReverbProperties::flLateReverbGain);
	classDefEaxReverb.def_readwrite("lateReverbDelay",&al::EfxEaxReverbProperties::flLateReverbDelay);
	classDefEaxReverb.def_readwrite("lateReverbPan",reinterpret_cast<Vector3 al::EfxEaxReverbProperties::*>(&al::EfxEaxReverbProperties::flLateReverbPan));
	classDefEaxReverb.def_readwrite("echoTime",&al::EfxEaxReverbProperties::flEchoTime);
	classDefEaxReverb.def_readwrite("echoDepth",&al::EfxEaxReverbProperties::flEchoDepth);
	classDefEaxReverb.def_readwrite("modulationTime",&al::EfxEaxReverbProperties::flModulationTime);
	classDefEaxReverb.def_readwrite("modulationDepth",&al::EfxEaxReverbProperties::flModulationDepth);
	classDefEaxReverb.def_readwrite("airAbsorptionGainHF",&al::EfxEaxReverbProperties::flAirAbsorptionGainHF);
	classDefEaxReverb.def_readwrite("hfReference",&al::EfxEaxReverbProperties::flHFReference);
	classDefEaxReverb.def_readwrite("lfReference",&al::EfxEaxReverbProperties::flLFReference);
	classDefEaxReverb.def_readwrite("roomRolloffFactor",&al::EfxEaxReverbProperties::flRoomRolloffFactor);
	classDefEaxReverb.def_readwrite("decayHFLimit",&al::EfxEaxReverbProperties::iDecayHFLimit);
	classDefAlEffect.scope[classDefEaxReverb];

	auto classDefChorus = luabind::class_<al::EfxChorusProperties>("ChorusProperties");
	classDefChorus.def(luabind::constructor<>());
	classDefChorus.def_readwrite("waveform",&al::EfxChorusProperties::iWaveform);
	classDefChorus.def_readwrite("phase",&al::EfxChorusProperties::iPhase);
	classDefChorus.def_readwrite("rate",&al::EfxChorusProperties::flRate);
	classDefChorus.def_readwrite("depth",&al::EfxChorusProperties::flDepth);
	classDefChorus.def_readwrite("feedback",&al::EfxChorusProperties::flFeedback);
	classDefChorus.def_readwrite("delay",&al::EfxChorusProperties::flDelay);
	classDefAlEffect.scope[classDefChorus];

	auto classDefDistortionProperties = luabind::class_<al::EfxDistortionProperties>("DistortionProperties");
	classDefDistortionProperties.def(luabind::constructor<>());
	classDefDistortionProperties.def_readwrite("edge",&al::EfxDistortionProperties::flEdge);
	classDefDistortionProperties.def_readwrite("gain",&al::EfxDistortionProperties::flGain);
	classDefDistortionProperties.def_readwrite("lowpassCutoff",&al::EfxDistortionProperties::flLowpassCutoff);
	classDefDistortionProperties.def_readwrite("eqCenter",&al::EfxDistortionProperties::flEQCenter);
	classDefDistortionProperties.def_readwrite("eqBandwidth",&al::EfxDistortionProperties::flEQBandwidth);
	classDefAlEffect.scope[classDefDistortionProperties];

	auto classDefEchoProperties = luabind::class_<al::EfxEchoProperties>("EchoProperties");
	classDefEchoProperties.def(luabind::constructor<>());
	classDefEchoProperties.def_readwrite("delay",&al::EfxEchoProperties::flDelay);
	classDefEchoProperties.def_readwrite("lrDelay",&al::EfxEchoProperties::flLRDelay);
	classDefEchoProperties.def_readwrite("damping",&al::EfxEchoProperties::flDamping);
	classDefEchoProperties.def_readwrite("feedback",&al::EfxEchoProperties::flFeedback);
	classDefEchoProperties.def_readwrite("spread",&al::EfxEchoProperties::flSpread);
	classDefAlEffect.scope[classDefEchoProperties];

	auto classDefFlangerProperties = luabind::class_<al::EfxFlangerProperties>("FlangerProperties");
	classDefFlangerProperties.def(luabind::constructor<>());
	classDefFlangerProperties.def_readwrite("waveform",&al::EfxFlangerProperties::iWaveform);
	classDefFlangerProperties.def_readwrite("phase",&al::EfxFlangerProperties::iPhase);
	classDefFlangerProperties.def_readwrite("rate",&al::EfxFlangerProperties::flRate);
	classDefFlangerProperties.def_readwrite("depth",&al::EfxFlangerProperties::flDepth);
	classDefFlangerProperties.def_readwrite("feedback",&al::EfxFlangerProperties::flFeedback);
	classDefFlangerProperties.def_readwrite("delay",&al::EfxFlangerProperties::flDelay);
	classDefAlEffect.scope[classDefFlangerProperties];

	auto classDefFrequencyShifterProperties = luabind::class_<al::EfxFrequencyShifterProperties>("FrequencyShifterProperties");
	classDefFrequencyShifterProperties.def(luabind::constructor<>());
	classDefFrequencyShifterProperties.def_readwrite("frequency",&al::EfxFrequencyShifterProperties::flFrequency);
	classDefFrequencyShifterProperties.def_readwrite("leftDirection",&al::EfxFrequencyShifterProperties::iLeftDirection);
	classDefFrequencyShifterProperties.def_readwrite("rightDirection",&al::EfxFrequencyShifterProperties::iRightDirection);
	classDefAlEffect.scope[classDefFrequencyShifterProperties];

	auto classDefVocalMorpherProperties = luabind::class_<al::EfxVocalMorpherProperties>("VocalMorpherProperties");
	classDefVocalMorpherProperties.def(luabind::constructor<>());
	classDefVocalMorpherProperties.def_readwrite("phonemeA",&al::EfxVocalMorpherProperties::iPhonemeA);
	classDefVocalMorpherProperties.def_readwrite("phonemeB",&al::EfxVocalMorpherProperties::iPhonemeB);
	classDefVocalMorpherProperties.def_readwrite("phonemeACoarseTuning",&al::EfxVocalMorpherProperties::iPhonemeACoarseTuning);
	classDefVocalMorpherProperties.def_readwrite("phonemeBCoarseTuning",&al::EfxVocalMorpherProperties::iPhonemeBCoarseTuning);
	classDefVocalMorpherProperties.def_readwrite("waveform",&al::EfxVocalMorpherProperties::iWaveform);
	classDefVocalMorpherProperties.def_readwrite("rate",&al::EfxVocalMorpherProperties::flRate);
	classDefAlEffect.scope[classDefVocalMorpherProperties];

	auto classDefPitchShifterProperties = luabind::class_<al::EfxPitchShifterProperties>("PitchShifterProperties");
	classDefPitchShifterProperties.def(luabind::constructor<>());
	classDefPitchShifterProperties.def_readwrite("coarseTune",&al::EfxPitchShifterProperties::iCoarseTune);
	classDefPitchShifterProperties.def_readwrite("fineTune",&al::EfxPitchShifterProperties::iFineTune);
	classDefAlEffect.scope[classDefPitchShifterProperties];

	auto classDefRingModulatorProperties = luabind::class_<al::EfxRingModulatorProperties>("RingModulatorProperties");
	classDefRingModulatorProperties.def(luabind::constructor<>());
	classDefRingModulatorProperties.def_readwrite("frequency",&al::EfxRingModulatorProperties::flFrequency);
	classDefRingModulatorProperties.def_readwrite("highpassCutoff",&al::EfxRingModulatorProperties::flHighpassCutoff);
	classDefRingModulatorProperties.def_readwrite("waveform",&al::EfxRingModulatorProperties::iWaveform);
	classDefAlEffect.scope[classDefRingModulatorProperties];

	auto classDefAutoWahProperties = luabind::class_<al::EfxAutoWahProperties>("AutoWahProperties");
	classDefAutoWahProperties.def(luabind::constructor<>());
	classDefAutoWahProperties.def_readwrite("attackTime",&al::EfxAutoWahProperties::flAttackTime);
	classDefAutoWahProperties.def_readwrite("releaseTime",&al::EfxAutoWahProperties::flReleaseTime);
	classDefAutoWahProperties.def_readwrite("resonance",&al::EfxAutoWahProperties::flResonance);
	classDefAutoWahProperties.def_readwrite("peakGain",&al::EfxAutoWahProperties::flPeakGain);
	classDefAlEffect.scope[classDefAutoWahProperties];

	auto classDefCompressor = luabind::class_<al::EfxCompressor>("CompressorProperties");
	classDefCompressor.def(luabind::constructor<>());
	classDefCompressor.def_readwrite("onOff",&al::EfxCompressor::iOnOff);
	classDefAlEffect.scope[classDefCompressor];

	auto classDefEqualizer = luabind::class_<al::EfxEqualizer>("EqualizerProperties");
	classDefEqualizer.def(luabind::constructor<>());
	classDefEqualizer.def_readwrite("lowGain",&al::EfxEqualizer::flLowGain);
	classDefEqualizer.def_readwrite("lowCutoff",&al::EfxEqualizer::flLowCutoff);
	classDefEqualizer.def_readwrite("mid1Gain",&al::EfxEqualizer::flMid1Gain);
	classDefEqualizer.def_readwrite("mid1Center",&al::EfxEqualizer::flMid1Center);
	classDefEqualizer.def_readwrite("mid1Width",&al::EfxEqualizer::flMid1Width);
	classDefEqualizer.def_readwrite("mid2Gain",&al::EfxEqualizer::flMid2Gain);
	classDefEqualizer.def_readwrite("mid2Center",&al::EfxEqualizer::flMid2Center);
	classDefEqualizer.def_readwrite("mid2Width",&al::EfxEqualizer::flMid2Width);
	classDefEqualizer.def_readwrite("highGain",&al::EfxEqualizer::flHighGain);
	classDefEqualizer.def_readwrite("highCutoff",&al::EfxEqualizer::flHighCutoff);
	classDefAlEffect.scope[classDefEqualizer];

	auto alSoundClassDef = luabind::class_<std::shared_ptr<ALSound>>("Source");
	Lua::ALSound::Client::register_class(alSoundClassDef);

	auto soundMod = luabind::module(lua.GetState(),"sound");
	soundMod[classDefAlEffect];
	soundMod[alSoundClassDef];

	RegisterVulkanLuaInterface(lua);
}
void CGame::RegisterLuaLibraries()
{
	GetLuaInterface().RegisterLibrary("util",{
		REGISTER_SHARED_UTIL
		{"calc_world_direction_from_2d_coordinates",Lua::util::Client::calc_world_direction_from_2d_coordinates},
		{"create_particle_tracer",Lua::util::Client::create_particle_tracer},
		{"fire_bullets",Lua::util::fire_bullets},
		{"create_muzzle_flash",Lua::util::Client::create_muzzle_flash},
		{"create_giblet",Lua::util::Client::create_giblet}
	});

	Lua::ai::client::register_library(GetLuaInterface());

	Game::RegisterLuaLibraries();
	ClientState::RegisterSharedLuaLibraries(GetLuaInterface());

	std::vector<luaL_Reg> debugFuncs = {
		{"draw_points",Lua::DebugRenderer::Client::DrawPoints},
		{"draw_lines",Lua::DebugRenderer::Client::DrawLines},
		{"draw_point",Lua::DebugRenderer::Client::DrawPoint},
		{"draw_line",Lua::DebugRenderer::Client::DrawLine},
		{"draw_box",Lua::DebugRenderer::Client::DrawBox},
		{"draw_mesh",Lua::DebugRenderer::Client::DrawMeshes},
		{"draw_sphere",Lua::DebugRenderer::Client::DrawSphere},
		{"draw_cone",Lua::DebugRenderer::Client::DrawCone},
		{"draw_truncated_cone",Lua::DebugRenderer::Client::DrawTruncatedCone},
		{"draw_cylinder",Lua::DebugRenderer::Client::DrawCylinder},
		{"draw_axis",Lua::DebugRenderer::Client::DrawAxis},
		{"draw_text",Lua::DebugRenderer::Client::DrawText},
		{"draw_path",Lua::DebugRenderer::Client::DrawPath},
		{"draw_spline",Lua::DebugRenderer::Client::DrawSpline},
		{"draw_plane",Lua::DebugRenderer::Client::DrawPlane},
		{"draw_frustum",Lua::DebugRenderer::Client::DrawFrustum}
	};
	for(auto &f : debugFuncs)
	{
		lua_pushtablecfunction(GetLuaState(),"debug",(f.name),(f.func));
	}

#ifdef PHYS_ENGINE_PHYSX
	lua_pushtablecfunction("physx","SetVisualizationParameter",Lua_physx_SetVisualizationParameter);
	lua_pushtablecfunction("physx","SetVisualizationEnabled",Lua_physx_SetVisualizationEnabled);
#endif
}
