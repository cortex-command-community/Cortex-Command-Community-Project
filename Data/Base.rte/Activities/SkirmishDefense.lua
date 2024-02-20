function SkirmishDefense:StartActivity(isNewGame)
	-- Count CPU teams
	self.CPUTeamCount = 0;
	for team = 0, Activity.MAXTEAMCOUNT - 1 do
		if self:TeamActive(team) and self:TeamIsCPU(team) then
			self.CPUTeamCount = self.CPUTeamCount + 1;
		end
	end

	-- Add a CPU team if we only have one team
	if self.CPUTeamCount == 0 then
		local activeTeams = 0;
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self:PlayerHuman(player) then
				activeTeams = activeTeams + 1;
			end
		end

		if activeTeams < 2 then
			local cputeam = -1;

			for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
				if self:PlayerActive(player) and self:PlayerHuman(player) then
					cputeam = self:GetTeamOfPlayer(player) + 1;
					if cputeam > Activity.TEAM_4 then
						cputeam = Activity.TEAM_1;
					end
				end
			end

			self.CPUTeam = cputeam;
			self.CPUTeamCount = 1;
		end
	end

	local aiTeams = {};
	for team = 0, Activity.MAXTEAMCOUNT - 1 do
		if self:TeamActive(team) and self:TeamIsCPU(team) then
			table.insert(aiTeams, team);
		end
	end

	-- Store data about terrain and enemy actors in the LZ map, use it to pick safe landing zones
	self.LZmap = require("Activities/Utility/LandingZoneMap");
	self.LZmap:Initialize(aiTeams);

	self.startTimer = Timer();

	self.AI = {};
	for _, team in ipairs(aiTeams) do
		self.AI[team] = {};
		-- Store data about player teams: self.AI[team].OnPlayerTeam[Act.Team] is true if "Act" is an enemy to the AI
		self.AI[team].OnPlayerTeam = {};
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self:PlayerActive(player) and self:PlayerHuman(player) then
				self.AI[team].OnPlayerTeam[self:GetTeamOfPlayer(player)] = true;
			end
		end

		self.AI[team].TechID = PresetMan:GetModuleID(self:GetTeamTech(team));
	end

	if isNewGame then
		self:StartNewGame(aiTeams);
	else
		self:ResumeLoadedGame(aiTeams);
	end
end

function SkirmishDefense:OnSave()
	self:SaveNumber("endless", self.endless and 1 or 0);

	self:SaveNumber("startTimer.ElapsedSimTimeMS", self.startTimer.ElapsedSimTimeMS);

	for team, aiData in pairs(self.AI) do
		local teamString = "ai." .. tostring(team) .. ".";
		for aiDataKey, aiDataValue in pairs(aiData) do
			local keyToSave = teamString .. aiDataKey;
			if type(aiDataValue) == "number" then
				self:SaveNumber(keyToSave, aiDataValue);
			elseif type(aiDataValue) == "boolean" then
				self:SaveNumber(keyToSave, aiDataValue and 1 or 0);
			elseif type(aiDataValue) == "string" then
				self:SaveString(keyToSave, aiDataValue);
			end
		end

		self:SaveNumber(teamString .. "SpawnTimer.ElapsedSimTimeMS", aiData.SpawnTimer.ElapsedSimTimeMS);
		self:SaveNumber(teamString .. "BombTimer.ElapsedSimTimeMS", aiData.BombTimer.ElapsedSimTimeMS);
		self:SaveNumber(teamString .. "HuntTimer.ElapsedSimTimeMS", aiData.HuntTimer.ElapsedSimTimeMS);
		self:SaveNumber(teamString .. "EngineerTimer.ElapsedSimTimeMS", aiData.EngineerTimer.ElapsedSimTimeMS);
	end
end

function SkirmishDefense:StartNewGame(aiTeams)
	self.addFogOfWar = self:GetFogOfWarEnabled();

	-- Set all actors defined in the ini-file to sentry mode
	for actor in MovableMan.AddedActors do
		if actor.ClassName == "AHuman" or actor.ClassName == "ACrab" then
			actor.AIMode = Actor.AIMODE_SENTRY;
		end
	end

	local soleHumanTeam = -1;
	for team = 0, Activity.MAXTEAMCOUNT - 1 do
		self:SetTeamFunds(self:GetStartingGold(), team);

		if self:TeamActive(team) and not self:TeamIsCPU(team) then
			soleHumanTeam = soleHumanTeam == -1 and team or false;
		end
	end
	-- If there's only one Human team, set all existing doors and actors to that team
	if soleHumanTeam ~= false and soleHumanTeam >= 0 then
		for actor in MovableMan.AddedActors do
			if actor.Team ~= soleHumanTeam then
				MovableMan:ChangeActorTeam(actor, soleHumanTeam);
			end
		end
	end

	-- Setup AI data variables
	for _, team in ipairs(aiTeams) do
		self.AI[team].defeated = false;
		self.AI[team].bombChance = math.min(math.max(self.Difficulty/100, 0), 0.95);
		self.AI[team].SpawnTimer = Timer();
		self.AI[team].BombTimer = Timer();
		self.AI[team].HuntTimer = Timer();
		self.AI[team].EngineerTimer = Timer();
		self.AI[team].timeToSpawn = 8000 - 50 * self.Difficulty;			-- Time before the first AI spawn: from 8s to 3s
		self.AI[team].timeToBomb = 60000 - 400 * self.Difficulty;			-- From 60s to 20s
		self.AI[team].timeToEngineer = 60000 - 300 * self.Difficulty;		-- From 60s to 30s
		self.AI[team].baseSpawnTime = 16000 - 50 * self.Difficulty;			-- From 16s to 10s
		self.AI[team].randomSpawnTime = 8000 - 40 * self.Difficulty;		-- From 8s to 4s
		self.AI[team].digToBrainProbability = 0;

		if self.Difficulty > 55 then
			self.AI[team].digToBrainProbability = self.Difficulty / 320;
		end

		-- Switch to endless mode
		if self:GetStartingGold() > 100000 then
			self.endless = true;
			self:SetTeamFunds(100000, team);
		else
			self:SetTeamFunds(80 * self.Difficulty + 2000, team); -- AI team gold: from 2k to 10k
		end
	end

	self:SetupHumanPlayerBrains();
