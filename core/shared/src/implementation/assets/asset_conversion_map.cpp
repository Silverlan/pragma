// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :assets.conversion;

bool util::port_source2_map(pragma::NetworkState *nw, const std::string &path)
{
	static auto *ptrConvertMap = reinterpret_cast<bool (*)(pragma::Game &, const std::string &)>(impl::get_module_func(nw, "convert_source2_map"));
	if(ptrConvertMap == nullptr)
		return false;
	auto lockWatcher = pragma::Engine::Get()->ScopeLockResourceWatchers();
	return ptrConvertMap(*nw->GetGameState(), path);
}

bool util::port_hl2_map(pragma::NetworkState *nw, const std::string &path)
{
	static auto *ptrConvertMap = reinterpret_cast<bool (*)(pragma::Game &, const std::string &)>(impl::get_module_func(nw, "convert_hl2_map"));
	if(ptrConvertMap == nullptr)
		return false;
	auto lockWatcher = pragma::Engine::Get()->ScopeLockResourceWatchers();
	return ptrConvertMap(*nw->GetGameState(), path);
}
