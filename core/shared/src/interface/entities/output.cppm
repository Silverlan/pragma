// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.output;

export import std;

export struct DLLNETWORK Output {
	Output(std::string pentities, std::string pinput, std::string pparam, float pdelay, int ptimes) : entities(pentities), input(pinput), param(pparam), delay(pdelay), times(ptimes) {}
	Output() : entities(""), input(""), param(""), delay(0.f), times(-1) {}
	std::string entities;
	std::string input;
	std::string param;
	float delay;
	int times;
};
