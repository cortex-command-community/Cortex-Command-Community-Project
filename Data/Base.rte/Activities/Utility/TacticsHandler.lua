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

function TacticsHandler:Initialize(activity, newGame, minimumSquadActorCount, maximumSquadActorCount, squadIdleTimeLimitMS)
	
	print("TacticsHandlerinited")
	
	self.Activity = activity;
	
	self.taskUpdateTimer = Timer();
	self.taskUpdateDelay = 1000;
	
	self.teamToCheckNext = 0;
	
	if not minimumSquadActorCount then
		minimumSquadActorCount = 2;
	end
	if not maximumSquadActorCount then
		maximumSquadActorCount = 5;
	end
	if not squadIdleTimeLimitMS then
		squadIdleTimeLimitMS = 45000;
	end
	
	self.minimumSquadActorCount = minimumSquadActorCount;
	self.maximumSquadActorCount = maximumSquadActorCount;
	self.squadIdleTimeLimitMS = squadIdleTimeLimitMS;
	
	if newGame then
	
		self.saveTable = {};
		self.saveTable.teamList = {};
		
		for i = 0, self.Activity.TeamCount do
			self.saveTable.teamList[i] = {};
			self.saveTable.teamList[i].squadList = {};
			self.saveTable.teamList[i].taskList = {};
		end
		--print("activity team count: " .. self.Activity.TeamCount)
		
		-- We cannot account for actors added outside of our system
		
		-- for actor in MovableMan.AddedActors do
			-- if actor.Team ~= -1 then
				-- table.insert(self.saveTable.teamList[actor.Team].actorList, actor);
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
	self.saveTable = saveLoadHandler:ReadSavedStringAsTable("tacticsHandlerTeamList");
	for team = 0, #self.saveTable.teamList do
		for squad = 1, #self.saveTable.teamList[team].squadList do
			for actorIndex = 1, #self.saveTable.teamList[team].squadList[squad].Actors do	
				-- note that if it IS a number then it's probably a stale UniqueID with no actor
				-- that we didn't convert when saving... but that's fine, we deal with stale IDs constantly.
				if type(self.saveTable.teamList[team].squadList[squad].Actors[actorIndex]) ~= "number" then
					print("tacticshandler converted following actor to following uniqueid:")
					print(self.saveTable.teamList[team].squadList[squad].Actors[actorIndex])
					self.saveTable.teamList[team].squadList[squad].Actors[actorIndex] = self.saveTable.teamList[team].squadList[squad].Actors[actorIndex].UniqueID;
					print(self.saveTable.teamList[team].squadList[squad].Actors[actorIndex])
				else
					print("tacticshandler removed stale uniqueID on load");
					self.saveTable.teamList[team].squadList[squad].Actors[actorIndex] = -1;
				end
			end
		end
	end
	print("loaded tacticshandler!");
	
	self:ReapplyAllTasks();
	
end

