// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.wms;

import :request_filter;

RequestFilter operator|(const RequestFilter &a, const RequestFilter &b) { return static_cast<RequestFilter>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b)); }
RequestFilter operator|=(const RequestFilter &a, const RequestFilter &b) { return static_cast<RequestFilter>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b)); }
RequestFilter operator&(const RequestFilter &a, const RequestFilter &b) { return static_cast<RequestFilter>(static_cast<unsigned int>(a) & static_cast<unsigned int>(b)); }
bool operator!(const RequestFilter &a) { return (static_cast<unsigned int>(a) == 0) ? true : false; }
