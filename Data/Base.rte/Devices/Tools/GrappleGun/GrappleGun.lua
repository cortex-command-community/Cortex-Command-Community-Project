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
end

function Update(self)
    local parent = self:GetRootParent()

    -- Ensure the gun is held by a valid, player-controlled Actor.
    if not parent or not IsActor(parent) then
        self:Deactivate() -- If not held by an actor, deactivate.
        return
    end

    local parentActor = ToActor(parent) -- Cast to Actor base type.
    -- Specific casting to AHuman or ACrab can be done if needed for type-specific logic.

    if not parentActor:IsPlayerControlled() or parentActor.Status >= Actor.DYING then
        self:Deactivate() -- Deactivate if not player controlled or if player is dying.
        return
    end

    local controller = parentActor:GetController()
    if not controller then
        self:Deactivate() -- Should not happen if IsPlayerControlled is true, but good check.
        return
    end

    -- Deactivate if equipped in the background arm and a foreground item exists,
    -- to allow the foreground item (e.g., another weapon) to be used.
    if parentActor.EquippedBGItem and parentActor.EquippedBGItem.ID == self.ID and parentActor.EquippedItem then
        self:Deactivate()
        -- Potentially return here if no further logic should run for a BG equipped grapple gun.
    end

    -- Magazine handling (visual representation of the hook's availability)
    if self.Magazine and MovableMan:IsParticle(self.Magazine) then
        local magazineParticle = ToMOSParticle(self.Magazine)
        
        -- Double tapping crouch retrieves the hook (if a grapple is active)
        -- This logic seems to be for initiating a retrieve action from the gun itself.
        -- The actual unhooking is handled by the Grapple.lua script's tap detection.
        -- This section might be redundant if Grapple.lua's tap detection is comprehensive.
        if magazineParticle.Scale == 1 then -- Assuming Scale 1 means hook is "loaded" / available to fire
            -- The following stance offsets seem to be for when the hook is *not* fired yet.
            -- Consider if this is the correct condition.
            local parentSprite = ToMOSprite(self:GetParent()) -- Assuming self:GetParent() is the gun's sprite component
            if parentSprite then
                local spriteWidth = parentSprite:GetSpriteWidth() or 0
                self.StanceOffset = Vector(spriteWidth, 1)
                self.SharpStanceOffset = Vector(spriteWidth, 1)
            end

            -- Crouch-tap logic (potentially for recalling an active hook)
            if controller:IsState(Controller.BODY_PRONE) then
                if self.canTap then
                    controller:SetState(Controller.BODY_PRONE, false) -- Prevent continuous prone state
                    self.tapTimerJump:Reset()
                    -- self.didTap = true; -- Mark that a tap occurred (if used elsewhere)
                    self.canTap = false
                    self.tapCounter = self.tapCounter + 1
                end
            else
                self.canTap = true -- Allow first tap when not prone
            end

            if self.tapTimerJump:IsPastSimMS(self.tapTime) then
                self.tapCounter = 0 -- Reset counter if too much time has passed
            else
                if self.tapCounter >= self.tapAmount then
                    -- If enough taps, activate the gun. This might be intended to fire/recall.
                    -- If a grapple is already out, Grapple.lua's tap detection should handle recall.
                    -- If no grapple is out, this would fire a new one.
                    -- Clarify the intent: is this to fire, or to send a signal to an existing grapple?
                    self:Activate() -- This will typically fire the HDFirearm.
                    self.tapCounter = 0
                end
            end
        end

        -- Guide arrow visibility logic
        -- Show if magazine scale is 0 (hook is fired) AND not sharp aiming, OR if parent is moving fast.
        local shouldShowGuide = false
        if magazineParticle.Scale == 0 and not controller:IsState(Controller.AIM_SHARP) then
            shouldShowGuide = true
        elseif parentActor.Vel and parentActor.Vel:MagnitudeIsGreaterThan(6) then
            shouldShowGuide = true
        end
        self.guide = shouldShowGuide
    else
        self.guide = false -- No magazine or not a particle, so no guide based on it.
    end

    -- Draw the guide arrow if enabled and valid
    if self.guide and self.arrow and self.arrow.ID ~= rte.NoMOID then
        local frame = 0
        if parentActor.Vel and parentActor.Vel:MagnitudeIsGreaterThan(12) then
            frame = 1 -- Use a different arrow frame for higher speeds
        end
        
        -- Calculate positions for drawing the arrow
        -- EyePos might not exist on all Actor types, ensure parentActor has it or use a fallback.
        local eyePos = parentActor.EyePos or Vector(0,0) 
        local startPos = (parentActor.Pos + eyePos + self.Pos)/3 -- Averaged position
        local aimAngle = parentActor:GetAimAngle(true)
        local aimDistance = parentActor.AimDistance or 50 -- Default AimDistance if not present
        local guidePos = startPos + Vector(aimDistance + (parentActor.Vel and parentActor.Vel.Magnitude or 0), 0):RadRotate(aimAngle)
        
        -- Ensure the arrow MO still exists before trying to draw with it
        if MovableMan:IsValid(self.arrow) then
             PrimitiveMan:DrawBitmapPrimitive(ActivityMan:GetActivity():ScreenOfPlayer(controller.Player), guidePos, self.arrow, aimAngle, frame)
        else
            self.arrow = nil -- Arrow MO was deleted, nullify reference
        end
    end
    
    -- Ensure magazine is visually "full" and ready if no grapple is active.
    -- This assumes the HDFirearm's standard magazine logic handles firing.
    -- If a grapple claw MO (the projectile) is active, Grapple.lua will hide the magazine.
    -- This section ensures it's visible when no grapple is out.
    if self.Magazine and MovableMan:IsParticle(self.Magazine) then
        local magParticle = ToMOSParticle(self.Magazine)
        local isActiveGrapple = false
        -- Check if there's an active grapple associated with this gun
        for mo_instance in MovableMan:GetMOsByPreset("Grapple Gun Claw") do
            if mo_instance and mo_instance.parentGun and mo_instance.parentGun.ID == self.ID then
                isActiveGrapple = true
                break
            end
        end

        if not isActiveGrapple then
            magParticle.RoundCount = 1 -- Visually full
            magParticle.Scale = 1      -- Visible
            magParticle.Frame = 0      -- Standard frame
        else
            magParticle.Scale = 0 -- Hidden by active grapple (Grapple.lua also does this)
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