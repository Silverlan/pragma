--[[
	Copyright (C) 2021 Silverlan

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

function util.import_assets(path,logCb)
	if(type(path) == "string") then path = util.Path.CreateFilePath(path) end
	logCb = logCb or function(msg) print(msg) end
	local zipFile = util.ZipFile.open(path:GetString(),util.ZipFile.OPEN_MODE_READ)
	if(zipFile == nil) then
		logCb("Unable to open zip-archive '" .. path:GetString() .. "': Unsupported archive format?")
		return
	end
	local files = zipFile:GetFileList()

	local IMPORT_ASSET_TYPE_NATIVE = 0
	local IMPORT_ASSET_TYPE_SOURCE = 1
	local IMPORT_ASSET_TYPE_IMPORT = 2
	local IMPORT_ASSET_TYPE_NONE = 3
	local assetTypeToString = {
		[IMPORT_ASSET_TYPE_NATIVE] = "native",
		[IMPORT_ASSET_TYPE_SOURCE] = "Source Engine",
		[IMPORT_ASSET_TYPE_IMPORT] = "import",
		[IMPORT_ASSET_TYPE_NONE] = "none"
	}
	local nativeTypeMap = {}
	for _,ext in ipairs(asset.get_supported_extensions(asset.TYPE_MODEL,asset.FORMAT_TYPE_NATIVE)) do
		nativeTypeMap[ext] = true
	end
	local importTypeMap = {}
	for _,ext in ipairs(asset.get_supported_extensions(asset.TYPE_MODEL,asset.FORMAT_TYPE_IMPORT)) do
		importTypeMap[ext] = true
	end
	local function detect_format_type(files)
		for _,f in ipairs(files) do
			local path = util.Path.CreateFilePath(f)
			if(path:IsFile()) then
				local ext = path:GetFileExtension()
				if(ext ~= nil) then
					if(nativeTypeMap[ext] ~= nil) then return IMPORT_ASSET_TYPE_NATIVE end
				end
				if(importTypeMap[ext] ~= nil) then
					if(ext == "mdl") then return IMPORT_ASSET_TYPE_SOURCE end
					importTypeMap[ext] = path
				end
			end
		end

		for ext,path in pairs(importTypeMap) do
			if(path ~= true) then return IMPORT_ASSET_TYPE_IMPORT end
		end
		return IMPORT_ASSET_TYPE_NONE
	end

	local assetType = detect_format_type(files)
	if(assetType == IMPORT_ASSET_TYPE_NONE) then logCb("Could not detect model asset type! Attempting generic asset import...")
	else logCb("Detected " .. assetTypeToString[assetType] .. " asset type.") end

	local extractedFiles = {}
	local mdlAssets = {}
	if(assetType == IMPORT_ASSET_TYPE_NONE or assetType == IMPORT_ASSET_TYPE_SOURCE or assetType == IMPORT_ASSET_TYPE_NATIVE) then
		local tMaterials = {}
		local tModels = {}
		local tMaps = {}
		for _,f in ipairs(files) do
			local path = util.Path.CreateFilePath(f)
			if(path:IsFile()) then
				local ext = path:GetFileExtension()

				local nCur = 0
				local c,n = path:GetComponent(nCur)
				while(c ~= nil) do
					local t
					c = c:lower()
					if(c == "models") then t = tModels
					elseif(c == "materials") then t = tMaterials
					elseif(c == "maps") then t = tMaps end
					if(t ~= nil) then
						table.insert(t,{path:GetString():sub(nCur +1),f})
						break
					end
					nCur = n
					c,n = path:GetComponent(nCur)
				end
			end
		end

		for _,t in ipairs({tMaterials,tModels,tMaps}) do
			for _,f in ipairs(t) do
				local outPath = "addons/imported/" .. f[1]
				logCb("Extracing file '" .. f[2] .. "' to '" .. outPath .. "'...")
				local result,msg = zipFile:ExtractFile(f[2],outPath)
				if(result == false) then
					logCb("{[c:ff0000]}Failed to extract file: " .. msg .. "{[/c]}")
				else table.insert(extractedFiles,outPath) end
			end
		end

		local function find_assets(t,assetExt,type)
			local assets = {}
			for _,f in ipairs(t) do
				local ext = file.get_file_extension(f[1])
				if(ext ~= nil and ext:lower() == assetExt) then
					local path = util.Path.CreateFilePath(f[1])
					path:PopFront()
					path:RemoveFileExtension()
					logCb("Detected " .. type .. ": '" .. path:GetString() .. "'")
					table.insert(assets,path:GetString())
				end
			end
			return assets
		end
		-- Source Engine models still have to be imported
		find_assets(tMaterials,"vmt","material")
		mdlAssets = find_assets(tModels,"mdl","model")
		find_assets(tMaps,"bsp","map")
	elseif(assetType == IMPORT_ASSET_TYPE_IMPORT) then
		local extractMap = {}
		local filePaths = {}
		for i,f in ipairs(files) do
			filePaths[i] = util.Path.CreateFilePath(f)
		end
		for _,path in ipairs(filePaths) do
			if(path:IsFile()) then
				local ext = path:GetFileExtension()
				if(ext ~= nil and importTypeMap[ext] ~= nil) then
					logCb("Detected import model '" .. path:GetString() .. "'.")
					extractMap[path:GetPath()] = true
					table.insert(mdlAssets,path:GetFileName())
				end
			end
		end

		-- We'll copy all detected model assets, with all assets (and folders) located in the same directory
		-- to "addons/imported/", so Pragma can locate them.
		for rootPath,_ in pairs(extractMap) do
			for i,path in ipairs(filePaths) do
				if(path:MakeRelative(rootPath)) then
					local outPath = "addons/imported/models/" .. path:GetString()
					logCb("Extracing file '" .. files[i] .. "' to '" .. outPath .. "'...")
					local result,msg = zipFile:ExtractFile(files[i],outPath)
					if(result == false) then
						logCb("{[c:ff0000]}Failed to extract file: " .. msg .. "{[/c]}")
					else table.insert(extractedFiles,outPath) end
				end
			end
		end
	end

	zipFile = nil
	collectgarbage()

	local function import_next_model()
		if(#mdlAssets == 0) then
			logCb("All models have been imported successfully!")
			logCb("Removing temporary files...")
			-- We don't need to keep the materials or models around, because they have been
			-- converted to native formats. We do, however, need to keep the textures.
			for _,f in ipairs(extractedFiles) do
				local ext = file.get_file_extension(f)
				if(ext == nil or ext:lower() ~= "vtf") then
					file.delete(f)
				end
			end
			file.delete(path:GetString())
			return
		end
		local mdl = mdlAssets[1]
		table.remove(mdlAssets,1)
		time.create_simple_timer(0.25,function()
			logCb("Importing model '" .. mdl .. "'...")
			local mdl = game.load_model(mdl)
			if(mdl ~= nil) then logCb("Model has been imported successfully!")
			else logCb("{[c:ff0000]}Failed to import model!{[/c]}") end
			asset.clear_unused(asset.TYPE_MODEL)
			asset.clear_unused(asset.TYPE_MATERIAL)
			asset.clear_unused(asset.TYPE_TEXTURE)
			import_next_model()
		end)
	end
	import_next_model()
end
