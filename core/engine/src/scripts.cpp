#include "stdafx_engine.h"
#include "pragma/scripts.h"
#include <fsys/filesystem.h>
#include <sharedutils/util_string.h>

ScriptData::~ScriptData()
{
	std::unordered_map<std::string,ScriptValue*>::iterator i;
	for(i=m_values.begin();i!=m_values.end();i++)
		delete i->second;
}

bool ScriptData::Read(std::vector<ScriptData*> *data,const char *fName)
{
	std::string path(fName);
	std::transform(path.begin(),path.end(),path.begin(),::tolower);

	std::string pathTotal = "scripts\\";
	pathTotal += fName;

	return ReadData(data,pathTotal);
}

void ScriptData::ReadList(ScriptValue *sval,VFilePtr f)
{
	char buf[4096];
	while(f->ReadString(buf,4096))
	{
		std::string sbuf(buf);
		if(sbuf.length() > 0 && sbuf[0] != '\0')
		{
			ustring::remove_comment(sbuf);
			ustring::remove_whitespace(sbuf);
			if(sbuf.length() > 0)
			{
				if(sbuf.back() == '{')
					break;
			}
		}
	}
	while(f->ReadString(buf,4096))
	{
		std::string sbuf(buf);
		if(sbuf.length() > 0 && sbuf[0] != '\0')
		{
			ustring::remove_comment(sbuf);
			ustring::remove_whitespace(sbuf);
			if(sbuf.back() == '}')
				return;
			ustring::remove_quotes(sbuf);
			if(sbuf.length() > 0)
				sval->list->push_back(sbuf);
		}
	}
}

void ScriptData::ReadBlock(std::unordered_map<std::string,ScriptValue*> *values,VFilePtr f)
{
	char buf[4096];
	while(f->ReadString(buf,4096))
	{
		std::string sbuf(buf);
		if(sbuf.length() > 0 && sbuf[0] != '\0')
		{
			ustring::remove_comment(sbuf);
			ustring::remove_whitespace(sbuf);
			if(sbuf.length() > 0)
			{
				if(sbuf.back() == '}')
					return;
				size_t eq = sbuf.find('=');
				if(eq != ustring::NOT_FOUND)
				{
					std::string key = sbuf.substr(0,eq -1);
					std::transform(key.begin(),key.end(),key.begin(),::tolower);
					std::string val = sbuf.substr(eq +1,sbuf.length());
					ScriptValue *sv = new ScriptValue(ScriptValue::TYPE_KEYVALUE);
					std::unordered_map<std::string,ScriptValue*>::iterator i = values->find(key);
					if(i != values->end())
						delete (*values)[key];
					sv->value = new std::string(val);
					(*values)[key] = sv;
				}
				else
				{
					std::string key = sbuf;
					ScriptValue *sv = new ScriptValue(ScriptValue::TYPE_LIST);
					std::unordered_map<std::string,ScriptValue*>::iterator i = values->find(key);
					if(i != values->end())
						delete (*values)[key];
					(*values)[key] = sv;
					ReadList(sv,f);
				}
			}
		}
	}
}

bool ScriptData::ReadData(std::vector<ScriptData*> *data,std::string path)
{
	StringToLower(path);
	auto f = FileManager::OpenFile(path.c_str(),"r");
	if(f == NULL) return false;
	char buf[4096];
	std::unordered_map<std::string,ScriptValue*> *values;
	while(f->ReadString(buf,4096))
	{
		std::string sbuf(buf);
		if(sbuf.length() > 0 && sbuf[0] != '\0')
		{
			ustring::remove_comment(sbuf);
			ustring::remove_whitespace(sbuf);
			if(sbuf.length() > 0)
			{
				std::string name;
				if(sbuf.back() == '{')
					name = sbuf.substr(0,sbuf.length() -2);
				else
					name = sbuf;
				ustring::remove_quotes(name);
				while((sbuf.back() != '{') && f->ReadString(buf,4096))
				{
					sbuf = buf;
					ustring::remove_comment(sbuf);
					ustring::remove_whitespace(sbuf);
				}
				ScriptData *scriptData = new ScriptData;
				scriptData->GetValues(&values);
				scriptData->m_name = name;
				ReadBlock(values,f);
				data->push_back(scriptData);
			}
		}
	}
	return true;
}

void ScriptData::GetValues(std::unordered_map<std::string,ScriptValue*> **values) {*values = &m_values;}

std::string ScriptData::GetName() {return m_name;}

ScriptValue *ScriptData::GetValue(std::string key)
{
	std::unordered_map<std::string,ScriptValue*>::iterator i = m_values.find(key);
	if(i == m_values.end())
		return NULL;
	return i->second;
}