-- SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

-- Create an entity

local ent = ents.create("entity")
assert(ent ~= nil)
ent:Spawn()
ent:Remove()
assert(ent:IsValid() == false)

-- Print all entities
console.print_table(ents.get_all())

return true
