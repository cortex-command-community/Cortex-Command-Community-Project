---@diagnostic disable: undefined-global
-- Grapple Gun State Manager Module
-- Handles grapple state transitions, collision checks for attachment,
-- and effects related to the grapple's state.

-- Localize Cortex Command globals
local CreateMOPixel = CreateMOPixel
local SceneMan = SceneMan
local MovableMan = MovableMan
local Vector = Vector
local rte = rte

local RopeStateManager = {}

--[[
  Initializes the core state variables for the grapple instance.
  Called from Grapple.lua's Create function.
  @param grappleInstance The grapple instance.
]]
function RopeStateManager.initState(grappleInstance)
    grappleInstance.actionMode = 0    -- 0: Start/Inactive, 1: Flying, 2: Grabbed Terrain, 3: Grabbed MO
    grappleInstance.limitReached = false -- True if rope is at max extension.
    grappleInstance.canRelease = false   -- True if the grapple is in a state where it can be released by player action.
    grappleInstance.currentLineLength = 0 -- The current physics target length of the rope.
    -- grappleInstance.longestLineLength = 0 -- Seems unused, consider removing.
    grappleInstance.setLineLength = 0     -- The length explicitly set by input or logic.
    
    grappleInstance.target = nil -- Stores the MO if actionMode is 3.
    grappleInstance.stickPosition = nil -- Offset from target MO's origin.
    grappleInstance.stickRotation = nil -- Initial rotation of target MO.
    grappleInstance.stickDirection = nil -- Initial rotation of the grapple claw itself.

    grappleInstance.shouldBreak = false -- Flag to indicate rope should break.
    grappleInstance.ropePhysicsInitialized = false -- Flag for one-time physics setups if needed.
end

