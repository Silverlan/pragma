/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/model/animation/bone.hpp"

pragma::animation::Bone::Bone() : parent(), ID(0) {}

pragma::animation::Bone::Bone(const Bone &other) : ID(other.ID), name {other.name}, parent {}
{
	for(auto &pair : other.children)
		children[pair.first] = std::make_shared<Bone>(*pair.second);
#ifdef _MSC_VER
	static_assert(sizeof(Bone) == 112, "Update this function when making changes to this class!");
#endif
}

bool pragma::animation::Bone::IsAncestorOf(const Bone &other) const
{
	if(other.parent.expired())
		return false;
	auto *parent = other.parent.lock().get();
	return (parent == this) ? true : IsAncestorOf(*parent);
}
bool pragma::animation::Bone::IsDescendantOf(const Bone &other) const { return other.IsAncestorOf(*this); }
bool pragma::animation::Bone::operator==(const Bone &other) const
{
#ifdef _MSC_VER
	static_assert(sizeof(Bone) == 112, "Update this function when making changes to this class!");
#endif
	if(!(name == other.name && ID == other.ID && children.size() == other.children.size() && parent.expired() == other.parent.expired()))
		return false;
	for(auto &pair : children) {
		if(other.children.find(pair.first) == other.children.end())
			return false;
	}
	return true;
}

std::ostream &operator<<(std::ostream &out, const pragma::animation::Bone &o)
{
	out << "Bone";
	out << "[Name:" << o.name << "]";
	out << "[Id:" << o.name << "]";
	out << "[Children:" << o.children.size() << "]";
	out << "[Parent:";
	if(o.parent.expired())
		out << "NULL";
	else
		out << o.parent.lock()->name;
	out << "]";
	return out;
}
