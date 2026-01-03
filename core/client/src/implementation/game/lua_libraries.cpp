// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

static std::optional<std::string> find_asset_file(const std::string &name, pragma::asset::Type type)
{
	if(type == pragma::asset::Type::Texture) {
		pragma::material::TextureType type;
		auto found = false;
		auto filePath = translate_image_path(name, type, nullptr, &found);
		if(found == false)
			return {};
		pragma::util::Path path {filePath};
		path.PopFront();
		return path.GetString();
	}
	return find_file(name, type);
}
static bool is_asset_loaded(pragma::NetworkState &nw, const std::string &name, pragma::asset::Type type)
{
	switch(type) {
	case pragma::asset::Type::Texture:
		{
			auto *asset = static_cast<pragma::material::CMaterialManager &>(nw.GetMaterialManager()).GetTextureManager().FindCachedAsset(name);
			return asset != nullptr;
		}
	case pragma::asset::Type::ParticleSystem:
		{
			return pragma::ecs::CParticleSystemComponent::IsParticleFilePrecached(name);
		}
	}
	return is_loaded(nw, name, type);
}

static std::optional<uint32_t> save_image(lua::State *l, pragma::image::ImageBuffer &imgBuffer, std::string fileName, pragma::image::TextureInfo &imgWriteInfo, bool cubemap, const pragma::LuaCore::LuaThreadWrapper &tw)
{
	if(Lua::file::validate_write_operation(l, fileName) == false)
		return {};
	auto pImgBuffer = imgBuffer.shared_from_this();
	auto task = [pImgBuffer, fileName = std::move(fileName), imgWriteInfo, cubemap]() -> pragma::LuaCore::LuaThreadPool::ResultHandler {
		auto result = pragma::get_cgame()->SaveImage(*pImgBuffer, fileName, imgWriteInfo, cubemap);
		return [result](lua::State *l) { luabind::object {l, result}.push(l); };
	};
	if(tw.IsPool())
		return tw.GetPool().AddTask(task);
	tw.GetTask()->AddSubTask(task);
	return {};
}

static bool save_image(lua::State *l, pragma::image::ImageBuffer &imgBuffer, std::string fileName, pragma::image::TextureInfo &imgWriteInfo, bool cubemap)
{
	if(Lua::file::validate_write_operation(l, fileName) == false)
		return false;
	return pragma::get_cgame()->SaveImage(imgBuffer, fileName, imgWriteInfo, cubemap);
}

static std::vector<std::string> &get_image_file_extensions()
{
	static std::vector<std::string> exts;
	if(exts.empty()) {
		exts.reserve(pragma::math::to_integral(pragma::image::ImageFormat::Count));
		auto n = pragma::math::to_integral(pragma::image::ImageFormat::Count);
		for(auto i = decltype(n) {0u}; i < n; ++i)
			exts.push_back(pragma::image::get_file_extension(static_cast<pragma::image::ImageFormat>(i)));
	}
	return exts;
}

static std::pair<bool, std::optional<std::string>> save_image(lua::State *l, pragma::image::ImageBuffer &imgBuffer, std::string fileName, pragma::image::ImageFormat format, float quality, pragma::LuaCore::LuaThreadWrapper *tw = nullptr)
{
	if(Lua::file::validate_write_operation(l, fileName) == false)
		return std::pair<bool, std::optional<std::string>> {false, {}};
	ufile::remove_extension_from_filename(fileName, get_image_file_extensions());
	fileName += '.' + pragma::image::get_file_extension(format);
	auto fp = pragma::fs::open_file<pragma::fs::VFilePtrReal>(fileName, pragma::fs::FileMode::Write | pragma::fs::FileMode::Binary);
	if(!fp)
		return std::pair<bool, std::optional<std::string>> {false, {}};
	if(tw) {
		auto pImgBuffer = imgBuffer.shared_from_this();
		auto task = [fp, pImgBuffer, format, quality]() -> pragma::LuaCore::LuaThreadPool::ResultHandler {
			pragma::fs::File f {fp};
			auto result = pragma::image::save_image(f, *pImgBuffer, format, quality);
			return [result](lua::State *l) { luabind::object {l, result}.push(l); };
		};
		if(tw->IsPool())
			tw->GetPool().AddTask(task);
		else
			tw->GetTask()->AddSubTask(task);
		return std::pair<bool, std::optional<std::string>> {true, fileName};
	}
	pragma::fs::File f {fp};
	auto result = pragma::image::save_image(f, imgBuffer, format, quality);
	return std::pair<bool, std::optional<std::string>> {result, fileName};
}
static std::pair<bool, std::optional<std::string>> save_image(lua::State *l, pragma::image::ImageBuffer &imgBuffer, std::string fileName, pragma::image::ImageFormat format) { return save_image(l, imgBuffer, fileName, format, 1.f); }
static bool save_image(lua::State *l, luabind::table<> t, std::string fileName, pragma::image::TextureInfo &texInfo, bool cubemap)
{
	auto n = Lua::GetObjectLength(l, t);
	std::vector<std::shared_ptr<pragma::image::ImageBuffer>> imgBufs;
	imgBufs.reserve(n);
	auto o = luabind::object {luabind::from_stack(l, 1)};
	uint32_t maxWidth = 0;
	uint32_t maxHeight = 0;
	for(luabind::iterator it {o}, end; it != end; ++it) {
		auto val = *it;
		auto *imgBuf = luabind::object_cast<pragma::image::ImageBuffer *>(val);
		imgBufs.push_back(imgBuf->shared_from_this());
		maxWidth = pragma::math::max(maxWidth, imgBuf->GetWidth());
		maxHeight = pragma::math::max(maxHeight, imgBuf->GetHeight());
	}
	for(auto &imgBuf : imgBufs)
		imgBuf->Resize(maxWidth, maxHeight);
	if(imgBufs.empty())
		return false;
	auto &imgBuf = imgBufs.front();
	pragma::image::TextureSaveInfo saveInfo {};
	saveInfo.texInfo = texInfo;
	saveInfo.width = imgBuf->GetWidth();
	saveInfo.height = imgBuf->GetHeight();
	saveInfo.szPerPixel = imgBuf->GetPixelSize();
	saveInfo.numLayers = imgBufs.size();
	saveInfo.numMipmaps = 0;
	saveInfo.cubemap = cubemap;
	return pragma::image::save_texture(
	  fileName,
	  [&imgBufs](uint32_t iLayer, uint32_t iMipmap, std::function<void(void)> &outDeleter) -> const uint8_t * {
		  if(iMipmap > 0)
			  return nullptr;
		  return static_cast<uint8_t *>(imgBufs.at(iLayer)->GetData());
	  },
	  saveInfo);
}
static bool save_image(lua::State *l, luabind::table<> t, std::string fileName, pragma::image::TextureInfo &texInfo) { return save_image(l, t, fileName, texInfo, false); }

