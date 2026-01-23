// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "wms_shareddefinitions.hpp"

export module pragma.wms:request_filter;

export {
    enum class RequestFilter : unsigned int { NONE = 0, OR = 1, AND = 2, NOT_EMPTY = 4, NOT_FULL = 8, EMPTY = 16, NO_PASSWORD = 32 };
    DLLWMSSHARED RequestFilter operator&(const RequestFilter &a, const RequestFilter &b);
    DLLWMSSHARED RequestFilter operator|(const RequestFilter &a, const RequestFilter &b);
    DLLWMSSHARED RequestFilter operator|=(const RequestFilter &a, const RequestFilter &b);
    DLLWMSSHARED bool operator!(const RequestFilter &a);
}