--[[
  Checks for collisions when the grapple is flying, to transition to an attached state.
  @param grappleInstance The grapple instance.
  @return True if the state changed (grapple attached), false otherwise.
]]
function RopeStateManager.checkAttachmentCollisions(grappleInstance)
    if grappleInstance.actionMode ~= 1 then return false end -- Only process in flying state.
    
    local stateChanged = false
    
    -- More precise collision detection with velocity-based scaling
    local baseRayLength = math.max(3, (grappleInstance.Diameter or 4) * 1.2) -- Reduced from *2
    local velocityComponent = math.min(8, (grappleInstance.Vel and grappleInstance.Vel.Magnitude or 0) * 0.6) -- Cap velocity influence
    local rayLength = baseRayLength + velocityComponent
    rayLength = math.max(5, rayLength) -- Reduced minimum from 10 to 5

    local rayDirection = Vector(1,0) -- Default direction
    if grappleInstance.Vel and grappleInstance.Vel.Magnitude and grappleInstance.Vel.Magnitude > 0.005 then -- Reduced threshold for better sensitivity
        local mag = grappleInstance.Vel.Magnitude
        if mag ~= 0 then
            rayDirection = Vector(grappleInstance.Vel.X / mag, grappleInstance.Vel.Y / mag)
        end
    end
    
    -- Primary ray (most precise) - velocity direction
    local collisionRay = rayDirection * rayLength
    local hitPoint = Vector()
    
    -- Secondary ray (fallback) - shorter but still directional  
    local secondaryRayLength = math.max(3, baseRayLength * 0.6) -- Reduced from 0.75
    local secondaryHitPoint = Vector()
    
    -- Close-range radius (last resort) - much smaller
    local closeRangeRadius = math.max(2, (grappleInstance.Diameter or 4) * 0.8) -- Reduced significantly
    local terrainHit = false
    local finalHitPoint = Vector()

    -- 1. Check for Terrain Collision (primary ray)
    local terrainHit = SceneMan:CastStrengthRay(grappleInstance.Pos, collisionRay, 0, hitPoint, 0, rte.airID, grappleInstance.mapWrapsX)
    
    -- 2. Check for terrain with secondary shorter ray for better sensitivity
    local secondaryTerrainHit = false
    if not terrainHit then
        secondaryTerrainHit = SceneMan:CastStrengthRay(grappleInstance.Pos, rayDirection * secondaryRayLength, 0, secondaryHitPoint, 0, rte.airID, grappleInstance.mapWrapsX)
        if secondaryTerrainHit then
            hitPoint = secondaryHitPoint
            terrainHit = true
        end
    end
    
    -- 3. Check for close-range terrain collision (only if moving slowly or nearly stopped)
    if not terrainHit and (not grappleInstance.Vel or grappleInstance.Vel.Magnitude < 3) then
        -- Only use close-range when hook is moving slowly (more precise)
        local checkAngles = {0, math.pi/2, math.pi, 3*math.pi/2} -- Reduced from 8 to 4 directions
        for _, angle in ipairs(checkAngles) do
            local checkDir = Vector(math.cos(angle), math.sin(angle)) * closeRangeRadius
            local closeRangeHit = Vector()
            if SceneMan:CastStrengthRay(grappleInstance.Pos, checkDir, 0, closeRangeHit, 0, rte.airID, grappleInstance.mapWrapsX) then
                hitPoint = closeRangeHit
                terrainHit = true
                break
            end
        end
    end
    
    if terrainHit then
        grappleInstance.actionMode = 2 -- Transition to "Grabbed Terrain"
        grappleInstance.Pos = hitPoint -- Snap grapple to the hit point.
        grappleInstance.apx[grappleInstance.currentSegments] = hitPoint.X -- Update anchor point
        grappleInstance.apy[grappleInstance.currentSegments] = hitPoint.Y -- Update anchor point
        grappleInstance.lastX[grappleInstance.currentSegments] = hitPoint.X -- Ensure lastPos is also updated for stability
        grappleInstance.lastY[grappleInstance.currentSegments] = hitPoint.Y
        stateChanged = true
        if grappleInstance.stickSound then grappleInstance.stickSound:Play(grappleInstance.Pos) end
    else
        -- 3. Check for Movable Object (MO) Collision (primary ray)
        local hitMORayInfo = SceneMan:CastMORay(grappleInstance.Pos, collisionRay, 
                                            (grappleInstance.parent and grappleInstance.parent.ID or 0), -- Exclude parent actor
                                            -2, -- Hit any team except own if negative, or specific team. -2 for any other.
                                            rte.airID, false, 0) -- flags, filter
        
        -- 4. Check for MO with secondary ray if primary failed
        if not (hitMORayInfo and type(hitMORayInfo) == "table" and hitMORayInfo.MOSPtr and hitMORayInfo.MOSPtr.ID ~= rte.NoMOID) then
            hitMORayInfo = SceneMan:CastMORay(grappleInstance.Pos, rayDirection * secondaryRayLength, 
                                                (grappleInstance.parent and grappleInstance.parent.ID or 0),
                                                -2, rte.airID, false, 0)
        end
        
        if hitMORayInfo and type(hitMORayInfo) == "table" and hitMORayInfo.MOSPtr and hitMORayInfo.MOSPtr.ID ~= rte.NoMOID then
            local hitMO = hitMORayInfo.MOSPtr
            
            -- Filter out tiny particles or debris (improved target selection)
            local minGrappableSize = 3 -- Minimum diameter for grappable objects
            if hitMO.Diameter and hitMO.Diameter < minGrappableSize then
                -- Skip tiny objects, continue to secondary ray check
                local secondaryHit = SceneMan:CastMORay(grappleInstance.Pos, rayDirection * secondaryRayLength, 
                                                    (grappleInstance.parent and grappleInstance.parent.ID or 0),
                                                    -2, rte.airID, false, 0)
                if secondaryHit and type(secondaryHit) == "table" and secondaryHit.MOSPtr and secondaryHit.MOSPtr.ID ~= rte.NoMOID then
                    hitMO = secondaryHit.MOSPtr
                    hitMORayInfo = secondaryHit
                else
                    hitMO = nil -- No valid target found
                    hitMORayInfo = nil
                end
            end
            
            if hitMO and hitMORayInfo then
                grappleInstance.target = hitMO -- Store the hit MO.
                
                -- If the MO is pinned (e.g., a static object like a bunker piece, or a character that used "Pin Self"), treat it like terrain.
                -- Also consider MOs that are not Actors but might be part of the terrain/level.
                local isPinnedActor = MovableMan:IsActor(hitMO) and ToActor(hitMO):IsPinned()
                -- One could add more conditions here, e.g. checking hitMO.Material.Mass == 0 for static terrain pieces if applicable
                
                if isPinnedActor or (not MovableMan:IsActor(hitMO) and hitMO.Material and hitMO.Material.Mass == 0) then 
                    grappleInstance.actionMode = 2 -- Grabbed Terrain (effectively)
                    grappleInstance.Pos = hitMORayInfo.HitPos -- Snap grapple to the hit point on MO
                    grappleInstance.apx[grappleInstance.currentSegments] = hitMORayInfo.HitPos.X -- Update anchor point
                    grappleInstance.apy[grappleInstance.currentSegments] = hitMORayInfo.HitPos.Y -- Update anchor point
                    grappleInstance.lastX[grappleInstance.currentSegments] = hitMORayInfo.HitPos.X
                    grappleInstance.lastY[grappleInstance.currentSegments] = hitMORayInfo.HitPos.Y
                    -- For stickDirection, it might be better to use the hit normal if available,
                    -- otherwise, the direction from player to hook is a fallback.
                    -- local hitNormal = hitMORayInfo.HitNormal 
                    -- grappleInstance.stickDirection = hitNormal or (grappleInstance.Pos - grappleInstance.parent.Pos):Normalized()
                    grappleInstance.stickDirection = (grappleInstance.Pos - (grappleInstance.parent and grappleInstance.parent.Pos or grappleInstance.Pos)):Normalized()

                    stateChanged = true
                -- Check if the MO is an Actor and is physical (can be grappled)
                elseif MovableMan:IsActor(hitMO) and ToActor(hitMO):IsPhysical() then
                    grappleInstance.actionMode = 3 -- Grabbed MO
                    grappleInstance.Pos = hitMORayInfo.HitPos -- Snap grapple to hit point on MO
                    grappleInstance.apx[grappleInstance.currentSegments] = hitMORayInfo.HitPos.X -- Update anchor point
                    grappleInstance.apy[grappleInstance.currentSegments] = hitMORayInfo.HitPos.Y -- Update anchor point
                    grappleInstance.lastX[grappleInstance.currentSegments] = hitMORayInfo.HitPos.X
                    grappleInstance.lastY[grappleInstance.currentSegments] = hitMORayInfo.HitPos.Y
                    
                    grappleInstance.stickOffset = grappleInstance.Pos - hitMO.Pos -- Relative position on MO
                    grappleInstance.stickAngle = hitMO.RotAngle -- Initial angle of MO
                    -- grappleInstance.stickDirection = (grappleInstance.Pos - grappleInstance.parent.Pos):Normalized()
                    grappleInstance.stickDirection = (grappleInstance.Pos - (grappleInstance.parent and grappleInstance.parent.Pos or grappleInstance.Pos)):Normalized()

                    stateChanged = true
                end
                -- If it's not a pinnable MO and not a physical Actor, it's ignored (e.g., a non-physical particle)
            end
        end
    end
    
    -- Actions to take if the state changed to an attached state.
    if stateChanged then
        -- Play sound before potential errors if parent.Pos is nil, though parent should be valid.
        if grappleInstance.stickSound then grappleInstance.stickSound:Play(grappleInstance.Pos) end
        
        -- Update line length to current distance upon sticking.
        if grappleInstance.parent and grappleInstance.parent.Pos then
            local distVec = grappleInstance.Pos - grappleInstance.parent.Pos
            grappleInstance.currentLineLength = math.floor(distVec.Magnitude)
        else
            -- Fallback if parent or parent.Pos is nil. This indicates a deeper issue elsewhere.
            -- Setting to a large portion of maxLineLength as a temporary measure.
            grappleInstance.currentLineLength = grappleInstance.maxLineLength * 0.9 
        end
        -- Ensure currentLineLength is within valid bounds immediately after calculating.
        grappleInstance.currentLineLength = math.max(10, math.min(grappleInstance.currentLineLength, grappleInstance.maxLineLength))

        grappleInstance.setLineLength = grappleInstance.currentLineLength
        grappleInstance.Vel = Vector(0,0) -- Stop the hook's independent movement.
        grappleInstance.PinStrength = 1000 -- Make it "stick" firmly.
        grappleInstance.Frame = 1 -- Change sprite frame to "stuck" appearance if applicable.
        
        grappleInstance.canRelease = true -- Now that it's stuck, player can choose to release it.
        grappleInstance.limitReached = (grappleInstance.currentLineLength >= grappleInstance.maxLineLength - 0.1)
        grappleInstance.ropePhysicsInitialized = false -- May need re-init for rope physics with new anchor.
    end
    
    return stateChanged
