-- LuaMetal desktop (milestone 4 + input)
-- palette, widgets, cursor, window manager: all Lua. C only polls ps/2.

local COL_DESKTOP  = 0x141428
local COL_TASKBAR  = 0x0f0f1e
local COL_TASKBRD  = 0x2a2a4a
local COL_WIN      = 0x1b1b30
local COL_TITLE    = 0x6c5ce7
local COL_TITLEACT = 0x8f7bff
local COL_TEXT     = 0xe8e8f0
local COL_DIM      = 0x8888aa
local COL_CLOSE    = 0xff5f56
local COL_INPUT    = 0x0c0c18

local W = gfx.width()
local H = gfx.height()

-- window state
local win = { x = math.floor(W/2 - 180), y = math.floor(H/2 - 130), w = 360, h = 240, visible = true }
local dragging, grabx, graby = false, 0, 0
local typed = ""
local shift = false
local msgline = "type something..."

-- ps/2 set 1 make codes -> char
local MAP = {
  [0x1e]="1",[0x02]="1",[0x03]="2",[0x04]="3",[0x05]="4",[0x06]="5",
  [0x07]="6",[0x08]="7",[0x09]="8",[0x0a]="9",[0x0b]="0",
  [0x1e]="a",[0x30]="b",[0x2e]="c",[0x20]="d",[0x12]="e",[0x21]="f",
  [0x22]="g",[0x23]="h",[0x17]="i",[0x24]="j",[0x25]="k",[0x26]="l",
  [0x32]="m",[0x31]="n",[0x18]="o",[0x19]="p",[0x10]="q",[0x13]="r",
  [0x1f]="s",[0x14]="t",[0x16]="u",[0x2f]="v",[0x11]="w",[0x2d]="x",
  [0x15]="y",[0x2c]="z",
  [0x0c]="-",[0x0d]="=",[0x1a]="[",[0x1b]="]",[0x2b]="\\",
  [0x27]=";",[0x28]="'",[0x29]="`",[0x33]=",",[0x34]=".",[0x35]="/",
}
local SH = {
  a="A",b="B",c="C",d="D",e="E",f="F",g="G",h="H",i="I",j="J",k="K",l="L",
  m="M",n="N",o="O",p="P",q="Q",r="R",s="S",t="T",u="U",v="V",w="W",
  x="X",y="Y",z="Z",
  ["1"]="!",["2"]="@",["3"]="#",["4"]="$",["5"]="%",["6"]="^",
  ["7"]="&",["8"]="*",["9"]="(",["0"]=")",
  ["-"]="_",["="]="+",["["]="{",["]"]="}",["\\"]="|",
  [";"]=":",["'"]='"',["`"]="~",[","]="<",["."]=">",["/"]="?",
}

local function read_keys()
  local got = false
  while true do
    local b = input.scancode()
    if b < 0 then break end
    got = true
    if b >= 0x80 then
      local code = b - 0x80
      if code == 0x2a or code == 0x36 then shift = false end
    elseif b == 0x2a or b == 0x36 then
      shift = true
    elseif b == 0x1c then
      if typed ~= "" then msgline = "you typed: " .. typed end
      typed = ""
    elseif b == 0x0e then
      typed = typed:sub(1, -2)
    elseif b == 0x39 then
      typed = typed .. " "
    else
      local ch = MAP[b]
      if ch then
        if shift and SH[ch] then ch = SH[ch] end
        if #typed < 40 then typed = typed .. ch end
      end
    end
  end
  return got
end

-- cursor: 1 = outline, 2 = fill, . = transparent
local CUR = {
  "1..........",
  "11.........",
  "121........",
  "1221.......",
  "12221......",
  "122221.....",
  "1222221....",
  "12222221...",
  "122222221..",
  "1222221....",
  "1212121....",
  "12.12121...",
  "12..12121..",
  "12...12121.",
}

local function draw_cursor(mx, my)
  for j = 1, #CUR do
    local row = CUR[j]
    for i = 1, #row do
      local c = row:sub(i, i)
      if c == "1" then gfx.pixel(mx + i - 1, my + j - 1, 0x000000)
      elseif c == "2" then gfx.pixel(mx + i - 1, my + j - 1, 0xffffff) end
    end
  end
