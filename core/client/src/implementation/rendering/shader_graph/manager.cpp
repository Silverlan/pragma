// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shader_graph.manager;

import :engine;
import :rendering.shaders;

using namespace pragma::rendering;

std::shared_ptr<pragma::shadergraph::Graph> ShaderGraphTypeManager::RegisterGraph(const std::string &identifier, std::shared_ptr<shadergraph::Graph> graph)
{
	auto it = m_graphs.find(identifier);
	if(it != m_graphs.end()) {
		// We'll keep the existing graph to prevent mismatches with already
		// registered shaders.
		auto curGraph = it->second->GetGraph();
		curGraph->Clear();
		curGraph->Merge(*graph);
		graph = curGraph;
	}
	auto fragFilePath = util::FilePath(ShaderGraphManager::GetShaderFilePath(m_typeName, identifier));
	fragFilePath.PopFront();
	auto strFragFilePath = fragFilePath.GetString();
	auto &shaderManager = get_cengine()->GetShaderManager();
	auto shader = shaderManager.GetShader(identifier);
	auto graphData = pragma::util::make_shared<ShaderGraphData>(m_typeName, identifier, graph);
	if(shader.valid()) {
		auto *sgShader = dynamic_cast<ShaderGraph *>(shader.get());
		if(!sgShader)
			throw std::runtime_error {"Shader '" + identifier + "' already exists and is not a ShaderGraph!"};
	}
	else {
		auto path = ShaderGraphManager::GetShaderFilePath(m_typeName, identifier);
		if(!fs::exists(path))
			graphData->GenerateGlsl();
		shaderManager.RegisterShader(identifier, [strFragFilePath, graph](prosper::IPrContext &context, const std::string &identifier) { return new ShaderGraph {context, graph, identifier, strFragFilePath}; });
	}
	m_graphs[identifier] = graphData;
	return graph;
}

std::shared_ptr<pragma::shadergraph::Graph> ShaderGraphTypeManager::RegisterGraph(const std::string &identifier)
{
	auto graph = CreateGraph();
	RegisterGraph(identifier, graph);
	return graph;
}

std::shared_ptr<pragma::shadergraph::Graph> ShaderGraphTypeManager::CreateGraph() const { return pragma::util::make_shared<shadergraph::Graph>(m_nodeRegistry); }

void ShaderGraphTypeManager::ReloadShader(const std::string &identifier)
{
	auto graphData = GetGraph(identifier);
	if(!graphData)
		return;
	graphData->GenerateGlsl();
}

std::shared_ptr<ShaderGraphData> ShaderGraphTypeManager::GetGraph(const std::string &identifier) const
{
	auto it = m_graphs.find(identifier);
	if(it != m_graphs.end())
		return it->second;
	return nullptr;
}

