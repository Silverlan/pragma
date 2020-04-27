/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __WMS_MESSAGE_H__
#define __WMS_MESSAGE_H__

#include "wms_shareddefinitions.h"

enum class DLLWMSSHARED WMSMessage : unsigned int
{
	FIRST=1500,
	HEARTBEAT,
	REQUEST_SERVERS,
	LAST
};

struct DLLWMSSHARED WMSMessageHeader
{
	WMSMessageHeader(unsigned int id);
	WMSMessageHeader();
	unsigned int version;
	unsigned int id;
	unsigned short size;
};

#endif