end

function SkirmishDefense:SetupHumanPlayerBrains()
	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) then
			-- Check if we already have a brain assigned
			if not self:GetPlayerBrain(player) then
				local foundBrain = MovableMan:GetUnassignedBrain(self:GetTeamOfPlayer(player));
				-- If we can't find an unassigned brain in the scene to give each player, then force to go into editing mode to place one
				if not foundBrain then
					self.ActivityState = Activity.EDITING;
					-- Open all doors so we can do pathfinding through them with the brain placement
					AudioMan:ClearMusicQueue();
					AudioMan:PlayMusic("Base.rte/Music/dBSoundworks/ccambient4.ogg", -1, -1);
					self:SetLandingZone(Vector(player*SceneMan.SceneWidth/4, 0), player);
				else
					-- Set the found brain to be the selected actor at start
					self:SetPlayerBrain(foundBrain, player);
					self:SwitchToActor(foundBrain, player, self:GetTeamOfPlayer(player));
					self:SetLandingZone(self:GetPlayerBrain(player).Pos, player);
					-- Set the observation target to the brain, so that if/when it dies, the view flies to it in observation mode
					self:SetObservationTarget(self:GetPlayerBrain(player).Pos, player);
				end
			end
		end
	end
end

function SkirmishDefense:ResumeLoadedGame(aiTeams)
	self.endless = self:LoadNumber("endless") ~= 0;

	self.startTimer.ElapsedSimTimeMS = self:LoadNumber("startTimer.ElapsedSimTimeMS");

	local loadAIData = function(self, team, aiDataKey, aiDataValueType, doReturnInsteadOfSettingValue)
		local keyToLoad = "ai." .. tostring(team) .. "." .. aiDataKey;
		local loadedValue;
		if aiDataValueType == "number" then
			loadedValue = self:LoadNumber(keyToLoad);
		elseif aiDataValueType == "boolean" then
			loadedValue = self:LoadNumber(keyToLoad) ~= 0;
		elseif aiDataValueType == "string" then
			loadedValue = self:LoadString(keyToLoad);
		end
		if doReturnInsteadOfSettingValue then
			return loadedValue;
		else
			self.AI[team][aiDataKey] = loadedValue;
		end
	end

	for _, team in ipairs(aiTeams) do
		self.AI[team].SpawnTimer = Timer();
		self.AI[team].SpawnTimer.ElapsedSimTimeMS = loadAIData(self, team, "SpawnTimer.ElapsedSimTimeMS", "number", true);
		self.AI[team].BombTimer = Timer();
		self.AI[team].BombTimer.ElapsedSimTimeMS = loadAIData(self, team, "BombTimer.ElapsedSimTimeMS", "number", true);
		self.AI[team].HuntTimer = Timer();
		self.AI[team].HuntTimer.ElapsedSimTimeMS = loadAIData(self, team, "HuntTimer.ElapsedSimTimeMS", "number", true);
		self.AI[team].EngineerTimer = Timer();
		self.AI[team].EngineerTimer.ElapsedSimTimeMS = loadAIData(self, team, "EngineerTimer.ElapsedSimTimeMS", "number", true);

		loadAIData(self, team, "defeated", "boolean");
		loadAIData(self, team, "bombChance", "number");
		loadAIData(self, team, "timeToSpawn", "number");
		loadAIData(self, team, "timeToBomb", "number");
		loadAIData(self, team, "timeToEngineer", "number");
		loadAIData(self, team, "baseSpawnTime", "number");
		loadAIData(self, team, "randomSpawnTime", "number");
		loadAIData(self, team, "digToBrainProbability", "number");
	end
end

function SkirmishDefense:EndActivity()
	-- Temp fix so music doesn't start playing if ending the Activity when changing resolution through the ingame settings.
	if not self:IsPaused() then
		-- Play sad music if no humans are left
		if self:HumanBrainCount() == 0 then
			AudioMan:ClearMusicQueue();
			AudioMan:PlayMusic("Base.rte/Music/dBSoundworks/udiedfinal.ogg", 2, -1.0);
			AudioMan:QueueSilence(10);
			AudioMan:QueueMusicStream("Base.rte/Music/dBSoundworks/ccambient4.ogg");
		else
			-- But if humans are left, then play happy music!
			AudioMan:ClearMusicQueue();
			AudioMan:PlayMusic("Base.rte/Music/dBSoundworks/uwinfinal.ogg", 2, -1.0);
			AudioMan:QueueSilence(10);
			AudioMan:QueueMusicStream("Base.rte/Music/dBSoundworks/ccambient4.ogg");
		end
	end
end


