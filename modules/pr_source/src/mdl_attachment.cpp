#include "mdl_attachment.h"

import::mdl::Attachment::Attachment(const VFilePtr &f)
{
	auto offset = f->Tell();

	auto stdAttachment = f->Read<mdl::mstudioattachment_t>();
	m_flags = stdAttachment.flags;
	m_bone = stdAttachment.localbone;
	m_transform = stdAttachment.local;

	f->Seek(offset +stdAttachment.sznameindex);
	m_name = f->ReadString();

	f->Seek(offset +sizeof(mdl::mstudioattachment_t));
}

uint32_t import::mdl::Attachment::GetFlags() const {return m_flags;}
int32_t import::mdl::Attachment::GetBone() const {return m_bone;}
const Mat3x4 &import::mdl::Attachment::GetTransform() const {return m_transform;}
const std::string &import::mdl::Attachment::GetName() const {return m_name;}

void import::mdl::Attachment::DebugPrint(const std::string &t)
{
	std::cout<<t<<"Flags: "<<m_flags<<std::endl;
	std::cout<<t<<"Bone: "<<m_bone<<std::endl;
	std::cout<<t<<"Name: "<<m_name<<std::endl;
	std::cout<<t<<"Transform: ("<<m_transform[0][0]<<","<<m_transform[0][1]<<","<<m_transform[0][2]<<","<<m_transform[0][3]<<") ("<<m_transform[1][0]<<","<<m_transform[1][1]<<","<<m_transform[1][2]<<","<<m_transform[1][3]<<") ("<<m_transform[2][0]<<","<<m_transform[2][1]<<","<<m_transform[2][2]<<","<<m_transform[2][3]<<")"<<std::endl;
}
