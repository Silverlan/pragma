// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :util.data_file;

pragma::util::DataFileBlock *pragma::util::DataFile::Read(const char *fName)
{
	auto f = pragma::fs::open_file(fName, fs::FileMode::Read);
	DataFileBlock *block = ReadBlock(f);
	return block;
}

pragma::util::DataFileBlock *pragma::util::DataFile::ReadBlock(fs::VFilePtr f)
{
	DataFileBlock *block = new DataFileBlock;
	while(!f->Eof()) {
		std::string sbuf = f->ReadLine();
		if(sbuf.length() > 0 && sbuf[0] != '\0') {
			string::remove_whitespace(sbuf);
			if(sbuf.length() > 0) {
				size_t cLast = sbuf.find_first_of(string::WHITESPACE);
				size_t cNext = sbuf.find_first_not_of(string::WHITESPACE, cLast);
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
					string::remove_quotes(sbuf);
					char c;
					do
						c = static_cast<char>(f->ReadChar());
					while(c != '{' && c != std::char_traits<char>::eof());
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
