// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:console.convar_handle;

import :console.convar;

export namespace pragma::console {
	class DLLNETWORK PtrConVar {
	  private:
		ConVar *m_cvar;
	  public:
		PtrConVar();
		ConVar *get();
		void set(ConVar *cvar);
	};

	///////////////////////

#pragma warning(push)
#pragma warning(disable : 4251)
	class DLLNETWORK ConVarHandle {
	  private:
		std::shared_ptr<PtrConVar> m_cvar;
	  public:
		ConVarHandle(std::shared_ptr<PtrConVar> &sptr);
		ConVar *operator->();
	};
#pragma warning(pop)

	typedef ConVarHandle CVar;
};