function SkirmishDefense:UpdateActivity()
	if self.ActivityState == Activity.OVER then
		return;
	end

	if self.ActivityState == Activity.EDITING then
		self.startTimer:Reset();
	else
		if not self.startTimer:IsPastSimMS(500) then
			-- Make sure all actors are in sentry mode
			for Act in MovableMan.Actors do
				if Act.ClassName == "AHuman" or Act.ClassName == "ACrab" then
					Act.AIMode = Actor.AIMODE_SENTRY;
				end
			end

			for team = 0, Activity.MAXTEAMCOUNT - 1 do
				if self:TeamActive(team) and self:TeamIsCPU(team) then
					if self.AI[team] then
						self.AI[team].SpawnTimer:Reset();
						self.AI[team].BombTimer:Reset();
						self.AI[team].EngineerTimer:Reset();
					end
				end
			end

			-- Add fog of war once the game is no longer in editing mode.
			if self.addFogOfWar then
				self.addFogOfWar = false;

				for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
					if self:PlayerActive(player) and self:PlayerHuman(player) then
						SceneMan:MakeAllUnseen(Vector(20, 20), self:GetTeamOfPlayer(player));
					end
				end

				for team = 0, Activity.MAXTEAMCOUNT - 1 do
					if self:TeamActive(team) and self:TeamIsCPU(team) then
						SceneMan:MakeAllUnseen(Vector(65, 65), team);
					end
				end

				for Act in MovableMan.AddedActors do
					if Act.ClassName ~= "ADoor" then
						for ang = 0, math.pi*2, 0.15 do
							SceneMan:CastSeeRay(Act.Team, Act.EyePos, Vector(30+FrameMan.PlayerScreenWidth*0.5, 0):RadRotate(ang), Vector(), 1, 5);
						end
					end
				end

				--[[for Act in MovableMan.Actors do
					if Act.ClassName ~= "ADoor" then
						for ang = 0, math.pi*2, 0.15 do
							SceneMan:CastSeeRay(Act.Team, Act.EyePos, Vector(30+FrameMan.PlayerScreenWidth*0.5, 0):RadRotate(ang), Vector(), 1, 5);
						end
					end
				end--]]
			end
		end

		-- Clear all objective markers, they get re-added each frame
		self:ClearObjectivePoints();
		-- Keep track of which teams we have set objective points for already, since multiple players can be on the same team
		local setTeam = { [Activity.TEAM_1] = false, [Activity.TEAM_2] = false, [Activity.TEAM_3] = false, [Activity.TEAM_4] = false };
		local teamtally = 0;
		local playertally = 0;
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self:PlayerActive(player) and self:PlayerHuman(player) then
				if not self.startTimer:IsPastSimMS(3000) then
					FrameMan:SetScreenText("Survive the assault!", self:ScreenOfPlayer(player), 333, 5000, true);
				end
				-- The current player's team
				local team = self:GetTeamOfPlayer(player);

				-- If player brain is dead then try to find another, maybe he just entered craft
				if not MovableMan:IsActor(self:GetPlayerBrain(player)) then
					local newBrain = MovableMan:GetUnassignedBrain(self:GetTeamOfPlayer(player));
					if newBrain then
						self:SetPlayerBrain(newBrain, player);
						self:SwitchToActor(newBrain, player, self:GetTeamOfPlayer(player));
						self:SetLandingZone(self:GetPlayerBrain(player).Pos, player);
						self:SetObservationTarget(self:GetPlayerBrain(player).Pos, player);
					end
				end

				-- Check if any player's brain is dead and we could not find another
				if not MovableMan:IsActor(self:GetPlayerBrain(player)) then
					self:SetPlayerBrain(nil, player);
					self:ResetMessageTimer(player);
					FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
					FrameMan:SetScreenText("Your brain has been destroyed!", self:ScreenOfPlayer(player), 333, -1, false);
				else
					playertally = playertally + 1;
					if not setTeam[team] then
						-- Add objective points
						self:AddObjectivePoint("Protect!", self:GetPlayerBrain(player).AboveHUDPos, team, GameActivity.ARROWDOWN);
						for otherPlayer = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
							if otherPlayer ~= player and self:PlayerActive(otherPlayer) and self:PlayerHuman(otherPlayer) and MovableMan:IsActor(self:GetPlayerBrain(otherPlayer)) then
								local otherTeam = self:GetTeamOfPlayer(otherPlayer);
								if otherTeam ~= team then
									self:AddObjectivePoint("Destroy!", self:GetPlayerBrain(otherPlayer).AboveHUDPos, team, GameActivity.ARROWDOWN);
								else
									self:AddObjectivePoint("Protect!", self:GetPlayerBrain(otherPlayer).AboveHUDPos, team, GameActivity.ARROWDOWN);
								end
							end
						end

						setTeam[team] = true;
						teamtally = teamtally + 1;
					end
				end
			end
		end

		-- Win/Lose Conditions player vs player
		if self.CPUTeamCount == 0 then
			if teamtally < 2 then
				for team = Activity.TEAM_1, Activity.TEAM_4 do
					if setTeam[team] then
						self.WinnerTeam = team;
						break;
					end
				end

				ActivityMan:EndActivity();
				return;
			end
		else
			-- Win/Lose Conditions player vs AI
			if playertally < 1 then
				for team = 0, Activity.MAXTEAMCOUNT - 1 do
					if self:TeamActive(team) and self:TeamIsCPU(team) then
						self.WinnerTeam = team;
						ActivityMan:EndActivity();
						return;
					end
				end
			end

			-- Win/Lose conditions multiple CPU's
			if self.CPUTeamCount > 0 then
				local survivedAIs = 0;
				for team = Activity.TEAM_1, Activity.TEAM_4 do
					if self:TeamActive(team) and self:TeamIsCPU(team) then
						if not self.AI[team].defeated then
							survivedAIs = survivedAIs + 1;
						end
					end
				end

				if survivedAIs == 0 then
					self.CPUTeamCount = 0;
				end
			end

			for team = 0, Activity.MAXTEAMCOUNT - 1 do
				if self:TeamActive(team) and self:TeamIsCPU(team) then
					self.LZmap:Update(); -- Update info about landing zones and player actors

					-- Check if any AI actors have reached their destination
					if self.AI[team].HuntTimer:IsPastSimMS(8000) then
						self.AI[team].HuntTimer:Reset();

						for Act in MovableMan.Actors do
							if Act.Team == team and Act.AIMode ~= Actor.AIMODE_GOLDDIG and (Act.ClassName == "AHuman" or Act.ClassName == "ACrab") then
								if (Act.AIMode == Actor.AIMODE_GOTO and SceneMan:ShortestDistance(Act:GetLastAIWaypoint(), Act.Pos, false).Largest < 100) or Act.AIMode == Actor.AIMODE_SENTRY or Act.Age > 80000 then
									-- Destination reached: hunt for the brain
									Act.AIMode = Actor.AIMODE_BRAINHUNT;
								end
							end
						end
					end

					-- The AI have money to buy units
					if self:GetTeamFunds(team) > 0 then
						if self.AI[team].SpawnTimer:IsPastSimMS(self.AI[team].timeToSpawn) then
							if self.AI[team].AttackPos then	-- Search for a LZ from where to attack the target
								if self.AI[team].DigToBrain then
									local easyPathLZx, easyPathLZobst, closeLZx, closeLZobst = self.LZmap:FindLZ(team, self.AI[team].AttackPos, 200); -- Heavy digger
									if easyPathLZx then	-- Search done
										self.AI[team].SpawnTimer:Reset();
										self.AI[team].digToBrainProbability = self.AI[team].digToBrainProbability * 0.4;
										self.AI[team].timeToSpawn = (self.AI[team].baseSpawnTime + math.random(self.AI[team].randomSpawnTime)) * rte.SpawnIntervalScale;
										if closeLZx and math.random() < 0.5 then
											self:CreateBreachDrop(closeLZx, team);
										else
											self:CreateBreachDrop(easyPathLZx, team);
										end

										-- Search for another target
										self.AI[team].AttackTarget = nil;
										self.AI[team].AttackPos = nil;
										self.AI[team].DigToBrain = nil;
									end
								else
									local easyPathLZx, easyPathLZobst, closeLZx, closeLZobst = self.LZmap:FindLZ(team, self.AI[team].AttackPos);
									if easyPathLZx then	-- Search done
										self.AI[team].SpawnTimer:Reset();
										if self.AI[team].digToBrainProbability > 0 then
											self.AI[team].digToBrainProbability = math.min(self.AI[team].digToBrainProbability+0.03, self.Difficulty/320);
										end

										local xPosLZ, obstacleHeight;
										if closeLZobst < 25 and easyPathLZobst < 25 then
											if math.random() < 0.6 then
												xPosLZ = closeLZx;
												obstacleHeight = closeLZobst;
											else
												xPosLZ = easyPathLZx;
												obstacleHeight = easyPathLZobst;
											end
										elseif closeLZobst > 100 then
											xPosLZ = easyPathLZx;
											obstacleHeight = easyPathLZobst;
										else
											if math.random() < 0.4 then
												xPosLZ = closeLZx;
												obstacleHeight = closeLZobst;
											else
												xPosLZ = easyPathLZx;
												obstacleHeight = easyPathLZobst;
											end
										end

										if obstacleHeight > 200 and math.random() < 0.6 then
											if math.random() < self.Difficulty/111 then
												self:CreateBreachDrop(xPosLZ, team); -- ~90% at max difficulty
												self.AI[team].timeToSpawn = (self.AI[team].baseSpawnTime + math.random(self.AI[team].randomSpawnTime)) * rte.SpawnIntervalScale;
											else
												self.AI[team].timeToSpawn = 500;
											end

											-- This target is very difficult to reach: cancel this attack and search for another target again soon
											self.AI[team].AttackTarget = nil;
											self.AI[team].AttackPos = nil;
										else
											self.AI[team].timeToSpawn = (self.AI[team].baseSpawnTime + math.random(self.AI[team].randomSpawnTime)) * rte.SpawnIntervalScale;

											if obstacleHeight < 30 then
												self:CreateHeavyDrop(xPosLZ, self.AI[team].AttackPos, team);
											elseif obstacleHeight < 100 then
												self:CreateMediumDrop(xPosLZ, self.AI[team].AttackPos, team);
											elseif obstacleHeight < 200 then
												self:CreateLightDrop(xPosLZ, self.AI[team].AttackPos, team);
											else
												self:CreateScoutDrop(xPosLZ, self.AI[team].AttackPos, team);

												-- This target is very difficult to reach: change target for the next attack
												self.AI[team].AttackTarget = nil;
												self.AI[team].AttackPos = nil;
											end

											if not MovableMan:IsActor(self.AI[team].AttackTarget) or math.random() < 0.4 then
												-- Change target for the next attack
												self.AI[team].AttackTarget = nil;
												self.AI[team].AttackPos = nil;
											else
												self.AI[team].AttackPos = Vector(self.AI[team].AttackTarget.Pos.X, self.AI[team].AttackTarget.Pos.Y);
											end
										end
									end
								end
							else	-- Select a player actor as a target for the next attack
								local safePosX = self.LZmap:FindSafeLZ(team) or math.random(SceneMan.SceneWidth-1);
								if safePosX then
									if math.random() < self.AI[team].digToBrainProbability then	-- Try digging straight to the brain
										local TargetActors = {};
										for Act in MovableMan.Actors do
											if self.AI[team].OnPlayerTeam[Act.Team] and Act:IsInGroup("Brains") then
												local distance = 20 * (SceneMan:ShortestDistance(Vector(safePosX, Act.Pos.Y), Act.Pos, false).Largest / SceneMan.SceneWidth);
												table.insert(TargetActors, {Act=Act, score=self.LZmap:SurfaceProximity(Act.Pos)+distance});
											end
										end

										self.AI[team].AttackTarget = self:SelectTarget(TargetActors);
										if self.AI[team].AttackTarget then
											self.AI[team].DigToBrain = true;
											self.AI[team].AttackPos = Vector(self.AI[team].AttackTarget.Pos.X, self.AI[team].AttackTarget.Pos.Y);
										else
											-- No target found
											self.AI[team].SpawnTimer:Reset();
											self.AI[team].timeToSpawn = 5000;
											self.AI[team].digToBrainProbability = 0;
										end
									else
										local TargetActors = {};
										for Act in MovableMan.Actors do
											if self.AI[team].OnPlayerTeam[Act.Team] and (Act.ClassName == "AHuman" or Act.ClassName == "ACrab" or Act.ClassName == "Actor") then
												local distance = 20 * (SceneMan:ShortestDistance(Vector(safePosX, Act.Pos.Y), Act.Pos, false).Largest / SceneMan.SceneWidth);
												table.insert(TargetActors, {Act=Act, score=self.LZmap:SurfaceProximity(Act.Pos)+distance});
											end
										end

										self.AI[team].AttackTarget = self:SelectTarget(TargetActors);
										if self.AI[team].AttackTarget then
											self.AI[team].AttackPos = Vector(self.AI[team].AttackTarget.Pos.X, self.AI[team].AttackTarget.Pos.Y);
										else
											-- No target found
											self.AI[team].SpawnTimer:Reset();
											self.AI[team].timeToSpawn = 5000;
										end
									end
								else
									-- No safe LZ found yet
									self.AI[team].SpawnTimer:Reset();
									self.AI[team].timeToSpawn = 5000;
								end
							end
						elseif self.AI[team].BombTimer:IsPastSimMS(self.AI[team].timeToBomb) then
							self.AI[team].BombTimer:Reset();
							self.AI[team].timeToBomb = (20000 - self.Difficulty * 75) * rte.SpawnIntervalScale;

							if math.random() < self.AI[team].bombChance then
								local bombPosX = self.LZmap:FindBombTarget(team);
								if bombPosX then
									self.AI[team].bombChance = math.max(self.AI[team].bombChance*0.85, 0.05);
									self:CreateBombDrop(bombPosX , team);
									self.AI[team].timeToBomb = (RangeRand(150, 250) * 100 - self.Difficulty * 120) * rte.SpawnIntervalScale; -- 20s to 8s
								end
							end
						elseif self.AI[team].EngineerTimer:IsPastSimMS(self.AI[team].timeToEngineer) then
							self.AI[team].EngineerTimer:Reset();

							if not self.AI[team].Engineer or not MovableMan:IsActor(self.AI[team].Engineer) then
								local digPosX = self.LZmap:FindSafeLZ(team);
								if digPosX then
									local Craft = RandomACDropShip("Craft", self.AI[team].TechID); -- Pick a drop-ship to deliver with
									if Craft then
										Craft.Team = team;
										Craft.Pos = Vector(digPosX, -30); -- Set the spawn point of the craft

										self.AI[team].Engineer = self:CreateEngineer(team);
										if self.AI[team].Engineer then
											Craft:AddInventoryItem(self.AI[team].Engineer);

											-- Subtract the total value of the craft+cargo from the CPU team's funds
											self:ChangeTeamFunds(-Craft:GetTotalValue(self.AI[team].TechID, 2), team);

											-- Spawn the Craft onto the scene
											MovableMan:AddActor(Craft);

											-- Wait a bit longer until the next check
											self.AI[team].timeToEngineer = self.AI[team].timeToEngineer * 1.1;
										end
									end
								end
							end
						end
					else	-- The AI is out of gold
						if self.endless then
							self:SetTeamFunds(100000, team);
						else
							local enemyPresent = false;
							local objectives = 0;
							local maxObjectivesToShow = 5;
							for _, actorCollection in ipairs{MovableMan.AddedActors, MovableMan.Actors} do
								if objectives >= maxObjectivesToShow then
									break;
								end
								for Act in actorCollection do
									if Act.Team == team and not Act:IsDead() then
										if Act.ClassName ~= "ADoor" then
											enemyPresent = true;

											-- Add objective points
											if Act.ClassName == "AHuman" or Act.ClassName == "ACrab" then
												objectives = objectives + 1;
												if objectives > maxObjectivesToShow then
													break;
												end

												for team = Activity.TEAM_1, Activity.TEAM_4 do
													self:AddObjectivePoint("Destroy!", Act.AboveHUDPos, team, GameActivity.ARROWDOWN);
												end
											end
										end
									end
								end
							end

							-- No AI actors left, remove the CPU-Team
							if not enemyPresent then
								self.AI[team].defeated = true;
							end
						end
					end
				end
			end
		end
	end
