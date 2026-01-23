// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:debug.performance_profiler;

import pragma.util;

export {
	namespace pragma {
		namespace debug {
			struct DLLNETWORK ProfilerResult {
				std::optional<std::chrono::nanoseconds> duration;
			};

			class DLLNETWORK Timer {
			  public:
				Timer(const Timer &) = delete;
				Timer &operator=(const Timer &) = delete;
				virtual ~Timer() = default;
				virtual bool Start()
				{
					++m_count;
					return true;
				}
				virtual bool Stop() = 0;
				virtual void ResetCounters() { m_count = 0; }
				size_t GetCount() const { return m_count; }
				virtual std::unique_ptr<ProfilerResult> GetResult() const = 0;
			  protected:
				Timer() = default;
				uint32_t m_count = 0;
			};

			class Profiler;
			class DLLNETWORK ProfilingStage : public std::enable_shared_from_this<ProfilingStage> {
			  public:
				using StageId = uint32_t;
				template<class TProfilingStage>
				static std::shared_ptr<TProfilingStage> Create(Profiler &profiler, std::thread::id tid, const std::string &name);
				static std::shared_ptr<ProfilingStage> Create(Profiler &profiler, std::thread::id tid, const std::string &name);
				virtual ~ProfilingStage() = default;
				ProfilingStage(const ProfilingStage &) = delete;
				ProfilingStage &operator=(const ProfilingStage &) = delete;
				virtual void Initialize();

				bool Start();
				bool Stop();
				void ResetCounters();
				std::unique_ptr<ProfilerResult> GetResult() const;
				size_t GetCount() const;

				std::thread::id GetThreadId() const { return m_threadId; }
				StageId GetStageId() const;
				const std::string &GetName() const;
				Profiler &GetProfiler();
				ProfilingStage *GetParent();
				void SetParent(ProfilingStage *parent);
				const std::vector<std::weak_ptr<ProfilingStage>> &GetChildren() const;
				const Timer &GetTimer() const;
				Timer &GetTimer();
			  protected:
				ProfilingStage(Profiler &profiler, std::thread::id tid, const std::string &name);
				virtual void InitializeTimer();
				std::shared_ptr<Timer> m_timer;
			  private:
				Profiler &m_profiler;
				std::weak_ptr<ProfilingStage> m_parent = {};
				std::vector<std::weak_ptr<ProfilingStage>> m_children = {};
				StageId m_stage;
				std::string m_name;
				std::thread::id m_threadId;
			};

			class CPUProfiler;
			class DLLNETWORK Profiler {
			  public:
				template<class TProfiler>
				static std::shared_ptr<TProfiler> Create();
				virtual std::shared_ptr<Timer> CreateTimer() { return nullptr; };
				virtual void Initialize();

				bool StartStage(ProfilingStage::StageId stage);
				bool StopStage(ProfilingStage::StageId stage);

				const ProfilingStage &GetRootStage() const;
				ProfilingStage &GetRootStage();
				ProfilingStage *GetStage(ProfilingStage::StageId stage);
				const std::vector<std::weak_ptr<ProfilingStage>> &GetStages() const;
				void AddStage(ProfilingStage &stage);
			  protected:
				Profiler() = default;
				std::shared_ptr<ProfilingStage> m_rootStage = nullptr;
			  private:
				ProfilingStage::StageId m_nextStageId = 0u;
				std::vector<std::weak_ptr<ProfilingStage>> m_stages = {};
				friend std::shared_ptr<ProfilingStage> ProfilingStage::Create(Profiler &profiler, std::thread::id tid, const std::string &name);
			};

			template<class TProfilingStage>
			class ProfilingStageManager {
			  public:
				TProfilingStage &GetProfilerStage(std::thread::id tid, const char *name);
				bool StartProfilerStage(const char *name);
				bool StopProfilerStage();
				void InitializeProfilingStageManager(Profiler &profiler);
			  protected:
				using ProfilingStageMap = std::unordered_map<const char *, std::shared_ptr<TProfilingStage>>;
				struct ProfilingThreadData {
					ProfilingStageMap stageMap;
					std::stack<const char *> stageStack;
#if ENABLE_DEBUG_HISTORY == 1
					struct HistoryItem {
						std::string name;
						std::string stackTrace;
					};
					std::vector<HistoryItem> history;
#endif
				};
				std::unordered_map<std::thread::id, ProfilingThreadData> m_threadProfilingMap;
				std::mutex m_profilingStageMutex;
			  private:
				Profiler *m_profiler = nullptr;
			};

			////////

			class DLLNETWORK CPUTimer : public Timer {
			  public:
				static std::shared_ptr<CPUTimer> Create();
				virtual bool Start() override;
				virtual bool Stop() override;
				virtual void ResetCounters() override;
				virtual std::unique_ptr<ProfilerResult> GetResult() const override;
			  private:
				CPUTimer() = default;
				util::Clock::time_point m_startTime = {};
				util::Clock::duration m_duration = {};
			};

			class DLLNETWORK CPUProfiler : public Profiler {
			  public:
				CPUProfiler() = default;
				virtual std::shared_ptr<Timer> CreateTimer() override;
			};

			template<class TProfilingStage>
			TProfilingStage &ProfilingStageManager<TProfilingStage>::GetProfilerStage(std::thread::id tid, const char *name)
			{
				auto itThread = m_threadProfilingMap.find(tid);
				if(itThread == m_threadProfilingMap.end()) {
					std::stringstream ss;
					ss << tid;
					throw std::runtime_error {"No profiling stages exist for thread " + ss.str() + "!"};
				}
				auto &profilingStages = itThread->second.stageMap;
				auto it = profilingStages.find(name);
				if(it == profilingStages.end())
					throw std::runtime_error {"Unknown profiler stage '" + std::string {name} + "'!"};
				return *it->second;
			}

			template<class TProfilingStage>
			bool ProfilingStageManager<TProfilingStage>::StartProfilerStage(const char *name)
			{
				std::unique_lock<std::mutex> lock {m_profilingStageMutex};

				auto tid = std::this_thread::get_id();
				auto itThread = m_threadProfilingMap.find(tid);
				if(itThread == m_threadProfilingMap.end())
					itThread = m_threadProfilingMap.insert(std::make_pair(tid, ProfilingStageMap {})).first;

				auto &threadData = itThread->second;
				auto &profilingStages = threadData.stageMap;
				auto &stageStack = threadData.stageStack;

#if ENABLE_DEBUG_HISTORY == 1
				std::string prefix(stageStack.size() * 2, ' ');
				threadData.history.push_back({prefix + "Start " + std::string {name}, pragma::util::get_formatted_stack_backtrace_string()});
				while(threadData.history.size() > 1000)
					threadData.history.erase(threadData.history.begin());
#endif

				auto it = profilingStages.find(name);
				if(it == profilingStages.end()) {
					const char *parentName = nullptr;
					if(!stageStack.empty())
						parentName = stageStack.top();
					std::stringstream ss;
					ss << tid;
					spdlog::info("Creating new profiling stage '{}' with parent '{}' for thread {}...", name, parentName ? parentName : "NULL", ss.str());
					auto stage = ProfilingStage::Create<TProfilingStage>(*m_profiler, tid, name);
					it = profilingStages.insert(std::make_pair(name, stage)).first;

					if(parentName) {
						auto &stageParent = GetProfilerStage(tid, parentName);
						stage->SetParent(&stageParent);
					}
					else
						stage->SetParent(&m_profiler->GetRootStage());
				}

				stageStack.push(name);
				return it->second->Start();
			}

			template<class TProfilingStage>
			bool ProfilingStageManager<TProfilingStage>::StopProfilerStage()
			{
				std::unique_lock<std::mutex> lock {m_profilingStageMutex};

				auto tid = std::this_thread::get_id();
				auto itThread = m_threadProfilingMap.find(tid);
				if(itThread == m_threadProfilingMap.end())
					throw std::logic_error {"Attempted to stop profiling stage on thread that has never started a profiling stage."};
				auto &threadData = itThread->second;
				auto &profilingStages = threadData.stageMap;
				auto &stageStack = threadData.stageStack;

				if(stageStack.empty())
					throw std::logic_error {"Attempted to stop profiling stage, but no stage has been started!"};

#if ENABLE_DEBUG_HISTORY == 1
				std::string prefix((stageStack.size() - 1) * 2, ' ');
				threadData.history.push_back({prefix + "Stop " + std::string {stageStack.top()}, pragma::util::get_formatted_stack_backtrace_string()});
#endif

				auto &stage = GetProfilerStage(tid, stageStack.top());
				stageStack.pop();
				return stage.Stop();
			}

			template<class TProfilingStage>
			void ProfilingStageManager<TProfilingStage>::InitializeProfilingStageManager(Profiler &profiler)
			{
				m_profiler = &profiler;
			}

			template<class TProfiler>
			std::shared_ptr<TProfiler> Profiler::Create()
			{
				auto profiler = std::shared_ptr<TProfiler> {new TProfiler {}};
				profiler->Initialize();
				return profiler;
			}

			template<class TProfilingStage>
			std::shared_ptr<TProfilingStage> ProfilingStage::Create(Profiler &profiler, std::thread::id tid, const std::string &name)
			{
				auto result = std::shared_ptr<TProfilingStage> {new TProfilingStage {profiler, tid, name}};
				result->Initialize();
				profiler.AddStage(*result);
				return result;
			}
		}
	}
};
