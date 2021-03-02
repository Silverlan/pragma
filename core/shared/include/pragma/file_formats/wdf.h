/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __WDF_H__
#define __WDF_H__
#include "pragma/networkdefinitions.h"
#include <string>
#include <fsys/filesystem.h>
class DLLNETWORK FWDF
{
private:
protected:
	template<class T>
		T Read();
	void Read(void *ptr,size_t size);
	std::string ReadString();
	VFilePtr m_file;
public:
};

template<class T>
	inline T FWDF::Read()
{
	return m_file->Read<T>();
}

#endif