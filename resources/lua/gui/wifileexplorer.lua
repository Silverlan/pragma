util.register_class("gui.WIFileExplorer",gui.Base)

function gui.WIFileExplorer:__init()
	gui.Base.__init(self)
end
function gui.WIFileExplorer:IsInRootDirectory()
	return (self:GetPath() == "/") and true or false
end
function gui.WIFileExplorer:SetPath(path)
	path = file.get_canonicalized_path(path)
	if(path:sub(-1) ~= "/") then path = path .. "/" end
	self:CallCallbacks("OnPathChanged",path)
	self.m_path = path
end
function gui.WIFileExplorer:SetRootPath(path)
	path = file.get_canonicalized_path(path)
	if(path:sub(-1) ~= "/") then path = path .. "/" end
	self.m_rootPath = path
end
function gui.WIFileExplorer:SetExtensions(exts)
	exts = exts or {}
	self.m_tExtensions = exts
end
function gui.WIFileExplorer:GetRootPath() return self.m_rootPath end
function gui.WIFileExplorer:GetPath() return self.m_path end
function gui.WIFileExplorer:GetAbsolutePath()
	local path = self:GetRootPath()
	local lpath = self:GetPath()
	if(lpath ~= "/") then path = path .. lpath end
	return path
end

function gui.WIFileExplorer:Update()
	self:ListFiles()
end

function gui.WIFileExplorer:OnInitialize()
	gui.Base.OnInitialize(self)

	self.m_path = "/"
	self.m_rootPath = ""
	self.m_tExtensions = {}
	self.m_tItems = {}

	local t = gui.create("WITable",self)
	t:SetAutoAlignToParent(true)
	t:SetScrollable(true)
	t:SetSelectableMode(gui.Table.SELECTABLE_MODE_SINGLE)
	t:SetRowHeight(20)
	local row = t:AddHeaderRow()
	row:SetValue(0,locale.get_text("name"))
	row:SetValue(1,locale.get_text("type"))
	row:SetValue(2,locale.get_text("size"))
	self.m_pFileList = t

	self:SetPath("/")
end
function gui.WIFileExplorer:GetSelectedFile()
	local path = self:GetAbsolutePath()
	local t = self.m_pFileList
	if(util.is_valid(t) == true) then
		local pRow = t:GetSelectedRow()
		if(pRow ~= nil) then
			local pCell = pRow:GetCell(0)
			if(pCell ~= nil) then
				local pText = pCell:GetFirstChild("WIText")
				if(pText ~= nil) then
					path = path .. pText:GetText()
				end
			end
		end
	end
	return path
end
function gui.WIFileExplorer:SetFileFinder(fcCallback)
	self.m_fileFinder = fcCallback
end
function gui.WIFileExplorer:ListFiles()
	local t = self.m_pFileList
	if(util.is_valid(t) == false) then return end
	t:Clear()
	self.m_tItems = {}

	local path = self:GetAbsolutePath()
	local tFiles,tDirectories
	if(self.m_fileFinder ~= nil) then tFiles,tDirectories = self.m_fileFinder(path .. "*")
	else tFiles,tDirectories = file.find(path .. "*") end
	table.sort(tFiles)
	table.sort(tDirectories)
	if(self:IsInRootDirectory() == false) then
		table.insert(tDirectories,1,"..")
	end

	local function create_icon(icon)
		local margin = 4
		local pContainer = gui.create("WIBase")
		local pIcon = gui.create("WISilkIcon",pContainer)
		pIcon:SetIcon(icon)
		local size = pIcon:GetSize()
		size.x = size.x +margin *2
		size.y = size.y +margin *2
		pContainer:SetSize(size)
		return pContainer
	end

	for _,file in ipairs(tDirectories) do
		local pIcon = create_icon("folder")

		local row = t:AddRow()
		row:InsertElement(0,pIcon)
		row:SetValue(0,file)
		row:SetValue(1,locale.get_text("file_folder"))
		row:SetValue(2,"")
		row:AddCallback("OnDoubleClick",function(pRow)
			if(util.is_valid(self) == false) then return end
			self:SetPath(self:GetPath() .. file)
			self:Update()
		end)
	end
	local numExts = #self.m_tExtensions
	for _,fName in ipairs(tFiles) do
		local ext = file.get_file_extension(fName)
		local bIsExt = (numExts == 0) and true or false
		if(bIsExt == false) then
			if(ext ~= nil) then
				for i=1,numExts do
					if(self.m_tExtensions[i] == ext) then
						bIsExt = true
						break
					end
				end
			end
		end
		if(bIsExt == true) then
			local pIcon = create_icon("page")

			local fPath = path .. fName
			local sz = file.get_size(fPath)
			local fileType = (ext ~= nil) and ext:upper() or locale.get_text("unknown")
			fileType = fileType .. " " .. locale.get_text("file")

			local row = t:AddRow()
			row:InsertElement(0,pIcon)
			row:SetValue(0,fName)
			row:SetValue(1,fileType)
			row:SetValue(2,util.get_pretty_bytes(sz))
			row:AddCallback("OnMouseEvent",function(pRow,button,action,mods)
				if(util.is_valid(self) == false) then return end
				if(button == input.MOUSE_BUTTON_LEFT and action == input.STATE_PRESS) then
					self:CallCallbacks("OnFileClicked",fName)
				end
			end)
			row:AddCallback("OnDoubleClick",function(pRow)
				if(util.is_valid(self) == false) then return end
				self:CallCallbacks("OnFileSelected",fPath)
			end)
		end
	end
end
gui.register("WIFileExplorer",gui.WIFileExplorer)
