#include "stdafx_shared.h"
#include "pragma/file_formats/util_vmf.hpp"
#include "pragma/model/poly.h"
#include "pragma/model/polymesh.h"

std::string vmf::impl::DataFileBlock::KeyValue(const std::string &key,int32_t i)
{
	auto it = keyvalues.find(key);
	if(it == keyvalues.end() || i >= it->second.size())
		return "";
	return it->second.at(i);
}

//////////////

std::shared_ptr<vmf::impl::DataFileBlock> vmf::impl::DataFile::Read(const char *fName)
{
	auto f = FileManager::OpenFile(fName,"r");
	if(f == nullptr)
		return nullptr;
	return ReadBlock(f);
}

std::shared_ptr<vmf::impl::DataFileBlock> vmf::impl::DataFile::ReadBlock(VFilePtr f)
{
	auto block = std::make_shared<DataFileBlock>();
	while(!f->Eof())
	{
		auto sbuf = f->ReadLine();
		if(sbuf.length() > 0 && sbuf.front() != '\0')
		{
			ustring::remove_whitespace(sbuf);
			if(sbuf.length() > 0)
			{
				auto cLast = sbuf.find_first_of(ustring::WHITESPACE);
				auto cNext = sbuf.find_first_not_of(ustring::WHITESPACE,cLast);
				if(cNext != std::string::npos)
				{
					std::string key,val;
					auto stKey = sbuf.find('\"');
					auto enKey = sbuf.find('\"',stKey +1);
					auto stVal = sbuf.find('\"',enKey +1);
					auto enVal = sbuf.find('\"',stVal +1);
					key = sbuf.substr(stKey +1,(enKey -stKey) -1);
					val = sbuf.substr(stVal +1,(enVal -stVal) -1);
					auto it = block->keyvalues.find(key);
					if(it == block->keyvalues.end())
						it = block->keyvalues.insert(std::unordered_map<std::string,std::vector<std::string>>::value_type(key,std::vector<std::string>())).first;
					it->second.push_back(val);
				}
				else if(sbuf.front() == '}') // End of block
					return block;
				else // Sub-Block
				{
					ustring::remove_quotes(sbuf);
					char c;
					do c = static_cast<char>(f->ReadChar());
					while(c != '{' && c != EOF);
					auto it = block->blocks.find(sbuf);
					std::vector<std::shared_ptr<DataFileBlock>> *blocks = nullptr;
					if(it == block->blocks.end())
						blocks = &(block->blocks[sbuf] = {});
					else
						blocks = &it->second;
					auto sub = ReadBlock(f);
					blocks->push_back(sub);
				}
			}
		}
	}
	return block; // File ended prematurely? (Or this is the main block)
}

//////////////

static void swap_yz_coordinates(Vector3 &v)
{
	auto y = v.y;
	v.y = v.z;
	v.z = -y;
}

