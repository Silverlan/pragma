--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

util.register_class("gui.DebugRenderStats", gui.Base)
function gui.DebugRenderStats:__init()
	tool.load_filmmaker_scripts()
	gui.Base.__init(self)
end
function gui.DebugRenderStats:OnRemove()
	game.set_render_stats_enabled(false)

	util.remove(self.m_cb)
	util.remove(self.m_cbRenderStatsAvailable)
	util.remove(self.m_cbActionInput)
end
function gui.DebugRenderStats:OnInitialize()
	gui.Base.OnInitialize(self)

	self:SetSize(512, 400)

	game.set_render_stats_enabled(true)
	self.m_cb = game.add_callback("PostRenderScenes", function()
		for _, sceneInfo in ipairs(game.get_queued_render_scenes()) do
			local renderStats = sceneInfo.renderStats
			if renderStats ~= nil then
				self:UpdateStats(renderStats)
			end
		end
	end)
	self.m_cbRenderStatsAvailable = game.add_callback("OnFrameRenderStatsAvailable", function(t)
		self.m_curStats = {}
		self.m_sceneNameToStats = {}
		self.m_engineGpuRenderTimes = t.times
		self.m_numScenes = t.numberOfScenes
		for i, stats in ipairs(t.stats) do
			self.m_curStats[i] = {
				scene = stats.scene,
				stats = stats.stats:Copy(),
			}
			self.m_sceneNameToStats[stats.scene] = self.m_curStats[i]
		end
	end)

	self.m_sliders = {}
	self.m_curStats = {}
	self.m_engineGpuRenderTimes = {}
	self.m_sceneNameToStats = {}
	self.m_scenes = {}

	self:InitializeUiElements()

	self.m_updateRate = 0.25
	self:SetThinkingEnabled(true)

	self.m_tNextUpdate = time.real_time()

	local pl = ents.get_local_player()
	local actionInputC = pl:GetEntity():GetComponent(ents.COMPONENT_ACTION_INPUT_CONTROLLER)
	if actionInputC ~= nil then
		self.m_cbActionInput = actionInputC:AddEventCallback(
			ents.ActionInputControllerComponent.EVENT_HANDLE_ACTION_INPUT,
			function(action, bPressed, magnitude)
				if action == input.ACTION_ATTACK2 and bPressed then
					if self:HasFocus() then
						self:TrapFocus(false)
						self:KillFocus()

						local windowSize = gui.get_window_size()
						input.set_cursor_pos(Vector2(windowSize.x * 0.5, windowSize.y * 0.5)) -- Reset mouse cursor to center of screen
					else
						self:TrapFocus()
						self:RequestFocus()
					end
					return util.EVENT_REPLY_HANDLED
				end
				if action == input.ACTION_JUMP and bPressed then
					self:SetLocked(not self:IsLocked())
					return util.EVENT_REPLY_HANDLED
				end
			end
		)
	end

	self:AddSlider({ self.m_tree:GetRoot(), self.m_data }, "Scenes:", function(stats)
		return self.m_numScenes
	end)
	-- self:AddSlider({self.m_tree:GetRoot(),self.m_data},"FPS:",function(stats) return end)
	self:AddSlider({ self.m_tree:GetRoot(), self.m_data }, "[GPU] Frame Execution Time:", function(stats)
		return self.m_engineGpuRenderTimes["frame"] or 0
	end, true)
	self:AddSlider({ self.m_tree:GetRoot(), self.m_data }, "[GPU] GUI Render Time:", function(stats)
		return self.m_engineGpuRenderTimes["gui"] or 0
	end, true)
	self:AddSlider({ self.m_tree:GetRoot(), self.m_data }, "[GPU] Scene Render Time:", function(stats)
		return self.m_engineGpuRenderTimes["scene"] or 0
	end, true)
	self:AddSlider({ self.m_tree:GetRoot(), self.m_data }, "[GPU] Present Time:", function(stats)
		return self.m_engineGpuRenderTimes["present"] or 0
	end, true)
	if openvr ~= nil then
		self:AddSlider({ self.m_tree:GetRoot(), self.m_data }, "[VR] Pose Wait Time:", function(stats)
			return openvr.get_smoothed_pose_wait_time()
		end, true)
	end
	-- self:AddSlider({self.m_tree:GetRoot(),self.m_data},"Total CPU Execution Time:",function(stats) return end,true)
