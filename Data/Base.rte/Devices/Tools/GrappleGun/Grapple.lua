---@diagnostic disable: undefined-global
-- filepath: /Cortex-Command-Community-Project/Data/Base.rte/Devices/Tools/GrappleGun/Grapple.lua
-- Main logic for the grapple claw MovableObject.

-- Load Modules
local RopePhysics = require("Devices.Tools.GrappleGun.Scripts.RopePhysics")
local RopeRenderer = require("Devices.Tools.GrappleGun.Scripts.RopeRenderer")
local RopeInputController = require("Devices.Tools.GrappleGun.Scripts.RopeInputController")
local RopeStateManager = require("Devices.Tools.GrappleGun.Scripts.RopeStateManager")
local Logger = require("Devices.Tools.GrappleGun.Scripts.Logger")

function Create(self)
    Logger.info("Grapple Create() - Starting initialization")
    
    self.lastPos = self.Pos
    
    self.mapWrapsX = SceneMan.SceneWrapsX
    self.climbTimer = Timer()
    self.mouseClimbTimer = Timer()
    self.tapTimer = Timer() -- Initialize tapTimer
    
    Logger.debug("Grapple Create() - Basic properties initialized")
    
    -- Initialize state using the state manager. This sets self.actionMode = 0.
    RopeStateManager.initState(self)
    Logger.debug("Grapple Create() - State initialized, actionMode = %d", self.actionMode)

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

    Logger.debug("Grapple Create() - Core properties set (fireVel=%d, maxLineLength=%d)", self.fireVel, self.maxLineLength)

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

    Logger.debug("Grapple Create() - Sound containers created")

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
    
    Logger.debug("Grapple Create() - Initializing rope segments at position (%.1f, %.1f)", px, py)
    
    for i = 0, self.maxSegments do
        self.apx[i] = px
        self.apy[i] = py
        self.lastX[i] = px
        self.lastY[i] = py
    end
    
    self.currentSegments = self.minSegments
    Logger.debug("Grapple Create() - %d rope segments initialized", self.maxSegments + 1)

    -- Parent gun, parent actor, and related properties (Vel, anchor points, parentRadius)
    -- will be determined and set in the first Update call.
    -- No self.ToDelete = true will be set in Create.
    
    -- Add these new flags:
    self.shouldUnhook = false      -- Flag set by gun to signal unhook
    
    -- Keep only the tap detection variables:
    self.tapCounter = 0
    self.canTap = false
    self.tapTime = 150
    self.tapAmount = 2
    self.tapTimer = Timer()
    
    Logger.info("Grapple Create() - Initialization complete")
end

