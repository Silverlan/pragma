// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <sharedutils/util_weak_handle.hpp>

export module pragma.shared:entities.base_player;

export {
	namespace pragma {class BasePlayerComponent;};
	class DLLNETWORK BasePlayer {
	  public:
		BasePlayer() = default;
		virtual ~BasePlayer() = default;
		util::WeakHandle<pragma::BasePlayerComponent> GetBasePlayerComponent();
	  protected:
		util::WeakHandle<pragma::BasePlayerComponent> m_basePlayerComponent = {};
	};
};
