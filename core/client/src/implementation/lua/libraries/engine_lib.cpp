// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :scripting.lua.libraries.engine;
import util_zip;
import :client_state;
import :engine;
import :entities.components.particle_system;
import :game;

std::shared_ptr<const pragma::gui::FontInfo> Lua::engine::create_font(lua::State *l, const std::string &identifier, const std::string &fontSetName, pragma::FontSetFlag features, uint32_t size, bool reload)
{
	auto *fontSet = pragma::get_cengine()->FindFontSet(fontSetName);
	if(!fontSet)
		return nullptr;
	auto *fontFileData = fontSet->FindFontFileCandidate(features);
	if(!fontFileData)
		return nullptr;
	if(fontFileData->fontSizeAdjustment)
		size += *fontFileData->fontSizeAdjustment;
	pragma::gui::FontSettings settings {};
	settings.fontSize = size;
	return pragma::gui::FontManager::LoadFont(identifier.c_str(), fontFileData->fileName, settings, reload);
}
std::shared_ptr<const pragma::gui::FontInfo> Lua::engine::create_font(lua::State *l, const std::string &identifier, const std::string &fontSetName, pragma::FontSetFlag features, uint32_t size) { return create_font(l, identifier, fontSetName, features, size, false); }
std::shared_ptr<const pragma::gui::FontInfo> Lua::engine::get_font(lua::State *l, const std::string &identifier) { return pragma::gui::FontManager::GetFont(identifier); }

void Lua::engine::register_library(lua::State *l)
{
	auto modEngine = luabind::module_(l, "engine");
	modEngine[(luabind::def("create_font", static_cast<std::shared_ptr<const pragma::gui::FontInfo> (*)(lua::State *, const std::string &, const std::string &, pragma::FontSetFlag, uint32_t, bool)>(create_font)),
	  luabind::def("create_font", static_cast<std::shared_ptr<const pragma::gui::FontInfo> (*)(lua::State *, const std::string &, const std::string &, pragma::FontSetFlag, uint32_t)>(create_font)), luabind::def("get_font", get_font),
	  luabind::def("set_fixed_frame_delta_time_interpretation", set_fixed_frame_delta_time_interpretation), luabind::def("clear_fixed_frame_delta_time_interpretation", clear_fixed_frame_delta_time_interpretation),
	  luabind::def("set_tick_delta_time_tied_to_frame_rate", set_tick_delta_time_tied_to_frame_rate), luabind::def("get_window_resolution", get_window_resolution), luabind::def("get_render_resolution", get_render_resolution),
	  luabind::def("get_staging_render_target", get_staging_render_target), luabind::def("get_current_frame_index", &get_current_frame_index), luabind::def("get_default_font_set_name", &pragma::CEngine::GetDefaultFontSetName),
	  luabind::def(
	    "get_font_sets", +[]() -> std::vector<std::string> {
		    std::vector<std::string> fontSets;
		    auto &fontSetMap = pragma::get_cengine()->GetFontSets();
		    fontSets.reserve(fontSetMap.size());
		    for(auto &[name, fontSet] : fontSetMap)
			    fontSets.push_back(name);
		    return fontSets;
	    }))];
	modEngine[luabind::def("toggle_console", &pragma::Engine::ToggleConsole)];
	modEngine[luabind::def(
	  "generate_info_dump", +[](const std::string &baseName) -> std::optional<std::string> {
		  std::string zipFileName;
		  std::string err;
		  auto zipFile = pragma::Engine::GenerateEngineDump(baseName, zipFileName, err);
		  if(!zipFile)
			  return {};
		  zipFile = nullptr;
		  return zipFileName;
	  })];

	register_shared_functions(l, modEngine);

	RegisterLibraryEnums(l, "engine",
	  {{"FONT_FEATURE_FLAG_NONE", pragma::math::to_integral(pragma::FontSetFlag::None)}, {"FONT_FEATURE_FLAG_BOLD_BIT", pragma::math::to_integral(pragma::FontSetFlag::Bold)}, {"FONT_FEATURE_FLAG_ITALIC_BIT", pragma::math::to_integral(pragma::FontSetFlag::Italic)},
	    {"FONT_FEATURE_FLAG_MONO_BIT", pragma::math::to_integral(pragma::FontSetFlag::Mono)}, {"FONT_FEATURE_FLAG_SERIF_BIT", pragma::math::to_integral(pragma::FontSetFlag::Serif)}, {"FONT_FEATURE_FLAG_SANS_BIT", pragma::math::to_integral(pragma::FontSetFlag::Sans)}});
}

