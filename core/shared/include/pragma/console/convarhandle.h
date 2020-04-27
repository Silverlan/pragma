/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __CONVARHANDLE_H__
#define __CONVARHANDLE_H__

#include "pragma/networkdefinitions.h"
#include <memory>
class DLLNETWORK ConVar;
class DLLNETWORK PtrConVar
{
private:
	ConVar *m_cvar;
public:
	PtrConVar();
	ConVar *get();
	void set(ConVar *cvar);
};

///////////////////////

#pragma warning(push)
#pragma warning(disable : 4251)
class DLLNETWORK ConVarHandle
{
private:
	std::shared_ptr<PtrConVar> m_cvar;
public:
	ConVarHandle(std::shared_ptr<PtrConVar> &sptr);
	ConVar *operator->();
};
#pragma warning(pop)

typedef ConVarHandle CVar;

#endif