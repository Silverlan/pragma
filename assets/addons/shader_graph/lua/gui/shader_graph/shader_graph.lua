--[[
    Copyright (C) 2024 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

include("/gui/pfm/tutorials/element_connector_line.lua")
include("node.lua")
include("node_socket.lua")

locale.load("sg_categories.txt")
locale.load("sg_nodes.txt")

local Element = util.register_class("gui.ShaderGraph", gui.Base)
function Element:OnInitialize()
	gui.Base.OnInitialize(self)

	self.m_nameToElementData = {}
	self.m_frameToNodeData = {}
	self.m_nodeData = {}
	self.m_linkElements = {}
	self:Clear()
	self:SetSize(10000, 10000)

	self:SetMouseInputEnabled(true)
end
function Element:SetShaderEditor(editor)
	self.m_shaderEditor = editor
end
function Element:MouseCallback(button, state, mods)
	if button == input.MOUSE_BUTTON_LEFT then
		if state == input.STATE_PRESS then
			self.m_selectionRect = gui.create("WISelectionRect", self)
			self.m_selectionRect:SetPos(self:GetCursorPos())
			self.m_selectionRect:SetZPos(10000)
		else
			local gnFrames = self.m_selectionRect:FindElements(function(el)
				return el:GetClass() == "wiframe"
			end)
			self:DeselectAll()
			for _, frame in ipairs(gnFrames) do
				local elementData = self.m_frameToNodeData[frame]
				if elementData ~= nil then
					elementData.nodeElement:SetSelected(true)
				end
			end
			util.remove(self.m_selectionRect)
		end
		return util.EVENT_REPLY_HANDLED
	end

	if button == input.MOUSE_BUTTON_MIDDLE then
		if state == input.STATE_PRESS then
			self.m_shaderEditor:GetTransformableElement():StartDrag()
		else
			self.m_shaderEditor:GetTransformableElement():EndDrag()
		end
		return util.EVENT_REPLY_HANDLED
	end

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
				local catNodes = {}
				local catList = {}
				for _, name in pairs(nodeTypes) do
					local node = reg:GetNode(name)
					if node ~= nil then
						local cat = node:GetCategory()
						if catNodes[cat] == nil then
							catNodes[cat] = {}
							table.insert(catList, {
								category = cat,
								name = locale.get_text("sg_cat_" .. cat),
							})
						end
						table.insert(catNodes[cat], name)
					end
				end
				table.sort(catList, function(a, b)
					return a.name < b.name
				end)
				for _, t in ipairs(catList) do
					local nodeTypes = catNodes[t.category]
					local pItem, pSubMenuCat = pSubMenu:AddSubMenu(t.name)
					for _, name in pairs(nodeTypes) do
						local locName = locale.get_text("sg_node_" .. name)
						pSubMenuCat:AddItem(locName, function(pItem)
							local graphNode = self.m_graph:AddNode(name)
							if graphNode ~= nil then
								local frame = self:AddNode(graphNode)
								local pos = self:GetCursorPos()
								frame:SetPos(pos.x - frame:GetWidth() * 0.5, pos.y - frame:GetHeight() * 0.5)
								self:InitializeLinks()
							end
						end)
					end
					pSubMenuCat:Update()
				end
				pSubMenu:Update()
			end
			pContext:Update()
			return util.EVENT_REPLY_HANDLED
		end
	end
end
function Element:GetNodeData()
	return self.m_nodeData
end
function Element:GetNodeElements()
	local t = {}
	for _, v in ipairs(self.m_nodeData) do
		table.insert(t, v.nodeElement)
	end
	return t
end
function Element:DeselectAll()
	for _, t in ipairs(self.m_nodeData) do
		t.nodeElement:SetSelected(false)
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
	self.m_frameToNodeData = {}
	self.m_graph = shader.create_graph("object")
end
function Element:SetGraph(graph)
	self:Clear()

	self.m_graph = graph

	local nodes = self.m_graph:GetNodes()
	for _, graphNode in ipairs(nodes) do
		local pos = graphNode:GetPos()
		local frame = self:AddNode(graphNode)
		pos.x = pos.x + self:GetHalfWidth()
		pos.y = pos.y + self:GetHalfHeight()
		frame:SetPos(pos)
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
	self:CallCallbacks("OnLinksChanged")
end
function Element:ClearLinks()
	util.remove(self.m_linkElements)
	self.m_linkElements = {}
	self:CallCallbacks("OnLinksChanged")
end
function Element:AddLink(elOutputSocket, elInputSocket)
	local l = gui.create("WIElementConnectorLine", self)
	l:SetPointToVisibleBounds(false)
	l:SetArrowScale(0.5)
	l:SetSize(self:GetSize())
	l:SetAnchor(0, 0, 1, 1)
	l:SetZPos(-1)
	l:Setup(elOutputSocket, elInputSocket)
	l:AddCallback("OnRemove", function()
		if elInputSocket:IsValid() then
			elInputSocket:SetLinked(false)
		end
		if elOutputSocket:IsValid() then
			elOutputSocket:SetLinked(false)
		end
	end)
	table.insert(self.m_linkElements, l)

	elInputSocket:SetLinked(true)
	elOutputSocket:SetLinked(true)
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
	for k, v in pairs(self.m_frameToNodeData) do
		if v == t then
			self.m_frameToNodeData[k] = nil
			break
		end
	end
	self:InitializeLinks()
end
function Element:AddNode(graphNode)
	local name = graphNode:GetName()
	local frame = gui.create("WIFrame", self)
	frame:SetTitle(name)
	frame:SetDetachButtonEnabled(false)
	frame:SetCloseButtonEnabled(false)
	frame:SetResizable(false)
	frame:SetSize(160, 128)
	frame:SetZPos(0)
	util.remove(frame:FindDescendantByName("background"))

	local elBg = gui.create("WIRoundedRect", frame)
	elBg:SetZPos(-100)
	elBg:SetBackgroundElement(true)
	elBg:SetAutoAlignToParent(true)
	elBg:AddStyleClass("background")

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
	elNode:SetShaderGraph(self)
	elNode:SetFrame(frame, elBg)
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
	local socketValues = {}
	for _, input in ipairs(graphNode:GetInputs()) do
		local socket = input:GetSocket()
		local enumSet = socket.enumSet
		local enumValues
		if enumSet ~= nil then
			enumValues = {}
			for k, v in pairs(enumSet:GetValueToName()) do
				table.insert(enumValues, { tostring(k), v })
			end
		end
		local elInput, elWrapper = elNode:AddInput(
			socket.name,
			socket.type,
			socket:IsLinkable(),
			socket.defaultValue,
			socket.min,
			socket.max,
			enumValues
		)

		table.insert(socketValues, {
			elWrapper = elWrapper,
			value = graphNode:GetInputValue(socket.name),
		})
	end
	elNode:ResetControls()

	for _, val in ipairs(socketValues) do
		print(val.elWrapper)
		val.elWrapper:SetValue(val.value)
	end

	local t = {
		frame = frame,
		nodeElement = elNode,
		graphNode = graphNode,
	}
	self.m_frameToNodeData[frame] = t
	table.insert(self.m_nodeData, t)
	self.m_nameToElementData[graphNode:GetName()] = t
	return frame
end
function Element:StopInteractiveLinkMode(applyLink)
	util.remove(self.m_interactiveLinkTargetElement)
	util.remove(self.m_interactiveLinkLine)
	util.remove(self.m_cbOnMouseRelease)
	self:SetThinkingEnabled(false)

	if applyLink and util.is_valid(self.m_interactiveLinkSourceSocket) then
		local outSocket = self.m_interactiveLinkSourceSocket
		local inSocket = self.m_interactiveLinkTargetSocket
		if util.is_valid(inSocket) == false then
			inSocket = outSocket
		end
		if outSocket:GetSocketType() == gui.GraphNodeSocket.SOCKET_TYPE_INPUT then
			local tmp = outSocket
			outSocket = inSocket
			inSocket = tmp
		end
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
	self.m_interactiveLinkTargetSocket = nil
end
function Element:StartInteractiveLinkMode(el)
	self:StopInteractiveLinkMode()
	self.m_interactiveLinkSourceSocket = el

	local elTgt = gui.create("WIBase", self)
	elTgt:SetPos(self:GetCursorPos())
	self.m_interactiveLinkTargetElement = elTgt
	self.m_cbOnMouseRelease = input.add_callback("OnMouseInput", function(mouseButton, state, mods)
		if mouseButton == input.MOUSE_BUTTON_LEFT and state == input.STATE_RELEASE then
			self:StopInteractiveLinkMode(true)
		end
	end)

	local l = gui.create("WIElementConnectorLine", self)
	l:SetPointToVisibleBounds(false)
	l:SetArrowScale(0.5)
	l:SetSize(self:GetSize())
	l:SetAnchor(0, 0, 1, 1)
	l:SetZPos(-1)
	if el:GetSocketType() == gui.GraphNodeSocket.SOCKET_TYPE_INPUT then
		l:Setup(elTgt, el)
	else
		l:Setup(el, elTgt)
	end
	self.m_interactiveLinkLine = l

	self:SetThinkingEnabled(true)
end
function Element:OnRemove()
	self:StopInteractiveLinkMode()
end
function Element:OnThink()
	local cursorPos = self:GetCursorPos()
	if
		util.is_valid(self.m_interactiveLinkTargetElement)
		and util.is_valid(self.m_interactiveLinkLine)
		and util.is_valid(self.m_interactiveLinkSourceSocket)
	then
		local elSocket = gui.get_element_under_cursor(function(el)
			return el:GetClass() == "wigraphnodesocket"
				and el:GetSocketType() ~= self.m_interactiveLinkSourceSocket:GetSocketType()
		end)
		self.m_interactiveLinkTargetSocket = elSocket
		if util.is_valid(elSocket) then
			local pos = elSocket:GetAbsolutePos() - self:GetAbsolutePos() + Vector2(0, elSocket:GetHeight() * 0.5)
			if elSocket:GetSocketType() == gui.GraphNodeSocket.SOCKET_TYPE_OUTPUT then
				pos.x = pos.x + elSocket:GetWidth()
			end
			self.m_interactiveLinkTargetElement:SetPos(pos)
		else
			self.m_interactiveLinkTargetElement:SetPos(cursorPos)
		end

		self.m_interactiveLinkLine:Update()
	end
end
gui.register("WIShaderGraph", Element)
