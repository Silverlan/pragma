#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/audio/c_laleffect.h"
#include "luasystem.h"
#include <alsound_effect.hpp>

void Lua::ALEffect::SetProperties(lua_State *l,al::PEffect &effect,const al::EfxEaxReverbProperties &props) {effect->SetProperties(props);}
void Lua::ALEffect::SetProperties(lua_State *l,al::PEffect &effect,const al::EfxChorusProperties &props) {effect->SetProperties(props);}
void Lua::ALEffect::SetProperties(lua_State *l,al::PEffect &effect,const al::EfxDistortionProperties &props) {effect->SetProperties(props);}
void Lua::ALEffect::SetProperties(lua_State *l,al::PEffect &effect,const al::EfxEchoProperties &props) {effect->SetProperties(props);}
void Lua::ALEffect::SetProperties(lua_State *l,al::PEffect &effect,const al::EfxFlangerProperties &props) {effect->SetProperties(props);}
void Lua::ALEffect::SetProperties(lua_State *l,al::PEffect &effect,const al::EfxFrequencyShifterProperties &props) {effect->SetProperties(props);}
void Lua::ALEffect::SetProperties(lua_State *l,al::PEffect &effect,const al::EfxVocalMorpherProperties &props) {effect->SetProperties(props);}
void Lua::ALEffect::SetProperties(lua_State *l,al::PEffect &effect,const al::EfxPitchShifterProperties &props) {effect->SetProperties(props);}
void Lua::ALEffect::SetProperties(lua_State *l,al::PEffect &effect,const al::EfxRingModulatorProperties &props) {effect->SetProperties(props);}
void Lua::ALEffect::SetProperties(lua_State *l,al::PEffect &effect,const al::EfxAutoWahProperties &props) {effect->SetProperties(props);}
void Lua::ALEffect::SetProperties(lua_State *l,al::PEffect &effect,const al::EfxCompressor &props) {effect->SetProperties(props);}
void Lua::ALEffect::SetProperties(lua_State *l,al::PEffect &effect,const al::EfxEqualizer &props) {effect->SetProperties(props);}
