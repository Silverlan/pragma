// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.classes.color;

void Lua::Color::Copy(lua::State *l, ::Color &col) { luabind::object(l, ::Color(col.r, col.g, col.b, col.a)).push(l); }
void Lua::Color::Set(lua::State *, ::Color &col, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	col.r = r;
	col.g = g;
	col.b = b;
	col.a = a;
}
void Lua::Color::Lerp(lua::State *l, ::Color &col, ::Color &other, float factor)
{
	auto c = col.Lerp(other, factor);
	Lua::Push<::Color>(l, c);
}
void Lua::Color::ToVector4(lua::State *l, ::Color &col)
{
	::Vector4 v {col.r / 255.f, col.g / 255.f, col.b / 255.f, col.a / 255.f};
	Lua::Push<::Vector4>(l, v);
}
void Lua::Color::ToVector(lua::State *l, ::Color &col)
{
	Vector3 v {col.r / 255.f, col.g / 255.f, col.b / 255.f};
	Lua::Push<Vector3>(l, v);
}
