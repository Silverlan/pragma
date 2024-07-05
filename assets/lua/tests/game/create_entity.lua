-- Create an entity
local ent = ents.create("entity")
assert(ent ~= nil)
ent:Spawn()
ent:Remove()
assert(ent:IsValid() == false)

-- Print all entities
console.print_table(ents.get_all())

return true