function Update (self)
    if self.ToDelete then 
        Logger.debug("Grapple Update() - ToDelete is true, exiting")
        return 
    end
    
    Logger.debug("Grapple Update() - Starting update, actionMode = %d", self.actionMode)
    
    -- First-time setup: Find parent, initialize velocity, anchor points, etc.
    if self.actionMode == 0 then
        Logger.info("Grapple Update() - First-time setup, searching for parent gun")
        local foundAndValidParent = false
        
        for gun_mo in MovableMan:GetMOsInRadius(self.Pos, self.hookRadius) do
            if gun_mo and gun_mo.ClassName == "HDFirearm" and gun_mo.PresetName == "Grapple Gun" then
                Logger.debug("Grapple Update() - Found potential parent gun: %s", gun_mo.PresetName)
                local hdfGun = ToHDFirearm(gun_mo)
                if hdfGun and SceneMan:ShortestDistance(self.Pos, hdfGun.MuzzlePos, self.mapWrapsX):MagnitudeIsLessThan(20) then
                    Logger.debug("Grapple Update() - Gun is within muzzle distance, validating")
                    self.parentGun = hdfGun
                    local rootParentMO = MovableMan:GetMOFromID(hdfGun.RootID)
                    if rootParentMO and MovableMan:IsActor(rootParentMO) then
                        self.parent = ToActor(rootParentMO)
                        Logger.info("Grapple Update() - Valid parent actor found: %s (ID: %d)", self.parent.PresetName, self.parent.ID)
                        
                        self.apx[0] = self.parent.Pos.X
                        self.apy[0] = self.parent.Pos.Y
                        self.lastX[0] = self.parent.Pos.X - (self.parent.Vel.X or 0)
                        self.lastY[0] = self.parent.Pos.Y - (self.parent.Vel.Y or 0)

                        -- Set initial velocity of the hook based on parent's aim and velocity
                        local aimAngle = self.parent:GetAimAngle(true)
                        self.Vel = (self.parent.Vel or Vector(0,0)) + Vector(self.fireVel, 0):RadRotate(aimAngle)
                        Logger.debug("Grapple Update() - Initial velocity set: (%.1f, %.1f), aim angle: %.2f", self.Vel.X, self.Vel.Y, aimAngle)
                        
                        -- Initialize hook's lastX/Y for its initial trajectory
                        self.lastX[self.currentSegments] = self.Pos.X - self.Vel.X 
                        self.lastY[self.currentSegments] = self.Pos.Y - self.Vel.Y

                        if self.parentGun then -- Should be valid here
                            self.parentGun:RemoveNumberValue("GrappleMode") -- Clear any previous mode
                            Logger.debug("Grapple Update() - Cleared previous GrappleMode from gun")
                        end

                        -- Determine parent's effective radius for terrain checks
                        self.parentRadius = 5 -- Default radius
                        if self.parent.Attachables and type(self.parent.Attachables) == "table" then
                            Logger.debug("Grapple Update() - Calculating parent radius from %d attachables", #self.parent.Attachables)
                            for _, part in ipairs(self.parent.Attachables) do
                                if part and part.Pos and part.Radius then
                                    local radcheck = SceneMan:ShortestDistance(self.parent.Pos, part.Pos, self.mapWrapsX).Magnitude + part.Radius
                                    if self.parentRadius == nil or radcheck > self.parentRadius then
                                        self.parentRadius = radcheck
                                    end
                                end
                            end
                        end
                        Logger.debug("Grapple Update() - Parent radius calculated: %.1f", self.parentRadius)
                        
                        self.actionMode = 1 -- Set to flying, initialization successful
                        Logger.info("Grapple Update() - Initialization successful, switching to flying mode")
                        
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
                        Logger.debug("Grapple Update() - Initialized %d rope segments for flight", self.currentSegments)
                        
                        foundAndValidParent = true
                    else
                        Logger.warn("Grapple Update() - Gun root is not a valid actor")
                    end -- if MovableMan:IsActor(rootParentMO)
                else
                    Logger.debug("Grapple Update() - Gun too far from muzzle or invalid")
                end -- if hdfGun and distance check
            end -- if gun_mo is grapple gun
        end -- for gun_mo

        if not foundAndValidParent then
            Logger.error("Grapple Update() - Failed to find valid parent, marking for deletion")
            self.ToDelete = true
            return -- Exit Update if initialization failed
        end
        -- If we reach here, initialization was successful, self.actionMode = 1
    end

    -- If ToDelete was set during initialization, or by other logic, exit.
    if self.ToDelete then 
        Logger.debug("Grapple Update() - ToDelete flag set, exiting")
        return 
    end

    -- Continuous validation checks for parent and gun
    -- self.parent should be an Actor if initialization succeeded and actionMode >= 1
    if not self.parent or self.parent.ID == rte.NoMOID then
        Logger.warn("Grapple Update() - Parent actor lost or invalid, marking for deletion")
        self.ToDelete = true
        return
    end
    
    local parentActor = self.parent -- self.parent is already an Actor type from the setup block
    
    -- Check if grapple gun still exists - either equipped or in inventory
    -- Smart gun reference management with extensive logging
    Logger.debug("Grapple Update() - Starting gun validation check")
    
    local needToSearchForGun = false
    local gunValidationReason = ""
    
    -- Check if our current gun reference exists
    if not self.parentGun then
        needToSearchForGun = true
        gunValidationReason = "No gun reference exists"
        Logger.warn("Grapple Update() - %s", gunValidationReason)
    else
        Logger.debug("Grapple Update() - Gun reference exists, checking validity...")
        
        -- Test if the gun reference is actually valid by safely checking properties
        local gunIsValid = false
        local validationDetails = {}
        
        -- Check 1: Can we access the gun's ID and is the gun object still valid?
        local success1, gunID = pcall(function() return self.parentGun.ID end)
        if success1 then
            validationDetails.id_accessible = true
            validationDetails.gun_id = gunID
            Logger.debug("Grapple Update() - Gun ID accessible: %d", gunID)
            
            -- Check if the gun object still exists in the game world by trying to get it from MovableMan
            local gunFromMovableMan = MovableMan:GetMOFromID(gunID)
            if gunFromMovableMan and gunFromMovableMan.ID == gunID then
                validationDetails.id_valid = true
                Logger.debug("Grapple Update() - Gun object exists in MovableMan")
            else
                needToSearchForGun = true
                gunValidationReason = string.format("Gun object no longer exists in MovableMan (ID: %d)", gunID)
                Logger.warn("Grapple Update() - %s", gunValidationReason)
            end
        else
            needToSearchForGun = true
            gunValidationReason = "Cannot access gun ID (gun object invalid)"
            Logger.warn("Grapple Update() - %s", gunValidationReason)
            validationDetails.id_accessible = false
        end
        
        -- Check 2: Can we access the gun's PresetName?
        if not needToSearchForGun then
            local success2, presetName = pcall(function() return self.parentGun.PresetName end)
            if success2 then
                validationDetails.preset_accessible = true
                validationDetails.preset_name = presetName
                Logger.debug("Grapple Update() - Gun PresetName accessible: %s", presetName or "nil")
                
                if presetName == "Grapple Gun" then
                    validationDetails.preset_valid = true
                    gunIsValid = true
                    Logger.debug("Grapple Update() - Gun preset name is correct")
                else
                    needToSearchForGun = true
                    gunValidationReason = string.format("Gun preset name incorrect: '%s' (expected 'Grapple Gun')", presetName or "nil")
                    Logger.warn("Grapple Update() - %s", gunValidationReason)
                end
            else
                needToSearchForGun = true
                gunValidationReason = "Cannot access gun PresetName (gun object corrupted)"
                Logger.warn("Grapple Update() - %s", gunValidationReason)
                validationDetails.preset_accessible = false
            end
        end
        
        -- Check 3: Can we access the gun's RootID?
        if not needToSearchForGun then
            local success3, rootID = pcall(function() return self.parentGun.RootID end)
            if success3 then
                validationDetails.rootid_accessible = true
                validationDetails.root_id = rootID
                Logger.debug("Grapple Update() - Gun RootID accessible: %d", rootID)
            else
                Logger.warn("Grapple Update() - Cannot access gun RootID (potential corruption)")
                validationDetails.rootid_accessible = false
                -- Don't mark for search yet, gun might still be valid
            end
        end
        
        -- Log detailed validation results
        Logger.debug("Grapple Update() - Gun validation details: ID_OK=%s, Preset_OK=%s, RootID_OK=%s, Overall_Valid=%s", 
                     tostring(validationDetails.id_valid), 
                     tostring(validationDetails.preset_valid), 
                     tostring(validationDetails.rootid_accessible), 
                     tostring(gunIsValid))
        
        if gunIsValid then
            Logger.debug("Grapple Update() - Current gun reference is valid, no search needed")
        end
    end
    
    -- Only search for gun if we actually need to
    local foundGun = false -- Initialize to false
    if needToSearchForGun then
        Logger.warn("Grapple Update() - Gun search triggered: %s", gunValidationReason)
        Logger.info("Grapple Update() - Performing comprehensive gun search...")
        
        local searchResults = {}
        
        -- Search Method 1: Check equipped items
        Logger.debug("Grapple Update() - Search Method 1: Checking equipped items")
        if parentActor.EquippedItem then
            Logger.debug("Grapple Update() - Main equipped item: %s (ID: %d)", 
                         parentActor.EquippedItem.PresetName or "Unknown", parentActor.EquippedItem.ID)
            if parentActor.EquippedItem.PresetName == "Grapple Gun" then
                self.parentGun = ToHDFirearm(parentActor.EquippedItem)
                foundGun = true
                searchResults.method = "main_equipped"
                searchResults.gun_id = self.parentGun.ID
                Logger.info("Grapple Update() - SUCCESS: Found grapple gun equipped in main hand (ID: %d)", self.parentGun.ID)
            end
        else
            Logger.debug("Grapple Update() - No main equipped item")
        end
        
        if not foundGun and parentActor.EquippedBGItem then
            Logger.debug("Grapple Update() - BG equipped item: %s (ID: %d)", 
                         parentActor.EquippedBGItem.PresetName or "Unknown", parentActor.EquippedBGItem.ID)
            if parentActor.EquippedBGItem.PresetName == "Grapple Gun" then
                self.parentGun = ToHDFirearm(parentActor.EquippedBGItem)
                foundGun = true
                searchResults.method = "bg_equipped"
                searchResults.gun_id = self.parentGun.ID
                Logger.info("Grapple Update() - SUCCESS: Found grapple gun equipped in BG hand (ID: %d)", self.parentGun.ID)
            end
        else
            if not parentActor.EquippedBGItem then
                Logger.debug("Grapple Update() - No BG equipped item")
            end
        end
        
        -- Search Method 2: Check inventory thoroughly
        if not foundGun then
            Logger.debug("Grapple Update() - Search Method 2: Checking inventory")
            if parentActor.Inventory then
                local inventoryCount = 0
                local grappleGunsFound = 0
                
                for item in parentActor.Inventory do
                    inventoryCount = inventoryCount + 1
                    if item then
                        Logger.debug("Grapple Update() - Inventory item %d: %s (ID: %d, RootID: %d)", 
                                     inventoryCount, item.PresetName or "Unknown", item.ID, item.RootID or -1)
                        if item.PresetName == "Grapple Gun" then
                            grappleGunsFound = grappleGunsFound + 1
                            if not foundGun then -- Take the first one we find
                                self.parentGun = ToHDFirearm(item)
                                foundGun = true
                                searchResults.method = "inventory"
                                searchResults.gun_id = self.parentGun.ID
                                searchResults.inventory_position = inventoryCount
                                Logger.info("Grapple Update() - SUCCESS: Found grapple gun in inventory position %d (ID: %d)", inventoryCount, self.parentGun.ID)
                            else
                                Logger.warn("Grapple Update() - Additional grapple gun found in inventory (ID: %d) - this is unusual", item.ID)
                            end
                        end
                    else
                        Logger.debug("Grapple Update() - Inventory item %d: nil", inventoryCount)
                    end
                end
                
                Logger.debug("Grapple Update() - Inventory search complete: %d items total, %d grapple guns found", inventoryCount, grappleGunsFound)
            else
                Logger.warn("Grapple Update() - Actor has no inventory")
            end
        end
        
        -- Search Method 3: Nearby area search
        if not foundGun then
            Logger.debug("Grapple Update() - Search Method 3: Nearby area search (radius: 150)")
            local nearbyGunsFound = 0
            
            for gun_mo in MovableMan:GetMOsInRadius(parentActor.Pos, 150) do
                if gun_mo and gun_mo.ClassName == "HDFirearm" then
                    Logger.debug("Grapple Update() - Nearby HDFirearm: %s (ID: %d, RootID: %d, Distance: %.1f)", 
                                 gun_mo.PresetName or "Unknown", gun_mo.ID, gun_mo.RootID,
                                 SceneMan:ShortestDistance(parentActor.Pos, gun_mo.Pos, self.mapWrapsX).Magnitude)
                    
                    if gun_mo.PresetName == "Grapple Gun" then
                        nearbyGunsFound = nearbyGunsFound + 1
                        Logger.debug("Grapple Update() - Found nearby grapple gun %d", nearbyGunsFound)
                        
                        -- Check ownership/accessibility
                        local isAccessible = false
                        if gun_mo.RootID == parentActor.ID then
                            isAccessible = true
                            Logger.debug("Grapple Update() - Gun belongs to our parent")
                        elseif gun_mo.RootID == rte.NoMOID then
                            isAccessible = true
                            Logger.debug("Grapple Update() - Gun is unowned")
                        else
                            local currentOwner = MovableMan:GetMOFromID(gun_mo.RootID)
                            if currentOwner and IsActor(currentOwner) then
                                if ToActor(currentOwner).Team == parentActor.Team and parentActor.Team >= 0 then
                                    isAccessible = true
                                    Logger.debug("Grapple Update() - Gun belongs to teammate")
                                else
                                    Logger.debug("Grapple Update() - Gun belongs to different team")
                                end
                            else
                                Logger.debug("Grapple Update() - Gun has invalid owner")
                            end
                        end
                        
                        if isAccessible and not foundGun then
                            self.parentGun = ToHDFirearm(gun_mo)
                            foundGun = true
                            searchResults.method = "nearby"
                            searchResults.gun_id = self.parentGun.ID
                            searchResults.distance = SceneMan:ShortestDistance(parentActor.Pos, gun_mo.Pos, self.mapWrapsX).Magnitude
                            Logger.info("Grapple Update() - SUCCESS: Found accessible nearby grapple gun (ID: %d, Distance: %.1f)", gun_mo.ID, searchResults.distance)
                        end
                    end
                end
            end
            
            Logger.debug("Grapple Update() - Nearby search complete: %d grapple guns found", nearbyGunsFound)
        end
        
        -- Report search results
        if foundGun then
            Logger.info("Grapple Update() - Gun search successful via method: %s", searchResults.method)
            
            -- Validate the newly found gun
            local newGunValid = false
            local success, newGunPreset = pcall(function() return self.parentGun.PresetName end)
            if success and newGunPreset == "Grapple Gun" then
                newGunValid = true
                Logger.debug("Grapple Update() - Newly found gun validated successfully")
            else
                Logger.error("Grapple Update() - Newly found gun failed validation!")
            end
            
            if newGunValid then
                -- Update magazine state for the found gun
                if self.parentGun.Magazine and MovableMan:IsParticle(self.parentGun.Magazine) then
                    local mag = ToMOSParticle(self.parentGun.Magazine)
                    mag.RoundCount = 0
                    mag.Scale = 0
                    Logger.debug("Grapple Update() - Updated magazine state for found gun")
                end
                
                -- Log detailed gun state
                local gunRootID = "unknown"
                local gunPos = "unknown"
                local success1, rootID = pcall(function() return self.parentGun.RootID end)
                if success1 then gunRootID = tostring(rootID) end
                local success2, pos = pcall(function() return self.parentGun.Pos end)
                if success2 then gunPos = string.format("(%.1f, %.1f)", pos.X, pos.Y) end
                
                Logger.info("Grapple Update() - Gun recovery complete: ID=%d, RootID=%s, Position=%s, Method=%s", 
                           searchResults.gun_id, gunRootID, gunPos, searchResults.method)
            end
        else
            Logger.error("Grapple Update() - Gun search failed - no grapple gun found anywhere!")
            Logger.error("Grapple Update() - Searched: equipped items, inventory items, nearby radius 150")
            
            -- Only delete if we're not already attached - if attached, enter gunless mode
            if self.actionMode > 1 then
                Logger.warn("Grapple Update() - Gun search failed but grapple is attached - entering gunless mode")
                self.parentGun = nil
                foundGun = false -- Explicitly set to false for gunless mode
            else
                Logger.error("Grapple Update() - Gun search failed and grapple not attached - marking for deletion")
                self.ToDelete = true
                return
            end
        end
    else
        -- We didn't need to search, so our existing gun reference is valid
        foundGun = true
        Logger.debug("Grapple Update() - No gun search needed, existing reference is valid")
    end
    
    -- Comprehensive gun accessibility check with detailed logging
    Logger.debug("Grapple Update() - Starting gun accessibility verification")
    
    local gunIsAccessible = false
    local accessibilityMethod = ""
    local accessibilityDetails = {}
    
    -- Get current gun state for logging
    local currentGunID = "unknown"
    local currentGunRootID = "unknown"
    local success1, gunID = pcall(function() return self.parentGun and self.parentGun.ID or rte.NoMOID end)
    if success1 then currentGunID = tostring(gunID) end
    local success2, rootID = pcall(function() return self.parentGun and self.parentGun.RootID or rte.NoMOID end)
    if success2 then currentGunRootID = tostring(rootID) end
    
    Logger.debug("Grapple Update() - Current gun state: ID=%s, RootID=%s, Parent ID=%d", 
                 currentGunID, currentGunRootID, parentActor.ID)
    
    -- Special case: If no gun found but grapple is attached, allow gunless mode
    if not foundGun and self.actionMode > 1 then
        Logger.warn("Grapple Update() - No gun available but grapple is attached - entering gunless mode")
        gunIsAccessible = true
        accessibilityMethod = "attached_without_gun"
        self.parentGun = nil -- Clear any invalid reference
        Logger.info("Grapple Update() - Grapple remains active in attached mode without gun control")
    elseif foundGun and self.parentGun then
        -- Normal accessibility checks for cases when we have a gun reference
        -- Accessibility Check 1: Is gun equipped?
        Logger.debug("Grapple Update() - Accessibility Check 1: Equipment status")
        if parentActor.EquippedItem and parentActor.EquippedItem.ID == tonumber(currentGunID) then
            gunIsAccessible = true
            accessibilityMethod = "equipped_main"
            accessibilityDetails.equipped_slot = "main"
            Logger.debug("Grapple Update() - Gun is equipped in main hand")
        elseif parentActor.EquippedBGItem and parentActor.EquippedBGItem.ID == tonumber(currentGunID) then
            gunIsAccessible = true
            accessibilityMethod = "equipped_bg"
            accessibilityDetails.equipped_slot = "background"
            Logger.debug("Grapple Update() - Gun is equipped in background hand")
        else
            Logger.debug("Grapple Update() - Gun is not equipped")
        end
        
        -- Accessibility Check 2: Is gun in inventory?
        if not gunIsAccessible then
            Logger.debug("Grapple Update() - Accessibility Check 2: Inventory status")
            if parentActor.Inventory then
                local inventoryCount = 0
                for item in parentActor.Inventory do
                    inventoryCount = inventoryCount + 1
                    if item and item.ID == tonumber(currentGunID) then
                        gunIsAccessible = true
                        accessibilityMethod = "inventory"
                        accessibilityDetails.inventory_position = inventoryCount
                        Logger.debug("Grapple Update() - Gun found in inventory at position %d", inventoryCount)
                        break
                    end
                end
                if not gunIsAccessible then
                    Logger.debug("Grapple Update() - Gun not found in inventory (%d items checked)", inventoryCount)
                end
            else
                Logger.debug("Grapple Update() - Actor has no inventory")
            end
        end
        
        -- Accessibility Check 3: Is gun nearby and owned by player?
        if not gunIsAccessible and self.parentGun then
            Logger.debug("Grapple Update() - Accessibility Check 3: Proximity and ownership")
            local gunDistance = SceneMan:ShortestDistance(parentActor.Pos, self.parentGun.Pos, self.mapWrapsX).Magnitude
            Logger.debug("Grapple Update() - Gun distance: %.1f units", gunDistance)
            
            if gunDistance < 100 then
                if currentGunRootID == tostring(rte.NoMOID) then
                    gunIsAccessible = true
                    accessibilityMethod = "nearby_unowned"
                    accessibilityDetails.distance = gunDistance
                    Logger.debug("Grapple Update() - Gun is nearby and unowned")
                elseif currentGunRootID == tostring(parentActor.ID) then
                    gunIsAccessible = true
                    accessibilityMethod = "nearby_owned"
                    accessibilityDetails.distance = gunDistance
                    Logger.debug("Grapple Update() - Gun is nearby and owned by parent")
                else
                    Logger.debug("Grapple Update() - Gun is nearby but owned by someone else (RootID: %s)", currentGunRootID)
                end
            else
                Logger.debug("Grapple Update() - Gun is too far away (%.1f > 100)", gunDistance)
            end
        end
        
        -- Special Check 4: If gun is owned by player but not equipped/in inventory, consider it accessible
        -- This handles cases where the gun might be in a weird state but still belongs to the player
        if not gunIsAccessible and currentGunRootID == tostring(parentActor.ID) then
            Logger.debug("Grapple Update() - Accessibility Check 4: Player ownership fallback")
            gunIsAccessible = true
            accessibilityMethod = "owned_fallback"
            Logger.debug("Grapple Update() - Gun is owned by parent (fallback access granted)")
        end
    else
        -- No gun reference and either not attached or gun search explicitly failed
        Logger.error("Grapple Update() - No gun available and not in valid attached state")
        gunIsAccessible = false
    end

    -- Final accessibility determination
    if gunIsAccessible then
        Logger.info("Grapple Update() - Gun accessibility CONFIRMED via method: %s", accessibilityMethod)
        if accessibilityDetails.equipped_slot then
            Logger.debug("Grapple Update() - Equipment details: slot=%s", accessibilityDetails.equipped_slot)
        elseif accessibilityDetails.inventory_position then
            Logger.debug("Grapple Update() - Inventory details: position=%d", accessibilityDetails.inventory_position)
        elseif accessibilityDetails.distance then
            Logger.debug("Grapple Update() - Proximity details: distance=%.1f", accessibilityDetails.distance)
        end
    else
        Logger.error("Grapple Update() - Gun accessibility FAILED - no valid access method found")
        Logger.error("Grapple Update() - Gun state at failure: ID=%s, RootID=%s, Position=(%.1f, %.1f)", 
                     currentGunID, currentGunRootID, self.parentGun.Pos.X, self.parentGun.Pos.Y)
        Logger.error("Grapple Update() - Player state: ID=%d, Position=(%.1f, %.1f), Team=%d", 
                     parentActor.ID, parentActor.Pos.X, parentActor.Pos.Y, parentActor.Team)
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
        Logger.debug("Grapple Update() - Flying mode: updating hook position")
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
        Logger.debug("Grapple Update() - Terrain grab mode: fixing hook position")
        -- Hook position is fixed where it grabbed
        self.Pos.X = self.apx[self.currentSegments] -- Ensure self.Pos matches anchor
        self.Pos.Y = self.apy[self.currentSegments]
        -- Velocity of the terrain anchor is zero
        self.lastX[self.currentSegments] = self.apx[self.currentSegments]
        self.lastY[self.currentSegments] = self.apy[self.currentSegments]
    elseif self.actionMode == 3 and self.target and self.target.ID ~= rte.NoMOID then -- Grabbed MO
        Logger.debug("Grapple Update() - MO grab mode: tracking target")
        local effective_target = RopeStateManager.getEffectiveTarget(self)
        if effective_target and effective_target.ID ~= rte.NoMOID then
            self.Pos = effective_target.Pos
            self.apx[self.currentSegments] = effective_target.Pos.X
            self.apy[self.currentSegments] = effective_target.Pos.Y
            self.lastX[self.currentSegments] = effective_target.Pos.X - (effective_target.Vel.X or 0)
            self.lastY[self.currentSegments] = effective_target.Pos.Y - (effective_target.Vel.Y or 0)
            Logger.debug("Grapple Update() - Target position: (%.1f, %.1f)", effective_target.Pos.X, effective_target.Pos.Y)
        else
            -- Target lost or invalid, consider unhooking or reverting to terrain grab
            Logger.warn("Grapple Update() - Target lost in MO grab mode, marking for deletion")
            self.ToDelete = true -- Or change actionMode to 2 if it should stick to the last location
            return
        end
    end
    
    -- Calculate current actual distance between player and hook
    self.lineVec = SceneMan:ShortestDistance(parentActor.Pos, self.Pos, self.mapWrapsX)
    self.lineLength = self.lineVec.Magnitude -- This is the visual length
    Logger.debug("Grapple Update() - Line length: %.1f", self.lineLength)

    -- State-dependent logic for currentLineLength (the physics length)
    if self.actionMode == 1 then -- Flying
        if self.lineLength >= self.maxShootDistance then
            if not self.limitReached then
                Logger.info("Grapple Update() - Maximum shoot distance reached (%.1f)", self.maxShootDistance)
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
        local oldLength = self.currentLineLength
        self.currentLineLength = math.max(10, math.min(self.currentLineLength, self.maxLineLength))
        if oldLength ~= self.currentLineLength then
            Logger.debug("Grapple Update() - Line length clamped from %.1f to %.1f", oldLength, self.currentLineLength)
        end
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
        Logger.debug("Grapple Update() - Flying mode: desired segments = %d (min: %d)", desiredSegments, minSegmentsForFlight)
    end
    
    -- Update segments if needed, with reduced hysteresis threshold for flight mode
    -- This ensures smoother transitions as the rope extends
    local segmentUpdateThreshold = self.actionMode == 1 and 1 or 2
    if desiredSegments ~= self.currentSegments and math.abs(desiredSegments - self.currentSegments) >= segmentUpdateThreshold then
        Logger.info("Grapple Update() - Resizing rope segments from %d to %d", self.currentSegments, desiredSegments)
        RopePhysics.resizeRopeSegments(self, desiredSegments)
    end

    -- Core rope physics simulation
    Logger.debug("Grapple Update() - Running rope physics simulation")
    RopePhysics.updateRopePhysics(self, parentActor.Pos, self.Pos, self.currentLineLength)
    
    -- Check for hook attachment collisions (only when flying)
    if self.actionMode == 1 then
        local stateChanged = RopeStateManager.checkAttachmentCollisions(self)
        if stateChanged then
            Logger.info("Grapple Update() - Hook attachment state changed, actionMode now: %d", self.actionMode)
            -- Rope physics may need re-initialization after attachment
            self.ropePhysicsInitialized = false
        end
    end
    
    -- Apply constraints and check for breaking
    Logger.debug("Grapple Update() - Applying rope constraints")
    local ropeBreaks = RopePhysics.applyRopeConstraints(self, self.currentLineLength)
    if ropeBreaks or self.shouldBreak then -- self.shouldBreak can be set by other logic
        Logger.warn("Grapple Update() - Rope breaks detected, marking for deletion")
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
        Logger.debug("Grapple Update() - Hook position updated to (%.1f, %.1f)", self.Pos.X, self.Pos.Y)
    end

    -- Aim the gun only if it's currently equipped AND we have a valid gun reference
    if self.parentGun and self.parentGun.ID ~= rte.NoMOID then
        local gunIsEquipped = (self.parentGun.RootID == parentActor.ID)
        
        if gunIsEquipped then
            local flipAng = parentActor.HFlipped and math.pi or 0
            self.parentGun.RotAngle = self.lineVec.AbsRadAngle + flipAng
            Logger.debug("Grapple Update() - Gun angle updated: %.2f", self.parentGun.RotAngle)
            
            -- Handle unhooking from firing the gun again - ONLY when gun is equipped
            if self.parentGun.FiredFrame then
                Logger.info("Grapple Update() - Gun fired while grapple active")
                if self.actionMode == 1 then -- If flying, just delete
                    Logger.info("Grapple Update() - Flying mode: marking for deletion")
                    if self.returnSound then 
                        self.returnSound:Play(parentActor.Pos)
                        Logger.debug("Grapple Update() - Return sound played for gun fire unhook (flying)")
                    end
                    self.ToDelete = true
                elseif self.actionMode > 1 then -- If attached, mark as ready to release
                    Logger.info("Grapple Update() - Attached mode: marking ready to release")
                    self.canRelease = true 
                end
            end
            -- If marked ready and gun is fired again (or activated for some guns)
            if self.canRelease and self.parentGun.FiredFrame and 
               (self.parentGun.Vel.Y ~= -1 or self.parentGun:IsActivated()) then
                Logger.info("Grapple Update() - Release condition met, marking for deletion")
                if self.returnSound then 
                    self.returnSound:Play(parentActor.Pos)
                    Logger.debug("Grapple Update() - Return sound played for gun fire unhook (attached)")
                end
                self.ToDelete = true
            end
        end
        
        -- Always hide magazine when grapple is active, regardless of equipped status
        if MovableMan:IsParticle(self.parentGun.Magazine) then -- Check if Magazine is a particle
             ToMOSParticle(self.parentGun.Magazine).Scale = 0 -- Hide magazine when grapple is active
        end
    else
        Logger.debug("Grapple Update() - No valid gun reference for aiming")
    end
    
    -- Player-specific controls and unhooking mechanisms
    if IsAHuman(parentActor) or IsACrab(parentActor) then
        if parentActor:IsPlayerControlled() then
            Logger.debug("Grapple Update() - Processing player controls")
            
            -- Only process gun-dependent controls if we have a valid gun reference
            if self.parentGun and self.parentGun.ID ~= rte.NoMOID then
                -- Refresh gun reference to ensure we have the latest gun instance
                RopeInputController.refreshGunReference(self)
                
                local controller = self.parent:GetController()
                local gunIsEquipped = self.parentGun and (self.parentGun.RootID == parentActor.ID)
                
                if controller and gunIsEquipped then
                    -- Only handle unhook inputs when gun is equipped
                    -- 1. Handle R key (reload) to unhook - use the module function
                    if RopeInputController.handleReloadKeyUnhook(self, controller) then
                        Logger.info("Grapple Update() - Reload key unhook triggered")
                        if self.returnSound then 
                            self.returnSound:Play(parentActor.Pos)
                            Logger.debug("Grapple Update() - Return sound played for R key unhook")
                        end
                        self.ToDelete = true
                        return
                    end
                    
                    -- 2. Handle pie menu unhook commands
                    if RopeInputController.handlePieMenuSelection(self) then
                        Logger.info("Grapple Update() - Pie menu unhook triggered")
                        if self.returnSound then 
                            self.returnSound:Play(parentActor.Pos)
                            Logger.debug("Grapple Update() - Return sound played for pie menu unhook")
                        end
                        self.ToDelete = true
                        return
                    end
                    
                    -- Set magazine to empty when grapple is active
                    if self.parentGun.Magazine then
                        self.parentGun.Magazine.RoundCount = 0
                        self.parentGun.Magazine.Scale = 0 -- Hide the magazine
                    end
                end
                
                if controller and gunIsEquipped then
                    -- 3. Handle double-tap crouch to unhook - use the module function
                    if RopeInputController.handleTapDetection(self, controller) then
                        Logger.info("Grapple Update() - Tap detection unhook triggered")
                        if self.returnSound then 
                            self.returnSound:Play(parentActor.Pos)
                            Logger.debug("Grapple Update() - Return sound played for tap unhook")
                        end
                        self.ToDelete = true
                        return
                    end
                    
                    -- Always allow rope movement controls when gun is equipped
                    RopeInputController.handleRopePulling(self)
                    RopeInputController.handleAutoRetraction(self, false)
                end
            else
                -- No valid gun reference, but grapple is attached - limited functionality
                Logger.debug("Grapple Update() - No gun reference, limited functionality")
                local controller = self.parent:GetController()
                if controller then
                    -- Allow basic unhook via double-tap when no gun (emergency unhook)
                    if RopeInputController.handleTapDetection(self, controller) then
                        Logger.info("Grapple Update() - Emergency tap detection unhook (no gun)")
                        if self.returnSound then 
                            self.returnSound:Play(parentActor.Pos)
                        end
                        self.ToDelete = true
                        return
                    end
                end
            end
        end
        
        -- Gun stance offset when holding the gun (only if we have a valid gun reference)
        if self.parentGun and self.parentGun.ID ~= rte.NoMOID and self.parentGun.RootID == parentActor.ID then
            local offsetAngle = parentActor.FlipFactor * (self.lineVec.AbsRadAngle - parentActor:GetAimAngle(true))
            self.parentGun.StanceOffset = Vector(self.lineLength, 0):RadRotate(offsetAngle)
            Logger.debug("Grapple Update() - Gun stance offset updated: angle=%.2f", offsetAngle)
        end
    end

    -- Render the rope
    Logger.debug("Grapple Update() - Rendering rope")
    RopeRenderer.drawRope(self, player)

    -- Final deletion check and cleanup
    if self.ToDelete then
        Logger.info("Grapple Update() - Preparing for deletion, cleaning up")
        if self.parentGun and self.parentGun.Magazine then
            -- Show the magazine as if the hook is being retracted
            local drawPos = parentActor.Pos + (self.lineVec * 0.5)
            self.parentGun.Magazine.Pos = drawPos
            self.parentGun.Magazine.Scale = 1
            self.parentGun.Magazine.Frame = 0
            Logger.debug("Grapple Update() - Magazine repositioned for retraction effect")
        end
        if self.returnSound then 
            self.returnSound:Play(parentActor.Pos)
            Logger.debug("Grapple Update() - Return sound played")
        end
    end
    
    Logger.debug("Grapple Update() - Update complete")
end

function Destroy(self)
    Logger.info("Grapple Destroy() - Starting cleanup")
    
    if self.crankSoundInstance and not self.crankSoundInstance.ToDelete then
        self.crankSoundInstance.ToDelete = true
        Logger.debug("Grapple Destroy() - Crank sound instance marked for deletion")
    end
    
    -- Try to restore magazine state via the input controller first
    RopeInputController.restoreMagazineState(self)
    
    -- Clean up references on the parent gun
    if self.parentGun and self.parentGun.ID ~= rte.NoMOID then
        Logger.debug("Grapple Destroy() - Cleaning up parent gun references")
        self.parentGun.HUDVisible = true
        self.parentGun:RemoveNumberValue("GrappleMode")
        self.parentGun.StanceOffset = Vector(0,0)
        
        -- Restore and show magazine when grapple is destroyed (fallback)
        if self.parentGun.Magazine then
            self.parentGun.Magazine.RoundCount = 1 -- Restore to 1 round when grapple returns
            self.parentGun.Magazine.Scale = 1 -- Make magazine visible again
            Logger.debug("Grapple Destroy() - Magazine restored and made visible (fallback)")
        end
    end
    
    Logger.info("Grapple Destroy() - Cleanup complete")
end
