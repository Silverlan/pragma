// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:util.timer;

export import :game.enums;
export import :scripting.lua.core;
export import :types;
export import :util.timer_handle;

export {
	class DLLNETWORK Timer {
	  public:
		Timer(float delay, unsigned int reps, LuaFunctionObject luaFunction, TimerType timetype = TimerType::CurTime);
		Timer(float delay, unsigned int reps, const CallbackHandle &hCallback, TimerType timetype = TimerType::CurTime);
		~Timer();
		void Update(pragma::Game *game);
		void Start(pragma::Game *game);
		void Pause();
		void Stop();
		void Remove(pragma::Game *game);
		bool IsValid();
		bool IsRunning();
		bool IsPaused();
		void InvalidateHandle(TimerHandle *hTimer);
		float GetTimeLeft();
		void SetTimeInterval(float time);
		float GetTimeInterval();
		unsigned int GetRepetitionsLeft();
		void SetRepetitions(unsigned int rep);
		std::shared_ptr<TimerHandle> CreateHandle();
		void SetCall(pragma::Game *game, LuaFunctionObject luaFunction);
		void SetCall(pragma::Game *game, const CallbackHandle &hCallback);

		void Call(pragma::Game *game);
	  private:
		TimerType m_timeType;
		float m_delay;
		unsigned int m_reps;
		LuaFunctionObject m_luaFunction {};
		CallbackHandle m_callback;
		double m_start;
		bool m_bRemove;
		bool m_bRunning;
		bool m_bIsValid;
		std::vector<std::shared_ptr<TimerHandle>> m_handles;

		double GetCurTime(pragma::Game *game);
		double GetDeltaTime(pragma::Game *game);
	  protected:
		float m_next;
		virtual void Reset();
		Timer();
	};
};
