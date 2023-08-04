if CLIENT == true then
	locale.load("weapon_physcannon.txt")
else
	resource.add_lua_file("physcannon.lua")
end

local function register_entity(gm)
	if gm.RegisterEntity == nil then
		return
	end
	gm:RegisterEntity(game.Sandbox.ENTITY_CATEGORY_WEAPON, "weapon_physcannon", "physcannon")
end
local gm = game.get_game_mode()
if gm ~= nil then
	register_entity(gm)
end
game.add_callback("OnGameModeInitialized", register_entity)
