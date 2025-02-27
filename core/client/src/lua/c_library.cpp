/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/util/c_util.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/lua/libraries/c_ldebugoverlay.h"
#include "pragma/lua/libraries/c_lgui.h"
#include "pragma/lua/libraries/c_lsound.h"
#include "pragma/lua/classes/c_lshader.h"
#include "pragma/lua/libraries/c_lutil.h"
#include "pragma/lua/libraries/c_linput.h"
#include "pragma/lua/libraries/lasset.hpp"
#include "pragma/lua/converters/gui_element_converter_t.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include "pragma/entities/components/lightmap_data_cache.hpp"
#include "pragma/input/input_binding_layer.hpp"
#include "pragma/audio/c_laleffect.h"
#include "pragma/audio/c_lalsound.hpp"
#include "pragma/gui/wiluabase.h"
#include "pragma/gui/wiconsole.hpp"
#include "pragma/lua/classes/c_lwibase.h"
#include "pragma/lua/classes/c_ldef_wgui.h"
#include "pragma/lua/libraries/c_limport.hpp"
#include "pragma/lua/policies/gui_element_policy.hpp"
#include "pragma/ai/c_lai.hpp"
#include "pragma/rendering/raytracing/cycles.hpp"
#include "pragma/rendering/shaders/c_shader_cubemap_to_equirectangular.hpp"
#include "pragma/rendering/shaders/c_shader_equirectangular_to_cubemap.hpp"
#include "pragma/asset/c_util_model.hpp"
#include <pragma/debug/debug_render_info.hpp>
#include <pragma/game/game_resources.hpp>
#include <pragma/util/giblet_create_info.hpp>
#include <pragma/lua/lua_entity_component.hpp>
#include <pragma/lua/classes/ldef_entity.h>
#include <pragma/lua/classes/thread_pool.hpp>
#include <pragma/lua/libraries/lfile.h>
#include <pragma/lua/libraries/lutil.hpp>
#include <pragma/lua/policies/default_parameter_policy.hpp>
#include <pragma/lua/policies/vector_policy.hpp>
#include <pragma/lua/policies/optional_policy.hpp>
#include <pragma/lua/converters/string_view_converter_t.hpp>
#include <pragma/lua/converters/vector_converter_t.hpp>
#include <pragma/lua/converters/pair_converter_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/lua/converters/optional_converter_t.hpp>
#include <pragma/lua/converters/thread_pool_converter_t.hpp>
#include <pragma/lua/policies/core_policies.hpp>
#include <pragma/lua/custom_constructor.hpp>
#include <pragma/lua/util_logging.hpp>
#include <pragma/model/modelmanager.h>
#include <pragma/input/inputhelper.h>
#include <sharedutils/util_file.h>
#include <sharedutils/util_path.hpp>
#include <util_image.hpp>
#include <util_image_buffer.hpp>
#include <util_texture_info.hpp>
#include <alsoundsystem.hpp>
#include <luainterface.hpp>
#include <cmaterialmanager.h>
#include <cmaterial_manager2.hpp>
#include <impl_texture_formats.h>
#include <prosper_window.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_prepared_command_buffer.hpp>
#include <luabind/copy_policy.hpp>
#include <fsys/ifile.hpp>
#include <wgui/types/witooltip.h>
#include <wgui/types/wiroot.h>
#include <wgui/types/wicontentwrapper.hpp>

import pragma.string.unicode;
import pragma.audio.util;
import pragma.platform;

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CEngine *c_engine;

static std::optional<std::string> find_asset_file(const std::string &name, pragma::asset::Type type)
{
	if(type == pragma::asset::Type::Texture) {
		TextureType type;
		auto found = false;
		auto filePath = translate_image_path(name, type, nullptr, &found);
		if(found == false)
			return {};
		::util::Path path {filePath};
		path.PopFront();
		return path.GetString();
	}
	return find_file(name, type);
}
static bool is_asset_loaded(NetworkState &nw, const std::string &name, pragma::asset::Type type)
{
	switch(type) {
	case pragma::asset::Type::Texture:
		{
			auto *asset = static_cast<msys::CMaterialManager &>(nw.GetMaterialManager()).GetTextureManager().FindCachedAsset(name);
			return asset != nullptr;
		}
	case pragma::asset::Type::ParticleSystem:
		{
			return pragma::CParticleSystemComponent::IsParticleFilePrecached(name);
		}
	}
	return is_loaded(nw, name, type);
}

static std::optional<uint32_t> save_image(lua_State *l, uimg::ImageBuffer &imgBuffer, std::string fileName, uimg::TextureInfo &imgWriteInfo, bool cubemap, const pragma::lua::LuaThreadWrapper &tw)
{
	if(Lua::file::validate_write_operation(l, fileName) == false)
		return {};
	auto pImgBuffer = imgBuffer.shared_from_this();
	auto task = [pImgBuffer, fileName = std::move(fileName), imgWriteInfo, cubemap]() -> pragma::lua::LuaThreadPool::ResultHandler {
		auto result = c_game->SaveImage(*pImgBuffer, fileName, imgWriteInfo, cubemap);
		return [result](lua_State *l) { luabind::object {l, result}.push(l); };
	};
	if(tw.IsPool())
		return tw.GetPool().AddTask(task);
	tw.GetTask()->AddSubTask(task);
	return {};
}

static bool save_image(lua_State *l, uimg::ImageBuffer &imgBuffer, std::string fileName, uimg::TextureInfo &imgWriteInfo, bool cubemap)
{
	if(Lua::file::validate_write_operation(l, fileName) == false)
		return false;
	return c_game->SaveImage(imgBuffer, fileName, imgWriteInfo, cubemap);
}

