/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shader_graph/module.hpp"

using namespace pragma::rendering;

void ShaderGraphModuleManager::RegisterFactory(const std::string &name, const Factory &factory) { m_factories[name] = factory; }
std::unique_ptr<ShaderGraphModule> ShaderGraphModuleManager::CreateModule(const std::string &name, ShaderGraph &shader, std::vector<pragma::shadergraph::GraphNode *> &&nodes) const
{
	auto it = m_factories.find(name);
	if(it == m_factories.end())
		return nullptr;
	auto &factory = it->second;
	auto module = factory(shader);
	if(!module)
		return nullptr;
	module->SetNodes(std::move(nodes));
	return module;
}
