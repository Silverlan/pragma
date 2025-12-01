// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

export module pragma.wms:core;

export import std.compat;

export {
    constexpr uint32_t WMS_PROTOCOL_VERSION = 2;
    std::string GetMasterServerIP();
    unsigned short GetMasterServerPort();
}
