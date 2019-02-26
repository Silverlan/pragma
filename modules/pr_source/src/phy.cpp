#include "phy.h"
#include <filesystem.h>
#include <util/util_string.h>

bool import::mdl::load_phy(const VFilePtr &f,std::vector<phy::PhyCollisionData> &solids,std::unordered_map<int32_t,phy::PhyRagdollConstraint> &constraints)
{
	auto header = f->Read<phy::phyheader_s>();

	// Read collision data
	solids.reserve(header.solidCount);
	for(auto i=decltype(header.solidCount){0};i<header.solidCount;++i)
	{
		solids.push_back({});
		auto &solid = solids.back();

		auto size = f->Read<int32_t>();
		auto offsetNextSolidData = f->Tell() +size;

		std::array<char,5> phyId {};
		phyId.back() = '\0';
		f->Read(phyId.data(),4);
		if(strcmp(phyId.data(),"VPHY") != 0)
			return false;
		// Unknown
		f->Seek(f->Tell() +sizeof(uint16_t) *2 +sizeof(int32_t) *16);
		//

		f->Read(phyId.data(),4);
		if(strcmp(phyId.data(),"IVPS") != 0)
			return false;

		std::vector<uint16_t> vertIds;
		auto offsetVertexData = f->Tell() +size;
		while(f->Tell() < offsetVertexData)
		{
			auto offsetFace = f->Tell();
			solid.faceSections.push_back({});
			auto &faceSection = solid.faceSections.back();
			auto vertexDataOffset = f->Read<int32_t>();

			offsetVertexData = offsetFace +vertexDataOffset;

			auto &boneIdx = faceSection.boneIdx = f->Read<int32_t>() -1;
			if(boneIdx < 0)
			{
				boneIdx = 0;
				solid.sourcePhyIsCollision = true;
			}
			f->Seek(f->Tell() +sizeof(int32_t));

			auto numTriangles = f->Read<int32_t>();
			for(auto i=decltype(numTriangles){0};i<numTriangles;++i)
			{
				auto triIdx = f->Read<uint8_t>();
				f->Seek(f->Tell() +sizeof(uint8_t) +sizeof(uint16_t));

				faceSection.faces.push_back({});
				auto &face = faceSection.faces.back();
				for(uint_fast8_t j=0;j<3;++j)
				{
					auto &vertIdx = face[j] = f->Read<uint16_t>();
					f->Seek(f->Tell() +sizeof(uint16_t));
					auto it = std::find(vertIds.begin(),vertIds.end(),vertIdx);
					if(it == vertIds.end())
						vertIds.push_back(vertIdx);
				}

			}
		}
		f->Seek(offsetVertexData);

		solid.vertices.reserve(vertIds.size());
		for(auto vertIdx : vertIds)
		{
			auto v = f->Read<Vector4>();
			solid.vertices.push_back({Vector3{v.x,v.y,v.z},Vector3{}});
		}
		f->Seek(offsetNextSolidData);
	}
	//

	// Read keyvalues
	do
	{
		auto offset = f->Tell();
		auto l = f->ReadLine();
		if(l.empty() || l != "solid {")
		{
			f->Seek(offset);
			break;
		}
		phy::PhyKeyValues keyValues {};
		int32_t solidIdx = -1;
		for(;;)
		{
			l = f->ReadLine();
			if(l.empty() || l == "}")
				break;
			std::string key,val;
			ustring::get_key_value(l,key,val," ");
			if(key == "index")
				solidIdx = atoi(val.c_str());
			else if(key == "name")
				keyValues.name = val;
			else if(key == "parent")
				keyValues.parent = val;
			else if(key == "mass")
				keyValues.mass = atof(val.c_str());
			else if(key == "surfaceprop")
				keyValues.surfaceProp = val;
			else if(key == "damping")
				keyValues.damping = atof(val.c_str());
			else if(key == "rotdamping")
				keyValues.rotDamping = atof(val.c_str());
			else if(key == "drag")
				keyValues.drag = atof(val.c_str());
			else if(key == "inertia")
				keyValues.inertia = atof(val.c_str());
			else if(key == "volume")
				keyValues.volume = atof(val.c_str());
			else if(key == "massbias")
				keyValues.massBias = atof(val.c_str());
		}
		if(solidIdx != -1)
			solids[solidIdx].keyValues = keyValues;
	}
	while(true);
	//

	// Ragdoll constraints
	do
	{
		auto offset = f->Tell();
		auto l = f->ReadLine();
		if(l.empty() || l != "ragdollconstraint {")
		{
			f->Seek(offset);
			break;
		}
		phy::PhyRagdollConstraint ragdollConstraint {};
		for(;;)
		{
			l = f->ReadLine();
			if(l.empty() || l == "}")
				break;
			std::string key,val;
			ustring::get_key_value(l,key,val," ");
			if(key == "parent")
				ragdollConstraint.parentIndex = atoi(val.c_str());
			else if(key == "child")
				ragdollConstraint.childIndex = atoi(val.c_str());
			else if(key == "xmin")
				ragdollConstraint.xmin = atof(val.c_str());
			else if(key == "xmax")
				ragdollConstraint.xmax = atof(val.c_str());
			else if(key == "xfriction")
				ragdollConstraint.xfriction = atof(val.c_str());
			else if(key == "ymin")
				ragdollConstraint.ymin = atof(val.c_str());
			else if(key == "ymax")
				ragdollConstraint.ymax = atof(val.c_str());
			else if(key == "yfriction")
				ragdollConstraint.yfriction = atof(val.c_str());
			else if(key == "zmin")
				ragdollConstraint.zmin = atof(val.c_str());
			else if(key == "zmax")
				ragdollConstraint.zmax = atof(val.c_str());
			else if(key == "zfriction")
				ragdollConstraint.zfriction = atof(val.c_str());
		}
		constraints.insert(std::make_pair(ragdollConstraint.childIndex,ragdollConstraint));
	}
	while(true);
	//

	return true;
}
