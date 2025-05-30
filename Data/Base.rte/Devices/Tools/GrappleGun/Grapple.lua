-- filepath: /home/cretin/git/Cortex-Command-Community-Project/Data/Base.rte/Devices/Tools/GrappleGun/Grapple.lua
-- Load Modules
local RopePhysics = require("Base.rte.Devices.Tools.GrappleGun.Scripts.RopePhysics")
local RopeRenderer = require("Base.rte.Devices.Tools.GrappleGun.Scripts.RopeRenderer")
local RopeInputController = require("Base.rte.Devices.Tools.GrappleGun.Scripts.RopeInputController")
local RopeStateManager = require("Base.rte.Devices.Tools.GrappleGun.Scripts.RopeStateManager")

function Create(self)
    self.lastPos = self.Pos
    
    self.mapWrapsX = SceneMan.SceneWrapsX
    self.climbTimer = Timer()
    self.mouseClimbTimer = Timer()
    self.actionMode = 0    -- 0 = start, 1 = flying, 2 = grab terrain, 3 = grab MO
    self.climb = 0
    self.canRelease = false

    self.tapTimer = Timer()
    self.tapCounter = 0
    self.didTap = false
    self.canTap = false

    self.fireVel = 40      -- This immediately overwrites the .ini FireVel
    self.maxLineLength = 400   -- Shorter rope for faster gameplay
    self.setLineLength = 0
    self.lineStrength = 40 -- How much "force" the rope can take before breaking

    self.limitReached = false
    self.stretchMode = false   -- Alternative elastic pull mode a là Liero
    self.stretchPullRatio = 0.1
    self.pieSelection = 0  -- 0 is nothing, 1 is full retract, 2 is partial retract, 3 is partial extend, 4 is full extend

    self.climbDelay = 8    -- Faster climbing for shorter rope
    self.tapTime = 150     -- Maximum amount of time between tapping for claw to return
    self.tapAmount = 2     -- How many times to tap to bring back rope
    self.mouseClimbLength = 200    -- Adjusted for shorter rope
    self.climbInterval = 4.0       -- Faster retraction/extension
    self.autoClimbIntervalA = 5.0  -- Faster auto-climbing
    self.autoClimbIntervalB = 3.0  -- Faster auto-climbing

    self.stickSound = CreateSoundContainer("Grapple Gun Claw Stick", "Base.rte")
    self.clickSound = CreateSoundContainer("Grapple Gun Click", "Base.rte")
    self.returnSound = CreateSoundContainer("Grapple Gun Return", "Base.rte")

    -- Rope physics variables from VelvetGrapple
    self.currentLineLength = 0
    self.longestLineLength = 0
    self.cablespring = 0.15 -- VelvetGrapple constraint stiffness
    
    -- Dynamic rope segment calculation variables
    self.minSegments = 4   -- Minimum number of segments
    self.maxSegments = 50  -- Maximum number of segments
    self.segmentLength = 12 -- Target length per segment (increased for better performance)
    self.currentSegments = self.minSegments -- Current number of segments
    
    -- Verlet physics friction for stability
    self.usefriction = 0.99 -- Matches VelvetGrapple
    
    -- Mousewheel control variables
    self.shiftScrollSpeed = 8.0 -- Faster rope control with Shift+Mousewheel

    --ESTABLISH LINE
    self.apx = {}
    self.apy = {}
    self.lastX = {}
    self.lastY = {}
    
    local px = self.Pos.X
    local py = self.Pos.Y
    
    -- Initialize with minimum number of segments
    for i = 0, self.maxSegments do
        self.apx[i] = px
        self.apy[i] = py
        self.lastX[i] = px
        self.lastY[i] = py
    end
    
    self.lastX[self.minSegments] = px - self.Vel.X
    self.lastY[self.minSegments] = py - self.Vel.Y
    self.currentSegments = self.minSegments -- Start with minimum segments
    --slots 0 and currentSegments are ANCHOR POINTS

    --Find the parent gun that fired us
    for gun in MovableMan:GetMOsInRadius(self.Pos, 50) do
        if gun and gun.ClassName == "HDFirearm" and gun.PresetName == "Grapple Gun" and SceneMan:ShortestDistance(self.Pos, ToHDFirearm(gun).MuzzlePos, self.mapWrapsX):MagnitudeIsLessThan(5) then
            self.parentGun = ToHDFirearm(gun)
            self.parent = MovableMan:GetMOFromID(gun.RootID)
            if MovableMan:IsActor(self.parent) then
                self.parent = ToActor(self.parent)
                if IsAHuman(self.parent) then
                    self.parent = ToAHuman(self.parent)
                elseif IsACrab(self.parent) then
                    self.parent = ToACrab(self.parent)
                end

                self.Vel = (self.parent.Vel * 0.5) + Vector(self.fireVel, 0):RadRotate(self.parent:GetAimAngle(true))
                self.parentGun:RemoveNumberValue("GrappleMode")
                for part in self.parent.Attachables do
                    local radcheck = SceneMan:ShortestDistance(self.parent.Pos, part.Pos, self.mapWrapsX).Magnitude + part.Radius
                    if self.parentRadius == nil or radcheck > self.parentRadius then
                        self.parentRadius = radcheck
                    end
                end

                self.actionMode = 1
            end
            break
        end
    end

    if self.parentGun == nil then   -- Failed to find our gun, abort
        self.ToDelete = true
    end
