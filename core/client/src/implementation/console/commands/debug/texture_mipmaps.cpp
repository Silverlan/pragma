// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"

module pragma.client;

import :console.commands;

static void debug_font(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.empty()) {
		Con::COUT << "Available fonts:" << Con::endl;
		auto &fonts = pragma::gui::FontManager::GetFonts();
		for(auto &pair : fonts)
			Con::COUT << pair.first << Con::endl;
		Con::COUT << Con::endl;
		return;
	}
	auto &fontName = argv.front();
	auto font = pragma::gui::FontManager::GetFont(fontName);
	if(font == nullptr) {
		Con::COUT << "No font by that name found!" << Con::endl;
		return;
	}
	auto glyphMap = font->GetGlyphMap();
	if(glyphMap == nullptr) {
		Con::COUT << "Font has invalid glyph map!" << Con::endl;
		return;
	}
	uint32_t width = glyphMap->GetImage().GetWidth();
	uint32_t height = glyphMap->GetImage().GetHeight();
	std::cout << "Glyph map size: " << width << "x" << height << std::endl;
	if(argv.size() > 1) {
		width = pragma::util::to_int(argv[1]);
		if(argv.size() > 2)
			height = pragma::util::to_int(argv[2]);
	}
	if(width == 0 || height == 0) {
		Con::COUT << "Invalid resolution: " << width << "x" << height << Con::endl;
		return;
	}
	static std::unique_ptr<DebugGameGUI> dbg = nullptr;
	if(dbg == nullptr) {
		dbg = std::make_unique<DebugGameGUI>([glyphMap, width, height]() {
			auto &wgui = pragma::gui::WGUI::GetInstance();
			auto *r = wgui.Create<pragma::gui::types::WIDebugMipMaps>();
			r->SetSize(width, height);
			r->SetTexture(glyphMap);
			r->Update();
			return r->GetHandle();
		});
		return;
	}
	dbg = nullptr;
}
namespace {
	auto UVN = pragma::console::client::register_command("debug_font", &debug_font, pragma::console::ConVarFlags::None, "Displays the glyph map for the specified font. If no arguments are specified, all available fonts will be listed. Usage: debug_font <fontName>");
}

static void debug_texture_mipmaps(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.empty()) {
		Con::CWAR << "No texture given!" << Con::endl;
		return;
	}
	auto &texPath = argv.front();
	auto &materialManager = static_cast<pragma::material::CMaterialManager &>(pragma::get_client_state()->GetMaterialManager());
	auto &textureManager = materialManager.GetTextureManager();
	auto a = textureManager.FindCachedAsset(texPath);
	auto asset = a ? pragma::material::TextureManager::GetAssetObject(*a) : nullptr;
	std::shared_ptr<pragma::material::Texture> texture = nullptr;
	if(asset && asset->HasValidVkTexture())
		texture = asset;
	else {
		auto *asset = materialManager.FindCachedAsset(texPath);
		auto mat = asset ? pragma::material::CMaterialManager::GetAssetObject(*asset) : nullptr;
		if(!mat) {
			Con::CWAR << "No material with name '" << texPath << "' found or loaded!" << Con::endl;
			return;
		}
		auto *diffuseMap = mat->GetDiffuseMap();
		if(diffuseMap == nullptr || diffuseMap->texture == nullptr || static_cast<pragma::material::Texture *>(diffuseMap->texture.get())->HasValidVkTexture() == false) {
			Con::CWAR << "Material '" << texPath << "' has no valid albedo map!" << Con::endl;
			return;
		}
		texture = std::static_pointer_cast<pragma::material::Texture>(diffuseMap->texture);
	}
	static std::unique_ptr<DebugGameGUI> dbg = nullptr;
	if(dbg == nullptr) {
		auto &vkTexture = texture->GetVkTexture();
		auto numMipmaps = vkTexture->GetImage().GetMipmapCount();
		Con::COUT << "Mipmap count for '" << texPath << "': " << numMipmaps << Con::endl;
		auto &path = texture->GetFilePath();
		if(path.has_value())
			Con::COUT << "File path: " << *path << Con::endl;
		auto type = texture->GetFileFormatType();
		if(type != pragma::material::TextureType::Invalid)
			Con::COUT << "File image type: " << magic_enum::enum_name(type) << Con::endl;
		dbg = std::make_unique<DebugGameGUI>([vkTexture]() {
			auto &wgui = pragma::gui::WGUI::GetInstance();
			auto *r = wgui.Create<pragma::gui::types::WIDebugMipMaps>();
			r->SetTexture(vkTexture);
			r->Update();
			return r->GetHandle();
		});
		return;
	}
	dbg = nullptr;
}
namespace {
	auto UVN = pragma::console::client::register_command("debug_texture_mipmaps", &debug_texture_mipmaps, pragma::console::ConVarFlags::None, "Displays the mipmaps of the given texture. Usage: debug_texture_mipmaps <texturePath>");
}
