-- LuaMetal desktop (milestone 4)
-- everything below is Lua drawing to the framebuffer via the C gfx module.

-- load the font + drawing helpers (these run first via boot modules)
-- (Font and Gfx are globals set by sys/font.lua and sys/gfx.lua)

-- modern dark palette
local COL_DESKTOP  = 0x141428
local COL_TASKBAR  = 0x0f0f1e
local COL_TASKBRD  = 0x2a2a4a
local COL_WIN      = 0x1b1b30
local COL_TITLE    = 0x6c5ce7   -- accent
local COL_TITLEACT = 0x8f7bff
local COL_TEXT     = 0xe8e8f0
local COL_DIM      = 0x8888aa
local COL_CLOSE    = 0xff5f56

local W = gfx.width()
local H = gfx.height()

-- desktop background
gfx.clear(COL_DESKTOP)

-- draw a subtle window
local wx, wy, ww, wh = math.floor(W/2 - 180), math.floor(H/2 - 120), 360, 200
-- drop shadow
gfx.rect(wx + 4, wy + 4, ww, wh, 0x0a0a16)
-- window body
gfx.rect(wx, wy, ww, wh, COL_WIN)
-- title bar
gfx.rect(wx, wy, ww, 24, COL_TITLE)
-- close button
gfx.rect(wx + ww - 20, wy + 6, 12, 12, COL_CLOSE)
-- window border
Gfx.frame(wx, wy, ww, wh, COL_TASKBRD)

-- window title
Gfx.text("LuaMetal", wx + 10, wy + 8, 0xffffff)

-- window content
Gfx.text("hello from lua", wx + 16, wy + 44, COL_TEXT)
Gfx.text("this desktop is", wx + 16, wy + 60, COL_DIM)
Gfx.text("drawn by lua 5.4", wx + 16, wy + 76, COL_DIM)

-- a little accent bar
gfx.rect(wx + 16, wy + 100, 120, 3, COL_TITLE)

-- status text
Gfx.text("bare metal. no linux.", wx + 16, wy + 120, COL_DIM)
Gfx.text("milestone 4: desktop", wx + 16, wy + 136, COL_TEXT)

-- taskbar at the bottom
local tby = H - 36
gfx.rect(0, tby, W, 36, COL_TASKBAR)
Gfx.hline(0, tby, W, COL_TASKBRD)
-- start button
gfx.rect(8, tby + 6, 70, 24, COL_TITLE)
Gfx.text("Start", 18, tby + 14, 0xffffff)
-- clock-ish label on the right
Gfx.text("LuaMetal", W - 80, tby + 14, COL_DIM)

print("desktop drawn: " .. W .. "x" .. H)
