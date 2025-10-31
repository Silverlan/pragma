// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"

export module pragma.shared:console.cvar_callback;

import :scripting.lua.function;

export {
	class ConVar;
	class NetworkState;
	class DLLNETWORK CvarCallback {
	  public:
		CvarCallback();
		CvarCallback(LuaFunction fc);
		CvarCallback(const std::function<void(NetworkState *, const ConVar &, const void *, const void *)> &f);
		void SetFunction(const std::function<void(NetworkState *, const ConVar &, const void *, const void *)> &f);
	  private:
		bool m_isLuaCallback = false;
		CallbackHandle m_callbackHandle;
	  public:
		bool IsLuaFunction() const;
		CallbackHandle &GetFunction();
	};
};
