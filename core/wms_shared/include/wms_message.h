// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __WMS_MESSAGE_H__
#define __WMS_MESSAGE_H__

#include "wms_shareddefinitions.h"

enum class DLLWMSSHARED WMSMessage : unsigned int { FIRST = 1500, HEARTBEAT, REQUEST_SERVERS, LAST };

struct DLLWMSSHARED WMSMessageHeader {
	WMSMessageHeader(unsigned int id);
	WMSMessageHeader();
	unsigned int version;
	unsigned int id;
	unsigned short size;
};

#endif
