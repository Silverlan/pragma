// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.wms;

import :message;

WMSMessageHeader::WMSMessageHeader(unsigned int _id) : id(_id), version(WMS_PROTOCOL_VERSION), size(0) {}

WMSMessageHeader::WMSMessageHeader() : WMSMessageHeader(0) {}