end

-- Pick an actor semi-randomly, with a larger probablility for actors with a lower score
function SkirmishDefense:SelectTarget(TargetActors)
	if #TargetActors > 1 then
		table.sort(TargetActors, function(A, B) return A.score < B.score end); -- Actors closer to the surface first

		local temperature = 5; -- a higher temperature means less random selection
		local sum = 0;
		local worstScore = TargetActors[#TargetActors].score;

		-- normalize the score
		for i, Data in pairs(TargetActors) do
			TargetActors[i].chance = 1 - Data.score / worstScore;
			sum = sum + math.exp(temperature*TargetActors[i].chance);
		end

		-- use Softmax to pick one of the n best LZs
		if sum > 0 then
			local pick = math.random() * sum;
			sum = 0;
			for _, Data in pairs(TargetActors) do
				sum = sum + math.exp(temperature*Data.chance);
				if sum >= pick then
					return Data.Act;
				end
			end
		end

		return TargetActors[1].Act;
	elseif #TargetActors == 1 then
		return TargetActors[1].Act;
	end
end

function SkirmishDefense:CreateHeavyDrop(xPosLZ, Destination, Team)
	local Craft = RandomACDropShip("Craft", self.AI[Team].TechID); -- Pick a craft to deliver with
	if Craft then
		-- The max allowed weight of this craft plus cargo
		local craftMaxMass = Craft.MaxInventoryMass;
		if craftMaxMass < 0 then
			craftMaxMass = math.huge;
		elseif craftMaxMass < 1 then
			Craft = RandomACDropShip("Craft", 0); -- MaxMass not defined
			craftMaxMass = Craft.MaxInventoryMass;
		end

		Craft.Team = Team;
		Craft.Pos = Vector(xPosLZ, -30); -- Set the spawn point of the craft

		local actorsInCargo;
		if Craft.MaxPassengers < 2 then
			actorsInCargo = 1;
		elseif Craft.MaxPassengers > 2 then
			actorsInCargo = math.random(2, Craft.MaxPassengers);
		else
			actorsInCargo = 2;
		end

		for _ = 1, actorsInCargo do
			local Passenger;
			if math.random() < self:GetCrabToHumanSpawnRatio(self.AI[Team].TechID) then
				Passenger = self:CreateCrab(Team);
			elseif RangeRand(0, 105) < self.Difficulty then
				Passenger = self:CreateHeavyInfantry(Team);
			else
				Passenger = self:CreateRandomInfantry(Team);
			end

			if Passenger then
				if Destination then
					Passenger.AIMode = Actor.AIMODE_GOTO;
					Passenger:AddAISceneWaypoint(Destination);
				end

				Craft:AddInventoryItem(Passenger);

				if Craft.InventoryMass > craftMaxMass then
					break;
				end
			end
		end

		-- Subtract the total value of the craft+cargo from the CPU team's funds
		self:ChangeTeamFunds(-Craft:GetTotalValue(self.AI[Team].TechID, 2), Team);

		-- Spawn the Craft onto the scene
		MovableMan:AddActor(Craft);
	end
end

function SkirmishDefense:CreateMediumDrop(xPosLZ, Destination, Team)
	-- Pick a craft to deliver with
	local Craft, actorsInCargo;
	if math.random() < 0.6 then
		Craft = RandomACDropShip("Craft", self.AI[Team].TechID);
		if Craft.MaxPassengers < 2 then
			actorsInCargo = 1;
		elseif Craft.MaxPassengers > 2 then
			actorsInCargo = math.random(2, Craft.MaxPassengers);
		else
			actorsInCargo = 2;
		end
	else
		Craft = RandomACRocket("Craft", self.AI[Team].TechID);
		actorsInCargo = Craft.MaxPassengers;
	end

	if Craft then
		-- The max allowed weight of this craft plus cargo
		local craftMaxMass = Craft.MaxInventoryMass;
		if craftMaxMass < 0 then
			craftMaxMass = math.huge;
		elseif craftMaxMass < 1 then
			if Craft.ClassName == "ACDropship" then
				Craft = RandomACDropShip("Craft", 0); -- MaxMass not defined
			else
				Craft = RandomACRocket("Craft", 0); -- MaxMass not defined
			end
			craftMaxMass = Craft.MaxInventoryMass;
		end

		Craft.Team = Team;
		Craft.Pos = Vector(xPosLZ, -30); -- Set the spawn point of the craft

		for _ = 1, actorsInCargo do
			local Passenger;
			if RangeRand(-5, 125) < self.Difficulty then
				Passenger = self:CreateMediumInfantry(Team);
			elseif math.random() < 0.65 then
				Passenger = self:CreateRandomInfantry(Team);
			else
				Passenger = self:CreateLightInfantry(Team);
			end

			if Passenger then
				if Destination then
					Passenger.AIMode = Actor.AIMODE_GOTO;
					Passenger:AddAISceneWaypoint(Destination);
				end

				Craft:AddInventoryItem(Passenger);

				if Craft.InventoryMass > craftMaxMass then
					break;
				end
			end
		end

		-- Subtract the total value of the craft+cargo from the CPU team's funds
		self:ChangeTeamFunds(-Craft:GetTotalValue(self.AI[Team].TechID, 2), Team);

		-- Spawn the Craft onto the scene
		MovableMan:AddActor(Craft);
	end
end

function SkirmishDefense:CreateLightDrop(xPosLZ, Destination, Team)
	-- Pick a craft to deliver with
	local Craft;
	if math.random() < 0.6 then
		Craft = RandomACDropShip("Craft", self.AI[Team].TechID);
	else
		Craft = RandomACRocket("Craft", self.AI[Team].TechID);
	end

	if Craft then
		-- The max allowed weight of this craft plus cargo
		local craftMaxMass = Craft.MaxInventoryMass;
		if craftMaxMass < 0 then
			craftMaxMass = math.huge;
		elseif craftMaxMass < 1 then
			if Craft.ClassName == "ACDropship" then
				Craft = RandomACDropShip("Craft", 0); -- MaxMass not defined
			else
				Craft = RandomACRocket("Craft", 0); -- MaxMass not defined
			end
			craftMaxMass = Craft.MaxInventoryMass;
		end

		Craft.Team = Team;
		Craft.Pos = Vector(xPosLZ, -30); -- Set the spawn point of the craft

		for _ = 1, Craft.MaxPassengers do
			local Passenger;
			if RangeRand(10, 200) < self.Difficulty then
				Passenger = self:CreateMediumInfantry(Team);
			else
				Passenger = self:CreateLightInfantry(Team);
			end

			if Passenger then
				if Destination then
					Passenger.AIMode = Actor.AIMODE_GOTO;
					Passenger:AddAISceneWaypoint(Destination);
				end

				Craft:AddInventoryItem(Passenger);

				if Craft.InventoryMass > craftMaxMass then
					break;
				end
			end
		end

		-- Subtract the total value of the craft+cargo from the CPU team's funds
		self:ChangeTeamFunds(-Craft:GetTotalValue(self.AI[Team].TechID, 2), Team);

		-- Spawn the Craft onto the scene
		MovableMan:AddActor(Craft);
	end
end

function SkirmishDefense:CreateScoutDrop(xPosLZ, Destination, Team)
	-- Pick a craft to deliver with
	local Craft;
	if math.random() < 0.6 then
		Craft = RandomACDropShip("Craft", self.AI[Team].TechID);
	else
		Craft = RandomACRocket("Craft", self.AI[Team].TechID);
	end

	if Craft then
		-- The max allowed weight of this craft plus cargo
		local craftMaxMass = Craft.MaxInventoryMass;
		if craftMaxMass < 0 then
			craftMaxMass = math.huge;
		elseif craftMaxMass < 1 then
			if Craft.ClassName == "ACDropship" then
				Craft = RandomACDropShip("Craft", 0); -- MaxMass not defined
			else
				Craft = RandomACRocket("Craft", 0); -- MaxMass not defined
			end
			craftMaxMass = Craft.MaxInventoryMass;
		end

		Craft.Team = Team;
		Craft.Pos = Vector(xPosLZ, -30); -- Set the spawn point of the craft

		for _ = 1, Craft.MaxPassengers do
			local Passenger;
			if math.random() < 0.3 then
				Passenger = self:CreateLightInfantry(Team);
			else
				Passenger = self:CreateScoutInfantry(Team);
			end

			if Passenger then
				if Destination then
					Passenger.AIMode = Actor.AIMODE_GOTO;
					Passenger:AddAISceneWaypoint(Destination);
				end

				Craft:AddInventoryItem(Passenger);

				if Craft.InventoryMass > craftMaxMass then
					break;
				end
			end
		end

		-- Subtract the total value of the craft+cargo from the CPU team's funds
		self:ChangeTeamFunds(-Craft:GetTotalValue(self.AI[Team].TechID, 2), Team);

		-- Spawn the Craft onto the scene
		MovableMan:AddActor(Craft);
	end
end

function SkirmishDefense:CreateBreachDrop(xPosLZ, Team)
	-- Pick a craft to deliver with
	local crateProb = 0;
	if self.Difficulty > 45 then
		crateProb = self.Difficulty / 300;
	end

	local Craft;
	if math.random() < crateProb then
		Craft = RandomACRocket("Craft - Crates", self.AI[Team].TechID);
	else
		Craft = RandomACRocket("Craft", self.AI[Team].TechID);
	end

	if Craft then
		Craft.Team = Team;
		Craft.Pos = Vector(xPosLZ, -30); -- Set the spawn point of the craft

		local Passenger = self:CreateBreachInfantry(Team);
		if Passenger then
			Craft:AddInventoryItem(Passenger);
		end

		-- Subtract the total value of the craft+cargo from the CPU team's funds
		self:ChangeTeamFunds(-Craft:GetTotalValue(self.AI[Team].TechID, 2), Team);

		-- Spawn the Craft onto the scene
		MovableMan:AddActor(Craft);
	end
end

function SkirmishDefense:CreateBombDrop(bombPosX, Team)
	local Craft = RandomACDropShip("Craft", self.AI[Team].TechID); -- Pick a craft to deliver with
	if Craft then
		-- The max allowed weight of this craft plus cargo
		local craftMaxMass = Craft.MaxInventoryMass;
		if craftMaxMass < 0 then
			craftMaxMass = math.huge;
		elseif craftMaxMass < 1 then
			Craft = RandomACDropShip("Craft", 0); -- MaxMass not defined
			craftMaxMass = Craft.MaxInventoryMass;
		end

		Craft.AIMode = Actor.AIMODE_BOMB; -- DropShips open doors at a high altitude in bomb mode
		Craft.Team = Team;
		Craft.Pos = Vector(bombPosX, -30); -- Set the spawn point of the craft

		for _ = 3, 5 do
			local Payload = RandomTDExplosive("Bombs - Payloads", self.AI[Team].TechID);
			if Payload then
				Craft:AddInventoryItem(Payload);
			end

			if Craft.InventoryMass > craftMaxMass then
				break;
			end
		end

		-- Subtract the total value of the craft+cargo from the CPU team's funds
		self:ChangeTeamFunds(-Craft:GetTotalValue(self.AI[Team].TechID, 2), Team);

		-- Spawn the Craft onto the scene
		MovableMan:AddActor(Craft);
	end
end



function SkirmishDefense:CreateCrab(Team)
	local Passenger = RandomACrab("Actors - Mecha", self.AI[Team].TechID);
	if Passenger then
		Passenger.AIMode = Actor.AIMODE_BRAINHUNT;
		Passenger.Team = Team;
		return Passenger;
	end
end

function SkirmishDefense:CreateRandomInfantry(Team)
	local Passenger = RandomAHuman("Actors", self.AI[Team].TechID);
	if Passenger then
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Primary", self.AI[Team].TechID));
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.AI[Team].TechID));

		local rand = math.random();
		if rand < 0.25 then
			Passenger:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", self.AI[Team].TechID));
		elseif rand < 0.50 then
			Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.AI[Team].TechID));
		elseif rand < 0.75 then
			Passenger:AddInventoryItem(RandomHeldDevice("Shields", self.AI[Team].TechID));
		else
			Passenger:AddInventoryItem(CreateHDFirearm("Medikit", "Base.rte"));
		end
		if math.random() < 0.05 then
			Passenger:AddInventoryItem(RandomHDFirearm("Tools - Breaching", self.AI[Team].TechID));
		end

		Passenger.AIMode = Actor.AIMODE_BRAINHUNT;
		Passenger.Team = Team;
		return Passenger;
	end
