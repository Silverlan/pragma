// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <string>
#include <unordered_set>

export module pragma.client.core.lua_input_binding_layer_register;

export namespace pragma {
	struct DLLCLIENT LuaInputBindingLayerRegister {
		void Add(InputBindingLayer &layer);
		void Remove(InputBindingLayer &Layer);
		~LuaInputBindingLayerRegister();
	  private:
		void Clear();
		std::unordered_set<InputBindingLayer *> m_layers;
	};
};
