--------------------------------------- Instructions ---------------------------------------

------- Require this in your script like so: 

-- self.dockingHandler = require("Activities/Utility/DockingHandler");
-- self.dockingHandler:Initialize(Activity, bool autoAssignUnknownDropships, bool newGame);

-- Place your Rocket Dock areas, numbered 1-2-3 etc.
-- A rocket dock object will be placed at their Center, so make sure to line it up.

-- Place your Dropship Dock areas numbered 1-2-3 etc.
-- If your map has a regular Up Orbit Direction, a dropship dock object will be placed at their center.
--
-- If your map is a Down Orbit Direction map, put this in the middle of the upside-down L formed by the path
-- your dropshio would have to take to enter your dock. The system will automatically create waypoints
-- below and to the side of this area. Make sure this is as close as possible to the drop position,
-- as the side waypoint will only go roughly the dropship's width.

-- Odd-numbered Down Orbit Direction dropship docks will assume the drop position is to the left of your area.
-- Even-numbered docks will assume it is to the right.

-- No dropshio dock objects are placed for Down Orbit Direction maps.

-- When ready, use SpawnDockingCraft(craft, int specificDockNumber) with a craft reference not currently in sim to send it a dock.
-- specificDockNumber is optional, and in fact should be used carefully as there is no safety checking to see if the dock is already in use.
-- It will readily override another docking craft at that number and stop updating the old one, resulting in undesirable behavior.

-- The handler will also automatically grab dropships that enter sim without its knowledge, put them on a wait list, and assign them a dock
-- when one is available if it is initialized to do so.

------- Saving/Loading

-- Saving and loading requires you to also have the SaveLoadHandler ready.
-- Simply run OnSave(instancedSaveLoadHandler) and OnLoad(instancedSaveLoadHandler) when appropriate.

--------------------------------------- Misc. Information ---------------------------------------

-- It is your responsibility to avoid collisions with any doors on the way.

-- Rockets will be helped along their path via velocity nudging. If they are knocked off-course they will abort the sequence
-- and try to leave the way they came.

-- This system will never dock a rocket and a dropship together at the same dock number, even if they are physically separate.
-- If you want all rocket and dropship docks usable simultaneously, make sure there is no number overlap.



local DockingHandler = {};

function DockingHandler:Create()
	local Members = {};

	setmetatable(Members, self);
	self.__index = self;

	return Members;
end

function DockingHandler:Initialize(activity, autoAssignUnknownDropships, newGame)
	
	print("dockhandler inited")
	print(SceneMan.SceneOrbitDirection)
	
	self.Activity = activity;
	
	self.autoAssignUnknownDropships = autoAssignUnknownDropships or false;
	
	-- Set up player dropship dock wait list
	
	self.playerDSDockCheckTimer = Timer();
	self.playerDSDockCheckDelay = 4000;
	
	if SceneMan.SceneOrbitDirection == 1 then
		self.undersideScene = true;
	end
	-- 0 is regular
	-- if it's 2 or 3, we don't know what the hell the player is doing...
	
	-- Set up docks
	-- TODO make better with iteration
	
	if newGame then
	
		self.mainTable = {};
		
		self.mainTable.playerDSDockWaitList = {};
		
		self.playerDSDockCheckTimer = Timer();
		self.playerDSDockCheckDelay = 4000;

		self.mainTable.activeDSDockTable = {};
		
		self.mainTable.activeDSDockTable[1] = {["dockPosition"] = SceneMan.Scene:GetArea("Dropship Dock 1").Center + Vector(0, 0),
		["activeCraft"] =  nil,
		["dockingStage"] =  nil};
		
		self.mainTable.activeDSDockTable[2] = {["dockPosition"] = SceneMan.Scene:GetArea("Dropship Dock 2").Center + Vector(0, 0),
		["activeCraft"] =  nil, 
		["dockingStage"] =  nil};
		
		
		self.mainTable.activeRocketDockTable = {};
		
		self.mainTable.activeRocketDockTable[1] = {["dockPosition"] = SceneMan.Scene:GetArea("Rocket Dock 1").Center + Vector(0, 0),
		["activeCraft"] =  nil, 
		["dockingStage"] =  nil};
		
		self.mainTable.activeRocketDockTable[2] = {["dockPosition"] = SceneMan.Scene:GetArea("Rocket Dock 2").Center + Vector(0, 0),
		["activeCraft"] =  nil, 
		["dockingStage"] =  nil};
		
		if not self.undersideScene then
		
			-- Place dropship capturer docks if relevant
		
			for i, dockTable in ipairs(self.mainTable.activeDSDockTable) do
				local dockObject = CreateMOSRotating("Dropship Dock 2", "Base.rte");
				dockObject.Pos = dockTable.dockPosition
				dockObject.MissionCritical = true;
				dockObject.GibImpulseLimit = 9999999999;
				dockObject.GibWoundLimit = 9999999999;
				dockObject.PinStrength = 9999999999;
				MovableMan:AddParticle(dockObject);
			end
		end
		
		-- Place rocket capturer docks
		
		for i, dockTable in ipairs(self.mainTable.activeRocketDockTable) do
			local dockObject = CreateMOSRotating("Rocket Dock 2", "Base.rte");
			dockObject.Pos = dockTable.dockPosition
			dockObject.MissionCritical = true;
			dockObject.GibImpulseLimit = 9999999999;
			dockObject.GibWoundLimit = 9999999999;
			dockObject.PinStrength = 9999999999;
			MovableMan:AddParticle(dockObject);
		end
	end
	
