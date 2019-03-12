include("/gui/wifileexplorer.lua")

util.register_class("gui.WIFileDialog",gui.Base)

FILE_DIALOG_TYPE_OPEN = 0
FILE_DIALOG_TYPE_SAVE = 1

function gui.WIFileDialog:__init()
	gui.Base.__init(self)
end
function gui.WIFileDialog:OnInitialize()
	gui.Base.OnInitialize(self)

	local pMain = gui.create("WIRect",self)
	pMain:SetColor(Color.DimGray)
	pMain:SetAutoAlignToParent(true)
	pMain:Update()
	self.m_pMain = pMain

	local pMainOutline = gui.create("WIOutlinedRect",self)
	pMainOutline:SetAutoAlignToParent(true)
	pMainOutline:SetColor(Color.DodgerBlue)

	local pPath = gui.create("WIText",self)
	self.m_pPath = pPath

	local pButtonOpen = gui.create("WIButton",self)
	pButtonOpen:AddCallback("OnPressed",function(pButton)
		if(util.is_valid(self)) then self:Close() end
		self:CallCallbacks("OnFileSelected",self:GetFilePath())
	end)
	self.m_pButtonOpen = pButtonOpen

	local pButtonCancel = gui.create("WIButton",self)
	pButtonCancel:SetText(locale.get_text("cancel"))
	pButtonCancel:AddCallback("OnPressed",function(pButton)
		if(util.is_valid(self)) then self:Close() end
	end)
	self.m_pButtonCancel = pButtonCancel

	local pLbFileName = gui.create("WIText",self)
	pLbFileName:SetText(locale.get_text("file_name") .. ":")
	pLbFileName:SizeToContents()
	self.m_pLbFileName = pLbFileName

	local pFileName = gui.create("WITextEntry",self)
	self.m_pFileName = pFileName

	local t = gui.create("WIFileExplorer",self)
	t:AddCallback("OnPathChanged",function(p,path)
		if(util.is_valid(self) == false or util.is_valid(self.m_pPath) == false) then return end
		self.m_pPath:SetText(path)
		self.m_pPath:SizeToContents()

		self:CallCallbacks("OnPathChanged",path)
	end)
	t:AddCallback("OnFileClicked",function(p,fName)
		if(util.is_valid(self) == false) then return end
		self:SetFileName(fName)
		
		self:CallCallbacks("OnFileClicked",fName)
	end)
	t:AddCallback("OnFileSelected",function(p,fPath)
		if(util.is_valid(self) == false) then return end
		self:CallCallbacks("OnFileSelected",fPath)
		self:Close()
	end)
	self.m_pFileList = t

	local parent = self:GetParent()
	local pBg = gui.create("WIRect",parent)
	pBg:SetAutoAlignToParent(true)
	local col = Color.Black:Copy()
	col.a = 220
	pBg:SetColor(col)
	self:RemoveElementOnRemoval(pBg)

	local sz = gui.get_window_size()
	local pFrame = gui.create("WITransformable",parent)
	pFrame:SetDraggable(true)
	pFrame:SetResizable(true)
	pFrame:SetMinSize(Vector2i(300,200))
	pFrame:SetMaxSize(Vector2i(800,512))
	pFrame:SetSize(512,256)
	self:RemoveElementOnRemoval(pFrame)
	self.m_pFrame = pFrame

	self:SetParent(pFrame)
	self:SetAutoAlignToParent(true)

	pFrame:TrapFocus(true)
	pFrame:RequestFocus()

	local pDrag = pFrame:GetDragArea()
	pDrag:SetHeight(18)
	pDrag:SetAutoAlignToParent(true,false)

	pFrame:SetPos(sz.x *0.5 -pFrame:GetWidth() *0.5,sz.y *0.5 -pFrame:GetHeight() *0.5)

	self:SetType(FILE_DIALOG_TYPE_OPEN)
end

