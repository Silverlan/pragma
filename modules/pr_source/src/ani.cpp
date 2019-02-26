#include <pragma/networkstate/networkstate.h>
#include <pragma/physics/physenvironment.h>
#include "ani.h"
#include "mdl.h"
#include <fsys/filesystem.h>
#include <array>

bool import::mdl::load_ani(const VFilePtr &f,const MdlInfo &mdlInfo)
{
	auto header = f->Read<studiohdr_t>();
	auto *id = reinterpret_cast<uint8_t*>(&header.id);
	if(id[0] != 0x49 || id[1] != 0x44 || id[2] != 0x41 || id[3] != 0x47)
		return false;

	auto &animDescs = mdlInfo.animationDescs;
	auto &animBlocks = mdlInfo.animationBlocks;
	for(auto it=animDescs.begin();it!=animDescs.end();++it)
	{
		auto &animDesc = *it;
		auto &stdAnimDesc = animDesc.GetStudioDesc();
		auto &animBlock = animBlocks[stdAnimDesc.animBlock];
		if(stdAnimDesc.animBlock > 0 && !(stdAnimDesc.flags &STUDIO_ALLZEROS))
		{
			auto &sections = animDesc.GetStudioSections();
			auto &sectionAnimations = animDesc.GetSectionAnimations();
			if(sectionAnimations.size() != sections.size())
				sectionAnimations.resize(sections.size());
			if(stdAnimDesc.sectionOffset != 0 && stdAnimDesc.sectionFrameCount > 0)
			{
				auto numSections = sections.size();
				for(auto i=decltype(numSections){0};i<numSections;++i)
				{
					auto &section = sections[i];
					auto &animBlock = animBlocks[section.animBlock];
					auto sectionFrameCount = (i < (numSections -1)) ? stdAnimDesc.sectionFrameCount : ((stdAnimDesc.frameCount -1) *stdAnimDesc.sectionFrameCount);

					f->Seek(animBlock.datastart +sections[i].animOffset);
					sectionAnimations[i] = std::make_shared<Animation>(mdlInfo.header,stdAnimDesc,f);
				}
			}
			else
			{
				uint32_t sectionIndex = 0;
				f->Seek(animBlock.datastart +stdAnimDesc.animOffset);
				if(sectionAnimations.empty())
					sectionAnimations.resize(1);
				sectionAnimations[0] = std::make_shared<Animation>(mdlInfo.header,stdAnimDesc,f);
			}
			if(stdAnimDesc.ikRuleCount > 0)
			{
				f->Seek(animBlock.datastart +stdAnimDesc.animblockIkRuleOffset);
				std::vector<mstudioikrule_t> stdIks(stdAnimDesc.ikRuleCount);
				f->Read(stdIks.data(),stdAnimDesc.ikRuleCount *sizeof(stdIks.front()));
			}
			if(stdAnimDesc.animBlock == 0 && stdAnimDesc.localHierarchyCount > 0)
				throw std::exception("");
		}
		if(stdAnimDesc.movementCount > 0)
		{
			/*auto movementOffset = mdlInfo.header.localanimindex +(it -animDescs.begin()) *sizeof(mstudioanimdesc_t);
			f->Seek(movementOffset +stdAnimDesc.movementOffset);
			auto endFrameIdx = f->Read<int32_t>();
			std::vector<mstudiomovement_t> stdMovement(stdAnimDesc.movementCount);
			f->Read(stdMovement.data(),stdAnimDesc.movementCount *sizeof(stdMovement.front())); // TODO: Does not work, why?
			stdMovement.back();*/
		}
	}
	return true;
}
