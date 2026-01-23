// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"

module pragma.client;

import :console.commands;

void CMD_debug_hdr_bloom(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	auto &wgui = pragma::gui::WGUI::GetInstance();
	auto *pRoot = wgui.GetBaseElement();
	if(pragma::get_cgame() == nullptr || argv.empty() || pRoot == nullptr)
		return;
	const std::string name = "debug_hdr_bloom";
	auto *pEl = pRoot->FindDescendantByName(name);
	auto v = pragma::util::to_int(argv.front());
	if(v == 0) {
		if(pEl != nullptr)
			pEl->Remove();
		return;
	}
	if(pEl != nullptr)
		return;
	pEl = wgui.Create<pragma::gui::types::WIDebugHDRBloom>();
	if(pEl == nullptr)
		return;
	pEl->SetName(name);
	pEl->SetSize(256, 256);
	pEl->SetZPos(std::numeric_limits<int>::max());
	pEl->Update();
}
namespace {
	auto UVN = pragma::console::client::register_command("debug_hdr_bloom", &CMD_debug_hdr_bloom, pragma::console::ConVarFlags::None, "Displays the scene bloom texture on screen. Usage: debug_hdr_bloom <1/0>");
}