function gui.WIFileDialog:IsInRootDirectory() if(util.is_valid(self.m_pFileList) == true) then return self.m_pFileList:IsInRootDirectory() else return false end end
function gui.WIFileDialog:SetPath(path) if(util.is_valid(self.m_pFileList) == true) then self.m_pFileList:SetPath(path) end end
function gui.WIFileDialog:SetRootPath(path) if(util.is_valid(self.m_pFileList) == true) then self.m_pFileList:SetRootPath(path) end end
function gui.WIFileDialog:SetExtensions(exts) if(util.is_valid(self.m_pFileList) == true) then self.m_pFileList:SetExtensions(exts) end end
function gui.WIFileDialog:GetRootPath() if(util.is_valid(self.m_pFileList) == true) then return self.m_pFileList:GetRootPath() else return "" end end
function gui.WIFileDialog:GetPath() if(util.is_valid(self.m_pFileList) == true) then return self.m_pFileList:GetPath() else return "" end end
function gui.WIFileDialog:GetAbsolutePath() if(util.is_valid(self.m_pFileList) == true) then return self.m_pFileList:GetAbsolutePath() else return "" end end
function gui.WIFileDialog:Update() if(util.is_valid(self.m_pFileList) == true) then self.m_pFileList:Update() end end

function gui.WIFileDialog:SetFileName(fileName)
	if(util.is_valid(self.m_pFileName) == false) then return end
	self.m_pFileName:SetText(fileName)
end
function gui.WIFileDialog:GetFileList() return self.m_pFileList end
function gui.WIFileDialog:GetFilePath()
	if(util.is_valid(self.m_pFileList) == false) then return "" end
	local path = self.m_pFileList:GetAbsolutePath()
	if(util.is_valid(self.m_pFileName) == false) then return "" end
	return path .. self.m_pFileName:GetText()
end
function gui.WIFileDialog:SetType(type)
	self.m_type = type
	if(type == FILE_DIALOG_TYPE_OPEN) then
		if(util.is_valid(self.m_pButtonOpen) == true) then
			self.m_pButtonOpen:SetText(locale.get_text("open"))
		end
	else
		if(util.is_valid(self.m_pButtonOpen) == true) then
			self.m_pButtonOpen:SetText(locale.get_text("save"))
		end
	end
end
function gui.WIFileDialog:GetType() return self.m_type end
function gui.WIFileDialog:GetFrame() return self.m_pFrame end
function gui.WIFileDialog:Close()
	self:RemoveSafely()
end
function gui.WIFileDialog:OnSizeChanged(w,h)
	local margin = 10
	if(util.is_valid(self.m_pButtonCancel) == false) then return end
	local wButton = 80
	self.m_pButtonCancel:SetWidth(wButton)
	self.m_pButtonCancel:SetPos(w -margin -wButton,h -50)

	if(util.is_valid(self.m_pButtonOpen) == false) then return end
	self.m_pButtonOpen:SetPos(w -170,h -50)
	self.m_pButtonOpen:AddCallback("OnPressed",function(pButton)
		
	end)
	if(util.is_valid(self.m_pPath) == false) then return end
	self.m_pPath:SetSize(256,24)
	self.m_pPath:SetPos(margin,5)

	if(util.is_valid(self.m_pFileList) == false) then return end
	self.m_pFileList:SetPos(margin,self.m_pPath:GetY() +self.m_pPath:GetHeight())
	self.m_pFileList:SetSize(w -margin *2,h -100)

	if(util.is_valid(self.m_pLbFileName) == false) then return end
	self.m_pLbFileName:SetPos(margin,h -47)
	if(util.is_valid(self.m_pFileName) == false) then return end
	local xOffset = self.m_pLbFileName:GetX() +self.m_pLbFileName:GetWidth()
	self.m_pFileName:SetPos(xOffset +margin,self.m_pLbFileName:GetY() -1)
	self.m_pFileName:SetSize(self.m_pButtonOpen:GetX() -xOffset -margin *2,24)
end
gui.register("WIFileDialog",gui.WIFileDialog)

local pDialog
gui.close_dialog = function()
	if(pDialog == nil or util.is_valid(pDialog) == false) then return end
	pDialog:Remove()
	pDialog = nil
end
gui.create_file_open_dialog = function(fcOnFileSelected)
	gui.close_dialog()

	pDialog = gui.create("WIFileDialog")
	pDialog:SetType(FILE_DIALOG_TYPE_OPEN)
	pDialog:Update()
	if(fcOnFileSelected ~= nil) then pDialog:AddCallback("OnFileSelected",fcOnFileSelected) end
	return pDialog
end
gui.create_file_save_dialog = function(fcOnFileSelected)
	gui.close_dialog()

	pDialog = gui.create("WIFileDialog")
	pDialog:SetType(FILE_DIALOG_TYPE_SAVE)
	pDialog:Update()
	if(fcOnFileSelected ~= nil) then pDialog:AddCallback("OnFileSelected",fcOnFileSelected) end
	return pDialog
end