end
function gui.DebugRenderStats:SetLocked(locked)
	self.m_locked = locked
end
function gui.DebugRenderStats:IsLocked()
	return self.m_locked or false
end
function gui.DebugRenderStats:InitializeUiElements()
	self:SetSize(64, 128)

	self.m_bg = gui.create("WIRect", self, 0, 0, self:GetWidth(), self:GetHeight(), 0, 0, 1, 1)
	self.m_bg:SetColor(Color(54, 54, 54))

	self.m_contents = gui.create("WIHBox", self, 0, 0, self:GetWidth(), self:GetHeight(), 0, 0, 1, 1)
	self.m_contents:SetAutoFillContents(true)

	local treeVBox = gui.create("WIVBox", self.m_contents)
	treeVBox:SetAutoFillContents(true)
	local resizer = gui.create("WIResizer", self.m_contents)
	local dataVBox = gui.create("WIVBox", self.m_contents)
	dataVBox:SetAutoFillContents(true)

	local function create_header_text(text, parent)
		local pHeader = gui.create("WIRect", parent, 0, 0, parent:GetWidth(), 21, 0, 0, 1, 0)
		pHeader:SetColor(Color(35, 35, 35))
		local pHeaderText = gui.create("WIText", pHeader)
		pHeaderText:SetColor(Color(152, 152, 152))
		pHeaderText:SetFont("pfm_medium")
		pHeaderText:SetText(text)
		pHeaderText:SizeToContents()
		pHeader:AddCallback("SetSize", function()
			if pHeaderText:IsValid() == false then
				return
			end
			pHeaderText:SetPos(
				pHeader:GetWidth() * 0.5 - pHeaderText:GetWidth() * 0.5,
				pHeader:GetHeight() * 0.5 - pHeaderText:GetHeight() * 0.5
			)
		end)
	end
	create_header_text(locale.get_text("tree"), treeVBox)
	create_header_text(locale.get_text("data"), dataVBox)

	-- Tree
	local treeScrollContainerBg = gui.create("WIBase", treeVBox, 0, 0, 64, 128)
	local treeScrollContainer = gui.create("WIScrollContainer", treeScrollContainerBg, 0, 0, 64, 128, 0, 0, 1, 1)
	treeScrollContainerBg:AddCallback("SetSize", function(el)
		if self:IsValid() and util.is_valid(self.m_tree) then
			self.m_tree:SetWidth(el:GetWidth())
		end
	end)
	self.m_tree = gui.create(
		"WIPFMTreeView",
		treeScrollContainer,
		0,
		0,
		treeScrollContainer:GetWidth(),
		treeScrollContainer:GetHeight()
	)
	self.m_tree:SetSelectable(gui.Table.SELECTABLE_MODE_SINGLE)

	-- Data
	local dataScrollContainerBg = gui.create("WIBase", dataVBox, 0, 0, 64, 128)
	local dataScrollContainer = gui.create("WIScrollContainer", dataScrollContainerBg, 0, 0, 64, 128, 0, 0, 1, 1)
	dataScrollContainerBg:AddCallback("SetSize", function(el)
		if self:IsValid() and util.is_valid(self.m_data) then
			self.m_data:SetWidth(el:GetWidth())
		end
	end)
	self.m_data =
		gui.create("WIVBox", dataScrollContainer, 0, 0, dataScrollContainer:GetWidth(), dataScrollContainer:GetHeight())
	self.m_data:SetAutoFillContentsToWidth(true)

	local inCallback = false
	treeScrollContainer:GetVerticalScrollBar():AddCallback("OnScrollOffsetChanged", function(el, offset)
		if inCallback == true then
			return
		end
		inCallback = true
		dataScrollContainer:GetVerticalScrollBar():SetScrollOffset(offset)
		inCallback = false
	end)
	dataScrollContainer:GetVerticalScrollBar():AddCallback("OnScrollOffsetChanged", function(el, offset)
		if inCallback == true then
			return
		end
		inCallback = true
		treeScrollContainer:GetVerticalScrollBar():SetScrollOffset(offset)
		inCallback = false
	end)