end

--[[
  Handles logic when the rope reaches its maximum allowed length.
  This is mostly for effects like sound, as the actual length constraint is handled by RopePhysics.
  @param grappleInstance The grapple instance.
  @return True if the limit was newly reached this frame, false otherwise.
]]
function RopeStateManager.checkLengthLimit(grappleInstance)
    -- This function's primary role is now for triggering effects when the length limit is hit.
    -- The actual physics of stopping at max length is handled in Grapple.lua (for flight)
    -- and RopePhysics.applyRopeConstraints (for attached states).

    local effectivelyAtMax = false
    if grappleInstance.actionMode == 1 then -- Flying
        effectivelyAtMax = (grappleInstance.lineLength >= grappleInstance.maxShootDistance - 0.1)
    else -- Attached
        effectivelyAtMax = (grappleInstance.currentLineLength >= grappleInstance.maxLineLength - 0.1)
    end

    if effectivelyAtMax then
        if not grappleInstance.limitReached then -- If it wasn't at limit last frame
            grappleInstance.limitReached = true
            if grappleInstance.clickSound and grappleInstance.parent and grappleInstance.parent.Pos then
                grappleInstance.clickSound:Play(grappleInstance.parent.Pos)
            end
            return true -- Newly reached limit
        end
    else
        grappleInstance.limitReached = false
    end
    return false -- Not newly at limit, or not at limit.
