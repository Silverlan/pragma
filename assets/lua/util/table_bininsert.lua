--[[
   table.bininsert( table, value [, comp] )
   
   Inserts a given value through BinaryInsert into the table sorted by [, comp].
   
   If 'comp' is given, then it must be a function that receives
   two table elements, and returns true when the first is less
   than the second, e.g. comp = function(a, b) return a > b end,
   will give a sorted table, with the biggest value on position 1.
   [, comp] behaves as in table.sort(table, value [, comp])
   returns the index where 'value' was inserted
]]--
do
   -- Avoid heap allocs for performance
   local fcomp_default = function( a,b ) return a < b end
   function table.bininsert(t, value, fcomp)
      -- Initialise compare function
      local fcomp = fcomp or fcomp_default
      --  Initialise numbers
      local iStart,iEnd,iMid,iState = 1,#t,1,0
      -- Get insert position
      while iStart <= iEnd do
         -- calculate middle
         iMid = math.floor( (iStart+iEnd)/2 )
         -- compare
         if fcomp( value,t[iMid] ) then
            iEnd,iState = iMid - 1,0
         else
            iStart,iState = iMid + 1,1
         end
      end
      table.insert( t,(iMid+iState),value )
      return (iMid+iState)
   end
end
-- CHILLCODE™