/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __SHADER_GRAPH_MANAGER_HPP__
#define __SHADER_GRAPH_MANAGER_HPP__

#include <pragma/clientdefinitions.h>
#include <string>
#include <unordered_map>
#include <memory>

import pragma.shadergraph;

namespace pragma::rendering {
	class DLLCLIENT ShaderGraphData {
	  public:
		ShaderGraphData(const std::string &typeName, const std::string &identifier, const std::shared_ptr<pragma::shadergraph::Graph> &graph) : m_typeName {typeName}, m_identifier {identifier}, m_graph {graph} {}
		const std::string &GetIdentifier() const { return m_identifier; }
		const std::string &GetTypeName() const { return m_typeName; }
		const std::shared_ptr<pragma::shadergraph::Graph> &GetGraph() const { return m_graph; }
		void GenerateGlsl();
	  private:
		std::string m_typeName;
		std::string m_identifier;
		std::shared_ptr<pragma::shadergraph::Graph> m_graph;
	};

	class ShaderGraphManager;
	class DLLCLIENT ShaderGraphTypeManager {
	  public:
		ShaderGraphTypeManager(const std::string &typeName, std::shared_ptr<pragma::shadergraph::NodeRegistry> nodeRegistry) : m_typeName {typeName}, m_nodeRegistry {nodeRegistry} {}
		void ReloadShader(const std::string &identifier);
		std::shared_ptr<ShaderGraphData> GetGraph(const std::string &identifier) const;
		const std::shared_ptr<pragma::shadergraph::NodeRegistry> &GetNodeRegistry() const { return m_nodeRegistry; }
		const std::unordered_map<std::string, std::shared_ptr<ShaderGraphData>> &GetGraphs() const { return m_graphs; }
	  private:
		friend ShaderGraphManager;
		std::shared_ptr<pragma::shadergraph::Graph> RegisterGraph(const std::string &identifier, std::shared_ptr<pragma::shadergraph::Graph> graph);
		std::shared_ptr<pragma::shadergraph::Graph> RegisterGraph(const std::string &identifier);
		std::shared_ptr<pragma::shadergraph::Graph> CreateGraph() const;
		std::string m_typeName;
		std::unordered_map<std::string, std::shared_ptr<ShaderGraphData>> m_graphs;
		std::shared_ptr<pragma::shadergraph::NodeRegistry> m_nodeRegistry;
	};

	class ShaderGraphModuleManager;
	class DLLCLIENT ShaderGraphManager {
	  public:
		static constexpr const char *ROOT_GRAPH_PATH = "scripts/shader_data/graphs/";
		static std::string GetShaderFilePath(const std::string &type, const std::string &identifier);
		static std::string GetShaderGraphFilePath(const std::string &type, const std::string &identifier);

		ShaderGraphManager();
		~ShaderGraphManager();
		const std::unordered_map<std::string, std::shared_ptr<ShaderGraphTypeManager>> &GetShaderGraphTypeManagers() const { return m_shaderGraphTypeManagers; }
		void RegisterGraphTypeManager(const std::string &type, std::shared_ptr<pragma::shadergraph::NodeRegistry> nodeRegistry);
		std::shared_ptr<pragma::shadergraph::Graph> RegisterGraph(const std::string &type, const std::string &identifier);
		std::shared_ptr<pragma::shadergraph::Graph> CreateGraph(const std::string &type) const;
		std::shared_ptr<pragma::shadergraph::Graph> LoadShader(const std::string &identifier, std::string &outErr, bool reload = false);
		void ReloadShader(const std::string &identifier);
		std::shared_ptr<ShaderGraphData> GetGraph(const std::string &identifier) const;
		void SyncGraph(const std::string &type, const std::string &identifier, const pragma::shadergraph::Graph &graph);
		std::shared_ptr<pragma::shadergraph::NodeRegistry> GetNodeRegistry(const std::string &type) const;

		ShaderGraphModuleManager &GetModuleManager();
		const ShaderGraphModuleManager &GetModuleManager() const;
	  private:
		std::unordered_map<std::string, std::shared_ptr<ShaderGraphTypeManager>> m_shaderGraphTypeManagers;
		std::unordered_map<std::string, std::string> m_shaderNameToType;
		std::unique_ptr<ShaderGraphModuleManager> m_moduleManager;
	};
}

#endif
