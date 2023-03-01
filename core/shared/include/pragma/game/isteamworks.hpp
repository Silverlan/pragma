/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __PR_ISTEAMWORKS_HPP__
#define __PR_ISTEAMWORKS_HPP__

#include <sharedutils/util_library.hpp>

#ifndef DLLSPEC_ISTEAMWORKS
#define DLLSPEC_ISTEAMWORKS
#endif

struct DLLSPEC_ISTEAMWORKS ISteamworks final {
	ISteamworks(util::Library &lib)
	    : initialize {lib.FindSymbolAddress<decltype(initialize)>("wv_steamworks_initialize")}, get_build_id {lib.FindSymbolAddress<decltype(get_build_id)>("wv_steamworks_get_build_id")}, shutdown {lib.FindSymbolAddress<decltype(shutdown)>("wv_steamworks_shutdown")},
	      update_subscribed_items {lib.FindSymbolAddress<decltype(update_subscribed_items)>("wv_steamworks_update_subscribed_items")}, subscribe_item {lib.FindSymbolAddress<decltype(subscribe_item)>("wv_steamworks_subscribe_item")}, unsubscribe_item {
	                                                                                                                                                                                                                                       lib.FindSymbolAddress<decltype(unsubscribe_item)>(
	                                                                                                                                                                                                                                         "wv_steamworks_unsubscribe_item")}
	{
		m_bValid = initialize != nullptr && get_build_id != nullptr && shutdown != nullptr;
	}
	ISteamworks() = default;
	bool (*const initialize)() = nullptr;
	int32_t (*const get_build_id)() = nullptr;
	void (*const shutdown)() = nullptr;
	void (*update_subscribed_items)() = nullptr;
	void (*subscribe_item)(uint64_t) = nullptr;
	void (*unsubscribe_item)(uint64_t) = nullptr;

	bool valid() const { return m_bValid; }
  private:
	bool m_bValid = false;
};

#endif
