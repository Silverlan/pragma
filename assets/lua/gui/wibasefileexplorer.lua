util.register_class("gui.BaseFileExplorer")

function gui.BaseFileExplorer:__init()
	self.m_path = "/"
	self.m_rootPath = ""
	self.m_tItems = {}
end
function gui.BaseFileExplorer:IsInRootDirectory()
	return (#self:GetPath() == 0)
end
function gui.BaseFileExplorer:SetFilter(filter)
	self.m_filter = filter
end
function gui.BaseFileExplorer:OnPathChanged(path) end
function gui.BaseFileExplorer:SetPath(path)
	path = util.DirPath(path)
	path:Canonicalize()
	path = path:GetString()
	self:OnPathChanged(path)
	self:CallCallbacks("OnPathChanged", path)
	self.m_path = path
end
function gui.BaseFileExplorer:ReloadPath()
	self:SetPath(self:GetPath())
	self:Update()
end
function gui.BaseFileExplorer:Refresh()
	self:ReloadPath()
end
function gui.BaseFileExplorer:SetRootPath(path)
	path = util.DirPath(path)
	path:Canonicalize()
	path = path:GetString()
	self.m_rootPath = path
end
function gui.BaseFileExplorer:SetExtensions(exts)
	exts = exts or {}
	self.m_tExtensions = {}
	for _, ext in ipairs(exts) do
		self.m_tExtensions[ext] = true
	end
end
function gui.BaseFileExplorer:GetRootPath()
	return self.m_rootPath
end
function gui.BaseFileExplorer:IsAtRoot()
	return self.m_path == "/"
end
function gui.BaseFileExplorer:GetPath()
	local path = self.m_path
	if #path > 0 and path:sub(1, 1) == "/" then
		path = path:sub(2)
	end
	return path
end
function gui.BaseFileExplorer:GetAbsolutePath()
	local path = self:GetRootPath()
	local lpath = self.m_path
	if lpath ~= "/" then
		path = path .. lpath
	end
	return path
end
function gui.BaseFileExplorer:SetFileFinder(fcCallback)
	self.m_fileFinder = fcCallback
end
function gui.BaseFileExplorer:MakeDirectoryExplorer()
	self.m_isDirExplorer = true
end
function gui.BaseFileExplorer:IsValidExtension(ext)
	if self.m_tExtensions == nil then
		return true
	end
	if ext == nil then
		return false
	end
	return self.m_tExtensions[ext] ~= nil
end
function gui.BaseFileExplorer:FindFiles()
	local path = self:GetAbsolutePath()
	local tFiles, tDirectories
	if self.m_fileFinder ~= nil then
		tFiles, tDirectories = self.m_fileFinder(path .. "*")
	else
		tFiles, tDirectories = file.find(path .. "*")
	end
	if self.m_isDirExplorer ~= true then
		table.sort(tFiles)
	end
	table.sort(tDirectories)
	if self:IsInRootDirectory() == false then
		table.insert(tDirectories, 1, "..")
	end

	if self.m_isDirExplorer ~= true then
		local tFilesFiltered = {}
		for _, fName in ipairs(tFiles) do
			local ext = file.get_file_extension(fName)
			if self:IsValidExtension(ext) then
				table.insert(tFilesFiltered, fName)
			end
		end
		tFiles = tFilesFiltered
	end

	local preSorted = false
	if self.m_filter ~= nil then
		tFiles, tDirectories, preSorted = self.m_filter(tFiles, tDirectories)
	end

	return tFiles, tDirectories, preSorted
end
function gui.BaseFileExplorer:ListFiles() end
