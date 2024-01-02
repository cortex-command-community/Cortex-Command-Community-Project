--------------------------------------- Instructions ---------------------------------------

--

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

function HUDHandler:QueueCameraPanEvent(team, name, pos, speed, holdTime, notCancellable)

	local cameraTable = {};
	
	if team and name and pos then
		
		cameraTable = {};
		
		cameraTable.Name = name;
		cameraTable.Position = pos;
		cameraTable.Speed = speed or 0.05;
		cameraTable.holdTime = holdTime or 5000;
		cameraTable.notCancellable = notCancellable or false;
		
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

function HUDHandler:RemoveCameraPanEvent(team, name)

	for i, cameraTable in ipairs(self.saveTable.teamTables[team].cameraQueue) do
		if cameraTable.Name == name then
			table.remove(self.saveTable.teamTables[team].cameraQueue, i);
			break;
		end
	end
	
end

function HUDHandler:RemoveAllCameraPanEvents(team)

	self.saveTable.teamTables[team].cameraQueue = {};
	
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
	
		-- Camera pan events
		
		local cameraTable = self.saveTable.teamTables[team].cameraQueue[1];
		
		if cameraTable then
		
			local pos = not cameraTable.Position.PresetName and cameraTable.Position or cameraTable.Position.Pos; -- severely ghetto mo check
		
			for k, player in pairs(self.saveTable.playersInTeamTables[team]) do
				CameraMan:SetScrollTarget(pos, cameraTable.Speed, player);
			end
			
			-- not ideal: anyone pressing any key can skip the panning event... but the alternatives are much more roundabout
			if self.teamCameraTimers[team]:IsPastSimMS(cameraTable.holdTime) or (not cameraTable.notCancellable and UInputMan:AnyKeyPress()) then
				table.remove(self.saveTable.teamTables[team].cameraQueue, 1);
				self.teamCameraTimers[team]:Reset();
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
		
		if not PerformanceMan.ShowPerformanceStats == true then
	
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