/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __WMS_SHARED_H__
#define __WMS_SHARED_H__

#include <string>

#define WMS_PROTOCOL_VERSION 2

std::string GetMasterServerIP();
unsigned short GetMasterServerPort();

#endif