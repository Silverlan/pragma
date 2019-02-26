#include "mdl_flexdesc.h"

import::mdl::FlexDesc::FlexDesc(const VFilePtr &f)
{
	auto offset = f->Tell();
	auto nameOffset = f->Read<int32_t>();
	if(nameOffset != 0)
	{
		f->Seek(offset +nameOffset);
		m_name = f->ReadString();
	}
	f->Seek(offset +sizeof(int32_t));
}

const std::string &import::mdl::FlexDesc::GetName() const {return m_name;}