Vector2i Lua::engine::get_text_size(lua::State *l, const std::string &text, const std::string &font)
{
	auto info = pragma::gui::FontManager::GetFont(font);
	if(info == nullptr)
		return {0, 0};
	int w = 0;
	int h = 0;
	pragma::gui::FontManager::GetTextSize(text, 0u, info.get(), &w, &h);
	return ::Vector2i {w, h};
}

Vector2i Lua::engine::get_text_size(lua::State *l, const std::string &text, const pragma::gui::FontInfo &font)
{
	int w = 0;
	int h = 0;
	pragma::gui::FontManager::GetTextSize(text, 0u, &font, &w, &h);
	return ::Vector2i {w, h};
}

std::pair<size_t, size_t> Lua::engine::get_truncated_text_length(lua::State *l, const std::string &text, const std::string &font, uint32_t maxWidth)
{
	auto info = pragma::gui::FontManager::GetFont(font);
	if(info == nullptr)
		return {0, 0};
	return get_truncated_text_length(l, text, *info, maxWidth);
}

std::pair<size_t, size_t> Lua::engine::get_truncated_text_length(lua::State *l, const std::string &text, const pragma::gui::FontInfo &font, uint32_t maxWidth)
{
	pragma::string::Utf8String uText {text};
	uint32_t offset = 0;
	size_t numChars = 0;
	uint32_t idx = 0;
	for(auto it = uText.begin(); it != uText.end(); ++it) {
		int w = 0;
		int h = 0;
		pragma::gui::FontManager::GetTextSize(*it, idx, &font, &w, &h);
		if(offset + w > maxWidth) {
			numChars = idx;
			break;
		}
		offset += w;
		++numChars;
		++idx;
	}

	return {numChars, offset};
}

void Lua::engine::precache_material(lua::State *l, const std::string &mat) { pragma::get_client_state()->PrecacheMaterial(mat.c_str()); }

void Lua::engine::precache_model(lua::State *l, const std::string &mdl) { pragma::get_cgame()->PrecacheModel(mdl); }

std::shared_ptr<prosper::Texture> Lua::engine::load_texture(lua::State *l, const std::string &name, pragma::util::AssetLoadFlags loadFlags)
{
	auto &texManager = static_cast<pragma::material::CMaterialManager &>(pragma::get_client_state()->GetMaterialManager()).GetTextureManager();
	auto tex = texManager.LoadAsset(name, loadFlags);
	if(tex == nullptr || std::static_pointer_cast<pragma::material::Texture>(tex)->HasValidVkTexture() == false)
		return nullptr;
	return std::static_pointer_cast<pragma::material::Texture>(tex)->GetVkTexture();
}
std::shared_ptr<prosper::Texture> Lua::engine::load_texture(lua::State *l, const std::string &name) { return load_texture(l, name, pragma::util::AssetLoadFlags::None); }

static std::optional<std::string> get_extension(const LFile &file)
{
	auto *fp = const_cast<LFile &>(file).GetHandle().get();
	if(!fp)
		return {};
	auto path = fp->GetFileName();
	if(path.has_value() == false)
		return {};
	std::string ext;
	return ufile::get_extension(*path, &ext) ? ext : std::optional<std::string> {};
}
std::shared_ptr<prosper::Texture> Lua::engine::load_texture(lua::State *l, const LFile &file, const std::string &cacheName, pragma::util::AssetLoadFlags loadFlags)
{
	auto &lf = Lua::Check<LFile>(l, 1);
	auto ext = get_extension(file);
	if(!ext.has_value())
		return nullptr;
	auto &texManager = static_cast<pragma::material::CMaterialManager &>(pragma::get_client_state()->GetMaterialManager()).GetTextureManager();
	auto f = std::make_unique<ufile::FileWrapper>(const_cast<LFile &>(file).GetHandle());
	auto tex = texManager.LoadAsset("", std::move(f), *ext, std::make_unique<pragma::material::TextureLoadInfo>(loadFlags | pragma::util::AssetLoadFlags::DontCache));
	if(tex == nullptr || std::static_pointer_cast<pragma::material::Texture>(tex)->HasValidVkTexture() == false)
		return nullptr;
	return std::static_pointer_cast<pragma::material::Texture>(tex)->GetVkTexture();
}
std::shared_ptr<prosper::Texture> Lua::engine::load_texture(lua::State *l, const LFile &file, const std::string &cacheName) { return load_texture(l, file, cacheName, pragma::util::AssetLoadFlags::None); }
std::shared_ptr<prosper::Texture> Lua::engine::load_texture(lua::State *l, const LFile &file, pragma::util::AssetLoadFlags loadFlags)
{
	auto &lf = Lua::Check<LFile>(l, 1);
	auto ext = get_extension(file);
	if(!ext.has_value())
		return nullptr;
	auto &texManager = static_cast<pragma::material::CMaterialManager &>(pragma::get_client_state()->GetMaterialManager()).GetTextureManager();
	auto f = std::make_unique<ufile::FileWrapper>(const_cast<LFile &>(file).GetHandle());
	auto tex = texManager.LoadAsset("", std::move(f), *ext, std::make_unique<pragma::material::TextureLoadInfo>(loadFlags));
	if(tex == nullptr || std::static_pointer_cast<pragma::material::Texture>(tex)->HasValidVkTexture() == false)
		return nullptr;
	return std::static_pointer_cast<pragma::material::Texture>(tex)->GetVkTexture();
}
std::shared_ptr<prosper::Texture> Lua::engine::load_texture(lua::State *l, const LFile &file) { return load_texture(l, file, pragma::util::AssetLoadFlags::None); }

