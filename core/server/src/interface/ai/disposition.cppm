// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"


export module pragma.server.ai.disposition;

export {
    enum class DISPOSITION : uint32_t { HATE, FEAR, NEUTRAL, LIKE, COUNT };
};
