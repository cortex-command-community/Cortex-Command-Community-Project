--------------------------------------- Instructions ---------------------------------------

------- Require this in your script like so: 

-- self.HUDHandler = require("Activities/Utility/HUDHandler");
-- self.HUDHandler:Initialize(Activity, bool newGame, bool verboseLogging);

-- Call self.HUDHandler:UpdateHUDHandler() every frame.

-- This utility lets you queue up camera panning events and set objectives to display on players' screens.

------- Camera stuff

-- HUDHandler:QueueCameraPanEvent(team, name, pos, speed, holdTime, notCancellable, cinematicBars, disableHUDFully, callback)
-- Only team, name, and pos are required.
-- pos is the destination of the camera pan event - note that it will start from wherever player screens happen to be just then.
-- If you want to pan between two positions specifically, you will need one pan event to set up the position (with a fast speed and low holdTime) first.
-- A speed of 0.05 is about average.
-- holdTime is the total time this event will go on, whether it reaches its destination at the given speed or not.
-- notCancellable == true will make the event unskippable, otherwise it can be skipped by pressing any button.
-- cinematicBars will enable or disable animated cinematic bars for this event.
-- disableHUDFully will disable all HUD elements, including things like the gold fund counter. However, even with this false, the HUDHandler objective list is disabled.
-- callback is a function to call when this event ends, in case you want to sync some code up with a camera pan. This will also happen if the event is skipped.

-- There is also GetCameraPanEventCount(int team), RemoveCameraPanEvent(int team, string name) and RemoveAllCameraPanEvents(int team) which are self-explanatory.

-- Manual cinematic bar operation can happen with HUDHandler:SetCinematicBars(int team, bool toggle, int thickness).
-- Note that this toggle state WILL be overriden by pan events, but your set thickness is permanent.

-- HUDHandler:SetCameraMinimumAndMaximumX(team, minimumX, maximumX) lets you set a minimum and maximum absolute X value for a particular team's players' cameras to go.
-- Use this to limit view in scripted missions, for example.

------- Objectives

-- Objectives are rather robust in how they display.

-- HUDHandler:AddObjective(objTeam, objInternalNameOrFullTable, objShortName, objType, objLongName, objDescription, objPos,
-- doNotShowInList, showArrowOnlyOnSpectatorView, showDescEvenWhenNotFirst, showDescOnlyOnSpectatorView)

-- Only team and internal name are required for a very basic objective display - the internal name will also be the objective's short name.

-- Internal name does not display on the list visually but is just there to keep track of objectives easier in code.
-- Short name displays when the objective doesn't fulfill the criteria to have its long name displayed, and also on in-game objective arrows if applicable.
-- objType is currently unused.
-- Long name is displayed if criteria is fulfilled, alongside the description if applicable.
-- Objective description is displayed if the objective is first in the list (the primary objective) or has showDescEvenWhenNotFirst set to true.
-- objPos will enable an in-game arrow and short name display at a position in the world. You can also set an MO here to have the objective follow it.
-- doNotShowInList will not show this objective in the HUD. Use it for internal objectives that only display in-game arrows, like a bunch of "kill actor" objectives.
-- showArrowOnlyOnSpectatorView will show the in-game arrow only while a player is in actor select mode, i.e. holding q or e.
-- showDescEvenWhenNotFirst will override description showing behavior and show this objective's description even if it's not first in the list.
-- showDescOnlyOnSpectatorView is self-explanatory.

-- Instead of an internal name, you can give this function a pre-constructed table. It should contain these keys:

-- internalName
-- shortName
-- Type
-- longName
-- Description
-- Position
-- doNotShowInList
-- showArrowOnlyOnSpectatorView
-- showDescEvenWhenNotFirst
-- showDescOnlyOnSpectatorView

-- RemoveObjective(team, internalName) to remove an objective. There is also RemoveAllObjectives(team).

-- MakeObjectivePrimary(team, internalName) will move an objective to the top of the list, displaying its long name and description if it has any.

------- Saving/Loading

-- Saving and loading requires you to also have the SaveLoadHandler ready.
-- Simply run OnSave(instancedSaveLoadHandler) and OnLoad(instancedSaveLoadHandler) when appropriate.

--------------------------------------- Misc. Information ---------------------------------------

--




local HUDHandler = {};

function HUDHandler:Create()
	local Members = {};

	setmetatable(Members, self);
	self.__index = self;

	return Members;
