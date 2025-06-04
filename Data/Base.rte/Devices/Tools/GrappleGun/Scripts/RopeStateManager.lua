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
    
    -- Much stricter collision detection with minimal ranges
    local baseRayLength = math.max(1, (grappleInstance.Diameter or 4) * 0.2) -- Reduced from 0.5 to 0.2
    local velocityComponent = math.min(1, (grappleInstance.Vel and grappleInstance.Vel.Magnitude or 0) * 0.1) -- Reduced from 0.2 to 0.1
    local rayLength = baseRayLength + velocityComponent
    rayLength = math.max(1, rayLength) -- Reduced minimum from 2 to 1

    local rayDirection = Vector(1,0) -- Default direction
    -- Require higher velocity threshold for directional casting
    if grappleInstance.Vel and grappleInstance.Vel.Magnitude and grappleInstance.Vel.Magnitude > 0.1 then -- Increased from 0.005 to 0.1
        local mag = grappleInstance.Vel.Magnitude
        if mag ~= 0 then
            rayDirection = Vector(grappleInstance.Vel.X / mag, grappleInstance.Vel.Y / mag)
        end
    end
    
    -- Primary ray (much shorter and more precise)
    local collisionRay = rayDirection * rayLength
    local hitPoint = Vector()
    
    -- Secondary ray (extremely short)  
    local secondaryRayLength = math.max(0.5, baseRayLength * 0.1) -- Reduced from 0.3 to 0.1
    local secondaryHitPoint = Vector()
    
    -- Close-range radius (extremely minimal)
    local closeRangeRadius = math.max(0.5, (grappleInstance.Diameter or 4) * 0.1) -- Reduced from 0.3 to 0.1
    local terrainHit = false

    -- 1. Check for Terrain Collision (primary ray) - require much higher strength
    local terrainHit = SceneMan:CastStrengthRay(grappleInstance.Pos, collisionRay, 15, hitPoint, 0, rte.airID, grappleInstance.mapWrapsX) -- Increased from 5 to 15
    
    -- 2. Secondary terrain check - even higher strength requirement
    if not terrainHit and grappleInstance.Vel and grappleInstance.Vel.Magnitude < 0.5 then -- Reduced from 1 to 0.5
        terrainHit = SceneMan:CastStrengthRay(grappleInstance.Pos, rayDirection * secondaryRayLength, 20, secondaryHitPoint, 0, rte.airID, grappleInstance.mapWrapsX) -- Increased from 8 to 20
        if terrainHit then
            hitPoint = secondaryHitPoint
        end
    end
    
    -- 3. Close-range terrain collision - extremely high strength requirement
    if not terrainHit and (not grappleInstance.Vel or grappleInstance.Vel.Magnitude < 0.1) then -- Reduced from 0.5 to 0.1
        -- Only check 1 direction instead of 2 - just forward
        local checkDir = rayDirection * closeRangeRadius
        local closeRangeHit = Vector()
        -- Require very high terrain strength for close-range detection
        if SceneMan:CastStrengthRay(grappleInstance.Pos, checkDir, 25, closeRangeHit, 0, rte.airID, grappleInstance.mapWrapsX) then -- Increased from 10 to 25
            hitPoint = closeRangeHit
            terrainHit = true
        end
    end
    
    -- Additional validation: Ensure hit point is actually close to grapple position
    if terrainHit then
        local distanceToHit = SceneMan:ShortestDistance(grappleInstance.Pos, hitPoint, grappleInstance.mapWrapsX).Magnitude
        if distanceToHit > rayLength * 1.1 then -- Allow only 10% tolerance
            terrainHit = false -- Reject if hit point is too far
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
        -- MO collision detection - also made stricter
        local hitMORayInfo = SceneMan:CastMORay(grappleInstance.Pos, collisionRay, 
                                            (grappleInstance.parent and grappleInstance.parent.ID or 0),
                                            -2, rte.airID, false, 0)
        
        -- Only try secondary MO ray if moving very slowly and primary failed
        if not (hitMORayInfo and type(hitMORayInfo) == "table" and hitMORayInfo.MOSPtr and hitMORayInfo.MOSPtr.ID ~= rte.NoMOID) then
            if grappleInstance.Vel and grappleInstance.Vel.Magnitude < 1 then -- Stricter velocity requirement
                hitMORayInfo = SceneMan:CastMORay(grappleInstance.Pos, rayDirection * secondaryRayLength, 
                                                    (grappleInstance.parent and grappleInstance.parent.ID or 0),
                                                    -2, rte.airID, false, 0)
            end
        end
        
        if hitMORayInfo and type(hitMORayInfo) == "table" and hitMORayInfo.MOSPtr and hitMORayInfo.MOSPtr.ID ~= rte.NoMOID then
            local hitMO = hitMORayInfo.MOSPtr
            
            -- Much stricter size filtering
            local minGrappableSize = 8 -- Increased from 3 to 8
            if hitMO.Diameter and hitMO.Diameter < minGrappableSize then
                hitMO = nil
                hitMORayInfo = nil
            end
            
            -- Additional validation: Ensure MO hit point is close enough
            if hitMO and hitMORayInfo.HitPos then
                local distanceToMOHit = SceneMan:ShortestDistance(grappleInstance.Pos, hitMORayInfo.HitPos, grappleInstance.mapWrapsX).Magnitude
                if distanceToMOHit > rayLength * 1.1 then -- Same 10% tolerance
                    hitMO = nil
                    hitMORayInfo = nil
                end
            end
            
            if hitMO and hitMORayInfo then
                grappleInstance.target = hitMO
                
                local isPinnedActor = MovableMan:IsActor(hitMO) and ToActor(hitMO):IsPinned()
                
                if isPinnedActor or (not MovableMan:IsActor(hitMO) and hitMO.Material and hitMO.Material.Mass == 0) then 
                    grappleInstance.actionMode = 2
                    grappleInstance.Pos = hitMORayInfo.HitPos
                    grappleInstance.apx[grappleInstance.currentSegments] = hitMORayInfo.HitPos.X
                    grappleInstance.apy[grappleInstance.currentSegments] = hitMORayInfo.HitPos.Y
                    grappleInstance.lastX[grappleInstance.currentSegments] = hitMORayInfo.HitPos.X
                    grappleInstance.lastY[grappleInstance.currentSegments] = hitMORayInfo.HitPos.Y
                    grappleInstance.stickDirection = (grappleInstance.Pos - (grappleInstance.parent and grappleInstance.parent.Pos or grappleInstance.Pos)):Normalized()
                    stateChanged = true
                elseif MovableMan:IsActor(hitMO) and ToActor(hitMO):IsPhysical() then
                    -- Additional validation for actor grappling - require minimum mass
                    local minGrappableActorMass = 15 -- Minimum mass for grappable actors
                    if hitMO.Mass and hitMO.Mass >= minGrappableActorMass then
                        grappleInstance.actionMode = 3
                        grappleInstance.Pos = hitMORayInfo.HitPos
                        grappleInstance.apx[grappleInstance.currentSegments] = hitMORayInfo.HitPos.X
                        grappleInstance.apy[grappleInstance.currentSegments] = hitMORayInfo.HitPos.Y
                        grappleInstance.lastX[grappleInstance.currentSegments] = hitMORayInfo.HitPos.X
                        grappleInstance.lastY[grappleInstance.currentSegments] = hitMORayInfo.HitPos.Y
                        
                        grappleInstance.stickOffset = grappleInstance.Pos - hitMO.Pos
                        grappleInstance.stickAngle = hitMO.RotAngle
                        grappleInstance.stickDirection = (grappleInstance.Pos - (grappleInstance.parent and grappleInstance.parent.Pos or grappleInstance.Pos)):Normalized()
                        stateChanged = true
                    end
                end
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
