gui.impl = gui.impl or {}
gui.close_dialog = function()
	if gui.impl.activeDialog == nil or util.is_valid(gui.impl.activeDialog) == false then
		return
	end
	gui.impl.activeDialog:RemoveSafely()
	gui.impl.activeDialog = nil
end
gui.DIALOG_RESULT_OK = 0
gui.DIALOG_RESULT_CANCELLED = 1
gui.DIALOG_RESULT_NO_SELECTION = 2
gui.create_dialog = function(fCreateElement, parent, noFrame)
	gui.close_dialog()

	local pDialog = gui.create("WIRect", parent)
	pDialog:SetColor(Color(0, 0, 0, 220))
	pDialog:SetAutoAlignToParent(true)
	pDialog:Update()
	gui.impl.activeDialog = pDialog

	if noFrame then
		local el = fCreateElement()
		el:SetParent(pDialog)
		return pDialog, nil, el
	end

	local frame = gui.create("WITransformable", pDialog)
	frame:SetDraggable(true)
	frame:SetResizable(true)
	frame:SetMinSize(Vector2i(300, 200))
	frame:SetMaxSize(Vector2i(800, 512))
	frame:SetSize(512, 256)

	frame:SetAutoAlignToParent(true)

	local pDrag = frame:GetDragArea()
	pDrag:SetHeight(31)
	pDrag:SetAutoAlignToParent(true, false)

	local el = fCreateElement()
	el:SetParent(frame)
	frame:SetSize(el:GetSize())
	el:SetAutoAlignToParent(true)

	local pMainOutline = gui.create("WIOutlinedRect", frame)
	pMainOutline:SetAutoAlignToParent(true)
	pMainOutline:AddStyleClass("outline")

	frame:CenterToParent()
	frame:TrapFocus(true)
	frame:RequestFocus()

	return pDialog, frame, el
end
