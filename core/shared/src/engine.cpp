/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"

#include "pragma/engine.h"
#include "pragma/engine_init.hpp"
#include "pragma/lua/libraries/ldebug.h"
#include "pragma/lua/lua_error_handling.hpp"
#include "pragma/logging.hpp"
#include <pragma/serverstate/serverstate.h>
#include <pragma/console/convarhandle.h>
#include "luasystem.h"
#include <sharedutils/netpacket.hpp>
#include <pragma/console/convars.h>
#include "pragma/console/engine_cvar.h"
#include "pragma/debug/intel_vtune.hpp"
#include "pragma/model/modelmanager.h"
#include "pragma/engine_version.h"
#include "pragma/console/cvar.h"
#include "pragma/debug/debug_performance_profiler.hpp"
#include "pragma/localization.h"
#include <pragma/asset/util_asset.hpp>
#include <sharedutils/util.h>
#include <sharedutils/util_clock.hpp>
#include <sharedutils/util_parallel_job.hpp>
#include <pragma/game/game_resources.hpp>
#include <pragma/util/resource_watcher.h>
#include <util_pad.hpp>
#include <material_manager2.hpp>
#include <pragma/networking/iserver.hpp>
#include <pragma/addonsystem/addonsystem.h>
#include <pragma/model/animation/activities.h>
#include <pragma/model/animation/animation_event.h>
#include <sharedutils/util_library.hpp>
#include <sharedutils/util_path.hpp>
#include <sharedutils/util_debug.h>
#include <fsys/filesystem.h>
#include <spdlog/pattern_formatter.h>

#ifdef __linux__
#include <pthread.h>
#include <fcntl.h>
#endif

import util_zip;

const pragma::IServerState &Engine::GetServerStateInterface() const
{
	if(m_libServer == nullptr) {
		auto path = util::Path::CreatePath(util::get_program_path());
#ifdef _WIN32
		path += "bin/";
		path += "server";
#else
		path += "lib/";
		path += "libserver";
#endif
		auto modPath = path.GetString();
		std::string err;
		m_libServer = util::Library::Load(modPath, {}, &err);
		if(m_libServer == nullptr) {
			throw std::logic_error {"Unable to load module '" + modPath + "': " + err};
			exit(EXIT_FAILURE);
		}

		m_iServerState.Initialize(*m_libServer);
		if(m_iServerState.valid() == false) {
			throw std::logic_error {"Unresolved server state functions!"};
			exit(EXIT_FAILURE);
		}
	}
	return m_iServerState;
}

decltype(Engine::DEFAULT_TICK_RATE) Engine::DEFAULT_TICK_RATE = ENGINE_DEFAULT_TICK_RATE;

extern "C" {
void DLLNETWORK RunEngine(int argc, char *argv[])
{
	auto en = InitializeServer(argc, argv);
	en = nullptr;
}
}

static std::unordered_map<std::string, std::shared_ptr<PtrConVar>> *conVarPtrs = NULL;
std::unordered_map<std::string, std::shared_ptr<PtrConVar>> &Engine::GetConVarPtrs() { return *conVarPtrs; }
ConVarHandle Engine::GetConVarHandle(std::string scvar)
{
	if(conVarPtrs == NULL) {
		static std::unordered_map<std::string, std::shared_ptr<PtrConVar>> ptrs;
		conVarPtrs = &ptrs;
	}
	return CVarHandler::GetConVarHandle(*conVarPtrs, scvar);
}

DLLNETWORK Engine *engine = NULL;
extern std::optional<std::string> g_lpLogFile;
extern util::LogSeverity g_lpLogLevelCon;
extern util::LogSeverity g_lpLogLevelFile;

Engine::Engine(int, char *[]) : CVarHandler(), m_logFile(nullptr), m_tickRate(Engine::DEFAULT_TICK_RATE), m_stateFlags {StateFlags::Running | StateFlags::MultiThreadedAssetLoadingEnabled}
{
	// TODO: File cache doesn't work with absolute paths at the moment
	// (e.g. addons/imported/models/some_model.pmdl would return false even if the file exists)
	filemanager::set_use_file_index_cache(true);

#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::open_domain();
#endif
	Locale::Init();
	// OpenConsole();

	m_mainThreadId = std::this_thread::get_id();

	m_lastTick = static_cast<long long>(m_ctTick());
	engine = this;

#ifdef __linux__
	//setup fork handler
	//The fork will dupe the process id, and by extension the std streams. Disable async stdin to children.
	pthread_atfork(nullptr, nullptr, []() {
		//child, after fork.
		int flags = fcntl(0, F_GETFL, 0);
		fcntl(0, F_SETFL, flags & ~O_NONBLOCK);
	});
#endif

	// Link package system to file system
	m_padPackageManager = upad::link_to_file_system();
	m_assetManager = std::make_unique<pragma::asset::AssetManager>();

	RegisterCallback<void>("Think");

	m_cpuProfiler = pragma::debug::CPUProfiler::Create<pragma::debug::CPUProfiler>();
	AddProfilingHandler([this](bool profilingEnabled) {
		if(profilingEnabled == false) {
			m_profilingStageManager = nullptr;
			return;
		}
		m_profilingStageManager = std::make_unique<pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage>>();
		m_profilingStageManager->InitializeProfilingStageManager(*m_cpuProfiler);
	});
}

