/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "wms_shared.h"

std::string GetMasterServerIP()
{
	//#ifdef _DEBUG
	//	return "127.0.0.1";
	//#else
	return "85.214.192.20";
	//return "::1";
	//#endif
}
unsigned short GetMasterServerPort()
{
	//#ifdef _DEBUG
	//	return 27015;
	//#else
	return 29155;
	//#endif
}
