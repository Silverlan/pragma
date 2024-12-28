--[[
    Copyright (C) 2024 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

local Element = util.register_class("gui.GraphNode", gui.Base)
function Element:OnInitialize()
	gui.Base.OnInitialize(self)

	self:SetSize(160, 128)

	local box = gui.create("WIVBox", self, 0, 0, self:GetWidth(), self:GetHeight())
	box:SetName("global_container")
	box:SetFixedWidth(true)
	box:AddCallback("SetSize", function()
		self:SetHeight(box:GetBottom())
	end)

	local outputControls = gui.create("WIPFMControlsMenu", box, 0, 0, box:GetWidth(), box:GetHeight())
	outputControls:SetAutoFillContentsToHeight(false)
	outputControls:SetFixedHeight(false)
	outputControls:Wrap("WIContentWrapper"):SetPaddingLeftRight(10)
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
	inputControls:Wrap("WIContentWrapper"):SetPaddingLeftRight(10)
	inputControls:SetAutoFillContentsToHeight(false)
	inputControls:SetFixedHeight(false)
	self.m_inputControls = inputControls

	local bottomPadding = gui.create("WIBase", box, 0, 0, 1, 10)

	self.m_inputs = {}
	self.m_outputs = {}

	outputControls:SetHeight(0)
	inputControls:SetHeight(0)
end
function Element:OnRemove()
	util.remove(self.m_framePosChangedCallback)
	util.remove(self.m_frameSizeChangedCallback)

	local function removeSocketElements(sockets)
		for id, elData in pairs(sockets) do
			if util.is_valid(elData.socketElement) then
				elData.socketElement:Remove()
			end
		end
	end
	removeSocketElements(self.m_inputs)
	removeSocketElements(self.m_outputs)
end
function Element:SetSelected(selected)
	self.m_selected = selected
	if selected then
		self.m_backgroundElement:AddStyleClass("node_background_selected")
	else
		self.m_backgroundElement:RemoveStyleClass("node_background_selected")
	end
	self.m_backgroundElement:RefreshSkin()
end
function Element:IsSelected()
	return self.m_selected or false
end
function Element:SetNode(name)
	self.m_node = name
end
function Element:GetNode()
	return self.m_node
end
function Element:SetFrame(frame, elBg)
	util.remove(self.m_framePosChangedCallback)
	util.remove(self.m_frameSizeChangedCallback)

	self.m_frame = frame
	self.m_backgroundElement = elBg
	self.m_framePosChangedCallback = frame:AddCallback("SetPos", function()
		self:UpdateSocketElementPositions()
	end)
	self.m_frameSizeChangedCallback = frame:AddCallback("SetSize", function()
		self:UpdateSocketElementPositions()
	end)
end
function Element:GetFrame()
	return self.m_frame
end
function Element:SetShaderGraph(graph)
	self.m_graph = graph
end
function Element:GetShaderGraph()
	return self.m_graph
end
function Element:AddControl(socketType, linkable, title, id, type, defaultVal, minVal, maxVal, enumValues)
	local ctrlMenu = (socketType == gui.GraphNodeSocket.SOCKET_TYPE_INPUT) and self.m_inputControls
		or self.m_outputControls
	local elCtrl
	local elWrapper
	if (socketType == gui.GraphNodeSocket.SOCKET_TYPE_INPUT) and type ~= nil then
		local udmType = shader.Socket.to_udm_type(type)
		local propInfo = {
			defaultValue = defaultVal,
			enumValues = enumValues,
			minValue = minVal,
			maxValue = maxVal,
		}
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
		elWrapper = wrapper
	else
		local el, wrapper = ctrlMenu:AddInfo(title, id)
		elCtrl = wrapper
		elWrapper = wrapper
	end
	local shaderGraph = self:GetShaderGraph()
	if util.is_valid(shaderGraph) == false then
		return
	end
	local el
	if linkable then
		el = gui.create("WIGraphNodeSocket", shaderGraph)
		el:SetSocket(self, id, socketType)
		el:SetMouseInputEnabled(true)
		el:SetZPos(2)
		el:AddCallback("OnMouseEvent", function(el, button, state, mods)
			if button == input.MOUSE_BUTTON_LEFT and state == input.STATE_PRESS then
				local graph = self:GetShaderGraph()
				if util.is_valid(graph) then
					graph:StartInteractiveLinkMode(el)
				end

				return util.EVENT_REPLY_HANDLED
			end
		end)
	end
	local t = (socketType == gui.GraphNodeSocket.SOCKET_TYPE_INPUT) and self.m_inputs or self.m_outputs
	t[id] = {
		socketElement = el,
		controlElement = elCtrl,
	}
	self:UpdateSocketPosition(t[id], socketType == gui.GraphNodeSocket.SOCKET_TYPE_OUTPUT)
	return el, elWrapper
end
function Element:UpdateSocketPosition(elData, output)
	local frame = self:GetFrame()
	local graph = self:GetShaderGraph()
	if util.is_valid(frame) == false or util.is_valid(graph) == false then
		return
	end
	local graphPos = graph:GetAbsolutePos()
	local elSocket = elData.socketElement
	local elCtrl = elData.controlElement
	if util.is_valid(elSocket) and elCtrl:IsValid() then
		local pos = elCtrl:GetAbsolutePos() - graphPos
		if output then
			elSocket:SetX(pos.x + elCtrl:GetWidth() - elSocket:GetWidth() * 0.5 + 7)
		else
			elSocket:SetX(pos.x - elSocket:GetWidth() * 0.5 - 7)
		end
		elSocket:SetY(pos.y + elCtrl:GetHeight() * 0.5 - elSocket:GetHeight() * 0.5)
	end
end
function Element:UpdateSocketElementPositions()
	local frame = self:GetFrame()
	local graph = self:GetShaderGraph()
	if util.is_valid(frame) == false or util.is_valid(graph) == false then
		return
	end
	local function updatePositions(sockets, output)
		for id, elData in pairs(sockets) do
			self:UpdateSocketPosition(elData, output)
		end
	end
	updatePositions(self.m_inputs, false)
	updatePositions(self.m_outputs, true)
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
function Element:ResetControls()
	self.m_outputControls:ResetControls()
	self.m_inputControls:ResetControls()
end
function Element:AddInput(name, type, linkable, defaultVal, minVal, maxVal, enumValues)
	local elSocket, elCtrl = self:AddControl(
		gui.GraphNodeSocket.SOCKET_TYPE_INPUT,
		linkable,
		name,
		name,
		type,
		defaultVal,
		minVal,
		maxVal,
		enumValues
	)
	return elSocket, elCtrl
end
function Element:AddOutput(name)
	local elSocket, elCtrl = self:AddControl(gui.GraphNodeSocket.SOCKET_TYPE_OUTPUT, true, name, name)
	return elSocket, elCtrl
end
gui.register("WIGraphNode", Element)
