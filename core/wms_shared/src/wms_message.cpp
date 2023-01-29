/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "wms_message.h"
#include "wms_shared.h"

WMSMessageHeader::WMSMessageHeader(unsigned int _id) : id(_id), version(WMS_PROTOCOL_VERSION), size(0) {}

WMSMessageHeader::WMSMessageHeader() : WMSMessageHeader(0) {}
