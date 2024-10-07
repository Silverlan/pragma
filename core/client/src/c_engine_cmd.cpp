/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_cengine.h"

namespace pragma::string {
	class Utf8String;
	class Utf8StringView;
	class Utf8StringArg;
};

#include "pragma/c_engine.h"
#include "pragma/audio/c_audio.hpp"
#include "pragma/rendering/render_processor.hpp"
#include <pragma/asset/util_asset.hpp>
#include <pragma/lua/libraries/ldebug.h>
#include <pragma/lua/libraries/lutil.hpp>
#include <pragma/lua/util.hpp>
#include <pragma/lua/libraries/lutil.hpp>
#include <pragma/logging.hpp>
#include <pragma/rendering/render_apis.hpp>
#include <pragma/console/convars.h>
#include <sharedutils/util_file.h>
#include <sharedutils/util_library.hpp>
#include <cmaterialmanager.h>
#include <cmaterial_manager2.hpp>
#include <texturemanager/texture.h>
#include <pragma/console/command_options.hpp>
#include <buffers/prosper_buffer.hpp>
#include <buffers/prosper_dynamic_resizable_buffer.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/entities/components/renderers/c_rasterization_renderer_component.hpp>
#include <pragma/localization.h>
#include <image/prosper_render_target.hpp>
#include <shader/prosper_shader_blur.hpp>
#include <prosper_window.hpp>
#include <fsys/directory_watcher.h>