std::shared_ptr<PolyMesh> vmf::impl::build_mesh(const std::vector<std::shared_ptr<DataFileBlock>> &sides,NetworkState *nwState)
{
	auto mesh = std::make_shared<PolyMesh>();
	for(auto &side : sides)
	{
		auto material = side->KeyValue("material");
		auto splane = side->KeyValue("plane");
		auto suaxis = side->KeyValue("uaxis");
		auto svaxis = side->KeyValue("vaxis");
		auto srot = side->KeyValue("rotation");
		auto slmapscale = side->KeyValue("lightmapscale");
		auto smoothinggroups = side->KeyValue("smoothing_groups");
		auto itDispInfo = side->blocks.find("dispinfo");

		auto ufound = suaxis.find(']');
		auto vfound = svaxis.find(']');
		if(ufound != ustring::NOT_FOUND && vfound != ustring::NOT_FOUND)
		{
			auto u = suaxis.substr(1,ufound -1);
			auto v = svaxis.substr(1,vfound -1);

			std::vector<std::string> vu;
			std::vector<std::string> vv;
			ustring::split(u,vu);
			ustring::split(v,vv);
			if(vu.size() == 4 && vv.size() == 4)
			{
				auto poly = std::shared_ptr<Poly>(new Poly(nwState));
				std::array<Vector3,3> plane {};
				auto st = splane.find_first_of('(');
				size_t en;
				for(auto &plane : plane)
				{
					en = splane.find(')',st +1);
					std::vector<std::string> vec;
					ustring::split(splane.substr(st +1,en -1),vec);
					plane.x = static_cast<float>(atof(vec.at(0).c_str()));
					plane.y = static_cast<float>(atof(vec.at(2).c_str()));
					plane.z = static_cast<float>(-atof(vec.at(1).c_str()));
					st = splane.find('(',en +1);
				}
				auto normal = glm::cross(plane[2] -plane[0],plane[2] -plane[1]);
				auto l = uvec::length(normal);
				if(l == 0.f)
				{
					std::cout<<"WARNING: Brush plane with no area! This may cause problems."<<std::endl;
					// Note: This case occurred with a cylinder created in hammer with a face count of 32.
					// Why this happens is unknown. With the normal specified below the cylinder worked, but
					// it might not in other cases?
					normal = Vector3{0.f,-1.f,0.f};
				}
				else
					normal /= l;
				auto d = -(glm::dot(normal,plane[0]));
				if(itDispInfo != side->blocks.end() && itDispInfo->second.empty() == false)
				{
					auto &disp = itDispInfo->second.front();
					auto itNormals = disp->blocks.find("normals");
					auto itDistances = disp->blocks.find("distances");
					auto itOffsets = disp->blocks.find("offsets");
					auto itAlphas = disp->blocks.find("alphas");
					if(itNormals == disp->blocks.end() || itDistances == disp->blocks.end() || itOffsets == disp->blocks.end() || itAlphas == disp->blocks.end() ||
						itNormals->first.empty() || itDistances->first.empty() || itOffsets->first.empty() || itAlphas->first.empty())
						std::cout<<"WARNING: Displacement with missing information. Skipping..."<<std::endl;
					else
					{
						auto &normals = itNormals->second.front();
						auto &distances = itDistances->second.front();
						auto &offsets = itOffsets->second.front();
						auto &alphas = itAlphas->second.front();

						auto spower = disp->KeyValue("power");
						auto sstartposition = disp->KeyValue("startposition");
						auto sflags = disp->KeyValue("flags");
						auto selevation = disp->KeyValue("elevation");
						auto ssubdiv = disp->KeyValue("subdiv");

						auto info = std::make_shared<DispInfo>();
						info->power = util::to_int(spower);
						info->startposition = {};
						info->startpositionId = std::numeric_limits<uint32_t>::max();
						if(sstartposition.empty() == false && sstartposition.front() == '[')
						{
							auto l = sstartposition.find(']');
							if(l != ustring::NOT_FOUND)
							{
								sstartposition = sstartposition.substr(1,l -1);
								std::vector<std::string> vec;
								ustring::split(sstartposition,vec);
								info->startposition = Vector3{
									util::to_float(vec.at(0)),
									util::to_float(vec.at(1)),
									util::to_float(vec.at(2))
								};
							}
						}
						swap_yz_coordinates(info->startposition);
						//info->startposition.z *= -1.f;
						info->flags = util::to_int(sflags);
						info->elevation = util::to_int(selevation);
						info->subdiv = util::to_int(ssubdiv);

						auto rows = static_cast<uint32_t>(pow(2,info->power)) +1;
						if(normals->keyvalues.size() < rows || distances->keyvalues.size() < rows || offsets->keyvalues.size() < rows || alphas->keyvalues.size() < rows)
						{
							info = nullptr;
							std::cout<<"WARNING: Displacement with missing information. Skipping..."<<std::endl;
						}
						else
						{
							bool bInvalid = false;
							for(auto i=decltype(rows){0u};i<rows;++i)
							{
								info->normals.push_back(std::vector<Vector3>());
								info->distances.push_back(std::vector<float>());
								info->offsets.push_back(std::vector<Vector3>());
								info->alphas.push_back(std::vector<Vector2>());
								auto &vNormals = info->normals.back();
								auto &vDistances = info->distances.back();
								auto &vOffsets = info->offsets.back();
								auto &vAlphas = info->alphas.back();
								std::string srow = "row";
								srow += std::to_string(i);
								auto srowNormals = normals->KeyValue(srow);
								auto srowDistances = distances->KeyValue(srow);
								auto srowOffsets = offsets->KeyValue(srow);
								auto srowAlphas = alphas->KeyValue(srow);
								std::vector<std::string> vSNormals;
								std::vector<std::string> vSDistances;
								std::vector<std::string> vSOffsets;
								std::vector<std::string> vSAlphas;
								ustring::split(srowNormals,vSNormals);
								ustring::split(srowDistances,vSDistances);
								ustring::split(srowOffsets,vSOffsets);
								ustring::split(srowAlphas,vSAlphas);
								if(vSNormals.size() != rows *3 || vSDistances.size() != rows || vSOffsets.size() != rows *3 || vSAlphas.size() != rows)
								{
									bInvalid = true;
									break;
								}
								for(auto j=decltype(rows){0u};j<(rows *3u);j+=3u)
								{
									vNormals.push_back(Vector3{
										util::to_float(vSNormals.at(j)),
										util::to_float(vSNormals.at(j +1)),
										util::to_float(vSNormals.at(j +2))
									});
									swap_yz_coordinates(vNormals.back());
									auto &n = vNormals.back();
									if(glm::length(n) == 0.f)
										n = glm::normalize(normal *((d > 0.f) ? 1.f : -1.f));
									else
										n = glm::normalize(n);
									//vNormals.back().z *= -1.f;
									vOffsets.push_back(Vector3{
										util::to_float(vSOffsets.at(j)),
										util::to_float(vSOffsets.at(j +1)),
										util::to_float(vSOffsets.at(j +2))
									});
									swap_yz_coordinates(vOffsets.back());
									//vOffsets.back().z *= -1.f;
								}
								for(auto j=decltype(rows){0u};j<rows;++j)
								{
									vDistances.push_back(util::to_float(vSDistances.at(j)));
									auto fAlpha = util::to_float(vSAlphas.at(j));
									if(fAlpha < 0.f || fAlpha > 255.f)
									{
										std::cout<<"WARNING: Displacement alpha out of bounds: \""<<srowAlphas<<"\". Clamping..."<<std::endl;
										if(fAlpha < 0.f)
											fAlpha = 0.f;
										else if(fAlpha > 255.f)
											fAlpha = 255.f;
									}
									vAlphas.push_back(Vector2{fAlpha /255.f,0.f});
								}
							}
							if(bInvalid == true)
							{
								info = nullptr;
								std::cout<<"WARNING: Displacement with missing information. Skipping..."<<std::endl;
							}
							else
							{
								mesh->SetHasDisplacements(true);
								poly->SetDisplacement(info);
							}
						}
					}
				}
				auto su = util::to_float(suaxis.substr(ufound +1));
				auto sv = -util::to_float(svaxis.substr(vfound +1));
				poly->SetTextureData(
					material,
					Vector3{util::to_float(vu.at(0)),util::to_float(vu.at(2)),-util::to_float(vu.at(1))},
					Vector3{util::to_float(vv.at(0)),util::to_float(vv.at(2)),-util::to_float(vv.at(1))},
					util::to_float(vu.at(3)),-util::to_float(vv.at(3)),
					su,sv,
					util::to_float(srot)
				);
				poly->SetMaterial(nwState->GetMaterialManager().Load(material));
				poly->SetNormal(normal);
				poly->SetDistance(d);
				mesh->AddPoly(poly);
			}
			else
				std::cout<<"WARNING: Side with invalid texture information. Skipping..."<<std::endl;
		}
	}
	auto err = mesh->BuildPolyMesh();
	if(err != 0)
	{
		if(err == -1)
			std::cout<<"WARNING: Displacement with incorrect vertex count! Skipping mesh..."<<std::endl;
		else if(err == -2)
			std::cout<<"WARNING: Displacement with invalid start position! Skipping mesh..."<<std::endl;
		else
			std::cout<<"WARNING: Unable to build mesh. Skipping..."<<std::endl;
		return nullptr;
	}
	mesh->Calculate();
	if(mesh->HasDisplacements())
	{
		auto &polys = mesh->GetPolys();
		for(int i=static_cast<int>(polys.size()) -1;i>=0;i--)
		{
			if(!polys[i]->IsDisplacement())
				polys.erase(polys.begin() +i);
		}
	}
	//auto &polys = mesh->GetPolys();
	/*for(int i=polys->size() -1;i>=0;i--)
	{
		Material *mat = (*polys)[i]->GetMaterial();
		if(mat != NULL)
		{
			DataBlock *data = mat->GetDataBlock();
			bool bRemoveFace;
			if(data->GetBool("removeface",&bRemoveFace) && bRemoveFace == true)
			{
				delete (*polys)[i];
				polys->erase(polys->begin() +i);
			}
		}
	}*/
	if(mesh->GetPolyCount() == 0)
		return nullptr;
	return mesh;
}