function TacticsHandler:OnSave(saveLoadHandler)
	print("saving tacticshandler")
	-- saving/loading destroys all not-in-sim entities forever
	-- fugg :DD
	-- salvage what we can, resolve our uniqueids into MOs that the saveloadhandler can handle at least

	
	for team = 0, #self.saveTable.teamList do
		for squad = 1, #self.saveTable.teamList[team].squadList do
			for actorIndex = 1, #self.saveTable.teamList[team].squadList[squad].Actors do
				print("Trying to save squad: " .. squad .. " of team " .. team);
				print("tacticshandler tried to save the following actor:")
				print(self.saveTable.teamList[team].squadList[squad].Actors[actorIndex])
				if type(self.saveTable.teamList[team].squadList[squad].Actors[actorIndex]) ~= "number" then
					print("ERROR: Tacticshandler UniqueID was not a number! Something's very broken.");
					print("It was: ");
					print(self.saveTable.teamList[team].squadList[squad].Actors[actorIndex]);
					break;
				end
				local actor = MovableMan:FindObjectByUniqueID(self.saveTable.teamList[team].squadList[squad].Actors[actorIndex]);
				if actor then
					self.saveTable.teamList[team].squadList[squad].Actors[actorIndex] = actor;
				end
			end
		end
	end
	
	-- for t, team in pairs(self.saveTable.teamList) do
		-- for sI, squad in pairs(team.squadList) do
			-- print("Trying to save squad: " .. sI .. " of team " .. t);
			-- print("This squad had task: " .. squad.taskName);
			-- for aI, uniqueID in pairs(squad.Actors) do
				-- print("tacticshandler tried to save the following actor:")
				-- print(uniqueID)
				-- if type(uniqueID) ~= "number" then
					-- print("ERROR: Tacticshandler UniqueID was not a number! Something's very broken.");
					-- print("It was: ");
					-- print(uniqueID);
					-- break;
				-- end
				-- local actor = MovableMan:FindObjectByUniqueID(uniqueID);
				-- if actor then
					-- squad.Actors[sI] = actor;
				-- else
					-- squad.Actors[sI] = nil;
				-- end
			-- end
		-- end
	-- end
					
	saveLoadHandler:SaveTableAsString("tacticsHandlerTeamList", self.saveTable);
	
	-- and now so we can actually keep playing...
	-- turn them back, lol
	
	for team = 0, #self.saveTable.teamList do
		for squad = 1, #self.saveTable.teamList[team].squadList do
			for actorIndex = 1, #self.saveTable.teamList[team].squadList[squad].Actors do
				if type(self.saveTable.teamList[team].squadList[squad].Actors[actorIndex]) ~= "number" and IsActor(self.saveTable.teamList[team].squadList[squad].Actors[actorIndex]) then
					self.saveTable.teamList[team].squadList[squad].Actors[actorIndex] = self.saveTable.teamList[team].squadList[squad].Actors[actorIndex].UniqueID;
					print("turned back to uniqueID: " .. self.saveTable.teamList[team].squadList[squad].Actors[actorIndex]);
				end
			end
		end
	end

	print("saved tacticshandler!")
end

-- NO LONGER USED!
-- old system before the switch to UniqueID... desynced on the regular
function TacticsHandler:InvalidateActor(infoTable)

	print("tried to invalidate, table values:")
	for k, v in pairs(infoTable) do
		print(k)
		print(v)
	end

	self.saveTable.teamList[infoTable.Team].squadList[infoTable.squadIndex].Actors[infoTable.actorIndex] = false;
	--print("actor invalidated through function")
	
end

function TacticsHandler:ReapplyAllTasks()
	print("ReapplyAllTasks")

	for team = 0, #self.saveTable.teamList do
		for i = 1, #self.saveTable.teamList[team].squadList do
			local squad = self.saveTable.teamList[team].squadList[i];
			local taskName = squad.taskName
			if not (taskName and self:GetTaskByName(taskName, team)) then
				self:RetaskSquad(squad, team, false);
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
		for t = 1, #self.saveTable.teamList[team].taskList do
			if self.saveTable.teamList[team].taskList[t].Name == taskName then
				return self.saveTable.teamList[team].taskList[t];
			end
		end
	else
		print("Tacticshandler tried to get task " .. taskName .. " by name but wasn't given a team!");
		return false;
	end
	
	-- not actually an error, sometimes we just use this to verify
	--print("Tacticshandler could not find task named: " .. taskName .. " in team: " .. team);
	return false;

end

function TacticsHandler:PickTask(team)

	--print("PickTask")

	if #self.saveTable.teamList[team].taskList > 0 then
		-- random weighted select
		local totalPriority = 0;
		for t = 1, #self.saveTable.teamList[team].taskList do
			totalPriority = totalPriority + self.saveTable.teamList[team].taskList[t].Priority;
		end
		
		local randomSelect = math.random(1, totalPriority);
		local finalSelection = 1;
		for t = 1, #self.saveTable.teamList[team].taskList do
			randomSelect = randomSelect - self.saveTable.teamList[team].taskList[t].Priority;
			if randomSelect <= 0 then
				--print("gotfinalselection")
				finalSelection = t;
			end
		end
		
		return self.saveTable.teamList[team].taskList[finalSelection];
	else
		return false;
	end
	
