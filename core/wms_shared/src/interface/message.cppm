// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "wms_shareddefinitions.hpp"

export module pragma.wms:message;

export {
	enum class WMSMessage : unsigned int { FIRST = 1500, HEARTBEAT, REQUEST_SERVERS, LAST };

	struct DLLWMSSHARED WMSMessageHeader {
		WMSMessageHeader(unsigned int id);
		WMSMessageHeader();
		unsigned int version;
		unsigned int id;
		unsigned short size;
	};
}
