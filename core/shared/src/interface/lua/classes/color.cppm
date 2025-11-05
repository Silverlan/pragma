// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.classes.color;

export import pragma.lua;
export import pragma.math;

export namespace Lua {
	namespace Color {
		DLLNETWORK void Copy(lua::State *l, ::Color &col);
		DLLNETWORK void Set(lua::State *l, ::Color &col, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
		DLLNETWORK void Lerp(lua::State *l, ::Color &col, ::Color &other, float factor);
		DLLNETWORK void ToVector4(lua::State *l, ::Color &col);
		DLLNETWORK void ToVector(lua::State *l, ::Color &col);
	};
};