end
function gui.DebugRenderStats:OnThink()
	local t = time.real_time()
	if self.m_tNextUpdate > t or self:IsLocked() then
		return
	end
	self.m_tNextUpdate = t + self.m_updateRate

	for _, sceneStats in ipairs(self.m_curStats) do
		local sceneName = sceneStats.scene
		sceneStats.cpuExecTime = 0.0
		sceneStats.gpuExecTime = 0.0
		sceneStats.drawCalls = 0
		sceneStats.numTris = 0
		for i = 0, game.RenderStats.RENDER_PASS_COUNT - 1 do
			sceneStats.cpuExecTime = sceneStats.cpuExecTime
				+ self:GetRenderPassTime(sceneName, i, game.RenderPassStats.TIMER_CPU_EXECUTION)
			sceneStats.gpuExecTime = sceneStats.gpuExecTime
				+ self:GetRenderPassTime(sceneName, i, game.RenderPassStats.TIMER_GPU_EXECUTION)
			sceneStats.drawCalls = sceneStats.drawCalls
				+ self:GetRenderCounter(sceneName, i, game.RenderPassStats.COUNTER_DRAW_CALLS)
			sceneStats.numTris = sceneStats.numTris
				+ self:GetRenderCounter(sceneName, i, game.RenderPassStats.COUNTER_DRAWN_TRIANGLES)
		end

		self:AddScene(sceneStats)
	end

	for _, sliderData in ipairs(self.m_sliders) do
		if sliderData.element:IsValid() then
			local val = sliderData.statsFn(self.m_curStats)
			if val then
				self:UpdateSlider(sliderData.element, val)
			end
		end
	end
end
function gui.DebugRenderStats:GetTime(sceneName, timer)
	if self.m_sceneNameToStats[sceneName] == nil then
		return
	end
	local stats = self.m_sceneNameToStats[sceneName].stats
	return stats:GetTime(timer)
end
function gui.DebugRenderStats:GetRenderPassTime(sceneName, passId, timer)
	if self.m_sceneNameToStats[sceneName] == nil then
		return
	end
	local stats = self.m_sceneNameToStats[sceneName].stats
	local passStats = stats:GetPassStats(passId)
	return passStats:GetTime(timer)
end
function gui.DebugRenderStats:GetRenderCounter(sceneName, passId, counter)
	if self.m_sceneNameToStats[sceneName] == nil then
		return
	end
	local stats = self.m_sceneNameToStats[sceneName].stats
	local passStats = stats:GetPassStats(passId)
	return passStats:GetCount(counter)
