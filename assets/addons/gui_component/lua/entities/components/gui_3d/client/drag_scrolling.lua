--[[
    Copyright (C) 2023 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

include("/gui/pfm/cursor_tracker.lua")

local Component = ents.GUI3D

function Component:UpdateDragScrolling()
	if self.m_dragScrollingCursorTracker ~= nil then
		local newPos = self.m_dragGetCursorPos()
		if newPos == nil then
			return
		end
		self.m_dragScrollingCursorTracker:Update(newPos)
		if self.m_dragScrollingCursorTracker:HasExceededMoveThreshold(60, math.AXIS_Y) then
			self:StartDragScrolling()
		elseif self.m_dragScrollingCursorTracker:HasExceededMoveThreshold(60, math.AXIS_X) then
			-- If the user has dragged the mouse horizontally, we'll assume they want an UI input.
			-- We need to do this, otherwise interacting with sliders would not be possible.
			-- This isn't ideal, but other solutions would require changes to the GUI system.
			local bt = self.m_dragScrollingButton
			local pos = self.m_dragGetCursorPos()
			self:StopDragScrolling()
			self:DoInjectMouseInput(bt, input.STATE_PRESS, pos)
		end
	end
	if self.m_lastDragCursorPos ~= nil then
		local cursorPos = self.m_dragGetCursorPos()
		if cursorPos ~= nil then
			local dtPos = cursorPos - self.m_lastDragCursorPos
			local dtPosClamped = Vector2(math.round(dtPos.x), math.round(dtPos.y))
			if (dtPosClamped.x ~= 0 or dtPosClamped.y ~= 0) and util.is_valid(self.m_pGui) then
				self.m_pGui:InjectScrollInput(cursorPos, dtPos, true)
				self.m_lastDragCursorPos = cursorPos + (dtPos - dtPosClamped)
			end
		end
	end
end
function Component:SetDragScrollingEnabled(enabled)
	self.m_dragScrollingEnabled = enabled
end
function Component:IsDragScrollingEnabled()
	return self.m_dragScrollingEnabled or false
end
function Component:IsDragScrollingActive()
	return self.m_lastDragCursorPos ~= nil
end
function Component:HandleDragScrollingMouseInput(bt, state, fGetCursorPos)
	if fGetCursorPos == nil or bt ~= input.MOUSE_BUTTON_LEFT then
		return util.EVENT_REPLY_UNHANDLED
	end
	if state == input.STATE_RELEASE and self:IsDragScrollingActive() then
		self:StopDragScrolling()
		return util.EVENT_REPLY_HANDLED
	end
	if state == input.STATE_PRESS and self:IsDragScrollingEnabled() then
		return self:InitDragScrollingMode(bt, fGetCursorPos) and util.EVENT_REPLY_HANDLED or util.EVENT_REPLY_UNHANDLED
	end
	if state == input.STATE_RELEASE and self.m_dragScrollingCursorTracker ~= nil then
		self.m_dragScrollingCursorTracker = nil
		-- We ended up not dragging; inject the press input instead
		self:DoInjectMouseInput(bt, input.STATE_PRESS, fGetCursorPos())
	end
	return util.EVENT_REPLY_UNHANDLED
end
function Component:InitDragScrollingMode(bt, fGetCursorPos)
	local pos = fGetCursorPos()
	if pos == nil then
		return false
	end
	self.m_dragScrollingButton = bt
	self.m_dragScrollingCursorTracker = gui.CursorTracker(pos)
	self.m_dragGetCursorPos = fGetCursorPos
	return true
end
function Component:StartDragScrolling()
	local fDragGetCursorPos = self.m_dragGetCursorPos
	self:StopDragScrolling()
	if fDragGetCursorPos == nil then
		return
	end
	self.m_dragGetCursorPos = fDragGetCursorPos
	self.m_lastDragCursorPos = fDragGetCursorPos()
end
function Component:StopDragScrolling()
	self.m_dragScrollingButton = nil
	self.m_lastDragCursorPos = nil
	self.m_dragScrollingCursorTracker = nil
	self.m_dragGetCursorPos = nil
end
