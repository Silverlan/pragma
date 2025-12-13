// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:util.data_file;

export import pragma.filesystem;

export namespace pragma::util {
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
		static DataFileBlock *ReadBlock(fs::VFilePtr f);
		static DataFileBlock *Read(const char *f);
	};
};