pragma::asset::AssetManager &Engine::GetAssetManager() { return *m_assetManager; }
const pragma::asset::AssetManager &Engine::GetAssetManager() const { return const_cast<Engine *>(this)->GetAssetManager(); }

void Engine::ClearConsole() { std::system("cls"); }

void Engine::SetConsoleType(ConsoleType type) { m_consoleType = type; }

Engine::ConsoleType Engine::GetConsoleType() const { return (m_consoleInfo && m_consoleInfo->console) ? ConsoleType::Terminal : ConsoleType::None; }

void Engine::SetReplicatedConVar(const std::string &cvar, const std::string &val)
{
	auto *client = GetClientState();
	if(client == nullptr)
		return;
	auto flags = client->GetConVarFlags(cvar);
	if((flags & ConVarFlags::Notify) == ConVarFlags::Notify)
		spdlog::info("ConVar '{}' has been changed to '{}'", cvar, val);
	//Con::cout<<"ConVar '"<<cvar<<"' has been changed to '"<<val<<"'"<<Con::endl;
	if((flags & ConVarFlags::Replicated) == ConVarFlags::None)
		return;
	client->SetConVar(cvar, val);
}

std::thread::id Engine::GetMainThreadId() const { return m_mainThreadId; }
std::optional<Engine::ConsoleOutput> Engine::PollConsoleOutput()
{
	if(m_bRecordConsoleOutput == false)
		return {};
	m_consoleOutputMutex.lock();
	if(m_consoleOutput.empty()) {
		m_consoleOutputMutex.unlock();
		return {};
	}
	auto r = m_consoleOutput.front();
	m_consoleOutput.pop();
	m_consoleOutputMutex.unlock();
	return r;
}
void Engine::SetRecordConsoleOutput(bool record)
{
	if(record == m_bRecordConsoleOutput)
		return;
	m_consoleOutputMutex.lock();
	m_bRecordConsoleOutput = record;
	m_consoleOutput = {};
	m_consoleOutputMutex.unlock();
}

CallbackHandle Engine::AddProfilingHandler(const std::function<void(bool)> &handler)
{
	auto hCb = FunctionCallback<void, bool>::Create(handler);
	m_profileHandlers.push_back(hCb);
	return hCb;
}

void Engine::SetProfilingEnabled(bool bEnabled)
{
	for(auto it = m_profileHandlers.begin(); it != m_profileHandlers.end();) {
		auto &hCb = *it;
		if(hCb.IsValid() == false) {
			it = m_profileHandlers.erase(it);
			continue;
		}
		hCb(bEnabled);
		++it;
	}
}

upad::PackageManager *Engine::GetPADPackageManager() const { return m_padPackageManager; }

void Engine::LockResourceWatchers()
{
	auto *sv = GetServerNetworkState();
	auto *cl = GetClientState();
	if(sv)
		sv->GetResourceWatcher().Lock();
	if(cl)
		cl->GetResourceWatcher().Lock();
}
void Engine::UnlockResourceWatchers()
{
	auto *sv = GetServerNetworkState();
	auto *cl = GetClientState();
	if(sv)
		sv->GetResourceWatcher().Unlock();
	if(cl)
		cl->GetResourceWatcher().Unlock();
}
void Engine::PollResourceWatchers()
{
	auto *sv = GetServerNetworkState();
	auto *cl = GetClientState();
	if(sv)
		sv->GetResourceWatcher().Poll();
	if(cl)
		cl->GetResourceWatcher().Poll();
}
util::ScopeGuard Engine::ScopeLockResourceWatchers()
{
	auto *sv = GetServerNetworkState();
	auto *cl = GetClientState();
	if(sv)
		sv->GetResourceWatcher().Lock();
	if(cl)
		cl->GetResourceWatcher().Lock();
	return util::ScopeGuard {[this, sv, cl]() {
		if(sv && GetServerNetworkState() == sv)
			sv->GetResourceWatcher().Unlock();
		if(cl && GetClientState() == cl)
			cl->GetResourceWatcher().Unlock();
	}};
}

void Engine::AddParallelJob(const util::ParallelJobWrapper &job, const std::string &jobName)
{
	m_parallelJobMutex.lock();
	m_parallelJobs.push_back({});
	m_parallelJobs.back().job = job;
	m_parallelJobs.back().name = jobName;
	m_parallelJobs.back().lastNotification = std::chrono::steady_clock::now();
	m_parallelJobs.back().lastProgressUpdate = std::chrono::steady_clock::now();
	m_parallelJobMutex.unlock();
}