end
function gui.DebugRenderStats:AddScene(statsData)
	local sceneName = statsData.scene
	if self.m_scenes[sceneName] == nil then
		local sceneCat = self:AddCategory({ self.m_tree:GetRoot(), self.m_data }, sceneName)
		local scene = {}
		scene.cat = sceneCat
		self.m_scenes[sceneName] = scene

		self:AddSlider(sceneCat, "[CPU] Execution Time:", function(stats)
			return self.m_sceneNameToStats[sceneName] and self.m_sceneNameToStats[sceneName].cpuExecTime or 0.0
		end, true)
		self:AddSlider(sceneCat, "[CPU] Render Time:", function(stats)
			return self:GetTime(sceneName, game.RenderStats.TIMER_RENDER_SCENE_CPU)
		end, true)
		self:AddSlider(sceneCat, "[GPU] Execution Time:", function(stats)
			return self.m_sceneNameToStats[sceneName] and self.m_sceneNameToStats[sceneName].gpuExecTime or 0.0
		end, true)
		self:AddSlider(sceneCat, "[GPU] Light Culling GPU Execution Time:", function(stats)
			return self:GetTime(sceneName, game.RenderStats.TIMER_LIGHT_CULLING_GPU)
		end, true)
		self:AddSlider(sceneCat, "[GPU] Render Time:", function(stats)
			return self:GetTime(sceneName, game.RenderStats.TIMER_RENDER_SCENE_GPU)
		end, true)
		self:AddSlider(sceneCat, "[GPU] Renderer:", function(stats)
			return self:GetTime(sceneName, game.RenderStats.TIMER_RENDERER_GPU)
		end, true)
		self:AddSlider(sceneCat, "[GPU] Update Render Buffers:", function(stats)
			return self:GetTime(sceneName, game.RenderStats.TIMER_UPDATE_RENDER_BUFFERS_GPU)
		end, true)
		self:AddSlider(sceneCat, "[GPU] Update Prepass Render Buffers:", function(stats)
			return self:GetTime(sceneName, game.RenderStats.TIMER_UPDATE_PREPASS_RENDER_BUFFERS_GPU)
		end, true)
		self:AddSlider(sceneCat, "[GPU] Render Shadows:", function(stats)
			return self:GetTime(sceneName, game.RenderStats.TIMER_RENDER_SHADOWS_GPU)
		end, true)
		self:AddSlider(sceneCat, "[GPU] Render Particles:", function(stats)
			return self:GetTime(sceneName, game.RenderStats.TIMER_RENDER_PARTICLES_GPU)
		end, true)
		local pp = self:AddSlider(sceneCat, "[GPU] Post-processing GPU Execution Time:", function(stats)
			local t = self:GetTime(sceneName, game.RenderStats.TIMER_POST_PROCESSING_GPU)
			if t == nil then
				return
			end
			t = t + self:GetTime(sceneName, game.RenderStats.TIMER_POST_PROCESSING_GPU_SSAO) -- SSAO is rendered before the actual post-processing stage, so we'll have to add it explicitly
			return t
		end, true)
		self:AddSlider(pp, "[GPU] Fog:", function(stats)
			return self:GetTime(sceneName, game.RenderStats.TIMER_POST_PROCESSING_GPU_FOG)
		end, true)
		self:AddSlider(pp, "[GPU] Bloom:", function(stats)
			return self:GetTime(sceneName, game.RenderStats.TIMER_POST_PROCESSING_GPU_BLOOM)
		end, true)
		self:AddSlider(pp, "[GPU] Tone Mapping:", function(stats)
			return self:GetTime(sceneName, game.RenderStats.TIMER_POST_PROCESSING_GPU_TONE_MAPPING)
		end, true)
		self:AddSlider(pp, "[GPU] FXAA:", function(stats)
			return self:GetTime(sceneName, game.RenderStats.TIMER_POST_PROCESSING_GPU_FXAA)
		end, true)
		self:AddSlider(pp, "[GPU] SSAO:", function(stats)
			return self:GetTime(sceneName, game.RenderStats.TIMER_POST_PROCESSING_GPU_SSAO)
		end, true)

		self:AddSlider(sceneCat, "Draw Calls:", function(stats)
			return self.m_sceneNameToStats[sceneName] and self.m_sceneNameToStats[sceneName].drawCalls or 0
		end)
		self:AddSlider(sceneCat, "Triangles:", function(stats)
			return self.m_sceneNameToStats[sceneName] and self.m_sceneNameToStats[sceneName].numTris or 0
		end)
		local passes = {
			[game.RenderStats.RENDER_PASS_LIGHTING_PASS] = "Lighting Pass",
			[game.RenderStats.RENDER_PASS_LIGHTING_PASS_TRANSLUCENT] = "Translucent Lighting Pass",
			[game.RenderStats.RENDER_PASS_PREPASS] = "Prepass",
			[game.RenderStats.RENDER_PASS_SHADOW_PASS] = "Shadow Pass",
		}
		for passId, name in pairs(passes) do
			local cat = self:AddCategory(sceneCat, name)

			self:AddSlider(cat, "CPU Wait Time (+Execution):", function(stats)
				return self:GetRenderPassTime(sceneName, passId, game.RenderPassStats.TIMER_CPU_EXECUTION)
			end, true)
			self:AddSlider(cat, "GPU Execution Time:", function(stats)
				return self:GetRenderPassTime(sceneName, passId, game.RenderPassStats.TIMER_GPU_EXECUTION)
			end, true)

			local catCounters = self:AddCategory(cat, "Counters")
			self:AddSlider(catCounters, "Draw Calls:", function(stats)
				return self:GetRenderCounter(sceneName, passId, game.RenderPassStats.COUNTER_DRAW_CALLS)
			end)
			self:AddSlider(catCounters, "Triangles:", function(stats)
				return self:GetRenderCounter(sceneName, passId, game.RenderPassStats.COUNTER_DRAWN_TRIANGLES)
			end)
			self:AddSlider(catCounters, "Shader State Changes:", function(stats)
				return self:GetRenderCounter(sceneName, passId, game.RenderPassStats.COUNTER_SHADER_STATE_CHANGES)
			end)
			self:AddSlider(catCounters, "Material State Changes:", function(stats)
				return self:GetRenderCounter(sceneName, passId, game.RenderPassStats.COUNTER_MATERIAL_STATE_CHANGES)
			end)
			self:AddSlider(catCounters, "Entity State Changes:", function(stats)
				return self:GetRenderCounter(sceneName, passId, game.RenderPassStats.COUNTER_ENTITY_STATE_CHANGES)
			end)
			self:AddSlider(catCounters, "Meshes:", function(stats)
				return self:GetRenderCounter(sceneName, passId, game.RenderPassStats.COUNTER_DRAWN_MESHES)
			end)
			self:AddSlider(catCounters, "Instance Sets:", function(stats)
				return self:GetRenderCounter(sceneName, passId, game.RenderPassStats.COUNTER_INSTANCE_SETS)
			end)
		end
	end
	--[[for i,val in ipairs(self.m_sliderValues) do
		local el = self.m_sliders[i].element
		if(el:IsValid()) then
			self:UpdateSlider(el,val)
		end
	end]]
	--[[self.m_numScenes = #t -1
		for _,stats in ipairs(t) do
			if(stats.scene == "accumulated") then
				-- TODO: Prepass
				local passStats = stats.stats:GetPassStats(game.RenderStats.RENDER_PASS_LIGHTING_PASS)
				for i,sliderData in ipairs(self.m_sliders) do
					self.m_sliderValues[i] = sliderData.statsFn(passStats)
				end
				break
			end
		end]]

	--[[
			self.m_curStats[i] = {
				scene = stats.scene,
				stats = stats.stats:Copy()
			}
]]
end
function gui.DebugRenderStats:AddLinkedItem(cat, nameL)
	local l = cat[1]:AddItem(nameL)
	local el = gui.create("WIVBox", cat[2])
	el:SetAutoFillContentsToWidth(true)
	el:GetVisibilityProperty():Link(cat[1]:GetChildContentsBox():GetVisibilityProperty())
	return { l, el }
