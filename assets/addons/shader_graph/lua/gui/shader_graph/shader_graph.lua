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

	self.m_nameToElementData = {}
	self.m_nodeData = {}
	self.m_linkElements = {}
	self:Clear()
	self:SetSize(1280, 1024)

	self:SetMouseInputEnabled(true)
end
function Element:MouseCallback(button, state, mods)
	if button == input.MOUSE_BUTTON_RIGHT and state == input.STATE_PRESS then
		local pContext = gui.open_context_menu(self)
		if util.is_valid(pContext) then
			pContext:AddItem("Load Graph", function()
				local pFileDialog = pfm.create_file_open_dialog(function(el, fileName)
					if fileName == nil then
						return
					end
					fileName = el:GetFilePath(false)

					local udmData, err = udm.load(fileName)
					if udmData == false then
						self:LogWarn("Failed to load shader graph file '" .. fileName .. "': " .. err)
						return
					end

					local reg = shader.get_test_node_register() -- TODO
					local graph, err = shader.ShaderGraph.load(udmData:GetAssetData(), reg)
					if graph == false then
						self:LogWarn("Failed to load shader graph '" .. fileName .. "': " .. err)
						return
					end
					self:LogInfo("Loaded shader graph '" .. fileName .. "'!")
					self:SetGraph(graph)
				end)
				pFileDialog:SetRootPath("scripts/shader_data/graphs")
				pFileDialog:SetExtensions({ shader.ShaderGraph.EXTENSION_ASCII, shader.ShaderGraph.EXTENSION_BINARY })
				pFileDialog:Update()
			end)
			pContext:AddItem("Save Graph", function()
				local pFileDialog = pfm.create_file_save_dialog(function(el, fileName)
					if fileName == nil then
						return
					end

					fileName = el:GetFilePath(false)
					fileName = file.remove_file_extension(
						fileName,
						{ shader.ShaderGraph.EXTENSION_ASCII, shader.ShaderGraph.EXTENSION_BINARY }
					)
					fileName = fileName .. "." .. shader.ShaderGraph.EXTENSION_ASCII
					local udmData = udm.create(shader.ShaderGraph.PSG_IDENTIFIER, shader.ShaderGraph.PSG_VERSION)
					local res, err = self.m_graph:Save(udmData:GetAssetData())
					if res == false then
						self:LogWarn("Failed to save shader graph: " .. err)
						return
					end
					file.create_path(file.get_file_path(fileName))
					res, err = udmData:SaveAscii(fileName)
					if res == false then
						self:LogWarn("Failed to save shader graph as '" .. fileName .. "': " .. err)
						return
					end
					self:LogInfo("Saved shader graph as '" .. fileName .. "'!")
				end)
				pFileDialog:SetRootPath("scripts/shader_data/graphs")
				pFileDialog:Update()
			end)
			pContext:AddItem("Generate GLSL", function()
				util.set_clipboard_string(self.m_graph:GenerateGlsl())
			end)
			local reg = shader.get_graph_node_registry("object")
			if reg ~= nil then
				local nodeTypes = reg:GetNodeTypes()
				table.sort(nodeTypes)
				local pItem, pSubMenu = pContext:AddSubMenu("Add Node")
				for _, name in pairs(nodeTypes) do
					pSubMenu:AddItem(name, function(pItem)
						local graphNode = self.m_graph:AddNode(name)
						if graphNode ~= nil then
							local frame = self:AddNode(graphNode)
							local pos = self:GetCursorPos()
							frame:SetPos(pos.x - frame:GetWidth() * 0.5, pos.y - frame:GetHeight() * 0.5)
							self:InitializeLinks()
						end
					end)
				end
				pSubMenu:Update()
			end
			pContext:Update()
			return util.EVENT_REPLY_HANDLED
		end
	end
end
function Element:GetGraph()
	return self.m_graph
end
function Element:Clear()
	self:ClearLinks()
	for _, t in ipairs(self.m_nodeData) do
		util.remove(t.frame)
	end
	self.m_nodeData = {}
	self.m_nameToElementData = {}
	self.m_graph = shader.create_graph("object")
end
function Element:SetGraph(graph)
	self:Clear()

	self.m_graph = graph

	local nodes = self.m_graph:GetNodes()
	local offset = 0
	for _, graphNode in ipairs(nodes) do
		local frame = self:AddNode(graphNode)
		frame:SetX(offset)
		offset = offset + frame:GetWidth() + 80
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
		local elOutput = self.m_nameToElementData[link.outputNode:GetName()].nodeElement
		local elInput = self.m_nameToElementData[link.inputNode:GetName()].nodeElement
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
	l:SetZPos(-1)
	l:Setup(elOutputSocket, elInputSocket)
	table.insert(self.m_linkElements, l)
end
function Element:RemoveNode(name)
	local t = self.m_nameToElementData[name]
	if t == nil then
		return
	end

	util.remove(t.frame)

	for i, tOther in ipairs(self.m_nodeData) do
		if tOther == t then
			table.remove(self.m_nodeData, i)
			break
		end
	end
	self.m_nameToElementData[name] = nil
	self:InitializeLinks()
end
function Element:AddNode(graphNode)
	local name = graphNode:GetName()
	local frame = gui.create("WIFrame", self)
	frame:SetTitle(name)
	frame:SetDetachButtonEnabled(false)
	frame:SetCloseButtonEnabled(false)
	frame:SetResizable(false)
	frame:SetSize(128, 128)
	frame:SetZPos(0)
	frame:AddCallback("OnDragStart", function(el, x, y)
		el:SetZPos(1)
	end)
	frame:AddCallback("OnDragEnd", function(el, x, y)
		el:SetZPos(0)
	end)

	frame:SetMouseInputEnabled(true)
	frame:AddCallback("OnMouseEvent", function(el, button, state, mods)
		if button == input.MOUSE_BUTTON_RIGHT and state == input.STATE_PRESS then
			local pContext = gui.open_context_menu(self)
			if util.is_valid(pContext) then
				pContext:AddItem("Remove Node", function()
					if self.m_graph:RemoveNode(name) then
						time.create_simple_timer(0.0, function()
							if self:IsValid() then
								self:RemoveNode(name)
							end
						end)
					end
				end)
				pContext:Update()
			end
			return util.EVENT_REPLY_HANDLED
		end
	end)

	local pDrag = frame:GetDragArea()
	pDrag:SetHeight(31)
	pDrag:SetAutoAlignToParent(true, false)

	local elNode = gui.create("WIGraphNode", frame)
	elNode:SetNode(graphNode:GetName())
	elNode:SetY(31)
	elNode:AddCallback("SetSize", function()
		frame:SetHeight(elNode:GetBottom())
	end)
	elNode:AddCallback("OnSocketValueChanged", function(elNode, id, val)
		local node = self.m_graph:GetNode(name)
		if node:SetInputValue(id, val) then
			self:CallCallbacks("OnNodeSocketValueChanged", name, id, val)
		end
	end)
	for _, output in ipairs(graphNode:GetOutputs()) do
		local socket = output:GetSocket()
		local elOutput = elNode:AddOutput(socket.name)
	end
	for _, input in ipairs(graphNode:GetInputs()) do
		local socket = input:GetSocket()
		local elInput = elNode:AddInput(socket.name, socket.type)
	end

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

	local t = {
		frame = frame,
		nodeElement = elNode,
		graphNode = name,
	}
	table.insert(self.m_nodeData, t)
	self.m_nameToElementData[graphNode:GetName()] = t
	return frame
end
gui.register("WIShaderGraph", Element)