end

function DockingHandler:OnLoad(saveLoadHandler)
	
	print("loading dockinghandler...");
	self.mainTable = saveLoadHandler:ReadSavedStringAsTable("dockingHandlerMainTable");
	print("loaded dockinghandler!");
	
end

function DockingHandler:OnSave(saveLoadHandler)
	
	print("saved docking maintable!")
	print(self.mainTable)
	saveLoadHandler:SaveTableAsString("dockingHandlerMainTable", self.mainTable);
	
end

function DockingHandler:SpawnDockingCraft(craft, specificDock)
	if self.undersideScene then
		return self:SpawnUndersideDockingCraft(craft, specificDock);
	else
		return self:SpawnRegularDockingCraft(craft, specificDock);
	end
end

function DockingHandler:UpdateDockingCraft()
	if self.undersideScene then
		return self:UpdateUndersideDockingCraft();
	else
		return self:UpdateRegularDockingCraft();
	end
end

function DockingHandler:SpawnUndersideDockingCraft(craft, specificDock)
	
	local dockingSuccess = false;
	
	craft.AIMode = Actor.AIMODE_GOTO;
	--craft.DeliveryState = ACraft.STANDBY;
	
	if IsACDropShip(craft) then
	
		local dockToDockAt = nil;
		local dockTable;
		if specificDock == nil then
			for i, dockInfoTable in ipairs(self.mainTable.activeDSDockTable) do
				if not dockInfoTable.activeCraft and not self.mainTable.activeRocketDockTable[i].activeCraft then
					dockToDockAt = i;
					dockTable = dockInfoTable;
					break;
				end
			end
		else
			dockToDockAt = specificDock;
		end
	
		if dockToDockAt then
			craft.Pos = Vector(dockTable.dockPosition.X, SceneMan.Scene.Height - 100);
			
			self.lastAddedCraftUniqueID = craft.UniqueID;
			
			-- Mark this craft's dock number, not used except to see if there's any dock at all
			craft:SetNumberValue("Dock Number", dockToDockAt);
			
			craft:AddAISceneWaypoint(dockTable.dockPosition + Vector(0, 500));
			craft:AddAISceneWaypoint(dockTable.dockPosition);
			local direction = dockToDockAt % 2 == 0 and -1 or 1;	
			craft:AddAISceneWaypoint(dockTable.dockPosition + Vector(275 * direction, 0))
			
			dockTable.activeCraft = craft;
			dockTable.dockingStage = 1;
			
			dockingSuccess = true;
		end
		
	else
	
		local dockToDockAt = nil;
		local dockTable;
		if specificDock == nil then
			for i, dockInfoTable in ipairs(self.mainTable.activeRocketDockTable) do
				if not dockInfoTable.activeCraft and not self.mainTable.activeDSDockTable[i].activeCraft then
					dockToDockAt = i;
					dockTable = dockInfoTable;
					break;
				end
			end
		else
			dockToDockAt = specificDock;
		end
	
		if dockToDockAt then
				
			craft.Pos = Vector(dockTable.dockPosition.X, SceneMan.Scene.Height - 100);
			craft.Vel = Vector(0, -30);
			
			self.lastAddedCraftUniqueID = craft.UniqueID;
			
			-- Mark this craft's dock number, not used except to see if there's any dock at all
			craft:SetNumberValue("Dock Number", dockToDockAt);
			
			craft:AddAISceneWaypoint(dockTable.dockPosition);
			
			dockTable.activeCraft = craft;
			dockTable.dockingStage = 1;
			
			dockingSuccess = true;
			
		end
	end
		
	if dockingSuccess == true then
		MovableMan:AddActor(craft);
		craft:UpdateMovePath();
	else
		--print("failed")
	end
	
	return dockingSuccess;

