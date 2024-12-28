--[[
    Copyright (C) 2024 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

local Element = util.register_class("gui.GraphNodeSocket", gui.Base)
Element.SOCKET_TYPE_INPUT = 0
Element.SOCKET_TYPE_OUTPUT = 1
function Element:OnInitialize()
	gui.Base.OnInitialize(self)

	self:SetSize(14, 14)

	local bg = gui.create("WITexturedRect", self, 0, 0, self:GetWidth(), self:GetHeight(), 0, 0, 1, 1)
	bg:SetMaterial("gui/pfm/circle_filled")
	bg:AddStyleClass("background2")
	self.m_bg = bg

	local inner = gui.create("WITexturedRect", self, 0, 0, self:GetWidth() - 4, self:GetHeight() - 4)
	inner:SetMaterial("gui/pfm/circle_filled")
	inner:CenterToParent()
	inner:SetColor(Color.Lime)
	inner:SetVisible(false)
	self.m_innerCircle = inner
end
function Element:SetLinked(linked)
	self.m_innerCircle:SetVisible(linked)
end
function Element:SetSocket(node, socket, socketType)
	self.m_node = node
	self.m_socket = socket
	self.m_socketType = socketType
end
function Element:GetNode()
	return self.m_node
end
function Element:GetSocket()
	return self.m_socket
end
function Element:GetSocketType()
	return self.m_socketType
end
gui.register("WIGraphNodeSocket", Element)
