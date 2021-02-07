include("/gamemodes/generic/client/cl_init.lua")
include("../shared.lua")
include("cl_hud.lua")

function game.Zen:OnPlayerJoined(pl)
	if(pl:IsLocalPlayer() == false) then return end
	local p = gui.create("WIZenHUD")
	p:SetSize(400,200)
end

function game.Zen:OnGameInitialized()
end

function game.Zen:OnMapInitialized()
end