end

function DockingHandler:UpdateUndersideDockingCraft()

	-- Dropship docking explanation:
	
	-- Stage 1 is below the dock, to line up any stray dropships such as detected player dropships awaiting AI delivery
	-- This is the most prone stage to collisions, but we just have to live with them in lieu of real pathfinding
	-- Stage 2 is in the dock, still in open air
	-- Stage 3 is in the dock's dropoff zone
	-- Stage 4 is back in the open air area of the dock
	-- Stage 5 is all the way outside the map, straight down


	-- Docking system initial tests
	
	self.lastAddedCraftUniqueID = nil;
	
	if self.autoAssignUnknownDropships then
	
		-- Monitor for unknown crafts that might want to deliver stuff
		
		for actor in MovableMan.AddedActors do
			if actor.UniqueID ~= self.lastAddedCraftUniqueID then
				if IsACDropShip(actor) and not actor:NumberValueExists("Dock Number") then
					local craft = ToACDropShip(actor);
					
					-- See if we have any docks available right now
					
					local noDockFound = true;
				
					for i, dockTable in ipairs(self.mainTable.activeDSDockTable) do
						if not dockTable.activeCraft and not self.mainTable.activeRocketDockTable[i].activeCraft then
							
							print("found dock for unknown craft")
							
							craft.AIMode = Actor.AIMODE_GOTO;
							--craft.Team = 0
							--craft.Pos = Vector(dockTable.dockPosition.X, SceneMan.Scene.Height - 100);
							--craft.DeliveryState = ACraft.STANDBY;
							
							-- Mark this craft's dock number, not used except to see if there's any dock at all
							craft:SetNumberValue("Dock Number", i);
							
							craft:AddAISceneWaypoint(dockTable.dockPosition + Vector(0, 500));
							craft:AddAISceneWaypoint(dockTable.dockPosition);
							local direction = i % 2 == 0 and -1 or 1;	
							craft:AddAISceneWaypoint(dockTable.dockPosition + Vector(275 * direction, 0))
							
							dockTable.activeCraft = craft;
							dockTable.dockingStage = 1;
							
							noDockFound = false;
							
							break;
						end
					end
					
					if noDockFound then
					
						-- Put the craft in the wait table
						
						table.insert(self.mainTable.playerDSDockWaitList, craft);
						
					end		
					
				end
			end
		end
		
		if self.playerDSDockCheckTimer:IsPastSimMS(self.playerDSDockCheckDelay) then
			
			self.playerDSDockCheckTimer:Reset();
			
			-- Iterate back to front so we can remove things safely
			
			for i=#self.mainTable.playerDSDockWaitList, 1, -1 do
			
				local craft = self.mainTable.playerDSDockWaitList[i];
				
				if not craft or not MovableMan:ValidMO(craft) then
					table.remove(self.mainTable.playerDSDockWaitList, i)
					
					-- this break will make everyone wait for 4 seconds again, but that's fine
					break;
				end
				
				craft = ToACDropShip(craft);
			
				-- See if we have any docks available
		
				for i2, dockTable in ipairs(self.mainTable.activeDSDockTable) do
					if not dockTable.activeCraft and not self.mainTable.activeRocketDockTable[i2].activeCraft then
						
						craft.AIMode = Actor.AIMODE_GOTO;
						--craft.Team = 0
						--craft.Pos = Vector(dockTable.dockPosition.X, SceneMan.Scene.Height - 100);
						--craft.DeliveryState = ACraft.STANDBY;
						
						-- Mark this craft's dock number, not used except to see if there's any dock at all
						craft:SetNumberValue("Dock Number", i2);
						
						craft:AddAISceneWaypoint(dockTable.dockPosition + Vector(0, 500));
						
						dockTable.activeCraft = craft;
						dockTable.dockingStage = 1;
						
						table.remove(self.mainTable.playerDSDockWaitList, i)
						
						break;
					end
				end
			end
		end
	end
				
	-- Monitor dropship activity
	
	for i, dockTable in ipairs(self.mainTable.activeDSDockTable) do
		if dockTable.activeCraft then
			
			local direction = i % 2 == 0 and -1 or 1;	
			local craft = dockTable.activeCraft;
			
			if craft and MovableMan:ValidMO(craft) then
			
				craft = ToACraft(craft)
				
				--craft.DeliveryState = ACraft.STANDBY;
				
				if dockTable.dockingStage == 4 then
				
					--print(SceneMan:ShortestDistance(craft.Pos, dockTable.dockPosition, true))
				
					local distFromDockArea = SceneMan:ShortestDistance(craft.Pos, dockTable.dockPosition, true).Magnitude
					--print(distFromDockArea)
					if distFromDockArea < 20 then
						dockTable.dockingStage = 5;
						craft:ClearAIWaypoints();
						craft.AIMode = Actor.AIMODE_RETURN;
						craft:CloseHatch();
						
					end	
					
				elseif dockTable.dockingStage == 3 then
				
					--print(SceneMan:ShortestDistance(craft.Pos, dockTable.dockPosition + Vector(200 * direction, 0), true))
				
					local distFromDockArea = SceneMan:ShortestDistance(craft.Pos, dockTable.dockPosition + Vector(275 * direction, 0), true).Magnitude
					--print(distFromDockArea)
					if distFromDockArea < 20 then
						craft:OpenHatch();
						craft.AIMode = Actor.AIMODE_NONE;
						if craft:IsInventoryEmpty() then
							craft:ClearAIWaypoints();
							craft:AddAISceneWaypoint(dockTable.dockPosition);
							craft.AIMode = Actor.AIMODE_GOTO;
							dockTable.dockingStage = 4;
							craft:CloseHatch();
						end
					end	
					
				elseif dockTable.dockingStage == 2 then
				
					--print(SceneMan:ShortestDistance(craft.Pos, dockTable.dockPosition, true))
				
					local distFromDockArea = SceneMan:ShortestDistance(craft.Pos, dockTable.dockPosition, true).Magnitude
					--print(distFromDockArea)
					if distFromDockArea < 20 then
						dockTable.dockingStage = 3;
					end			
					
				elseif dockTable.dockingStage == 1 then
				
					--print(SceneMan:ShortestDistance(craft.Pos, dockTable.dockPosition, true))
				
					local distFromDockArea = SceneMan:ShortestDistance(craft.Pos, dockTable.dockPosition  + Vector(0, 500), true).Magnitude
					--print(distFromDockArea)
					if distFromDockArea < 20 then
						dockTable.dockingStage = 2;
					end
					
				end
				
						
			else
				dockTable.activeCraft = nil;
				dockTable.dockingStage = nil;
			end

		end
	end
	
	-- Monitor rocket activity
	
	for i, dockTable in ipairs(self.mainTable.activeRocketDockTable) do
		if dockTable.activeCraft then

			local craft = dockTable.activeCraft;

			if craft and MovableMan:ValidMO(craft) then
			
				craft = ToACraft(craft)
				
				--craft.DeliveryState = ACraft.STANDBY;
				craft.AIMode = Actor.AIMODE_GOTO;
				
				if dockTable.dockingStage ~= 3 then
					-- help these fucking things along, i'm sorry they're too stupid
					local distVectorFromDockArea = SceneMan:ShortestDistance(craft.Pos, dockTable.dockPosition, true)
					if math.abs(distVectorFromDockArea.X) > 1 then -- we're helplessly off-course, abort
						print(distVectorFromDockArea.X)
						print("aborted")
						print(craft.Pos);
						print(dockTable.dockPosition)
						dockTable.dockingStage = 3;
						craft:ClearAIWaypoints();
						craft:AddAISceneWaypoint(Vector(craft.Pos.X, SceneMan.Scene.Height + 500));
						craft:CloseHatch();		
					elseif distVectorFromDockArea.X > 0.5 then
						craft.Vel.X = math.max(craft.Vel.X - 0.02 * TimerMan.DeltaTimeSecs, 0)
					elseif distVectorFromDockArea.X < -0.5 then
						craft.Vel.X = math.min(craft.Vel.X + 0.02 * TimerMan.DeltaTimeSecs, 0)
					end
					
					craft.AngularVel = craft.AngularVel/10;
						
					
				end
					
				if dockTable.dockingStage == 2 then
					craft:OpenHatch();
					if craft:IsInventoryEmpty() then
						dockTable.dockingStage = 3;
						craft:ClearAIWaypoints();
						craft:AddAISceneWaypoint(Vector(dockTable.dockPosition.X, SceneMan.Scene.Height + 500));
						craft:CloseHatch();
					end
				elseif dockTable.dockingStage == 1 and craft:NumberValueExists("Docked") then
					dockTable.dockingStage = 2;
					craft:ClearAIWaypoints();
					craft:AddAISceneWaypoint(Vector(craft.Pos.X, SceneMan.Scene.Height + 500));
					craft:OpenHatch();
				end

			else
				dockTable.activeCraft = nil;
				dockTable.dockingStage = nil;
			end

		end
	end
	
