--------------------------------------- Instructions ---------------------------------------

------- Require this in your script like so: 

-- self.tacticsHandler = require("Activities/Utility/TacticsHandler");
-- self.tacticsHandler:Initialize(Activity, bool newGame, int minimumSquadActorCount, int maximumSquadActorCount, int squadIdleTimeLimitMS, bool verboseLogging);
-- The squadActorCount arguments are for automatic squad merging. Squads above the maximum will not merge ever, and squads below the minimum will merge into new squads if possible.
-- Verbose logging will also have info above every tasked actor's head. Alt-L to toggle.

-- TacticsHandler is pretty complicated and not for the uninitiated.

-- First off, call TacticsHandler:UpdateTacticsHandler() every frame.

-- To get started at a basic level, do TacticsHandler:AddTask(string name, int team, AreaOrVectorOrMO taskPos, string taskType, int priority, number retaskTimeMultiplier)
-- Sending an Area taskPos will randomly select a position from that Area, except in the case of PatrolArea where it will continuously select positions within it to patrol.
-- These should be custom Areas specifically for tacticsHandler that are close to the ground! Otherwise, actors will try to fly up to positions in the air.

-- Available taskTypes: Attack, Defend, PatrolArea, Brainhunt, Sentry
-- Attack will move towards an area, but retask quickly - Defend is the same but will stay there for more time.
-- PatrolArea will pick positions, move to them, then pick more positions. It MUST have an Area as its taskPos, or it will revert to Defend type.
-- Brainhunt will ignore taskPos and just brainhunt.
-- Sentry will ignore taskPos and indefinitely set the squad to sentry mode, and never retask. Use defend with a high retaskTimeMultiplier if you want them
-- to actually go somewhere.

-- After you've done that, you can do TacticsHandler:AddSquad(int team, table squadTable, string taskName, bool applyTask, bool allowMerge)
-- squadTable should be a table of AHumans and ACrabs that you would like to squad up.
-- taskName is the name of the task.
-- applyTask should be true in most cases: this is actually setting AI modes and stuff to make them act properly. In some cases you might not want to do that
-- so you can add a squad into the system without immediately making TacticsHandler give them things to do.
-- allowMerge disables or enables merging behavior, where an existing squad under the minimumSquadActorCount will be rolled into this new one you're adding.

-- You are now officially using TacticsHandler. This squad will automatically do the task you have specified and will even switch tasks periodically if more are available.

-- Instead of adding squads to specific tasks, you can also rely on TacticsHandler to suggest a random task from the ones you have added.
-- UpdateTacticsHandler will periodically return variables team, task. Note that variable "task" is the actual task table and to use it you should do task.Name.

-- To remove a task, do TacticsHandler:RemoveTask(string name, int team). All squads assigned to this task will be retasked to other tasks.
-- If no task is available the squad will keep doing the removed task, but as soon as one becomes available later it will be retasked then.

-- If for some reason you'd like to task a squad without adding them into the system, or to immediately update a squad's AI according to a task,
-- use TacticsHandler:ApplyTaskToSquadActors(squad, task). Variable squad should be a table of actors, and variable task should be an actual task table.

-- Get a task table from a task name via GetTaskByName(string name, int team), or PickTask(int team) which will give you a random one.

------- Saving/Loading

-- Saving and loading requires you to also have the SaveLoadHandler ready.
-- Simply run OnSave(instancedSaveLoadHandler) and OnLoad(instancedSaveLoadHandler) when appropriate.

--------------------------------------- Misc. Information ---------------------------------------

--




local TacticsHandler = {};

function TacticsHandler:Create()
	local Members = {};

	setmetatable(Members, self);
	self.__index = self;

	return Members;
end

function TacticsHandler:Initialize(activity, newGame, minimumSquadActorCount, maximumSquadActorCount, squadIdleTimeLimitMS, verboseLogging)

	if verboseLogging then
		self.verboseLogging = true;
	end
	
	self.Activity = activity;
	
	self.taskUpdateTimer = Timer();
	self.taskUpdateDelay = 1000;
	
	self.teamToCheckNext = 0;
	
	self.debugTextDisabled = true;
	
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
	
	print("INFO: TacticsHandler initialized!")
	
