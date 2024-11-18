--[[
    Copyright (C) 2024 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

include("/gui/pfm/tutorials/element_connector_line.lua")
include("node.lua")
include("node_socket.lua")

local Element = util.register_class("gui.ShaderGraph", gui.Base)
function Element:OnInitialize()
	gui.Base.OnInitialize(self)

	self.m_nameToElement = {}
	self.m_nodeElements = {}
	self.m_linkElements = {}
	self:SetSize(1280, 1024)
	self:TrapFocus()
	self:RequestFocus()
end
function Element:SetGraph(graph)
	self.m_graph = graph

	local nodes = self.m_graph:GetNodes()
	for _, graphNode in ipairs(nodes) do
		self:AddNode(graphNode)
	end

	self:InitializeLinks()
end
function Element:InitializeLinks()
	self:ClearLinks()
	local links = {}
	local nodes = self.m_graph:GetNodes()
	for _, graphNode in ipairs(nodes) do
		for _, input in ipairs(graphNode:GetInputs()) do
			local socket = input:GetSocket()
			local output = input:GetLinkedOutput()
			if output ~= nil then
				table.insert(links, {
					inputNode = graphNode,
					input = socket.name,
					outputNode = output:GetNode(),
					output = output:GetSocket().name,
				})
			end
		end
	end

	for _, link in ipairs(links) do
		local elOutput = self.m_nameToElement[link.outputNode:GetName()]
		local elInput = self.m_nameToElement[link.inputNode:GetName()]
		local elOutputSocket = elOutput:GetOutputSocket(link.output)
		local elInputSocket = elInput:GetInputSocket(link.input)
		if util.is_valid(elOutputSocket) and util.is_valid(elInputSocket) then
			self:AddLink(elOutputSocket, elInputSocket)
		end
	end
end
function Element:ClearLinks()
	util.remove(self.m_linkElements)
	self.m_linkElements = {}
end
function Element:AddLink(elOutputSocket, elInputSocket)
	local l = gui.create("WIElementConnectorLine", self)
	l:SetSize(self:GetSize())
	l:SetAnchor(0, 0, 1, 1)
	l:Setup(elOutputSocket, elInputSocket)
	table.insert(self.m_linkElements, l)
end
function Element:AddNode(graphNode)
	local node = graphNode:GetNode()
	local elNode = gui.create("WIGraphNode", self)
	elNode:SetNode(graphNode:GetName())
	elNode:SetX(#self.m_nodeElements * 200)
	for _, output in ipairs(graphNode:GetOutputs()) do
		local socket = output:GetSocket()
		local elOutput = elNode:AddOutput(socket.name)
	end
	for _, input in ipairs(graphNode:GetInputs()) do
		local socket = input:GetSocket()
		local elInput = elNode:AddInput(socket.name)
	end
	--x = elNode:GetRight() + 80

	elNode:AddCallback("OnSocketClicked", function(elNode, elSocket, socketType, id)
		if util.is_valid(self.m_outSocket) == false then
			self.m_outSocket = elSocket
		else
			local outSocket = self.m_outSocket
			local inSocket = elSocket
			local outNode = outSocket:GetNode():GetNode()
			local outSocket = outSocket:GetSocket()
			local inNode = inSocket:GetNode():GetNode()
			local inSocket = inSocket:GetSocket()
			self.m_outSocket = nil

			if outNode == inNode then
				if outSocket == inSocket then
					self.m_graph:GetNode(outNode):Disconnect(outSocket)
					self:InitializeLinks()
					return
				end
			end
			self.m_graph:GetNode(outNode):Link(outSocket, self.m_graph:GetNode(inNode), inSocket)
			self:InitializeLinks()
		end
	end)

	table.insert(self.m_nodeElements, elNode)
	self.m_nameToElement[graphNode:GetName()] = elNode
end
gui.register("WIShaderGraph", Element)