pragma::material::Material *Lua::engine::load_material(lua::State *l, const std::string &mat, bool reload, bool loadInstantly) { return pragma::get_client_state()->LoadMaterial(mat, nullptr, reload, loadInstantly); }
pragma::material::Material *Lua::engine::load_material(lua::State *l, const std::string &mat, bool reload) { return load_material(l, mat, reload, true); }
pragma::material::Material *Lua::engine::load_material(lua::State *l, const std::string &mat) { return load_material(l, mat, false, true); }

pragma::material::Material *Lua::asset_client::get_error_material() { return pragma::get_client_state()->GetMaterialManager().GetErrorMaterial(); }

void Lua::asset_client::register_library(Interface &lua, luabind::module_ &modAsset)
{
	modAsset[(luabind::def("create_material", static_cast<std::shared_ptr<pragma::material::Material> (*)(const std::string &, const std::string &)>(create_material)),
	  luabind::def("create_material", static_cast<std::shared_ptr<pragma::material::Material> (*)(const std::string &)>(create_material)), luabind::def("create_material", static_cast<std::shared_ptr<pragma::material::Material> (*)(const ::udm::AssetData &)>(create_material)),
	  luabind::def("get_material", static_cast<pragma::material::Material *(*)(const std::string &)>(get_material)), luabind::def("precache_model", static_cast<void (*)(lua::State *, const std::string &)>(engine::precache_model)),
	  luabind::def("precache_material", static_cast<void (*)(lua::State *, const std::string &)>(engine::precache_material)), luabind::def("get_error_material", get_error_material))];
}

std::shared_ptr<pragma::material::Material> Lua::asset_client::create_material(const std::string &identifier, const std::string &shader)
{

	auto mat = pragma::get_client_state()->CreateMaterial(identifier, shader);
	if(!mat)
		return nullptr;
	return mat->shared_from_this();
}
std::shared_ptr<pragma::material::Material> Lua::asset_client::create_material(const std::string &shader)
{
	auto mat = pragma::get_client_state()->CreateMaterial(shader);
	if(!mat)
		return nullptr;
	return mat->shared_from_this();
}
std::shared_ptr<pragma::material::Material> Lua::asset_client::create_material(const ::udm::AssetData &data)
{
	std::string err;
	auto mat = pragma::get_client_state()->GetMaterialManager().CreateMaterial(data, err);
	if(!mat)
		return nullptr;
	return mat->shared_from_this();
}
pragma::material::Material *Lua::asset_client::get_material(const std::string &identifier)
{
	auto *asset = pragma::get_client_state()->GetMaterialManager().FindCachedAsset(identifier);
	return asset ? pragma::material::CMaterialManager::GetAssetObject(*asset).get() : nullptr;
}

