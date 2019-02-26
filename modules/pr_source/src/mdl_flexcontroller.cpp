#include "mdl_flexcontroller.h"

import::mdl::FlexController::FlexController(const VFilePtr &f)
{
	auto offset = f->Tell();
	auto stdFlexController = f->Read<mstudioflexcontroller_t>();
	if(stdFlexController.sztypeindex != 0)
	{
		f->Seek(offset +stdFlexController.sztypeindex);
		m_type = f->ReadString();
	}
	if(stdFlexController.sznameindex != 0)
	{
		f->Seek(offset +stdFlexController.sznameindex);
		m_name = f->ReadString();
	}
	m_min = stdFlexController.min;
	m_max = stdFlexController.max;

	f->Seek(offset +sizeof(mstudioflexcontroller_t));
}
std::pair<float,float> import::mdl::FlexController::GetRange() const {return {m_min,m_max};}
const std::string &import::mdl::FlexController::GetName() const {return m_name;}
const std::string &import::mdl::FlexController::GetType() const {return m_type;}

import::mdl::FlexControllerUi::FlexControllerUi(const VFilePtr &f)
{
	auto offset = f->Tell();
	auto flexControllerUi = f->Read<mstudioflexcontrollerui_t>();
	m_szIndices = {flexControllerUi.szindex0,flexControllerUi.szindex1,flexControllerUi.szindex2};
	m_remapType = flexControllerUi.remaptype;
	m_stereo = flexControllerUi.stereo;
	if(flexControllerUi.sznameindex != 0)
	{
		f->Seek(offset +flexControllerUi.sznameindex);
		m_name = f->ReadString();
	}
	f->Seek(offset +sizeof(mstudioflexcontrollerui_t));
}
bool import::mdl::FlexControllerUi::IsStereo() const {return m_stereo;}
uint8_t import::mdl::FlexControllerUi::GetRemapType() const {return m_remapType;}
int32_t import::mdl::FlexControllerUi::GetConfig0() const {return m_szIndices.at(0);}
int32_t import::mdl::FlexControllerUi::GetConfig1() const {return m_szIndices.at(1);}
int32_t import::mdl::FlexControllerUi::GetConfig2() const {return m_szIndices.at(2);}
const std::string &import::mdl::FlexControllerUi::GetName() const {return m_name;}