end

function DockingHandler:SpawnRegularDockingCraft(craft, specificDock)
	
	local dockingSuccess = false;
	
	craft.AIMode = Actor.AIMODE_GOTO;
	--craft.DeliveryState = ACraft.STANDBY;
	
	if IsACDropShip(craft) then
	
		local dockToDockAt = nil;
		local dockTable;
		if specificDock == nil then
			for i, dockInfoTable in ipairs(self.mainTable.activeDSDockTable) do
				if not dockInfoTable.activeCraft and not self.mainTable.activeRocketDockTable[i].activeCraft then
					dockToDockAt = i;
					dockTable = dockInfoTable;
					break;
				end
			end
		else
			dockToDockAt = specificDock;
		end
	
		if dockToDockAt then
			craft.Pos = Vector(dockTable.dockPosition.X, 0);
			
			self.lastAddedCraftUniqueID = craft.UniqueID;
			
			-- Mark this craft's dock number, not used except to see if there's any dock at all
			craft:SetNumberValue("Dock Number", dockToDockAt);
			
			craft:AddAISceneWaypoint(dockTable.dockPosition);	
			
			dockTable.activeCraft = craft;
			dockTable.dockingStage = 1;
			
			dockingSuccess = true;
		end
		
	else
	
		local dockToDockAt = nil;
		local dockTable;
		if specificDock == nil then
			for i, dockInfoTable in ipairs(self.mainTable.activeRocketDockTable) do
				if not dockInfoTable.activeCraft and not self.mainTable.activeDSDockTable[i].activeCraft then
					dockToDockAt = i;
					dockTable = dockInfoTable;
					break;
				end
			end
		else
			dockToDockAt = specificDock;
		end
	
		if dockToDockAt then
				
			craft.Pos = Vector(dockTable.dockPosition.X, 0);
			craft.Vel = Vector(0, 0);
			
			self.lastAddedCraftUniqueID = craft.UniqueID;
			
			-- Mark this craft's dock number, not used except to see if there's any dock at all
			craft:SetNumberValue("Dock Number", dockToDockAt);
			
			craft:AddAISceneWaypoint(dockTable.dockPosition);
			
			dockTable.activeCraft = craft;
			dockTable.dockingStage = 1;
			
			dockingSuccess = true;
			
		end
	end
		
	if dockingSuccess == true then
		MovableMan:AddActor(craft);
		craft:UpdateMovePath();
	else
		--print("failed")
		return false;
	end
	
	return true;
