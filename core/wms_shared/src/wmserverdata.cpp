/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "wmserverdata.h"
#include "mathutil/umath.h"
#include "sharedutils/datastream.h"
#include <iostream>

WMServerData::WMServerData() : engineVersion {0, 0, 0}, tcpPort(0), udpPort(0), players(0), maxPlayers(0), bots(0), password(false) {}

void WMServerData::Write(DataStream &stream) const
{
	stream->Write<util::Version>(engineVersion);
	stream->Write<unsigned short>(tcpPort);
	stream->Write<unsigned short>(udpPort);
	stream->Write<unsigned short>(players);
	stream->Write<unsigned short>(maxPlayers);
	stream->Write<unsigned short>(bots);
	stream->WriteString(name);
	stream->WriteString(map);
	stream->WriteString(gameMode);
	stream->Write<bool>(password);
}
void WMServerData::Read(DataStream &stream, WMServerData &data)
{
	data.engineVersion = stream->Read<util::Version>();
	data.tcpPort = stream->Read<unsigned short>();
	data.udpPort = stream->Read<unsigned short>();
	data.players = stream->Read<unsigned short>();
	data.maxPlayers = stream->Read<unsigned short>();
	data.bots = stream->Read<unsigned short>();
	data.name = stream->ReadString();
	data.map = stream->ReadString();
	data.gameMode = stream->ReadString();
	data.password = stream->Read<bool>();
}

std::ostream &operator<<(std::ostream &out, const WMServerData &data)
{
	out << "IP: " << data.ip << ":[UDP:" << data.udpPort << "; TCP:" << data.tcpPort << "]" << std::endl;
	out << "Name: " << data.name << std::endl;
	out << "Engine Version: " << data.engineVersion.ToString() << std::endl;
	out << "Map: " << data.map << std::endl;
	out << "Gamemode: " << data.gameMode << std::endl;
	out << "Players: " << data.players << "/" << data.maxPlayers << " (" << data.bots << " bots)" << std::endl;
	out << "Password Protected: " << data.password << std::endl;
	return out;
}