int Lua::engine::create_particle_system(lua::State *l)
{
	std::unordered_map<std::string, std::string> values;
	std::vector<std::string> children;
	pragma::ecs::CParticleSystemComponent *particle;
	auto bRecordKeyvalues = false;
	if(IsSet(l, 3))
		bRecordKeyvalues = CheckBool(l, 3);
	if(IsString(l, 1)) {
		std::string name = CheckString(l, 1);
		pragma::ecs::CParticleSystemComponent *parent = nullptr;
		if(IsSet(l, 2)) {
			auto &hParent = Lua::Check<pragma::ecs::CParticleSystemComponent>(l, 2);
			parent = &hParent;
		}
		particle = pragma::ecs::CParticleSystemComponent::Create(name, parent, bRecordKeyvalues);
	}
	else {
		struct ParticleData {
			ParticleData(const std::string &n, const luabind::object &t) : name(n), table(t) {}
			std::string name;
			luabind::object table;
		};
		std::vector<std::unique_ptr<ParticleData>> initializers;
		std::vector<std::unique_ptr<ParticleData>> operators;
		std::vector<std::unique_ptr<ParticleData>> renderers;

		CheckTable(l, 1);
		PushNil(l);
		while(GetNextPair(l, 1) != 0) {
			PushValue(l, -2);
			std::string key = ToString(l, -3);
			if(!IsTable(l, -2)) {
				std::string val = ToString(l, -2);
				pragma::string::to_lower(key);
				values[key] = val;
				RemoveValue(l, -3);
				RemoveValue(l, -2);
			}
			else if(key == "initializers" || key == "operators" || key == "renderers") {
				uint8_t type = (key == "initializers") ? 0 : ((key == "operators") ? 1 : 2);
				CheckTable(l, -2);
				auto t = GetStackTop(l) - 1;
				PushNil(l);                   /* 1 */
				while(GetNextPair(l, t) != 0) /* 2 */
				{
					PushValue(l, -2); /* 3 */
					const int8_t k = -3;
					const int8_t v = -2;
					auto *name = ToString(l, k);
					CheckTable(l, v);
					auto tObj = luabind::object(luabind::from_stack(l, v));
					if(type == 0)
						initializers.push_back(std::make_unique<ParticleData>(name, tObj));
					else if(type == 1)
						operators.push_back(std::make_unique<ParticleData>(name, tObj));
					else
						renderers.push_back(std::make_unique<ParticleData>(name, tObj));
					RemoveValue(l, k); /* 2 */
					RemoveValue(l, v); /* 1 */
				} /* 0 */
				RemoveValue(l, -3);
				RemoveValue(l, -2);
			}
			else if(key == "children") {
				int tchildren = GetStackTop(l) - 1;
				PushNil(l);
				while(GetNextPair(l, tchildren) != 0) {
					if(!IsTable(l, -1)) {
						std::string child = ToString(l, -1);
						pragma::string::to_lower(child);
						children.push_back(child);
						RemoveValue(l, -1);
					}
					else
						Pop(l, 1);
				}
				RemoveValue(l, -3);
				RemoveValue(l, -2);
			}
			else
				Pop(l, 2);
		}
		pragma::ecs::CParticleSystemComponent *parent = nullptr;
		if(IsSet(l, 2)) {
			auto &hParent = Lua::Check<pragma::ecs::CParticleSystemComponent>(l, 2);
			parent = &hParent;
		}
		particle = pragma::ecs::CParticleSystemComponent::Create(values, parent, bRecordKeyvalues);
		if(particle != nullptr) {
			particle->PushLuaObject(l); /* 1 */

			for(auto &initializer : initializers) {
				PushString(l, initializer->name); /* 2 */
				initializer->table.push(l);       /* 3 */
				particle->AddInitializer(initializer->name, pragma::ecs::get_particle_key_values(l, initializer->table));
				Pop(l, 2); /* 1 */
			}
			for(auto &op : operators) {
				PushString(l, op->name); /* 2 */
				op->table.push(l);       /* 3 */
				particle->AddOperator(op->name, pragma::ecs::get_particle_key_values(l, op->table));
				Pop(l, 2); /* 1 */
			}
			for(auto &renderer : renderers) {
				PushString(l, renderer->name); /* 2 */
				renderer->table.push(l);       /* 3 */
				particle->AddRenderer(renderer->name, pragma::ecs::get_particle_key_values(l, renderer->table));
				Pop(l, 2); /* 1 */
			}

			Pop(l, 1); /* 0 */
		}
	}
	if(particle == nullptr)
		return 0;
	for(unsigned int i = 0; i < children.size(); i++)
		pragma::ecs::CParticleSystemComponent::Create(children[i], particle, bRecordKeyvalues);
	particle->PushLuaObject(l);
	return 1;
}

bool Lua::engine::precache_particle_system(lua::State *l, const std::string &particle, bool reload) { return pragma::ecs::CParticleSystemComponent::Precache(particle, reload); }
bool Lua::engine::precache_particle_system(lua::State *l, const std::string &particle) { return precache_particle_system(l, particle, false); }

