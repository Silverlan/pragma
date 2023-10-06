-- This script is executed by the GitHub runner test workflows

print("CI Runner Test Script Start")

-- If a Lua error occurs, the contents of ci.txt will remain '0', otherwise '1'
file.write("ci.txt", "0")

-- Create an entity
local ent = ents.create("entity")
assert(ent ~= nil)
ent:Spawn()
ent:Remove()
assert(ent:IsValid() == false)

-- Print all entities
console.print_table(ents.get_all())

-- TODO: Run other tests

file.write("ci.txt", "1")

print("CI Runner Test Script End")

engine.shutdown()