std::string ShaderGraphManager::GetShaderFilePath(const std::string &type, const std::string &identifier)
{
	std::string shaderName = identifier;
	auto path = util::FilePath("shaders/programs/graph_shaders/", type, shaderName + ".frag");
	return path.GetString();
}
std::string ShaderGraphManager::GetShaderGraphFilePath(const std::string &type, const std::string &identifier)
{
	auto path = util::FilePath("scripts/shader_data/graphs/", type, identifier);
	auto strPath = path.GetString();
	if(!ufile::compare_extension(strPath, {shadergraph::Graph::EXTENSION_BINARY, shadergraph::Graph::EXTENSION_ASCII}))
		strPath += "." + std::string {shadergraph::Graph::EXTENSION_ASCII};
	return strPath;
}
ShaderGraphManager::ShaderGraphManager() : m_moduleManager {std::make_unique<ShaderGraphModuleManager>()} {}
ShaderGraphManager::~ShaderGraphManager() {}
ShaderGraphModuleManager &ShaderGraphManager::GetModuleManager() { return *m_moduleManager; }
const ShaderGraphModuleManager &ShaderGraphManager::GetModuleManager() const { return *m_moduleManager; }
void ShaderGraphManager::RegisterGraphTypeManager(const std::string &type, std::shared_ptr<shadergraph::NodeRegistry> nodeRegistry) { m_shaderGraphTypeManagers[type] = pragma::util::make_shared<ShaderGraphTypeManager>(type, nodeRegistry); }
std::shared_ptr<pragma::shadergraph::Graph> ShaderGraphManager::RegisterGraph(const std::string &type, const std::string &identifier)
{
	auto itType = m_shaderNameToType.find(identifier);
	if(itType != m_shaderNameToType.end() && itType->second != type)
		throw std::runtime_error {"Shader '" + identifier + "' already registered with type '" + itType->second + "'!"};
	auto it = m_shaderGraphTypeManagers.find(type);
	if(it == m_shaderGraphTypeManagers.end())
		return nullptr;
	auto graph = it->second->RegisterGraph(identifier);
	if(!graph)
		return nullptr;
	m_shaderNameToType[identifier] = type;
	return graph;
}
std::shared_ptr<pragma::shadergraph::Graph> ShaderGraphManager::CreateGraph(const std::string &type) const
{
	auto it = m_shaderGraphTypeManagers.find(type);
	if(it == m_shaderGraphTypeManagers.end())
		return nullptr;
	return it->second->CreateGraph();
}
std::shared_ptr<pragma::shadergraph::Graph> ShaderGraphManager::LoadShader(const std::string &identifier, std::string &outErr, bool reload)
{
	auto graph = GetGraph(identifier);
	if(graph && !reload)
		return graph->GetGraph();
	for(auto &[typeName, typeManager] : m_shaderGraphTypeManagers) {
		auto path = GetShaderGraphFilePath(typeName, identifier);
		if(!fs::exists(path))
			continue;
		auto graph = pragma::util::make_shared<shadergraph::Graph>(typeManager->GetNodeRegistry());
		auto result = graph->Load(path, outErr);
		if(!result)
			return nullptr;
		graph = typeManager->RegisterGraph(identifier, graph);
		m_shaderNameToType[identifier] = typeName;
		return graph;
	}
	outErr = "Failed to load shader graph '" + identifier + "': No shader graph file found!";
	return nullptr;
}
void ShaderGraphManager::ReloadShader(const std::string &identifier)
{
	auto itType = m_shaderNameToType.find(identifier);
	if(itType == m_shaderNameToType.end())
		return;
	auto &type = itType->second;
	auto it = m_shaderGraphTypeManagers.find(type);
	if(it == m_shaderGraphTypeManagers.end())
		return;
	it->second->ReloadShader(identifier);
}
std::shared_ptr<ShaderGraphData> ShaderGraphManager::GetGraph(const std::string &identifier) const
{
	auto itType = m_shaderNameToType.find(identifier);
	if(itType == m_shaderNameToType.end())
		return nullptr;
	auto &type = itType->second;
	auto it = m_shaderGraphTypeManagers.find(type);
	if(it == m_shaderGraphTypeManagers.end())
		return nullptr;
	return it->second->GetGraph(identifier);
}
void ShaderGraphManager::SyncGraph(const std::string &type, const std::string &identifier, const shadergraph::Graph &graph)
{
	auto graphData = GetGraph(identifier);
	if(!graphData) {
		auto itType = m_shaderNameToType.find(identifier);
		if(itType == m_shaderNameToType.end())
			return;
		auto it = m_shaderGraphTypeManagers.find(type);
		if(it == m_shaderGraphTypeManagers.end())
			return;
		auto newGraph = pragma::util::make_shared<shadergraph::Graph>(graph.GetNodeRegistry());
		it->second->RegisterGraph(identifier, newGraph);
		graphData = GetGraph(identifier);
		if(!graphData)
			return;
	}
	auto &curGraph = graphData->GetGraph();
	curGraph->Clear();
	curGraph->Merge(graph);
}

std::shared_ptr<pragma::shadergraph::NodeRegistry> ShaderGraphManager::GetNodeRegistry(const std::string &type) const
{
	auto it = m_shaderGraphTypeManagers.find(type);
	if(it == m_shaderGraphTypeManagers.end())
		return nullptr;
	return it->second->GetNodeRegistry();
}

void ShaderGraphData::GenerateGlsl()
{
	auto placeholder = fs::read_file("shaders/graph_placeholder.frag");
	if(!placeholder)
		return;

	std::string shaderName = m_identifier;
	std::ostringstream header;
	std::ostringstream body;
	m_graph->GenerateGlsl(header, body);

	auto strHeader = header.str();
	auto strBody = body.str();

	string::replace(strBody, "\n", "\n\t");

	auto code = *placeholder;
	string::replace(code, "{{header}}", strHeader);
	string::replace(code, "{{body}}", strBody);

	auto path = ShaderGraphManager::GetShaderFilePath(m_typeName, m_identifier);
	fs::create_path(ufile::get_path_from_filename(path));
	std::string err;
	auto f = fs::open_file<fs::VFilePtrReal>(path, fs::FileMode::Write, &err);
	if(!f)
		return;
	f->WriteString(code);
	f = {};
}
