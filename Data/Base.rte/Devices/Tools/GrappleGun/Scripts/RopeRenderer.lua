---@diagnostic disable: undefined-global
-- Grapple Gun Rope Renderer Module
-- Handles the visual rendering of the rope and optional debug information.

-- Localize Cortex Command globals
local PrimitiveMan = PrimitiveMan
local SceneMan = SceneMan
local FrameMan = FrameMan
local Vector = Vector

local RopeRenderer = {}

-- Configuration for rendering
local ROPE_COLOR = 97 -- Dark brown color, consistent with original.
local DEBUG_TEXT_COLOR = 1000 -- Standard white for debug text.
local DEBUG_LINE_HEIGHT = 12
local MAX_DEBUG_SEGMENTS_TO_SHOW = 10 -- Limit displayed segment lengths to avoid clutter.

--[[
  Draws a single segment of the rope.
  @param grappleInstance The grapple instance.
  @param segmentStartIdx Index of the starting point of the segment.
  @param segmentEndIdx   Index of the ending point of the segment.
  @param player          The player index for the screen context.
]]
function RopeRenderer.drawSegment(grappleInstance, segmentStartIdx, segmentEndIdx, player)
    -- Validate that the segment indices and corresponding points exist.
    if not grappleInstance.apx or
       not grappleInstance.apx[segmentStartIdx] or not grappleInstance.apy[segmentStartIdx] or
       not grappleInstance.apx[segmentEndIdx] or not grappleInstance.apy[segmentEndIdx] then
        -- print("RopeRenderer: Invalid segment indices or points for drawing.")
        return
    end
    
    local point1 = Vector(grappleInstance.apx[segmentStartIdx], grappleInstance.apy[segmentStartIdx])
    local point2 = Vector(grappleInstance.apx[segmentEndIdx], grappleInstance.apy[segmentEndIdx])
    
    -- Safety check for zero vectors, which might indicate uninitialized points.
    if (point1.X == 0 and point1.Y == 0) or (point2.X == 0 and point2.Y == 0) then
        -- print("RopeRenderer: Segment point is zero vector, skipping draw.")
        return
    end
    
    -- Calculate visual segment vector and length for sanity checking.
    local visualSegmentVec = SceneMan:ShortestDistance(point1, point2, grappleInstance.mapWrapsX)
    local visualSegmentLength = visualSegmentVec.Magnitude
    
    -- Safety check for excessively long visual segments, which could be an error or cause rendering issues.
    if visualSegmentLength > (grappleInstance.maxLineLength or 600) * 1.5 then -- Allow some slack over maxLineLength
        -- print("RopeRenderer: Visual segment length (" .. visualSegmentLength .. ") is excessively long, skipping draw.")
        return
    end
    
    -- Fix the DrawLinePrimitive call - remove player parameter if it's nil
    PrimitiveMan:DrawLinePrimitive(point1, point2, ROPE_COLOR)
end

--[[
  Draws the complete rope, iterating through its segments.
  Also triggers debug information drawing if conditions are met.
  @param grappleInstance The grapple instance.
  @param player          The player index for the screen context.
]]--
function RopeRenderer.drawRope(grappleInstance, player)
    if not grappleInstance or grappleInstance.currentSegments == nil or grappleInstance.currentSegments < 1 then
        return -- Nothing to draw if no segments.
    end

    -- Draw each segment of the rope.
    for i = 0, grappleInstance.currentSegments - 1 do
        RopeRenderer.drawSegment(grappleInstance, i, i + 1, player)
    end
    
    -- Optionally draw debug information.
    -- Condition: Parent exists, is player controlled, and a global debug flag could be added here.
    if grappleInstance.parent and grappleInstance.parent:IsPlayerControlled() then -- Add 'and GlobalDebugFlags.Grapple'
        RopeRenderer.drawDebugInfo(grappleInstance, player)
    end
end

--[[
  Draws debug information on screen regarding the rope's state.
  @param grappleInstance The grapple instance.
  @param player          The player index for the screen context.
]]
function RopeRenderer.drawDebugInfo(grappleInstance, player)
    -- Ensure parent is valid before trying to position debug text relative to it.
    if not grappleInstance.parent or not grappleInstance.parent.Pos then
        return
    end
    
    local screenPos = grappleInstance.parent.Pos + Vector(-120, -180) -- Adjusted for better visibility
    local currentLine = 0
    
    local function drawDebugText(text)
        local textPos = screenPos + Vector(0, currentLine * DEBUG_LINE_HEIGHT)
        FrameMan:SetScreenText(text, textPos.X, textPos.Y, DEBUG_TEXT_COLOR, false)
        currentLine = currentLine + 1
    end
    
    drawDebugText("=== GRAPPLE DEBUG ===")
    drawDebugText("Mode: " .. (grappleInstance.actionMode or "N/A"))
    drawDebugText(string.format("Target Length: %.1f", grappleInstance.currentLineLength or 0))
    drawDebugText(string.format("Visual Length: %.1f", grappleInstance.lineLength or 0)) -- Actual distance player-hook
    drawDebugText(string.format("Physics Length (Verlet): %.1f", grappleInstance.actualRopeLength or 0)) -- Sum of segment lengths
    drawDebugText("Max Length: " .. (grappleInstance.maxLineLength or "N/A"))
    drawDebugText("Segments: " .. (grappleInstance.currentSegments or 0))
    
    if grappleInstance.currentTension then
        drawDebugText(string.format("Tension (Stretch): %.2f%%", grappleInstance.currentTension * 100))
    end
    drawDebugText("Limit Reached: " .. tostring(grappleInstance.limitReached or false))
    
    -- Display individual segment lengths (limited count).
    if grappleInstance.apx and grappleInstance.currentSegments and grappleInstance.currentSegments > 0 then
        drawDebugText("--- SEGMENT LENGTHS ---")
        local segmentsToShow = math.min(MAX_DEBUG_SEGMENTS_TO_SHOW, grappleInstance.currentSegments)
        for i = 0, segmentsToShow - 1 do
            if grappleInstance.apx[i+1] and grappleInstance.apx[i] then
                 local p1 = Vector(grappleInstance.apx[i], grappleInstance.apy[i])
                 local p2 = Vector(grappleInstance.apx[i+1], grappleInstance.apy[i+1])
                 local len = SceneMan:ShortestDistance(p1, p2, grappleInstance.mapWrapsX).Magnitude
                 drawDebugText(string.format("Seg %d: %.1f", i, len))
            else
                drawDebugText(string.format("Seg %d: Invalid", i))
            end
        end
        
        if grappleInstance.currentSegments > MAX_DEBUG_SEGMENTS_TO_SHOW then
            drawDebugText("... (" .. (grappleInstance.currentSegments - MAX_DEBUG_SEGMENTS_TO_SHOW) .. " more)")
        end
    end
end

return RopeRenderer
