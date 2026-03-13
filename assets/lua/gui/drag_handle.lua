-- SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

local DragHandle = util.register_class("gui.DragHandle", gui.Base)
function DragHandle:OnInitialize()
	self.m_isDragging = false
	self:SetSize(64, 64)
	self:SetMouseInputEnabled(true)
	self:SetCursor(gui.CURSOR_SHAPE_HAND)

	self:AddCallback("OnCursorMoved", function(el, x, y)
		self:OnCursorMoved(x, y)
	end)
end
function DragHandle:SetReferenceElement(refEl)
	self.m_referenceElement = refEl
end
function DragHandle:GetReferenceElement()
	if(util.is_valid(self.m_referenceElement)) then return self.m_referenceElement end
	return self:GetParent()
end
function DragHandle:MouseCallback(mouseButton, state, mods)
	if mouseButton == input.MOUSE_BUTTON_LEFT then
		if state == input.STATE_PRESS then
			self:StartDragging()
		elseif state == input.STATE_RELEASE then
			self:StopDragging()
		end
		return util.EVENT_REPLY_HANDLED
	end
	return util.EVENT_REPLY_UNHANDLED
end
function DragHandle:IsDragging()
	return self.m_dragging
end
function DragHandle:StartDragging()
	if self:IsDragging() then
		return
	end
	self.m_dragging = true
	self.m_dragStartPos = self:GetReferenceElement():GetCursorPos()
	self.m_dragStartOffset = self:GetCursorPos()
	self:SetCursorMovementCheckEnabled(true)
	self:CallCallbacks("OnDragStart")
end
function DragHandle:StopDragging()
	if not self:IsDragging() then
		return
	end
	self.m_dragging = false
	self.m_dragStartPos = nil
	self.m_dragStartOffset = nil
	self:SetCursorMovementCheckEnabled(false)
	self:CallCallbacks("OnDragEnd")
end
function DragHandle:GetDragStartOffset() return self.m_dragStartOffset or Vector2i() end
function DragHandle:OnCursorMoved(x, y)
	local cursorPos = self:GetReferenceElement():GetCursorPos()
	local xdelta = cursorPos.x -self.m_dragStartPos.x
	local ydelta = cursorPos.y -self.m_dragStartPos.y
	self:CallCallbacks("OnDrag", xdelta, ydelta, cursorPos.x -self.m_dragStartOffset.x, cursorPos.y -self.m_dragStartOffset.y)
end
gui.register("drag_handle", DragHandle)