end

--[[
  Applies effects for "stretch mode" (currently disabled by default in Grapple.lua).
  If enabled, this would typically retract the hook.
  @param grappleInstance The grapple instance.
]]
function RopeStateManager.applyStretchMode(grappleInstance)
    if not grappleInstance.stretchMode or not grappleInstance.parent or not grappleInstance.parent.Pos then return end
    
    if grappleInstance.actionMode == 1 and grappleInstance.lineVec then -- Flying
        -- Example: Gradually retract the hook.
        local pullForceFactor = (grappleInstance.stretchPullRatio or 0.05) * 0.5
        local pullMagnitude = math.sqrt(grappleInstance.lineLength or 0) * pullForceFactor
        
        grappleInstance.Vel = grappleInstance.Vel - grappleInstance.lineVec:SetMagnitude(pullMagnitude)
    end
end


--[[
  Helper function to get the effective target MO, considering root parents.
  @param grappleInstance The grapple instance.
  @return The effective target MO, or nil.
]]
function RopeStateManager.getEffectiveTarget(grappleInstance)
    if not grappleInstance or not grappleInstance.target or grappleInstance.target.ID == rte.NoMOID then
        return nil
    end

    local currentTarget = grappleInstance.target
    -- If the direct hit target is part of a larger entity (e.g., a limb of an actor),
    -- try to use its root parent as the effective target, IF the root is "attachable" (conceptual).
    -- For now, we just get the root parent if it's different.
    if currentTarget.RootID and currentTarget.ID ~= currentTarget.RootID then
        local rootParent = MovableMan:GetMOFromID(currentTarget.RootID)
        if rootParent and rootParent.ID ~= rte.NoMOID then
            -- Add a check here if certain MO types shouldn't be "grabbed" by their root
            -- e.g., if IsAttachable(rootParent) then effective_target = rootParent end
            -- For now, always use root if available.
            return rootParent
        end
    end
    return currentTarget -- Return the original target if no valid root parent or same as root.
end


-- The following physics application functions (applyTerrainPullPhysics, applyMOPullPhysics)
-- are complex and were part of a system that applied direct forces.
-- In a pure Verlet constraint system (as aimed for in RopePhysics.lua),
-- these direct force applications can conflict or become redundant if the constraints
-- are correctly managing positions and by extension, velocities.
-- They are kept for reference or if a hybrid model is intended, but their direct usage
-- should be carefully considered alongside the constraint-based physics.
-- If RopePhysics.applyRopeConstraints correctly handles player/MO movement due to rope tension,
-- these functions might only be needed for secondary effects or very specific scenarios.

