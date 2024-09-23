--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

util.register_class("ents.SplitScreen", BaseEntityComponent)

ents.SplitScreen.SCREEN_LEFT = 1
ents.SplitScreen.SCREEN_RIGHT = 2

function ents.SplitScreen:Initialize()
	BaseEntityComponent.Initialize(self)

	self:AddEntityComponent(ents.COMPONENT_TRANSFORM)
	self:AddEntityComponent(ents.COMPONENT_TOGGLE)
	self:AddEntityComponent(ents.COMPONENT_CAMERA)

	self.m_scenes = {}
	self.m_scenes[ents.SplitScreen.SCREEN_LEFT] = {}
	self.m_scenes[ents.SplitScreen.SCREEN_RIGHT] = {}
end

function ents.SplitScreen:OnRemove()
	game.set_default_game_render_enabled(true)
	for _, sceneData in ipairs(self.m_scenes) do
		if util.is_valid(sceneData.scene) then
			util.remove(sceneData.scene)
		end
		if sceneData.ownCamera and util.is_valid(sceneData.camera) then
			util.remove(sceneData.camera:GetEntity())
		end
		if sceneData.renderer:IsValid() then
			util.remove(sceneData.renderer:GetEntity())
		end
	end
	util.remove(self.m_cbDrawScenes)
	util.remove(self.m_uiEl)
end

function ents.SplitScreen:SetResolution(width, height)
	self.m_width = width
	self.m_height = height
end

function ents.SplitScreen:CreateScene(idx)
	local sceneCreateInfo = ents.SceneComponent.CreateInfo()
	sceneCreateInfo.sampleCount = prosper.SAMPLE_COUNT_1_BIT
	local gameScene = game.get_scene()
	local scene = ents.create_scene(sceneCreateInfo, gameScene)
	scene:Link(gameScene)
	scene:GetEntity():SetName("split_screen_" .. ((idx == ents.SplitScreen.SCREEN_LEFT) and "left" or "right"))

	local entRenderer = self:GetEntity():CreateChild("rasterization_renderer")
	local renderer = entRenderer:GetComponent(ents.COMPONENT_RENDERER)
	local rasterizer = entRenderer:GetComponent(ents.COMPONENT_RASTERIZATION_RENDERER)
	rasterizer:SetSSAOEnabled(true)
	renderer:InitializeRenderTarget(scene, self.m_width or gameScene:GetWidth(), self.m_height or gameScene:GetHeight())

	local drawSceneInfo = game.DrawSceneInfo()
	drawSceneInfo.scene = scene
	drawSceneInfo.outputImage = renderer:GetPresentationTexture():GetImage()

	scene:SetRenderer(renderer)
	self.m_scenes[idx].scene = scene
	self.m_scenes[idx].renderer = renderer
	self.m_scenes[idx].drawSceneInfo = drawSceneInfo
end

function ents.SplitScreen:SetCamera(idx, cam, ownCamera)
	if ownCamera == nil then
		ownCamera = not util.is_same_object(cam, game.get_primary_camera())
	end
	self.m_scenes[idx].scene:SetActiveCamera(cam)
	self.m_scenes[idx].camera = cam
	self.m_scenes[idx].ownCamera = ownCamera
end

function ents.SplitScreen:OnEntitySpawn()
	self:CreateScene(ents.SplitScreen.SCREEN_LEFT)
	self:CreateScene(ents.SplitScreen.SCREEN_RIGHT)

	self.m_cbDrawScenes = game.add_callback("RenderScenes", function(drawSceneInfo)
		self:RenderEyes(drawSceneInfo)
	end)

	local el = gui.get_base_element()
	self.m_uiEl = gui.create("WIBase")
	self.m_uiEl:SetSize(el:GetWidth(), el:GetHeight())

	local elTexLeft =
		gui.create("WITexturedRect", self.m_uiEl, 0, 0, self.m_uiEl:GetWidth() / 2, self.m_uiEl:GetHeight(), 0, 0, 1, 1)
	elTexLeft:SetTexture(self.m_scenes[ents.SplitScreen.SCREEN_LEFT].renderer:GetPresentationTexture())

	local elTexRight = gui.create(
		"WITexturedRect",
		self.m_uiEl,
		elTexLeft:GetRight(),
		0,
		self.m_uiEl:GetWidth() / 2,
		self.m_uiEl:GetHeight(),
		0,
		0,
		1,
		1
	)
	elTexRight:SetTexture(self.m_scenes[ents.SplitScreen.SCREEN_RIGHT].renderer:GetPresentationTexture())
end

function ents.SplitScreen:RenderEyes(drawSceneInfo)
	game.set_default_game_render_enabled(false)
	for _, sceneData in ipairs(self.m_scenes) do
		--if(_ > 1) then break end
		if util.is_valid(sceneData.scene) then
			game.queue_scene_for_rendering(sceneData.drawSceneInfo)
		end
	end
end
ents.register_component("split_screen", ents.SplitScreen, "util")
