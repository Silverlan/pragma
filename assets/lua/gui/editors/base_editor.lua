include("/gui/wimenubar.lua")
include("/gui/witoolbar.lua")
include("/gui/wifiledialog.lua")
include("/gui/pfm/infobar.lua")
include("/gui/pfm/frame.lua")

util.register_class("gui.WIBaseEditor", gui.Base)
function gui.WIBaseEditor:__init()
	gui.Base.__init(self)
end

function gui.WIBaseEditor:OnVisibilityChanged(bVisible)
	if bVisible == false then
		gui.close_context_menu()
	end
end

function gui.WIBaseEditor:OpenContextMenu()
	return gui.open_context_menu(self)
end

function gui.WIBaseEditor:OnInitialize()
	self:SetMouseInputEnabled(true)
	self:TrapFocus(true)
	self:RequestFocus()
	self:SetBaseElement(true)

	self.m_windowFrames = {}

	local pMain = gui.create("WIRect", self)
	pMain:SetColor(Color.Beige) --Color(40,40,40,255))
	pMain:Update()
	self.m_pMain = pMain
	self.m_windowToFrame = {}
	self.m_windowFactories = {}

	self.m_menuBarContainer = gui.create("WIBase", self)

	self.m_menuBar = gui.create("WIMenuBar", self.m_menuBarContainer)
	self.m_menuBar:SetName("menu_bar")
	self.m_menuBar:AddCallback("OnClose", function(pMenuBar)
		if util.is_valid(self) then
			return
		end
		self:Close()
	end)

	self:SetSize(1024, 768)
	self.m_menuBarContainer:SetSize(self:GetWidth(), 20)
	self.m_menuBarContainer:SetAnchor(0, 0, 1, 0)
	self.m_menuBar:SetSize(self.m_menuBarContainer:GetSize())
	self.m_menuBar:SetAnchor(0, 0, 1, 1)

	local pInfoBar = gui.create("WIPFMInfobar", self)
	pInfoBar:SetName("info_bar")
	pInfoBar:SetWidth(self:GetWidth())
	pInfoBar:SetY(self:GetHeight() - pInfoBar:GetHeight())
	pInfoBar:SetAnchor(0, 1, 1, 1)
	self.m_infoBar = pInfoBar
end

function gui.WIBaseEditor:RegisterFrame(category, frame)
	self.m_windowFrames[category] = frame
end
function gui.WIBaseEditor:GetFrames()
	return self.m_windowFrames
end

function gui.WIBaseEditor:GetFirstFrame()
	if util.is_valid(self.m_firstFrame) then
		return self.m_firstFrame
	end
	for _, frame in pairs(self.m_windowFrames) do
		if util.is_valid(frame) then
			self.m_firstFrame = frame
			return frame
		end
	end
end

function gui.WIBaseEditor:OnRemove() end

function gui.WIBaseEditor:OnSizeChanged(w, h)
	if util.is_valid(self.m_infoBar) == false or util.is_valid(self.m_menuBar) == false then
		return
	end
	self.m_pMain:SetSize(w, h - self.m_menuBar:GetHeight() - self.m_infoBar:GetHeight())
	self.m_pMain:SetY(self.m_menuBar:GetHeight())
end

function gui.WIBaseEditor:GetInfoBar()
	return self.m_infoBar
end

function gui.WIBaseEditor:SetDeveloperModeEnabled(dev)
	self.m_devModeEnabled = dev or false
end
function gui.WIBaseEditor:IsDeveloperModeEnabled()
	return self.m_devModeEnabled or false
end

