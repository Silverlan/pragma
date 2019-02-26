#include "mdl_hitboxset.h"

import::mdl::HitboxSet::HitboxSet(const VFilePtr &f)
{
	auto offset = f->Tell();

	auto stdHitboxSet = f->Read<mdl::mstudiohitboxset_t>();

	if(stdHitboxSet.sznameindex != 0)
	{
		f->Seek(offset +stdHitboxSet.sznameindex);
		m_name = f->ReadString();
	}

	f->Seek(offset +stdHitboxSet.hitboxindex);
	m_hitboxes.reserve(stdHitboxSet.numhitboxes);
	for(auto i=decltype(stdHitboxSet.numhitboxes){0};i<stdHitboxSet.numhitboxes;++i)
	{
		m_hitboxes.push_back({});
		auto &hb = m_hitboxes.back();

		auto hbOffset = f->Tell();
		auto stdHitbox = f->Read<mstudiobbox_t>();
		if(stdHitbox.szhitboxnameindex != 0)
		{
			f->Seek(hbOffset +stdHitbox.szhitboxnameindex);
			hb.name = f->ReadString();
		}
		hb.boneId = stdHitbox.bone;
		hb.groupId = stdHitbox.group;

		hb.boundingBox.first = stdHitbox.bbmin;
		hb.boundingBox.second = stdHitbox.bbmax;
		umath::swap(hb.boundingBox.first.y,hb.boundingBox.first.z);
		umath::swap(hb.boundingBox.second.y,hb.boundingBox.second.z);
		umath::negate(hb.boundingBox.first.y);
		umath::negate(hb.boundingBox.second.y);
		uvec::to_min_max(hb.boundingBox.first,hb.boundingBox.second);

		f->Seek(hbOffset +sizeof(stdHitbox));
	}

	f->Seek(offset +sizeof(mdl::mstudiohitboxset_t));
}

const std::string &import::mdl::HitboxSet::GetName() const {return m_name;}
const std::vector<import::mdl::HitboxSet::Hitbox> &import::mdl::HitboxSet::GetHitboxes() const {return m_hitboxes;}

void import::mdl::HitboxSet::DebugPrint(const std::string &t)
{
	std::cout<<t<<"Name: "<<m_name<<std::endl;
}