static std::vector<std::string> &get_image_file_extensions()
{
	static std::vector<std::string> exts;
	if(exts.empty()) {
		exts.reserve(umath::to_integral(uimg::ImageFormat::Count));
		auto n = umath::to_integral(uimg::ImageFormat::Count);
		for(auto i = decltype(n) {0u}; i < n; ++i)
			exts.push_back(uimg::get_file_extension(static_cast<uimg::ImageFormat>(i)));
	}
	return exts;
}

static std::pair<bool, std::optional<std::string>> save_image(lua_State *l, uimg::ImageBuffer &imgBuffer, std::string fileName, uimg::ImageFormat format, float quality, pragma::lua::LuaThreadWrapper *tw = nullptr)
{
	if(Lua::file::validate_write_operation(l, fileName) == false)
		return std::pair<bool, std::optional<std::string>> {false, {}};
	ufile::remove_extension_from_filename(fileName, get_image_file_extensions());
	fileName += '.' + uimg::get_file_extension(format);
	auto fp = filemanager::open_file<VFilePtrReal>(fileName, filemanager::FileMode::Write | filemanager::FileMode::Binary);
	if(!fp)
		return std::pair<bool, std::optional<std::string>> {false, {}};
	if(tw) {
		auto pImgBuffer = imgBuffer.shared_from_this();
		auto task = [fp, pImgBuffer, format, quality]() -> pragma::lua::LuaThreadPool::ResultHandler {
			fsys::File f {fp};
			auto result = uimg::save_image(f, *pImgBuffer, format, quality);
			return [result](lua_State *l) { luabind::object {l, result}.push(l); };
		};
		if(tw->IsPool())
			tw->GetPool().AddTask(task);
		else
			tw->GetTask()->AddSubTask(task);
		return std::pair<bool, std::optional<std::string>> {true, fileName};
	}
	fsys::File f {fp};
	auto result = uimg::save_image(f, imgBuffer, format, quality);
	return std::pair<bool, std::optional<std::string>> {result, fileName};
}
static std::pair<bool, std::optional<std::string>> save_image(lua_State *l, uimg::ImageBuffer &imgBuffer, std::string fileName, uimg::ImageFormat format) { return save_image(l, imgBuffer, fileName, format, 1.f); }
static bool save_image(lua_State *l, luabind::table<> t, std::string fileName, uimg::TextureInfo &texInfo, bool cubemap)
{
	auto n = Lua::GetObjectLength(l, t);
	std::vector<std::shared_ptr<uimg::ImageBuffer>> imgBufs;
	imgBufs.reserve(n);
	auto o = luabind::object {luabind::from_stack(l, 1)};
	uint32_t maxWidth = 0;
	uint32_t maxHeight = 0;
	for(luabind::iterator it {o}, end; it != end; ++it) {
		auto val = *it;
		auto *imgBuf = luabind::object_cast<uimg::ImageBuffer *>(val);
		imgBufs.push_back(imgBuf->shared_from_this());
		maxWidth = umath::max(maxWidth, imgBuf->GetWidth());
		maxHeight = umath::max(maxHeight, imgBuf->GetHeight());
	}
	for(auto &imgBuf : imgBufs)
		imgBuf->Resize(maxWidth, maxHeight);
	if(imgBufs.empty())
		return false;
	auto &imgBuf = imgBufs.front();
	uimg::TextureSaveInfo saveInfo {};
	saveInfo.texInfo = texInfo;
	saveInfo.width = imgBuf->GetWidth();
	saveInfo.height = imgBuf->GetHeight();
	saveInfo.szPerPixel = imgBuf->GetPixelSize();
	saveInfo.numLayers = imgBufs.size();
	saveInfo.numMipmaps = 0;
	saveInfo.cubemap = cubemap;
	return uimg::save_texture(
	  fileName,
	  [&imgBufs](uint32_t iLayer, uint32_t iMipmap, std::function<void(void)> &outDeleter) -> const uint8_t * {
		  if(iMipmap > 0)
			  return nullptr;
		  return static_cast<uint8_t *>(imgBufs.at(iLayer)->GetData());
	  },
	  saveInfo);
}
static bool save_image(lua_State *l, luabind::table<> t, std::string fileName, uimg::TextureInfo &texInfo) { return save_image(l, t, fileName, texInfo, false); }

static bool save_image(lua_State *l, prosper::IImage &img, std::string fileName, uimg::TextureInfo &imgWriteInfo)
{
	if(Lua::file::validate_write_operation(l, fileName) == false)
		return false;
	return c_game->SaveImage(img, fileName, imgWriteInfo);
}

static luabind::object load_image(lua_State *l, const std::string &fileName, bool loadAsynch, const std::optional<uimg::Format> &targetFormat)
{
	std::string ext;
	if(ufile::get_extension(fileName, &ext) == false)
		return {};
	auto fp = FileManager::OpenFile<VFilePtrReal>(fileName.c_str(), "rb");
	if(fp == nullptr)
		return {};
	auto pixelFormat = uimg::PixelFormat::LDR;
	if(ustring::compare<std::string>(ext, "hdr"))
		pixelFormat = uimg::PixelFormat::Float;

	if(loadAsynch) {
		class ImageLoadJob : public util::ParallelWorker<std::shared_ptr<uimg::ImageBuffer>> {
		  public:
			ImageLoadJob(VFilePtr fp, uimg::PixelFormat pixelFormat, std::optional<uimg::Format> targetFormat)
			{
				AddThread([this, fp, pixelFormat, targetFormat]() {
					fsys::File f {fp};
					m_imgBuffer = uimg::load_image(f, pixelFormat);
					if(m_imgBuffer == nullptr) {
						SetStatus(util::JobStatus::Failed, "Unable to open image!");
						UpdateProgress(1.f);
						return;
					}
					if(targetFormat.has_value()) {
						if(IsCancelled())
							return;
						UpdateProgress(0.9f);
						m_imgBuffer->Convert(*targetFormat);
					}
					UpdateProgress(1.f);
				});
			}

			virtual std::shared_ptr<uimg::ImageBuffer> GetResult() override { return m_imgBuffer; }
		  private:
			std::shared_ptr<uimg::ImageBuffer> m_imgBuffer = nullptr;
		};
		return {l, util::create_parallel_job<ImageLoadJob>(fp, pixelFormat, targetFormat)};
	}
	fsys::File f {fp};
	auto imgBuffer = uimg::load_image(f, pixelFormat);
	if(imgBuffer == nullptr)
		return {};
	if(targetFormat.has_value())
		imgBuffer->Convert(*targetFormat);
	return {l, imgBuffer};
}

