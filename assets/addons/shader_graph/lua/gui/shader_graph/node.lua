--[[
    Copyright (C) 2024 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

local Element = util.register_class("gui.GraphNode", gui.Base)
function Element:OnInitialize()
	gui.Base.OnInitialize(self)

	self:SetSize(128, 128)

	local bg = gui.create("WIRect", self, 0, 0, self:GetWidth(), self:GetHeight(), 0, 0, 1, 1)
	self.m_bg = bg

	local box = gui.create("WIVBox", self, 0, 0, self:GetWidth(), self:GetHeight())
	box:SetName("global_container")
	box:SetFixedWidth(true)
	box:AddCallback("SetSize", function()
		self:SetHeight(box:GetBottom())
	end)

	local outputControls = gui.create("WIPFMControlsMenu", box, 0, 0, box:GetWidth(), box:GetHeight())
	outputControls:SetAutoFillContentsToHeight(false)
	outputControls:SetFixedHeight(false)
	self.m_outputControls = outputControls

	local offsetControls = 0
	local inputControls = gui.create(
		"WIPFMControlsMenu",
		box,
		offsetControls,
		outputControls:GetBottom(),
		box:GetWidth() - offsetControls,
		box:GetHeight()
	)
	inputControls:SetAutoFillContentsToHeight(false)
	inputControls:SetFixedHeight(false)
	self.m_inputControls = inputControls

	self.m_inputs = {}
	self.m_outputs = {}

	outputControls:SetHeight(0)
	inputControls:SetHeight(0)
	outputControls:ResetControls()
	inputControls:ResetControls()
end
function Element:SetNode(name)
	self.m_node = name
end
function Element:GetNode()
	return self.m_node
end
function Element:AddControl(socketType, title, id, type)
	local ctrlMenu = (socketType == gui.GraphNodeSocket.SOCKET_TYPE_INPUT) and self.m_inputControls
		or self.m_outputControls
	local elCtrl
	if socketType == gui.GraphNodeSocket.SOCKET_TYPE_INPUT and type ~= nil then
		local udmType = shader.Socket.to_udm_type(type)
		local propInfo = {}
		if type == shader.Socket.TYPE_COLOR then
			propInfo.specializationType = "color"
		end
		local wrapper = ctrlMenu:AddPropertyControl(udmType, id, title, propInfo)
		wrapper:SetOnChangeValueHandler(function(val, isFinal, initialValue)
			if isFinal then
				self:CallCallbacks("OnSocketValueChanged", id, val)
			end
		end)
		elCtrl = wrapper:GetWrapperElement()
	else
		local el, wrapper = ctrlMenu:AddText(title, id, "")
		elCtrl = wrapper
	end
	local el = gui.create("WIGraphNodeSocket", elCtrl)
	el:SetSocket(self, id, socketType)
	el:SetMouseInputEnabled(true)
	el:AddCallback("OnMouseEvent", function(el, button, state, mods)
		if button == input.MOUSE_BUTTON_LEFT and state == input.STATE_PRESS then
			self:CallCallbacks("OnSocketClicked", el, socketType, id)
			return util.EVENT_REPLY_HANDLED
		end
	end)
	local t = (socketType == gui.GraphNodeSocket.SOCKET_TYPE_INPUT) and self.m_inputs or self.m_outputs
	t[id] = {
		socketElement = el,
		controlElement = elCtrl,
	}
	return el, elCtrl
end
function Element:GetSocket(socketType, name)
	local t = (socketType == gui.GraphNodeSocket.SOCKET_TYPE_INPUT) and self.m_inputs or self.m_outputs
	if t[name] == nil then
		return
	end
	return t[name].socketElement
end
function Element:GetInputSocket(name)
	return self:GetSocket(gui.GraphNodeSocket.SOCKET_TYPE_INPUT, name)
end
function Element:GetOutputSocket(name)
	return self:GetSocket(gui.GraphNodeSocket.SOCKET_TYPE_OUTPUT, name)
end
function Element:AddInput(name, type)
	local elSocket, elCtrl = self:AddControl(gui.GraphNodeSocket.SOCKET_TYPE_INPUT, name, name, type)
	elSocket:SetX(elSocket:GetWidth() * -0.5)
	elSocket:SetY(elCtrl:GetHeight() * 0.5 - elSocket:GetHeight() * 0.5)
	return elSocket
end
function Element:AddOutput(name)
	local elSocket, elCtrl = self:AddControl(gui.GraphNodeSocket.SOCKET_TYPE_OUTPUT, name, name)
	elSocket:SetX(elCtrl:GetWidth() - elSocket:GetWidth() * 0.5)
	elSocket:SetY(elCtrl:GetHeight() * 0.5 - elSocket:GetHeight() * 0.5)
	elSocket:SetAnchor(1, 0, 1, 0)
	return elSocket
end
gui.register("WIGraphNode", Element)
