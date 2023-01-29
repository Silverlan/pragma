/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __SCRIPTS_H__
#define __SCRIPTS_H__
#include "pragma/definitions.h"
#include <string>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <fsys/filesystem.h>

struct DLLNETWORK ScriptValue {
	enum TYPE { TYPE_KEYVALUE, TYPE_LIST };
	ScriptValue(TYPE type)
	{
		this->type = type;
		switch(type) {
		case TYPE_KEYVALUE:
			{
				value = new std::string;
				break;
			}
		case TYPE_LIST:
			{
				list = new std::vector<std::string>;
				break;
			}
		}
	}
	~ScriptValue()
	{
		switch(type) {
		case TYPE_KEYVALUE:
			{
				delete value;
				break;
			}
		case TYPE_LIST:
			{
				delete list;
				break;
			}
		}
	}
	ScriptValue::TYPE type;
	std::string *value;
	std::vector<std::string> *list;
};

class DLLNETWORK ScriptData {
  public:
	~ScriptData();
  private:
	static bool ReadData(std::vector<ScriptData *> *data, std::string path);
	static void ReadList(ScriptValue *sval, VFilePtr f);
	static void ReadBlock(std::unordered_map<std::string, ScriptValue *> *values, VFilePtr f);
	std::unordered_map<std::string, ScriptValue *> m_values;
	std::string m_name;
  public:
	static bool Read(std::vector<ScriptData *> *data, const char *fName);
	void GetValues(std::unordered_map<std::string, ScriptValue *> **values);
	ScriptValue *GetValue(std::string key);
	std::string GetName();
};
#endif
