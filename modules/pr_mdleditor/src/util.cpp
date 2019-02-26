#include "stdafx_mdleditor.h"
#include "util.h"
#include <sharedutils/util_file.h>
#include <pragma/model/model.h>

uint32_t import::util::add_texture(Model &mdl,const std::string &name)
{
	auto fname = name;
	std::string ext;
	if(ufile::get_extension(name,&ext) == true)
		fname = fname.substr(0,fname.length() -(ext.length() +1));
	auto &meta = mdl.GetMetaInfo();
	auto it = std::find(meta.textures.begin(),meta.textures.end(),fname);
	auto idx = 0u;
	if(it != meta.textures.end())
		idx = it -meta.textures.begin();
	else
	{
		meta.textures.push_back(fname);
		idx = meta.textures.size() -1;
	}
	auto *texGroup = mdl.GetTextureGroup(0);
	if(texGroup == nullptr)
		texGroup = mdl.CreateTextureGroup();
	texGroup->textures.push_back(idx);
	return idx;
}
