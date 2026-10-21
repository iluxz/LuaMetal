-- Higher-level drawing, 100% Lua, on top of the C `gfx` primitives.
-- This is where the "OS look" starts to live.

Gfx = {}

function Gfx.text(str, x, y, color, bg)
  local cx = x
  for i = 1, #str do
    local code = string.byte(str, i)
    local g = Font[code]
    if g then
      for ry = 0, 7 do
        local row = g[ry + 1]
        for rx = 0, 7 do
          -- bit0 (LSB) is the leftmost pixel
          local on = (row >> rx) & 1
          if on == 1 then
            gfx.pixel(cx + rx, y + ry, color)
          elseif bg then
            gfx.pixel(cx + rx, y + ry, bg)
          end
        end
      end
    end
    cx = cx + 8
  end
  return cx
end

function Gfx.text_width(str) return #str * 8 end

function Gfx.hline(x, y, w, color)
  gfx.rect(x, y, w, 1, color)
end

function Gfx.vline(x, y, h, color)
  gfx.rect(x, y, 1, h, color)
end

function Gfx.frame(x, y, w, h, color)
  Gfx.hline(x, y, w, color)
  Gfx.hline(x, y + h - 1, w, color)
  Gfx.vline(x, y, h, color)
  Gfx.vline(x + w - 1, y, h, color)
end

return Gfx
