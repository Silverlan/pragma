// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __GIBLET_CREATE_INFO_HPP__
#define __GIBLET_CREATE_INFO_HPP__

#include "pragma/networkdefinitions.h"
#include <cinttypes>
#include <string>
#include <mathutil/uvec.h>

#pragma warning(push)
#pragma warning(disable : 4251)
#pragma pack(push, 1)
struct DLLNETWORK GibletCreateInfo {
	enum class DLLNETWORK PhysShape : uint32_t { Model = 0, None, Sphere, Box, Cylinder };
	std::string model;
	uint32_t skin = 0;
	float scale = 1.f;
	float mass = 5.f;
	float lifetime = 10.f;
	Vector3 position = {};
	Quat rotation = uquat::identity();
	Vector3 angularVelocity = {};
	Vector3 velocity = {};

	Vector3 physTranslationOffset = {};
	EulerAngles physRotationOffset = {};
	float physRadius = 0.0;
	float physHeight = 0.0;
	PhysShape physShape = PhysShape::Model;
};
#pragma pack(pop)
lua_registercheck(GibletCreateInfo, GibletCreateInfo);
#pragma warning(pop)

#endif
