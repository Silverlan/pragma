#include <util_dmx.hpp>
#include <fsys/filesystem.h>
#include <pragma/networkstate/networkstate.h>
#include <pragma/model/model.h>

namespace import
{
	bool load_dmx(NetworkState *nw,const VFilePtr &f,const std::function<std::shared_ptr<Model>()> &fCreateModel);
};
#pragma optimize("",off)
bool import::load_dmx(NetworkState *nw,const VFilePtr &f,const std::function<std::shared_ptr<Model>()> &fCreateModel)
{
	auto vf = f;
	auto dmxData = dmx::FileData::Load(vf);
	if(dmxData == nullptr)
		return false;
	std::function<void(const dmx::FileData&)> fPrintData = nullptr;
	fPrintData = [](const dmx::FileData &data) {
		for(auto &el : data.GetElements())
		{
			el->name;
		//	el->GUID
		}
		//dmxData->GetElements
	};
	fPrintData(*dmxData);
	std::cout<<"T"<<std::endl;
	return true;
}
#pragma optimize("",on)