end

function TacticsHandler:OnMessage(message, object)
	
end

function TacticsHandler:OnLoad(saveLoadHandler)
	
	print("INFO: TacticsHandler loading...");
	
	self.saveTable = saveLoadHandler:ReadSavedStringAsTable("tacticsHandlerSaveTable");
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
	
	print("INFO: TacticsHandler loaded!");
	
	self:ReapplyAllTasks();
	
end

function TacticsHandler:OnSave(saveLoadHandler)
	print("INFO: TacticsHandler saving...");
	-- saving/loading destroys all not-in-sim entities forever
	-- fugg :DD
	-- salvage what we can, resolve our uniqueids into MOs that the saveloadhandler can handle at least

	
	for team = 0, #self.saveTable.teamList do
		for squad = 1, #self.saveTable.teamList[team].squadList do
			if self.verboseLogging then
				print("INFO: TacticsHandler is trying to save squad: " .. squad .. " of team " .. team);
			end
			for actorIndex = 1, #self.saveTable.teamList[team].squadList[squad].Actors do
				if type(self.saveTable.teamList[team].squadList[squad].Actors[actorIndex]) ~= "number" then
					print("ERROR: TacticsHandler UniqueID during saving was not a number! Something's very broken.");
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
					
	saveLoadHandler:SaveTableAsString("tacticsHandlerSaveTable", self.saveTable);
	
	-- and now so we can actually keep playing...
	-- turn them back, lol
	
	for team = 0, #self.saveTable.teamList do
		for squad = 1, #self.saveTable.teamList[team].squadList do
			for actorIndex = 1, #self.saveTable.teamList[team].squadList[squad].Actors do
				if type(self.saveTable.teamList[team].squadList[squad].Actors[actorIndex]) ~= "number" and IsActor(self.saveTable.teamList[team].squadList[squad].Actors[actorIndex]) then
					self.saveTable.teamList[team].squadList[squad].Actors[actorIndex] = self.saveTable.teamList[team].squadList[squad].Actors[actorIndex].UniqueID;
				end
			end
		end
	end

	print("INFO: TacticsHandler saved!");
end

function TacticsHandler:ReapplyAllTasks()
	if self.verboseLogging then
		print("INFO: TacticsHandler is reapplying all tasks.");
	end

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
		print("ERROR: TacticsHandler tried to get task " .. taskName .. " by name but wasn't given a team!");
		return false;
	end
	
	-- not actually an error, sometimes we just use this to verify
	--print("Tacticshandler could not find task named: " .. taskName .. " in team: " .. team);
	return false;

end

function TacticsHandler:PickTask(team, updateCall)

	if self.verboseLogging and not updateCall then
		print("INFO: TacticsHandler is picking a task for team: " .. team);
	end

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
		if self.verboseLogging and not updateCall then
			print("INFO: TacticsHandler found task " .. self.saveTable.teamList[team].taskList[finalSelection].Name .. " when picking a task for team: " .. team);
		end
		return self.saveTable.teamList[team].taskList[finalSelection];
	else
		if self.verboseLogging then
			print("INFO: TacticsHandler found no task when picking a task for team: " .. team);
		end
		return false;
	end
	
end

function TacticsHandler:ApplyTaskToSquadActors(squad, task)
	if squad and task then	
		if self.verboseLogging then
			local taskType = "None, defaulting to Brainhunt";
			if task.Type then taskType = task.Type; end
			print("INFO: TacticsHandler is applying task " .. task.Name .. " of type " .. taskType);
		end
		local randomPatrolPoint;
		if task.Type == "PatrolArea" then
			randomPatrolPoint = task.Position.RandomPoint;
		end
		for actorIndex = 1, #squad do
			local actor = MovableMan:FindObjectByUniqueID(squad[actorIndex]);
			if actor then
				actor = ToActor(actor);
				if self.verboseLogging then
					actor:FlashWhite(1000);
				end
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
		print("ERROR: TacticsHandler tried to task a squad's actors, but was not given a squad or a task!");
		return false;
	end

	return true;
