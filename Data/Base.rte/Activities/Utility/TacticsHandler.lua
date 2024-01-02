--------------------------------------- Instructions ---------------------------------------

--

--------------------------------------- Misc. Information ---------------------------------------

--




local TacticsHandler = {};

function TacticsHandler:Create()
	local Members = {};

	setmetatable(Members, self);
	self.__index = self;

	return Members;
end

function TacticsHandler:Initialize(activity, newGame)
	
	print("TacticsHandlerinited")
	
	self.Activity = activity;
	
	self.taskUpdateTimer = Timer();
	self.taskUpdateDelay = 1000;
	
	self.teamToCheckNext = 0;
	
	if newGame then
	
		self.teamList = {};
		
		for i = 0, self.Activity.TeamCount do
			self.teamList[i] = {};
			self.teamList[i].squadList = {};
			self.teamList[i].taskList = {};
		end
		--print("activity team count: " .. self.Activity.TeamCount)
		
		-- We cannot account for actors added outside of our system
		
		-- for actor in MovableMan.AddedActors do
			-- if actor.Team ~= -1 then
				-- table.insert(self.teamList[actor.Team].actorList, actor);
			-- end
		-- end
	end
	
end

function TacticsHandler:OnMessage(message, object)

	--print("tacticshandlergotmessage")

	if message == "TacticsHandler_InvalidateActor" and object then
		self:InvalidateActor(object);
		--print("was told to invalidate actor!")
		--for k, v in pairs(object) do
		--	print(k .. v);
		--end
	end
	
end

function TacticsHandler:OnLoad(saveLoadHandler)
	
	print("loading tacticshandler...");
	self.teamList = saveLoadHandler:ReadSavedStringAsTable("tacticsHandlerTeamList");
	print("loaded tacticshandler!");
	
	self:ReapplyAllTasks();
	
end

function TacticsHandler:OnSave(saveLoadHandler)
	saveLoadHandler:SaveTableAsString("tacticsHandlerTeamList", self.teamList);
end

function TacticsHandler:InvalidateActor(infoTable)

	print("tried to invalidate, table values:")
	for k, v in pairs(infoTable) do
		print(k)
		print(v)
	end

	self.teamList[infoTable.Team].squadList[infoTable.squadIndex].Actors[infoTable.actorIndex] = false;
	--print("actor invalidated through function")
	
end

function TacticsHandler:ReapplyAllTasks()
	print("ReapplyAllTasks")

	for team = 0, #self.teamList do
		for i = 1, #self.teamList[team].squadList do
			local squad = self.teamList[team].squadList[i];
			local taskName = squad.taskName
			if not (taskName and self:GetTaskByName(taskName, team)) then
				self:RetaskSquad(squad, team);
			else
				local task = self:GetTaskByName(taskName, team);
				self:ApplyTaskToSquadActors(squad.Actors, task);
			end
		end
	end

end

function TacticsHandler:GetTaskByName(taskName, team)
	
	--print("GetTaskByName")
	
	if team then
		for t = 1, #self.teamList[team].taskList do
			if self.teamList[team].taskList[t].Name == taskName then
				return self.teamList[team].taskList[t];
			end
		end
	end
	
	return false;

end

function TacticsHandler:PickTask(team)

	--print("PickTask")

	if #self.teamList[team].taskList > 0 then
		-- random weighted select
		local totalPriority = 0;
		for t = 1, #self.teamList[team].taskList do
			totalPriority = totalPriority + self.teamList[team].taskList[t].Priority;
		end
		
		local randomSelect = math.random(1, totalPriority);
		local finalSelection = 1;
		for t = 1, #self.teamList[team].taskList do
			randomSelect = randomSelect - self.teamList[team].taskList[t].Priority;
			if randomSelect <= 0 then
				--print("gotfinalselection")
				finalSelection = t;
			end
		end
		
		return self.teamList[team].taskList[finalSelection];
	else
		return false;
	end
	
end