extern DLLCLIENT void debug_render_stats(bool enabled, bool full, bool print, bool continuous);
extern bool g_dumpRenderQueues;
void CEngine::RegisterConsoleCommands()
{
	Engine::RegisterConsoleCommands();
	auto &conVarMap = *console_system::client::get_convar_map();
	RegisterSharedConsoleCommands(conVarMap);
	conVarMap.RegisterConCommand(
	  "lua_exec_cl",
	  [](NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  if(argv.empty()) {
			  Con::cwar << "No argument given to execute!" << Con::endl;
			  return;
		  }
		  if(!state->IsGameActive() || state->GetGameState() == nullptr) {
			  Con::cwar << "No game is active! Lua code cannot be executed without an active game!" << Con::endl;
			  return;
		  }
		  auto fname = argv.at(0);
		  if(argv.size() > 1 && argv[1] == "nocache") {
			  Lua::set_ignore_include_cache(true);
			  state->GetGameState()->ExecuteLuaFile(fname);
			  Lua::set_ignore_include_cache(false);
			  return;
		  }
		  Lua::global::include(state->GetLuaState(), fname, nullptr, true, false);
	  },
	  ConVarFlags::None, "Opens and executes a lua-file on the client.",
	  [](const std::string &arg, std::vector<std::string> &autoCompleteOptions) {
		  std::vector<std::string> resFiles;
		  auto path = Lua::SCRIPT_DIRECTORY_SLASH + arg;
		  FileManager::FindFiles((path + "*." + Lua::FILE_EXTENSION).c_str(), &resFiles, nullptr);
		  FileManager::FindFiles((path + "*." + Lua::FILE_EXTENSION_PRECOMPILED).c_str(), &resFiles, nullptr);
		  autoCompleteOptions.reserve(resFiles.size());
		  path = ufile::get_path_from_filename(path.substr(4));
		  for(auto &mapName : resFiles) {
			  auto fullPath = path + mapName;
			  ustring::replace(fullPath, "\\", "/");
			  autoCompleteOptions.push_back(fullPath);
		  }
	  });
	conVarMap.RegisterConVar<bool>("cl_downscale_imported_high_resolution_rma_textures", true, ConVarFlags::Archive, "If enabled, imported high-resolution RMA textures will be downscaled to a more memory-friendly size.");
	conVarMap.RegisterConVarCallback("cl_downscale_imported_high_resolution_rma_textures", std::function<void(NetworkState *, const ConVar &, bool, bool)> {[](NetworkState *nw, const ConVar &cv, bool oldVal, bool newVal) -> void {
		//static_cast<msys::CMaterialManager&>(static_cast<ClientState*>(nw)->GetMaterialManager()).SetDownscaleImportedRMATextures(newVal);
	}});
	conVarMap.RegisterConVar<uint8_t>("render_debug_mode", 0, ConVarFlags::None,
	  "0 = Disabled, 1 = Ambient Occlusion, 2 = Albedo Colors, 3 = Metalness, 4 = Roughness, 5 = Diffuse Lighting, 6 = Normals, 7 = Normal Map, 8 = Reflectance, 9 = IBL Prefilter, 10 = IBL Irradiance, 11 = Emission, 12 = Lightmaps, 13 = Lightmap Uvs, 14 = Unlit, 15 = Show CSM cascades, 16 = Shadow Map Depth, 17 = Forward+ Heatmap, 18 = Specular, 19 = Indirect Lightmap, 20 = Dominant Lightmap.");
	conVarMap.RegisterConVar<bool>("render_enable_verbose_output", false, ConVarFlags::None, "Enables or disables verbose rendering output.");
	conVarMap.RegisterConVar<bool>("render_ibl_enabled", true, ConVarFlags::Archive, "Enables or disables image-based lighting.");
	conVarMap.RegisterConVar<bool>("render_dynamic_lighting_enabled", true, ConVarFlags::Archive, "Enables or disables dynamic lighting.");
	conVarMap.RegisterConVar<bool>("render_dynamic_shadows_enabled", true, ConVarFlags::Archive, "Enables or disables dynamic shadows.");
	conVarMap.RegisterConVar<std::string>("render_api", "vulkan", ConVarFlags::Archive | ConVarFlags::Replicated, "The underlying rendering API to use.", "<renderApi>", [](const std::string &arg, std::vector<std::string> &autoCompleteOptions) {
		auto renderAPIs = pragma::rendering::get_available_graphics_apis();
		auto it = renderAPIs.begin();
		std::vector<std::string_view> similarCandidates {};
		ustring::gather_similar_elements(
		  arg,
		  [&it, &renderAPIs]() -> std::optional<std::string_view> {
			  if(it == renderAPIs.end())
				  return {};
			  auto &name = *it;
			  ++it;
			  return name;
		  },
		  similarCandidates, 15);

		autoCompleteOptions.reserve(similarCandidates.size());
		for(auto &candidate : similarCandidates) {
			auto strOption = std::string {candidate};
			ufile::remove_extension_from_filename(strOption);
			autoCompleteOptions.push_back(strOption);
		}
	});
	conVarMap.RegisterConCommand(
	  "render_api_info",
	  [this](NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  auto &renderAPI = GetRenderAPI();
		  auto &context = GetRenderContext();
		  Con::cout << "Active render API: " << renderAPI << " (" << context.GetAPIAbbreviation() << ")" << Con::endl;
	  },
	  ConVarFlags::None, "Prints information about the current render API to the console.");
	conVarMap.RegisterConCommand(
	  "debug_render_stats",
	  [this](NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  std::unordered_map<std::string, pragma::console::CommandOption> commandOptions {};
		  pragma::console::parse_command_options(argv, commandOptions);
		  auto full = util::to_boolean(pragma::console::get_command_option_parameter_value(commandOptions, "full", "0"));
		  debug_render_stats(true, full, true, false);
	  },
	  ConVarFlags::None, "Prints information about the next frame.");
	conVarMap.RegisterConVar<bool>("render_multithreaded_rendering_enabled", true, ConVarFlags::Archive, "Enables or disables multi-threaded rendering. Some renderers (like OpenGL) don't support multi-threaded rendering and will ignore this flag.");
	conVarMap.RegisterConVarCallback("render_multithreaded_rendering_enabled", std::function<void(NetworkState *, const ConVar &, bool, bool)> {[this](NetworkState *nw, const ConVar &cv, bool, bool enabled) -> void { GetRenderContext().SetMultiThreadedRenderingEnabled(enabled); }});
	conVarMap.RegisterConVarCallback("render_enable_verbose_output", std::function<void(NetworkState *, const ConVar &, bool, bool)> {[this](NetworkState *nw, const ConVar &cv, bool, bool enabled) -> void { pragma::rendering::VERBOSE_RENDER_OUTPUT_ENABLED = enabled; }});
	conVarMap.RegisterConCommand(
	  "crash",
	  [this](NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  Con::cwar << "Crash command has been invoked. Crashing intentionally..." << Con::endl;
		  if(!argv.empty() && argv.front() == "exception") {
			  throw std::runtime_error {"Crash!"};
			  return;
		  }
		  volatile int *a = reinterpret_cast<volatile int *>(NULL);
		  *a = 1;
	  },
	  ConVarFlags::None, "Forces the engine to crash.");
	conVarMap.RegisterConCommand(
	  "debug_render_memory_budget",
	  [this](NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  auto budget = GetRenderContext().DumpMemoryBudget();
		  if(!budget.has_value()) {
			  Con::cout << "No memory budget information available!" << Con::endl;
			  return;
		  }
		  Con::cout << *budget << Con::endl;
	  },
	  ConVarFlags::None, "Prints information about the current GPU memory budget.");
	conVarMap.RegisterConCommand(
	  "debug_render_memory_stats",
	  [this](NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  auto stats = GetRenderContext().DumpMemoryStats();
		  if(!stats.has_value()) {
			  Con::cout << "No memory stats information available!" << Con::endl;
			  return;
		  }
		  Con::cout << *stats << Con::endl;
	  },
	  ConVarFlags::None, "Prints statistics about the current GPU memory usage.");
	conVarMap.RegisterConCommand(
	  "debug_dump_shader_code",
	  [this](NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  if(argv.empty()) {
			  Con::cwar << "No shader specified!" << Con::endl;
			  return;
		  }
		  auto &shaderName = argv.front();
		  auto shader = GetShader(shaderName);
		  if(shader.expired()) {
			  Con::cwar << "WARNING:: Shader '" << shaderName << "' is invalid!" << Con::endl;
			  return;
		  }
		  std::vector<std::string> glslCodePerStage;
		  std::vector<prosper::ShaderStage> glslCodeStages;
		  std::string infoLog, debugInfoLog;
		  prosper::ShaderStage errStage;
		  auto result = GetRenderContext().GetParsedShaderSourceCode(*shader, glslCodePerStage, glslCodeStages, infoLog, debugInfoLog, errStage);
		  if(result == false) {
			  Con::cwar << "Parsing shader '" << shaderName << "' has failed:" << Con::endl;
			  Con::cwar << "Info Log: " << infoLog << Con::endl;
			  Con::cwar << "Debug info Log: " << debugInfoLog << Con::endl;
			  Con::cwar << "Stage: " << prosper::util::to_string(errStage) << Con::endl;
			  return;
		  }
		  std::string path = "shader_dump/" + shaderName + "/";
		  FileManager::CreatePath(path.c_str());
		  for(auto i = decltype(glslCodeStages.size()) {0u}; i < glslCodeStages.size(); ++i) {
			  auto &glslCode = glslCodePerStage[i];
			  auto stage = glslCodeStages[i];
			  std::string stageName;
			  switch(stage) {
			  case prosper::ShaderStage::Compute:
				  stageName = "compute";
				  break;
			  case prosper::ShaderStage::Fragment:
				  stageName = "fragment";
				  break;
			  case prosper::ShaderStage::Geometry:
				  stageName = "geometry";
				  break;
			  case prosper::ShaderStage::TessellationControl:
				  stageName = "tessellation_control";
				  break;
			  case prosper::ShaderStage::TessellationEvaluation:
				  stageName = "tessellation_evaluation";
				  break;
			  case prosper::ShaderStage::Vertex:
				  stageName = "vertex";
				  break;
			  }
			  static_assert(umath::to_integral(prosper::ShaderStage::Count) == 6);
			  auto stageFileName = path + stageName + ".gls";
			  auto f = FileManager::OpenFile<VFilePtrReal>(stageFileName.c_str(), "w");
			  if(f) {
				  f->WriteString(glslCode);
				  f = nullptr;
			  }
			  else
				  Con::cwar << "Unable to write file '" << stageFileName << "'!" << Con::endl;
		  }
		  Con::cout << "Done! Written shader files to '" << path << "'!" << Con::endl;
	  },
	  ConVarFlags::None, "Dumps the glsl code for the specified shader.");
	conVarMap.RegisterConCommand(
	  "debug_dump_render_queues", [this](NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float) { g_dumpRenderQueues = true; }, ConVarFlags::None, "Prints all render queues for the next frame to the console.");
	conVarMap.RegisterConVar<bool>("debug_hide_gui", false, ConVarFlags::None, "Disables GUI rendering.");

	conVarMap.RegisterConVar<bool>("render_vsync_enabled", true, ConVarFlags::Archive, "Enables or disables vsync. OpenGL only.");
	conVarMap.RegisterConVarCallback("render_vsync_enabled", std::function<void(NetworkState *, const ConVar &, bool, bool)> {[this](NetworkState *nw, const ConVar &cv, bool oldVal, bool newVal) -> void { GetRenderContext().GetWindow()->SetVSyncEnabled(newVal); }});

	conVarMap.RegisterConVar<std::string>("audio_api", "fmod", ConVarFlags::Archive | ConVarFlags::Replicated, "The underlying audio API to use.", "<audioApi>", [](const std::string &arg, std::vector<std::string> &autoCompleteOptions) {
		auto audioAPIs = pragma::audio::get_available_audio_apis();
		auto it = audioAPIs.begin();
		std::vector<std::string_view> similarCandidates {};
		ustring::gather_similar_elements(
		  arg,
		  [&it, &audioAPIs]() -> std::optional<std::string_view> {
			  if(it == audioAPIs.end())
				  return {};
			  auto &name = *it;
			  ++it;
			  return name;
		  },
		  similarCandidates, 15);

		autoCompleteOptions.reserve(similarCandidates.size());
		for(auto &candidate : similarCandidates) {
			auto strOption = std::string {candidate};
			ufile::remove_extension_from_filename(strOption);
			autoCompleteOptions.push_back(strOption);
		}
	});

	conVarMap.RegisterConVar<uint32_t>("render_instancing_threshold", 2, ConVarFlags::Archive, "The threshold at which to start instancing entities if instanced rendering is enabled (render_instancing_threshold). Must not be lower than 2!", "[2,inf]");
	conVarMap.RegisterConVar<bool>("render_instancing_enabled", false, ConVarFlags::Archive, "Enables or disables instanced rendering.");
	conVarMap.RegisterConVar<uint32_t>("render_queue_worker_thread_count", 3, ConVarFlags::Archive, "Number of threads to use for generating render queues.", "[1,10]");
	conVarMap.RegisterConVar<uint32_t>("render_queue_entities_per_worker_job", 5, ConVarFlags::Archive, "Number of entities for each job processed by a worker thread.", "[1,50]");
	conVarMap.RegisterConVar<uint32_t>("render_queue_worker_jobs_per_batch", 2, ConVarFlags::Archive, "Number of worker jobs to accumulate in a batch before assigning a worker.", "[0,10]");

	conVarMap.RegisterConCommand(
	  "debug_textures",
	  [this](NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  auto &texManager = static_cast<msys::CMaterialManager &>(static_cast<ClientState *>(GetClientState())->GetMaterialManager()).GetTextureManager();
		  std::vector<std::shared_ptr<Texture>> textures;
		  auto &cache = texManager.GetCache();
		  textures.reserve(cache.size());
		  for(auto &pair : cache) {
			  auto asset = texManager.GetAsset(pair.second);
			  if(!asset)
				  continue;
			  textures.push_back(msys::TextureManager::GetAssetObject(*asset));
		  }
		  std::vector<prosper::DeviceSize> textureSizes;
		  std::vector<size_t> sortedIndices {};
		  textureSizes.reserve(textures.size());
		  sortedIndices.reserve(textures.size());

		  auto fGetImageSize = [](prosper::IImage &img) -> prosper::DeviceSize {
			  auto size = img.GetStorageSize();
			  if(size.has_value()) {
				  auto *buf = img.GetMemoryBuffer();
				  if(buf)
					  size = buf->GetSize();
			  }
			  return size.has_value() ? *size : 0;
		  };

		  for(auto &tex : textures) {
			  prosper::DeviceSize size = 0;
			  auto &vkTex = tex->GetVkTexture();
			  if(vkTex) {
				  auto &img = vkTex->GetImage();
				  size = fGetImageSize(img);
			  }
			  sortedIndices.push_back(textureSizes.size());
			  textureSizes.push_back(size);
		  }
		  std::sort(sortedIndices.begin(), sortedIndices.end(), [&textureSizes](size_t idx0, size_t idx1) { return textureSizes[idx0] > textureSizes[idx1]; });

		  Con::cout << textures.size() << " textures are currently loaded:" << Con::endl;
		  Con::cout << std::left << std::setw(35) << "Name" << std::setw(10) << "Use Count" << std::setw(12) << "Resolution";
		  Con::cout << std::setw(10) << "Layers" << std::setw(10) << "Mipmaps" << std::setw(10) << "Tiling" << std::setw(22) << "Format";
		  Con::cout << std::setw(12) << "Size" << std::setw(10) << "Last Used" << std::setw(30) << "Filename" << Con::endl;

		  auto fPrintImageInfo = [&fGetImageSize](const std::string &fileName, prosper::IImage &img, bool perfWarnings = true) {
			  auto &context = img.GetContext();
			  auto useCount = img.shared_from_this().use_count() - 1;
			  auto imgName = img.GetDebugName();
			  ustring::truncate_string(imgName, 35);
			  Con::cout << std::left << std::setw(35) << imgName;

			  if(useCount == 0)
				  util::set_console_color(util::ConsoleColorFlags::Intensity | util::ConsoleColorFlags::Red);
			  Con::cout << std::setw(10) << useCount;
			  if(useCount == 0)
				  util::reset_console_color();

			  std::string res = std::to_string(img.GetWidth()) + "x" + std::to_string(img.GetHeight());
			  Con::cout << std::setw(12) << res;
			  Con::cout << std::setw(10) << img.GetLayerCount();

			  auto numMipmaps = img.GetMipmapCount();
			  if(numMipmaps <= 1 && perfWarnings)
				  util::set_console_color(util::ConsoleColorFlags::Intensity | util::ConsoleColorFlags::Red);
			  Con::cout << std::setw(10) << numMipmaps;
			  if(numMipmaps <= 1 && perfWarnings)
				  util::reset_console_color();

			  auto tiling = img.GetTiling();
			  auto optimal = tiling == prosper::ImageTiling::Optimal;
			  if(!optimal)
				  util::set_console_color(util::ConsoleColorFlags::Intensity | util::ConsoleColorFlags::Red);
			  Con::cout << std::setw(10) << prosper::util::to_string(tiling);
			  if(!optimal)
				  util::reset_console_color();

			  auto format = img.GetFormat();
			  auto isCompressed = prosper::util::is_compressed_format(format);
			  if(!isCompressed && perfWarnings)
				  util::set_console_color(util::ConsoleColorFlags::Intensity | util::ConsoleColorFlags::Red);
			  Con::cout << std::setw(22) << prosper::util::to_string(format);
			  if(!isCompressed && perfWarnings)
				  util::reset_console_color();

			  Con::cout << std::setw(12) << util::get_pretty_bytes(fGetImageSize(img));

			  if(context.IsValidationEnabled() == false)
				  Con::cout << std::setw(10) << "n/a";
			  else {
				  auto time = context.GetLastUsageTime(img);
				  if(time.has_value() == false)
					  util::set_console_color(util::ConsoleColorFlags::Intensity | util::ConsoleColorFlags::Red);
				  Con::cout << std::setw(10);
				  if(time.has_value()) {
					  auto t = std::chrono::steady_clock::now();
					  auto dt = t - *time;
					  Con::cout << util::get_pretty_duration(std::chrono::duration_cast<std::chrono::milliseconds>(dt).count()) << " ago";
				  }
				  else
					  Con::cout << "Never";
				  if(time.has_value() == false)
					  util::reset_console_color();
			  }

			  Con::cout << std::setw(30) << fileName << Con::endl;

			  /*auto deviceLocal = umath::is_flag_set(img.GetCreateInfo().memoryFeatures, prosper::MemoryFeatureFlags::DeviceLocal);
			  if(!deviceLocal) {
				  util::set_console_color(util::ConsoleColorFlags::Intensity | util::ConsoleColorFlags::Red);
				  Con::cout << "\tPerformance Warning: Image memory is not device local!" << Con::endl;
				  util::reset_console_color();
			  }*/
		  };

		  prosper::DeviceSize totalSize = 0;
		  for(auto idx : sortedIndices) {
			  auto &tex = textures[idx];
			  auto &filePath = tex->GetName();
			  auto &vkTex = tex->GetVkTexture();
			  if(vkTex)
				  fPrintImageInfo(filePath, vkTex->GetImage());
			  // else
			  //	  Con::cout << "\tNULL" << Con::endl;

			  totalSize += textureSizes[idx];
		  }
		  Con::cout << "Total memory: " << util::get_pretty_bytes(totalSize) << Con::endl << Con::endl;

		  auto *client = GetClientState();
		  auto *game = client ? static_cast<CGame *>(client->GetGameState()) : nullptr;
		  if(game) {
			  auto cIt = EntityCIterator<pragma::CRasterizationRendererComponent> {*game};
			  Con::cout << "Number of scenes: " << cIt.GetCount() << Con::endl;
			  for(auto &rast : cIt) {
				  Con::cout << "Renderer " << rast.GetEntity().GetName() << ":" << Con::endl;
				  auto &hdrInfo = rast.GetHDRInfo();
				  std::unordered_set<prosper::IImage *> images;
				  auto fAddTex = [&images](const std::shared_ptr<prosper::Texture> &tex) {
					  if(tex == nullptr)
						  return;
					  images.insert(&tex->GetImage());
				  };
				  auto fAddRt = [&fAddTex](const std::shared_ptr<prosper::RenderTarget> &rt) {
					  if(rt == nullptr)
						  return;
					  auto n = rt->GetAttachmentCount();
					  for(auto i = decltype(n) {0u}; i < n; ++i) {
						  auto *tex = rt->GetTexture(i);
						  if(tex == nullptr)
							  continue;
						  fAddTex(tex->shared_from_this());
					  }
				  };
				  fAddRt(hdrInfo.sceneRenderTarget);
				  fAddTex(hdrInfo.bloomTexture);
				  fAddRt(hdrInfo.bloomBlurRenderTarget);
				  if(hdrInfo.bloomBlurSet) {
					  fAddRt(hdrInfo.bloomBlurSet->GetStagingRenderTarget());
					  fAddRt(hdrInfo.bloomBlurSet->GetFinalRenderTarget());
				  }
				  // fAddRt(hdrInfo.hdrPostProcessingRenderTarget);
				  fAddRt(hdrInfo.toneMappedRenderTarget);
				  fAddRt(hdrInfo.toneMappedPostProcessingRenderTarget);
				  fAddRt(hdrInfo.ssaoInfo.renderTarget);
				  fAddRt(hdrInfo.ssaoInfo.renderTargetBlur);
				  fAddTex(hdrInfo.prepass.textureNormals);
				  fAddTex(hdrInfo.prepass.textureDepth);
				  // fAddTex(hdrInfo.prepass.textureDepthSampled);
				  fAddRt(hdrInfo.prepass.renderTarget);
				  /*auto &glowInfo = rast->GetGlowInfo();
				fAddRt(glowInfo.renderTarget);
				if(glowInfo.blurSet)
				{
					fAddRt(glowInfo.blurSet->GetStagingRenderTarget());
					fAddRt(glowInfo.blurSet->GetFinalRenderTarget());
				}*/

				  Con::cout << images.size() << " images:" << Con::endl;
				  prosper::DeviceSize totalSceneSize = 0;
				  for(auto &img : images) {
					  fPrintImageInfo("<implementation>", *img, false);
					  totalSceneSize += fGetImageSize(*img);
				  }
				  Con::cout << "Total scene image size: " << util::get_pretty_bytes(totalSceneSize) << Con::endl << Con::endl;
			  }
		  }

		  auto &imgBufs = GetRenderContext().GetDeviceImageBuffers();
		  totalSize = 0;
		  for(auto &imgBuf : imgBufs)
			  totalSize += imgBuf->GetSize() - imgBuf->GetFreeSize();
		  Con::cout << "Total device image memory: " << util::get_pretty_bytes(totalSize) << Con::endl;
	  },
	  ConVarFlags::None, "Prints information about the currently loaded textures.");
#if LUA_ENABLE_RUN_GUI == 1
	conVarMap.RegisterConCommand(
	  "lua_exec_gui",
	  [](NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  if(argv.empty())
			  return;
		  Lua::set_ignore_include_cache(true);
		  //client->LoadGUILuaFile(argv.front());
		  Lua::set_ignore_include_cache(false);
	  },
	  ConVarFlags::None, "Opens and executes a lua-file on the GUI state.",
	  [](const std::string &arg, std::vector<std::string> &autoCompleteOptions) {
		  std::vector<std::string> resFiles;
		  auto path = Lua::SCRIPT_DIRECTORY_SLASH + arg;
		  FileManager::FindFiles((path + "*." + Lua::FILE_EXTENSION).c_str(), &resFiles, nullptr);
		  FileManager::FindFiles((path + "*." + Lua::FILE_EXTENSION_PRECOMPILED).c_str(), &resFiles, nullptr);
		  autoCompleteOptions.reserve(resFiles.size());
		  for(auto &mapName : resFiles) {
			  auto fullPath = path.substr(4) + mapName;
			  ustring::replace(fullPath, "\\", "/");
			  autoCompleteOptions.push_back(fullPath);
		  }
	  });
#endif
	conVarMap.RegisterConCommand(
	  "asset_clear_unused_textures", [this](NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float) { ClearUnusedAssets(pragma::asset::Type::Texture, true); }, ConVarFlags::None, "Clears all unused textures from memory.");
	conVarMap.RegisterConCommand(
	  "vr_preinitialize",
	  [this](NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  auto *cl = static_cast<ClientState *>(GetClientState());
		  if(!cl)
			  return;
		  std::string err;
		  auto lib = cl->InitializeLibrary("openvr/pr_openvr", &err, cl->GetLuaState());
		  if(!lib) {
			  Con::cwar << "Unable to preinitialize VR: " << err << Con::endl;
			  return;
		  }
		  auto *isHmdPresent = lib->FindSymbolAddress<bool (*)()>("is_hmd_present");
		  auto *preInit = lib->FindSymbolAddress<void (*)()>("preinitialize_openvr");
		  if(!isHmdPresent || !preInit) {
			  Con::cwar << "Required VR functions not found in openvr module!" << Con::endl;
			  return;
		  }
		  if(!isHmdPresent()) {
			  Con::cwar << "VR HMD could not be found!" << Con::endl;
			  if(argv.empty() || !util::to_boolean(argv.front()))
				  return;
		  }
		  preInit();
	  },
	  ConVarFlags::None, "Pre-initializes openvr.");
	conVarMap.RegisterConCommand(
	  "locale_localize",
	  [this](NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  if(argv.size() < 4) {
			  Con::cwar << "Insufficient arguments supplied!" << Con::endl;
			  std::vector<std::string> files;
			  filemanager::find_files("scripts/localization/en/texts/*.txt", &files, nullptr);
			  std::sort(files.begin(), files.end());
			  Con::cout << "Available groups: " << Con::endl;
			  for(auto &f : files) {
				  ufile::remove_extension_from_filename(f);
				  Con::cout << f << Con::endl;
			  }
			  return;
		  }
		  auto category = argv[0];
		  auto lan = argv[1];
		  auto identifier = argv[2];
		  auto text = argv[3];
		  Con::cout << "Localizing '" << identifier << "' in category '" << category << "' for language '" << lan << "' as '" << text << "'..." << Con::endl;
		  auto res = Locale::Localize(identifier, lan, category, text);
		  if(res)
			  Con::cout << "Done!" << Con::endl;
		  else
			  Con::cwar << "Localization failed!" << Con::endl;
	  },
	  ConVarFlags::None, "Adds the specified text to the localization files. Usage: locale_localize <group> <language> <textIdentifier> <localizedText>");
	conVarMap.RegisterConCommand(
	  "debug_start_lua_debugger_server_cl",
	  [this](NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float) {
		  auto *l = state->GetLuaState();
		  if(!l) {
			  Con::cwar << "Unable to start debugger server: No active Lua state!" << Con::endl;
			  return;
		  }
		  Lua::util::start_debugger_server(l);
	  },
	  ConVarFlags::None, "Starts the Lua debugger server for the clientside lua state.");
}
