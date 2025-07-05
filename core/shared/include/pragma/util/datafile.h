// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __DATAFILE_H__
#define __DATAFILE_H__
#include "pragma/definitions.h"
#include <unordered_map>
#include <vector>
#include <fsys/filesystem.h>

struct DLLNETWORK DataFileBlock {
	~DataFileBlock()
	{
		std::unordered_map<std::string, std::vector<DataFileBlock *> *>::iterator i;
		for(i = blocks.begin(); i != blocks.end(); i++) {
			for(int j = 0; j < i->second->size(); j++)
				delete(*i->second)[j];
			delete i->second;
		}
	}
	std::unordered_map<std::string, std::string> keyvalues;
	std::unordered_map<std::string, std::vector<DataFileBlock *> *> blocks;
};

class DLLNETWORK DataFile {
  private:
  public:
	static DataFileBlock *ReadBlock(VFilePtr f);
	static DataFileBlock *Read(const char *f);
};
#endif
