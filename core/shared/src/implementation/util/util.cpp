// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :util.core;

import pragma.oskit;

pragma::util::ParallelJob<std::vector<Vector2> &> pragma::util::generate_lightmap_uvs(NetworkState &nwState, uint32_t atlastWidth, uint32_t atlasHeight, const std::vector<math::Vertex> &verts, const std::vector<uint32_t> &tris)
{
	auto lib = nwState.InitializeLibrary("pr_uvatlas");
	if(lib == nullptr)
		return {};
	auto *fGenerateAtlasUvs = lib->FindSymbolAddress<void (*)(uint32_t, uint32_t, const std::vector<math::Vertex> &, const std::vector<uint32_t> &, ParallelJob<std::vector<Vector2> &> &)>("pr_uvatlas_generate_atlas_uvs");
	if(fGenerateAtlasUvs == nullptr)
		return {};
	ParallelJob<std::vector<Vector2> &> job {};
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
		if(c == std::char_traits<char>::eof())
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
			Con::CWAR << "  > ";
		else
			Con::CWAR << "    ";
		Con::CWAR << line << '\n';

		if(it == lines.end() - 1)
			lineOffset = line.length() - len;
	}
	Con::CWAR << std::string(static_cast<size_t>(charIdx) + 4 + lineOffset, ' ') << '^' << Con::endl;

	Con::CWAR << Con::endl;
	return true;
}
template<typename T>
static std::shared_ptr<udm::Data> load_udm_asset(T &&f, std::string *optOutErr)
{
	using TBase = pragma::util::base_type<T>;
	pragma::fs::VFilePtr fptr = nullptr;
	try {
		if constexpr(std::is_same_v<TBase, std::string>)
			return udm::Data::Load(f);
		else {
			if(typeid(*f) == typeid(pragma::fs::File))
				fptr = static_cast<pragma::fs::File *>(f.get())->GetFile();
			return udm::Data::Load(std::move(f));
		}
	}
	catch(const udm::AsciiException &e) {
		if(optOutErr)
			*optOutErr = e.what();
		Con::CWAR << "[UDM] Failed to load UDM asset";
		if constexpr(std::is_same_v<TBase, std::string>)
			Con::COUT << " '" << f << "'";
		else {
			auto *ptr = static_cast<pragma::fs::VFilePtrInternalReal *>(fptr.get());
			if(ptr) {
				auto path = pragma::util::Path::CreateFile(ptr->GetPath());
				path.MakeRelative(pragma::util::get_program_path());
				Con::CWAR << " '" << path.GetString() << "'";
			}
		}
		Con::CWAR << ": " << e.what() << ":\n";
		if constexpr(std::is_same_v<std::remove_const_t<std::remove_reference_t<T>>, std::string>) {
			auto fptr = pragma::fs::open_file(f.c_str(), pragma::fs::FileMode::Read);
			if(fptr) {
				pragma::fs::File f {fptr};
				print_code_snippet(f, e.lineIndex, e.charIndex);
			}
		}
		else {
			if(fptr) {
				pragma::fs::File f {fptr};
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
std::shared_ptr<udm::Data> pragma::util::load_udm_asset(const std::string &fileName, std::string *optOutErr) { return ::load_udm_asset(fileName, optOutErr); }
std::shared_ptr<udm::Data> pragma::util::load_udm_asset(std::unique_ptr<ufile::IFile> &&f, std::string *optOutErr) { return ::load_udm_asset(std::move(f), optOutErr); }

void pragma::util::write_udm_entity(udm::LinkedPropertyWrapperArg udm, EntityHandle &hEnt)
{
	if(hEnt.valid())
		udm = uuid_to_string(hEnt->GetUuid());
}
EntityHandle pragma::util::read_udm_entity(Game &game, udm::LinkedPropertyWrapperArg udm)
{
	std::string uuid;
	udm(uuid);
	if(is_uuid(uuid)) {
		ecs::EntityIterator entIt {game, ecs::EntityIterator::FilterFlags::Default | ecs::EntityIterator::FilterFlags::Pending};
		entIt.AttachFilter<EntityIteratorFilterUuid>(uuid_string_to_bytes(uuid));
		auto it = entIt.begin();
		auto *ent = (it != entIt.end()) ? *it : nullptr;
		return ent ? ent->GetHandle() : EntityHandle {};
	}
	return EntityHandle {};
}
EntityHandle pragma::util::read_udm_entity(BaseEntityComponent &c, udm::LinkedPropertyWrapperArg udm) { return read_udm_entity(*c.GetEntity().GetNetworkState()->GetGameState(), udm); }

std::shared_ptr<pragma::util::HairFile> pragma::util::HairFile::Load(const udm::AssetData &data, std::string &outErr)
{
	auto anim = Create();
	if(anim->LoadFromAssetData(data, outErr) == false)
		return nullptr;
	return anim;
}
std::shared_ptr<pragma::util::HairFile> pragma::util::HairFile::Create() { return std::shared_ptr<HairFile> {new HairFile {}}; }
bool pragma::util::HairFile::Save(udm::AssetData &outData, std::string &outErr) const
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
bool pragma::util::HairFile::LoadFromAssetData(const udm::AssetData &data, std::string &outErr)
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

std::optional<std::string> pragma::util::convert_udm_file_to_ascii(const std::string &fileName, std::string &outErr)
{
	auto formatType = udm::Data::GetFormatType(fileName, outErr);
	if(formatType.has_value() == false) {
		outErr = "Unable to load UDM data: " + outErr;
		return {};
	}
	if(*formatType == udm::FormatType::Ascii)
		return fileName; // Already in ascii format
	auto udmData = load_udm_asset(fileName, &outErr);
	if(udmData == nullptr) {
		outErr = "Unable to load UDM data: " + outErr;
		return {};
	}
	std::string rpath;
	if(fs::find_absolute_path(fileName, rpath) == false) {
		outErr = "Unable to locate UDM file on disk!";
		return {};
	}
	auto path = Path::CreateFile(rpath);
	path.MakeRelative(get_program_path());
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
		res = udmData->SaveAscii(outFileName, udm::AsciiSaveFlags ::Default | udm::AsciiSaveFlags::IncludeHeader | udm::AsciiSaveFlags::DontCompressLz4Arrays);
	}
	catch(const udm::Exception &e) {
		outErr = "Unable to save UDM data: " + std::string {e.what()};
	}
	if(!res)
		return {};
	fs::remove_file(fileName);
	return Lua::file::to_relative_path(outFileName);
}
std::optional<std::string> pragma::util::convert_udm_file_to_binary(const std::string &fileName, std::string &outErr)
{
	auto formatType = udm::Data::GetFormatType(fileName, outErr);
	if(formatType.has_value() == false) {
		outErr = "Unable to load UDM data: " + outErr;
		return {};
	}
	if(*formatType == udm::FormatType::Binary)
		return fileName; // Already in binary format
	auto udmData = load_udm_asset(fileName, &outErr);
	if(udmData == nullptr) {
		outErr = "Unable to load UDM data: " + outErr;
		return {};
	}
	std::string rpath;
	if(fs::find_absolute_path(fileName, rpath) == false) {
		outErr = "Unable to locate UDM file on disk!";
		return {};
	}
	auto path = Path::CreateFile(rpath);
	path.MakeRelative(get_program_path());
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
	fs::remove_file(fileName);
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

std::pair<std::vector<double>, std::vector<double>> pragma::util::generate_two_pass_gaussian_blur_coefficients(uint32_t radius, double sigma, bool linear, bool correction)
{
	std::vector<double> offsets;
	std::vector<double> weights;
	::generate_two_pass_gaussian_blur_coefficients(radius, sigma, offsets, weights, linear, correction);
	return {std::move(offsets), std::move(weights)};
}
//

extern std::string g_lpUserDataDir;
extern std::vector<std::string> g_lpResourceDirs;
pragma::util::Path pragma::util::get_user_data_dir()
{
	if(!g_lpUserDataDir.empty())
		return g_lpUserDataDir;
	return get_program_path();
}

std::vector<pragma::util::Path> pragma::util::get_resource_dirs()
{
	std::vector<Path> paths;
	paths.reserve(g_lpResourceDirs.size());
	for(auto &path : g_lpResourceDirs)
		paths.push_back(path);
	return paths;
}

bool pragma::util::show_notification(const std::string &summary, const std::string &body)
{
	if(get_engine()->IsCLIOnly())
		return false;
	// Only show notification if program is not in focus
	if(get_engine()->IsProgramInFocus())
		return false;

	auto iconPath = engine_info::get_icon_path();
	std::string absIconPath;
	fs::find_absolute_path(iconPath.GetString(), absIconPath);

	oskit::NotificationInfo info {};
	info.appName = engine_info::get_name();
	info.title = summary;
	info.appIcon = absIconPath;
	info.body = body;
	return oskit::show_notification(info);
}

bool pragma::util::add_file_to_zip_archive(uzip::ZIPFile &zipArchive, const std::string &fileName, const void *data, uint64_t size, bool bOverwrite)
{
	return zipArchive.AddFile(fileName, data, size, bOverwrite);
}
