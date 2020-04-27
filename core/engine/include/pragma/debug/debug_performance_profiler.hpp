/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __DEBUG_PERFORMANCE_PROFILER_HPP__
#define __DEBUG_PERFORMANCE_PROFILER_HPP__

#include "pragma/definitions.h"
#include <mathutil/umath.h>
#include <sharedutils/util_clock.hpp>
#include <optional>
#include <memory>
#include <unordered_map>
#include <chrono>
#include <array>
#include <set>

namespace pragma
{
	namespace debug
	{
		struct DLLENGINE ProfilerResult
		{
			std::optional<std::chrono::nanoseconds> duration;
		};

		class DLLENGINE Timer
		{
		public:
			Timer(const Timer&)=delete;
			Timer &operator=(const Timer&)=delete;
			virtual ~Timer()=default;
			virtual bool Start()=0;
			virtual bool Stop()=0;
			virtual std::unique_ptr<ProfilerResult> GetResult() const=0;
		protected:
			Timer()=default;
		};

		class Profiler;
		class DLLENGINE ProfilingStage
			: public std::enable_shared_from_this<ProfilingStage>
		{
		public:
			using StageId = uint32_t;
			template<class TProfilingStage>
				static std::shared_ptr<TProfilingStage> Create(Profiler &profiler,const std::string &name,TProfilingStage *parent=nullptr);
			static std::shared_ptr<ProfilingStage> Create(Profiler &profiler,const std::string &name,ProfilingStage *parent=nullptr);
			virtual ~ProfilingStage()=default;
			ProfilingStage(const ProfilingStage&)=delete;
			ProfilingStage &operator=(const ProfilingStage&)=delete;
			virtual void Initialize(ProfilingStage *parent);

			bool Start();
			bool Stop();
			std::unique_ptr<ProfilerResult> GetResult() const;

			StageId GetStageId() const;
			const std::string &GetName() const;
			Profiler &GetProfiler();
			ProfilingStage *GetParent();
			const std::vector<std::weak_ptr<ProfilingStage>> &GetChildren() const;
			const Timer &GetTimer() const;
			Timer &GetTimer();
		protected:
			ProfilingStage(Profiler &profiler,const std::string &name);
			virtual void InitializeTimer();
			std::shared_ptr<Timer> m_timer;
		private:
			Profiler &m_profiler;
			std::weak_ptr<ProfilingStage> m_parent = {};
			std::vector<std::weak_ptr<ProfilingStage>> m_children = {};
			StageId m_stage;
			std::string m_name;
		};

		class CPUProfiler;
		class DLLENGINE Profiler
		{
		public:
			template<class TProfiler>
				static std::shared_ptr<TProfiler> Create();
			virtual std::shared_ptr<Timer> CreateTimer() {return nullptr;};
			virtual void Initialize();

			bool StartStage(ProfilingStage::StageId stage);
			bool StopStage(ProfilingStage::StageId stage);

			const ProfilingStage &GetRootStage() const;
			ProfilingStage &GetRootStage();
			ProfilingStage *GetStage(ProfilingStage::StageId stage);
			const std::vector<std::weak_ptr<ProfilingStage>> &GetStages() const;
			void AddStage(ProfilingStage &stage);
		protected:
			Profiler()=default;
			std::shared_ptr<ProfilingStage> m_rootStage = nullptr;
		private:
			ProfilingStage::StageId m_nextStageId = 0u;
			std::vector<std::weak_ptr<ProfilingStage>> m_stages = {};
			friend std::shared_ptr<ProfilingStage> ProfilingStage::Create(Profiler &profiler,const std::string &name,ProfilingStage *parent);
		};

		template<class TProfilingStage,typename TPhaseEnum>
			class ProfilingStageManager
		{
		public:
			TProfilingStage &GetProfilerStage(TPhaseEnum phase);
			bool StartProfilerStage(TPhaseEnum phase);
			bool StopProfilerStage(TPhaseEnum phase);
			void InitializeProfilingStageManager(Profiler &profiler,const std::array<std::shared_ptr<TProfilingStage>,umath::to_integral(TPhaseEnum::Count)> &stages);
		protected:
			std::array<std::shared_ptr<TProfilingStage>,umath::to_integral(TPhaseEnum::Count)> m_profilingStages = {};
		private:
			Profiler *m_profiler = nullptr;
		};

		////////

		class DLLENGINE CPUTimer
			: public Timer
		{
		public:
			static std::shared_ptr<CPUTimer> Create();
			virtual bool Start() override;
			virtual bool Stop() override;
			virtual std::unique_ptr<ProfilerResult> GetResult() const override;
		private:
			CPUTimer()=default;
			util::Clock::time_point m_startTime = {};
			util::Clock::time_point m_stopTime = {};
		};

		class DLLENGINE CPUProfiler
			: public Profiler
		{
		public:
			CPUProfiler()=default;
			virtual std::shared_ptr<Timer> CreateTimer() override;
		};
	};
};

template<class TProfilingStage,typename TPhaseEnum>
	TProfilingStage &pragma::debug::ProfilingStageManager<TProfilingStage,TPhaseEnum>::GetProfilerStage(TPhaseEnum phase)
{
	return *m_profilingStages.at(umath::to_integral(phase));
}

template<class TProfilingStage,typename TPhaseEnum>
	bool pragma::debug::ProfilingStageManager<TProfilingStage,TPhaseEnum>::StartProfilerStage(TPhaseEnum phase)
{
	auto &stage = GetProfilerStage(phase);
	return stage.Start();
}

template<class TProfilingStage,typename TPhaseEnum>
	bool pragma::debug::ProfilingStageManager<TProfilingStage,TPhaseEnum>::StopProfilerStage(TPhaseEnum phase)
{
	auto &stage = GetProfilerStage(phase);
	return stage.Stop();
}

template<class TProfilingStage,typename TPhaseEnum>
	void pragma::debug::ProfilingStageManager<TProfilingStage,TPhaseEnum>::InitializeProfilingStageManager(Profiler &profiler,const std::array<std::shared_ptr<TProfilingStage>,umath::to_integral(TPhaseEnum::Count)> &stages)
{
	m_profiler = &profiler;
	m_profilingStages = stages;
}

template<class TProfiler>
	std::shared_ptr<TProfiler> pragma::debug::Profiler::Create()
{
	auto profiler = std::shared_ptr<TProfiler>{new TProfiler{}};
	profiler->Initialize();
	return profiler;
}

template<class TProfilingStage>
	std::shared_ptr<TProfilingStage> pragma::debug::ProfilingStage::Create(Profiler &profiler,const std::string &name,TProfilingStage *parent)
{
	if(parent == nullptr)
		parent = static_cast<TProfilingStage*>(&profiler.GetRootStage());
	auto result = std::shared_ptr<TProfilingStage>{new TProfilingStage{profiler,name}};
	result->Initialize(parent);
	profiler.AddStage(*result);
	return result;
}

#endif
