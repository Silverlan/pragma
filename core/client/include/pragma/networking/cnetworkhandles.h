/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#define CLIENT_NWHANDLES \
	void HandlePacket(NetPacket &packet); \
	void HandleError(ClientEvent ev,const boost::system::error_code &err); \
	void HandleTerminate(); \
	void HandleReceiveGameInfo(NetPacket &packet); \
	void HandleReceiveServerInfo(NetPacket &packet); \
	void HandleConnect(); \
	void HandleNewPlayer(NetPacket &packet); \
	void HandleResource(NetPacket &packet); \
	void HandleResourceFragment(NetPacket &packet);