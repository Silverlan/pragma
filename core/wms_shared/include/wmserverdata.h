// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __WMSERVERDATA_H__
#define __WMSERVERDATA_H__

#include "wms_shareddefinitions.h"
#include <string>
#include <sharedutils/util_version.h>

class DataStream;
#pragma warning(push)
#pragma warning(disable : 4251)
struct DLLWMSSHARED WMServerData {
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
	static void Read(DataStream &stream, WMServerData &data);
};
DLLWMSSHARED std::ostream &operator<<(std::ostream &out, const WMServerData &data);
#pragma warning(pop)

#endif
