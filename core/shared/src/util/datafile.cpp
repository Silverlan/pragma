/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/util/datafile.h"
#include <sharedutils/util_string.h>

DataFileBlock *DataFile::Read(const char *fName)
{
	auto f = FileManager::OpenFile(fName, "r");
	DataFileBlock *block = ReadBlock(f);
	return block;
}

DataFileBlock *DataFile::ReadBlock(VFilePtr f)
{
	DataFileBlock *block = new DataFileBlock;
	while(!f->Eof()) {
		std::string sbuf = f->ReadLine();
		if(sbuf.length() > 0 && sbuf[0] != '\0') {
			ustring::remove_whitespace(sbuf);
			if(sbuf.length() > 0) {
				size_t cLast = sbuf.find_first_of(ustring::WHITESPACE);
				size_t cNext = sbuf.find_first_not_of(ustring::WHITESPACE, cLast);
				if(cNext != size_t(-1)) {
					std::string key, val;
					size_t stKey = sbuf.find('\"');
					size_t enKey = sbuf.find('\"', stKey + 1);
					size_t stVal = sbuf.find('\"', enKey + 1);
					size_t enVal = sbuf.find('\"', stVal + 1);
					key = sbuf.substr(stKey + 1, (enKey - stKey) - 1);
					val = sbuf.substr(stVal + 1, (enVal - stVal) - 1);
					block->keyvalues[key] = val;
				}
				else if(sbuf[0] == '}') // End of block
					return block;
				else // Sub-Block
				{
					ustring::remove_quotes(sbuf);
					char c;
					do
						c = static_cast<char>(f->ReadChar());
					while(c != '{' && c != EOF);
					std::unordered_map<std::string, std::vector<DataFileBlock *> *>::iterator i = block->blocks.find(sbuf);
					std::vector<DataFileBlock *> *blocks;
					if(i == block->blocks.end()) {
						blocks = new std::vector<DataFileBlock *>;
						block->blocks[sbuf] = blocks;
					}
					else
						blocks = i->second;
					DataFileBlock *sub = ReadBlock(f);
					blocks->push_back(sub);
				}
			}
		}
	}
	return block; // File ended prematurely? (Or this is the main block)
}