function TacticsHandler:ApplyTaskToSquadActors(squad, task)
	if task then	
		--print("Applying Task:" .. task.Name)
		squad.taskName = task.Name;
		local randomPatrolPoint;
		if task.Type == "PatrolArea" then
			randomPatrolPoint = task.Position.RandomPoint;
		end
		for actorIndex = 1, #squad do
			local actor = squad[actorIndex];
			-- this is really strange. actors can get turned into blank Entity s without us knowing about it.
			-- no clue why it happens, but sanity check it here.
			if actor and actor.PresetName ~= "" then
				actor = ToActor(squad[actorIndex]);
				actor:FlashWhite(1000);
				actor:ClearAIWaypoints();
				if task.Type == "Defend" or task.Type == "Attack" then

					actor.AIMode = Actor.AIMODE_GOTO;
					if task.Position.PresetName then -- ghetto check if this is an MO, IsMOSRotating wigs out
						actor:AddAIMOWaypoint(task.Position);
					else
						actor:AddAISceneWaypoint(task.Position);
						if task.Type == "Defend" then
							actor:SetNumberValue("tacticsHandlerRandomStopDistance", math.random(20, 120));
						end
					end
					actor:UpdateMovePath();
				elseif task.Type == "PatrolArea" then
					actor.AIMode = Actor.AIMODE_GOTO;
					actor:AddAISceneWaypoint(randomPatrolPoint);
					--print("officially changed ai mode")
					actor:UpdateMovePath();
				elseif task.Type == "Sentry" then
					actor.AIMode = Actor.AIMODE_SENTRY;
				else
					actor.AIMode = Actor.AIMODE_BRAINHUNT;
				end
			else
				--print("during task application, actor was invalidated")
				actor = false; -- do some cleanup while we're at it
			end
		end
	else
		--print("couldnotfindttask")
		return false;
	end

	return true;
end

function TacticsHandler:RetaskSquad(squad, team)
	print("retasking squad with task name: " .. squad.taskName)

	local newTask = self:PickTask(team);
	if newTask then
		squad.taskName = newTask.Name;
		return self:ApplyTaskToSquadActors(squad.Actors, newTask);
	else
		return false;
	end
end

function TacticsHandler:RemoveTask(name, team)

	print("Removedtask: " .. name);

	if name and team then
		local task;
		local taskIndex;
		for i = 1, #self.teamList[team].taskList do
			if self.teamList[team].taskList[i].Name == name then
				task = self.teamList[team].taskList[i];
				taskIndex = i;
				break;
			end
		end
		if task then
			table.remove(self.teamList[team].taskList, taskIndex);
			--print("actuallydeletedttask")
			-- retask squads before deleting
			for i = 1, #self.teamList[team].squadList do
				--print(self.teamList[team].squadList[i].taskName)
				--print("task name to del: " .. task.Name);
				if self.teamList[team].squadList[i].taskName == task.Name then
					--print("tried to retask")
					self:RetaskSquad(self.teamList[team].squadList[i], team);
				end
			end
		else
			--print("Tactics Handler was asked to remove a task it didn't have!");
			return false;
		end
	else
		--print("Tactics Handler was asked to remove a task, but not given a name and a team!");
		return false;
	end
	
	return true;
	
end

function TacticsHandler:AddTask(name, team, taskPos, taskType, priority)

	print("AddTask")
	
	local task;

	if name and team and taskPos then
	
		for i = 1, #self.teamList[team].taskList do
			if self.teamList[team].taskList[i].Name == name then
				--print("Tactics Handler tried to add a task to a specific team with a name that already existed!");
				return false;
			end
		end
	
		if not taskType then
			taskType = "Attack";
		end
		if not priority then
			priority = 5;
		end
		
		task = {};
		task.Name = name;
		task.Type = taskType;
		task.Position = taskPos;
		if task.Position.Name and not taskType == "PatrolArea" then -- ghetto isarea check	
			-- non-patrol task types have no applicable behavior for areas, so just pick a point and stick with it.
			task.Position = task.Position.RandomPoint;
		end
		task.Priority = priority;
		
		table.insert(self.teamList[team].taskList, task);
		
		--print("Added new task with name: " .. task.Name)
		
	else
		--print("Tactics Handler tried to add a task with no name, no team, or no task position!");
		return false;
	end
	
	return task;
	
end