void Engine::Close()
{
	if(umath::is_flag_set(m_stateFlags, StateFlags::Closed))
		return;
	umath::set_flag(m_stateFlags, StateFlags::Closed);

	if(ShouldRunUpdaterOnClose()) {
		std::string processPath;
#ifdef _WIN32
		processPath = "bin/updater.exe";
#else
		processPath = "lib/updater";
#endif
		if(!::util::start_process(processPath.c_str(), "-executable=" + util::get_program_name()))
			Con::cwar << "Failed to launch updater '" << processPath << "'! Please execute manually to install update." << Con::endl;
	}

	// Cancel all running jobs, then wait until
	// they have completed
	for(auto &jobInfo : m_parallelJobs)
		jobInfo.job->Cancel();
	for(auto &jobInfo : m_parallelJobs)
		jobInfo.job->Wait();
	m_parallelJobs.clear();

	umath::set_flag(m_stateFlags, StateFlags::Running, false);
	util::close_external_archive_manager();
	CloseServerState();

	ClearCommands();
	CloseConsole();
	EndLogging();

	Con::set_output_callback(nullptr);
	Locale::Clear();
}

static uint32_t clear_assets(NetworkState *state, pragma::asset::Type type, bool verbose)
{
	if(!state)
		return 0;
	auto *l = state->GetLuaState();
	if(l)
		Lua::debug::collectgarbage(l); // Make sure any potential Lua references to asset files that can be cleared are cleared
	uint32_t n = 0;
	switch(type) {
	case pragma::asset::Type::Model:
		{
			auto &mdlManager = state->GetModelManager();
			if(!verbose)
				n = mdlManager.ClearUnused();
			else {
				auto &cache = mdlManager.GetCache();

				std::unordered_map<Model *, std::string> oldCache;
				for(auto &pair : cache) {
					auto asset = mdlManager.GetAsset(pair.second);
					if(!asset)
						continue;
					oldCache[pragma::asset::ModelManager::GetAssetObject(*asset).get()] = pair.first;
				}

				n = mdlManager.ClearUnused();

				std::unordered_map<Model *, std::string> newCache;
				for(auto &pair : cache) {
					auto asset = mdlManager.GetAsset(pair.second);
					if(!asset)
						continue;
					newCache[pragma::asset::ModelManager::GetAssetObject(*asset).get()] = pair.first;
				}

				for(auto &pair : oldCache) {
					auto it = newCache.find(pair.first);
					if(it != newCache.end())
						continue;
					spdlog::info("Model {} was cleared from cache!", pair.second);
				}
			}
			break;
		}
	case pragma::asset::Type::Material:
		{
			auto &matManager = state->GetMaterialManager();
			if(!verbose)
				n = matManager.ClearUnused();
			else {
				auto &cache = matManager.GetCache();

				std::unordered_map<Material *, std::string> oldCache;
				for(auto &pair : cache) {
					auto asset = matManager.GetAsset(pair.second);
					if(!asset)
						continue;
					oldCache[msys::MaterialManager::GetAssetObject(*asset).get()] = pair.first;
				}

				n = matManager.ClearUnused();

				std::unordered_map<Material *, std::string> newCache;
				for(auto &pair : cache) {
					auto asset = matManager.GetAsset(pair.second);
					if(!asset)
						continue;
					newCache[msys::MaterialManager::GetAssetObject(*asset).get()] = pair.first;
				}

				for(auto &pair : oldCache) {
					auto it = newCache.find(pair.first);
					if(it != newCache.end())
						continue;
					spdlog::info("Material {} was cleared from cache!", pair.second);
				}
			}
			break;
		}
	}
	return n;
}
uint32_t Engine::DoClearUnusedAssets(pragma::asset::Type type) const
{
	uint32_t n = 0;
	n += clear_assets(GetServerNetworkState(), type, IsVerbose());
	n += clear_assets(GetClientState(), type, IsVerbose());
	return n;
}
uint32_t Engine::ClearUnusedAssets(pragma::asset::Type type, bool verbose) const
{
	auto n = DoClearUnusedAssets(type);
	if(verbose)
		spdlog::info("{} assets have been cleared!", n);
	return n;
}
void Engine::SetAssetMultiThreadedLoadingEnabled(bool enabled)
{
	auto *sv = GetServerNetworkState();
	if(sv) {
		sv->GetModelManager().GetLoader().SetMultiThreadingEnabled(enabled);
		auto &matManager = sv->GetMaterialManager();
		matManager.GetLoader().SetMultiThreadingEnabled(enabled);
	}
}
void Engine::UpdateAssetMultiThreadedLoadingEnabled() { SetAssetMultiThreadedLoadingEnabled(umath::is_flag_set(m_stateFlags, StateFlags::MultiThreadedAssetLoadingEnabled)); }
uint32_t Engine::ClearUnusedAssets(const std::vector<pragma::asset::Type> &types, bool verbose) const
{
	uint32_t n = 0;
	for(auto type : types)
		n += ClearUnusedAssets(type, false);
	if(verbose)
		spdlog::info("{} assets have been cleared!", n);
	return n;
}

void Engine::SetRunUpdaterOnClose(bool run) { umath::set_flag(m_stateFlags, StateFlags::RunUpdaterOnClose, run); }
bool Engine::ShouldRunUpdaterOnClose() const { return umath::is_flag_set(m_stateFlags, StateFlags::RunUpdaterOnClose); }

