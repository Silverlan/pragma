/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "pragma/util/util_game.hpp"
#include "pragma/networkstate/networkstate.h"
#include "pragma/util/lookup_identifier.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/lua/libraries/lfile.h"
#include <sharedutils/util_library.hpp>
#include <sharedutils/util_path.hpp>
#include <fsys/ifile.hpp>
#include <udm.hpp>

util::ParallelJob<std::vector<Vector2> &> util::generate_lightmap_uvs(NetworkState &nwState, uint32_t atlastWidth, uint32_t atlasHeight, const std::vector<umath::Vertex> &verts, const std::vector<uint32_t> &tris)
{
	auto lib = nwState.InitializeLibrary("pr_uvatlas");
	if(lib == nullptr)
		return {};
	auto *fGenerateAtlasUvs = lib->FindSymbolAddress<void (*)(uint32_t, uint32_t, const std::vector<umath::Vertex> &, const std::vector<uint32_t> &, util::ParallelJob<std::vector<Vector2> &> &)>("pr_uvatlas_generate_atlas_uvs");
	if(fGenerateAtlasUvs == nullptr)
		return {};
	util::ParallelJob<std::vector<Vector2> &> job {};
	fGenerateAtlasUvs(atlastWidth, atlasHeight, verts, tris, job);
	if(job.IsValid() == false)
		return {};
	return job;
}

static bool print_code_snippet(ufile::IFile &f, uint32_t lineIdx, uint32_t charIdx)
{
	f.Seek(0);
	uint32_t numLinesPrint = 3;
	if(lineIdx < 2)
		numLinesPrint = lineIdx + 1;
	auto startLineIdx = lineIdx + 1 - numLinesPrint;

	uint32_t curLine = 0;
	for(;;) {
		if(curLine == startLineIdx)
			break;
		auto c = f.ReadChar();
		if(c == '\n') {
			++curLine;
			continue;
		}
		if(c == EOF)
			return false;
	}

	std::vector<std::string> lines;
	lines.resize(numLinesPrint);
	for(auto &l : lines)
		l = f.ReadLine();

	uint32_t lineOffset = 0;
	for(auto it = lines.begin(); it != lines.end(); ++it) {
		auto &line = *it;
		size_t pos = 0;
		auto len = line.length();
		while(pos < line.length()) {
			auto c = line[pos];
			if(c == '\t')
				line = line.substr(0, pos) + "    " + line.substr(pos + 1);
			++pos;
		}
		if(it == lines.end() - 1)
			Con::cwar << "  > ";
		else
			Con::cwar << "    ";
		Con::cwar << line << '\n';

		if(it == lines.end() - 1)
			lineOffset = line.length() - len;
	}
	Con::cwar << std::string(static_cast<size_t>(charIdx) + 4 + lineOffset, ' ') << '^' << Con::endl;

	Con::cwar << Con::endl;
	return true;
}
template<typename T>
static std::shared_ptr<udm::Data> load_udm_asset(T &&f, std::string *optOutErr)
{
	using TBase = util::base_type<T>;
	VFilePtr fptr = nullptr;
	try {
		if constexpr(std::is_same_v<TBase, std::string>)
			return udm::Data::Load(f);
		else {
			if(typeid(f) == typeid(fsys::File))
				fptr = static_cast<fsys::File *>(f.get())->GetFile();
			return udm::Data::Load(std::move(f));
		}
	}
	catch(const udm::AsciiException &e) {
		if(optOutErr)
			*optOutErr = e.what();
		Con::cwar << "[UDM] Failed to load UDM asset";
		if constexpr(std::is_same_v<TBase, std::string>)
			Con::cout << " '" << f << "'";
		else {
			auto *ptr = static_cast<VFilePtrInternalReal *>(fptr.get());
			if(ptr) {
				auto path = util::Path::CreateFile(ptr->GetPath());
				path.MakeRelative(util::get_program_path());
				Con::cwar << " '" << path.GetString() << "'";
			}
		}
		Con::cwar << ": " << e.what() << ":\n";
		if constexpr(std::is_same_v<std::remove_const_t<std::remove_reference_t<T>>, std::string>) {
			auto fptr = FileManager::OpenFile(f.c_str(), "r");
			if(fptr) {
				fsys::File f {fptr};
				print_code_snippet(f, e.lineIndex, e.charIndex);
			}
		}
		else {
			if(fptr) {
				fsys::File f {fptr};
				print_code_snippet(f, e.lineIndex, e.charIndex);
			}
		}
	}
	catch(const udm::Exception &e) {
		if(optOutErr)
			*optOutErr = e.what();
		return nullptr;
	}
	return nullptr;
}
std::shared_ptr<udm::Data> util::load_udm_asset(const std::string &fileName, std::string *optOutErr) { return ::load_udm_asset(fileName, optOutErr); }
std::shared_ptr<udm::Data> util::load_udm_asset(std::unique_ptr<ufile::IFile> &&f, std::string *optOutErr) { return ::load_udm_asset(std::move(f), optOutErr); }

