#include "mdl_bodypart.h"

import::mdl::BodyPart::BodyPart(const VFilePtr &f)
{
	auto offset = f->Tell();

	auto stdBodyPart = f->Read<mdl::mstudiobodyparts_t>();

	if(stdBodyPart.sznameindex != 0)
	{
		f->Seek(offset +stdBodyPart.sznameindex);
		m_name = f->ReadString();
	}

	f->Seek(offset +stdBodyPart.modelindex);
	m_models.reserve(stdBodyPart.nummodels);
	for(auto i=decltype(stdBodyPart.nummodels){0};i<stdBodyPart.nummodels;++i)
	{
		m_models.push_back({});
		auto &mdl = m_models.back();
		auto &meshes = mdl.meshes;
		auto &eyeballs = mdl.eyeballs;

		auto offset = f->Tell();
		auto &stdMdl = mdl.stdMdl = f->Read<mstudiomodel_t>();
		mdl.vertexCount = stdMdl.numvertices;
		// Read meshes
		f->Seek(offset +stdMdl.meshindex);
		meshes.reserve(stdMdl.nummeshes);
		for(auto j=decltype(stdMdl.nummeshes){0};j<stdMdl.nummeshes;++j)
		{
			meshes.push_back(Model::Mesh{});
			auto &mesh = mdl.meshes.back();
			auto offsetMesh = f->Tell();
			mesh.stdMesh = f->Read<mstudiomesh_t>();
			
			if(mesh.stdMesh.numflexes > 0 && mesh.stdMesh.flexindex != 0)
				ReadFlexes(f,offsetMesh,mesh);
			f->Seek(offsetMesh +sizeof(mstudiomesh_t));
		}

		// Read eyeballs
		f->Seek(offset +stdMdl.eyeballindex);
		for(auto j=decltype(stdMdl.numeyeballs){0};j<stdMdl.numeyeballs;++j)
		{
			eyeballs.push_back(Model::Eyeball{});
			auto &eyeball = eyeballs.back();
			auto offsetEyeball = f->Tell();
			eyeball.stdEyeball = f->Read<mstudioeyeball_t>();

			if(eyeball.stdEyeball.sznameindex != 0)
			{
				f->Seek(offsetEyeball +eyeball.stdEyeball.sznameindex);
				eyeball.name = f->ReadString();
			}

			f->Seek(offsetEyeball +sizeof(mstudioeyeball_t));
		}

		f->Seek(offset +sizeof(mstudiomodel_t));
	}
	f->Seek(offset +sizeof(mdl::mstudiobodyparts_t));
}

void import::mdl::BodyPart::ReadVertAnims(const VFilePtr &f,uint64_t offset,Model::Mesh::Flex &flex)
{
	auto &stdFlex = flex.stdFlex;
	auto &flexVerts = flex.vertAnim;
	flexVerts.resize(stdFlex.numverts);
	f->Seek(offset +stdFlex.vertindex);
	for(auto i=decltype(stdFlex.numverts){0};i<stdFlex.numverts;++i)
	{
		auto &flexVert = flexVerts.at(i);
		switch(stdFlex.vertanimtype)
		{
		case StudioVertAnimType_t::STUDIO_VERT_ANIM_WRINKLE:
			flexVert = std::make_shared<mstudiovertanim_wrinkle_t>();
			break;
		default:
			flexVert = std::make_shared<mstudiovertanim_t>();
			break;
		}
		flexVert->index = f->Read<uint16_t>();
		flexVert->speed = f->Read<uint8_t>();
		flexVert->side = f->Read<uint8_t>();

		f->Read(flexVert->delta.data(),sizeof(flexVert->delta));
		f->Read(flexVert->ndelta.data(),sizeof(flexVert->ndelta));

		if(stdFlex.vertanimtype == StudioVertAnimType_t::STUDIO_VERT_ANIM_WRINKLE)
			static_cast<mstudiovertanim_wrinkle_t&>(*flexVert).wrinkledelta = f->Read<int16_t>();
	}
}

void import::mdl::BodyPart::ReadFlexes(const VFilePtr &f,uint64_t offset,Model::Mesh &mesh)
{
	auto &stdMesh = mesh.stdMesh;
	f->Seek(offset +stdMesh.flexindex);
	mesh.flexes.reserve(stdMesh.numflexes);
	for(auto i=decltype(stdMesh.numflexes){0};i<stdMesh.numflexes;++i)
	{
		auto flexOffset = f->Tell();
		mesh.flexes.push_back({});
		auto &flex = mesh.flexes.back();
		auto &stdFlex = flex.stdFlex = f->Read<mstudioflex_t>();
		auto offset = f->Tell();
		if(stdFlex.numverts > 0 && stdFlex.vertindex != 0)
			ReadVertAnims(f,flexOffset,flex);
		f->Seek(offset);
	}
}

const std::vector<import::mdl::BodyPart::Model> &import::mdl::BodyPart::GetModels() const {return m_models;}
const std::string &import::mdl::BodyPart::GetName() const {return m_name;}

void import::mdl::BodyPart::DebugPrint(const std::string &t)
{
	std::cout<<t<<"Name: "<<m_name<<std::endl;
}