void Engine::ClearCache()
{
	constexpr auto clearAssetFiles = false;
	spdlog::info("Clearing cached files...");
	auto fRemoveDir = [](const std::string &name) {
		spdlog::info("Removing '{}'", name);
		auto result = FileManager::RemoveDirectory(name.c_str());
		if(result == false)
			spdlog::warn("Failed to remove cache directory '{}'! Please remove it manually.", name);
		return result;
	};
	fRemoveDir("cache");
	if(clearAssetFiles) {
		fRemoveDir("addons/imported");
		fRemoveDir("addons/converted");
	}

	spdlog::info("Removing addon cache directories...");
	for(auto &addonInfo : AddonSystem::GetMountedAddons()) {
		auto path = addonInfo.GetAbsolutePath() + "\\cache";
		if(FileManager::ExistsSystem(path) == false)
			continue;
		if(FileManager::RemoveSystemDirectory(path.c_str()) == false)
			spdlog::warn("Failed to remove cache directory '{}'! Please remove it manually.", path);
	}

	// Give it a bit of time to complete
	std::this_thread::sleep_for(std::chrono::milliseconds {500});

	// Re-create the directories
	FileManager::CreatePath("cache");
	if(clearAssetFiles) {
		FileManager::CreatePath("addons/imported");
		FileManager::CreatePath("addons/converted");
	}

	// Give it a bit of time to complete
	std::this_thread::sleep_for(std::chrono::milliseconds {500});
	// The addon system needs to be informed right away, to make sure the new directories are re-mounted
	AddonSystem::Poll();
	Con::cout << "Cache cleared successfully! Please restart the Engine." << Con::endl;
}

ServerState *Engine::GetServerState() const
{
	if(m_svInstance == nullptr)
		return nullptr;
	return static_cast<ServerState *>(m_svInstance->state.get());
}

NetworkState *Engine::GetServerNetworkState() const { return static_cast<NetworkState *>(GetServerState()); }

void Engine::EndGame()
{
	auto *sv = GetServerNetworkState();
	if(sv != nullptr)
		sv->EndGame();
	CloseServer();
}

void Engine::SetMountExternalGameResources(bool b)
{
	m_bMountExternalGameResources = b;
	if(b == true)
		InitializeExternalArchiveManager();
}
bool Engine::ShouldMountExternalGameResources() const { return m_bMountExternalGameResources; }

pragma::debug::CPUProfiler &Engine::GetProfiler() const { return *m_cpuProfiler; }
pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage> *Engine::GetProfilingStageManager() { return m_profilingStageManager.get(); }
bool Engine::StartProfilingStage(const char *stage) { return m_profilingStageManager && m_profilingStageManager->StartProfilerStage(stage); }
bool Engine::StopProfilingStage() { return m_profilingStageManager && m_profilingStageManager->StopProfilerStage(); }

void Engine::RunTickEvents()
{
	m_tickEventQueueMutex.lock();
	if(m_tickEventQueue.empty()) {
		m_tickEventQueueMutex.unlock();
		return;
	}
	auto queue = std::move(m_tickEventQueue);
	m_tickEventQueue = {};
	m_tickEventQueueMutex.unlock();

	while(!queue.empty()) {
		queue.front()();
		queue.pop();
	}
}
void Engine::AddTickEvent(const std::function<void()> &ev)
{
	m_tickEventQueueMutex.lock();
	m_tickEventQueue.push(ev);
	m_tickEventQueueMutex.unlock();
}

void Engine::Tick()
{
	Locale::Poll();
	m_ctTick.Update();
	ProcessConsoleInput();
	RunTickEvents();

	StartProfilingStage("Tick");
	StartProfilingStage("ServerTick");
	auto *sv = GetServerNetworkState();
	if(sv != NULL)
		sv->Tick();
	StopProfilingStage(); // ServerTick
	StopProfilingStage(); // Tick

	UpdateParallelJobs();
}

void Engine::UpdateParallelJobs()
{
	// Update background tasks
	// Note: We can't use iterators here, since 'Poll' can potentially add new jobs which can invalidate the iterator
	for(auto i = decltype(m_parallelJobs.size()) {0u}; i < m_parallelJobs.size();) {
		auto &jobInfo = m_parallelJobs.at(i);
		auto progress = jobInfo.job->GetProgress();
		auto t = std::chrono::steady_clock::now();
		auto tDelta = t - jobInfo.lastProgressUpdate;
		if(progress != jobInfo.lastProgress) {
			jobInfo.timeRemaining = util::clock::to_seconds(tDelta) / (progress - jobInfo.lastProgress) * (1.f - progress);
			jobInfo.lastProgress = progress;
			jobInfo.lastProgressUpdate = t;
		}
		if((t - jobInfo.lastNotification) >= jobInfo.notificationFrequency) {
			auto percent = progress * 100.f;
			Con::cout << "Progress of worker '" << jobInfo.name << "' at " << umath::floor(percent) << "%.";
			if(jobInfo.timeRemaining.has_value()) {
				auto msgDur = util::get_pretty_duration(*jobInfo.timeRemaining * std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::seconds {1}).count());
				Con::cout << " Approximately " << msgDur << " remaining!";
			}
			Con::cout << Con::endl;
			jobInfo.lastNotification = t;
		}

		if(jobInfo.job->Poll() == false) {
			++i;
			continue;
		}
		m_parallelJobs.erase(m_parallelJobs.begin() + i);
	}
}

ConVarMap *Engine::GetConVarMap() { return console_system::engine::get_convar_map(); }