end

function TacticsHandler:RetaskSquad(squad, team, allowMerge)
	if self.verboseLogging then
		print("INFO: TacticsHandler is retasking a squad of team " .. team .. " with current task name of " .. squad.taskName);
	end

	local newTask = self:PickTask(team);
	if newTask then
	
		if self.verboseLogging then
			print("INFO: TacticsHandler chose new task: " .. newTask.Name);
		end
	
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
			if self.verboseLogging then
				print("INFO: TacticsHandler merged down a squad being retasked.");
			end
			for k, actor in pairs(squad.Actors) do
				table.insert(squadToMergeInto.Actors, actor);
			end
			squad.Actors = {}; -- will be wiped next update for being empty
			squad.activeActorCount = 0;
			return self:ApplyTaskToSquadActors(squadToMergeInto.Actors, taskName);	
		else
			squad.taskName = newTask.Name;
			return self:ApplyTaskToSquadActors(squad.Actors, newTask);
		end
	else
		if self.verboseLogging then
			print("INFO: TacticsHandler could not find a new task when retasking.");
		end
		return false;
	end
end

function TacticsHandler:RemoveTask(name, team)

	if name and team then
		if self.verboseLogging then
			print("INFO: TacticsHandler is removing task " .. name .. " from team " .. team);
		end
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
			if self.verboseLogging then
				print("INFO: TacticsHandler tried to remove task " .. name .. " but it did not exist.");
			end
			return false;
		end
	else
		print("ERROR: TacticsHandler was asked to remove a task, but not given a name or a team!");
		return false;
	end
	
	return true;
	
end

function TacticsHandler:AddTask(name, team, taskPos, taskType, priority, retaskTimeMultiplier)
	
	if not retaskTimeMultiplier then
		retaskTimeMultiplier = 1;
	end
	
	local task;

	if name and team and taskPos then
	
		for i = 1, #self.saveTable.teamList[team].taskList do
			if self.saveTable.teamList[team].taskList[i].Name == name then
				print("ERROR: TacticsHandler tried to add task " .. name .. " to team " .. team .. " but it was already there!");
				return false;
			end
		end
		
		if self.verboseLogging then
			print("INFO: TacticsHandler is adding the following task:");
			print("Name: " .. name);
			print("Team: " .. team);
			if taskPos.PresetName then
				print("Task position: MO " .. taskPos.PresetName);	
			elseif taskPos.Name then
				print("Task position: Area " .. taskPos.Name);
			else
				print("Task position: " .. taskPos.X .. ", " .. taskPos.Y);
			end
			print("Task type: " .. taskType);
			print("Task priority: " .. priority);
			print("Task retask time mult.: " .. retaskTimeMultiplier);
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
			if not taskPos.RandomPoint then
				print("WARNING: TacticsHandler added task is type PatrolArea, but its position is not an Area. Adding it as type Defend instead...");
				taskType = "Defend";
				retaskTime = 180000 * retaskTimeMultiplier;
			end
		elseif taskType == "Brainhunt" then
			retaskTime = 240000 * retaskTimeMultiplier;
		end
		
		if not priority then
			priority = 10;
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
		
		if self.verboseLogging then
			print("INFO: TacticsHandler successfully added above task.");
		end
		
	else
		print("ERROR: TacticsHandler tried to add a task with no name, no team, or no task position!");
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
	
		if not self:GetTaskByName(taskName, team) then
			print("ERROR: TacticsHandler tried to add a squad with non-existent task name " .. taskName .. "!");
			return false;
		end
	
		if #squadTable == 0 then
			print("ERROR: TacticsHandler tried to add a squad with no actors in it!");
			return false;
		end
		
		if self.verboseLogging then
			print("INFO: TacticsHandler is adding a squad with the following task name: " .. taskName);
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
			if self.verboseLogging then
				print("INFO: TacticsHandler merged a newly added squad into an old one instead of adding it. New task name: " .. taskNmae);
			end
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
				self:ApplyTaskToSquadActors(squadEntry.Actors, task);
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
		print("ERROR: TacticsHandler tried to add a squad without an actor table, a team, or a task name!");
		return false;
	end
	
	return true;
	
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
						print("ERROR: Tacticshandler UniqueID during update was not a number! Something's very broken.");
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
						
							if self.verboseLogging then
								actor:FlashWhite(100);
							end
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
										if self.verboseLogging then
											print("INFO: TacticsHandler re-patrolled squad " .. i .. " of team " .. team);
										end
										self:ApplyTaskToSquadActors(self.saveTable.teamList[team].squadList[i].Actors, task)
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
									if self.verboseLogging then
										print("INFO: TacticsHandler retasked squad " .. i .. " of team " .. team .. " due to idleness or retask timing.");
									end
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
				if self.verboseLogging then
					print("INFO: TacticsHandler removed dead squad " .. i .. " of team " .. team);
				end
				table.remove(self.saveTable.teamList[team].squadList, i) -- squad wiped, remove it
				squadRemoved = true;
			end
		else
			if self.verboseLogging then
				print("INFO: TacticsHandler retasked squad " .. i .. " of team " .. team .. " due to expired task " .. squad.taskName);
			end
		
			self:RetaskSquad(self.saveTable.teamList[team].squadList[i], team, false);
		end
	end

