local models = { "cube", "empty", "error", "player/soldier", "maps/empty_sky/skybox_3" }

for _, mdlName in ipairs(models) do
	local mdl = game.load_model(mdlName)
	if mdl == nil then
		return false, "Failed to load model '" .. mdlName .. "'!"
	end
end

return true