end

function TacticsHandler:ApplyTaskToSquadActors(squad, task)
	if task then	
		--print("Applying Task:" .. task.Name)
		local randomPatrolPoint;
		if task.Type == "PatrolArea" then
			randomPatrolPoint = task.Position.RandomPoint;
			print("Task being applied is PatrolArea");
		end
		for actorIndex = 1, #squad do
			local actor = MovableMan:FindObjectByUniqueID(squad[actorIndex]);
			if actor then
				actor = ToActor(actor);
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
					--actor:SetMovePathToUpdate();
				elseif task.Type == "PatrolArea" then
					actor.AIMode = Actor.AIMODE_GOTO;
					actor:AddAISceneWaypoint(randomPatrolPoint);
					--print("officially changed ai mode")
					actor:UpdateMovePath();
					--actor:SetMovePathToUpdate();
				elseif task.Type == "Sentry" then
					actor.AIMode = Actor.AIMODE_SENTRY;
				else
					actor.AIMode = Actor.AIMODE_BRAINHUNT;
				end
				--print("during applying task to actors, this actor was iterated over: " .. actor.PresetName)
			end
		end
	else
		--print("couldnotfindttask")
		return false;
	end

	return true;
end

function TacticsHandler:RetaskSquad(squad, team, allowMerge)
	print("retasking squad with task name: " .. squad.taskName)

	local newTask = self:PickTask(team);
	if newTask then
	
		squad.idleTimer:Reset();
		squad.retaskTimer:Reset();
	
		-- iterate through all squads and see if any are under our minimumSquadActorCount
		-- if so, fold this one into that one instead of making a new squad, and apply the new task
		local squadToMergeInto;
		if allowMerge and squad.activeActorCount > 0 and #squad.Actors < self.maximumSquadActorCount then
			for k, checkedSquad in pairs(self.saveTable.teamList[team].squadList) do
				if checkedSquad.activeActorCount > 0 and checkedSquad.activeActorCount < self.minimumSquadActorCount then
					squadToMergeInto = checkedSquad;
					break;
				end
			end
		end
		
		if squadToMergeInto then
			for k, actor in pairs(squad.Actors) do
				table.insert(squadToMergeInto.Actors, actor);
				print("during retasking, this actor was moved: " .. actor);
			end
			squad.Actors = {}; -- will be wiped next update for being empty
			squad.activeActorCount = 0;
			print("during retasking, squads were merged");
			return self:ApplyTaskToSquadActors(squadToMergeInto.Actors, taskName);	
		else
			squad.taskName = newTask.Name;
			print("new task: " .. newTask.Name);
			return self:ApplyTaskToSquadActors(squad.Actors, newTask);
		end
	else
		return false;
	end
end

function TacticsHandler:RemoveTask(name, team)

	print("Removedtask: " .. name);

	if name and team then
		local task;
		local taskIndex;
		for i = 1, #self.saveTable.teamList[team].taskList do
			if self.saveTable.teamList[team].taskList[i].Name == name then
				task = self.saveTable.teamList[team].taskList[i];
				taskIndex = i;
				break;
			end
		end
		if task then
			table.remove(self.saveTable.teamList[team].taskList, taskIndex);
			--print("actuallydeletedttask")
			-- retask squads
			for i = 1, #self.saveTable.teamList[team].squadList do
				--print(self.saveTable.teamList[team].squadList[i].taskName)
				--print("task name to del: " .. task.Name);
				if self.saveTable.teamList[team].squadList[i].taskName == task.Name then
					--print("tried to retask")
					self:RetaskSquad(self.saveTable.teamList[team].squadList[i], team, false);
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