std::unique_ptr<Engine::ConVarInfoList> &Engine::GetConVarConfig(NwStateType type)
{
	assert(type == NwStateType::Server);
	return m_svConfig;
}
Engine::StateInstance &Engine::GetStateInstance(NetworkState &nw)
{
	assert(m_svInstance != nullptr);
	return *m_svInstance;
}
Engine::StateInstance &Engine::GetServerStateInstance() { return *m_svInstance; }

void Engine::SetVerbose(bool bVerbose) { umath::set_flag(m_stateFlags, StateFlags::Verbose, bVerbose); }
bool Engine::IsVerbose() const { return umath::is_flag_set(m_stateFlags, StateFlags::Verbose); }

void Engine::SetConsoleSubsystem(bool consoleSubsystem) { umath::set_flag(m_stateFlags, StateFlags::ConsoleSubsystem, consoleSubsystem); }
bool Engine::IsConsoleSubsystem() const { return umath::is_flag_set(m_stateFlags, StateFlags::ConsoleSubsystem); }

void Engine::SetDeveloperMode(bool devMode) { umath::set_flag(m_stateFlags, StateFlags::DeveloperMode, devMode); }
bool Engine::IsDeveloperModeEnabled() const { return umath::is_flag_set(m_stateFlags, StateFlags::DeveloperMode); }

void Engine::SetNonInteractiveMode(bool nonInteractiveMode) { umath::set_flag(m_stateFlags, StateFlags::NonInteractiveMode, nonInteractiveMode); }
bool Engine::IsNonInteractiveMode() const { return umath::is_flag_set(m_stateFlags, StateFlags::NonInteractiveMode); }

void Engine::SetCLIOnly(bool cliOnly) { umath::set_flag(m_stateFlags, StateFlags::CLIOnly, cliOnly); }
bool Engine::IsCLIOnly() const { return umath::is_flag_set(m_stateFlags, StateFlags::CLIOnly); }

void Engine::Release() { Close(); }

bool Engine::Initialize(int argc, char *argv[])
{
	InitLaunchOptions(argc, argv);

	pragma::detail::initialize_logger(g_lpLogLevelCon, g_lpLogLevelFile, g_lpLogFile);
	spdlog::info("Engine Version: {}", get_pretty_engine_version());
	auto f = filemanager::open_file("git_info.txt", filemanager::FileMode::Read, fsys::SearchFlags::Local | fsys::SearchFlags::NoMounts);
	if(f) {
		spdlog::info("Git Info:");
		auto str = f->ReadString();
		ustring::replace(str, "\n", spdlog::details::os::default_eol);
		spdlog::info(str);
	}

#if 0
	Con::cout<<"----- Logger test -----"<<Con::endl;
	Con::cout<<"cout output"<<Con::endl;
	Con::cwar<<"cwar output"<<Con::endl;
	Con::cerr<<"cerr output"<<Con::endl;
	Con::crit<<"crit output"<<Con::endl;
	Con::csv<<"csv output"<<Con::endl;
	Con::ccl<<"ccl output"<<Con::endl;
	
	spdlog::trace("trace log");
	spdlog::debug("debug log");
	spdlog::info("info log");
	spdlog::warn("warn log");
	spdlog::error("error log");
	spdlog::critical("critical log");
	Con::cout<<"----- Logger test -----"<<Con::endl;
	spdlog::default_logger()->flush();
#endif

	// These need to exist, so they can be automatically mounted
	if(FileManager::Exists("addons/imported") == false)
		FileManager::CreatePath("addons/imported");
	if(FileManager::Exists("addons/converted") == false)
		FileManager::CreatePath("addons/converted");

	pragma::register_engine_animation_events();
	pragma::register_engine_activities();

	Con::set_output_callback([this](const std::string_view &output, Con::MessageFlags flags, const Color *color) {
		if(m_bRecordConsoleOutput == false)
			return;
		m_consoleOutputMutex.lock();
		m_consoleOutput.push({std::string {output}, flags, color ? std::make_shared<Color>(*color) : nullptr});
		m_consoleOutputMutex.unlock();
	});

	CVarHandler::Initialize();
	RegisterConsoleCommands();

	// Initialize Server Instance
	auto matManager = msys::MaterialManager::Create();
	matManager->SetImportDirectory("addons/converted/materials");
	InitializeAssetManager(*matManager);

	pragma::asset::update_extension_cache(pragma::asset::Type::Map);
	pragma::asset::update_extension_cache(pragma::asset::Type::Sound);
	pragma::asset::update_extension_cache(pragma::asset::Type::ParticleSystem);
	pragma::asset::update_extension_cache(pragma::asset::Type::Texture);
	pragma::asset::update_extension_cache(pragma::asset::Type::Material);

	auto matErr = matManager->LoadAsset("error");
	m_svInstance = std::unique_ptr<StateInstance>(new StateInstance {matManager, matErr.get()});
	//

	if(Lua::get_extended_lua_modules_enabled()) {
		RegisterConCommand("l", [this](NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float) { RunConsoleCommand("lua_run", argv); });
	}
	if(!IsServerOnly())
		LoadConfig();

	auto cacheVersion = GetConVarInt("cache_version");
	auto cacheVersionTarget = GetConVarInt("cache_version_target");
	if(cacheVersion != cacheVersionTarget) {
		SetConVar("cache_version", std::to_string(cacheVersionTarget));
		ClearCache();
	}

	ServerState *server = OpenServerState();
	if(server != nullptr && IsServerOnly())
		LoadConfig();

	if(!GetConVarBool("asset_file_cache_enabled"))
		filemanager::set_use_file_index_cache(false);
	if(!GetConVarBool("asset_multithreading_enabled"))
		SetAssetMultiThreadedLoadingEnabled(false);
	return true;
}