end

function SkirmishDefense:CreateLightInfantry(Team)
	local Passenger = RandomAHuman("Actors - Light", self.AI[Team].TechID);
	if Passenger.ModuleID ~= self.AI[Team].TechID then
		Passenger = RandomAHuman("Actors", self.AI[Team].TechID);
	end

	if Passenger then
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Light", self.AI[Team].TechID));
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.AI[Team].TechID));

		local rand = math.random();
		if rand < 0.5 then
			Passenger:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", self.AI[Team].TechID));
		elseif rand < 0.8 then
			Passenger:AddInventoryItem(CreateHDFirearm("Medikit", "Base.rte"));
		else
			Passenger:AddInventoryItem(RandomHDFirearm("Tools - Breaching", self.AI[Team].TechID));
		end

		Passenger.AIMode = Actor.AIMODE_BRAINHUNT;
		Passenger.Team = Team;
		return Passenger;
	end
end

function SkirmishDefense:CreateHeavyInfantry(Team)
	local Passenger = RandomAHuman("Actors - Heavy", self.AI[Team].TechID);
	if Passenger.ModuleID ~= self.AI[Team].TechID then
		Passenger = RandomAHuman("Actors", self.AI[Team].TechID);
	end

	if Passenger then
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Heavy", self.AI[Team].TechID));

		if math.random() < 0.3 then
			Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Light", self.AI[Team].TechID));
			if math.random() < 0.25 then
				Passenger:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", self.AI[Team].TechID));
			elseif math.random() < 0.35 then
				Passenger:AddInventoryItem(CreateHDFirearm("Medikit", "Base.rte"));
			end
		else
			Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.AI[Team].TechID));
			if math.random() < 0.3 then
				Passenger:AddInventoryItem(RandomHeldDevice("Shields", self.AI[Team].TechID));
				Passenger:AddInventoryItem(CreateHDFirearm("Medikit", "Base.rte"));
			else
				Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.AI[Team].TechID));
			end
		end

		Passenger.AIMode = Actor.AIMODE_BRAINHUNT;
		Passenger.Team = Team;
		return Passenger;
	end
