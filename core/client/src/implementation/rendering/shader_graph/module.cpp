// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shader_graph.sg_module;

using namespace pragma::rendering;

void ShaderGraphModuleManager::RegisterFactory(const std::string &name, const Factory &factory) { m_factories[name] = factory; }
std::unique_ptr<ShaderGraphModule> ShaderGraphModuleManager::CreateModule(const std::string &name, ShaderGraph &shader, std::vector<shadergraph::GraphNode *> &&nodes) const
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
