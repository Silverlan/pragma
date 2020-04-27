/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include <sharedutils/util_file.h>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

#define WPT_VERSION 0x0001

// See c_lengine.cpp as well
bool CParticleSystemComponent::Save(VFilePtrReal &f,const std::vector<CParticleSystemComponent*> &particleSystems)
{
	for(auto *ps : particleSystems)
	{
		if(ps->IsRecordingKeyValues() == false)
			return false;
	}
	f->Write<char>('W');
	f->Write<char>('P');
	f->Write<char>('T');
	f->Write<unsigned int>(WPT_VERSION);
	f->Write<unsigned int>(CUInt32(particleSystems.size()));
	std::vector<unsigned long long> offsets;
	offsets.reserve(particleSystems.size());
	for(auto *ps : particleSystems)
	{
		f->WriteString(ps->GetParticleSystemName());
		offsets.push_back(f->Tell());
		f->Write<unsigned long long>((unsigned long long)(0));
	}
	auto i = 0ull;
	for(auto *ps : particleSystems)
	{
		unsigned long long offset = f->Tell();
		f->Seek(offsets.at(i++));
		f->Write<unsigned long long>(offset);
		f->Seek(offset);

		auto &keyValues = *ps->GetKeyValues();
		f->Write<unsigned int>(CUInt32(keyValues.size()));
		for(auto &pair : keyValues)
		{
			f->WriteString(pair.first);
			f->WriteString(pair.second);
		}
		auto &initializers = ps->m_initializers;
		f->Write<unsigned int>(CUInt32(initializers.size()));
		for(auto &initializer : initializers)
		{
			f->WriteString(initializer->GetName());
			auto &keyValues = *initializer->GetKeyValues();
			f->Write<unsigned int>(CUInt32(keyValues.size()));
			for(auto &pair : keyValues)
			{
				f->WriteString(pair.first);
				f->WriteString(pair.second);
			}
		}
		auto &operators = ps->m_operators;
		f->Write<unsigned int>(CUInt32(operators.size()));
		for(auto &op : operators)
		{
			f->WriteString(op->GetName());
			auto &keyValues = *op->GetKeyValues();
			f->Write<unsigned int>(CUInt32(keyValues.size()));
			for(auto &pair : keyValues)
			{
				f->WriteString(pair.first);
				f->WriteString(pair.second);
			}
		}
		auto &renderers = ps->m_renderers;
		f->Write<unsigned int>(CUInt32(renderers.size()));
		for(auto &renderer : renderers)
		{
			f->WriteString(renderer->GetName());
			auto &keyValues = *renderer->GetKeyValues();
			f->Write<unsigned int>(CUInt32(keyValues.size()));
			for(auto &pair : keyValues)
			{
				f->WriteString(pair.first);
				f->WriteString(pair.second);
			}
		}
		auto children = ps->GetChildren();
		for(auto it=children.begin();it!=children.end();)
		{
			auto &hChild = *it;
			if(hChild.valid())
			{
				++it;
				continue;
			}
			it = children.erase(it);
		}
		f->Write<unsigned char>(CUInt8(children.size()));
		for(auto &hChild : children)
			f->WriteString(hChild->GetParticleSystemName());
	}
	return true;
}
bool CParticleSystemComponent::Save(const std::string &fileName,const std::vector<CParticleSystemComponent*> &particleSystems)
{
	for(auto *ps : particleSystems)
	{
		if(ps->IsRecordingKeyValues() == false)
			return false;
	}
	auto name = FileManager::GetCanonicalizedPath(fileName);
	ustring::to_lower(name);
	name = "particles\\" +name;
	ufile::remove_extension_from_filename(name);
	name += ".wpt";
	auto f = FileManager::OpenFile<VFilePtrReal>(name.c_str(),"wb");
	if(f == nullptr)
		return false;
	return Save(f,particleSystems);
}
