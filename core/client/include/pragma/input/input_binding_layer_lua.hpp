/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

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