void util::write_udm_entity(udm::LinkedPropertyWrapperArg udm, EntityHandle &hEnt)
{
	if(hEnt.valid())
		udm = util::uuid_to_string(hEnt->GetUuid());
}
EntityHandle util::read_udm_entity(Game &game, udm::LinkedPropertyWrapperArg udm)
{
	std::string uuid;
	udm(uuid);
	if(util::is_uuid(uuid)) {
		EntityIterator entIt {game, EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
		entIt.AttachFilter<EntityIteratorFilterUuid>(util::uuid_string_to_bytes(uuid));
		auto it = entIt.begin();
		auto *ent = (it != entIt.end()) ? *it : nullptr;
		return ent ? ent->GetHandle() : EntityHandle {};
	}
	return EntityHandle {};
}
EntityHandle util::read_udm_entity(::pragma::BaseEntityComponent &c, udm::LinkedPropertyWrapperArg udm) { return read_udm_entity(*c.GetEntity().GetNetworkState()->GetGameState(), udm); }

std::shared_ptr<util::HairFile> util::HairFile::Load(const udm::AssetData &data, std::string &outErr)
{
	auto anim = Create();
	if(anim->LoadFromAssetData(data, outErr) == false)
		return nullptr;
	return anim;
}
std::shared_ptr<util::HairFile> util::HairFile::Create() { return std::shared_ptr<HairFile> {new HairFile {}}; }
bool util::HairFile::Save(udm::AssetData &outData, std::string &outErr) const
{
	outData.SetAssetType(PHAIR_IDENTIFIER);
	outData.SetAssetVersion(PHAIR_VERSION);
	auto udm = *outData;

	auto udmConfig = udm["config"];
	udmConfig["numSegments"] = m_hairConfig.numSegments;
	udmConfig["hairPerSquareMeter"] = m_hairConfig.hairPerSquareMeter;
	udmConfig["defaultThickness"] = m_hairConfig.defaultThickness;
	udmConfig["defaultLength"] = m_hairConfig.defaultLength;
	udmConfig["defaultHairStrength"] = m_hairConfig.defaultHairStrength;
	udmConfig["randomHairLengthFactor"] = m_hairConfig.randomHairLengthFactor;
	udmConfig["curvature"] = m_hairConfig.curvature;

	auto udmData = udm["data"];
	if(!m_hairData.hairPoints.empty())
		udmData["points"] = udm::compress_lz4_blob(m_hairData.hairPoints);
	if(!m_hairData.hairUvs.empty())
		udmData["uvs"] = udm::compress_lz4_blob(m_hairData.hairUvs);
	if(!m_hairData.hairNormals.empty())
		udmData["normals"] = udm::compress_lz4_blob(m_hairData.hairNormals);
	if(!m_hairData.hairPointTriangles.empty())
		udmData["triangles"] = udm::compress_lz4_blob(m_hairData.hairPointTriangles);
	if(!m_hairData.hairPointBarycentric.empty())
		udmData["barycentricCoords"] = udm::compress_lz4_blob(m_hairData.hairPointBarycentric);
	return true;
}
bool util::HairFile::LoadFromAssetData(const udm::AssetData &data, std::string &outErr)
{
	if(data.GetAssetType() != PHAIR_IDENTIFIER) {
		outErr = "Incorrect format!";
		return false;
	}

	auto udm = *data;
	auto version = data.GetAssetVersion();
	if(version < 1) {
		outErr = "Invalid version!";
		return false;
	}

	auto udmConfig = udm["config"];
	udmConfig["numSegments"](m_hairConfig.numSegments);
	udmConfig["hairPerSquareMeter"](m_hairConfig.hairPerSquareMeter);
	udmConfig["defaultThickness"](m_hairConfig.defaultThickness);
	udmConfig["defaultLength"](m_hairConfig.defaultLength);
	udmConfig["defaultHairStrength"](m_hairConfig.defaultHairStrength);
	udmConfig["randomHairLengthFactor"](m_hairConfig.randomHairLengthFactor);
	udmConfig["curvature"](m_hairConfig.curvature);

	auto udmData = udm["data"];
	udmData["points"].GetBlobData(m_hairData.hairPoints);
	udmData["uvs"].GetBlobData(m_hairData.hairUvs);
	udmData["normals"].GetBlobData(m_hairData.hairNormals);
	udmData["triangles"].GetBlobData(m_hairData.hairPointTriangles);
	udmData["barycentricCoords"].GetBlobData(m_hairData.hairPointBarycentric);
	return true;
}

std::optional<std::string> util::convert_udm_file_to_ascii(const std::string &fileName, std::string &outErr)
{
	auto formatType = udm::Data::GetFormatType(fileName, outErr);
	if(formatType.has_value() == false) {
		outErr = "Unable to load UDM data: " + outErr;
		return {};
	}
	if(*formatType == udm::FormatType::Ascii)
		return fileName; // Already in ascii format
	auto udmData = util::load_udm_asset(fileName, &outErr);
	if(udmData == nullptr) {
		outErr = "Unable to load UDM data: " + outErr;
		return {};
	}
	std::string rpath;
	if(FileManager::FindAbsolutePath(fileName, rpath) == false) {
		outErr = "Unable to locate UDM file on disk!";
		return {};
	}
	auto path = util::Path::CreateFile(rpath);
	path.MakeRelative(util::get_program_path());
	auto outFileName = path.GetString();
	std::string ext;
	ufile::get_extension(outFileName, &ext);
	ufile::remove_extension_from_filename(outFileName);
	if(ext.empty())
		ext = "udm";
	else if(ext.length() > 2) {
		if(ext.at(ext.length() - 1) == 'b' && ext.at(ext.length() - 2) == '_')
			ext = ext.substr(0, ext.length() - 2);
	}
	outFileName += '.' + ext;
	auto res = true;
	try {
		res = udmData->SaveAscii(outFileName, ::udm::AsciiSaveFlags ::Default | udm::AsciiSaveFlags::IncludeHeader | udm::AsciiSaveFlags::DontCompressLz4Arrays);
	}
	catch(const udm::Exception &e) {
		outErr = "Unable to save UDM data: " + std::string {e.what()};
	}
	if(!res)
		return {};
	filemanager::remove_file(fileName);
	return Lua::file::to_relative_path(outFileName);
}
std::optional<std::string> util::convert_udm_file_to_binary(const std::string &fileName, std::string &outErr)
{
	auto formatType = udm::Data::GetFormatType(fileName, outErr);
	if(formatType.has_value() == false) {
		outErr = "Unable to load UDM data: " + outErr;
		return {};
	}
	if(*formatType == udm::FormatType::Binary)
		return fileName; // Already in binary format
	auto udmData = util::load_udm_asset(fileName, &outErr);
	if(udmData == nullptr) {
		outErr = "Unable to load UDM data: " + outErr;
		return {};
	}
	std::string rpath;
	if(FileManager::FindAbsolutePath(fileName, rpath) == false) {
		outErr = "Unable to locate UDM file on disk!";
		return {};
	}
	auto path = util::Path::CreateFile(rpath);
	path.MakeRelative(util::get_program_path());
	auto outFileName = path.GetString();
	std::string ext;
	ufile::get_extension(outFileName, &ext);
	ufile::remove_extension_from_filename(outFileName);
	if(ext.empty())
		ext = "udm_b";
	else
		ext += "_b";
	outFileName += '.' + ext;
	auto res = true;
	try {
		res = udmData->Save(outFileName);
	}
	catch(const udm::Exception &e) {
		outErr = "Unable to save UDM data: " + std::string {e.what()};
	}
	if(!res)
		return {};
	filemanager::remove_file(fileName);
	return Lua::file::to_relative_path(outFileName);
}

// See https://lisyarus.github.io/blog/graphics/2023/02/24/blur-coefficients-generator.html
static void generate_two_pass_gaussian_blur_coefficients(uint32_t uradius, double sigma, std::vector<double> &outOffsets, std::vector<double> &outWeights, bool linear = true, bool correction = true)
{
	if(sigma == 0.0)
		return;
	auto radius = static_cast<int32_t>(uradius);

	std::vector<double> weights;
	double sumWeights = 0.0;
	for(int i = -radius; i <= radius; i++) {
		double w = 0.0;
		if(correction) {
			w = (erf((i + 0.5) / sigma / sqrt(2.0)) - erf((i - 0.5) / sigma / sqrt(2.0))) / 2.0;
		}
		else {
			w = exp(-i * i / sigma / sigma);
		}
		sumWeights += w;
		weights.push_back(w);
	}

	for(int i = 0; i < weights.size(); i++)
		weights[i] /= sumWeights;

	auto &offsets = outOffsets;
	auto &newWeights = outWeights;
	offsets.reserve(2 * radius + 1);
	newWeights.reserve(2 * radius + 1);

	auto hasZeros = false;
	if(linear) {
		for(int i = -radius; i <= radius; i += 2) {
			if(i == radius) {
				offsets.push_back(i);
				newWeights.push_back(weights[i + radius]);
			}
			else {
				const double w0 = weights[i + radius + 0];
				const double w1 = weights[i + radius + 1];

				const double w = w0 + w1;
				if(w > 0) {
					offsets.push_back(i + w1 / w);
				}
				else {
					hasZeros = true;
					offsets.push_back(i);
				}
				newWeights.push_back(w);
			}
		}
	}
	else {
		for(int i = -radius; i <= radius; i++) {
			offsets.push_back(i);
		}

		for(double w : weights)
			if(w == 0.0)
				hasZeros = true;

		newWeights = weights;
	}
}

std::pair<std::vector<double>, std::vector<double>> util::generate_two_pass_gaussian_blur_coefficients(uint32_t radius, double sigma, bool linear, bool correction)
{
	std::vector<double> offsets;
	std::vector<double> weights;
	::generate_two_pass_gaussian_blur_coefficients(radius, sigma, offsets, weights, linear, correction);
	return {std::move(offsets), std::move(weights)};
}
//