void Engine::InitializeAssetManager(util::FileAssetManager &assetManager) const
{
	assetManager.SetLogHandler(&pragma::log);
	assetManager.SetExternalSourceFileImportHandler([this, &assetManager](const std::string &path, const std::string &outputPath) -> std::optional<std::string> {
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
		::debug::get_domain().BeginTask("import_asset_file");
		util::ScopeGuard sg {[]() { ::debug::get_domain().EndTask(); }};
#endif
		auto *nw = GetClientState();
		if(!nw || !nw->IsGameActive()) {
			auto *nwSv = GetServerNetworkState();
			if(nwSv && nwSv->IsGameActive())
				nw = nwSv;
		}
		if(!nw)
			return {};
		auto &rootDir = assetManager.GetRootDirectory();
		auto &extensions = assetManager.GetSupportedFormatExtensions();
		for(auto &extInfo : extensions) {
			auto relPath = util::Path::CreateFile(path + '.' + extInfo.extension);
			auto formatPath = rootDir;
			formatPath += relPath;

			auto p = rootDir;
			p += util::Path::CreateFile(outputPath + '.' + extInfo.extension);

			auto portSuccess = false;
			if(extInfo.extension == "bsp")
				portSuccess = util::port_hl2_map(nw, relPath.GetString());
			else if(extInfo.extension == "mdl")
				portSuccess = util::port_hl2_model(nw, ufile::get_path_from_filename(formatPath.GetString()), ufile::get_file_from_filename(p.GetString()));
			else
				portSuccess = util::port_file(nw, formatPath.GetString(), p.GetString());
			if(portSuccess)
				return extInfo.extension;
		}
		return {};
	});
}

void Engine::InitializeExternalArchiveManager() { util::initialize_external_archive_manager(GetServerNetworkState()); }

void Engine::RunLaunchCommands()
{
	spdlog::info("Running launch commands...");
	for(auto it = m_launchCommands.rbegin(); it != m_launchCommands.rend(); ++it) {
		auto &cmd = *it;
		spdlog::debug("Running launch command '{}'...", cmd.command);
		RunConsoleCommand(cmd.command, cmd.args);
		if(ustring::compare(cmd.command.c_str(), "map", false)) {
			// We'll delay all remaining commands until after the map has been loaded
			std::vector<LaunchCommand> remainingCommands;
			++it;
			for(auto it2 = it; it2 != m_launchCommands.rend(); ++it2) {
				if(ustring::compare(it2->command.c_str(), "map", false))
					continue;
				remainingCommands.push_back(*it2);
			}

			auto *nw = GetClientState();
			if(!nw)
				nw = GetServerState();
			if(nw) {
				spdlog::info("{} game commands will be executed after map load.", remainingCommands.size());
				auto cbOnGameStart = FunctionCallback<void>::Create(nullptr);
				cbOnGameStart.get<Callback<void>>()->SetFunction([this, cbOnGameStart, nw, remainingCommands = std::move(remainingCommands)]() mutable {
					auto cmds0 = std::move(remainingCommands);
					auto *game = nw->GetGameState();
					if(game) {
						auto cbOnGameReady = FunctionCallback<void>::Create(nullptr);
						cbOnGameReady.get<Callback<void>>()->SetFunction([this, cbOnGameReady, cmds0 = std::move(cmds0)]() mutable {
							auto cmds1 = std::move(cmds0);
							auto *pThis = this;

							// This will invalidate all captured variables!
							if(cbOnGameReady.IsValid())
								cbOnGameReady.Remove();

							spdlog::info("Executing game commands...");
							for(auto &cmd : cmds1) {
								spdlog::debug("Executing game command '{}'...", cmd.command);
								pThis->RunConsoleCommand(cmd.command, cmd.args);
							}
						});
						game->AddCallback("OnGameReady", cbOnGameReady);
					}

					if(cbOnGameStart.IsValid())
						cbOnGameStart.Remove();
				});
				nw->AddCallback("OnGameStart", cbOnGameStart);
			}
			break;
		}
	}
	m_launchCommands.clear();
}

Lua::Interface *Engine::GetLuaInterface(lua_State *l)
{
	auto *sv = GetServerNetworkState();
	if(sv == nullptr)
		return nullptr;
	auto *sg = sv->GetGameState();
	if(sg == nullptr)
		return nullptr;
	if(sg->GetLuaState() == l)
		return &sg->GetLuaInterface();
	return nullptr;
}

NetworkState *Engine::GetNetworkState(lua_State *l)
{
	auto *sv = GetServerNetworkState();
	if(sv == NULL)
		return NULL;
	if(sv->GetLuaState() == l)
		return sv;
	return NULL;
}

