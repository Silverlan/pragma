util.register_class("gui.WIEditorWindow", gui.Base)
function gui.WIEditorWindow:__init()
	gui.Base.__init(self)
end
function gui.WIEditorWindow:OnInitialize()
	gui.Base.OnInitialize(self)

	local pTabbedPanel = gui.create("WITabbedPanel", self)
	pTabbedPanel:SetAutoAlignToParent(true)
	self.m_pTabbedPanel = pTabbedPanel
end
function gui.WIEditorWindow:SetFrame(frame)
	self.m_pFrame = frame
end
function gui.WIEditorWindow:GetFrame()
	return self.m_pFrame
end
function gui.WIEditorWindow:GetTabbedPanel()
	return self.m_pTabbedPanel
end
function gui.WIEditorWindow:AddTab(name)
	if util.is_valid(self.m_pTabbedPanel) == false then
		return
	end
	return self.m_pTabbedPanel:AddTab(name)
end
function gui.WIEditorWindow:FindTab(name)
	if util.is_valid(self.m_pTabbedPanel) == false then
		return
	end
	return self.m_pTabbedPanel:FindTab(name)
end
function gui.WIEditorWindow:Update()
	if util.is_valid(self.m_pTabbedPanel) then
		self.m_pTabbedPanel:Update()
	end
end
gui.register("WIEditorWindow", gui.WIEditorWindow)