end

function DockingHandler:UpdateRegularDockingCraft()

	self.lastAddedCraftUniqueID = nil;
	
	if self.autoAssignUnknownDropships then
	
		-- Monitor for unknown crafts that might want to deliver stuff
		
		for actor in MovableMan.AddedActors do
			if actor.UniqueID ~= self.lastAddedCraftUniqueID then
				if IsACDropShip(actor) and not actor:NumberValueExists("Dock Number") then
					local craft = ToACDropShip(actor);
					
					-- See if we have any docks available right now
					
					local noDockFound = true;
				
					for i, dockTable in ipairs(self.mainTable.activeDSDockTable) do
						if not dockTable.activeCraft and not self.mainTable.activeRocketDockTable[i].activeCraft then
							
							craft.AIMode = Actor.AIMODE_GOTO;
							--craft.Team = 0
							--craft.Pos = Vector(dockTable.dockPosition.X, SceneMan.Scene.Height - 100);
							--craft.DeliveryState = ACraft.STANDBY;
							
							-- Mark this craft's dock number, not used except to see if there's any dock at all
							craft:SetNumberValue("Dock Number", i);
							
							craft:AddAISceneWaypoint(dockTable.dockPosition + Vector(0, -500));
							craft:AddAISceneWaypoint(dockTable.dockPosition);
							
							dockTable.activeCraft = craft;
							dockTable.dockingStage = 1;
							
							noDockFound = false;
							
							break;
						end
					end
					
					if noDockFound then
					
						-- Put the craft in the wait table
						
						table.insert(self.mainTable.playerDSDockWaitList, craft);
						
					end		
					
				end
			end
		end
		
		if self.playerDSDockCheckTimer:IsPastSimMS(self.playerDSDockCheckDelay) then
			
			self.playerDSDockCheckTimer:Reset();
			
			-- Iterate back to front so we can remove things safely
			
			for i=#self.mainTable.playerDSDockWaitList, 1, -1 do
			
				local craft = self.mainTable.playerDSDockWaitList[i];
				
				if not craft or not MovableMan:ValidMO(craft) then
					table.remove(self.mainTable.playerDSDockWaitList, i)
					
					-- this break will make everyone wait for 4 seconds again, but that's fine
					break;
				end
				
				craft = ToACDropShip(craft);
			
				-- See if we have any docks available
		
				for i2, dockTable in ipairs(self.mainTable.activeDSDockTable) do
					if not dockTable.activeCraft and not self.mainTable.activeRocketDockTable[i2].activeCraft then
						
						craft.AIMode = Actor.AIMODE_GOTO;
						--craft.Team = 0
						--craft.Pos = Vector(dockTable.dockPosition.X, SceneMan.Scene.Height - 100);
						--craft.DeliveryState = ACraft.STANDBY;
						
						-- Mark this craft's dock number, not used except to see if there's any dock at all
						craft:SetNumberValue("Dock Number", i2);
						
						craft:AddAISceneWaypoint(dockTable.dockPosition + Vector(0, -500));
						craft:AddAISceneWaypoint(dockTable.dockPosition);
						
						dockTable.activeCraft = craft;
						dockTable.dockingStage = 1;
						
						table.remove(self.mainTable.playerDSDockWaitList, i)
						
						break;
					end
				end
			end
		end
	end
				
	-- Monitor dropship activity
	
	for i, dockTable in ipairs(self.mainTable.activeDSDockTable) do
		if dockTable.activeCraft then
			
			local direction = i % 2 == 0 and -1 or 1;	
			local craft = dockTable.activeCraft;
			
			if craft and MovableMan:ValidMO(craft) then
			
				craft = ToACraft(craft)
				
				PrimitiveMan:DrawCirclePrimitive(craft.Pos,60, 100);
				
				--craft.DeliveryState = ACraft.STANDBY;
				
				if dockTable.dockingStage == 2 then
					craft:OpenHatch();
					if craft:IsInventoryEmpty() then
						dockTable.dockingStage = 3;
						craft:ClearAIWaypoints();
						craft.AIMode = Actor.AIMODE_RETURN;
						craft.DeliveryState = ACraft.LAUNCH;
						craft:CloseHatch();
					end
				elseif dockTable.dockingStage == 1 and craft:NumberValueExists("Docked") then
					dockTable.dockingStage = 2;
					craft:ClearAIWaypoints();
					craft:AddAISceneWaypoint(Vector(craft.Pos.X, -5000));
					craft:OpenHatch();	
				end
				
						
			else
				dockTable.activeCraft = nil;
				dockTable.dockingStage = nil;
			end

		end
	end
	
	-- Monitor rocket activity
	
	for i, dockTable in ipairs(self.mainTable.activeRocketDockTable) do
		if dockTable.activeCraft then

			local craft = dockTable.activeCraft;

			if craft and MovableMan:ValidMO(craft) then
			
				craft = ToACraft(craft)
				
				--craft.DeliveryState = ACraft.STANDBY;
				
				if dockTable.dockingStage ~= 3 then
					-- help these fucking things along, i'm sorry they're too stupid
					local distVectorFromDockArea = SceneMan:ShortestDistance(craft.Pos, dockTable.dockPosition, true)
					if math.abs(distVectorFromDockArea.X) > 1 then -- we're helplessly off-course, abort
						print(distVectorFromDockArea.X)
						print("aborted")
						print(craft.Pos);
						print(dockTable.dockPosition)
						dockTable.dockingStage = 3;
						craft:ClearAIWaypoints();
						craft:AddAISceneWaypoint(Vector(craft.Pos.X, SceneMan.Scene.Height + 500));
						craft:CloseHatch();		
					elseif distVectorFromDockArea.X > 0.5 then
						craft.Vel.X = math.max(craft.Vel.X - 0.02 * TimerMan.DeltaTimeSecs, 0)
					elseif distVectorFromDockArea.X < -0.5 then
						craft.Vel.X = math.min(craft.Vel.X + 0.02 * TimerMan.DeltaTimeSecs, 0)
					end
					
					craft.AngularVel = craft.AngularVel/10;
						
					
				end
					
				if dockTable.dockingStage == 2 then
					craft:OpenHatch();
					if craft:IsInventoryEmpty() then
						dockTable.dockingStage = 3;
						-- The dock itself handles it returning in this case
						--craft.AIMode = Actor.AIMODE_RETURN;
						--craft.DeliveryState = ACraft.LAUNCH;
						craft:CloseHatch();
					end
				elseif dockTable.dockingStage == 1 and craft:NumberValueExists("Docked") then
					dockTable.dockingStage = 2;
					craft:ClearAIWaypoints();
					craft:AddAISceneWaypoint(Vector(craft.Pos.X, -5000));
					craft:OpenHatch();
				end

			else
				dockTable.activeCraft = nil;
				dockTable.dockingStage = nil;
			end

		end
	end
	
end

return DockingHandler:Create();