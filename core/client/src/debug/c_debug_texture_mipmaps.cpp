/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientdefinitions.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/console/c_cvar_global_functions.h"
#include "pragma/debug/c_debug_game_gui.h"
#include "pragma/gui/widebugmipmaps.h"
#include <texture_type.h>
#include <pragma/game/game_resources.hpp>
#include <sharedutils/magic_enum.hpp>
#include <wgui/wgui.h>
#include <wgui/fontmanager.h>
#include <cmaterialmanager.h>
#include <cmaterial_manager2.hpp>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
void Console::commands::debug_font(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.empty()) {
		Con::cout << "Available fonts:" << Con::endl;
		auto &fonts = FontManager::GetFonts();
		for(auto &pair : fonts)
			Con::cout << pair.first << Con::endl;
		Con::cout << Con::endl;
		return;
	}
	auto &fontName = argv.front();
	auto font = FontManager::GetFont(fontName);
	if(font == nullptr) {
		Con::cout << "No font by that name found!" << Con::endl;
		return;
	}
	auto glyphMap = font->GetGlyphMap();
	if(glyphMap == nullptr) {
		Con::cout << "Font has invalid glyph map!" << Con::endl;
		return;
	}
	uint32_t width = glyphMap->GetImage().GetWidth();
	uint32_t height = glyphMap->GetImage().GetHeight();
	std::cout << "Glyph map size: " << width << "x" << height << std::endl;
	if(argv.size() > 1) {
		width = util::to_int(argv[1]);
		if(argv.size() > 2)
			height = util::to_int(argv[2]);
	}
	if(width == 0 || height == 0) {
		Con::cout << "Invalid resolution: " << width << "x" << height << Con::endl;
		return;
	}
	static std::unique_ptr<DebugGameGUI> dbg = nullptr;
	if(dbg == nullptr) {
		dbg = std::make_unique<DebugGameGUI>([glyphMap, width, height]() {
			auto &wgui = WGUI::GetInstance();
			auto *r = wgui.Create<WIDebugMipMaps>();
			r->SetSize(width, height);
			r->SetTexture(glyphMap);
			r->Update();
			return r->GetHandle();
		});
		return;
	}
	dbg = nullptr;
}

void Console::commands::debug_texture_mipmaps(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.empty()) {
		Con::cwar << "No texture given!" << Con::endl;
		return;
	}
	auto &texPath = argv.front();
	auto &materialManager = static_cast<msys::CMaterialManager &>(client->GetMaterialManager());
	auto &textureManager = materialManager.GetTextureManager();
	auto a = textureManager.FindCachedAsset(texPath);
	auto asset = a ? msys::TextureManager::GetAssetObject(*a) : nullptr;
	std::shared_ptr<Texture> texture = nullptr;
	if(asset && asset->HasValidVkTexture())
		texture = asset;
	else {
		auto *asset = materialManager.FindCachedAsset(texPath);
		auto mat = asset ? msys::CMaterialManager::GetAssetObject(*asset) : nullptr;
		if(!mat) {
			Con::cwar << "No material with name '" << texPath << "' found or loaded!" << Con::endl;
			return;
		}
		auto *diffuseMap = mat->GetDiffuseMap();
		if(diffuseMap == nullptr || diffuseMap->texture == nullptr || static_cast<Texture *>(diffuseMap->texture.get())->HasValidVkTexture() == false) {
			Con::cwar << "Material '" << texPath << "' has no valid albedo map!" << Con::endl;
			return;
		}
		texture = std::static_pointer_cast<Texture>(diffuseMap->texture);
	}
	static std::unique_ptr<DebugGameGUI> dbg = nullptr;
	if(dbg == nullptr) {
		auto &vkTexture = texture->GetVkTexture();
		auto numMipmaps = vkTexture->GetImage().GetMipmapCount();
		Con::cout << "Mipmap count for '" << texPath << "': " << numMipmaps << Con::endl;
		auto &path = texture->GetFilePath();
		if(path.has_value())
			Con::cout << "File path: " << *path << Con::endl;
		auto type = texture->GetFileFormatType();
		if(type != TextureType::Invalid)
			Con::cout << "File image type: " << magic_enum::enum_name(type) << Con::endl;
		dbg = std::make_unique<DebugGameGUI>([vkTexture]() {
			auto &wgui = WGUI::GetInstance();
			auto *r = wgui.Create<WIDebugMipMaps>();
			r->SetTexture(vkTexture);
			r->Update();
			return r->GetHandle();
		});
		return;
	}
	dbg = nullptr;
}
