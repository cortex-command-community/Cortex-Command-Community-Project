-- Grapple Gun Rope Renderer Module
-- Handles the rendering and visualization of the rope

local RopeRenderer = {}

-- Draw a rope segment with varying thickness based on tension
function RopeRenderer.drawSegment(grappleInstance, a, b, player) -- Changed self to grappleInstance for clarity
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
    
    -- Calculate rope segment tension
    local segmentVec = SceneMan:ShortestDistance(vect1, vect2, grappleInstance.mapWrapsX)
    local segmentLength = segmentVec.Magnitude
    
    -- Safety check for very long segments (probably invalid)
    if segmentLength > 1000 then
        return
    end
    
    local targetLength = math.max(1, grappleInstance.currentLineLength) / math.max(1, grappleInstance.currentSegments)
    local tensionRatio = segmentLength / math.max(1, targetLength)
    
    -- Color based on tension (normal: light brown, stretched: reddish)
    local ropeColor = 155 -- Default light brown color
    
    -- Tense rope shows as red
    if tensionRatio > 1.2 then
        -- Rope is under high tension - show as reddish
        ropeColor = 13 -- Reddish color
    elseif tensionRatio < 0.8 then
        -- Rope is slack - show as darker brown
        ropeColor = 97 -- Darker brown
    end
    
    -- Draw the rope with appropriate color
    PrimitiveMan:DrawLinePrimitive(player, vect1, vect2, ropeColor)
    
    -- Draw thicker line for stressed segments
    if tensionRatio > 1.2 then
        -- Draw a second line for thickness
        local perpVec = Vector(-segmentVec.Y, segmentVec.X):SetMagnitude(0.5)
        local v1a = Vector(vect1.X + perpVec.X, vect1.Y + perpVec.Y)
        local v1b = Vector(vect1.X - perpVec.X, vect1.Y - perpVec.Y)
        local v2a = Vector(vect2.X + perpVec.X, vect2.Y + perpVec.Y)
        local v2b = Vector(vect2.X - perpVec.X, vect2.Y - perpVec.Y)
        
        PrimitiveMan:DrawLinePrimitive(player, v1a, v2a, ropeColor)
        PrimitiveMan:DrawLinePrimitive(player, v1b, v2b, ropeColor)
    end
end

-- Draw the entire rope
function RopeRenderer.drawRope(grappleInstance, player) -- Changed self to grappleInstance
    -- If we're in flight mode, draw a simple direct line to ensure visibility
    if grappleInstance.actionMode == 1 then
        -- Draw a direct line from player to hook for better visibility during flight
        if grappleInstance.parent then
            PrimitiveMan:DrawLinePrimitive(player, grappleInstance.parent.Pos, grappleInstance.Pos, 155)
        end
    else
        -- Draw regular rope segments with physics
        for i = 0, grappleInstance.currentSegments - 1 do
            RopeRenderer.drawSegment(grappleInstance, i, i + 1, player) -- Use RopeRenderer.drawSegment
        end
    end
end

-- Show tension indicator above player when rope is tense
function RopeRenderer.showTensionIndicator(grappleInstance, player) -- Changed self to grappleInstance
    if not grappleInstance.parent or player <= 0 or not grappleInstance.parent:IsPlayerControlled() then
        return
    end
    
    -- Only show when rope is under tension (original logic was comparing lineLength and currentLineLength)
    -- This needs to be adapted based on how tension is actually determined in Grapple.lua
    -- For now, let's assume a simple tension model if currentLineLength is less than a set lineLength
    -- This part might need adjustment based on the main Grapple.lua logic for 'lineStrength' or similar
    local currentTension = 0
    if grappleInstance.setLineLength > 0 and grappleInstance.currentLineLength < grappleInstance.setLineLength then
         currentTension = (grappleInstance.setLineLength - grappleInstance.currentLineLength) / grappleInstance.setLineLength
    end

    if currentTension < 0.1 then -- Show only if tension is somewhat significant
        return
    end
    
    local tensionRatio = math.min(currentTension * 5, 1.0) -- Scale for visibility, max 1.0

    -- Calculate indicator position (above player)
    local indicatorPos = Vector(grappleInstance.parent.Pos.X, grappleInstance.parent.Pos.Y - grappleInstance.parent.Height * 0.5 - 12)
    
    -- Visual indicator style based on tension
    local indicatorWidth = 20
    local indicatorHeight = 3
    
    -- Draw tension bar background
    PrimitiveMan:DrawBoxFillPrimitive(player, 
        indicatorPos - Vector(indicatorWidth/2 + 1, indicatorHeight/2 + 1), 
        indicatorPos + Vector(indicatorWidth/2 + 1, indicatorHeight/2 + 1), 
        13) -- Dark background (using color 13 as in original)
        
    -- Draw tension bar fill
    PrimitiveMan:DrawBoxFillPrimitive(player, 
        indicatorPos - Vector(indicatorWidth/2, indicatorHeight/2), 
        indicatorPos + Vector(indicatorWidth/2 * tensionRatio, indicatorHeight/2), 
        13) -- Red fill (using color 13 as in original, was 5)
        
    -- Draw warning text if close to breaking (e.g. tensionRatio > 0.8)
    if tensionRatio > 0.8 then
        local warningPos = Vector(indicatorPos.X, indicatorPos.Y - 10)
        PrimitiveMan:DrawTextPrimitive(player, warningPos, "TENSION!", 162)
    end
end

-- Debug information display function
function RopeRenderer.showDebugInfo(grappleInstance, player, debugTextPos) -- Changed self to grappleInstance, added debugTextPos
    if not grappleInstance.parent or player <= 0 or not grappleInstance.parent:IsPlayerControlled() then
        return
    end
    
    -- Position for debug text - allow passing it in, or default
    local pos = debugTextPos or Vector(grappleInstance.parent.Pos.X - 60, grappleInstance.parent.Pos.Y - 60)
    
    -- Show rope state information
    PrimitiveMan:DrawTextPrimitive(player, pos, "Rope State:", 162)
    pos.Y = pos.Y + 10
    PrimitiveMan:DrawTextPrimitive(player, pos, "Mode: " .. grappleInstance.actionMode, 162)
    pos.Y = pos.Y + 10
    PrimitiveMan:DrawTextPrimitive(player, pos, "Length: " .. string.format("%.2f", grappleInstance.currentLineLength), 162)
    pos.Y = pos.Y + 10
    PrimitiveMan:DrawTextPrimitive(player, pos, "Segments: " .. grappleInstance.currentSegments, 162)
    pos.Y = pos.Y + 10
    PrimitiveMan:DrawTextPrimitive(player, pos, "Set Length: " .. grappleInstance.setLineLength, 162)
end

return RopeRenderer