int Lua::engine::save_particle_system(lua::State *l)
{
	std::string name = CheckString(l, 1);
	CheckTable(l, 2);
	name = pragma::asset::get_normalized_path(name, pragma::asset::Type::ParticleSystem);
	name = "particles/" + name;
	std::string rootPath;
	if(file::validate_write_operation(l, name, rootPath) == false) {
		PushBool(l, false);
		return 1;
	}

	{
		auto t = 2;

		PushInt(l, 1);
		GetTableValue(l, t);
		auto bParticleSystem = Lua::IsType<pragma::util::WeakHandle<pragma::ecs::CParticleSystemComponent>>(l, -1);
		Pop(l, 1);
		if(bParticleSystem) {
			auto numParticleSystems = GetObjectLength(l, t);
			std::vector<pragma::ecs::CParticleSystemComponent *> particleSystems;
			particleSystems.reserve(numParticleSystems);
			std::function<void(const pragma::ecs::CParticleSystemComponent &)> fIncludeChildren = nullptr;
			fIncludeChildren = [&particleSystems, &fIncludeChildren](const pragma::ecs::CParticleSystemComponent &ps) {
				auto &children = ps.GetChildren();
				particleSystems.reserve(particleSystems.size() + children.size());
				for(auto &hChild : children) {
					if(hChild.child.expired())
						continue;
					auto *ps = hChild.child.get();
					if(ps->IsRecordingKeyValues() == false) {
						Con::CWAR << "Cannot save particle system '" << ps->GetParticleSystemName() << "', which wasn't created with the \"record key-values\" flag set! Skipping..." << Con::endl;
						return;
					}
					particleSystems.push_back(hChild.child.get());
					fIncludeChildren(*hChild.child.get());
				}
			};
			for(auto i = decltype(numParticleSystems) {0u}; i < numParticleSystems; ++i) {
				PushInt(l, i + 1u);
				GetTableValue(l, t);
				auto &ps = Lua::Check<pragma::ecs::CParticleSystemComponent>(l, -1);
				if(ps.IsRecordingKeyValues() == false)
					Con::CWAR << "Cannot save particle system '" << ps.GetParticleSystemName() << "', which wasn't created with the \"record key-values\" flag set! Skipping..." << Con::endl;
				else {
					particleSystems.push_back(&ps);
					fIncludeChildren(ps);
				}
				Pop(l, 1);
			}
			if(particleSystems.empty()) {
				Con::CWAR << "No particles to save. Particle file will not be generated!" << Con::endl;
				PushBool(l, false);
				return 1;
			}

			pragma::fs::create_path(ufile::get_path_from_filename(name));
			auto f = pragma::fs::open_file<pragma::fs::VFilePtrReal>(name, pragma::fs::FileMode::Write | pragma::fs::FileMode::Binary);
			if(f != nullptr)
				PushBool(l, pragma::ecs::CParticleSystemComponent::Save(f, particleSystems));
			else
				PushBool(l, false);
			return 1;
		}
	}

	std::unordered_map<std::string, pragma::asset::ParticleSystemData> particles;
	PushValue(l, 2);
	int tparticles = 2;
	PushNil(l);
	while(GetNextPair(l, tparticles) != 0) {
		if(IsTable(l, -1)) {
			PushValue(l, -2);
			pragma::asset::ParticleSystemData data {};
			std::string particle = ToString(l, -3);
			RemoveValue(l, -3);
			PushValue(l, -2);
			int table = GetStackTop(l);
			PushNil(l);
			while(GetNextPair(l, table) != 0) {
				PushValue(l, -2);
				std::string key = ToString(l, -3);
				RemoveValue(l, -3);
				pragma::string::to_lower(key);
				if(key == "initializers" || key == "operators" || key == "renderers") {
					if(IsTable(l, -2)) {
						auto numOperators = GetObjectLength(l, -2);
						PushValue(l, -2);
						int tmod = GetStackTop(l);
						for(auto i = decltype(numOperators) {0u}; i < numOperators; ++i) {
							PushInt(l, i + 1);
							GetTableValue(l, tmod);
							auto tOp = GetStackTop(l);
							CheckTable(l, tOp);

							PushString(l, "operatorType");
							GetTableValue(l, tOp);
							std::string opType = CheckString(l, -1);
							Pop(l, 1);

							std::vector<pragma::asset::ParticleModifierData> modData;
							modData.push_back(pragma::asset::ParticleModifierData {opType});
							int dataIdx = 0;
							char dataType = -1;
							PushNil(l);
							while(GetNextPair(l, tOp) != 0) {
								if(!IsTable(l, -1)) {
									if(dataType == 0)
										Pop(l, 1);
									else {
										dataType = 1;
										PushValue(l, -2);
										std::string modKey = ToString(l, -3);
										std::string modVal = ToString(l, -2);
										RemoveValue(l, -3);
										RemoveValue(l, -2);
										modData[dataIdx].settings.insert(std::unordered_map<std::string, std::string>::value_type(modKey, modVal));
									}
								}
								else if(dataType != 1) {
									dataType = 0;
									if(dataIdx > 0)
										modData.push_back(pragma::asset::ParticleModifierData {opType});
									dataIdx++;
									int tModSubSettings = GetStackTop(l);
									PushNil(l);
									while(GetNextPair(l, tModSubSettings) != 0) {
										PushValue(l, -2);
										std::string modKey = ToString(l, -3);
										std::string modVal = ToString(l, -2);
										RemoveValue(l, -3);
										RemoveValue(l, -2);
										modData[dataIdx - 1].settings.insert(std::unordered_map<std::string, std::string>::value_type(modKey, modVal));
									}
									Pop(l, 1);
								}
								else
									Pop(l, 1);
							}
							Pop(l, 1);
							for(unsigned int i = 0; i < modData.size(); i++) {
								if(modData[i].settings.empty() == false) {
									if(key == "initializers")
										data.initializers.push_back(modData[i]);
									else if(key == "operators")
										data.operators.push_back(modData[i]);
									else if(key == "renderers")
										data.renderers.push_back(modData[i]);
								}
							}
						}
						Pop(l, 1);
						RemoveValue(l, -2);
					}
					else
						RemoveValue(l, -2);
				}
				else if(key == "children") {
					if(IsTable(l, -2)) {
						PushValue(l, -2);
						int tchildren = GetStackTop(l);
						PushNil(l);
						while(GetNextPair(l, tchildren) != 0) {
							CheckTable(l, -1);
							auto tChild = GetStackTop(l);

							pragma::asset::ParticleChildData childData {};

							PushString(l, "childName");
							GetTableValue(l, tChild);
							childData.childName = CheckString(l, -1);
							Pop(l, 1);

							PushString(l, "delay");
							GetTableValue(l, tChild);
							if(IsSet(l, -1))
								childData.delay = CheckNumber(l, -1);
							Pop(l, 1);

							data.children.push_back(childData);

							Pop(l, 1);
						}
						Pop(l, 1);
						RemoveValue(l, -2);
					}
					else
						RemoveValue(l, -2);
				}
				else if(IsTable(l, -2))
					RemoveValue(l, -2);
				else {
					std::string val = ToString(l, -2);
					RemoveValue(l, -2);
					data.settings.insert(std::unordered_map<std::string, std::string>::value_type(key, val));
				}
			}
			Pop(l, 1);
			particles.insert(std::make_pair(particle, data));
		}
		else
			Pop(l, 1);
	}
	Pop(l, 2);
	PushBool(l, pragma::asset::save_particle_system(name, particles, rootPath));
	return 1;
}
std::shared_ptr<prosper::RenderTarget> Lua::engine::get_staging_render_target() { return pragma::get_cengine()->GetRenderContext().GetWindow().GetStagingRenderTarget(); }
void Lua::engine::set_fixed_frame_delta_time_interpretation(uint16_t fps) { pragma::get_cengine()->SetFixedFrameDeltaTimeInterpretationByFPS(fps); }
void Lua::engine::clear_fixed_frame_delta_time_interpretation() { pragma::get_cengine()->SetFixedFrameDeltaTimeInterpretation({}); }
void Lua::engine::set_tick_delta_time_tied_to_frame_rate(bool tieToFrameRate) { pragma::get_cengine()->SetTickDeltaTimeTiedToFrameRate(tieToFrameRate); }
Vector2i Lua::engine::get_window_resolution()
{
	auto &window = pragma::get_cengine()->GetRenderContext().GetWindow();
	return window.IsValid() ? window->GetSize() : ::Vector2i {};
}
Vector2i Lua::engine::get_render_resolution() { return pragma::get_cengine()->GetRenderResolution(); }
uint32_t Lua::engine::get_current_frame_index() { return pragma::get_cengine()->GetRenderContext().GetLastFrameId(); }