function TacticsHandler:AddTaskedSquad(team, squadTable, taskName)

	--print("AddTaskSquad" .. team)
	
	if team and squadTable and taskName then
		local squadEntry = {};
		squadEntry.Actors = squadTable;
		if #squadTable == 0 then
			print("Tried to add a squad with no actors in it?!");
			return false;
		end
		squadEntry.taskName = taskName;
		table.insert(self.teamList[team].squadList, squadEntry); 
		for k, act in ipairs(squadEntry.Actors) do
			local squadInfo = {};
			squadInfo.Team = team;
			squadInfo.squadIndex = #self.teamList[team].squadList;
			squadInfo.actorIndex = k;
			--print("added script and sent message")
			--print("task: " .. taskName)
			act:AddScript("Base.rte/Activities/Utility/TacticsActorInvalidator.lua");
			act:SendMessage("TacticsHandler_InitSquadInfo", squadInfo);
		end
		--print("addedtaskedsquad: " .. #self.teamList[team].squadList)
		--print("newtaskname: " .. self.teamList[team].squadList[#self.teamList[team].squadList].taskName)
	else
		print("Tried to add a tasked squad without all required arguments!");
		return false;
	end
	
	return true;
	
end

function TacticsHandler:CommunicateSquadIndexesToActors()

	for team, v in pairs(self.teamList) do
		for squad = 1, #self.teamList[team].squadList do
			for actorIndex = 1, #self.teamList[team].squadList[squad].Actors do
				if actor and MovableMan:ValidMO(actor) then
					actor:SendMessage("TacticsHandler_UpdateSquadIndex", squad);
				end
			end
		end
	end

end

function TacticsHandler:UpdateSquads(team)

	--print("now checking team: " .. team);
	
	local squadRemoved = false;

	-- backwards iterate to remove safely
	for i = #self.teamList[team].squadList, 1, -1 do
		local task = self:GetTaskByName(self.teamList[team].squadList[i].taskName, team);
		if task then
		
			local task = self:GetTaskByName(self.teamList[team].squadList[i].taskName, team);
			
			local wholePatrolSquadIdle = true;
			local noActors = true;
			
			for actorIndex = 1, #self.teamList[team].squadList[i].Actors do
				local actor = self.teamList[team].squadList[i].Actors[actorIndex];
				--print(actor)
				-- this is really strange. actors can get turned into blank Entity s without us knowing about it.
				-- no clue why it happens, but sanity check it here.
				if actor and actor.PresetName ~= "" then
					noActors = false;
					if actor.HasEverBeenAddedToMovableMan then
						actor = ToActor(self.teamList[team].squadList[i].Actors[actorIndex]);
					
						actor:FlashWhite(100);
						--print("detected actor! " .. actor.PresetName .. " of team " .. actor.Team);

						-- all is well, update task
						
						local pos = not task.Position.PresetName and task.Position or task.Position.Pos; -- severely ghetto MO check
						
						if task.Type == "Attack" then
						
						elseif task.Type == "Defend" then
							if actor:NumberValueExists("tacticsHandlerRandomStopDistance") then
								local dist = SceneMan:ShortestDistance(actor.Pos, pos, SceneMan.SceneWrapsX);
								if dist.Magnitude < actor:GetNumberValue("tacticsHandlerRandomStopDistance") then
									actor:ClearAIWaypoints();
									actor.AIMode = Actor.AIMODE_SENTRY;
								end
							end
						elseif task.Type == "PatrolArea" then
							local dist = SceneMan:ShortestDistance(actor.Pos, actor:GetLastAIWaypoint(), SceneMan.SceneWrapsX);
							--print("squad: " .. i .. "patrol dist: " .. dist.Magnitude)
							if actor.AIMode == Actor.AIMODE_SENTRY or dist.Magnitude < 40 then
								actor.AIMode = Actor.AIMODE_SENTRY;
								if actorIndex == #self.teamList[team].squadList[i].Actors and wholePatrolSquadIdle == true then
									-- if we're the last one and the whole squad is ready to go
									print("squad repatrolled")
									self:ApplyTaskToSquadActors(self.teamList[team].squadList[i].Actors, task)
								end
							else
								--print("squad: " .. i .. "patrolsquadnotfullyidle")
								wholePatrolSquadIdle = false;
							end
						end

						if actor:GetLastAIWaypoint().Magnitude == 0 then
							-- our waypoint is 0, 0, so something's gone wrong
							--print("weirdwaypoint")
							--self:ApplyTaskToSquadActors(self.teamList[team].squadList[i], task);
						end
					end
				else
					actor = false;
				end
			end
			
			if noActors then
				print("removed wiped squad")
				table.remove(self.teamList[team].squadList, i) -- squad wiped, remove it
				squadRemoved = true;
			end
		else
			print("retasking not actual task")
		
			self:RetaskSquad(self.teamList[team].squadList[i], team);
		end
	end
	
	-- squad indexes have shifted if we've removed any, so tell all the actors that
	if squadRemoved then
		self:CommunicateSquadIndexesToActors()
	end

end

function TacticsHandler:UpdateTacticsHandler()

	if self.taskUpdateTimer:IsPastSimMS(self.taskUpdateDelay) then
		self.taskUpdateTimer:Reset();

		local team = self.teamToCheckNext;
		
		self.teamToCheckNext = (self.teamToCheckNext + 1) % self.Activity.TeamCount;
		
		-- check and update all tasked squads
		
		self:UpdateSquads(team);
		
		local task = self:PickTask(team);
		if task then
			return team, task;
		else
			print("found no tasks")
		end
		
	end
	
	return false;
		

end

return TacticsHandler:Create();