end

local function draw()
  gfx.clear(COL_DESKTOP)

  -- window
  local wx, wy, ww, wh = win.x, win.y, win.w, win.h
  if win.visible then
    gfx.rect(wx + 4, wy + 4, ww, wh, 0x0a0a16)          -- shadow
    gfx.rect(wx, wy, ww, wh, COL_WIN)                    -- body
    gfx.rect(wx, wy, ww, 24, COL_TITLE)                  -- title bar
    gfx.rect(wx + ww - 20, wy + 6, 12, 12, COL_CLOSE)    -- close
    Gfx.frame(wx, wy, ww, wh, COL_TASKBRD)
    Gfx.text("LuaMetal", wx + 10, wy + 8, 0xffffff)
    Gfx.text("hello from lua", wx + 16, wy + 40, COL_TEXT)
    Gfx.text("drag me by the title bar", wx + 16, wy + 56, COL_DIM)
    Gfx.text(msgline, wx + 16, wy + 84, COL_DIM)
    -- input box
    gfx.rect(wx + 16, wy + 108, ww - 32, 24, COL_INPUT)
    Gfx.frame(wx + 16, wy + 108, ww - 32, 24, COL_TITLE)
    Gfx.text("> " .. typed .. "_", wx + 22, wy + 116, 0x7bf1a8)
    gfx.rect(wx + 16, wy + 148, 120, 3, COL_TITLE)
    Gfx.text("bare metal. no linux.", wx + 16, wy + 164, COL_DIM)
    Gfx.text("milestone 4: desktop", wx + 16, wy + 180, COL_TEXT)
    Gfx.text("lua drew all of this", wx + 16, wy + 196, COL_TEXT)
  end

  -- taskbar
  local tby = H - 36
  gfx.rect(0, tby, W, 36, COL_TASKBAR)
  Gfx.hline(0, tby, W, COL_TASKBRD)
  gfx.rect(8, tby + 6, 70, 24, COL_TITLE)
  Gfx.text("Start", 18, tby + 14, 0xffffff)
  Gfx.text("LuaMetal", W - 80, tby + 14, COL_DIM)
end

local pmx, pmy, pbtn = -1, -1, 0

function step()
  local mx, my, btn = input.mx(), input.my(), input.btn()
  local dirty = false

  -- typing
  if read_keys() then dirty = true end

  local down = (btn & 1) ~= 0
  local was  = (pbtn & 1) ~= 0

  if down and not was then
    -- close button?
    if win.visible and mx >= win.x + win.w - 24 and mx <= win.x + win.w - 4
       and my >= win.y + 2 and my <= win.y + 22 then
      win.visible = false
      msgline = "window closed. click Start"
    -- start button?
    elseif my >= H - 32 and mx <= 78 then
      win.visible = true
      msgline = "welcome back"
    -- title bar -> start drag
    elseif win.visible and mx >= win.x and mx < win.x + win.w
       and my >= win.y and my < win.y + 24 then
      dragging = true
      grabx = mx - win.x
      graby = my - win.y
    end
    dirty = true
  elseif not down and was then
    dragging = false
    dirty = true
  end

  if dragging then
    local nx, ny = mx - grabx, my - graby
    if nx < 0 then nx = 0 end
    if ny < 0 then ny = 0 end
    if nx > W - win.w then nx = W - win.w end
    if ny > H - win.h - 36 then ny = H - win.h - 36 end
    if nx ~= win.x or ny ~= win.y then
      win.x, win.y = nx, ny
      dirty = true
    end
  end

  if mx ~= pmx or my ~= pmy then dirty = true end
  pmx, pmy, pbtn = mx, my, btn

  if dirty then
    draw()
    draw_cursor(mx, my)
    gfx.flip()
  end
end

draw()
draw_cursor(input.mx(), input.my())
gfx.flip()
print("desktop ready: " .. W .. "x" .. H .. " — step loop engaged")
