/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __PRAGMA_INPUT_BINDING_LAYER_HPP__
#define __PRAGMA_INPUT_BINDING_LAYER_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/input/c_keybind.h"
#include <string>
#include <unordered_map>

struct DLLCLIENT InputBindingLayer
{
	static bool Save(const std::vector<std::shared_ptr<InputBindingLayer>> &layers,udm::AssetDataArg outData,std::string &outErr);
	static bool Load(const udm::AssetData &data,std::vector<std::shared_ptr<InputBindingLayer>> &outLayers,std::string &outErr);

	std::string identifier;
	uint32_t priority = 0;
	bool enabled = true;
	void MapKey(short c,std::string cmd);
	void MapKey(short c,luabind::function<> function);
	void MapKey(short c,std::unordered_map<std::string,std::vector<std::string>> &binds);
	void AddKeyMapping(short c,std::string cmd);
	void RemoveKeyMapping(short c,std::string cmd);
	void UnmapKey(short c);
	void ClearLuaKeyMappings();
	virtual std::unordered_map<short,KeyBind> &GetKeyMappings();
	const std::unordered_map<short,KeyBind> &GetKeyMappings() const {return const_cast<InputBindingLayer*>(this)->GetKeyMappings();}
	void ClearKeyMappings();
private:
	std::unordered_map<short,KeyBind> m_keybinds;
};

struct DLLCLIENT CoreInputBindingLayer
	: public InputBindingLayer
{
	virtual std::unordered_map<short,KeyBind> &GetKeyMappings() override;
};

#endif