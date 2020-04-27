/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "wms_request_filter.h"

RequestFilter operator|(const RequestFilter &a,const RequestFilter &b)
{
	return static_cast<RequestFilter>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
}
RequestFilter operator|=(const RequestFilter &a,const RequestFilter &b)
{
	return static_cast<RequestFilter>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
}
RequestFilter operator&(const RequestFilter &a,const RequestFilter &b)
{
	return static_cast<RequestFilter>(static_cast<unsigned int>(a) &static_cast<unsigned int>(b));
}
bool operator!(const RequestFilter &a) {return (static_cast<unsigned int>(a) == 0) ? true : false;}