static bool save_image(lua::State *l, prosper::IImage &img, std::string fileName, pragma::image::TextureInfo &imgWriteInfo)
{
	if(Lua::file::validate_write_operation(l, fileName) == false)
		return false;
	return pragma::get_cgame()->SaveImage(img, fileName, imgWriteInfo);
}

static luabind::object load_image(lua::State *l, const std::string &fileName, bool loadAsynch, const std::optional<pragma::image::Format> &targetFormat)
{
	std::string ext;
	if(ufile::get_extension(fileName, &ext) == false)
		return {};
	auto fp = pragma::fs::open_file<pragma::fs::VFilePtrReal>(fileName, pragma::fs::FileMode::Read | pragma::fs::FileMode::Binary);
	if(fp == nullptr)
		return {};
	auto pixelFormat = pragma::image::PixelFormat::LDR;
	if(pragma::string::compare<std::string>(ext, "hdr"))
		pixelFormat = pragma::image::PixelFormat::Float;

	if(loadAsynch) {
		class ImageLoadJob : public pragma::util::ParallelWorker<std::shared_ptr<pragma::image::ImageBuffer>> {
		  public:
			ImageLoadJob(pragma::fs::VFilePtr fp, pragma::image::PixelFormat pixelFormat, std::optional<pragma::image::Format> targetFormat)
			{
				AddThread([this, fp, pixelFormat, targetFormat]() {
					pragma::fs::File f {fp};
					m_imgBuffer = pragma::image::load_image(f, pixelFormat);
					if(m_imgBuffer == nullptr) {
						SetStatus(pragma::util::JobStatus::Failed, "Unable to open image!");
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

			virtual std::shared_ptr<pragma::image::ImageBuffer> GetResult() override { return m_imgBuffer; }
		  private:
			std::shared_ptr<pragma::image::ImageBuffer> m_imgBuffer = nullptr;
		};
		return {l, pragma::util::create_parallel_job<ImageLoadJob>(fp, pixelFormat, targetFormat)};
	}
	pragma::fs::File f {fp};
	auto imgBuffer = pragma::image::load_image(f, pixelFormat);
	if(imgBuffer == nullptr)
		return {};
	if(targetFormat.has_value())
		imgBuffer->Convert(*targetFormat);
	return {l, imgBuffer};
}

static luabind::object load_image(lua::State *l, const std::string &fileName, bool loadAsynch, pragma::image::Format targetFormat) { return load_image(l, fileName, loadAsynch, std::optional<pragma::image::Format> {targetFormat}); }

static luabind::object load_image(lua::State *l, const std::string &fileName, bool loadAsynch) { return load_image(l, fileName, loadAsynch, std::optional<pragma::image::Format> {}); }

static luabind::object load_image(lua::State *l, const std::string &fileName) { return load_image(l, fileName, false); }

static pragma::util::ParallelJob<pragma::image::ImageLayerSet> capture_raytraced_screenshot(lua::State *l, uint32_t width, uint32_t height, uint32_t samples, bool hdrOutput, bool denoise)
{
	pragma::rendering::cycles::RenderImageInfo renderImgInfo {};
	auto *pCam = pragma::get_cgame()->GetRenderCamera<pragma::CCameraComponent>();
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
	return pragma::rendering::cycles::render_image(*pragma::get_client_state(), sceneInfo, renderImgInfo);
}
static pragma::util::ParallelJob<pragma::image::ImageLayerSet> capture_raytraced_screenshot(lua::State *l, uint32_t width, uint32_t height, uint32_t samples, bool hdrOutput) { return capture_raytraced_screenshot(l, width, height, samples, hdrOutput, true); }
static pragma::util::ParallelJob<pragma::image::ImageLayerSet> capture_raytraced_screenshot(lua::State *l, uint32_t width, uint32_t height, uint32_t samples) { return capture_raytraced_screenshot(l, width, height, samples, false, true); }
static pragma::util::ParallelJob<pragma::image::ImageLayerSet> capture_raytraced_screenshot(lua::State *l, uint32_t width, uint32_t height) { return capture_raytraced_screenshot(l, width, height, 1'024, false, true); }

static bool asset_import(pragma::NetworkState &nw, const std::string &name, const std::string &outputName, pragma::asset::Type type)
{
	if(type == pragma::asset::Type::Map)
		return pragma::util::port_hl2_map(&nw, name);
	auto *manager = nw.GetAssetManager(type);
	if(!manager)
		return false;
	return manager->Import(name, outputName);
}

void pragma::CGame::RegisterLuaLibraries()
{
	Lua::util::register_library(GetLuaState());

	auto osMod = luabind::module(GetLuaState(), "os");
	Lua::util::register_os(GetLuaState(), osMod);

	auto utilMod = luabind::module(GetLuaState(), "util");
	utilMod[luabind::def("is_windowless", &CEngine::IsWindowless)];
	utilMod[luabind::def("is_cpu_rendering_only", &CEngine::IsCPURenderingOnly)];
	Lua::util::register_shared(GetLuaState(), utilMod);

	auto svgImageInfoDef = luabind::class_<image::SvgImageInfo>("SvgImageInfo");
	svgImageInfoDef.def(luabind::constructor<>());
	svgImageInfoDef.def_readwrite("styleSheet", &image::SvgImageInfo::styleSheet);
	svgImageInfoDef.def_readwrite("width", &image::SvgImageInfo::width);
	svgImageInfoDef.def_readwrite("height", &image::SvgImageInfo::height);
	utilMod[svgImageInfoDef];

	utilMod[(luabind::def("calc_world_direction_from_2d_coordinates", Lua::util::calc_world_direction_from_2d_coordinates), luabind::def("calc_world_direction_from_2d_coordinates", Lua::util::Client::calc_world_direction_from_2d_coordinates),
	  luabind::def("create_particle_tracer", Lua::util::Client::create_particle_tracer), luabind::def("create_muzzle_flash", Lua::util::Client::create_muzzle_flash), luabind::def("fire_bullets", static_cast<luabind::object (*)(lua::State *, game::BulletInfo &)>(Lua::util::fire_bullets)),
	  luabind::def("save_image", static_cast<bool (*)(lua::State *, image::ImageBuffer &, std::string, image::TextureInfo &, bool)>(save_image)),
	  luabind::def("save_image", static_cast<bool (*)(lua::State *, image::ImageBuffer &, std::string, image::TextureInfo &, bool)>(save_image), luabind::default_parameter_policy<5, false> {}),
	  luabind::def(
	    "save_image", +[](lua::State *l, image::ImageBuffer &imgBuffer, std::string fileName, image::TextureInfo &imgWriteInfo, const LuaCore::LuaThreadWrapper &tw) { return save_image(l, imgBuffer, fileName, imgWriteInfo, false, tw); }),
	  luabind::def(
	    "save_image", +[](lua::State *l, image::ImageBuffer &imgBuffer, std::string fileName, image::ImageFormat format, float quality) { return save_image(l, imgBuffer, fileName, format, quality); }),
	  luabind::def(
	    "save_image", +[](lua::State *l, image::ImageBuffer &imgBuffer, std::string fileName, image::ImageFormat format, float quality, const LuaCore::LuaThreadWrapper &tw) { return save_image(l, imgBuffer, fileName, format, quality, const_cast<LuaCore::LuaThreadWrapper *>(&tw)); }),
	  luabind::def("save_image", static_cast<std::pair<bool, std::optional<std::string>> (*)(lua::State *, image::ImageBuffer &, std::string, image::ImageFormat)>(save_image)),
	  luabind::def("save_image", static_cast<bool (*)(lua::State *, luabind::table<>, std::string, image::TextureInfo &, bool)>(save_image)), luabind::def("save_image", static_cast<bool (*)(lua::State *, luabind::table<>, std::string, image::TextureInfo &)>(save_image)),
	  luabind::def("save_image", static_cast<bool (*)(lua::State *, prosper::IImage &, std::string, image::TextureInfo &)>(save_image)), luabind::def("load_image", static_cast<luabind::object (*)(lua::State *, const std::string &, bool, image::Format)>(load_image)),
	  luabind::def("load_image", static_cast<luabind::object (*)(lua::State *, const std::string &, bool)>(load_image)), luabind::def("load_image", static_cast<luabind::object (*)(lua::State *, const std::string &)>(load_image)),
	  luabind::def("load_svg", static_cast<std::shared_ptr<image::ImageBuffer> (*)(const std::string &, const image::SvgImageInfo &)>(&image::load_svg)),
	  luabind::def(
	    "load_svg", +[](const std::string &fileName) -> std::shared_ptr<image::ImageBuffer> { return image::load_svg(fileName); }),
	  luabind::def("screenshot", util::screenshot), luabind::def("capture_raytraced_screenshot", static_cast<util::ParallelJob<image::ImageLayerSet> (*)(lua::State *, uint32_t, uint32_t, uint32_t, bool, bool)>(capture_raytraced_screenshot)),
	  luabind::def("capture_raytraced_screenshot", static_cast<util::ParallelJob<image::ImageLayerSet> (*)(lua::State *, uint32_t, uint32_t, uint32_t, bool)>(capture_raytraced_screenshot)),
	  luabind::def("capture_raytraced_screenshot", static_cast<util::ParallelJob<image::ImageLayerSet> (*)(lua::State *, uint32_t, uint32_t, uint32_t)>(capture_raytraced_screenshot)),
	  luabind::def("capture_raytraced_screenshot", static_cast<util::ParallelJob<image::ImageLayerSet> (*)(lua::State *, uint32_t, uint32_t)>(capture_raytraced_screenshot)),
	  luabind::def(
	    "cubemap_to_equirectangular_texture",
	    +[](lua::State *l, prosper::Texture &cubemap) -> luabind::
	                                                    object {
		                                                    auto *shader = static_cast<ShaderCubemapToEquirectangular *>(get_cengine()->GetShader("cubemap_to_equirectangular").get());
		                                                    if(shader == nullptr)
			                                                    return {};
		                                                    auto equiRect = shader->CubemapToEquirectangularTexture(cubemap);
		                                                    if(equiRect == nullptr)
			                                                    return {};
		                                                    return {l, equiRect};
	                                                    }),
	  luabind::def(
	    "equirectangular_to_cubemap_texture", +[](lua::State *l, prosper::Texture &equiRect, uint32_t resolution) -> luabind::object {
		    auto *shader = static_cast<ShaderEquirectangularToCubemap *>(get_cengine()->GetShader("equirectangular_to_cubemap").get());
		    if(shader == nullptr)
			    return {};
		    auto tex = shader->EquirectangularTextureToCubemap(equiRect, resolution);
		    if(tex == nullptr)
			    return {};
		    return {l, tex};
	    }))];
	utilMod[(
	  // luabind::def("fire_bullets",static_cast<int32_t(*)(lua::State*)>(Lua::util::fire_bullets)),
	  luabind::def("get_clipboard_string", Lua::util::Client::get_clipboard_string), luabind::def("set_clipboard_string", Lua::util::Client::set_clipboard_string), luabind::def("create_giblet", Lua::util::Client::create_giblet),
	  luabind::def("get_image_format_file_extension", image::get_file_extension), luabind::def("bake_directional_lightmap_atlas", Lua::util::Client::bake_directional_lightmap_atlas))];

	auto imgWriteInfoDef = luabind::class_<image::TextureInfo>("TextureInfo");
	imgWriteInfoDef.def(luabind::constructor<>());
	imgWriteInfoDef.add_static_constant("INPUT_FORMAT_KEEP_INPUT_IMAGE_FORMAT", math::to_integral(image::TextureInfo::InputFormat::KeepInputImageFormat));
	imgWriteInfoDef.add_static_constant("INPUT_FORMAT_R16G16B16A16_FLOAT", math::to_integral(image::TextureInfo::InputFormat::R16G16B16A16_Float));
	imgWriteInfoDef.add_static_constant("INPUT_FORMAT_R32G32B32A32_FLOAT", math::to_integral(image::TextureInfo::InputFormat::R32G32B32A32_Float));
	imgWriteInfoDef.add_static_constant("INPUT_FORMAT_R32_FLOAT", math::to_integral(image::TextureInfo::InputFormat::R32_Float));
	imgWriteInfoDef.add_static_constant("INPUT_FORMAT_R8G8B8A8_UINT", math::to_integral(image::TextureInfo::InputFormat::R8G8B8A8_UInt));

	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_KEEP_INPUT_IMAGE_FORMAT", math::to_integral(image::TextureInfo::OutputFormat::KeepInputImageFormat));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_RGB", math::to_integral(image::TextureInfo::OutputFormat::RGB));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_RGBA", math::to_integral(image::TextureInfo::OutputFormat::RGBA));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_DXT1", math::to_integral(image::TextureInfo::OutputFormat::DXT1));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_DXT1A", math::to_integral(image::TextureInfo::OutputFormat::DXT1a));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_DXT3", math::to_integral(image::TextureInfo::OutputFormat::DXT3));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_DXT5", math::to_integral(image::TextureInfo::OutputFormat::DXT5));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_DXT5N", math::to_integral(image::TextureInfo::OutputFormat::DXT5n));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC1", math::to_integral(image::TextureInfo::OutputFormat::BC1));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC1A", math::to_integral(image::TextureInfo::OutputFormat::BC1a));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC2", math::to_integral(image::TextureInfo::OutputFormat::BC2));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC3", math::to_integral(image::TextureInfo::OutputFormat::BC3));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC3N", math::to_integral(image::TextureInfo::OutputFormat::BC3n));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC4", math::to_integral(image::TextureInfo::OutputFormat::BC4));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC5", math::to_integral(image::TextureInfo::OutputFormat::BC5));
	// imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_DXT1N", pragma::math::to_integral(image::TextureInfo::OutputFormat::DXT1n));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_CTX1", math::to_integral(image::TextureInfo::OutputFormat::CTX1));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC6", math::to_integral(image::TextureInfo::OutputFormat::BC6));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC7", math::to_integral(image::TextureInfo::OutputFormat::BC7));
	// imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_BC3_RGBM", pragma::math::to_integral(image::TextureInfo::OutputFormat::BC3_RGBM));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_ETC1", math::to_integral(image::TextureInfo::OutputFormat::ETC1));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_ETC2_R", math::to_integral(image::TextureInfo::OutputFormat::ETC2_R));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_ETC2_RG", math::to_integral(image::TextureInfo::OutputFormat::ETC2_RG));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_ETC2_RGB", math::to_integral(image::TextureInfo::OutputFormat::ETC2_RGB));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_ETC2_RGBA", math::to_integral(image::TextureInfo::OutputFormat::ETC2_RGBA));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_ETC2_RGB_A1", math::to_integral(image::TextureInfo::OutputFormat::ETC2_RGB_A1));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_ETC2_RGBM", math::to_integral(image::TextureInfo::OutputFormat::ETC2_RGBM));

	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_COLOR_MAP", math::to_integral(image::TextureInfo::OutputFormat::ColorMap));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_COLOR_MAP_1BIT_ALPHA", math::to_integral(image::TextureInfo::OutputFormat::ColorMap1BitAlpha));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_COLOR_MAP_SHARP_ALPHA", math::to_integral(image::TextureInfo::OutputFormat::ColorMapSharpAlpha));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_COLOR_MAP_SMOOTH_ALPHA", math::to_integral(image::TextureInfo::OutputFormat::ColorMapSmoothAlpha));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_NORMAL_MAP", math::to_integral(image::TextureInfo::OutputFormat::NormalMap));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_HDR_COLOR_MAP", math::to_integral(image::TextureInfo::OutputFormat::HDRColorMap));
	imgWriteInfoDef.add_static_constant("OUTPUT_FORMAT_GRADIENT_MAP", math::to_integral(image::TextureInfo::OutputFormat::GradientMap));

	imgWriteInfoDef.add_static_constant("CONTAINER_FORMAT_DDS", math::to_integral(image::TextureInfo::ContainerFormat::DDS));
	imgWriteInfoDef.add_static_constant("CONTAINER_FORMAT_KTX", math::to_integral(image::TextureInfo::ContainerFormat::KTX));

	imgWriteInfoDef.add_static_constant("FLAG_NONE", math::to_integral(image::TextureInfo::Flags::None));
	imgWriteInfoDef.add_static_constant("FLAG_BIT_CONVERT_TO_NORMAL_MAP", math::to_integral(image::TextureInfo::Flags::ConvertToNormalMap));
	imgWriteInfoDef.add_static_constant("FLAG_BIT_SRGB", math::to_integral(image::TextureInfo::Flags::SRGB));
	imgWriteInfoDef.add_static_constant("FLAG_BIT_GENERATE_MIPMAPS", math::to_integral(image::TextureInfo::Flags::GenerateMipmaps));

	imgWriteInfoDef.add_static_constant("MIPMAP_FILTER_BOX", math::to_integral(image::TextureInfo::MipmapFilter::Box));
	imgWriteInfoDef.add_static_constant("MIPMAP_FILTER_KAISER", math::to_integral(image::TextureInfo::MipmapFilter::Kaiser));

	imgWriteInfoDef.add_static_constant("WRAP_MODE_CLAMP", math::to_integral(image::TextureInfo::WrapMode::Clamp));
	imgWriteInfoDef.add_static_constant("WRAP_MODE_REPEAT", math::to_integral(image::TextureInfo::WrapMode::Repeat));
	imgWriteInfoDef.add_static_constant("WRAP_MODE_MIRROR", math::to_integral(image::TextureInfo::WrapMode::Mirror));

	imgWriteInfoDef.def_readwrite("inputFormat", reinterpret_cast<std::underlying_type_t<decltype(image::TextureInfo::inputFormat)> image::TextureInfo::*>(&image::TextureInfo::inputFormat));
	imgWriteInfoDef.def_readwrite("outputFormat", reinterpret_cast<std::underlying_type_t<decltype(image::TextureInfo::outputFormat)> image::TextureInfo::*>(&image::TextureInfo::outputFormat));
	imgWriteInfoDef.def_readwrite("containerFormat", reinterpret_cast<std::underlying_type_t<decltype(image::TextureInfo::containerFormat)> image::TextureInfo::*>(&image::TextureInfo::containerFormat));
	imgWriteInfoDef.def_readwrite("flags", reinterpret_cast<std::underlying_type_t<decltype(image::TextureInfo::flags)> image::TextureInfo::*>(&image::TextureInfo::flags));
	imgWriteInfoDef.def_readwrite("mipMapFilter", reinterpret_cast<std::underlying_type_t<decltype(image::TextureInfo::mipMapFilter)> image::TextureInfo::*>(&image::TextureInfo::mipMapFilter));
	imgWriteInfoDef.def_readwrite("wrapMode", reinterpret_cast<std::underlying_type_t<decltype(image::TextureInfo::wrapMode)> image::TextureInfo::*>(&image::TextureInfo::wrapMode));

	imgWriteInfoDef.def("SetNormalMap", static_cast<void (*)(lua::State *, image::TextureInfo &)>([](lua::State *l, image::TextureInfo &writeInfo) { writeInfo.SetNormalMap(); }));
	utilMod[imgWriteInfoDef];

	Lua::RegisterLibraryEnums(GetLuaState(), "util",
	  {{"IMAGE_FORMAT_PNG", math::to_integral(image::ImageFormat::PNG)}, {"IMAGE_FORMAT_BMP", math::to_integral(image::ImageFormat::BMP)}, {"IMAGE_FORMAT_TGA", math::to_integral(image::ImageFormat::TGA)}, {"IMAGE_FORMAT_JPG", math::to_integral(image::ImageFormat::JPG)},
	    {"IMAGE_FORMAT_HDR", math::to_integral(image::ImageFormat::HDR)}, {"IMAGE_FORMAT_COUNT", math::to_integral(image::ImageFormat::Count)},

	    {"PIXEL_FORMAT_LDR", math::to_integral(image::PixelFormat::LDR)}, {"PIXEL_FORMAT_HDR", math::to_integral(image::PixelFormat::HDR)}, {"PIXEL_FORMAT_FLOAT", math::to_integral(image::PixelFormat::Float)}});

	Lua::ai::client::register_library(GetLuaInterface());

	Game::RegisterLuaLibraries();
	ClientState::RegisterSharedLuaLibraries(GetLuaInterface());

	auto consoleMod = luabind::module(GetLuaState(), "console");
	consoleMod[luabind::def("save_config", +[](CEngine &engine) { engine.SaveClientConfig(); })];

	GetLuaInterface().RegisterLibrary("asset",
	  {{"export_map", Lua::util::Client::export_map}, {"import_model", Lua::util::Client::import_model}, {"import_gltf", Lua::util::Client::import_gltf}, {"export_texture", Lua::util::Client::export_texture}, {"export_material", Lua::util::Client::export_material},
	    {"export_texture_as_vtf", static_cast<int32_t (*)(lua::State *)>([](lua::State *l) -> int32_t {
		     std::string fileName = Lua::CheckString(l, 1);
		     if(Lua::file::validate_write_operation(l, fileName) == false || fs::create_path(ufile::get_path_from_filename(fileName)) == false) {
			     Lua::PushBool(l, false);
			     return 1;
		     }

		     auto &img = Lua::Check<prosper::IImage>(l, 2);
		     auto vtfOutputFormat = asset::prosper_format_to_vtf(img.GetFormat());
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
			     vtfOutputFormat = asset::prosper_format_to_vtf(static_cast<prosper::Format>(Lua::CheckInt(l, arg)));
		     ++arg;
		     if(vtfOutputFormat.has_value() == false) {
			     Lua::PushBool(l, false);
			     return 1;
		     }

		     asset::VtfInfo vtfInfo {};
		     vtfInfo.outputFormat = *vtfOutputFormat;
		     math::set_flag(vtfInfo.flags, asset::VtfInfo::Flags::Srgb, srgb);
		     math::set_flag(vtfInfo.flags, asset::VtfInfo::Flags::NormalMap, normalMap);
		     math::set_flag(vtfInfo.flags, asset::VtfInfo::Flags::GenerateMipmaps, generateMipmaps);
		     auto result = pragma::asset::export_texture_as_vtf(fileName, img, vtfInfo, nullptr, false);
		     Lua::PushBool(l, result);
		     return 1;
	     })},
	    {"exists", static_cast<int32_t (*)(lua::State *)>([](lua::State *l) -> int32_t {
		     std::string name = Lua::CheckString(l, 1);
		     auto type = static_cast<asset::Type>(Lua::CheckInt(l, 2));
		     auto *nw = get_cengine()->GetNetworkState(l);
		     auto fileName = find_asset_file(name, type);
		     Lua::PushBool(l, fileName.has_value());
		     return 1;
	     })},
	    {"find_file", static_cast<int32_t (*)(lua::State *)>([](lua::State *l) -> int32_t {
		     std::string name = Lua::CheckString(l, 1);
		     auto type = static_cast<asset::Type>(Lua::CheckInt(l, 2));
		     auto *nw = get_cengine()->GetNetworkState(l);
		     auto fileName = find_asset_file(name, type);
		     if(fileName.has_value() == false)
			     return 0;
		     Lua::PushString(l, *fileName);
		     return 1;
	     })},
	    {"is_loaded", static_cast<int32_t (*)(lua::State *)>([](lua::State *l) -> int32_t {
		     std::string name = Lua::CheckString(l, 1);
		     auto type = static_cast<asset::Type>(Lua::CheckInt(l, 2));
		     auto *nw = get_cengine()->GetNetworkState(l);
		     Lua::PushBool(l, is_asset_loaded(*nw, name, type));
		     return 1;
	     })},
	    {"import_texture", static_cast<int32_t (*)(lua::State *)>([](lua::State *l) -> int32_t {
		     auto &texImportInfo = Lua::Check<asset::TextureImportInfo>(l, 2);
		     std::string outputPath = Lua::CheckString(l, 3);
		     auto result = false;
		     std::string errMsg;
		     if(Lua::IsString(l, 1)) {
			     std::string name = Lua::CheckString(l, 1);
			     result = pragma::asset::import_texture(name, texImportInfo, outputPath, errMsg);
		     }
		     else if(Lua::IsType<LFile>(l, 1)) {
			     auto &f = Lua::Check<LFile>(l, 1);
			     auto fp = std::make_unique<ufile::FileWrapper>(f.GetHandle());
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
	modAsset[(luabind::def("clear_unused_textures", static_cast<uint32_t (*)()>([]() -> uint32_t { return static_cast<material::CMaterialManager &>(get_client_state()->GetMaterialManager()).GetTextureManager().ClearUnused(); })),

	  luabind::def(
	    "load",
	    +[](lua::State *l, LFile &f, asset::Type type) -> Lua::var<bool, luabind::object> {
		    // See also core/server/src/lua/library.cpp
		    auto *manager = get_engine()->GetNetworkState(l)->GetAssetManager(type);
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
		    case asset::Type::Model:
			    return luabind::object {l, std::static_pointer_cast<asset::Model>(asset)};
		    case asset::Type::Material:
			    return luabind::object {l, std::static_pointer_cast<material::Material>(asset)};
		    case asset::Type::Texture:
			    return luabind::object {l, std::static_pointer_cast<material::Texture>(asset)};
		    }
		    return luabind::object {};
	    }),
	  luabind::def(
	    "load",
	    +[](lua::State *l, const std::string &name, asset::Type type) -> Lua::var<bool, luabind::object> {
		    // See also core/server/src/lua/library.cpp
		    auto *manager = get_engine()->GetNetworkState(l)->GetAssetManager(type);
		    if(!manager)
			    return luabind::object {l, false};
		    auto asset = manager->LoadAsset(name);
		    switch(type) {
		    case asset::Type::Model:
			    return luabind::object {l, std::static_pointer_cast<asset::Model>(asset)};
		    case asset::Type::Material:
			    return luabind::object {l, std::static_pointer_cast<material::Material>(asset)};
		    case asset::Type::Texture:
			    return luabind::object {l, std::static_pointer_cast<material::Texture>(asset)};
		    }
		    return luabind::object {};
	    }),
	  luabind::def(
	    "reload",
	    +[](lua::State *l, const std::string &name, asset::Type type) -> Lua::var<bool, luabind::object> {
		    auto *manager = get_cengine()->GetNetworkState(l)->GetAssetManager(type);
		    if(!manager)
			    return luabind::object {l, false};
		    auto asset = manager->ReloadAsset(name);
		    switch(type) {
		    case asset::Type::Model:
			    return luabind::object {l, std::static_pointer_cast<asset::Model>(asset)};
		    case asset::Type::Material:
			    return luabind::object {l, std::static_pointer_cast<material::Material>(asset)};
		    case asset::Type::Texture:
			    return luabind::object {l, std::static_pointer_cast<material::Texture>(asset)};
		    }
		    return luabind::object {};
	    }),
	  luabind::def(
	    "import", +[](NetworkState &nw, const std::string &name, asset::Type type) -> bool { return asset_import(nw, name, name, type); }),
	  luabind::def("import", +[](NetworkState &nw, const std::string &name, const std::string &outputName, asset::Type type) -> bool { return asset_import(nw, name, outputName, type); }))];
	auto defMapExportInfo = luabind::class_<asset::MapExportInfo>("MapExportInfo");
	defMapExportInfo.def(luabind::constructor<>());
	defMapExportInfo.def_readwrite("includeMapLightSources", &asset::MapExportInfo::includeMapLightSources);
	defMapExportInfo.def("AddCamera", &asset::MapExportInfo::AddCamera);
	defMapExportInfo.def("AddLightSource", &asset::MapExportInfo::AddLightSource);
	modAsset[defMapExportInfo];

	Lua::asset::register_library(GetLuaInterface(), false);
	Lua::asset_client::register_library(GetLuaInterface(), modAsset);

	auto defTexImportInfo = luabind::class_<asset::TextureImportInfo>("TextureImportInfo");
	defTexImportInfo.def(luabind::constructor<>());
	defTexImportInfo.def_readwrite("srgb", &asset::TextureImportInfo::srgb);
	defTexImportInfo.def_readwrite("normalMap", &asset::TextureImportInfo::normalMap);
	GetLuaInterface().RegisterLibrary("asset");
	modAsset[defTexImportInfo];

	Lua::RegisterLibraryEnums(GetLuaState(), "asset",
	  {{"TEXTURE_LOAD_FLAG_NONE", math::to_integral(material::TextureLoadFlags::None)}, {"TEXTURE_LOAD_FLAG_LOAD_INSTANTLY_BIT", math::to_integral(material::TextureLoadFlags::LoadInstantly)}, {"TEXTURE_LOAD_FLAG_RELOAD_BIT", math::to_integral(material::TextureLoadFlags::Reload)},
	    {"TEXTURE_LOAD_FLAG_DONT_CACHE_BIT", math::to_integral(material::TextureLoadFlags::DontCache)}});

	auto &utilImport = GetLuaInterface().RegisterLibrary("import", {{"export_scene", static_cast<int32_t (*)(lua::State *)>(Lua::lib_export::export_scene)}});

	auto modDebug = luabind::module_(GetLuaState(), "debug");
	modDebug[(luabind::def("draw_points", &Lua::DebugRenderer::Client::DrawPoints), luabind::def("draw_lines", &Lua::DebugRenderer::Client::DrawLines), luabind::def("draw_point", &Lua::DebugRenderer::Client::DrawPoint),
	  luabind::def("draw_line", static_cast<std::shared_ptr<debug::DebugRenderer::BaseObject> (*)(const Vector3 &, const Vector3 &, const debug::DebugRenderInfo &)>(&Lua::DebugRenderer::Client::DrawLine)),
	  luabind::def("draw_line", static_cast<std::shared_ptr<debug::DebugRenderer::BaseObject> (*)(const Vector3 &, const Vector3 &)>(&Lua::DebugRenderer::Client::DrawLine)), luabind::def("draw_box", &Lua::DebugRenderer::Client::DrawBox),
	  luabind::def("draw_mesh", &Lua::DebugRenderer::Client::DrawMeshes), luabind::def("draw_mesh", &Lua::DebugRenderer::Client::DrawMesh), luabind::def("draw_sphere", &Lua::DebugRenderer::Client::DrawSphere, luabind::default_parameter_policy<3, 1> {}),
	  luabind::def("draw_sphere", &Lua::DebugRenderer::Client::DrawSphere), luabind::def("draw_truncated_cone", &Lua::DebugRenderer::Client::DrawTruncatedCone, luabind::default_parameter_policy<6, 12u> {}),
	  luabind::def("draw_truncated_cone", &Lua::DebugRenderer::Client::DrawTruncatedCone), luabind::def("draw_cylinder", &Lua::DebugRenderer::Client::DrawCylinder, luabind::default_parameter_policy<5, 12u> {}), luabind::def("draw_cylinder", &Lua::DebugRenderer::Client::DrawCylinder),
	  luabind::def("draw_cone", &Lua::DebugRenderer::Client::DrawCone, luabind::default_parameter_policy<5, 12u> {}), luabind::def("draw_cone", &Lua::DebugRenderer::Client::DrawCone), luabind::def("draw_pose", &Lua::DebugRenderer::Client::DrawAxis),
	  luabind::def("draw_text", static_cast<std::shared_ptr<debug::DebugRenderer::BaseObject> (*)(const std::string &, const Vector2 &, const debug::DebugRenderInfo &)>(&Lua::DebugRenderer::Client::DrawText)),
	  luabind::def("draw_text", static_cast<std::shared_ptr<debug::DebugRenderer::BaseObject> (*)(const std::string &, float, const debug::DebugRenderInfo &)>(&Lua::DebugRenderer::Client::DrawText)),
	  luabind::def("draw_text", static_cast<std::shared_ptr<debug::DebugRenderer::BaseObject> (*)(const std::string &, const debug::DebugRenderInfo &)>(&Lua::DebugRenderer::Client::DrawText)), luabind::def("draw_path", &Lua::DebugRenderer::Client::DrawPath),
#ifdef __clang__
	  luabind::def("draw_spline", &Lua::DebugRenderer::Client::DrawSpline),
	  luabind::def(
	    "draw_spline", +[](const std::vector<Vector3> &path, uint32_t numSegments, const debug::DebugRenderInfo &renderInfo) { return Lua::DebugRenderer::Client::DrawSpline(path, numSegments, renderInfo, 1.f); }),
#else
	  luabind::def("draw_spline", &Lua::DebugRenderer::Client::DrawSpline, luabind::default_parameter_policy<4, 1.f> {}),
#endif
	  luabind::def("draw_plane", static_cast<std::shared_ptr<debug::DebugRenderer::BaseObject> (*)(const math::Plane &, const debug::DebugRenderInfo &)>(&Lua::DebugRenderer::Client::DrawPlane)),
	  luabind::def("draw_plane", static_cast<std::shared_ptr<debug::DebugRenderer::BaseObject> (*)(const Vector3 &, float, const debug::DebugRenderInfo &)>(&Lua::DebugRenderer::Client::DrawPlane)),
	  luabind::def("draw_frustum", static_cast<std::shared_ptr<debug::DebugRenderer::BaseObject> (*)(CCameraComponent &, const debug::DebugRenderInfo &)>(&Lua::DebugRenderer::Client::DrawFrustum)),
	  luabind::def("draw_frustum", static_cast<std::shared_ptr<debug::DebugRenderer::BaseObject> (*)(const std::vector<Vector3> &, const debug::DebugRenderInfo &)>(&Lua::DebugRenderer::Client::DrawFrustum)),
	  luabind::def("create_collection", +[](const std::vector<std::shared_ptr<debug::DebugRenderer::BaseObject>> &objects) -> std::shared_ptr<debug::DebugRenderer::BaseObject> { return pragma::util::make_shared<debug::DebugRenderer::CollectionObject>(objects); }))];
}