function TacticsHandler:AddTask(name, team, taskPos, taskType, priority, retaskTimeMultiplier)

	print("AddTask")
	
	if not retaskTimeMultiplier then
		retaskTimeMultiplier = 1;
	end
	
	local task;

	if name and team and taskPos then
	
		for i = 1, #self.saveTable.teamList[team].taskList do
			if self.saveTable.teamList[team].taskList[i].Name == name then
				--print("Tactics Handler tried to add a task to a specific team with a name that already existed!");
				return false;
			end
		end
	
		local retaskTime;
		if not taskType then
			taskType = "Attack";
			retaskTime = 120000;
		elseif taskType == "Attack" then
			retaskTime = 120000 * retaskTimeMultiplier;
		elseif taskType == "Defend" then
			retaskTime = 180000 * retaskTimeMultiplier;
		elseif taskType == "PatrolArea" then
			retaskTime = 100000 * retaskTimeMultiplier;
		elseif taskType == "Brainhunt" then
			retaskTime = 240000 * retaskTimeMultiplier;
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
		task.retaskTime = retaskTime;
		
		table.insert(self.saveTable.teamList[team].taskList, task);
		
		--print("Added new task with name: " .. task.Name)
		
	else
		print("Tactics Handler tried to add a task with no name, no team, or no task position!");
		return false;
	end
	
	return task;
	
end

