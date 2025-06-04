---@diagnostic disable: undefined-global
-- filepath: /Cortex-Command-Community-Project/Data/Base.rte/Devices/Tools/GrappleGun/Grapple.lua
-- Main logic for the grapple claw MovableObject.

-- Load Modules
local RopePhysics = require("Devices.Tools.GrappleGun.Scripts.RopePhysics")
local RopeRenderer = require("Devices.Tools.GrappleGun.Scripts.RopeRenderer")
local RopeInputController = require("Devices.Tools.GrappleGun.Scripts.RopeInputController")
local RopeStateManager = require("Devices.Tools.GrappleGun.Scripts.RopeStateManager")
function Create(self)
    self.lastPos = self.Pos
    
    self.mapWrapsX = SceneMan.SceneWrapsX
    self.climbTimer = Timer()
    self.mouseClimbTimer = Timer()
    self.tapTimer = Timer() -- Initialize tapTimer
    
    -- Initialize state using the state manager. This sets self.actionMode = 0.
    RopeStateManager.initState(self)
    -- self.initializationOk = true -- This flag is effectively replaced by checking self.actionMode == 0 in Update.

    -- Core grapple properties
    self.fireVel = 40      -- Initial velocity of the hook. Overwrites .ini FireVel.
    self.hookRadius = 360   -- Reduced from 360 for more precise parent finding

    self.maxLineLength = 1000   -- Maximum allowed length of the rope.
    self.maxShootDistance = self.maxLineLength * 0.95 -- Hook will detach if it travels further than this before sticking.
    self.setLineLength = 0 -- Target length set by input/logic.
    self.lineStrength = 10000 -- Force threshold for breaking (effectively unbreakable).

    self.limitReached = false -- True if the rope has reached its maxLineLength.
    self.stretchMode = false   -- Disabled for rigid rope behavior.
    self.stretchPullRatio = 0.0 -- No stretching for rigid rope.
    self.pieSelection = 0  -- Current pie menu selection (0: none, 1: full retract, etc.).


    -- Timing and interval properties for rope actions
    self.climbDelay = 8    -- Delay between climb ticks.
    self.tapTime = 150     -- Max time between taps for double-tap unhook.
    self.tapAmount = 2     -- Number of taps required for unhook.
    self.tapCounter = 0    -- Current tap count for multi-tap detection.
    self.canTap = false    -- Flag to register the first tap in a sequence.
    self.mouseClimbLength = 200    -- Duration mouse scroll input is considered active.
    self.climbInterval = 4.0       -- Amount rope length changes per climb tick.
    self.autoClimbIntervalA = 5.0  -- Auto-retract speed (primary).
    self.autoClimbIntervalB = 3.0  -- Auto-extend speed (secondary, e.g., from pie menu).

    -- Sound effects
    self.stickSound = CreateSoundContainer("Grapple Gun Claw Stick", "Base.rte")
    self.clickSound = CreateSoundContainer("Grapple Gun Click", "Base.rte")
    self.returnSound = CreateSoundContainer("Grapple Gun Return", "Base.rte")
    self.crankSoundInstance = nil

    -- Rope physics variables
    self.currentLineLength = 0 
    self.cablespring = 0.01 
    
    self.minSegments = 1
    self.maxSegments = 1000
    self.segmentLength = 6
    self.currentSegments = self.minSegments
    
    self.shiftScrollSpeed = 1.0

    self.apx = {} 
    self.apy = {} 
    self.lastX = {}
    self.lastY = {}
    
    local px = self.Pos.X
    local py = self.Pos.Y
    
    for i = 0, self.maxSegments do
        self.apx[i] = px
        self.apy[i] = py
        self.lastX[i] = px
        self.lastY[i] = py
    end
    
    self.currentSegments = self.minSegments

    -- Parent gun, parent actor, and related properties (Vel, anchor points, parentRadius)
    -- will be determined and set in the first Update call.
    -- No self.ToDelete = true will be set in Create.
    
    -- Add these new flags:
    self.shouldUnhook = false      -- Flag set by gun to signal unhook
    -- self.reloadKeyPressed = false  -- Track R key state to prevent spam
    
    -- Keep only the tap detection variables:
    self.tapCounter = 0
    self.canTap = false
    self.tapTime = 150
    self.tapAmount = 2
    self.tapTimer = Timer()
