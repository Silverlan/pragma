#include "pragma/entities/environment/effects/particlesystemdata.h"
#include <fsys/filesystem.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util_file.h>


#define WPT_VERSION 0x0001
void pragma::asset::get_particle_system_file_path(std::string &path)
{
	path = FileManager::GetCanonicalizedPath(path);
	ustring::to_lower(path);
	if(ustring::compare(path.c_str(),(std::string{"particles"} +FileManager::GetDirectorySeparator()).c_str(),false,10) == false)
		path = "particles\\" +path;
	ufile::remove_extension_from_filename(path);
	path += ".wpt";
}
// See c_particlesystem_save.cpp as well
bool pragma::asset::save_particle_system(const std::string &name,const std::unordered_map<std::string,CParticleSystemData> &particles)
{
	auto ptPath = name;
	get_particle_system_file_path(ptPath);
	FileManager::CreatePath(ufile::get_path_from_filename(ptPath).c_str());
	auto f = FileManager::OpenFile<VFilePtrReal>(ptPath.c_str(),"wb");
	if(f == NULL)
		return false;
	f->Write<char>('W');
	f->Write<char>('P');
	f->Write<char>('T');
	f->Write<unsigned int>(WPT_VERSION);
	f->Write<unsigned int>(CUInt32(particles.size()));
	std::vector<unsigned long long> offsets;
	for(auto &pair : particles)
	{
		f->WriteString(pair.first);
		offsets.push_back(f->Tell());
		f->Write<unsigned long long>((unsigned long long)(0));
	}
	unsigned int i = 0;
	for(auto &pair : particles)
	{
		unsigned long long offset = f->Tell();
		f->Seek(offsets[i]);
		f->Write<unsigned long long>(offset);
		f->Seek(offset);

		auto &data = pair.second;
		f->Write<unsigned int>(CUInt32(data.settings.size()));
		for(auto &pair : data.settings)
		{
			f->WriteString(pair.first);
			f->WriteString(pair.second);
		}
		f->Write<unsigned int>(CUInt32(data.initializers.size()));
		for(unsigned int k=0;k<data.initializers.size();k++)
		{
			auto &modData = data.initializers[k];
			f->WriteString(modData.name);
			f->Write<unsigned int>(CUInt32(modData.settings.size()));
			for(auto &pair : modData.settings)
			{
				f->WriteString(pair.first);
				f->WriteString(pair.second);
			}
		}
		f->Write<unsigned int>(CUInt32(data.operators.size()));
		for(unsigned int k=0;k<data.operators.size();k++)
		{
			auto &modData = data.operators[k];
			f->WriteString(modData.name);
			f->Write<unsigned int>(CUInt32(modData.settings.size()));
			std::unordered_map<std::string,std::string>::iterator l;
			for(auto &pair : modData.settings)
			{
				f->WriteString(pair.first);
				f->WriteString(pair.second);
			}
		}
		f->Write<unsigned int>(CUInt32(data.renderers.size()));
		for(unsigned int k=0;k<data.renderers.size();k++)
		{
			auto &modData = data.renderers[k];
			f->WriteString(modData.name);
			f->Write<unsigned int>(CUInt32(modData.settings.size()));
			for(auto &pair : modData.settings)
			{
				f->WriteString(pair.first);
				f->WriteString(pair.second);
			}
		}
		auto &children = pair.second.children;
		f->Write<unsigned char>(CUInt8(children.size()));
		for(unsigned char j=0;j<children.size();j++)
			f->WriteString(children[j]);
		i++;
	}
	return true;
}

