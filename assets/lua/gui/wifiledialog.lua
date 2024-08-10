include("/gui/wifileexplorer.lua")
include("/gui/dialog.lua")

util.register_class("gui.WIFileDialog", gui.Base)

gui.WIFileDialog.TYPE_OPEN = 0
gui.WIFileDialog.TYPE_SAVE = 1

function gui.WIFileDialog:__init()
	gui.Base.__init(self)
end
function gui.WIFileDialog:OnInitialize()
	gui.Base.OnInitialize(self)

	local pMain = gui.create("WIRect", self)
	pMain:SetColor(Color.DimGray)
	pMain:SetAutoAlignToParent(true)
	pMain:Update()
	self.m_pMain = pMain

	local pPath = gui.create("WIText", self)
	self.m_pPath = pPath

	local pButtonOpen = gui.create("WIButton", self)
	pButtonOpen:AddCallback("OnPressed", function(pButton)
		self:CallCallbacks("OnFileSelected", self:GetFilePath(true))
		self:Close()
	end)
	self.m_pButtonOpen = pButtonOpen

	local pButtonCancel = gui.create("WIButton", self)
	pButtonCancel:SetText(locale.get_text("cancel"))
	pButtonCancel:AddCallback("OnPressed", function(pButton)
		self:Close()
	end)
	self.m_pButtonCancel = pButtonCancel

	local pLbFileName = gui.create("WIText", self)
	pLbFileName:SetText(locale.get_text("file_name") .. ":")
	pLbFileName:SizeToContents()
	self.m_pLbFileName = pLbFileName

	local pFileName = gui.create("WITextEntry", self)
	self.m_pFileName = pFileName

	local t = gui.create("WIFileExplorer", self)
	t:AddCallback("OnPathChanged", function(p, path)
		if util.is_valid(self) == false or util.is_valid(self.m_pPath) == false then
			return
		end
		self.m_pPath:SetText(path)
		self.m_pPath:SizeToContents()

		self:CallCallbacks("OnPathChanged", path)
	end)
	t:AddCallback("OnFileClicked", function(p, fName)
		if util.is_valid(self) == false then
			return
		end
		self:SetFileName(fName)

		self:CallCallbacks("OnFileClicked", fName)
	end)
	t:AddCallback("OnFileSelected", function(p, fPath)
		if util.is_valid(self) == false then
			return
		end
		self:CallCallbacks("OnFileSelected", fPath)
		self:Close()
	end)
	t:SetColumnEnabled(gui.WIFileExplorer.COLUMN_DATE_MODIFIED, true)
	t:UpdateColumns()
	self.m_pFileList = t

	local options = gui.create("WIPFMControlsMenu", self)
	options:SetAutoFillContentsToWidth(true)
	options:SetAutoFillContentsToHeight(false)
	self.m_pOptions = options

	self:SetType(gui.WIFileDialog.TYPE_OPEN)
	self:SetSize(512, 256)
end

function gui.WIFileDialog:MakeDirectoryExplorer()
	if util.is_valid(self.m_pFileList) == true then
		self.m_pFileList:MakeDirectoryExplorer()
	end
end
function gui.WIFileDialog:IsInRootDirectory()
	if util.is_valid(self.m_pFileList) == true then
		return self.m_pFileList:IsInRootDirectory()
	else
		return false
	end
end
function gui.WIFileDialog:SetPath(path)
	if util.is_valid(self.m_pFileList) == true then
		self.m_pFileList:SetPath(path)
	end
end
function gui.WIFileDialog:SetRootPath(path)
	if util.is_valid(self.m_pFileList) == true then
		self.m_pFileList:SetRootPath(path)
	end
end
function gui.WIFileDialog:SetExtensions(exts)
	if util.is_valid(self.m_pFileList) == true then
		self.m_pFileList:SetExtensions(exts)
	end
end
function gui.WIFileDialog:GetRootPath()
	if util.is_valid(self.m_pFileList) == true then
		return self.m_pFileList:GetRootPath()
	else
		return ""
	end
end
function gui.WIFileDialog:GetPath()
	if util.is_valid(self.m_pFileList) == true then
		return self.m_pFileList:GetPath()
	else
		return ""
	end
end
function gui.WIFileDialog:GetAbsolutePath()
	if util.is_valid(self.m_pFileList) == true then
		return self.m_pFileList:GetAbsolutePath()
	else
		return ""
	end
end
function gui.WIFileDialog:GetSelectedFile(relativePath)
	if util.is_valid(self.m_pFileList) == true then
		return self.m_pFileList:GetSelectedFile(relativePath)
	else
		return ""
	end