end

function HUDHandler:Initialize(activity, newGame, verboseLogging)

	self.verboseLogging = verboseLogging
	
	self.Activity = activity;
	
	self.objListStartXOffset = 25;
	self.objListStartYOffset = 25;
	self.objListSpacing = 15;
	
	self.descriptionSpacing = 15;
	
	-- reinit camera timers always
	
	self.teamCameraTimers = {};
	
	for team = 0, self.Activity.TeamCount do
		self.teamCameraTimers[team] = Timer();
	end
	
	if newGame then
		
		self.saveTable = {};
		
		self.saveTable.playersInTeamTables = {};
		self.saveTable.teamTables = {};

		for team = 0, self.Activity.TeamCount do
			self.saveTable.playersInTeamTables[team] = {};
			self.saveTable.teamTables[team] = {};
			self.saveTable.teamTables[team].Objectives = {};
			
			self.saveTable.teamTables[team].cameraQueue = {};
			self.saveTable.teamTables[team].cinematicBars = false;
			self.saveTable.teamTables[team].cinematicBarThickness = 50;
			self.saveTable.teamTables[team].cinematicBarAnimationTimer = Timer();
		end
		
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self.Activity:PlayerActive(player) and self.Activity:PlayerHuman(player) then
				table.insert(self.saveTable.playersInTeamTables[self.Activity:GetTeamOfPlayer(player)], player);
			end
		end
		
	end
	
	print("INFO: HUDHandler initialized!")
	
	-- DEBUG TEST OBJECTIVES
	
	--self:AddObjective(0, "TestObj1", "TestOne", "Attack", "Test the objective One", "This is a test One objective! Testing.", nil, true, true);
	--self:AddObjective(0, "TestObj2", "TestTwo", "Attack", "Test the objective Two", "This is a test Two objective! Testing.", nil, true, true);
	
end

function HUDHandler:OnLoad(saveLoadHandler)
	
	print("INFO: HUDHandler loading...");
	self.saveTable = saveLoadHandler:ReadSavedStringAsTable("HUDHandlerMainTable");
	
	-- i can't tell why it goes wonky when just loaded straight, but it does, so:
	
	for team, teamTable in pairs(self.saveTable.teamTables) do
		local tempTable = {};
		for i, objTable in ipairs(teamTable.Objectives) do
			tempTable[i] = {};
			for k, v in pairs(objTable) do
				tempTable[i][k] = v;
			end
		end
		self:RemoveAllObjectives(team);
		for k, objTable in ipairs(tempTable) do
			self:AddObjective(team, objTable);
		end
	end
	
	print("INFO: HUDHandler loaded!");
	
end

function HUDHandler:OnSave(saveLoadHandler)
	
	print("INFO: HUDHandler saving...");
	saveLoadHandler:SaveTableAsString("HUDHandlerMainTable", self.saveTable);	
	print("INFO: HUDHandler saved!");
	
	-- camera timers deliberately not saved, might as well reset them all
	
end

function HUDHandler:MakeRelativeToScreenPos(player, vector)

	local vector = Vector(
		CameraMan:GetOffset(player).X + vector.X,
		CameraMan:GetOffset(player).Y + vector.Y
	)
	return vector;
	
end

function HUDHandler:SetCinematicBars(team, toggle, thickness)

	if thickness == nil then
		thickness = 50;
	end

	if team and toggle ~= nil then
	
		if toggle ~= self.saveTable.teamTables[team].cinematicBars then
			self.saveTable.teamTables[team].cinematicBarAnimationTimer:Reset();
		end
		self.saveTable.teamTables[team].cinematicBars = toggle;
		
		if not thickness == false then
			self.saveTable.teamTables[team].cinematicBarThickness = thickness;
			if self.verboseLogging then
				print("INFO: HUDHandler set Cinematic Bars for team " .. team .. " to " .. tostring(toggle) .. " with thickness " .. thickness);
			end						
		end
		
	else
		print("ERROR: HUDHandler tried to set cinematics bars, but was not given a team or a new toggle setting!");
		return false;
	end
	
	return true;

end

