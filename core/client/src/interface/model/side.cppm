// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "material.h"

export module pragma.client:model.side;

export {
  class DLLCLIENT CSide : public Side {
    public:
    CSide();
    public:
  };

  DLLCLIENT Con::c_cout &operator<<(Con::c_cout &os, const CSide side);
  DLLCLIENT std::ostream &operator<<(std::ostream &os, const CSide side);
};