bool Engine::IsMultiPlayer() const
{
	auto *sv = GetServerNetworkState();
	if(sv == nullptr)
		return false;
	return sv->IsMultiPlayer();
}
bool Engine::IsSinglePlayer() const { return !IsMultiPlayer(); }

void Engine::StartServer(bool singlePlayer)
{
	auto *sv = GetServerNetworkState();
	if(sv == NULL)
		return;
	GetServerStateInterface().start_server(singlePlayer);
}

void Engine::CloseServer()
{
	auto *sv = GetServerNetworkState();
	if(sv == NULL)
		return;
	GetServerStateInterface().close_server();
}

bool Engine::IsClientConnected() { return false; }

bool Engine::IsServerRunning()
{
	auto *sv = GetServerNetworkState();
	if(sv == NULL)
		return false;
	return GetServerStateInterface().is_server_running();
}

void Engine::StartNewGame(const std::string &map, bool singlePlayer)
{
	EndGame();
	auto *sv = GetServerNetworkState();
	if(sv == nullptr)
		return;
	sv->StartNewGame(map, singlePlayer);
}

void Engine::StartDefaultGame(const std::string &map)
{
	EndGame();
	StartNewGame(map.c_str(), false);
}

std::optional<uint64_t> Engine::GetServerSteamId() const
{
	auto *svState = GetServerNetworkState();
	if(svState == nullptr)
		return {};
	std::optional<uint64_t> steamId;
	GetServerStateInterface().get_server_steam_id(steamId);
	return steamId;
}

static auto cvRemoteDebugging = GetConVar("sh_lua_remote_debugging");
int32_t Engine::GetRemoteDebugging() const { return cvRemoteDebugging->GetInt(); }