function HUDHandler:DrawCinematicBars(team)

	local draw = false;
	local factor;

	if self.saveTable.teamTables[team].cinematicBars then
		factor = 1 - math.min(1, self.saveTable.teamTables[team].cinematicBarAnimationTimer.ElapsedSimTimeMS/2000);
		draw = true;
	elseif not self.saveTable.teamTables[team].cinematicBarAnimationTimer:IsPastSimMS(2500) then
		factor = math.min(1, self.saveTable.teamTables[team].cinematicBarAnimationTimer.ElapsedSimTimeMS/2000);
		draw = true;
	end
	
	if draw then
		for k, player in pairs(self.saveTable.playersInTeamTables[team]) do
			local topBarStart = self:MakeRelativeToScreenPos(player, Vector(-50, -50))
			topBarStart = topBarStart + Vector(0, -self.saveTable.teamTables[team].cinematicBarThickness*factor);
			local topBarEnd = self:MakeRelativeToScreenPos(player, Vector(FrameMan.PlayerScreenWidth + 50, self.saveTable.teamTables[team].cinematicBarThickness))
			topBarEnd = topBarEnd + Vector(0, -self.saveTable.teamTables[team].cinematicBarThickness*factor);
		
			local bottomBarStart = self:MakeRelativeToScreenPos(player, Vector(-50, FrameMan.PlayerScreenHeight - self.saveTable.teamTables[team].cinematicBarThickness))
			bottomBarStart = bottomBarStart + Vector(0, self.saveTable.teamTables[team].cinematicBarThickness*factor);
			local bottomBarEnd = self:MakeRelativeToScreenPos(player, Vector(FrameMan.PlayerScreenWidth + 50, FrameMan.PlayerScreenHeight + 50))
			bottomBarEnd = bottomBarEnd + Vector(0, self.saveTable.teamTables[team].cinematicBarThickness*factor);
			
			PrimitiveMan:DrawBoxFillPrimitive(player, topBarStart, topBarEnd, 255);
			PrimitiveMan:DrawBoxFillPrimitive(player, bottomBarStart, bottomBarEnd, 255);
		end
		return true;
	end
	
	return false;

end

