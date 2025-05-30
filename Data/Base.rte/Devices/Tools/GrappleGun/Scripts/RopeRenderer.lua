-- Grapple Gun Rope Renderer Module
-- Handles the rendering and visualization of the rope

local RopeRenderer = {}

-- Draw a rope segment with consistent appearance
function RopeRenderer.drawSegment(grappleInstance, a, b, player)
    -- Make sure we have valid points to draw
    if not grappleInstance.apx[a] or not grappleInstance.apy[a] or not grappleInstance.apx[b] or not grappleInstance.apy[b] then
        return
    end
    
    local vect1 = Vector(grappleInstance.apx[a], grappleInstance.apy[a])
    local vect2 = Vector(grappleInstance.apx[b], grappleInstance.apy[b])
    
    -- Safety check for invalid coordinates
    if vect1.X == 0 and vect1.Y == 0 or vect2.X == 0 and vect2.Y == 0 then
        return
    end
    
    -- Calculate rope segment for safety check
    local segmentVec = SceneMan:ShortestDistance(vect1, vect2, grappleInstance.mapWrapsX)
    local segmentLength = segmentVec.Magnitude
    
    -- Safety check for very long segments (probably invalid)
    if segmentLength > 1000 then
        return
    end
    
    -- Use consistent color for all rope segments
    local ropeColor = 97 -- Dark brown color
    
    -- Draw the rope with consistent appearance
    PrimitiveMan:DrawLinePrimitive(player, vect1, vect2, ropeColor)
end

-- Draw the complete rope with debug information
function RopeRenderer.drawRope(grappleInstance, player)
    -- If we're in flight mode, draw a simple direct line
    if grappleInstance.actionMode == 1 then
        -- Draw a direct line from player to hook for visibility during flight
        if grappleInstance.parent then
            PrimitiveMan:DrawLinePrimitive(player, grappleInstance.parent.Pos, grappleInstance.Pos, 97)
        end
    else
        -- Draw regular rope segments with physics
        for i = 0, grappleInstance.currentSegments - 1 do
            RopeRenderer.drawSegment(grappleInstance, i, i + 1, player)
        end
    end
    
    -- Always draw debug information when player is controlling
    RopeRenderer.drawDebugInfo(grappleInstance, player)
end

-- Draw debug information about rope segments and lengths
function RopeRenderer.drawDebugInfo(grappleInstance, player)
    if not grappleInstance.parent or not grappleInstance.parent:IsPlayerControlled() then
        return
    end
    
    local screenPos = grappleInstance.parent.Pos + Vector(-100, -150)
    local lineHeight = 12
    local currentLine = 0
    
    -- Display current rope statistics
    local currentPos = screenPos + Vector(0, currentLine * lineHeight)
    FrameMan:SetScreenText("=== ROPE DEBUG INFO ===", currentPos.X, currentPos.Y, 1000, false)
    currentLine = currentLine + 1
    
    currentPos = screenPos + Vector(0, currentLine * lineHeight)
    FrameMan:SetScreenText("Current Length: " .. math.floor(grappleInstance.currentLineLength or 0), 
                          currentPos.X, currentPos.Y, 1000, false)
    currentLine = currentLine + 1
    
    if grappleInstance.actualRopeLength then
        currentPos = screenPos + Vector(0, currentLine * lineHeight)
        FrameMan:SetScreenText("Actual Length: " .. math.floor(grappleInstance.actualRopeLength), 
                              currentPos.X, currentPos.Y, 1000, false)
        currentLine = currentLine + 1
    end
    
    currentPos = screenPos + Vector(0, currentLine * lineHeight)
    FrameMan:SetScreenText("Max Length: " .. math.floor(grappleInstance.maxLineLength), 
                          currentPos.X, currentPos.Y, 1000, false)
    currentLine = currentLine + 1
    
    currentPos = screenPos + Vector(0, currentLine * lineHeight)
    FrameMan:SetScreenText("Segments: " .. (grappleInstance.currentSegments or 0), 
                          currentPos.X, currentPos.Y, 1000, false)
    currentLine = currentLine + 1
    
    if grappleInstance.currentTension then
        local tensionPercent = math.floor(grappleInstance.currentTension * 100)
        currentPos = screenPos + Vector(0, currentLine * lineHeight)
        FrameMan:SetScreenText("Tension: " .. tensionPercent .. "%", 
                              currentPos.X, currentPos.Y, 1000, false)
        currentLine = currentLine + 1
    end
    
    currentPos = screenPos + Vector(0, currentLine * lineHeight)
    FrameMan:SetScreenText("Line Strength: " .. (grappleInstance.lineStrength or "N/A"), 
                          currentPos.X, currentPos.Y, 1000, false)
    currentLine = currentLine + 1
    
    -- Display individual segment lengths (limit to first 10 segments to avoid clutter)
    if grappleInstance.segmentLengths then
        currentPos = screenPos + Vector(0, currentLine * lineHeight)
        FrameMan:SetScreenText("--- SEGMENT LENGTHS ---", 
                              currentPos.X, currentPos.Y, 1000, false)
        currentLine = currentLine + 1
        
        local segmentsToShow = math.min(10, #grappleInstance.segmentLengths)
        for i = 0, segmentsToShow - 1 do
            if grappleInstance.segmentLengths[i] then
                local segmentText = "Seg " .. i .. ": " .. math.floor(grappleInstance.segmentLengths[i] * 10) / 10
                currentPos = screenPos + Vector(0, currentLine * lineHeight)
                FrameMan:SetScreenText(segmentText, 
                                      currentPos.X, currentPos.Y, 1000, false)
                currentLine = currentLine + 1
            end
        end
        
        if #grappleInstance.segmentLengths > 10 then
            currentPos = screenPos + Vector(0, currentLine * lineHeight)
            FrameMan:SetScreenText("... (" .. (#grappleInstance.segmentLengths - 10) .. " more segments)", 
                                  currentPos.X, currentPos.Y, 1000, false)
        end
    end
end

return RopeRenderer
