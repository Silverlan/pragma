// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shader_graph.manager;

export import :rendering.shader_graph.sg_module;
export import pragma.shadergraph;

export namespace pragma::rendering {
	class DLLCLIENT ShaderGraphData {
	  public:
		ShaderGraphData(const std::string &typeName, const std::string &identifier, const std::shared_ptr<shadergraph::Graph> &graph) : m_typeName {typeName}, m_identifier {identifier}, m_graph {graph} {}
		const std::string &GetIdentifier() const { return m_identifier; }
		const std::string &GetTypeName() const { return m_typeName; }
		const std::shared_ptr<shadergraph::Graph> &GetGraph() const { return m_graph; }
		void GenerateGlsl();
	  private:
		std::string m_typeName;
		std::string m_identifier;
		std::shared_ptr<shadergraph::Graph> m_graph;
	};

	class ShaderGraphManager;
	class DLLCLIENT ShaderGraphTypeManager {
	  public:
		ShaderGraphTypeManager(const std::string &typeName, std::shared_ptr<shadergraph::NodeRegistry> nodeRegistry) : m_typeName {typeName}, m_nodeRegistry {nodeRegistry} {}
		void ReloadShader(const std::string &identifier);
		std::shared_ptr<ShaderGraphData> GetGraph(const std::string &identifier) const;
		const std::shared_ptr<shadergraph::NodeRegistry> &GetNodeRegistry() const { return m_nodeRegistry; }
		const std::unordered_map<std::string, std::shared_ptr<ShaderGraphData>> &GetGraphs() const { return m_graphs; }
	  private:
		friend ShaderGraphManager;
		std::shared_ptr<shadergraph::Graph> RegisterGraph(const std::string &identifier, std::shared_ptr<shadergraph::Graph> graph);
		std::shared_ptr<shadergraph::Graph> RegisterGraph(const std::string &identifier);
		std::shared_ptr<shadergraph::Graph> CreateGraph() const;
		std::string m_typeName;
		std::unordered_map<std::string, std::shared_ptr<ShaderGraphData>> m_graphs;
		std::shared_ptr<shadergraph::NodeRegistry> m_nodeRegistry;
	};

	class DLLCLIENT ShaderGraphManager {
	  public:
		static constexpr const char *ROOT_GRAPH_PATH = "scripts/shader_data/graphs/";
		static std::string GetShaderFilePath(const std::string &type, const std::string &identifier);
		static std::string GetShaderGraphFilePath(const std::string &type, const std::string &identifier);

		ShaderGraphManager();
		~ShaderGraphManager();
		const std::unordered_map<std::string, std::shared_ptr<ShaderGraphTypeManager>> &GetShaderGraphTypeManagers() const { return m_shaderGraphTypeManagers; }
		void RegisterGraphTypeManager(const std::string &type, std::shared_ptr<shadergraph::NodeRegistry> nodeRegistry);
		std::shared_ptr<shadergraph::Graph> RegisterGraph(const std::string &type, const std::string &identifier);
		std::shared_ptr<shadergraph::Graph> CreateGraph(const std::string &type) const;
		std::shared_ptr<shadergraph::Graph> LoadShader(const std::string &identifier, std::string &outErr, bool reload = false);
		void ReloadShader(const std::string &identifier);
		std::shared_ptr<ShaderGraphData> GetGraph(const std::string &identifier) const;
		void SyncGraph(const std::string &type, const std::string &identifier, const shadergraph::Graph &graph);
		std::shared_ptr<shadergraph::NodeRegistry> GetNodeRegistry(const std::string &type) const;

		ShaderGraphModuleManager &GetModuleManager();
		const ShaderGraphModuleManager &GetModuleManager() const;
	  private:
		std::unordered_map<std::string, std::shared_ptr<ShaderGraphTypeManager>> m_shaderGraphTypeManagers;
		std::unordered_map<std::string, std::string> m_shaderNameToType;
		std::unique_ptr<ShaderGraphModuleManager> m_moduleManager;
	};
}
