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

function HUDHandler:Initialize(activity, newGame)
	
	print("HUDHandlerinited")
	
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
		
		self.mainTable = {};
		
		self.mainTable.playersInTeamTables = {};
		self.mainTable.teamTables = {};

		for team = 0, self.Activity.TeamCount do
			self.mainTable.playersInTeamTables[team] = {};
			self.mainTable.teamTables[team] = {};
			self.mainTable.teamTables[team].Objectives = {};
			
			self.mainTable.teamTables[team].cameraQueue = {};
		end
		
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self.Activity:PlayerActive(player) and self.Activity:PlayerHuman(player) then
				table.insert(self.mainTable.playersInTeamTables[self.Activity:GetTeamOfPlayer(player)], player);
			end
		end
		
	end
	
	-- DEBUG TEST OBJECTIVES
	
	--self:AddObjective(0, "TestObj1", "TestOne", "Attack", "Test the objective One", "This is a test One objective! Testing.", nil, true, true);
	--self:AddObjective(0, "TestObj2", "TestTwo", "Attack", "Test the objective Two", "This is a test Two objective! Testing.", nil, true, true);
	
end

function HUDHandler:OnLoad(saveLoadHandler)
	
	print("loading HUDHandler...");
	self.mainTable = saveLoadHandler:ReadSavedStringAsTable("HUDHandlerMainTable");
	print("loaded HUDHandler!");
	
end

function HUDHandler:OnSave(saveLoadHandler)
	
	print("saving HUD handler")
	saveLoadHandler:SaveTableAsString("HUDHandlerMainTable", self.mainTable);	
	print("saved HUDHandler!");
	
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
		print("HUD Handler tried to add a camera pan event with no team, no name, or no position!");
		return false;
	end
	
	for i, cameraTable in ipairs(self.mainTable.teamTables[team].cameraQueue) do
		if cameraTable.Name == name then
			print("HUD Handler tried to add a camera pan event with a name already in use!");
			return false;
		end
	end
	
	if #self.mainTable.teamTables[team].cameraQueue == 0 then
		self.teamCameraTimers[team]:Reset();
	end
	
	table.insert(self.mainTable.teamTables[team].cameraQueue, cameraTable);
	
	return self.mainTable.teamTables[team].cameraQueue[#self.mainTable.teamTables[team].cameraQueue];
	
end

function HUDHandler:RemoveCameraPanEvent(team, name)

	for i, cameraTable in ipairs(self.mainTable.teamTables[team].cameraQueue) do
		if cameraTable.Name == name then
			table.remove(self.mainTable.teamTables[team].cameraQueue, i);
			break;
		end
	end
	
end

function HUDHandler:RemoveAllCameraPanEvents(team)

	self.mainTable.teamTables[team].cameraQueue = {};
	
end

function HUDHandler:AddObjective(objTeam, objInternalNameOrFullTable, objShortName, objType, objLongName, objDescription, objPos, doNotShowInList, showArrowOnlyOnSpectatorView, alwaysShowDescription)

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
		objTable.alwaysShowDescription = alwaysShowDescription or false;
		
	else
		print("HUD Handler tried to add an objective with no team or no internal name!");
		return false;
	end
	
	for i, objTable in ipairs(self.mainTable.teamTables[objTeam].Objectives) do
		if objTable.internalName == objInternalName then
			print("HUD Handler tried to add an objective with an internal name already in use!");
			return false;
		end
	end
	
	table.insert(self.mainTable.teamTables[objTeam].Objectives, objTable);
	
	return self.mainTable.teamTables[objTeam].Objectives[#self.mainTable.teamTables[objTeam].Objectives];
	
end

function HUDHandler:RemoveObjective(objTeam, objInternalName)

	for i, objTable in ipairs(self.mainTable.teamTables[objTeam].Objectives) do
		if objTable.internalName == objInternalName then
			table.remove(self.mainTable.teamTables[objTeam].Objectives, i);
			break;
		end
	end
	
end

function HUDHandler:RemoveAllObjectives(team)

	self.mainTable.teamTables[team].Objectives = {};
	
end

function HUDHandler:MakeObjectivePrimary(objTeam, objInternalName)

	for i, objTable in ipairs(self.mainTable.teamTables[objTeam].Objectives) do
		if objTable.internalName == objInternalName then
			table.insert(objTable, 1, table.remove(objTable, i));
			break;
		end
	end
	
end

function HUDHandler:UpdateHUDHandler()

	self.Activity:ClearObjectivePoints();

	for team = 0, #self.mainTable.teamTables do
	
		-- Camera pan events
		
		local cameraTable = self.mainTable.teamTables[team].cameraQueue[1];
		
		if cameraTable then
		
			local pos = not cameraTable.Position.PresetName and cameraTable.Position or cameraTable.Position.Pos; -- severely ghetto mo check
		
			for k, player in pairs(self.mainTable.playersInTeamTables[team]) do
				CameraMan:SetScrollTarget(pos, cameraTable.Speed, player);
			end
			
			-- not ideal: anyone pressing any key can skip the panning event... but the alternatives are much more roundabout
			if self.teamCameraTimers[team]:IsPastSimMS(cameraTable.holdTime) or (not cameraTable.notCancellable and UInputMan:AnyKeyPress()) then
				table.remove(self.mainTable.teamTables[team].cameraQueue, 1);
				self.teamCameraTimers[team]:Reset();
			end
			
		end
			
		-- Objectives
	
		local skippedListings = 0;
		local extraDescSpacing = 0;
		for i, objTable in pairs(self.mainTable.teamTables[team].Objectives) do
			local showArrows = false;
			for k, player in pairs(self.mainTable.playersInTeamTables[team]) do

				if objTable.showArrowOnlyOnSpectatorView == false or (self.Activity:GetViewState(player) == Activity.ACTORSELECT) then
					showArrows = true;
				end
				
				if objTable.doNotShowInList then
				
					skippedListings = skippedListings + 1;
						
				else
				
					local spacing = (self.objListSpacing*(i-skippedListings));
					
					if (i - skippedListings == 1) or objTable.alwaysShowDescription then
						local vec = Vector(self.objListStartXOffset, self.objListStartYOffset + spacing + extraDescSpacing);
						local pos = self:MakeRelativeToScreenPos(player, vec)
						PrimitiveMan:DrawTextPrimitive(pos, objTable.longName, false, 0, 0);
						-- Description
						PrimitiveMan:DrawTextPrimitive(pos + Vector(10, self.descriptionSpacing), objTable.Description, true, 0, 0);
						
						extraDescSpacing = 10*(i-skippedListings);
						
					else
						local vec = Vector(self.objListStartXOffset, self.objListStartYOffset + spacing + extraDescSpacing);
						local pos = self:MakeRelativeToScreenPos(player, vec)
						PrimitiveMan:DrawTextPrimitive(pos, objTable.shortName, false, 0, 0);
					end					
					
				end
			end
			
			-- c++ objectives are per team, not per player, so we can't do it per player yet...
			
			if objTable.Position and showArrows then
				local pos = not objTable.Position.PresetName and objTable.Position or objTable.Position.Pos; -- severely ghetto mo check
				self.Activity:AddObjectivePoint(objTable.shortName, pos, team, GameActivity.ARROWDOWN);
			end
			
		end
	end

end


return HUDHandler:Create();