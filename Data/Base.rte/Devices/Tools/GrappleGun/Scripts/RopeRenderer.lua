-- Grapple Gun Rope Renderer Module
-- Handles the rendering and visualization of the rope

local RopeRenderer = {}

-- Calculate total rope distance across all segments
function RopeRenderer.calculateTotalRopeDistance(grappleInstance)
    local totalDistance = 0
    
    for i = 0, grappleInstance.currentSegments - 1 do
        if grappleInstance.apx[i] and grappleInstance.apy[i] and grappleInstance.apx[i+1] and grappleInstance.apy[i+1] then
            local vect1 = Vector(grappleInstance.apx[i], grappleInstance.apy[i])
            local vect2 = Vector(grappleInstance.apx[i+1], grappleInstance.apy[i+1])
            local segmentVec = SceneMan:ShortestDistance(vect1, vect2, grappleInstance.mapWrapsX)
            totalDistance = totalDistance + segmentVec.Magnitude
        end
    end
    
    return totalDistance
end

-- Calculate distance from start of rope to current segment
function RopeRenderer.calculateDistanceToSegment(grappleInstance, segmentIndex)
    local distanceToSegment = 0
    
    for i = 0, segmentIndex - 1 do
        if grappleInstance.apx[i] and grappleInstance.apy[i] and grappleInstance.apx[i+1] and grappleInstance.apy[i+1] then
            local vect1 = Vector(grappleInstance.apx[i], grappleInstance.apy[i])
            local vect2 = Vector(grappleInstance.apx[i+1], grappleInstance.apy[i+1])
            local segmentVec = SceneMan:ShortestDistance(vect1, vect2, grappleInstance.mapWrapsX)
            distanceToSegment = distanceToSegment + segmentVec.Magnitude
        end
    end
    
    return distanceToSegment
end

-- Draw a rope segment with varying thickness based on tension and color gradient
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
    
    -- Calculate position along rope for color gradient based on total distance (0.0 = start, 1.0 = end)
    local totalRopeDistance = RopeRenderer.calculateTotalRopeDistance(grappleInstance)
    local distanceToCurrentSegment = RopeRenderer.calculateDistanceToSegment(grappleInstance, a)
    local gradient_position = 0
    
    if totalRopeDistance > 0 then
        gradient_position = distanceToCurrentSegment / totalRopeDistance
    else
        -- Fallback to simple segment ratio if distance calculation fails
        gradient_position = a / math.max(1, grappleInstance.currentSegments)
    end
    
    -- Tension-based color override (takes precedence over gradient)
    local ropeColor = 155 -- Default light brown
    if tensionRatio > 1.2 then
        -- Rope is under high tension - show as reddish regardless of position
        ropeColor = 13 -- Reddish color
    elseif tensionRatio < 0.8 then
        -- Rope is slack - show as darker color
        ropeColor = 97 -- Darker brown
    else
        -- Normal tension - apply smooth gradient color with mathematical interpolation
        -- Linear interpolation from light brown (155) to dark brown (97)
        local startColor = 155 -- Light brown near start (grapple gun)
        local endColor = 97   -- Dark brown near end (hook)
        
        -- Calculate interpolated color value using linear interpolation
        -- formula: result = start + (end - start) * t, where t is 0.0 to 1.0
        ropeColor = math.floor(startColor + (endColor - startColor) * gradient_position)
        
        -- Ensure color stays within valid range
        ropeColor = math.max(97, math.min(155, ropeColor))
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
    if grappleInstance.limitReached and grappleInstance.actionMode > 1 then
        -- Calculate overall rope tension based on total distance vs target
        local totalRopeDistance = RopeRenderer.calculateTotalRopeDistance(grappleInstance)
        local targetTotalDistance = grappleInstance.currentLineLength
        
        local currentTension = 0
        if targetTotalDistance > 0 and totalRopeDistance > targetTotalDistance then
            -- Rope is stretched beyond target length
            currentTension = (totalRopeDistance - targetTotalDistance) / targetTotalDistance
        elseif grappleInstance.setLineLength > 0 and grappleInstance.currentLineLength < grappleInstance.setLineLength then
            -- Fallback to original calculation method
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
end

-- Debug information display function
function RopeRenderer.showDebugInfo(grappleInstance, player, debugTextPos) -- Changed self to grappleInstance, added debugTextPos
    if not grappleInstance.parent or player <= 0 or not grappleInstance.parent:IsPlayerControlled() then
        return
    end
    
    -- Position for debug text - allow passing it in, or default
    local pos = debugTextPos or Vector(grappleInstance.parent.Pos.X - 60, grappleInstance.parent.Pos.Y - 60)
    
    -- Calculate total rope distance
    local totalDistance = RopeRenderer.calculateTotalRopeDistance(grappleInstance)
    
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
    pos.Y = pos.Y + 10
    PrimitiveMan:DrawTextPrimitive(player, pos, "Total Distance: " .. string.format("%.2f", totalDistance), 162)
end

return RopeRenderer
