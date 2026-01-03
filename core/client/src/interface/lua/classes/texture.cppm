// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:scripting.lua.classes.texture;

export import pragma.cmaterialsystem;
export import pragma.lua;
export import pragma.prosper;

export namespace Lua {
	namespace Texture {
		DLLCLIENT std::shared_ptr<prosper::Texture> GetVkTexture(lua::State *l, pragma::material::Texture &tex);
	};
};