end

function SkirmishDefense:CreateMediumInfantry(Team)
	local Passenger = RandomAHuman("Actors - Heavy", self.AI[Team].TechID);
	if Passenger.ModuleID ~= self.AI[Team].TechID then
		Passenger = RandomAHuman("Actors", self.AI[Team].TechID);
	end

	if Passenger then
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Light", self.AI[Team].TechID));
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.AI[Team].TechID));

		if math.random() < 0.3 then
			Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.AI[Team].TechID));
		else
			Passenger:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", self.AI[Team].TechID));
		end
		if math.random() < 0.5 then
			Passenger:AddInventoryItem(CreateHDFirearm("Medikit", "Base.rte"));
		end

		Passenger.AIMode = Actor.AIMODE_BRAINHUNT;
		Passenger.Team = Team;
		return Passenger;
	end
end

function SkirmishDefense:CreateEngineer(Team)
	local Passenger = RandomAHuman("Actors - Light", self.AI[Team].TechID);
	if Passenger.ModuleID ~= self.AI[Team].TechID then
		Passenger = RandomAHuman("Actors", self.AI[Team].TechID);
	end

	if Passenger then
		if math.random() < 0.7 then
			Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Light", self.AI[Team].TechID));
		else
			Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.AI[Team].TechID));
			local rand = math.random();
			if rand < 0.2 then
				Passenger:AddInventoryItem(RandomHeldDevice("Shields", self.AI[Team].TechID));
			elseif rand < 0.4 then
				Passenger:AddInventoryItem(CreateHDFirearm("Medikit", "Base.rte"));
			else
				Passenger:AddInventoryItem(RandomTDExplosive("Tools - Breaching", self.AI[Team].TechID));
			end
		end
		Passenger:AddInventoryItem(RandomHDFirearm("Tools - Diggers", self.AI[Team].TechID));

		Passenger.AIMode = Actor.AIMODE_GOLDDIG;
		Passenger.Team = Team;
		return Passenger;
	end
