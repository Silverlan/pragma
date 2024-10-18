--[[
	Copyright (C) 2021 Silverlan

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

util.register_class("util.IAssetImportFileHandler")
function util.IAssetImportFileHandler:__init() end
function util.IAssetImportFileHandler:ExtractFile(fileName)
	return false, "Not implemented"
end
function util.IAssetImportFileHandler:GetFileList()
	return {}
end

util.register_class("util.ZipAssetImportFileHandler", util.IAssetImportFileHandler)
function util.ZipAssetImportFileHandler:__init(zipFile)
	util.IAssetImportFileHandler.__init(self)
	self.m_zipFile = zipFile
end
function util.ZipAssetImportFileHandler:ExtractFile(fileName, outPath)
	return self.m_zipFile:ExtractFile(fileName, outPath)
end
function util.ZipAssetImportFileHandler:GetFileList()
	return self.m_zipFile:GetFileList()
end

util.register_class("util.DropAssetImportFileHandler", util.IAssetImportFileHandler)
function util.DropAssetImportFileHandler:__init(tFiles)
	util.IAssetImportFileHandler.__init(self)
	local fileNames = {}
	for _, f in ipairs(tFiles) do
		local path = util.Path.CreateFilePath(f)
		table.insert(fileNames, path:GetString())
	end
	self.m_files = fileNames
end
function util.DropAssetImportFileHandler:ExtractFile(fileName, outPath)
	local f = game.open_dropped_file(fileName, true)
	if f == nil then
		return false, "Failed to open input file!"
	end
	local fOut = file.open(outPath, bit.bor(file.OPEN_MODE_WRITE, file.OPEN_MODE_BINARY))
	if fOut == nil then
		f:Close()
		return false, "Failed to open output file!"
	end
	local ds = f:Read(f:GetSize())
	f:Close()
	fOut:Write(ds)
	fOut:Close()
	return true
end
function util.DropAssetImportFileHandler:GetFileList()
	return self.m_files
end

util.register_class("util.ListAssetImportFileHandler", util.IAssetImportFileHandler)
function util.ListAssetImportFileHandler:__init(tFiles)
	util.IAssetImportFileHandler.__init(self)
	self.m_files = tFiles
end
function util.ListAssetImportFileHandler:ExtractFile(fileName, outPath)
	local r = file.copy(fileName, outPath)
	if r == false then
		return false, "Failed to copy input file!"
	end
	return r
	--[[local f = file.open(fname)
	if(f == nil) then return false,"Failed to open input file!" end
	local fOut = file.open(outPath,bit.bor(file.OPEN_MODE_WRITE,file.OPEN_MODE_BINARY))
	if(fOut == nil) then f:Close() return false,"Failed to open output file!" end
	local ds = f:Read(f:GetSize())
	f:Close()
	fOut:Write(ds)
	fOut:Close()
	return true]]
end
function util.ListAssetImportFileHandler:GetFileList()
	return self.m_files
end

local function import_assets(handler, settings)
	local basePath = settings.basePath or ""
	local logCb = settings.logger
		or function(msg, severity)
			if severity ~= log.SEVERITY_INFO then
				console.print_warning(msg)
			else
				print(msg)
			end
		end
	local callback = settings.modelImportCallback
	local onComplete = settings.onComplete
	local clearFiles = settings.clearFiles

	local files = handler:GetFileList()
	for i, f in ipairs(files) do
		files[i] = util.Path.CreateFilePath(f):GetString()
	end

	local function installFiles(dropPath, installPath)
		local numInstalled = 0
		local i = 1
		while i <= #files do
			local f = files[i]
			if f:sub(1, #dropPath) == dropPath then
				local addonFilePath = f:sub(#dropPath + 1)
				local fileInstallPath = installPath .. addonFilePath
				file.create_path(file.get_file_path(fileInstallPath))
				logCb("Extracting file '" .. f .. "' to '" .. fileInstallPath .. "'...", log.SEVERITY_INFO)
				local result, msg = handler:ExtractFile(f, fileInstallPath)
				if result == false then
					logCb(
						"Failed to extract file '" .. f .. "' to '" .. fileInstallPath .. "': " .. msg,
						log.SEVERITY_ERROR
					)
				end
				table.remove(files, i)
				numInstalled = numInstalled + 1
			else
				i = i + 1
			end
		end
		return numInstalled
	end

	for assetType = 0, asset.TYPE_COUNT - 1 do
		local exts = asset.get_supported_extensions(assetType, asset.FORMAT_TYPE_NATIVE)
		local rootAssetDir = asset.get_asset_root_directory(assetType)
		local altRootAssetDir
		if assetType == asset.TYPE_AUDIO then
			altRootAssetDir = "sound" -- We'll allow "sound" as alternative, since that is the asset directory name in the Sourc Engine
		end
		local i = 1
		while i <= #files do
			local f = files[i]
			local ext = file.get_file_extension(f, exts)
			if ext ~= nil then
				local path = util.Path.CreateFilePath(f)
				local pathC = path:ToComponents()
				if pathC[#pathC - 3] == "addons" then
					-- Found Pragma assets that appear to be part of a Pragma addon.
					-- Install the addon directly to Pragma.
					local pathToAddons = path:Copy()
					pathToAddons:PopBack() -- Pop filename
					pathToAddons:PopBack() -- Pop asset dir
					pathToAddons:PopBack() -- Pop addon
					local numInstalled = installFiles(pathToAddons:GetString(), "addons/")
					assert(numInstalled > 0)
				else
					local numInstalled = 0
					for j = #pathC - 1, 1, -1 do
						if pathC[j] == rootAssetDir or pathC[j] == altRootAssetDir then
							local addonName = pathC[j - 1]
							if addonName == nil then
								-- Asset has been dropped directly into Pragma.
								-- Just put it in the "imported" addon
								addonName = "imported"
							end
							local installPath = "addons/" .. addonName .. "/" .. pathC[j] .. "/"
							local dropPath = util.Path()
							for k = 1, j do
								dropPath = dropPath + (pathC[k] .. "/")
							end
							numInstalled = installFiles(dropPath:GetString(), installPath)
							assert(numInstalled > 0)
							break
						end
					end
					if numInstalled == 0 then
						-- The root addon dir for this asset type does not exist in the import files.
						if #pathC == 1 then
							-- It's just the asset file, install it directly
							installFiles(f, "addons/imported/" .. rootAssetDir .. "/" .. f)
						else
							-- Take everything except the root dir
							local installPath = path:Copy()
							installPath:PopFront()
							installPath = util.Path.CreateFilePath("addons/imported/" .. rootAssetDir .. "/")
								+ installPath
							installFiles(f, "addons/imported/" .. rootAssetDir .. "/" .. f)
						end
						i = i + 1
					end
				end
			else
				i = i + 1
			end
		end
	end

	local IMPORT_ASSET_TYPE_NATIVE = 0
	local IMPORT_ASSET_TYPE_SOURCE = 1
	local IMPORT_ASSET_TYPE_IMPORT = 2
	local IMPORT_ASSET_TYPE_NONE = 3
	local assetTypeToString = {
		[IMPORT_ASSET_TYPE_NATIVE] = "native",
		[IMPORT_ASSET_TYPE_SOURCE] = "Source Engine",
		[IMPORT_ASSET_TYPE_IMPORT] = "import",
		[IMPORT_ASSET_TYPE_NONE] = "none",
	}
	local nativeTypeMap = {}
	for _, ext in ipairs(asset.get_supported_extensions(asset.TYPE_MODEL, asset.FORMAT_TYPE_NATIVE)) do
		nativeTypeMap[ext] = true
	end
	local importTypeMap = {}
	for _, ext in ipairs(asset.get_supported_extensions(asset.TYPE_MODEL, asset.FORMAT_TYPE_IMPORT)) do
		importTypeMap[ext] = true
	end
	local function detect_format_type(files)
		for _, f in ipairs(files) do
			local path = util.Path.CreateFilePath(f)
			if path:IsFile() then
				local ext = path:GetFileExtension()
				if ext ~= nil then
					if nativeTypeMap[ext] ~= nil then
						return IMPORT_ASSET_TYPE_NATIVE
					end
				end
				if importTypeMap[ext] ~= nil then
					if ext == "mdl" or ext == "bsp" or ext == "vmt" or ext == "vtf" then
						return IMPORT_ASSET_TYPE_SOURCE
					end
					importTypeMap[ext] = path
				end
			end
		end

		for ext, path in pairs(importTypeMap) do
			if path ~= true then
				return IMPORT_ASSET_TYPE_IMPORT
			end
		end
		return IMPORT_ASSET_TYPE_NONE
	end

	local assetType = detect_format_type(files)
	if assetType == IMPORT_ASSET_TYPE_NONE then
		logCb("Could not detect model asset type! Attempting generic asset import...", log.SEVERITY_WARNING)
	else
		logCb("Detected " .. assetTypeToString[assetType] .. " asset type.", log.SEVERITY_INFO)
	end

	local extractedFiles = {}
	local mdlAssets = {}
	local importedAssets = {}
	if
		assetType == IMPORT_ASSET_TYPE_NONE
		or assetType == IMPORT_ASSET_TYPE_SOURCE
		or assetType == IMPORT_ASSET_TYPE_NATIVE
	then
		local tMaterials = {}
		local tModels = {}
		local tMaps = {}
		for _, f in ipairs(files) do
			local path = util.Path.CreateFilePath(f)
			if path:IsFile() then
				local ext = path:GetFileExtension()

				local nCur = 0
				local c, n = path:GetComponent(nCur)
				while c ~= nil do
					local t
					c = c:lower()
					if c == "models" then
						t = tModels
					elseif c == "materials" then
						t = tMaterials
					elseif c == "maps" then
						t = tMaps
					end
					if t ~= nil then
						table.insert(t, { path:GetString():sub(nCur + 1), f })
						break
					end
					nCur = n
					c, n = path:GetComponent(nCur)
				end
			end
		end

		if #tMaterials > 0 or #tModels > 0 or #tMaps > 0 then
			for _, t in ipairs({ tMaterials, tModels, tMaps }) do
				for _, f in ipairs(t) do
					local outPath = "addons/imported/" .. f[1] .. basePath
					logCb("Extracing file '" .. f[2] .. "' to '" .. outPath .. "'...", log.SEVERITY_INFO)
					local result, msg = handler:ExtractFile(f[2], outPath)
					if result == false then
						logCb("Failed to extract file: " .. msg, log.SEVERITY_ERROR)
					else
						table.insert(extractedFiles, outPath)
					end
				end
			end

			local function find_assets(t, assetExt, type)
				local assets = {}
				for _, f in ipairs(t) do
					local ext = file.get_file_extension(f[1])
					if ext ~= nil and ext:lower() == assetExt then
						local path = util.Path.CreateFilePath(f[1])
						path:PopFront()
						path:RemoveFileExtension()
						logCb("Detected " .. type .. ": '" .. path:GetString() .. "'", log.SEVERITY_INFO)
						table.insert(assets, path:GetString())
					end
				end
				return assets
			end
			-- Source Engine models still have to be imported
			find_assets(tMaterials, "vmt", "material")
			mdlAssets = find_assets(tModels, "mdl", "model")
			find_assets(tMaps, "bsp", "map")
		else
			local sourceEngineTypes = table.table_to_map({
				"vmt",
				"vtf",
				"mdl",
				"bsp",
				"vvd",
				"vtx",
				"phy",
				"ani",
			}, true)
			-- Assets are not organized in directories, just install them directly
			local assetTypes = { asset.TYPE_MODEL, asset.TYPE_MATERIAL, asset.TYPE_TEXTURE, asset.TYPE_MAP }
			for _, f in ipairs(files) do
				local path = util.Path.CreateFilePath(f)
				if path:IsFile() then
					local ext = path:GetFileExtension()
					if ext ~= nil then
						local nativeType = false
						for _, assetType in ipairs(assetTypes) do
							if asset.is_supported_extension(ext, assetType) then
								nativeType = true
								local outPath = util.Path.CreatePath(
									"addons/imported/" .. asset.get_asset_root_directory(assetType)
								) + util.Path.CreateFilePath(file.get_file_name(f))
								local result, msg = handler:ExtractFile(f, outPath:GetString())
								if result == false then
									logCb("Failed to extract file: " .. msg, log.SEVERITY_ERROR)
								end -- else table.insert(extractedFiles,outPath:GetString()) end
								break
							end
						end
						if nativeType == false and sourceEngineTypes[ext] == true then
							local rootDir = ""
							if ext == "vmt" or ext == "vtf" then
								rootDir = "materials/"
							elseif ext == "bsp" then
								rootDir = "maps/"
							else
								rootDir = "models/"
							end
							local outPath = "addons/imported/" .. rootDir .. file.get_file_name(f)
							local result, msg = handler:ExtractFile(f, outPath)
							if result == false then
								logCb("Failed to extract file: " .. msg, log.SEVERITY_ERROR)
							end -- else table.insert(extractedFiles,outPath) end
						end
					end
				end
			end
		end
	elseif assetType == IMPORT_ASSET_TYPE_IMPORT then
		-- Reset file list. If there are model import assets, they may require some of the assets (e.g. textures) that we have
		-- already copied.
		files = handler:GetFileList()

		local extractMap = {}
		local filePaths = {}
		for i, f in ipairs(files) do
			filePaths[i] = util.Path.CreateFilePath(f)
		end
		for _, path in ipairs(filePaths) do
			if path:IsFile() then
				local ext = path:GetFileExtension()
				if ext ~= nil and importTypeMap[ext] ~= nil then
					logCb("Detected import model '" .. path:GetString() .. "'.", log.SEVERITY_INFO)
					extractMap[path:GetPath()] = true
					table.insert(mdlAssets, path:GetFileName())
				end
			end
		end

		-- We'll copy all detected model assets, with all assets (and folders) located in the same directory
		-- to "addons/imported/", so Pragma can locate them.
		for rootPath, _ in pairs(extractMap) do
			for i, path in ipairs(filePaths) do
				path = path:Copy()
				if path:MakeRelative(rootPath) then
					local outPath = "addons/imported/models/" .. basePath .. path:GetString()
					file.create_path(file.get_file_path(outPath))
					logCb("Extracing file '" .. files[i] .. "' to '" .. outPath .. "'...", log.SEVERITY_INFO)
					local result, msg = handler:ExtractFile(files[i], outPath)
					if result == false then
						logCb("Failed to extract file: " .. msg, log.SEVERITY_ERROR)
					else
						table.insert(extractedFiles, outPath)
					end
				end
			end
		end
	end

	handler = nil
	collectgarbage()

	logCb("Found " .. #mdlAssets .. " model assets!", log.SEVERITY_INFO)
	local function import_next_model()
		if #mdlAssets == 0 then
			logCb("All models have been imported successfully!", log.SEVERITY_INFO)
			logCb("Removing temporary files...", log.SEVERITY_INFO)
			-- We don't need to keep the materials or models around, because they have been
			-- converted to native formats. We do, however, need to keep the textures.
			for _, f in ipairs(extractedFiles) do
				local ext = file.get_file_extension(f)
				if
					ext == nil
					or (
						asset.is_supported_extension(ext, asset.TYPE_MODEL) == false
						and asset.is_supported_extension(ext, asset.TYPE_MATERIAL) == false
						and asset.is_supported_extension(ext, asset.TYPE_TEXTURE) == false
						and asset.is_supported_extension(ext, asset.TYPE_MAP) == false
					)
				then
					file.delete(f)
				end
			end
			if clearFiles ~= nil then
				for _, f in ipairs(clearFiles) do
					file.delete(f)
				end
			end
			if onComplete ~= nil then
				onComplete(importedAssets)
			end
			return
		end
		local mdl = basePath .. mdlAssets[1]
		table.remove(mdlAssets, 1)
		time.create_simple_timer(0.25, function()
			logCb("Importing model '" .. mdl .. "'...", log.SEVERITY_INFO)

			local ext = file.get_file_extension(mdl)
			local handled = false
			if ext ~= nil then
				ext = ext:lower()
				local absPath = file.find_absolute_path(asset.get_asset_root_directory(asset.TYPE_MODEL) .. "/" .. mdl)
				if ext == "gltf" or ext == "glb" then
					handled = true
					if absPath ~= nil then
						local res, errMsg =
							asset.import_gltf(absPath, file.get_file_path(mdl), not settings.importAsCollection)
						if res ~= false then
							if #res.models > 0 then
								importedAssets[asset.TYPE_MODEL] = importedAssets[asset.TYPE_MODEL] or {}
								for _, mdlName in ipairs(res.models) do
									table.insert(importedAssets[asset.TYPE_MODEL], mdlName)

									if callback ~= nil then
										callback(asset.TYPE_MODEL, mdlName)
									end
								end
							end
							if #res.mapName > 0 then
								importedAssets[asset.TYPE_MAP] = importedAssets[asset.TYPE_MAP] or {}
								table.insert(importedAssets[asset.TYPE_MODEL], res.mapName)
							end

							if #res.mapName == 0 then
								logCb(#res.models .. " models have been imported!", log.SEVERITY_INFO)
							else
								logCb(
									#res.models .. " models and new map '" .. res.mapName .. "' have been imported!",
									log.SEVERITY_INFO
								)
							end
						else
							logCb("Unable to import model '" .. absPath .. "': " .. errMsg, log.SEVERITY_ERROR)
						end
					else
						logCb(
							"Unable to import model '" .. mdl .. "': Could not determine absolute file path!",
							log.SEVERITY_ERROR
						)
					end
				else
					local res = asset.import(absPath, mdl, asset.TYPE_MODEL)
					if res == false then
						logCb("Failed to import model '" .. mdl .. "'!", log.SEVERITY_ERROR)
					end
					mdl = file.remove_file_extension(mdl, { ext })
				end
			end

			if handled == false then
				local mdlName = mdl
				local mdl = game.load_model(mdlName)
				if mdl ~= nil then
					logCb("Model '" .. mdlName .. "' has been imported successfully!", log.SEVERITY_INFO)
					importedAssets[asset.TYPE_MODEL] = importedAssets[asset.TYPE_MODEL] or {}
					table.insert(importedAssets[asset.TYPE_MODEL], mdlName)

					if callback ~= nil then
						callback(asset.TYPE_MODEL, mdlName)
					end
				else
					logCb("Failed to import model!", log.SEVERITY_ERROR)
				end
			end
			asset.clear_unused(asset.TYPE_MODEL)
			asset.clear_unused(asset.TYPE_MATERIAL)
			asset.clear_unused(asset.TYPE_TEXTURE)
			import_next_model()
		end)
	end
	import_next_model()
end

function util.import_assets(files, settings)
	settings = settings or {}
	settings.logger = settings.logger
		or function(msg, severity)
			if severity ~= log.SEVERITY_INFO then
				console.print_warning(msg)
			else
				print(msg)
			end
		end
	settings.dropped = settings.dropped or false
	if type(files) ~= "table" then
		files = { files }
	end

	local nonZipFiles = {}
	local zipExts = {}
	for _, ext in ipairs(util.ZipFile.get_supported_format_extensions()) do
		zipExts[ext] = true
	end
	local hasZipAssets = false
	for _, f in ipairs(files) do
		local filePath
		if util.get_type_name(f) == "File" then
			filePath = f:GetPath()
		else
			filePath = f
		end
		local ext = file.get_file_extension(filePath)
		if ext ~= nil and zipExts[ext] == true then
			local f
			if settings.dropped then
				f = game.open_dropped_file(filePath, true)
			else
				f = file.open(filePath, bit.bor(file.OPEN_MODE_READ, file.OPEN_MODE_BINARY))
			end
			if f == nil then
				settings.logger("Unable to open zip-archive '" .. filePath .. "': File not found!", log.SEVERITY_ERROR)
			else
				local zipFile = util.ZipFile.open(f, util.ZipFile.OPEN_MODE_READ)
				if zipFile == nil then
					settings.logger(
						"Unable to open zip-archive '" .. filePath .. "': Unsupported archive format?",
						log.SEVERITY_ERROR
					)
				-- util.open_path_in_explorer(file.get_file_path(filePath), file.get_file_name(filePath))
				else
					hasZipAssets = true
					local handler = util.ZipAssetImportFileHandler(zipFile)
					zipFile = nil
					import_assets(handler, settings)
				end
				f:Close()
			end
		else
			table.insert(nonZipFiles, f)
		end
	end
	if #nonZipFiles == 0 then
		if hasZipAssets == false then
			settings.logger("No assets were detected!")
		end
		return
	end
	local handler = settings.dropped and util.DropAssetImportFileHandler(nonZipFiles)
		or util.ListAssetImportFileHandler(nonZipFiles)
	import_assets(handler, settings)
end