function gui.WIBaseEditor:AddWindowsMenuBarItem(fcView)
	self.m_menuBar
		:AddItem(locale.get_text("view"), function(pContext)
			local pItem, pSubMenu = pContext:AddSubMenu(locale.get_text("windows"))
			pItem:SetName("windows")
			local windows = {}
			for identifier, data in pairs(self.m_windowFactories) do
				table.insert(windows, { data.title, identifier })
			end
			table.sort(windows, function(a, b)
				return a[1] < b[1]
			end)
			for _, wdata in ipairs(windows) do
				local pSubItem = pSubMenu:AddItem(wdata[1], function(pItem)
					self:OpenWindow(wdata[2])
					self:GoToWindow(wdata[2])
				end)
				pSubItem:SetName(wdata[2])
			end
			pSubMenu:ScheduleUpdate()

			if fcView ~= nil then
				fcView(pContext)
			end

			pContext:ScheduleUpdate()
		end)
		:SetName("view")
end

function gui.WIBaseEditor:InitializeGenericLayout()
	self.m_contents = gui.create(
		"WIHBox",
		self,
		0,
		self.m_menuBar:GetHeight(),
		self:GetWidth(),
		self:GetHeight() - self.m_menuBar:GetHeight() - self.m_infoBar:GetHeight(),
		0,
		0,
		1,
		1
	)
	self.m_contents:SetName("contents")
	self.m_contents:SetAutoFillContents(true)
end

function gui.WIBaseEditor:GetContentsElement()
	return self.m_contents
end

function gui.WIBaseEditor:GoToWindow(identifier)
	for _, window in ipairs(self.m_frames) do
		window:SetActiveTab(identifier)
	end
end

function gui.WIBaseEditor:RegisterWindow(category, identifier, title, factory)
	self.m_windowFactories[identifier] = {
		category = category,
		title = title,
		factory = factory,
	}
end

function gui.WIBaseEditor:GetPrimaryWindow(window)
	return self.m_windowToFrame[window]
end

function gui.WIBaseEditor:CloseWindow(identifier)
	if self.m_windowFactories[identifier] == nil or util.is_valid(self.m_windowFactories[identifier].element) then
		return
	end
	local data = self.m_windowFactories[identifier]
	local frame = self.m_windowToFrame[self.m_windowFactories[identifier].element]
	if util.is_valid(frame) then
		frame:RemoveTab(identifier)
	end
end

function gui.WIBaseEditor:GetOpenWindowIdentifiers()
	local t = {}
	for identifier, data in pairs(self.m_windowFactories) do
		if util.is_valid(data.element) then
			table.insert(t, identifier)
		end
	end
	return t
end

function gui.WIBaseEditor:IsWindowOpen(identifier)
	local window = self:GetWindow(identifier)
	return util.is_valid(window)
end

function gui.WIBaseEditor:IsWindowActive(identifier)
	if self:IsWindowOpen(identifier) == false then
		return false
	end
	return self:GetWindow(identifier):IsVisible()
end
function gui.WIBaseEditor:GetOpenWindows()
	return self.m_windowToFrame
end
function gui.WIBaseEditor:GetWindowFrame(identifier)
	local data = self.m_windowFactories[identifier]
	if data == nil then
		return
	end
	return self.m_windowFrames[data.category]
end
function gui.WIBaseEditor:RegisterWindows()
	for _, windowData in ipairs(pfm.get_registered_windows()) do
		self:RegisterWindow(windowData.category, windowData.name, windowData.localizedName, function()
			return windowData.factory(self)
		end)
	end
end
function gui.WIBaseEditor:OpenWindow(identifier, goToWindow)
	if self.m_windowFactories[identifier] == nil or util.is_valid(self.m_windowFactories[identifier].element) then
		if goToWindow then
			self:GoToWindow(identifier)
		end
		if self.m_windowFactories[identifier] ~= nil then
			local frame = self.m_windowToFrame[self.m_windowFactories[identifier].element]
			return frame:FindTab(identifier), self.m_windowFactories[identifier].element, frame
		end
		return
	end
	local data = self.m_windowFactories[identifier]
	local frame = self.m_windowFrames[data.category]
	if util.is_valid(frame) == false then
		frame = self:GetFirstFrame()
	end
	if util.is_valid(frame) == false then
		return
	end
	local el = data.factory()
	if util.is_valid(el) == false then
		return
	end
	self.m_windowToFrame[el] = frame
	data.element = el
	local tab = frame:AddTab(identifier, data.title, el)
	if goToWindow then
		self:GoToWindow(identifier)
	end
	self:OnWindowOpened(identifier, el, frame, tab)
	self:CallCallbacks("OnWindowOpened", identifier, el, frame, tab)
	return tab, el, frame