--[[
  Applies physics forces when the grapple is attached to terrain.
  (Primarily for a force-based system, review if needed with Verlet constraints)
  @param grappleInstance The grapple instance.
  @return True if the rope should break from this interaction, false otherwise.
]]
function RopeStateManager.applyTerrainPullPhysics(grappleInstance)
    if grappleInstance.actionMode ~= 2 or not grappleInstance.parent then return false end
    
    -- If RopePhysics.applyRopeConstraints provides tension force/direction, use that.
    if grappleInstance.ropeTensionForce and grappleInstance.ropeTensionDirection and grappleInstance.parent.AddForce then
        local actor = grappleInstance.parent
        local raw_force_magnitude = grappleInstance.ropeTensionForce
        local force_direction = grappleInstance.ropeTensionDirection -- Should be towards the hook point

        -- Apply actor protection/scaling to this force
        -- This is a simplified protection; a more detailed one would consider mass, velocity, health.
        local safe_force_magnitude = math.min(raw_force_magnitude, (actor.Mass or 10) * 0.5) -- Cap force based on mass
        
        local final_force_vector = force_direction * safe_force_magnitude
        actor:AddForce(final_force_vector) -- AddForce at center of mass
        
        -- No breaking logic here, as RopePhysics handles breaking by stretch.
        return false 
    end
    
    -- Fallback or alternative spring logic (if not using tension from constraints directly for forces)
    -- This section would be active if grappleInstance.ropeTensionForce is nil.
    -- ... (original complex spring logic could be here) ...
    -- However, this is likely to conflict with a pure constraint system.
    
    return false -- Default: no break from this function.
end

--[[
  Applies physics forces when the grapple is attached to a Movable Object.
  (Primarily for a force-based system, review if needed with Verlet constraints)
  @param grappleInstance The grapple instance.
  @return True if the rope should break, false otherwise.
]]
function RopeStateManager.applyMOPullPhysics(grappleInstance)
    if grappleInstance.actionMode ~= 3 or not grappleInstance.target or grappleInstance.target.ID == rte.NoMOID or not grappleInstance.parent then
        return false -- Or true if target is lost, to signal unhook.
    end

    local effective_target = RopeStateManager.getEffectiveTarget(grappleInstance)
    if not effective_target or effective_target.ID == rte.NoMOID then
        return true -- Signal unhook.
    end

    -- Update hook's visual position to stick to the target MO.
    if effective_target.Pos and grappleInstance.stickPosition then
        local rotatedStickPos = Vector(grappleInstance.stickPosition.X, grappleInstance.stickPosition.Y)
        if effective_target.RotAngle and grappleInstance.stickRotation then
             rotatedStickPos:RadRotate(effective_target.RotAngle - grappleInstance.stickRotation)
        end
        grappleInstance.Pos = effective_target.Pos + rotatedStickPos
        if effective_target.RotAngle and grappleInstance.stickRotation and grappleInstance.stickDirection then
            grappleInstance.RotAngle = grappleInstance.stickDirection + (effective_target.RotAngle - grappleInstance.stickRotation)
        end
    end
    
    -- If RopePhysics.applyRopeConstraints provides tension, apply forces to player and target.
    if grappleInstance.ropeTensionForce and grappleInstance.ropeTensionDirection then
        local actor = grappleInstance.parent
        local raw_force_magnitude = grappleInstance.ropeTensionForce
        local force_direction_on_actor = grappleInstance.ropeTensionDirection -- Towards hook

        local total_mass = (actor.Mass or 10) + (effective_target.Mass or 10)
        local actor_force_share = (effective_target.Mass or 10) / total_mass
        local target_force_share = (actor.Mass or 10) / total_mass
        
        -- Simplified protection and force application
        local actor_pull_force = math.min(raw_force_magnitude * actor_force_share, (actor.Mass or 10) * 0.5)
        local target_pull_force = math.min(raw_force_magnitude * target_force_share, (effective_target.Mass or 10) * 0.8)

        if actor.AddForce then actor:AddForce(force_direction_on_actor * actor_pull_force) end
        if effective_target.AddForce then effective_target:AddForce(-force_direction_on_actor * target_pull_force) end
        
        return false -- No breaking from this function.
    end

    -- Fallback or alternative spring logic for MOs...
    -- ... (original complex MO spring logic) ...
    -- Again, likely to conflict with pure constraint system.

    -- Check if target MO is destroyed or invalid.
    if not MovableMan:IsValid(effective_target) or effective_target.ToDelete then
        return true -- Signal to delete the hook.
    end
    
    return false -- Default: no break.
end


--[[
  Determines if the grapple can be released by the player.
  @param grappleInstance The grapple instance.
  @return True if releasable, false otherwise.
]]
function RopeStateManager.canReleaseGrapple(grappleInstance)
    -- The 'canRelease' flag is set to true in checkAttachmentCollisions when the hook sticks.
    -- It can be set to false if, for example, the hook is mid-flight or during a special animation.
    return grappleInstance.canRelease or false -- Default to false if nil.
end

return RopeStateManager
