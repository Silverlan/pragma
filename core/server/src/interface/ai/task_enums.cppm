// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:ai.task_enums;

export import std.compat;

// These also have to be registered in SGame constructor!
export namespace pragma {
	namespace ai {
		enum class DLLSERVER Task : uint32_t { MoveToTarget = 0, PlayAnimation, PlayActivity, PlayLayeredAnimation, PlayLayeredActivity, MoveRandom, PlaySound, DebugPrint, DebugDrawText, Decorator, Wait, TurnToTarget, Random, LookAtTarget, Event };
	};
};