function HUDHandler:QueueCameraPanEvent(team, name, pos, speed, holdTime, notCancellable, cinematicBars, disableHUDFully, callback)
	local cameraTable = {};
	
	if team and name and pos then
		cameraTable = {};
		
		cameraTable.Name = name;
		cameraTable.Position = pos;
		cameraTable.Speed = speed or 0.05;
		cameraTable.holdTime = holdTime or 5000;
		cameraTable.notCancellable = notCancellable or false;
		cameraTable.cinematicBars = cinematicBars or false;
		cameraTable.disableHUDFully = disableHUDFully or false;
		cameraTable.callback = callback;
	else
		print("ERROR: HUDHandler tried to add a camera pan event with no team, no name, or no position!");
		return false;
	end
	
	for i, cameraTable in ipairs(self.saveTable.teamTables[team].cameraQueue) do
		if cameraTable.Name == name then
			print("ERROR: HUDHandler tried to add a camera pan event with a name already in use!");
			return false;
		end
	end
	
	if #self.saveTable.teamTables[team].cameraQueue == 0 then
		self.teamCameraTimers[team]:Reset();
	end
	
	table.insert(self.saveTable.teamTables[team].cameraQueue, cameraTable);
	
	return self.saveTable.teamTables[team].cameraQueue[#self.saveTable.teamTables[team].cameraQueue];
	
end

function HUDHandler:GetCameraPanEventCount(team)

	if team then
		return #self.saveTable.teamTables[team].cameraQueue;
	else
		print("ERROR: HUDHandlier tried to get a camera pan event count but wasn't given a team!");
		return false;
	end
	
end

function HUDHandler:RemoveCameraPanEvent(team, name)

	for i, cameraTable in ipairs(self.saveTable.teamTables[team].cameraQueue) do
		if cameraTable.Name == name then
			table.remove(self.saveTable.teamTables[team].cameraQueue, i);
			break;
		end
	end
	
end

function HUDHandler:RemoveAllCameraPanEvents(team, doNotResetHUD)

	self.saveTable.teamTables[team].cameraQueue = {};
	
	if not doNotResetHUD then
		for team = 0, #self.saveTable.teamTables do
			FrameMan:SetHudDisabled(false);
			self:SetCinematicBars(team, false, false);
		end
	end
	
end

function HUDHandler:SetCameraMinimumAndMaximumX(team, minimumX, maximumX)

	if team then
	
		if minimumX and not maximumX then
			self.maximumX = SceneMan.SceneWidth;
		elseif maximumX and maximumX < minimumX then
			print("ERROR: HUDHandler tried to set a camera max X that was smaller than the min X!");
			return false;
		end
	
		self.saveTable.teamTables[team].cameraMinimumX = minimumX;
		self.saveTable.teamTables[team].cameraMaximumX = maximumX;
		
		if self.verboseLogging then
			print("INFO: HUDHandler set Camera Minimum X " .. minimumX .. " and Camera Maximum X " .. maximumX .. " for team " .. team);
		end

	else
		print("ERROR: HUDHandler tried to set camera min/max X without being given any arguments!");
		return false;
	end
	
	if SceneMan.SceneWrapsX then
		print("WARNING: HUDHandler set camera minimum and maximum X with the current scene wrapping at the X axis!");
	end
	
	return true;
	
end

function HUDHandler:AddObjective(objTeam, objInternalNameOrFullTable, objShortName, objType, objLongName, objDescription, objPos, doNotShowInList, showArrowOnlyOnSpectatorView, showDescEvenWhenNotFirst, showDescOnlyOnSpectatorView)

	local objTable;
	
	if type(objInternalNameOrFullTable) == "table" then
		objTable = objInternalNameOrFullTable;
	elseif objTeam and objInternalNameOrFullTable then
		
		objTable = {};
		
		objTable.internalName = objInternalNameOrFullTable;
		objTable.shortName = objShortName or objInternalNameOrFullTable;
		objTable.Type = objType or "Generic";
		objTable.longName = objLongName or objShortName;
		objTable.Description = objDescription or ""
		objTable.Position = objPos;
		objTable.doNotShowInList = doNotShowInList or false;
		objTable.showArrowOnlyOnSpectatorView = showArrowOnlyOnSpectatorView or false;
		objTable.showDescEvenWhenNotFirst = showDescEvenWhenNotFirst or false;
		objTable.showDescOnlyOnSpectatorView = showDescOnlyOnSpectatorView == false or true;
		
	else
		print("ERROR: HUD Handler tried to add an objective with no team or no internal name!");
		return false;
	end
	
	for i, objTable in ipairs(self.saveTable.teamTables[objTeam].Objectives) do
		if objTable.internalName == objInternalName then
			print("ERROR: HUD Handler tried to add an objective with an internal name already in use!");
			return false;
		end
	end
	
	table.insert(self.saveTable.teamTables[objTeam].Objectives, objTable);
	
	return self.saveTable.teamTables[objTeam].Objectives[#self.saveTable.teamTables[objTeam].Objectives];
	
end

function HUDHandler:RemoveObjective(objTeam, objInternalName)

	for i, objTable in ipairs(self.saveTable.teamTables[objTeam].Objectives) do
		if objTable.internalName == objInternalName then
			table.remove(self.saveTable.teamTables[objTeam].Objectives, i);
			break;
		end
	end
	
end

function HUDHandler:RemoveAllObjectives(team)

	self.saveTable.teamTables[team].Objectives = {};
	
end

function HUDHandler:MakeObjectivePrimary(objTeam, objInternalName)

	for i, objTable in ipairs(self.saveTable.teamTables[objTeam].Objectives) do
		if objTable.internalName == objInternalName then
			table.insert(objTable, 1, table.remove(objTable, i));
			break;
		end
	end
	
end

function HUDHandler:UpdateHUDHandler()
	self.Activity:ClearObjectivePoints();

	for team = 0, #self.saveTable.teamTables do
	
		-- Cinematic bars
		local disableDrawingObjectives = self:DrawCinematicBars(team);	
	
		local cameraTable = self.saveTable.teamTables[team].cameraQueue[1];

		if cameraTable then
			-- Camera pan events
			if cameraTable.Position then
				local pos = not cameraTable.Position.PresetName and cameraTable.Position or cameraTable.Position.Pos; -- severely ghetto mo check
			
				for k, player in pairs(self.saveTable.playersInTeamTables[team]) do
					CameraMan:SetScrollTarget(pos, cameraTable.Speed, player);
					self.Activity:SetViewState(Activity.OBSERVE, player);
					if cameraTable.disableHUDFully then
						FrameMan:SetHudDisabled(true, self.Activity:ScreenOfPlayer(player));
						disableDrawingObjectives = true;
					end
				end
			end
			
			if cameraTable.cinematicBars then
				self:SetCinematicBars(team, true, false);
			end
			
			-- not ideal: anyone pressing any key can skip the panning event... but the alternatives are much more roundabout
			if self.teamCameraTimers[team]:IsPastSimMS(cameraTable.holdTime) or (not cameraTable.notCancellable and UInputMan:AnyKeyPress()) then
				-- Callback functions
				if cameraTable.callback and type(cameraTable.callback) == "function" then
					cameraTable.callback();
				end
				
				table.remove(self.saveTable.teamTables[team].cameraQueue, 1);
				self.teamCameraTimers[team]:Reset();
				if not self.saveTable.teamTables[team].cameraQueue[1] then
					self:SetCinematicBars(team, false, false);
					for k, player in pairs(self.saveTable.playersInTeamTables[team]) do
						self.Activity:SetViewState(Activity.NORMAL, player);
						FrameMan:SetHudDisabled(false, self.Activity:ScreenOfPlayer(player));
					end
				else
					if not self.saveTable.teamTables[team].cameraQueue[1].cinematicBars then
						self:SetCinematicBars(team, false, false);
					end
					if not self.saveTable.teamTables[team].cameraQueue[1].disableHUDFully then
						for k, player in pairs(self.saveTable.playersInTeamTables[team]) do
							FrameMan:SetHudDisabled(false, self.Activity:ScreenOfPlayer(player));
						end
					end
				end
			end
			
		else -- enforce min/max limits
		
			for k, player in pairs(self.saveTable.playersInTeamTables[team]) do
				if self.saveTable.teamTables[team].cameraMinimumX then
					local adjustedCameraMinimumX = self.saveTable.teamTables[team].cameraMinimumX + (0.5 * (FrameMan.PlayerScreenWidth - 960))
					if CameraMan:GetScrollTarget(player).X < adjustedCameraMinimumX then
						CameraMan:SetScrollTarget(Vector(adjustedCameraMinimumX, CameraMan:GetScrollTarget(player).Y), 0.25, player);
					end
				end
			
				if self.saveTable.teamTables[team].cameraMaximumX then
					if CameraMan:GetScrollTarget(player).X > self.saveTable.teamTables[team].cameraMaximumX then
						CameraMan:SetScrollTarget(Vector(self.saveTable.teamTables[team].cameraMaximumX, CameraMan:GetScrollTarget(player).Y), 0.25, player);
					end
				end
			end
		end

		-- Objectives
		if not PerformanceMan.ShowPerformanceStats == true and not disableDrawingObjectives then
			local skippedListings = 0;
			local extraDescSpacing = 0;
			for i, objTable in pairs(self.saveTable.teamTables[team].Objectives) do
				local spectatorView = false;
				for k, player in pairs(self.saveTable.playersInTeamTables[team]) do
					if objTable.showArrowOnlyOnSpectatorView == false or (self.Activity:GetViewState(player) == Activity.ACTORSELECT) then
						spectatorView = true;
					end
					
					if objTable.doNotShowInList then
						skippedListings = skippedListings + 1;
					else
						local spacing = (self.objListSpacing*(i-skippedListings));
						if ((i - skippedListings == 1) or objTable.showDescEvenWhenNotFirst) then
							local vec = Vector(self.objListStartXOffset, self.objListStartYOffset + spacing + extraDescSpacing);
							local pos = self:MakeRelativeToScreenPos(player, vec)
							PrimitiveMan:DrawTextPrimitive(pos, objTable.longName, false, 0, 0);
							-- Description
							if not (objTable.showDescOnlyOnSpectatorView and not spectatorView) then
								PrimitiveMan:DrawTextPrimitive(pos + Vector(10, self.descriptionSpacing), objTable.Description, true, 0, 0);			
								extraDescSpacing = 10*(i-skippedListings);
							end
							
						else
							local vec = Vector(self.objListStartXOffset, self.objListStartYOffset + spacing + extraDescSpacing);
							local pos = self:MakeRelativeToScreenPos(player, vec)
							PrimitiveMan:DrawTextPrimitive(pos, objTable.shortName, false, 0, 0);
						end					
						
					end
				end
				
				-- c++ objectives are per team, not per player, so we can't do it per player yet...
				if objTable.Position and spectatorView then
					local pos = not objTable.Position.PresetName and objTable.Position or objTable.Position.Pos; -- severely ghetto mo check
					self.Activity:AddObjectivePoint(objTable.shortName, pos, team, GameActivity.ARROWDOWN);
				end
			end
		end
	end

end


return HUDHandler:Create();