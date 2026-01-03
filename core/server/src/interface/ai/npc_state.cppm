// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:ai.tasks.npc_state;

export {
	enum class NPCSTATE : int { NONE, IDLE, ALERT, COMBAT, SCRIPT };
};