end

function Update(self)
    if self.ToDelete then return end

    -- First-time setup: Find parent, initialize velocity, anchor points, etc.
    if self.actionMode == 0 then
        local foundAndValidParent = false
        for gun_mo in MovableMan:GetMOsInRadius(self.Pos, self.hookRadius) do
            if gun_mo and gun_mo.ClassName == "HDFirearm" and gun_mo.PresetName == "Grapple Gun" then
                local hdfGun = ToHDFirearm(gun_mo)
                if hdfGun and SceneMan:ShortestDistance(self.Pos, hdfGun.MuzzlePos, self.mapWrapsX):MagnitudeIsLessThan(20) then
                    self.parentGun = hdfGun
                    local rootParentMO = MovableMan:GetMOFromID(hdfGun.RootID)
                    if rootParentMO and MovableMan:IsActor(rootParentMO) then
                        self.parent = ToActor(rootParentMO)
                        self.apx[0] = self.parent.Pos.X
                        self.apy[0] = self.parent.Pos.Y
                        self.lastX[0] = self.parent.Pos.X - (self.parent.Vel.X or 0)
                        self.lastY[0] = self.parent.Pos.Y - (self.parent.Vel.Y or 0)

                        -- Set initial velocity of the hook based on parent's aim and velocity
                        local aimAngle = self.parent:GetAimAngle(true)
                        self.Vel = (self.parent.Vel or Vector(0,0)) + Vector(self.fireVel, 0):RadRotate(aimAngle)
                        
                        -- Initialize hook's lastX/Y for its initial trajectory
                        self.lastX[self.currentSegments] = self.Pos.X - self.Vel.X 
                        self.lastY[self.currentSegments] = self.Pos.Y - self.Vel.Y

                        if self.parentGun then -- Should be valid here
                            self.parentGun:RemoveNumberValue("GrappleMode") -- Clear any previous mode
                        end

                        -- Determine parent's effective radius for terrain checks
                        self.parentRadius = 5 -- Default radius
                        if self.parent.Attachables and type(self.parent.Attachables) == "table" then
                            for _, part in ipairs(self.parent.Attachables) do
                                if part and part.Pos and part.Radius then
                                    local radcheck = SceneMan:ShortestDistance(self.parent.Pos, part.Pos, self.mapWrapsX).Magnitude + part.Radius
                                    if self.parentRadius == nil or radcheck > self.parentRadius then
                                        self.parentRadius = radcheck
                                    end
                                end
                            end
                        end
                        self.actionMode = 1 -- Set to flying, initialization successful
                        
                        -- Initialize rope segments for display during flight with proper physics
                        -- First segment is at the shooter's position, last segment is at hook position
                        -- Use more segments for better physics and visuals
                        self.currentSegments = 4 -- Start with more segments for better physics during flight
                        self.apx[0] = self.parent.Pos.X
                        self.apy[0] = self.parent.Pos.Y
                        self.lastX[0] = self.parent.Pos.X - (self.parent.Vel.X or 0)
                        self.lastY[0] = self.parent.Pos.Y - (self.parent.Vel.Y or 0)
                        
                        -- Initialize the hook segment
                        self.apx[self.currentSegments] = self.Pos.X
                        self.apy[self.currentSegments] = self.Pos.Y
                        self.lastX[self.currentSegments] = self.Pos.X - (self.Vel.X or 0)
                        self.lastY[self.currentSegments] = self.Pos.Y - (self.Vel.Y or 0)
                        
                        -- Initialize intermediate segments with a natural drape
                        for i = 1, self.currentSegments - 1 do
                            local t = i / self.currentSegments
                            self.apx[i] = self.parent.Pos.X + t * (self.Pos.X - self.parent.Pos.X)
                            self.apy[i] = self.parent.Pos.Y + t * (self.Pos.Y - self.parent.Pos.Y)
                            -- Add slight droop for natural look
                            self.apy[i] = self.apy[i] + math.sin(t * math.pi) * 2
                            -- Initialize lastX/Y with small velocity matching the overall direction
                            self.lastX[i] = self.apx[i] - (self.Vel.X or 0) * 0.2
                            self.lastY[i] = self.apy[i] - (self.Vel.Y or 0) * 0.2
                        end
                        
                        foundAndValidParent = true
                    end -- if MovableMan:IsActor(rootParentMO)
                end -- if hdfGun and distance check
            end -- if gun_mo is grapple gun
        end -- for gun_mo

        if not foundAndValidParent then
            self.ToDelete = true
            return -- Exit Update if initialization failed
        end
        -- If we reach here, initialization was successful, self.actionMode = 1
    end

    -- If ToDelete was set during initialization, or by other logic, exit.
    if self.ToDelete then return end

    -- Continuous validation checks for parent and gun
    -- self.parent should be an Actor if initialization succeeded and actionMode >= 1
    if not self.parent or self.parent.ID == rte.NoMOID then
        self.ToDelete = true
        return
    end
    
    local parentActor = self.parent -- self.parent is already an Actor type from the setup block
    
    if not self.parentGun or self.parentGun.ID == rte.NoMOID or not parentActor:HasObject("Grapple Gun") then
        self.ToDelete = true
        return
    end

    -- Standard update flags
    self.ToSettle = false -- Grapple claw should not settle

    -- Update player anchor point (segment 0)
    self.apx[0] = parentActor.Pos.X
    self.apy[0] = parentActor.Pos.Y
    self.lastX[0] = parentActor.Pos.X - (parentActor.Vel.X or 0)
    self.lastY[0] = parentActor.Pos.Y - (parentActor.Vel.Y or 0)

    -- Update hook anchor point (segment self.currentSegments)
    -- This depends on whether the hook is attached or flying
    if self.actionMode == 1 then -- Flying
        -- Hook position is determined by its own physics
        self.apx[self.currentSegments] = self.Pos.X
        self.apy[self.currentSegments] = self.Pos.Y
        -- Initialize lastX/Y for the hook end if not set
        if not self.lastX[self.currentSegments] then
            self.lastX[self.currentSegments] = self.Pos.X - (self.Vel.X or 0)
            self.lastY[self.currentSegments] = self.Pos.Y - (self.Vel.Y or 0)
        end
        
        -- Use full Verlet physics during flight, not just simple line positioning
        -- This ensures consistent rope behavior across all action modes
    elseif self.actionMode == 2 then -- Grabbed terrain
        -- Hook position is fixed where it grabbed
        self.Pos.X = self.apx[self.currentSegments] -- Ensure self.Pos matches anchor
        self.Pos.Y = self.apy[self.currentSegments]
        -- Velocity of the terrain anchor is zero
        self.lastX[self.currentSegments] = self.apx[self.currentSegments]
        self.lastY[self.currentSegments] = self.apy[self.currentSegments]
    elseif self.actionMode == 3 and self.target and self.target.ID ~= rte.NoMOID then -- Grabbed MO
        local effective_target = RopeStateManager.getEffectiveTarget(self)
        if effective_target and effective_target.ID ~= rte.NoMOID then
            self.Pos = effective_target.Pos
            self.apx[self.currentSegments] = effective_target.Pos.X
            self.apy[self.currentSegments] = effective_target.Pos.Y
            self.lastX[self.currentSegments] = effective_target.Pos.X - (effective_target.Vel.X or 0)
            self.lastY[self.currentSegments] = effective_target.Pos.Y - (effective_target.Vel.Y or 0)
        else
            -- Target lost or invalid, consider unhooking or reverting to terrain grab
            self.ToDelete = true -- Or change actionMode to 2 if it should stick to the last location
            return
        end
    end
    
    -- Calculate current actual distance between player and hook
    self.lineVec = SceneMan:ShortestDistance(parentActor.Pos, self.Pos, self.mapWrapsX)
    self.lineLength = self.lineVec.Magnitude -- This is the visual length

    -- State-dependent logic for currentLineLength (the physics length)
    if self.actionMode == 1 then -- Flying
        if self.lineLength >= self.maxShootDistance then
            if not self.limitReached then
                self.clickSound:Play(parentActor.Pos)
                self.limitReached = true
            end
            self.currentLineLength = self.maxShootDistance -- Physics length capped
            -- The RopePhysics.applyRopeConstraints will handle the "binding"
        else
            self.currentLineLength = self.lineLength -- Physics length matches visual
            self.limitReached = false
        end
        self.setLineLength = self.currentLineLength -- Keep setLineLength synchronized during flight
    else -- Attached (Terrain or MO)
        -- currentLineLength is controlled by input or auto-climbing, clamped.
        self.currentLineLength = math.max(10, math.min(self.currentLineLength, self.maxLineLength))
        self.setLineLength = self.currentLineLength -- Keep setLineLength synchronized
        -- limitReached is true if currentLineLength is at maxLineLength, false otherwise
        self.limitReached = (self.currentLineLength >= self.maxLineLength - 0.1) -- Small tolerance
    end

    -- Dynamic rope segment calculation
    local desiredSegments = RopePhysics.calculateOptimalSegments(self, math.max(1, self.currentLineLength))
    
   -- In flying mode, ensure we have enough intermediate segments for proper Verlet physics
    if self.actionMode == 1 then
        -- For short distances, use at least 6 segments
        -- For longer distances, use enough segments for proper rope physics
        -- This higher segment count is essential for proper Verlet physics simulation
        local minSegmentsForFlight = math.max(6, math.floor(self.lineLength / 25))
        desiredSegments = math.max(minSegmentsForFlight, desiredSegments)
    end
    
    -- Update segments if needed, with reduced hysteresis threshold for flight mode
    -- This ensures smoother transitions as the rope extends
    local segmentUpdateThreshold = self.actionMode == 1 and 1 or 2
    if desiredSegments ~= self.currentSegments and math.abs(desiredSegments - self.currentSegments) >= segmentUpdateThreshold then
        RopePhysics.resizeRopeSegments(self, desiredSegments)
    end

    -- Core rope physics simulation
    RopePhysics.updateRopePhysics(self, parentActor.Pos, self.Pos, self.currentLineLength)
    
    -- Check for hook attachment collisions (only when flying)
    if self.actionMode == 1 then
        local stateChanged = RopeStateManager.checkAttachmentCollisions(self)
        if stateChanged then
            -- Rope physics may need re-initialization after attachment
            self.ropePhysicsInitialized = false
        end
    end
    
    -- Apply constraints and check for breaking
    local ropeBreaks = RopePhysics.applyRopeConstraints(self, self.currentLineLength)
    if ropeBreaks or self.shouldBreak then -- self.shouldBreak can be set by other logic
        self.ToDelete = true
        if parentActor:IsPlayerControlled() then
            FrameMan:SetScreenScrollSpeed(10.0)
            if self.returnSound then self.returnSound:Play(parentActor.Pos) end
        end
        return -- Exit update if rope breaks
    end

    -- Update hook's own position if it's not attached to an MO
    -- If attached to terrain (actionMode 2), its position is already fixed by its anchor point.
    -- If flying (actionMode 1), its position is determined by its Verlet integration + constraints.
    if self.actionMode == 1 then
         -- The hook's self.Pos is updated by its own physics, but constraints might adjust segment end
        self.Pos.X = self.apx[self.currentSegments]
        self.Pos.Y = self.apy[self.currentSegments]
    end

    -- Aim the gun
    if self.parentGun and self.parentGun.ID ~= rte.NoMOID then
        local flipAng = parentActor.HFlipped and math.pi or 0
        self.parentGun.RotAngle = self.lineVec.AbsRadAngle + flipAng
        if MovableMan:IsParticle(self.parentGun.Magazine) then -- Check if Magazine is a particle
             ToMOSParticle(self.parentGun.Magazine).Scale = 0 -- Hide magazine when grapple is active
        end

        -- Handle unhooking from firing the gun again
        if self.parentGun.FiredFrame then
            if self.actionMode == 1 then -- If flying, just delete
                self.ToDelete = true
            elseif self.actionMode > 1 then -- If attached, mark as ready to release
                self.canRelease = true 
            end
        end
        -- If marked ready and gun is fired again (or activated for some guns)
        if self.canRelease and self.parentGun.FiredFrame and 
           (self.parentGun.Vel.Y ~= -1 or self.parentGun:IsActivated()) then -- Original logic for release condition
            self.ToDelete = true
        end
    end
    
    -- Player-specific controls and unhooking mechanisms
    if IsAHuman(parentActor) or IsACrab(parentActor) then
        if parentActor:IsPlayerControlled() then
            local controller = self.parent:GetController()
            if controller then
                -- ONLY use RopeInputController for all input handling
                
                -- 1. R key to unhook (when holding gun)
                if RopeInputController.handleReloadKeyUnhook(self, controller) then
                    print("Unhooking via R key!")
                    self.ToDelete = true
                    return
                end
                
                -- 2. Double crouch-tap to unhook (when NOT holding gun)
                if RopeInputController.handleTapDetection(self, controller) then
                    print("Unhooking via double crouch!")
                    self.ToDelete = true
                    return
                end
                
                -- 3. Pie menu unhook
                if RopeInputController.handlePieMenuSelection(self) then
                    print("Unhooking via pie menu!")
                    self.ToDelete = true
                    return
                end
                
                -- 4. Other rope controls
                RopeInputController.handleRopePulling(self)
                RopeInputController.handleAutoRetraction(self, false)
            end
        end
        
        -- Gun stance offset when holding the gun
        if self.parentGun and self.parentGun.RootID == parentActor.ID then
             if MovableMan:IsParticle(self.parentGun.Magazine) then -- Check if Magazine is a particle
                ToMOSParticle(self.parentGun.Magazine).RoundCount = 0 -- Visually empty
             end
            local offsetAngle = parentActor.FlipFactor * (self.lineVec.AbsRadAngle - parentActor:GetAimAngle(true))
            self.parentGun.StanceOffset = Vector(self.lineLength, 0):RadRotate(offsetAngle)
        end
    end
    
    -- Render the rope
    RopeRenderer.drawRope(self, player)

    -- Final deletion check and cleanup
    if self.ToDelete then
        if self.parentGun and MovableMan:IsParticle(self.parentGun.Magazine) then
            local mag = ToMOSParticle(self.parentGun.Magazine)
            -- Show magazine briefly as if hook is retracting
            mag.Pos = parentActor.Pos + (self.lineVec * 0.5)
            mag.Scale = 1
            mag.Frame = 0 -- Assuming frame 0 is the visible magazine
        end
        if self.returnSound then self.returnSound:Play(parentActor.Pos) end
    end
end

function Destroy(self)
    if self.crankSoundInstance and not self.crankSoundInstance.ToDelete then
        self.crankSoundInstance.ToDelete = true
    end
    
    -- Clean up references on the parent gun
    if self.parentGun and self.parentGun.ID ~= rte.NoMOID then
        self.parentGun.HUDVisible = true -- Assuming it was hidden
        self.parentGun:RemoveNumberValue("GrappleMode")
        -- Reset stance offset if it was modified
        self.parentGun.StanceOffset = Vector(0,0) 
        if MovableMan:IsParticle(self.parentGun.Magazine) then
            ToMOSParticle(self.parentGun.Magazine).Scale = 1 -- Ensure magazine is visible
        end
    end
end
