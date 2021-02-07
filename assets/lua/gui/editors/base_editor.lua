include("/gui/wimenubar.lua")
include("/gui/witoolbar.lua")
include("/gui/wifiledialog.lua")
include("/gui/pfm/infobar.lua")
include("/gui/pfm/frame.lua")

util.register_class("gui.WIBaseEditor",gui.Base)
function gui.WIBaseEditor:__init()
	gui.Base.__init(self)
end

function gui.WIBaseEditor:OnVisibilityChanged(bVisible)
	if(bVisible == false) then gui.close_context_menu() end
end

function gui.WIBaseEditor:OnInitialize()
	self:SetMouseInputEnabled(true)
	self:TrapFocus(true)
	self:RequestFocus()

	local pMain = gui.create("WIRect",self)
	pMain:SetColor(Color.Beige)--Color(40,40,40,255))
	pMain:Update()
	self.m_pMain = pMain
	self.m_windowToPrimaryWindow = {}
	self.m_windowFactories = {}

	self.m_menuBar = gui.create("WIMenuBar",self)
	self.m_menuBar:AddCallback("OnClose",function(pMenuBar)
		if(util.is_valid(self)) then return end
		self:Close()
	end)

	self:SetSize(1024,768)
	self.m_menuBar:SetSize(self:GetWidth(),20)
	self.m_menuBar:SetAnchor(0,0,1,0)

	local pInfoBar = gui.create("WIPFMInfobar",self)
	pInfoBar:SetWidth(self:GetWidth())
	pInfoBar:SetY(self:GetHeight() -pInfoBar:GetHeight())
	pInfoBar:SetAnchor(0,1,1,1)
	self.m_infoBar = pInfoBar
end

function gui.WIBaseEditor:OnSizeChanged(w,h)
	if(util.is_valid(self.m_infoBar) == false or util.is_valid(self.m_menuBar) == false) then return end
	self.m_pMain:SetSize(w,h -self.m_menuBar:GetHeight() -self.m_infoBar:GetHeight())
	self.m_pMain:SetY(self.m_menuBar:GetHeight())
end

function gui.WIBaseEditor:GetInfoBar() return self.m_infoBar end

function gui.WIBaseEditor:SetDeveloperModeEnabled(dev) self.m_devModeEnabled = dev or false end
function gui.WIBaseEditor:IsDeveloperModeEnabled() return self.m_devModeEnabled or false end

function gui.WIBaseEditor:AddWindowsMenuBarItem()
	self.m_menuBar:AddItem(locale.get_text("windows"),function(pContext)
		for identifier,data in pairs(self.m_windowFactories) do
			pContext:AddItem(data.title,function(pItem)
				self:OpenWindow(identifier)
				self:GoToWindow(identifier)
			end)
		end
		pContext:Update()
	end)
end

function gui.WIBaseEditor:InitializeGenericLayout()
	self.m_contents = gui.create("WIHBox",self,0,self.m_menuBar:GetHeight(),self:GetWidth(),self:GetHeight() -self.m_menuBar:GetHeight() -self.m_infoBar:GetHeight(),0,0,1,1)
	self.m_contents:SetAutoFillContents(true)
end

function gui.WIBaseEditor:GoToWindow(identifier)
	for _,window in ipairs(self.m_frames) do
		window:SetActiveTab(identifier)
	end
end

function gui.WIBaseEditor:RegisterWindow(primaryWindow,identifier,title,factory)
	self.m_windowFactories[identifier] = {
		window = primaryWindow,
		title = title,
		factory = factory
	}
end

function gui.WIBaseEditor:GetPrimaryWindow(window) return self.m_windowToPrimaryWindow[window] end

function gui.WIBaseEditor:CloseWindow(identifier)
	if(self.m_windowFactories[identifier] == nil or util.is_valid(self.m_windowFactories[identifier].element)) then return end
	local data = self.m_windowFactories[identifier]
	if(util.is_valid(data.window)) then data.window:RemoveTab(identifier) end
end

function gui.WIBaseEditor:IsWindowOpen(identifier)
	local window = self:GetWindow(identifier)
	return util.is_valid(window)
end

function gui.WIBaseEditor:IsWindowActive(identifier)
	if(self:IsWindowOpen(identifier) == false) then return false end
	return self:GetWindow(identifier):IsVisible()
end

function gui.WIBaseEditor:OpenWindow(identifier,goToWindow)
	if(self.m_windowFactories[identifier] == nil or util.is_valid(self.m_windowFactories[identifier].element)) then
		if(goToWindow) then self:GoToWindow(identifier) end
		if(self.m_windowFactories[identifier] ~= nil) then
			return self.m_windowFactories[identifier].window:FindTab(identifier),self.m_windowFactories[identifier].element
		end
		return
	end
	local data = self.m_windowFactories[identifier]
	if(util.is_valid(data.window) == false) then return end
	local el = data.factory()
	if(util.is_valid(el) == false) then return end
	self.m_windowToPrimaryWindow[el] = data.window
	data.element = el
	local tab = data.window:AddTab(identifier,data.title,el)
	if(goToWindow) then self:GoToWindow(identifier) end
	return tab,el
end

function gui.WIBaseEditor:GetWindow(identifier) return self.m_windowFactories[identifier] ~= nil and self.m_windowFactories[identifier].element or nil end

function gui.WIBaseEditor:ClearLayout()
	if(util.is_valid(self.m_contents)) then self.m_contents:Remove() end
	self.m_frames = {}
end

function gui.WIBaseEditor:AddFrame(parent)
	parent = parent or self.m_contents
	if(util.is_valid(parent) == false) then return end
	local frame = gui.create("WIPFMFrame",parent)
	if(frame == nil) then return end
	table.insert(self.m_frames,frame)
	return frame
end

function gui.WIBaseEditor:GetMenuBar() return self.m_menuBar end
function gui.WIBaseEditor:SetBackgroundColor(col) self.m_pMain:SetColor(col) end
function gui.WIBaseEditor:CreateWindow(class)
	local pFrame = gui.create("WIFrame")
	if(pFrame == nil) then return end
	local p = gui.create(class or "WIEditorWindow",pFrame)
	if(p == nil) then
		pFrame:Remove()
		return
	end
	p:SetFrame(pFrame)
	return p
end

function gui.WIBaseEditor:Close()
	self:Remove()
end
function gui.WIBaseEditor:Open()

end
