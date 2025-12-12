// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.base_player;

export import pragma.util;

export {
	namespace pragma {
		class BasePlayerComponent;
	};
	class DLLNETWORK BasePlayer {
	  public:
		BasePlayer() = default;
		virtual ~BasePlayer() = default;
		pragma::util::WeakHandle<pragma::BasePlayerComponent> GetBasePlayerComponent();
	  protected:
		pragma::util::WeakHandle<pragma::BasePlayerComponent> m_basePlayerComponent = {};
	};
};
