/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __WMSERVERDATA_H__
#define __WMSERVERDATA_H__

#include "wms_shareddefinitions.h"
#include <string>
#include <sharedutils/util_version.h>

class DataStream;
#pragma warning(push)
#pragma warning(disable : 4251)
struct DLLWMSSHARED WMServerData
{
	WMServerData();
	util::Version engineVersion;
	std::string ip;
	unsigned short tcpPort;
	unsigned short udpPort;
	unsigned short players;
	unsigned short maxPlayers;
	unsigned short bots;
	std::string name;
	std::string map;
	std::string gameMode;
	bool password;
	void Write(DataStream &stream) const;
	static void Read(DataStream &stream,WMServerData &data);
};
DLLWMSSHARED std::ostream &operator<<(std::ostream &out,const WMServerData &data);
#pragma warning(pop)

#endif
