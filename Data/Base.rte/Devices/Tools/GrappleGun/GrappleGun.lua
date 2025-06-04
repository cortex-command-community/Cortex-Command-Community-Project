---@diagnostic disable: undefined-global
-- Localize Cortex Command globals
local Timer = Timer
local PresetMan = PresetMan
local CreateMOSRotating = CreateMOSRotating
local IsActor = IsActor 
local Actor = Actor 
local ToMOSParticle = ToMOSParticle
local ToMOSprite = ToMOSprite
local PrimitiveMan = PrimitiveMan
local ActivityMan = ActivityMan
local MovableMan = MovableMan 
local Vector = Vector
local Controller = Controller -- For Controller.BODY_PRONE etc.
local rte = rte

function Create(self)
    -- Timers and counters for tap-based controls (e.g., double-tap to retrieve hook)
    self.tapTimerAim = Timer() -- Unused? Or intended for a different tap action.
    self.tapTimerJump = Timer() -- Used for crouch-tap detection.
    self.tapCounter = 0
    -- self.didTap = false -- Seems unused, consider removing.
    self.canTap = false -- Flag to register the first tap in a sequence.

    self.tapTime = 200 -- Max milliseconds between taps for them to count as a sequence.
    self.tapAmount = 2 -- Number of taps required.
    
    self.guide = false -- Whether to show the aiming guide arrow.

    -- Create the guide arrow MOSRotating. This is a visual aid.
    -- Ensure "Grapple Gun Guide Arrow" preset exists and is a MOSRotating.
    local arrowPreset = PresetMan:GetPreset("Grapple Gun Guide Arrow", "MOSRotating", "Grapple Gun Guide Arrow")
    if arrowPreset and arrowPreset.ClassName == "MOSRotating" then
        self.arrow = CreateMOSRotating("Grapple Gun Guide Arrow")
        if self.arrow then
            self.arrow.GlobalAccurateDelete = true -- Ensure it cleans up properly
        end
    else
        self.arrow = nil -- Preset not found or incorrect type
        -- Log an error or warning if preset is missing/incorrect
        -- print("Warning: Grapple Gun Guide Arrow preset not found or incorrect type.")
    end

    self.originalRoundCount = 1
    self.hasGrappleActive = false
end

function Update(self)
    local parent = self:GetRootParent()

    -- Ensure the gun is held by a valid, player-controlled Actor.
    if not parent or not IsActor(parent) then
        self:Deactivate()
        return
    end

    local parentActor = ToActor(parent)

    if not parentActor:IsPlayerControlled() or parentActor.Status >= Actor.DYING then
        self:Deactivate()
        return
    end

    local controller = parentActor:GetController()
    if not controller then
        self:Deactivate()
        return
    end

    -- Magazine handling (visual representation of the hook's availability)
    if self.Magazine and MovableMan:IsParticle(self.Magazine) then
        local magazineParticle = ToMOSParticle(self.Magazine)
        
        -- Check if we have an active grapple
        local hasActiveGrapple = false
        for mo in MovableMan.AddedActors do
            if mo and mo.PresetName == "Grapple Gun Claw" and mo.parentGun and mo.parentGun.ID == self.ID then
                hasActiveGrapple = true
                break
            end
        end
        
        -- Update magazine based on grapple state
        if hasActiveGrapple then
            magazineParticle.RoundCount = 0 -- Empty when grapple is out
            magazineParticle.Scale = 0 -- Hidden
            self.hasGrappleActive = true
        elseif self.hasGrappleActive and not hasActiveGrapple then
            -- Grapple just returned, restore ammo
            magazineParticle.RoundCount = 1
            magazineParticle.Scale = 1
            magazineParticle.Frame = 0
            self.hasGrappleActive = false
        end
        
        -- Set stance offset when hook is loaded
        if magazineParticle.Scale == 1 then
            local parentSprite = ToMOSprite(self:GetParent())
            if parentSprite then
                local spriteWidth = parentSprite:GetSpriteWidth() or 0
                self.StanceOffset = Vector(spriteWidth, 1)
                self.SharpStanceOffset = Vector(spriteWidth, 1)
            end
        end

        -- Guide arrow visibility logic
        local shouldShowGuide = false
        if magazineParticle.Scale == 0 and not controller:IsState(Controller.AIM_SHARP) then
            shouldShowGuide = true
        elseif parentActor.Vel and parentActor.Vel:MagnitudeIsGreaterThan(6) then
            shouldShowGuide = true
        end
        self.guide = shouldShowGuide
    else
        self.guide = false
    end

    -- Draw the guide arrow if enabled and valid
    if self.guide and self.arrow and self.arrow.ID ~= rte.NoMOID then
        local frame = 0
        if parentActor.Vel and parentActor.Vel:MagnitudeIsGreaterThan(12) then
            frame = 1
        end
        
        local eyePos = parentActor.EyePos or Vector(0,0)
        local startPos = (parentActor.Pos + eyePos + self.Pos)/3
        local aimAngle = parentActor:GetAimAngle(true)
        local aimDistance = parentActor.AimDistance or 50
        local guidePos = startPos + Vector(aimDistance + (parentActor.Vel and parentActor.Vel.Magnitude or 0), 0):RadRotate(aimAngle)
        
        if MovableMan:IsValid(self.arrow) then
             PrimitiveMan:DrawBitmapPrimitive(ActivityMan:GetActivity():ScreenOfPlayer(controller.Player), guidePos, self.arrow, aimAngle, frame)
        else
            self.arrow = nil
        end
    end
end

function Destroy(self)
    -- Clean up the guide arrow if it exists
    if self.arrow and self.arrow.ID ~= rte.NoMOID and MovableMan:IsValid(self.arrow) then
        MovableMan:RemoveMO(self.arrow)
        self.arrow = nil
    end
end