static luabind::object load_image(lua_State *l, const std::string &fileName, bool loadAsynch, uimg::Format targetFormat) { return load_image(l, fileName, loadAsynch, std::optional<uimg::Format> {targetFormat}); }

static luabind::object load_image(lua_State *l, const std::string &fileName, bool loadAsynch) { return load_image(l, fileName, loadAsynch, std::optional<uimg::Format> {}); }

static luabind::object load_image(lua_State *l, const std::string &fileName) { return load_image(l, fileName, false); }

static util::ParallelJob<uimg::ImageLayerSet> capture_raytraced_screenshot(lua_State *l, uint32_t width, uint32_t height, uint32_t samples, bool hdrOutput, bool denoise)
{
	pragma::rendering::cycles::RenderImageInfo renderImgInfo {};
	auto *pCam = c_game->GetRenderCamera();
	if(pCam) {
		renderImgInfo.camPose = pCam->GetEntity().GetPose();
		renderImgInfo.viewProjectionMatrix = pCam->GetProjectionMatrix() * pCam->GetViewMatrix();
		renderImgInfo.nearZ = pCam->GetNearZ();
		renderImgInfo.farZ = pCam->GetFarZ();
		renderImgInfo.fov = pCam->GetFOV();
	}
	pragma::rendering::cycles::SceneInfo sceneInfo {};
	sceneInfo.width = width;
	sceneInfo.height = height;
	sceneInfo.samples = samples;
	sceneInfo.denoise = denoise;
	sceneInfo.hdrOutput = hdrOutput;
	return pragma::rendering::cycles::render_image(*client, sceneInfo, renderImgInfo);
}
static util::ParallelJob<uimg::ImageLayerSet> capture_raytraced_screenshot(lua_State *l, uint32_t width, uint32_t height, uint32_t samples, bool hdrOutput) { return capture_raytraced_screenshot(l, width, height, samples, hdrOutput, true); }
static util::ParallelJob<uimg::ImageLayerSet> capture_raytraced_screenshot(lua_State *l, uint32_t width, uint32_t height, uint32_t samples) { return capture_raytraced_screenshot(l, width, height, samples, false, true); }
static util::ParallelJob<uimg::ImageLayerSet> capture_raytraced_screenshot(lua_State *l, uint32_t width, uint32_t height) { return capture_raytraced_screenshot(l, width, height, 1'024, false, true); }

static bool asset_import(NetworkState &nw, const std::string &name, const std::string &outputName, pragma::asset::Type type)
{
	if(type == pragma::asset::Type::Map)
		return util::port_hl2_map(&nw, name);
	auto *manager = nw.GetAssetManager(type);
	if(!manager)
		return false;
	return manager->Import(name, outputName);
}

void CGame::RegisterLuaLibraries()
{
	Lua::util::register_library(GetLuaState());

	auto osMod = luabind::module(GetLuaState(), "os");
	Lua::util::register_os(GetLuaState(), osMod);

	auto utilMod = luabind::module(GetLuaState(), "util");
	Lua::util::register_shared(GetLuaState(), utilMod);
	utilMod[luabind::def("calc_world_direction_from_2d_coordinates", Lua::util::calc_world_direction_from_2d_coordinates), luabind::def("calc_world_direction_from_2d_coordinates", Lua::util::Client::calc_world_direction_from_2d_coordinates),
	  luabind::def("create_particle_tracer", Lua::util::Client::create_particle_tracer), luabind::def("create_muzzle_flash", Lua::util::Client::create_muzzle_flash), luabind::def("fire_bullets", static_cast<luabind::object (*)(lua_State *, BulletInfo &)>(Lua::util::fire_bullets)),
	  luabind::def("save_image", static_cast<bool (*)(lua_State *, uimg::ImageBuffer &, std::string, uimg::TextureInfo &, bool)>(save_image)),
	  luabind::def("save_image", static_cast<bool (*)(lua_State *, uimg::ImageBuffer &, std::string, uimg::TextureInfo &, bool)>(save_image), luabind::default_parameter_policy<5, false> {}),
	  luabind::def(
	    "save_image", +[](lua_State *l, uimg::ImageBuffer &imgBuffer, std::string fileName, uimg::TextureInfo &imgWriteInfo, const pragma::lua::LuaThreadWrapper &tw) { return save_image(l, imgBuffer, fileName, imgWriteInfo, false, tw); }),
	  luabind::def(
	    "save_image", +[](lua_State *l, uimg::ImageBuffer &imgBuffer, std::string fileName, uimg::ImageFormat format, float quality) { return save_image(l, imgBuffer, fileName, format, quality); }),
	  luabind::def(
	    "save_image", +[](lua_State *l, uimg::ImageBuffer &imgBuffer, std::string fileName, uimg::ImageFormat format, float quality, const pragma::lua::LuaThreadWrapper &tw) { return save_image(l, imgBuffer, fileName, format, quality, const_cast<pragma::lua::LuaThreadWrapper *>(&tw)); }),
	  luabind::def("save_image", static_cast<std::pair<bool, std::optional<std::string>> (*)(lua_State *, uimg::ImageBuffer &, std::string, uimg::ImageFormat)>(save_image)),
	  luabind::def("save_image", static_cast<bool (*)(lua_State *, luabind::table<>, std::string, uimg::TextureInfo &, bool)>(save_image)), luabind::def("save_image", static_cast<bool (*)(lua_State *, luabind::table<>, std::string, uimg::TextureInfo &)>(save_image)),
	  luabind::def("save_image", static_cast<bool (*)(lua_State *, prosper::IImage &, std::string, uimg::TextureInfo &)>(save_image)), luabind::def("load_image", static_cast<luabind::object (*)(lua_State *, const std::string &, bool, uimg::Format)>(load_image)),
	  luabind::def("load_image", static_cast<luabind::object (*)(lua_State *, const std::string &, bool)>(load_image)), luabind::def("load_image", static_cast<luabind::object (*)(lua_State *, const std::string &)>(load_image)), luabind::def("screenshot", ::util::screenshot),
	  luabind::def("capture_raytraced_screenshot", static_cast<util::ParallelJob<uimg::ImageLayerSet> (*)(lua_State *, uint32_t, uint32_t, uint32_t, bool, bool)>(capture_raytraced_screenshot)),
	  luabind::def("capture_raytraced_screenshot", static_cast<util::ParallelJob<uimg::ImageLayerSet> (*)(lua_State *, uint32_t, uint32_t, uint32_t, bool)>(capture_raytraced_screenshot)),
	  luabind::def("capture_raytraced_screenshot", static_cast<util::ParallelJob<uimg::ImageLayerSet> (*)(lua_State *, uint32_t, uint32_t, uint32_t)>(capture_raytraced_screenshot)),
	  luabind::def("capture_raytraced_screenshot", static_cast<util::ParallelJob<uimg::ImageLayerSet> (*)(lua_State *, uint32_t, uint32_t)>(capture_raytraced_screenshot)),
	  luabind::def(
	    "cubemap_to_equirectangular_texture",
	    +[](lua_State *l, prosper::Texture &cubemap) -> luabind::object {
		    auto *shader = static_cast<pragma::ShaderCubemapToEquirectangular *>(c_engine->GetShader("cubemap_to_equirectangular").get());
		    if(shader == nullptr)
			    return {};
		    auto equiRect = shader->CubemapToEquirectangularTexture(cubemap);
		    if(equiRect == nullptr)
			    return {};
		    return {l, equiRect};
	    }),
	  luabind::def(
	    "equirectangular_to_cubemap_texture", +[](lua_State *l, prosper::Texture &equiRect, uint32_t resolution) -> luabind::object {
		    auto *shader = static_cast<pragma::ShaderEquirectangularToCubemap *>(c_engine->GetShader("equirectangular_to_cubemap").get());
		    if(shader == nullptr)
			    return {};
		    auto tex = shader->EquirectangularTextureToCubemap(equiRect, resolution);
		    if(tex == nullptr)
			    return {};
		    return {l, tex};
	    })];
	utilMod[
	  // luabind::def("fire_bullets",static_cast<int32_t(*)(lua_State*)>(Lua::util::fire_bullets)),
	  luabind::def("get_clipboard_string", Lua::util::Client::get_clipboard_string), luabind::def("set_clipboard_string", Lua::util::Client::set_clipboard_string), luabind::def("create_giblet", Lua::util::Client::create_giblet),
	  luabind::def("get_image_format_file_extension", uimg::get_file_extension), luabind::def("bake_directional_lightmap_atlas", Lua::util::Client::bake_directional_lightmap_atlas)];

	auto imgWriteInfoDef = luabind::class_<uimg::TextureInfo>("TextureInfo");
	imgWriteInfoDef.def(luabind::constructor<>());
	imgWriteInfoDef.add_static_constant("INPUT_FORMAT_KEEP_INPUT_IMAGE_FORMAT", umath::to_integral(uimg::TextureInfo::InputFormat::KeepInputImageFormat));
	imgWriteInfoDef.add_static_constant("INPUT_FORMAT_R16G16B16A16_FLOAT", umath::to_integral(uimg::TextureInfo::InputFormat::R16G16B16A16_Float));
	imgWriteInfoDef.add_static_constant("INPUT_FORMAT_R32G32B32A32_FLOAT", umath::to_integral(uimg::TextureInfo::InputFormat::R32G32B32A32_Float));
	imgWriteInfoDef.add_static_constant("INPUT_FORMAT_R32_FLOAT", umath::to_integral(uimg::TextureInfo::InputFormat::R32_Float));
	imgWriteInfoDef.add_static_constant("INPUT_FORMAT_R8G8B8A8_UINT", umath::to_integral(uimg::TextureInfo::InputFormat::R8G8B8A8_UInt));

	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_KEEP_INPUT_IMAGE_FORMAT", umath::to_integral(uimg::TextureInfo::OutputFormat::KeepInputImageFormat));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_RGB", umath::to_integral(uimg::TextureInfo::OutputFormat::RGB));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_RGBA", umath::to_integral(uimg::TextureInfo::OutputFormat::RGBA));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_DXT1", umath::to_integral(uimg::TextureInfo::OutputFormat::DXT1));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_DXT1A", umath::to_integral(uimg::TextureInfo::OutputFormat::DXT1a));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_DXT3", umath::to_integral(uimg::TextureInfo::OutputFormat::DXT3));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_DXT5", umath::to_integral(uimg::TextureInfo::OutputFormat::DXT5));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_DXT5N", umath::to_integral(uimg::TextureInfo::OutputFormat::DXT5n));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC1", umath::to_integral(uimg::TextureInfo::OutputFormat::BC1));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC1A", umath::to_integral(uimg::TextureInfo::OutputFormat::BC1a));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC2", umath::to_integral(uimg::TextureInfo::OutputFormat::BC2));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC3", umath::to_integral(uimg::TextureInfo::OutputFormat::BC3));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC3N", umath::to_integral(uimg::TextureInfo::OutputFormat::BC3n));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC4", umath::to_integral(uimg::TextureInfo::OutputFormat::BC4));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC5", umath::to_integral(uimg::TextureInfo::OutputFormat::BC5));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_DXT1N", umath::to_integral(uimg::TextureInfo::OutputFormat::DXT1n));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_CTX1", umath::to_integral(uimg::TextureInfo::OutputFormat::CTX1));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC6", umath::to_integral(uimg::TextureInfo::OutputFormat::BC6));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC7", umath::to_integral(uimg::TextureInfo::OutputFormat::BC7));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC3_RGBM", umath::to_integral(uimg::TextureInfo::OutputFormat::BC3_RGBM));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_ETC1", umath::to_integral(uimg::TextureInfo::OutputFormat::ETC1));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_ETC2_R", umath::to_integral(uimg::TextureInfo::OutputFormat::ETC2_R));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_ETC2_RG", umath::to_integral(uimg::TextureInfo::OutputFormat::ETC2_RG));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_ETC2_RGB", umath::to_integral(uimg::TextureInfo::OutputFormat::ETC2_RGB));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_ETC2_RGBA", umath::to_integral(uimg::TextureInfo::OutputFormat::ETC2_RGBA));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_ETC2_RGB_A1", umath::to_integral(uimg::TextureInfo::OutputFormat::ETC2_RGB_A1));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_ETC2_RGBM", umath::to_integral(uimg::TextureInfo::OutputFormat::ETC2_RGBM));

	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_COLOR_MAP", umath::to_integral(uimg::TextureInfo::OutputFormat::ColorMap));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_COLOR_MAP_1BIT_ALPHA", umath::to_integral(uimg::TextureInfo::OutputFormat::ColorMap1BitAlpha));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_COLOR_MAP_SHARP_ALPHA", umath::to_integral(uimg::TextureInfo::OutputFormat::ColorMapSharpAlpha));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_COLOR_MAP_SMOOTH_ALPHA", umath::to_integral(uimg::TextureInfo::OutputFormat::ColorMapSmoothAlpha));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_NORMAL_MAP", umath::to_integral(uimg::TextureInfo::OutputFormat::NormalMap));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_HDR_COLOR_MAP", umath::to_integral(uimg::TextureInfo::OutputFormat::HDRColorMap));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_GRADIENT_MAP", umath::to_integral(uimg::TextureInfo::OutputFormat::GradientMap));

	imgWriteInfoDef.add_static_constant("CONTAINER_FORMAT_DDS", umath::to_integral(uimg::TextureInfo::ContainerFormat::DDS));
	imgWriteInfoDef.add_static_constant("CONTAINER_FORMAT_KTX", umath::to_integral(uimg::TextureInfo::ContainerFormat::KTX));

	imgWriteInfoDef.add_static_constant("FLAG_NONE", umath::to_integral(uimg::TextureInfo::Flags::None));
	imgWriteInfoDef.add_static_constant("FLAG_BIT_CONVERT_TO_NORMAL_MAP", umath::to_integral(uimg::TextureInfo::Flags::ConvertToNormalMap));
	imgWriteInfoDef.add_static_constant("FLAG_BIT_SRGB", umath::to_integral(uimg::TextureInfo::Flags::SRGB));
	imgWriteInfoDef.add_static_constant("FLAG_BIT_GENERATE_MIPMAPS", umath::to_integral(uimg::TextureInfo::Flags::GenerateMipmaps));

	imgWriteInfoDef.add_static_constant("MIPMAP_FILTER_BOX", umath::to_integral(uimg::TextureInfo::MipmapFilter::Box));
	imgWriteInfoDef.add_static_constant("MIPMAP_FILTER_KAISER", umath::to_integral(uimg::TextureInfo::MipmapFilter::Kaiser));

	imgWriteInfoDef.add_static_constant("WRAP_MODE_CLAMP", umath::to_integral(uimg::TextureInfo::WrapMode::Clamp));
	imgWriteInfoDef.add_static_constant("WRAP_MODE_REPEAT", umath::to_integral(uimg::TextureInfo::WrapMode::Repeat));
	imgWriteInfoDef.add_static_constant("WRAP_MODE_MIRROR", umath::to_integral(uimg::TextureInfo::WrapMode::Mirror));

	imgWriteInfoDef.def_readwrite("inputFormat", reinterpret_cast<std::underlying_type_t<decltype(uimg::TextureInfo::inputFormat)> uimg::TextureInfo::*>(&uimg::TextureInfo::inputFormat));
	imgWriteInfoDef.def_readwrite("outputFormat", reinterpret_cast<std::underlying_type_t<decltype(uimg::TextureInfo::outputFormat)> uimg::TextureInfo::*>(&uimg::TextureInfo::outputFormat));
	imgWriteInfoDef.def_readwrite("containerFormat", reinterpret_cast<std::underlying_type_t<decltype(uimg::TextureInfo::containerFormat)> uimg::TextureInfo::*>(&uimg::TextureInfo::containerFormat));
	imgWriteInfoDef.def_readwrite("flags", reinterpret_cast<std::underlying_type_t<decltype(uimg::TextureInfo::flags)> uimg::TextureInfo::*>(&uimg::TextureInfo::flags));
	imgWriteInfoDef.def_readwrite("mipMapFilter", reinterpret_cast<std::underlying_type_t<decltype(uimg::TextureInfo::mipMapFilter)> uimg::TextureInfo::*>(&uimg::TextureInfo::mipMapFilter));
	imgWriteInfoDef.def_readwrite("wrapMode", reinterpret_cast<std::underlying_type_t<decltype(uimg::TextureInfo::wrapMode)> uimg::TextureInfo::*>(&uimg::TextureInfo::wrapMode));

	imgWriteInfoDef.def("SetNormalMap", static_cast<void (*)(lua_State *, uimg::TextureInfo &)>([](lua_State *l, uimg::TextureInfo &writeInfo) { writeInfo.SetNormalMap(); }));
	utilMod[imgWriteInfoDef];

	Lua::RegisterLibraryEnums(GetLuaState(), "util",
	  {{"IMAGE_FORMAT_PNG", umath::to_integral(uimg::ImageFormat::PNG)}, {"IMAGE_FORMAT_BMP", umath::to_integral(uimg::ImageFormat::BMP)}, {"IMAGE_FORMAT_TGA", umath::to_integral(uimg::ImageFormat::TGA)}, {"IMAGE_FORMAT_JPG", umath::to_integral(uimg::ImageFormat::JPG)},
	    {"IMAGE_FORMAT_HDR", umath::to_integral(uimg::ImageFormat::HDR)}, {"IMAGE_FORMAT_COUNT", umath::to_integral(uimg::ImageFormat::Count)},

	    {"PIXEL_FORMAT_LDR", umath::to_integral(uimg::PixelFormat::LDR)}, {"PIXEL_FORMAT_HDR", umath::to_integral(uimg::PixelFormat::HDR)}, {"PIXEL_FORMAT_FLOAT", umath::to_integral(uimg::PixelFormat::Float)}});

	Lua::ai::client::register_library(GetLuaInterface());

	Game::RegisterLuaLibraries();
	ClientState::RegisterSharedLuaLibraries(GetLuaInterface());

	auto consoleMod = luabind::module(GetLuaState(), "console");
	consoleMod[luabind::def("save_config", +[](CEngine &engine) { engine.SaveClientConfig(); })];

	GetLuaInterface().RegisterLibrary("asset",
	  {{"export_map", Lua::util::Client::export_map}, {"import_model", Lua::util::Client::import_model}, {"import_gltf", Lua::util::Client::import_gltf}, {"export_texture", Lua::util::Client::export_texture}, {"export_material", Lua::util::Client::export_material},
	    {"export_texture_as_vtf", static_cast<int32_t (*)(lua_State *)>([](lua_State *l) -> int32_t {
		     std::string fileName = Lua::CheckString(l, 1);
		     if(Lua::file::validate_write_operation(l, fileName) == false || FileManager::CreatePath(ufile::get_path_from_filename(fileName).c_str()) == false) {
			     Lua::PushBool(l, false);
			     return 1;
		     }

		     auto &img = Lua::Check<prosper::IImage>(l, 2);
		     auto vtfOutputFormat = pragma::asset::prosper_format_to_vtf(img.GetFormat());
		     auto srgb = true;
		     auto normalMap = false;
		     auto generateMipmaps = false;
		     int32_t arg = 3;

		     if(Lua::IsSet(l, arg))
			     srgb = Lua::CheckBool(l, arg);
		     ++arg;

		     if(Lua::IsSet(l, arg))
			     normalMap = Lua::CheckBool(l, arg);
		     ++arg;

		     if(Lua::IsSet(l, arg))
			     generateMipmaps = Lua::CheckBool(l, arg);
		     ++arg;

		     if(Lua::IsSet(l, arg))
			     vtfOutputFormat = pragma::asset::prosper_format_to_vtf(static_cast<prosper::Format>(Lua::CheckInt(l, arg)));
		     ++arg;
		     if(vtfOutputFormat.has_value() == false) {
			     Lua::PushBool(l, false);
			     return 1;
		     }

		     pragma::asset::VtfInfo vtfInfo {};
		     vtfInfo.outputFormat = *vtfOutputFormat;
		     umath::set_flag(vtfInfo.flags, pragma::asset::VtfInfo::Flags::Srgb, srgb);
		     umath::set_flag(vtfInfo.flags, pragma::asset::VtfInfo::Flags::NormalMap, normalMap);
		     umath::set_flag(vtfInfo.flags, pragma::asset::VtfInfo::Flags::GenerateMipmaps, generateMipmaps);
		     auto result = pragma::asset::export_texture_as_vtf(fileName, img, vtfInfo, nullptr, false);
		     Lua::PushBool(l, result);
		     return 1;
	     })},
	    {"exists", static_cast<int32_t (*)(lua_State *)>([](lua_State *l) -> int32_t {
		     std::string name = Lua::CheckString(l, 1);
		     auto type = static_cast<pragma::asset::Type>(Lua::CheckInt(l, 2));
		     auto *nw = c_engine->GetNetworkState(l);
		     auto fileName = find_asset_file(name, type);
		     Lua::PushBool(l, fileName.has_value());
		     return 1;
	     })},
	    {"find_file", static_cast<int32_t (*)(lua_State *)>([](lua_State *l) -> int32_t {
		     std::string name = Lua::CheckString(l, 1);
		     auto type = static_cast<pragma::asset::Type>(Lua::CheckInt(l, 2));
		     auto *nw = c_engine->GetNetworkState(l);
		     auto fileName = find_asset_file(name, type);
		     if(fileName.has_value() == false)
			     return 0;
		     Lua::PushString(l, *fileName);
		     return 1;
	     })},
	    {"is_loaded", static_cast<int32_t (*)(lua_State *)>([](lua_State *l) -> int32_t {
		     std::string name = Lua::CheckString(l, 1);
		     auto type = static_cast<pragma::asset::Type>(Lua::CheckInt(l, 2));
		     auto *nw = c_engine->GetNetworkState(l);
		     Lua::PushBool(l, is_asset_loaded(*nw, name, type));
		     return 1;
	     })},
	    {"import_texture", static_cast<int32_t (*)(lua_State *)>([](lua_State *l) -> int32_t {
		     auto &texImportInfo = Lua::Check<::pragma::asset::TextureImportInfo>(l, 2);
		     std::string outputPath = Lua::CheckString(l, 3);
		     auto result = false;
		     std::string errMsg;
		     if(Lua::IsString(l, 1)) {
			     std::string name = Lua::CheckString(l, 1);
			     result = pragma::asset::import_texture(name, texImportInfo, outputPath, errMsg);
		     }
		     else if(Lua::IsFile(l, 1)) {
			     auto f = Lua::CheckFile(l, 1);
			     auto fp = std::make_unique<ufile::FileWrapper>(f->GetHandle());
			     result = pragma::asset::import_texture(std::move(fp), texImportInfo, outputPath, errMsg);
		     }
		     else {
			     auto &img = Lua::Check<prosper::IImage>(l, 1);
			     result = pragma::asset::import_texture(img, texImportInfo, outputPath, errMsg);
		     }
		     Lua::PushBool(l, result);
		     if(result == false) {
			     Lua::PushString(l, errMsg);
			     return 2;
		     }
		     return 1;
	     })}});

	auto modAsset = luabind::module_(GetLuaState(), "asset");
	modAsset[luabind::def("clear_unused_textures", static_cast<uint32_t (*)()>([]() -> uint32_t { return static_cast<msys::CMaterialManager &>(client->GetMaterialManager()).GetTextureManager().ClearUnused(); })),

	  luabind::def(
	    "load",
	    +[](lua_State *l, LFile &f, pragma::asset::Type type) -> Lua::var<bool, luabind::object> {
		    // See also core/server/src/lua/library.cpp
		    auto *manager = pragma::get_engine()->GetNetworkState(l)->GetAssetManager(type);
		    if(!manager)
			    return luabind::object {l, false};
		    auto fh = f.GetHandle();
		    if(!fh)
			    return luabind::object {l, false};
		    auto fp = std::make_unique<ufile::FileWrapper>(fh);
		    auto fileName = fp->GetFileName();
		    if(!fileName.has_value())
			    return luabind::object {l, false};
		    std::string ext;
		    if(ufile::get_extension(*fileName, &ext) == false)
			    return luabind::object {l, false};
		    auto loadInfo = manager->CreateDefaultLoadInfo();
		    loadInfo->flags |= util::AssetLoadFlags::DontCache | util::AssetLoadFlags::IgnoreCache;
		    auto asset = manager->LoadAsset(ufile::get_file_from_filename(*fileName), std::move(fp), ext, std::move(loadInfo));
		    switch(type) {
		    case pragma::asset::Type::Model:
			    return luabind::object {l, std::static_pointer_cast<Model>(asset)};
		    case pragma::asset::Type::Material:
			    return luabind::object {l, std::static_pointer_cast<Material>(asset)};
		    case pragma::asset::Type::Texture:
			    return luabind::object {l, std::static_pointer_cast<Texture>(asset)};
		    }
		    return luabind::object {};
	    }),
	  luabind::def(
	    "load",
	    +[](lua_State *l, const std::string &name, pragma::asset::Type type) -> Lua::var<bool, luabind::object> {
		    // See also core/server/src/lua/library.cpp
		    auto *manager = pragma::get_engine()->GetNetworkState(l)->GetAssetManager(type);
		    if(!manager)
			    return luabind::object {l, false};
		    auto asset = manager->LoadAsset(name);
		    switch(type) {
		    case pragma::asset::Type::Model:
			    return luabind::object {l, std::static_pointer_cast<Model>(asset)};
		    case pragma::asset::Type::Material:
			    return luabind::object {l, std::static_pointer_cast<Material>(asset)};
		    case pragma::asset::Type::Texture:
			    return luabind::object {l, std::static_pointer_cast<Texture>(asset)};
		    }
		    return luabind::object {};
	    }),
	  luabind::def(
	    "reload",
	    +[](lua_State *l, const std::string &name, pragma::asset::Type type) -> Lua::var<bool, luabind::object> {
		    auto *manager = c_engine->GetNetworkState(l)->GetAssetManager(type);
		    if(!manager)
			    return luabind::object {l, false};
		    auto asset = manager->ReloadAsset(name);
		    switch(type) {
		    case pragma::asset::Type::Model:
			    return luabind::object {l, std::static_pointer_cast<Model>(asset)};
		    case pragma::asset::Type::Material:
			    return luabind::object {l, std::static_pointer_cast<Material>(asset)};
		    case pragma::asset::Type::Texture:
			    return luabind::object {l, std::static_pointer_cast<Texture>(asset)};
		    }
		    return luabind::object {};
	    }),
	  luabind::def(
	    "import", +[](NetworkState &nw, const std::string &name, pragma::asset::Type type) -> bool { return asset_import(nw, name, name, type); }),
	  luabind::def("import", +[](NetworkState &nw, const std::string &name, const std::string &outputName, pragma::asset::Type type) -> bool { return asset_import(nw, name, outputName, type); })];
	auto defMapExportInfo = luabind::class_<pragma::asset::MapExportInfo>("MapExportInfo");
	defMapExportInfo.def(luabind::constructor<>());
	defMapExportInfo.def_readwrite("includeMapLightSources", &pragma::asset::MapExportInfo::includeMapLightSources);
	defMapExportInfo.def("AddCamera", &pragma::asset::MapExportInfo::AddCamera);
	defMapExportInfo.def("AddLightSource", &pragma::asset::MapExportInfo::AddLightSource);
	modAsset[defMapExportInfo];

	Lua::asset::register_library(GetLuaInterface(), false);

	auto defTexImportInfo = luabind::class_<pragma::asset::TextureImportInfo>("TextureImportInfo");
	defTexImportInfo.def(luabind::constructor<>());
	defTexImportInfo.def_readwrite("srgb", &pragma::asset::TextureImportInfo::srgb);
	defTexImportInfo.def_readwrite("normalMap", &pragma::asset::TextureImportInfo::normalMap);
	GetLuaInterface().RegisterLibrary("asset");
	modAsset[defTexImportInfo];

	Lua::RegisterLibraryEnums(GetLuaState(), "asset",
	  {{"TEXTURE_LOAD_FLAG_NONE", umath::to_integral(TextureLoadFlags::None)}, {"TEXTURE_LOAD_FLAG_LOAD_INSTANTLY_BIT", umath::to_integral(TextureLoadFlags::LoadInstantly)}, {"TEXTURE_LOAD_FLAG_RELOAD_BIT", umath::to_integral(TextureLoadFlags::Reload)},
	    {"TEXTURE_LOAD_FLAG_DONT_CACHE_BIT", umath::to_integral(TextureLoadFlags::DontCache)}});

	auto &utilImport = GetLuaInterface().RegisterLibrary("import", {{"export_scene", static_cast<int32_t (*)(lua_State *)>(Lua::lib_export::export_scene)}});

	auto modDebug = luabind::module_(GetLuaState(), "debug");
	modDebug[luabind::def("draw_points", &Lua::DebugRenderer::Client::DrawPoints), luabind::def("draw_lines", &Lua::DebugRenderer::Client::DrawLines), luabind::def("draw_point", &Lua::DebugRenderer::Client::DrawPoint),
	  luabind::def("draw_line", static_cast<std::shared_ptr<::DebugRenderer::BaseObject> (*)(const Vector3 &, const Vector3 &, const DebugRenderInfo &)>(&Lua::DebugRenderer::Client::DrawLine)),
	  luabind::def("draw_line", static_cast<std::shared_ptr<::DebugRenderer::BaseObject> (*)(const Vector3 &, const Vector3 &)>(&Lua::DebugRenderer::Client::DrawLine)), luabind::def("draw_box", &Lua::DebugRenderer::Client::DrawBox),
	  luabind::def("draw_mesh", &Lua::DebugRenderer::Client::DrawMeshes), luabind::def("draw_mesh", &Lua::DebugRenderer::Client::DrawMesh), luabind::def("draw_sphere", &Lua::DebugRenderer::Client::DrawSphere, luabind::default_parameter_policy<3, 1> {}),
	  luabind::def("draw_sphere", &Lua::DebugRenderer::Client::DrawSphere), luabind::def("draw_truncated_cone", &Lua::DebugRenderer::Client::DrawTruncatedCone, luabind::default_parameter_policy<6, 12u> {}),
	  luabind::def("draw_truncated_cone", &Lua::DebugRenderer::Client::DrawTruncatedCone), luabind::def("draw_cylinder", &Lua::DebugRenderer::Client::DrawCylinder, luabind::default_parameter_policy<5, 12u> {}), luabind::def("draw_cylinder", &Lua::DebugRenderer::Client::DrawCylinder),
	  luabind::def("draw_cone", &Lua::DebugRenderer::Client::DrawCone, luabind::default_parameter_policy<5, 12u> {}), luabind::def("draw_cone", &Lua::DebugRenderer::Client::DrawCone), luabind::def("draw_pose", &Lua::DebugRenderer::Client::DrawAxis),
	  luabind::def("draw_text", static_cast<std::shared_ptr<::DebugRenderer::BaseObject> (*)(const std::string &, const Vector2 &, const DebugRenderInfo &)>(&Lua::DebugRenderer::Client::DrawText)),
	  luabind::def("draw_text", static_cast<std::shared_ptr<::DebugRenderer::BaseObject> (*)(const std::string &, float, const DebugRenderInfo &)>(&Lua::DebugRenderer::Client::DrawText)),
	  luabind::def("draw_text", static_cast<std::shared_ptr<::DebugRenderer::BaseObject> (*)(const std::string &, const DebugRenderInfo &)>(&Lua::DebugRenderer::Client::DrawText)), luabind::def("draw_path", &Lua::DebugRenderer::Client::DrawPath),
#ifdef __clang__
	  luabind::def("draw_spline", &Lua::DebugRenderer::Client::DrawSpline),
	  luabind::def(
	    "draw_spline", +[](const std::vector<Vector3> &path, uint32_t numSegments, const DebugRenderInfo &renderInfo) { return Lua::DebugRenderer::Client::DrawSpline(path, numSegments, renderInfo, 1.f); }),
#else
	  luabind::def("draw_spline", &Lua::DebugRenderer::Client::DrawSpline, luabind::default_parameter_policy<4, 1.f> {}),
#endif
	  luabind::def("draw_plane", static_cast<std::shared_ptr<::DebugRenderer::BaseObject> (*)(const umath::Plane &, const DebugRenderInfo &)>(&Lua::DebugRenderer::Client::DrawPlane)),
	  luabind::def("draw_plane", static_cast<std::shared_ptr<::DebugRenderer::BaseObject> (*)(const Vector3 &, float, const DebugRenderInfo &)>(&Lua::DebugRenderer::Client::DrawPlane)),
	  luabind::def("draw_frustum", static_cast<std::shared_ptr<::DebugRenderer::BaseObject> (*)(pragma::CCameraComponent &, const DebugRenderInfo &)>(&Lua::DebugRenderer::Client::DrawFrustum)),
	  luabind::def("draw_frustum", static_cast<std::shared_ptr<::DebugRenderer::BaseObject> (*)(const std::vector<Vector3> &, const DebugRenderInfo &)>(&Lua::DebugRenderer::Client::DrawFrustum)),
	  luabind::def("create_collection", +[](const std::vector<std::shared_ptr<::DebugRenderer::BaseObject>> &objects) -> std::shared_ptr<::DebugRenderer::BaseObject> { return std::make_shared<::DebugRenderer::CollectionObject>(objects); })];
}
