// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.client:model.side;

export import pragma.shared;

export {
	class DLLCLIENT CSide : public Side {
	  public:
		CSide();
	  public:
	};

	DLLCLIENT Con::c_cout &operator<<(Con::c_cout &os, const CSide side);
	DLLCLIENT std::ostream &operator<<(std::ostream &os, const CSide side);
};
