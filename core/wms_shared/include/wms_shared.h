// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __WMS_SHARED_H__
#define __WMS_SHARED_H__

#include <string>

#define WMS_PROTOCOL_VERSION 2

std::string GetMasterServerIP();
unsigned short GetMasterServerPort();

#endif
