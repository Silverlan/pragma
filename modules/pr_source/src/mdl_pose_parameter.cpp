#include "mdl_pose_parameter.h"

import::mdl::PoseParameter::PoseParameter(const VFilePtr &f)
{
	auto offset = f->Tell();

	auto stdPoseParameter = f->Read<mdl::mstudioposeparamdesc_t>();

	if(stdPoseParameter.sznameindex != 0)
	{
		f->Seek(offset +stdPoseParameter.sznameindex);
		m_name = f->ReadString();
	}

	m_start = stdPoseParameter.start;
	m_end = stdPoseParameter.end;
	m_loop = stdPoseParameter.loop;

	f->Seek(offset +sizeof(mdl::mstudioposeparamdesc_t));
}

const std::string &import::mdl::PoseParameter::GetName() const {return m_name;}
float import::mdl::PoseParameter::GetStart() const {return m_start;}
float import::mdl::PoseParameter::GetEnd() const {return m_end;}
float import::mdl::PoseParameter::GetLoop() const {return m_loop;}

void import::mdl::PoseParameter::DebugPrint(const std::string &t)
{
	std::cout<<t<<"Name: "<<m_name<<std::endl;
}
