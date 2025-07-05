// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PRAGMA_INPUT_BINDING_LAYER_LUA_HPP__
#define __PRAGMA_INPUT_BINDING_LAYER_LUA_HPP__

#include "pragma/clientdefinitions.h"
#include <string>
#include <unordered_set>

struct InputBindingLayer;
namespace pragma {
	struct DLLCLIENT LuaInputBindingLayerRegister {
		void Add(InputBindingLayer &layer);
		void Remove(InputBindingLayer &Layer);
		~LuaInputBindingLayerRegister();
	  private:
		void Clear();
		std::unordered_set<InputBindingLayer *> m_layers;
	};
};

#endif
