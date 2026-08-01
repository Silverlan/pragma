// SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.debug_tracker;

export import pragma.gui;

export namespace pragma::gui {
	DLLCLIENT void set_debug_tracking_enabled(const types::WIBase &el, bool enabled);
	DLLCLIENT const debug_tracker::Log *get_debug_tracking_log(const types::WIBase &el);
	DLLCLIENT void print_debug_tracking_log(const types::WIBase &el);
};