end

function SkirmishDefense:CreateBreachInfantry(Team)
	local Passenger = RandomAHuman("Actors - Light", self.AI[Team].TechID);
	if Passenger.ModuleID ~= self.AI[Team].TechID then
		Passenger = RandomAHuman("Actors", self.AI[Team].TechID);
	end

	if Passenger then
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Light", self.AI[Team].TechID));
		Passenger:AddInventoryItem(RandomHDFirearm("Tools - Breaching", self.AI[Team].TechID));

		Passenger.AIMode = Actor.AIMODE_BRAINHUNT;
		Passenger.Team = Team;
		return Passenger;
	end
end

function SkirmishDefense:CreateScoutInfantry(Team)
	local Passenger = RandomAHuman("Actors - Light", self.AI[Team].TechID);
	if Passenger.ModuleID ~= self.AI[Team].TechID then
		Passenger = RandomAHuman("Actors", self.AI[Team].TechID);
	end

	if Passenger then
		if math.random() < 0.15 then
			Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Sniper", self.AI[Team].TechID));
		end
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.AI[Team].TechID));

		if math.random() < 0.3 then
			Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.AI[Team].TechID));
		else
			Passenger:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", self.AI[Team].TechID));
			Passenger:AddInventoryItem(CreateHDFirearm("Medikit", "Base.rte"));
		end

		Passenger.AIMode = Actor.AIMODE_BRAINHUNT;
		Passenger.Team = Team;
		return Passenger;
	end
end