end

function TacticsHandler:UpdateTacticsHandler()


	if self.taskUpdateTimer:IsPastSimMS(self.taskUpdateDelay) then
		self.taskUpdateTimer:Reset();

		local team = self.teamToCheckNext;
		
		self.teamToCheckNext = (self.teamToCheckNext + 1) % self.Activity.TeamCount;
		
		-- check and update all tasked squads
		
		self:UpdateSquads(team);
		
		local task = self:PickTask(team, true);
		if task then
			return team, task;
		else

		end
		
	end
	
	-- Printing info for everyone
	if self.verboseLogging then
		
		if UInputMan.FlagAltState then

			if UInputMan:KeyPressed(Key.L) then
				if not self.debugTextDisabled then
					self.debugTextDisabled = true;
				else
					self.debugTextDisabled = false;
				end
			end
		end
	
		if not self.debugTextDisabled then
			for team, v in pairs(self.saveTable.teamList) do

				for i = 1, #self.saveTable.teamList[team].squadList do
					--print("checking squad: " .. i .. " of team: " .. team);
					local squad = self.saveTable.teamList[team].squadList[i];
					local task = self:GetTaskByName(squad.taskName, team);
					if task then

						if #squad.Actors > 0 then
							for actorIndex = 1, #self.saveTable.teamList[team].squadList[i].Actors do
								local actor = MovableMan:FindObjectByUniqueID(self.saveTable.teamList[team].squadList[i].Actors[actorIndex]);
								if actor then
									local dist = SceneMan:ShortestDistance(actor.Pos, ToActor(actor):GetLastAIWaypoint(), SceneMan.SceneWrapsX).Magnitude;
									PrimitiveMan:DrawTextPrimitive(actor.Pos + Vector(0, -80), "Squad: " .. tostring(i), true, 1);
									PrimitiveMan:DrawTextPrimitive(actor.Pos + Vector(0, -70), "Actor index: " .. tostring(actorIndex), true, 1);
									PrimitiveMan:DrawTextPrimitive(actor.Pos + Vector(0, -60), "Task name: " .. tostring(squad.taskName), true, 1);
									PrimitiveMan:DrawTextPrimitive(actor.Pos + Vector(0, -50), "AI mode: " .. ToActor(actor).AIMode, true, 1);
									PrimitiveMan:DrawTextPrimitive(actor.Pos + Vector(0, -40), "Dist to waypoint: " .. tostring(math.floor(dist)), true, 1);
								end
							end
						end
					end
				end
			end
		end
	end
	
	return false;
		

end

return TacticsHandler:Create();