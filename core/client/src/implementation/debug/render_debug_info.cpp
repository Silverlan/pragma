// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :debug.render_debug_info;
static pragma::debug::RenderDebugInfo render_debug_info;
void pragma::debug::RenderDebugInfo::Reset()
{
	triangleCount = 0;
	vertexCount = 0;
	meshCount = 0;
	entityCount = 0;
	materialCount = 0;
	shadowMeshCount = 0;
	shaderCount = 0;
	staticMeshCount = 0;
	shadowEntityCount = 0;
}

pragma::debug::RenderDebugInfo &pragma::debug::get_render_debug_info() { return render_debug_info; }

static void debug_render_info(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	auto &info = pragma::debug::get_render_debug_info();
	std::cout << "-------------- Render Info --------------" << std::endl;
	std::cout << "Rendered in last frame:" << std::endl;
	std::cout << "Triangles: " << info.triangleCount << std::endl;
	std::cout << "Vertices: " << info.vertexCount << std::endl;
	std::cout << "Entity changes " << info.entityCount << std::endl;
	std::cout << "Shader changes: " << info.shaderCount << std::endl;
	std::cout << "Material changes: " << info.materialCount << std::endl;
	std::cout << "Meshes: " << info.meshCount << " (" << ((CFloat(info.staticMeshCount) / CFloat(info.meshCount)) * 100.f) << "% static)" << std::endl;
	std::cout << "Average number of triangles per mesh: " << CUInt32(CDouble(info.triangleCount) / CDouble(info.meshCount)) << std::endl;
	std::cout << std::endl << "Shadow Info:" << std::endl;
	std::cout << "Shadow Entity changes: " << info.shadowEntityCount << std::endl;
	std::cout << "Shadow Meshes: " << info.shadowMeshCount << std::endl;
	std::cout << "-----------------------------------------" << std::endl;
}
namespace {
	auto UVN = pragma::console::client::register_command("debug_render_info", &debug_render_info, pragma::console::ConVarFlags::None, "Prints some timing information to the console.");
}
