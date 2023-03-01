/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __OUTPUT_H__
#define __OUTPUT_H__

#include "pragma/networkdefinitions.h"
#include <string>

struct DLLNETWORK Output {
	Output(std::string pentities, std::string pinput, std::string pparam, float pdelay, int ptimes) : entities(pentities), input(pinput), param(pparam), delay(pdelay), times(ptimes) {}
	Output() : entities(""), input(""), param(""), delay(0.f), times(-1) {}
	std::string entities;
	std::string input;
	std::string param;
	float delay;
	int times;
};

#endif