end

function gui.WIBaseEditor:OnWindowOpened(identifier, el, frame, tab) end

function gui.WIBaseEditor:GetWindows()
	local windows = {}
	for identifier, data in pairs(self.m_windowFactories) do
		if util.is_valid(data.element) then
			windows[identifier] = data.element
		end
	end
	return windows
end

function gui.WIBaseEditor:GetWindow(identifier)
	return self.m_windowFactories[identifier] ~= nil and self.m_windowFactories[identifier].element or nil
end

function gui.WIBaseEditor:DetachWindow(identifier)
	local elWindow, frame, tab = self:GetWindowElements(identifier)
	if util.is_valid(frame) == false then
		return false
	end
	frame:DetachTab(identifier)
	return true
end

function gui.WIBaseEditor:AttachWindow(identifier)
	local elWindow, frame, tab = self:GetWindowElements(identifier)
	if util.is_valid(frame) == false then
		return false
	end
	frame:AttachTab(identifier)
	return true
end

function gui.WIBaseEditor:IsWindowDetached(identifier)
	local elWindow, frame, tab = self:GetWindowElements(identifier)
	if util.is_valid(frame) == false then
		return false
	end
	return frame:IsTabDetached(identifier)
end

function gui.WIBaseEditor:GetWindowElements(identifier)
	if self.m_windowFactories[identifier] == nil then
		return
	end
	local data = self.m_windowFactories[identifier]
	local elWindow = data.element
	local frame = self.m_windowFrames[data.category]
	local tab = util.is_valid(frame) and frame:FindTab(identifier) or nil
	return elWindow, frame, tab
end

function gui.WIBaseEditor:ClearLayout()
	if util.is_valid(self.m_contents) then
		self.m_contents:Remove()
	end
	self.m_frames = {}
end

function gui.WIBaseEditor:AddFrame(parent)
	parent = parent or self.m_contents
	if util.is_valid(parent) == false then
		return
	end
	local frame = gui.create("WIPFMFrame", parent)
	if frame == nil then
		return
	end
	frame:AddCallback("PopulateWindowMenu", function(frame, pContext)
		local frameCategories = {}
		for cat, frameOther in pairs(self.m_windowFrames) do
			if frameOther:IsValid() and frameOther == frame then
				frameCategories[cat] = true
			end
		end
		local windows = {}
		for identifier, windowData in pairs(self.m_windowFactories) do
			if frameCategories[windowData.category] then
				table.insert(windows, { windowData.title, identifier })
			end
		end
		table.sort(windows, function(a, b)
			return a[1] < b[1]
		end)
		for _, wdata in ipairs(windows) do
			pContext
				:AddItem(wdata[1], function()
					self:OpenWindow(wdata[2])
					self:GoToWindow(wdata[2])
				end)
				:SetName(wdata[2])
		end
	end)

	table.insert(self.m_frames, frame)
	return frame
end

function gui.WIBaseEditor:GetMenuBar()
	return self.m_menuBar
end

function gui.WIBaseEditor:GetMenuBarContainer()
	return self.m_menuBarContainer
end

function gui.WIBaseEditor:SetBackgroundColor(col)
	self.m_pMain:SetColor(col)
end
function gui.WIBaseEditor:CreateWindow(class)
	local pFrame = gui.create("WIFrame")
	if pFrame == nil then
		return
	end
	local p = gui.create(class or "WIEditorWindow", pFrame)
	if p == nil then
		pFrame:Remove()
		return
	end
	p:SetFrame(pFrame)
	return p
end

function gui.WIBaseEditor:Close()
	prosper.wait_idle(true)
	self:Remove()
end
function gui.WIBaseEditor:Open() end