function TacticsHandler:AddSquad(team, squadTable, taskName, applyTask, allowMerge)

	--print("AddTaskSquad" .. team)
	
	if not allowMerge == false then
		allowMerge = true;
	end
	
	if team and squadTable and taskName then
	
		if #squadTable == 0 then
			print("Tried to add a squad with no actors in it?!");
			return false;
		end
	
		-- iterate through all squads and see if any are under our minimumSquadActorCount
		-- if so, fold this one into that one instead of making a new squad, and apply the new task
		local squadToMergeInto;
		if allowMerge and #squadTable < self.maximumSquadActorCount then
			for k, checkedSquad in pairs(self.saveTable.teamList[team].squadList) do
				if checkedSquad.activeActorCount > 0 and checkedSquad.activeActorCount < self.minimumSquadActorCount then
					squadToMergeInto = checkedSquad;
					break;
				end
			end
		end
		
		if squadToMergeInto then
			for k, actor in pairs(squadTable) do
				table.insert(squadToMergeInto.Actors, actor.UniqueID);
				--print("during adding squad, this actor was merged");
				--print(actor)
			end
			print("newly added squad was instead merged into old one");
			self:ApplyTaskToSquadActors(squadToMergeInto.Actors, taskName);
		else
	
			local squadEntry = {};
			squadEntry.Actors = {};
			
			for k, actor in pairs(squadTable) do
				table.insert(squadEntry.Actors, actor.UniqueID);
			end
			
			squadEntry.taskName = taskName;
			squadEntry.activeActorCount = #squadTable;
			squadEntry.idleTimer = Timer();
			squadEntry.retaskTimer = Timer();
			local task = self:GetTaskByName(taskName, team);
			squadEntry.retaskTime = task.retaskTime;
			table.insert(self.saveTable.teamList[team].squadList, squadEntry); 
			
			if applyTask then
				self:ApplyTaskToSquadActors(squadEntry.Actors, taskName);
			end
			
		end
		
		-- old system before the switch to UniqueID... desynced on the regular
		
		-- for k, act in ipairs(squadEntry.Actors) do
			-- local squadInfo = {};
			-- squadInfo.Team = team;
			-- squadInfo.squadIndex = #self.saveTable.teamList[team].squadList;
			-- squadInfo.actorIndex = k;
			-- --print("added script and sent message")
			-- --print("task: " .. taskName)
			-- act:AddScript("Base.rte/Activities/Utility/TacticsActorInvalidator.lua");
			-- act:SendMessage("TacticsHandler_InitSquadInfo", squadInfo);
		-- end
		
		--print("addedtaskedsquad: " .. #self.saveTable.teamList[team].squadList)
		--print("newtaskname: " .. self.saveTable.teamList[team].squadList[#self.saveTable.teamList[team].squadList].taskName)
	else
		print("Tried to add a tasked squad without all required arguments!");
		return false;
	end
	
	return true;
	
end


-- NO LONGER USED!
-- old system before the switch to UniqueID... desynced on the regular
function TacticsHandler:CommunicateSquadIndexesToActors()

	for team, v in pairs(self.saveTable.teamList) do
		for squad = 1, #self.saveTable.teamList[team].squadList do
			for actorIndex = 1, #self.saveTable.teamList[team].squadList[squad].Actors do
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
	for i = #self.saveTable.teamList[team].squadList, 1, -1 do
		--print("checking squad: " .. i .. " of team: " .. team);
		local squad = self.saveTable.teamList[team].squadList[i];
		local task = self:GetTaskByName(squad.taskName, team);
		if task then
			
			local wholePatrolSquadIdle = true;
			squad.activeActorCount = 0;
			if #squad.Actors > 0 then
				for actorIndex = 1, #self.saveTable.teamList[team].squadList[i].Actors do
					--print("squad actor: " .. self.saveTable.teamList[team].squadList[i].Actors[actorIndex]);
				end
			end
			if #squad.Actors > 0 then
				for actorIndex = 1, #self.saveTable.teamList[team].squadList[i].Actors do
					if type(self.saveTable.teamList[team].squadList[i].Actors[actorIndex]) ~= "number" then
						print("ERROR: Tacticshandler UniqueID was not a number! Something's very broken.");
						print("It was: " .. self.saveTable.teamList[team].squadList[i].Actors[actorIndex]);
						break;
					end
					--print("uniqueID being checked:" .. self.saveTable.teamList[team].squadList[i].Actors[actorIndex]);
					local actor = MovableMan:FindObjectByUniqueID(self.saveTable.teamList[team].squadList[i].Actors[actorIndex]);
					--print(actor)
					if actor then
						local lastActor = actorIndex == #self.saveTable.teamList[team].squadList[i].Actors;
						squad.activeActorCount = squad.activeActorCount + 1;
						if actor.HasEverBeenAddedToMovableMan then
							actor = ToActor(actor);
						
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
								if actor.AIMode == Actor.AIMODE_SENTRY or (not actor.IsWaitingOnNewMovePath and dist:MagnitudeIsLessThan(40)) then
									actor.AIMode = Actor.AIMODE_SENTRY;
									if lastActor and wholePatrolSquadIdle == true then
										-- if we're the last one and the whole squad is ready to go
										--print("squad repatrolled")
										--self:ApplyTaskToSquadActors(self.saveTable.teamList[team].squadList[i].Actors, task)
									end
								else
									--print("squad: " .. i .. "patrolsquadnotfullyidle")
									wholePatrolSquadIdle = false;
								end
							end
							
							if task.Type ~= "Sentry" then
								if actor.AIMode ~= Actor.AIMODE_SENTRY then
									squad.idleTimer:Reset();
								end
							end
							
							if lastActor then
								if squad.idleTimer:IsPastSimMS(self.squadIdleTimeLimitMS) or squad.retaskTimer:IsPastSimMS(task.retaskTime) then
									self:RetaskSquad(squad, team, false);
								end
							end

							if not actor.IsWaitingOnNewMovePath and actor:GetLastAIWaypoint():IsZero() then
								-- our waypoint is 0, 0, so something's gone wrong
								--print("weirdwaypoint")
								--self:ApplyTaskToSquadActors(self.saveTable.teamList[team].squadList[i], task);
							end
						end
					end
				end
			end
			
			if squad.activeActorCount == 0 then
				print("removed wiped squad")
				table.remove(self.saveTable.teamList[team].squadList, i) -- squad wiped, remove it
				squadRemoved = true;
			end
		else
			print("retasking not actual task")
		
			self:RetaskSquad(self.saveTable.teamList[team].squadList[i], team, false);
		end
	end
	
	-- old system before the switch to UniqueID... desynced on the regular
	
	-- squad indexes have shifted if we've removed any, so tell all the actors that
	--if squadRemoved then
	--	self:CommunicateSquadIndexesToActors()
	--end

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