end
function gui.DebugRenderStats:AddCategory(parent, name)
	local cat = self:AddLinkedItem(parent, name)
	gui.create("WIBase", cat[2], 0, 0, 1, cat[1]:GetHeight())
	return cat
end
function gui.DebugRenderStats:AddSlider(cat, label, f, timer)
	timer = timer or false
	local sub = self:AddLinkedItem(cat, label, "")
	local el = sub[2]
	local elSlider = gui.create("WIProgressBar", el, 0, 0, sub[1]:GetWidth(), sub[1]:GetHeight())
	elSlider:SetRange(0.0, 0.01, timer and 0.01 or 1)
	elSlider:SetProgress(0.5)
	if timer then
		elSlider:SetPostFix(" ms")
	end
	table.insert(self.m_sliders, {
		element = elSlider,
		statsFn = f,
	})

	--[[local elLbl = gui.create("WIText",self.m_labelBox)
	elLbl:SetText(label)
	elLbl:SizeToContents()

	local el = gui.create("WIProgressBar",self.m_sliderBox)
	el:SetRange(0.0,1.0,0.01)
	el:SetProgress(0.5)
	table.insert(self.m_sliders,{
		element = el,
		label = elLbl,
		statsFn = f
	})
	table.insert(self.m_sliderValues,0.0)

	elLbl:SetHeight(24)
	el:SetHeight(24)]]
	return sub
end
function gui.DebugRenderStats:UpdateSlider(el, value)
	local min, max, stepSize = el:GetRange()
	if value > max then
		el:SetRange(min, value, stepSize)
	end
	el:SetValue(value)
end
gui.register("WIDebugRenderStats", gui.DebugRenderStats)