end
function gui.WIFileDialog:Update()
	if util.is_valid(self.m_pFileList) == true then
		self.m_pFileList:Update()
	end
	self:OnSizeChanged(self:GetWidth(), self:GetHeight())
end

function gui.WIFileDialog:SetFileName(fileName)
	if util.is_valid(self.m_pFileName) == false then
		return
	end
	self.m_pFileName:SetText(fileName)
end
function gui.WIFileDialog:GetFileList()
	return self.m_pFileList
end
function gui.WIFileDialog:MakePathRelative(path)
	local p = util.Path.CreateFilePath(path)
	p:MakeRelative(self:GetRootPath())
	return p:GetString()
end
function gui.WIFileDialog:MakePathAbsolute(path)
	return util.Path.CreatePath(self:GetRootPath()):GetString() .. path
end
function gui.WIFileDialog:GetFilePath(relativePath)
	if util.is_valid(self.m_pFileList) == false then
		return ""
	end
	local path = relativePath and self.m_pFileList:GetPath() or self.m_pFileList:GetAbsolutePath()
	if path == "/" then
		path = ""
	end
	if util.is_valid(self.m_pFileName) == false then
		return ""
	end
	return path .. self.m_pFileName:GetText()
end
function gui.WIFileDialog:SetType(type)
	self.m_type = type
	if type == gui.WIFileDialog.TYPE_OPEN then
		if util.is_valid(self.m_pButtonOpen) == true then
			self.m_pButtonOpen:SetText(locale.get_text("open"))
		end
	else
		if util.is_valid(self.m_pButtonOpen) == true then
			self.m_pButtonOpen:SetText(locale.get_text("save"))
		end
	end
end
function gui.WIFileDialog:GetType()
	return self.m_type
end
function gui.WIFileDialog:GetFrame()
	return self.m_pFrame
end
function gui.WIFileDialog:Close()
	gui.close_dialog()
end
function gui.WIFileDialog:GetOptionsPanel()
	return self.m_pOptions
end
function gui.WIFileDialog:OnSizeChanged(w, h)
	local margin = 10
	if util.is_valid(self.m_pButtonCancel) == false then
		return
	end
	local wButton = 80
	self.m_pButtonCancel:SetWidth(wButton)
	self.m_pButtonCancel:SetPos(w - margin - wButton, h - 50)

	if util.is_valid(self.m_pButtonOpen) == false then
		return
	end
	self.m_pButtonOpen:SetPos(w - 170, h - 50)
	if util.is_valid(self.m_pPath) == false then
		return
	end
	self.m_pPath:SetSize(256, 24)
	self.m_pPath:SetPos(margin, 5)

	local wFileList = w - margin * 2
	self.m_pOptions:Update()
	self.m_pOptions:SizeToContents()
	self.m_pOptions:SetWidth(wFileList)

	if util.is_valid(self.m_pFileList) == false then
		return
	end
	self.m_pFileList:SetPos(margin, self.m_pPath:GetY() + self.m_pPath:GetHeight())
	self.m_pFileList:SetSize(wFileList, h - 100 - self.m_pOptions:GetHeight())

	self.m_pOptions:SetPos(self.m_pFileList:GetLeft(), self.m_pFileList:GetBottom() + 10)

	if util.is_valid(self.m_pLbFileName) == false then
		return
	end
	self.m_pLbFileName:SetPos(margin, h - 47)
	if util.is_valid(self.m_pFileName) == false then
		return
	end
	local xOffset = self.m_pLbFileName:GetX() + self.m_pLbFileName:GetWidth()
	self.m_pFileName:SetPos(xOffset + margin, self.m_pLbFileName:GetY() - 1)
	self.m_pFileName:SetSize(self.m_pButtonOpen:GetX() - xOffset - margin * 2, 24)
end
gui.register("WIFileDialog", gui.WIFileDialog)

gui.create_file_open_dialog = function(fcOnFileSelected)
	local dialog, frame, fileDialog = gui.create_dialog(function()
		local el = gui.create("WIFileDialog")
		el:SetType(gui.WIFileDialog.TYPE_OPEN)
		if fcOnFileSelected ~= nil then
			el:AddCallback("OnFileSelected", fcOnFileSelected)
		end
		return el
	end)
	frame:GetDragArea():SetHeight(18)
	return fileDialog
end
gui.create_file_save_dialog = function(fcOnFileSelected)
	local dialog, frame, fileDialog = gui.create_dialog(function()
		local el = gui.create("WIFileDialog")
		el:SetType(gui.WIFileDialog.TYPE_SAVE)
		if fcOnFileSelected ~= nil then
			el:AddCallback("OnFileSelected", fcOnFileSelected)
		end
		return el
	end)
	frame:GetDragArea():SetHeight(18)
	return fileDialog
end
