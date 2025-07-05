// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "wms_message.h"
#include "wms_shared.h"

WMSMessageHeader::WMSMessageHeader(unsigned int _id) : id(_id), version(WMS_PROTOCOL_VERSION), size(0) {}

WMSMessageHeader::WMSMessageHeader() : WMSMessageHeader(0) {}