extern std::string __lp_map;
extern std::string __lp_gamemode;
static auto cvMountExternalResources = GetConVar("sh_mount_external_game_resources");
void Engine::Start()
{
	if(cvMountExternalResources->GetBool() == true)
		SetMountExternalGameResources(true);
	if(!__lp_gamemode.empty()) {
		std::vector<std::string> argv = {__lp_gamemode};
		RunConsoleCommand("sv_gamemode", argv);
	}
	if(!__lp_map.empty())
		StartDefaultGame(__lp_map);

	RunLaunchCommands();

	InvokeConVarChangeCallbacks("steam_steamworks_enabled");

	spdlog::debug("Starting main game loop...");
	//const double FRAMES_PER_SECOND = GetTickRate();
	//const double SKIP_TICKS = 1000 /FRAMES_PER_SECOND;
	const int MAX_FRAMESKIP = 5;
	long long nextTick = GetTickCount();
	int loops;
	do {
		StartProfilingStage("Think");
		Think();
		StopProfilingStage();

		loops = 0;
		auto tickRate = GetTickRate();
		auto skipTicks = static_cast<long long>(1'000 / tickRate);

		auto t = GetTickCount();
		while(t > nextTick && loops < MAX_FRAMESKIP) {
			Tick();

			m_lastTick = static_cast<long long>(m_ctTick());
			nextTick += skipTicks; //SKIP_TICKS);
			loops++;
		}
		if(t > nextTick)
			nextTick = t; // This should only happen after loading times
	} while(IsRunning());
	Close();
}

void Engine::UpdateTickCount() { m_ctTick.Update(); }

#ifdef _WIN32
extern std::string g_crashExceptionMessage;
#endif
std::unique_ptr<uzip::ZIPFile> Engine::GenerateEngineDump(const std::string &baseName, std::string &outZipFileName, std::string &outErr)
{
	auto programPath = util::Path::CreatePath(util::get_program_path());
	outZipFileName = util::get_date_time(baseName + "_%Y-%m-%d_%H-%M-%S.zip");
	auto zipName = programPath + outZipFileName;
	auto zipFile = uzip::ZIPFile::Open(zipName.GetString(), uzip::OpenMode::Write);
	if(!zipFile) {
		outErr = "Failed to create dump file '" + zipName.GetString() + "'";
		return nullptr;
	}

#ifdef _WIN32
	// Write Exception
	if(g_crashExceptionMessage.empty() == false)
		zipFile->AddFile("exception.txt", g_crashExceptionMessage);

	// Write Stack Backtrace
	zipFile->AddFile("stack_backtrace.txt", util::get_formatted_stack_backtrace_string());
#endif

	// Write Info
	if(engine != nullptr)
		engine->DumpDebugInformation(*zipFile.get());
	return zipFile;
}

void Engine::DumpDebugInformation(uzip::ZIPFile &zip) const
{
	std::stringstream engineInfo;
	engineInfo << "System: ";
	if(util::is_windows_system())
		engineInfo << "Windows";
	else
		engineInfo << "Linux";
	if(util::is_x64_system())
		engineInfo << " x64";
	else
		engineInfo << " x86";
	if(engine != nullptr)
		engineInfo << "\nEngine Version: " << get_pretty_engine_version();
	auto *nw = static_cast<NetworkState *>(GetServerNetworkState());
	if(nw == nullptr)
		nw = GetClientState();
	if(nw != nullptr) {
		auto *game = nw->GetGameState();
		if(game != nullptr) {
			auto &mapInfo = game->GetMapInfo();
			engineInfo << "\nMap: " << mapInfo.name << " (" << mapInfo.fileName << ")";
		}
	}
	zip.AddFile("engine.txt", engineInfo.str());

	auto fAddFile = [&zip](const std::string &fileName, const std::string &zipFileName) {
		std::string path = util::get_program_path() + "/" + fileName;
		std::ifstream t {path};
		if(t.is_open()) {
			std::stringstream buffer;
			buffer << t.rdbuf();
			std::string log = buffer.str();
			zip.AddFile(zipFileName, log);
		}
	};

	auto logFileName = pragma::detail::get_log_file_name();
	if(logFileName.has_value()) {
		pragma::flush_loggers();

		/* For some reason this will fail sometimes
		auto logContents = filemanager::read_file(*logFileName);
		if(logContents.has_value()) {
			zip.AddFile("log.txt", *logContents);
		}
		*/

		fAddFile(*logFileName, "log.txt");
	}

	fAddFile("git_info.txt", "git_info.txt");

	auto fWriteConvars = [&zip](const std::map<std::string, std::shared_ptr<ConConf>> &cvarMap, const std::string &fileName) {
		std::stringstream convars;
		for(auto &pair : cvarMap) {
			if(pair.second->GetType() != ConType::Variable)
				continue;
			auto *cv = static_cast<ConVar *>(pair.second.get());
			if(umath::is_flag_set(cv->GetFlags(), ConVarFlags::Password))
				continue; // Don't store potentially personal passwords in the crashdump
			convars << pair.first << " \"" << cv->GetString() << "\"\n";
		}
		zip.AddFile(fileName, convars.str());
	};
	fWriteConvars(GetConVars(), "cvars_en.txt");

	auto fWriteLuaTraceback = [&zip](lua_State *l, const std::string &identifier) {
		if(!l)
			return;
		std::stringstream ss;
		if(!Lua::PrintTraceback(l, ss))
			return;
		auto strStack = Lua::StackToString(l);
		if(strStack.has_value())
			ss << "\n\n" << *strStack;
		zip.AddFile("lua_traceback_" + identifier + ".txt", ss.str());
	};
	if(GetClientState()) {
		fWriteConvars(GetClientState()->GetConVars(), "cvars_cl.txt");
		fWriteLuaTraceback(GetClientState()->GetLuaState(), "cl");
	}
	if(GetServerNetworkState()) {
		fWriteConvars(GetServerNetworkState()->GetConVars(), "cvars_sv.txt");
		fWriteLuaTraceback(GetServerNetworkState()->GetLuaState(), "sv");
	}
}

const long long &Engine::GetLastTick() const { return m_lastTick; }

long long Engine::GetDeltaTick() const { return GetTickCount() - m_lastTick; }

void Engine::Think()
{
	AddonSystem::Poll(); // Required for dynamic mounting of addons
	UpdateTickCount();
	CallCallbacks<void>("Think");
	auto *sv = GetServerNetworkState();
	if(sv != NULL)
		sv->Think();
}

ServerState *Engine::OpenServerState()
{
	CloseServerState();
	std::unique_ptr<ServerState> svState;
	GetServerStateInterface().create_server_state(svState);
	m_svInstance->state = std::move(svState);
	auto *sv = GetServerNetworkState();
	sv->Initialize();
	UpdateAssetMultiThreadedLoadingEnabled();
	return static_cast<ServerState *>(sv);
}

void Engine::CloseServerState()
{
	auto *sv = GetServerNetworkState();
	if(sv == nullptr)
		return;
	sv->Close();
	m_svInstance->state = nullptr;
	GetServerStateInterface().clear_server_state();
}

NetworkState *Engine::GetClientState() const { return NULL; }

NetworkState *Engine::GetActiveState() { return GetServerNetworkState(); }

bool Engine::IsActiveState(NetworkState *state) { return state == GetActiveState(); }

void Engine::AddLaunchConVar(std::string cvar, std::string val) { m_launchCommands.push_back({cvar, {val}}); }

void Engine::ShutDown() { umath::set_flag(m_stateFlags, StateFlags::Running, false); }

void Engine::HandleLocalHostPlayerClientPacket(NetPacket &p) {}
void Engine::HandleLocalHostPlayerServerPacket(NetPacket &p) { return GetServerStateInterface().handle_local_host_player_server_packet(p); }

bool Engine::ConnectLocalHostPlayerClient() { return GetServerStateInterface().connect_local_host_player_client(); }

Engine::~Engine()
{
	engine = nullptr;
	if(umath::is_flag_set(m_stateFlags, StateFlags::Running))
		throw std::runtime_error("Engine has to be closed before it can be destroyed!");
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::close_domain();
#endif

	spdlog::info("Closing logger...");
	pragma::detail::close_logger();
}

Engine *pragma::get_engine() { return engine; }
ServerState *pragma::get_server_state() { return engine->GetServerStateInterface().get_server_state(); }

REGISTER_ENGINE_CONVAR_CALLBACK(debug_profiling_enabled, [](NetworkState *, const ConVar &, bool, bool enabled) {
	if(engine == nullptr)
		return;
	engine->SetProfilingEnabled(enabled);
});
