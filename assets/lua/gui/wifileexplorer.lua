include("wibasefileexplorer.lua")

util.register_class("gui.WIFileExplorer",gui.Base,gui.BaseFileExplorer)

function gui.WIFileExplorer:__init()
	gui.Base.__init(self)
	gui.BaseFileExplorer.__init(self)
end

function gui.WIFileExplorer:OnUpdate()
	self:ListFiles()
end

function gui.WIFileExplorer:OnInitialize()
	gui.Base.OnInitialize(self)

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
function gui.WIFileExplorer:GetSelectedFile(relativePath)
	relativePath = relativePath or false
	local path
	if(relativePath == false) then path = self:GetAbsolutePath()
	else
		path = self:GetPath()
		if(path == "/") then path = "" end
	end
	local t = self.m_pFileList
	if(util.is_valid(t) == true) then
		local pRow = t:GetFirstSelectedRow()
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
	if(#path > 0 and path:sub(1,1) == "/") then path = path:sub(2,#path) end
	return path
end
function gui.WIFileExplorer:ListFiles()
	local t = self.m_pFileList
	if(util.is_valid(t) == false) then return end
	t:Clear()
	self.m_tItems = {}

	local tFiles,tDirectories = self:FindFiles()

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
	local path = self:GetPath()
	local numExts = #self.m_tExtensions
	if(self.m_isDirExplorer ~= true) then
		for _,fName in ipairs(tFiles) do
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
	t:SetSize(self:GetSize())
end
gui.register("WIFileExplorer",gui.WIFileExplorer)