end

function Update(self)
    if self.parent and IsMOSRotating(self.parent) and self.parent:HasObject("Grapple Gun") then
        local controller = self.parent:GetController()
        local player = controller and controller.Player or 0 -- Get player for drawing, fallback to 0
        local startPos = self.parent.Pos

        self.ToDelete = false
        self.ToSettle = false

        -- Make sure we have a minimum viable rope length to avoid issues
        if self.actionMode == 1 and self.currentLineLength < 1 then
            self.currentLineLength = math.max(1, SceneMan:ShortestDistance(self.parent.Pos, self.Pos, self.mapWrapsX).Magnitude)
        end
        
        -- Update line length when in flight
        if self.actionMode == 1 then
            -- Immediately update rope length based on actual hook position
            self.lineVec = SceneMan:ShortestDistance(self.parent.Pos, self.Pos, self.mapWrapsX)
            self.lineLength = self.lineVec.Magnitude
            self.currentLineLength = self.lineLength
            
            -- Update rope anchor points directly
            self.apx[0] = self.parent.Pos.X
            self.apy[0] = self.parent.Pos.Y
            self.apx[self.currentSegments] = self.Pos.X
            self.apy[self.currentSegments] = self.Pos.Y
        end
        
        -- Calculate optimal number of segments based on rope length using our module function
        local desiredSegments = RopePhysics.calculateOptimalSegments(self, math.max(1, self.currentLineLength))
        
        -- Resize rope if needed (don't resize on every minor change to avoid performance issues)
        if desiredSegments ~= self.currentSegments then
            -- Add some hysteresis to prevent frequent resizing at length boundaries
            if math.abs(desiredSegments - self.currentSegments) > 1 then
                RopePhysics.resizeRopeSegments(self, desiredSegments)
            end
        end
        
        -- Rope physics simulation using VelvetGrapple approach
        local cablelength = self.currentLineLength / math.max(1, self.currentSegments) -- Dynamic per-segment length
        
        -- Set anchor points and update physics for all segments
        local i = self.currentSegments
        -- HANDLE ALL LINE JOINTS (from n down to 0)
        while i > -1 do
            if i == 0 or (i == self.currentSegments and self.limitReached == false and (self.actionMode == 1 or self.actionMode > 1)) then
                -- Anchor points: 0 (player) and n (hook)
                if i == 0 then -- POINT 0: ANCHOR TO GUN
                    local usepos = self.parent.Pos
                    self.apx[i] = usepos.X
                    self.apy[i] = usepos.Y
                    self.lastX[i] = self.lastPos.X
                    self.lastY[i] = self.lastPos.Y
                else -- POINT n: ANCHOR TO GRAPPLE if IN FLIGHT
                    local usepos = self.Pos
                    self.apx[i] = usepos.X
                    self.apy[i] = usepos.Y
                    self.lastX[i] = usepos.X
                    self.lastY[i] = usepos.Y
                end
            else
                if not (i == self.currentSegments and self.actionMode == 2) then
                    -- CALCULATE BASIC PHYSICS
                    local accX = 0
                    local accY = 0.05

                    local velX = self.apx[i] - self.lastX[i]
                    local velY = self.apy[i] - self.lastY[i]

                    local ufriction = self.usefriction
                    if i == self.currentSegments then 
                        ufriction = 0.99
                        accY = 0.5
                    end

                    local nextX = (velX + accX) * ufriction
                    local nextY = (velY + accY) * ufriction

                    self.lastX[i] = self.apx[i]
                    self.lastY[i] = self.apy[i]

                    -- Use physics module for collision handling
                    RopePhysics.verletCollide(self, i, nextX, nextY)
                end
                
                if i == self.currentSegments and self.actionMode == 3 then
                    if self.target and self.target.ID ~= rte.NoMOID then
                        local target = self.target
                        if target.ID ~= target.RootID then
                            local mo = target:GetRootParent()
                            if mo.ID ~= rte.NoMOID and IsAttachable(target) then
                                target = mo
                            end
                        end

                        self.lastX[i] = self.apx[i]-target.Vel.X
                        self.lastY[i] = self.apy[i]-target.Vel.Y
                    else    -- Our MO has been destroyed, return hook
                        self.ToDelete = true
                    end
                end
            end
            
            -- Get optimized iteration count based on rope conditions
            local maxIterations = RopePhysics.optimizePhysicsIterations(self)

            i = i-1
        end

        -- Draw the rope using the renderer module
        RopeRenderer.drawRope(self, player)
        
        -- Show rope tension indicator when necessary
        RopeRenderer.showTensionIndicator(self, player)
        
        -- Show debug info temporarily to help diagnose issues
        local debugPos = self.Pos + Vector(10, -30) -- Define a position for debug text
        RopeRenderer.showDebugInfo(self, player, debugPos)

        -- Update hook position based on rope physics
        if self.actionMode == 1 and self.limitReached == true then
            self.Pos.X = self.apx[self.currentSegments]
            self.Pos.Y = self.apy[self.currentSegments]
        end

        self.lineVec = SceneMan:ShortestDistance(self.parent.Pos, self.Pos, self.mapWrapsX)
        self.lineLength = self.lineVec.Magnitude

        -- Update current line length
        if self.actionMode == 1 and self.limitReached == false then 
            -- Always update rope length while in flight
            self.currentLineLength = self.lineLength 
        end

        -- Check if line length exceeds maximum
        RopeStateManager.checkLineLengthUpdate(self)

        if self.parentGun and self.parentGun.ID ~= rte.NoMOID then
            self.parent = ToMOSRotating(MovableMan:GetMOFromID(self.parentGun.RootID))

            if self.parentGun.Magazine then
                self.parentGun.Magazine.Scale = 0
            end

            startPos = self.parentGun.Pos
            local flipAng = self.parent.HFlipped and 3.14 or 0
            self.parentGun.RotAngle = self.lineVec.AbsRadAngle + flipAng

            -- Handle pie menu selection
            if RopeInputController.handlePieMenuSelection(self) then
                self.ToDelete = true
            end

            -- Handle unhooking from firing
            if self.parentGun.FiredFrame then
                if self.actionMode == 1 then
                    self.ToDelete = true
                else
                    self.canRelease = true
                end
            end

            if self.parentGun.FiredFrame and self.canRelease and 
               (Vector(self.parentGun.Vel.X, self.parentGun.Vel.Y) ~= Vector(0, -1) or 
                self.parentGun:IsActivated()) then
                self.ToDelete = true
            end
        end

        if IsAHuman(self.parent) then
            self.parent = ToAHuman(self.parent)
            -- We now have a user that controls this grapple (controller already obtained above)
            -- Point the gun towards the hook if our user is holding it
            if (self.parentGun and self.parentGun.ID ~= rte.NoMOID) and (self.parentGun:GetRootParent().ID == self.parent.ID) then
                if self.parent:IsPlayerControlled() then
                    if controller:IsState(Controller.WEAPON_RELOAD) then
                        -- Only unhook with R if holding the Grapple Gun
                        if self.parent.EquippedItem and self.parentGun and self.parent.EquippedItem.ID == self.parentGun.ID then
                            self.ToDelete = true
                        end
                    end
                    
                    if self.parentGun.Magazine then
                        self.parentGun.Magazine.RoundCount = 0
                    end
                end
                
                local offset = Vector(self.lineLength, 0):RadRotate(self.parent.FlipFactor * (self.lineVec.AbsRadAngle - self.parent:GetAimAngle(true)))
                self.parentGun.StanceOffset = offset
            end
        end
        
        -- Add crank sound if not already present
        if MovableMan:IsParticle(self.crankSound) then
            self.crankSound.ToDelete = false
            self.crankSound.ToSettle = false
            self.crankSound.Pos = startPos
            if self.lastSetLineLength ~= self.currentLineLength then
                self.crankSound:EnableEmission(true)
            else
                self.crankSound:EnableEmission(false)
            end
        else
            self.crankSound = CreateAEmitter("Grapple Gun Sound Crank")
            self.crankSound.Pos = startPos
            MovableMan:AddParticle(self.crankSound)
        end

        self.lastSetLineLength = self.currentLineLength

        if self.actionMode == 1 then    -- Hook is in flight
            -- Apply stretch mode physics for retracting the hook
            RopeStateManager.applyStretchMode(self)
            
            -- Check for collisions and update state if needed
            RopeStateManager.checkAttachmentCollisions(self)
            
            -- Check for length limit and apply physics if needed
            RopeStateManager.checkLengthLimit(self)
        elseif self.actionMode > 1 then -- Hook has stuck
            -- Update rope anchor point for hook position
            self.apx[self.currentSegments] = self.Pos.X
            self.apy[self.currentSegments] = self.Pos.Y

            -- Actor mass and velocity affect pull strength negatively, rope length affects positively
            self.parentForces = 1 + (self.parent.Vel.Magnitude * 10 + self.parent.Mass)/(1 + self.lineLength)
            
            -- Check if there is terrain between the hook and the user
            local terrVector = Vector()
            local terrCheck = false
            if self.parentRadius ~= nil then
                terrCheck = SceneMan:CastStrengthRay(self.parent.Pos, 
                                                     self.lineVec:SetMagnitude(self.parentRadius), 
                                                     0, terrVector, 2, rte.airID, self.mapWrapsX)
            end

            -- Process automatic retraction
            RopeInputController.handleAutoRetraction(self, terrCheck)

            -- Process input based climbing
            RopeInputController.handleRopePulling(self)
            
            -- Process terrain pull physics
            if self.actionMode == 2 and RopeStateManager.applyTerrainPullPhysics(self) then
                self.ToDelete = true
            end
            
            -- Process MO pull physics
            if self.actionMode == 3 and RopeStateManager.applyMOPullPhysics(self) then
                self.ToDelete = true
            end
        end
        
        -- Check if we should unhook via double-tap mechanic
        if RopeInputController.handleTapDetection(self, controller) then
            self.ToDelete = true
        end
        
        -- Check if we should unhook via R key press
        if RopeInputController.handleReloadKeyUnhook(self, controller) then
            self.ToDelete = true
        end

        -- Special handling for hook deletion - show magazine and play sound
        if self.ToDelete == true then
            if self.parentGun and self.parentGun.Magazine then
                -- Show the magazine as if the hook is being retracted
                local drawPos = self.parent.Pos + (self.lineVec * 0.5)
                self.parentGun.Magazine.Pos = drawPos
                self.parentGun.Magazine.Scale = 1
                self.parentGun.Magazine.Frame = 0
            end
            self.returnSound:Play(self.parent.Pos)
        end

    elseif self.parentGun and IsHDFirearm(self.parentGun) then
        self.parent = self.parentGun
    else
        self.ToDelete = true
    end

    if self.parentGun then
        self.lastPos = self.parent.Pos
    end
end

function Destroy(self)
    if MovableMan:IsParticle(self.crankSound) then
        self.crankSound.ToDelete = true
    end
    
    if self.parentGun and self.parentGun.ID ~= rte.NoMOID then
        self.parentGun.HUDVisible = true
        self.parentGun:RemoveNumberValue("GrappleMode")
    end
end