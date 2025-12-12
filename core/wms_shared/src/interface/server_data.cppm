// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "wms_shareddefinitions.hpp"

export module pragma.wms:server_data;

export import pragma.util;

export {
	#pragma warning(push)
	#pragma warning(disable : 4251)
	struct DLLWMSSHARED WMServerData {
		WMServerData();
		pragma::util::Version engineVersion;
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
		void Write(pragma::util::DataStream &stream) const;
		static void Read(pragma::util::DataStream &stream, WMServerData &data);
	};
	DLLWMSSHARED std::ostream &operator<<(std::ostream &out, const WMServerData &data);
	#pragma warning(pop)
}
