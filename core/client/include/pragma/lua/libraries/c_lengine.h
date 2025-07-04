// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_LENGINE_H__
#define __C_LENGINE_H__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>
#include <pragma/util/font_set.hpp>
#include <image/prosper_texture.hpp>

class FontInfo;
class LFile;
enum class TextureLoadFlags : uint32_t;
namespace util {
	enum class AssetLoadFlags : uint32_t;
};
namespace Lua {
	namespace asset_client {
		DLLCLIENT void register_library(Lua::Interface &lua, luabind::module_ &modAsset);
		DLLCLIENT Material *get_error_material();
		DLLCLIENT std::shared_ptr<Material> create_material(const std::string &identifier, const std::string &shader);
		DLLCLIENT std::shared_ptr<Material> create_material(const std::string &shader);
		DLLCLIENT std::shared_ptr<Material> create_material(const ::udm::AssetData &data);
		DLLCLIENT Material *get_material(const std::string &identifier);
	};
	namespace engine {
		DLLCLIENT void precache_material(lua_State *l, const std::string &mat);
		DLLCLIENT void precache_model(lua_State *l, const std::string &mdl);
		DLLCLIENT Material *load_material(lua_State *l, const std::string &mat, bool reload, bool loadInstantly);
		DLLCLIENT Material *load_material(lua_State *l, const std::string &mat, bool reload);
		DLLCLIENT Material *load_material(lua_State *l, const std::string &mat);
		DLLCLIENT std::shared_ptr<prosper::Texture> load_texture(lua_State *l, const std::string &name, ::util::AssetLoadFlags loadFlags);
		DLLCLIENT std::shared_ptr<prosper::Texture> load_texture(lua_State *l, const std::string &name);
		DLLCLIENT std::shared_ptr<prosper::Texture> load_texture(lua_State *l, const LFile &file, const std::string &cacheName, ::util::AssetLoadFlags loadFlags);
		DLLCLIENT std::shared_ptr<prosper::Texture> load_texture(lua_State *l, const LFile &file, const std::string &cacheName);
		DLLCLIENT std::shared_ptr<prosper::Texture> load_texture(lua_State *l, const LFile &file, ::util::AssetLoadFlags loadFlags);
		DLLCLIENT std::shared_ptr<prosper::Texture> load_texture(lua_State *l, const LFile &file);
		DLLCLIENT int create_particle_system(lua_State *l);
		DLLCLIENT bool precache_particle_system(lua_State *l, const std::string &particle, bool reload);
		DLLCLIENT bool precache_particle_system(lua_State *l, const std::string &particle);
		DLLCLIENT int save_particle_system(lua_State *l);
		DLLCLIENT std::shared_ptr<const FontInfo> create_font(lua_State *l, const std::string &identifier, const std::string &font, FontSetFlag features, uint32_t size, bool reload);
		DLLCLIENT std::shared_ptr<const FontInfo> create_font(lua_State *l, const std::string &identifier, const std::string &font, FontSetFlag features, uint32_t size);
		DLLCLIENT std::shared_ptr<const FontInfo> get_font(lua_State *l, const std::string &identifier);
		DLLCLIENT Vector2i get_text_size(lua_State *l, const std::string &text, const std::string &font);
		DLLCLIENT Vector2i get_text_size(lua_State *l, const std::string &text, const FontInfo &font);
		DLLCLIENT std::pair<size_t, size_t> get_truncated_text_length(lua_State *l, const std::string &text, const std::string &font, uint32_t maxWidth);
		DLLCLIENT std::pair<size_t, size_t> get_truncated_text_length(lua_State *l, const std::string &text, const FontInfo &font, uint32_t maxWidth);
		DLLCLIENT std::shared_ptr<prosper::RenderTarget> get_staging_render_target();
		DLLCLIENT void set_fixed_frame_delta_time_interpretation(uint16_t fps);
		DLLCLIENT void clear_fixed_frame_delta_time_interpretation();
		DLLCLIENT void set_tick_delta_time_tied_to_frame_rate(bool tieToFrameRate);
		DLLCLIENT Vector2i get_window_resolution();
		DLLCLIENT Vector2i get_render_resolution();
		DLLCLIENT uint32_t get_current_frame_index();

		DLLCLIENT void register_library(lua_State *l);
	};
};

#endif
