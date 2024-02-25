--[[

*** INSTRUCTIONS ***

This activity can be run on any scene with a "Red Brain" and "Green Brain" area.

Add defender units by placing areas named:
"Red Defender 1" to "Red Defender 7"
"Red Miner 1" to "Red Miner 3"
"Green Defender 1" to "Green Defender 7"
"Green Miner 1" to "Green Miner 3"

Activate fog of war during the build phase by adding areas:
"Red Build Area" and "Green Build Area" where the players are allowed to build

--]]

function BrainvsBrain:StartActivity(isNewGame)
	self.winTimer = Timer();

	if self.CPUTeam ~= Activity.NOTEAM then
		self.spawnTimer = Timer();
		self.hunterTimer = Timer();

		-- Store data about terrain and enemy actors in the LZ map, use it to pick safe landing zones
		self.LZMap = require("Activities/Utility/LandingZoneMap");
		self.LZMap:Initialize({self.CPUTeam});	-- a list of AI teams

		-- Store data about player teams: OnPlayerTeam[A.Team] is true if "A" is an enemy to the AI
		self.OnPlayerTeam = {};
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self:PlayerActive(player) and self:PlayerHuman(player) then
				self.OnPlayerTeam[self:GetTeamOfPlayer(player)] = true;
			end
		end
	end

	self.TechName = {};
	self.TechName[Activity.TEAM_1] = self:GetTeamTech(Activity.TEAM_1);
	self.TechName[Activity.TEAM_2] = self:GetTeamTech(Activity.TEAM_2);

	if self.CPUTeam ~= Activity.NOTEAM then
		self.CPUTechID = PresetMan:GetModuleID(self.TechName[self.CPUTeam]);
	end

	if isNewGame then
		self:StartNewGame();
	else
		self:ResumeLoadedGame();
	end
end

function BrainvsBrain:OnSave()
	if self.CPUTeam ~= Activity.NOTEAM then
		self:SaveNumber("CPUTechID", self.CPUTechID);

		self:SaveNumber("bombChance", self.bombChance);
		self:SaveNumber("spawnDelay", self.spawnDelay);
		self:SaveNumber("spawnTimerElapsedSimTimeMS", self.spawnTimer.ElapsedSimTimeMS)
		self:SaveNumber("hunterDelay", self.hunterDelay);
		self:SaveNumber("hunterTimerElapsedSimTimeMS", self.hunterTimer.ElapsedSimTimeMS);
	end
end

function BrainvsBrain:StartNewGame()
	self.first_update = true;

	-- Add a CPU team if we only have one player
	if not self.CPUTeam or self.CPUTeam == Activity.NOTEAM then
		local active_players = 0;
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self:PlayerHuman(player) then
				active_players = active_players + 1;
			end
		end

		if active_players < 2 then
			self.CPUTeam = Activity.TEAM_2;
			for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
				if self:PlayerActive(player) and self:PlayerHuman(player) then
					if self:GetTeamOfPlayer(player) == Activity.TEAM_2 then
						self.CPUTeam = Activity.TEAM_1;
						break;
					end
				end
			end
		else
			self.CPUTeam = Activity.NOTEAM;
		end
	end

	if self.CPUTeam == Activity.NOTEAM then
		-- No AI player, go in to edit mode
		self.ActivityState = Activity.EDITING;

		-- Add fog so the players cannot build in the same area
		local RedAreaString = "Red Build Area Center";
		local GreenAreaString = "Green Build Area Center";
		if SceneMan.Scene:HasArea(RedAreaString) and SceneMan.Scene:HasArea(GreenAreaString) then
			local fogWidth = 20;

			SceneMan:MakeAllUnseen(Vector(fogWidth, fogWidth), Activity.TEAM_1);
			SceneMan:MakeAllUnseen(Vector(fogWidth, fogWidth), Activity.TEAM_2);

			-- Reveal the build areas
			local RedCenter = SceneMan.Scene:GetArea(RedAreaString):GetCenterPoint();
			local GreenCenter = SceneMan.Scene:GetArea(GreenAreaString):GetCenterPoint();
			local range = SceneMan:ShortestDistance(RedCenter, GreenCenter, false).Magnitude * 0.45;

			for y = fogWidth/2, SceneMan.SceneHeight, fogWidth do
				for x = fogWidth/2, SceneMan.SceneWidth, fogWidth do
					if SceneMan:ShortestDistance(RedCenter, Vector(x, y), false).Largest < range then
						SceneMan:RevealUnseen(x, y, Activity.TEAM_1);
					elseif SceneMan:ShortestDistance(GreenCenter, Vector(x, y), false).Largest < range then
						SceneMan:RevealUnseen(x, y, Activity.TEAM_2);
					end
				end
			end
		end
	else
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self:PlayerActive(player) and self:PlayerHuman(player) then
				-- Check if we already have a brain assigned
				if not self:GetPlayerBrain(player) then
					local foundBrain = MovableMan:GetUnassignedBrain(self:GetTeamOfPlayer(player));
					-- Spawn a brain if we can't find an unassigned brain in the scene to give each player
					if not foundBrain then
						local techID = PresetMan:GetModuleID(self:GetTeamTech(self:GetTeamOfPlayer(player)));
						local Brain;
						if techID ~= -1 then
							Brain = PresetMan:GetLoadout("Infantry Brain", techID, false);
							if Brain then
								Brain:RemoveInventoryItem("Constructor");
							end
						end
						if not Brain then
							Brain = RandomAHuman("Brains", techID);
							if Brain then
								local Weapon = RandomHDFirearm("Weapons - Primary", self:GetTeamTech(self:GetTeamOfPlayer(player)));
								if Weapon then
									Brain:AddInventoryItem(Weapon);
								end
							end
						end

						if Brain then
							Brain.AIMode = Actor.AIMODE_SENTRY;
							Brain.Team = self:GetTeamOfPlayer(player);

							if Brain.Team == Activity.TEAM_1 then
								Brain.Pos = SceneMan.Scene:GetArea("Red Brain"):GetRandomPoint();
							else
								Brain.Pos = SceneMan.Scene:GetArea("Green Brain"):GetRandomPoint();
							end

							-- make sure we are inside the scene
							if SceneMan.SceneWrapsX then
								if Brain.Pos.X < 0 then
									Brain.Pos.X = Brain.Pos.X + SceneMan.SceneWidth;
								elseif Brain.Pos.X >= SceneMan.SceneWidth then
									Brain.Pos.X = Brain.Pos.X - SceneMan.SceneWidth;
								end
							else
								Brain.Pos.X = math.max(math.min(Brain.Pos.X, SceneMan.SceneWidth-5), 5);
							end

							Brain.Pos = SceneMan:MovePointToGround(Brain.Pos, Brain.Height*0.3, 2);
							MovableMan:AddActor(Brain);
							self:SetPlayerBrain(Brain, player);
							self:SetObservationTarget(self:GetPlayerBrain(player).Pos, player);
						end
					else
						-- Set the found brain to be the selected actor at start
						self:SetPlayerBrain(foundBrain, player);
						self:SwitchToActor(foundBrain, player, self:GetTeamOfPlayer(player));

						-- Set the observation target to the brain, so that if/when it dies, the view flies to it in observation mode
						self:SetObservationTarget(self:GetPlayerBrain(player).Pos, player);
					end
				end
			end
		end
	end

	-- Set all actors defined in the ini-file to sentry mode
	for actor in MovableMan.AddedActors do
		if actor.ClassName == "AHuman" or actor.ClassName == "ACrab" then
			if actor.AIMode ~= Actor.AIMODE_GOLDDIG then
				actor.AIMode = Actor.AIMODE_SENTRY;
			end
		end
	end

	if self.CPUTeam == Activity.NOTEAM then
		self:SetTeamFunds(self:GetStartingGold(), Activity.TEAM_1);
		self:SetTeamFunds(self:GetStartingGold(), Activity.TEAM_2);
	else
		if self.CPUTeam == Activity.TEAM_1 then
			self:SetTeamFunds(self:GetStartingGold(), Activity.TEAM_2);
		else
			self:SetTeamFunds(self:GetStartingGold(), Activity.TEAM_1);
		end

		self:SetTeamFunds(self:GetStartingGold()*(self.Difficulty/100+0.5), self.CPUTeam);
		self.bombChance = math.random(self.Difficulty*0.7, self.Difficulty) / 120;

		local techID = PresetMan:GetModuleID(self:GetTeamTech(self.CPUTeam));
		if techID ~= -1 then
			self.CPUBrain = PresetMan:GetLoadout("Infantry Brain", techID, false);
			if self.CPUBrain then
				self.CPUBrain:RemoveInventoryItem("Constructor");
			end
		end
		if not self.CPUBrain then
			self.CPUBrain = RandomAHuman("Brains", techID);
			if self.CPUBrain then
				local Weapon = CreateHDFirearm("SMG", "Base.rte");
				if Weapon then
					self.CPUBrain:AddInventoryItem(Weapon);
				end
			end
		end

		if self.CPUBrain then
			self.CPUBrain.AIMode = Actor.AIMODE_SENTRY;
			self.CPUBrain.Team = self.CPUTeam;

			if self.CPUBrain.Team == Activity.TEAM_1 then
				self.CPUBrain.Pos = SceneMan.Scene:GetArea("Red Brain"):GetRandomPoint();
			else
				self.CPUBrain.Pos = SceneMan.Scene:GetArea("Green Brain"):GetRandomPoint();
			end

			-- Make sure we are inside the scene
			if SceneMan.SceneWrapsX then
				if self.CPUBrain.Pos.X < 0 then
					self.CPUBrain.Pos.X = self.CPUBrain.Pos.X + SceneMan.SceneWidth;
				elseif self.CPUBrain.Pos.X >= SceneMan.SceneWidth then
					self.CPUBrain.Pos.X = self.CPUBrain.Pos.X - SceneMan.SceneWidth;
				end
			else
				self.CPUBrain.Pos.X = math.max(math.min(self.CPUBrain.Pos.X, SceneMan.SceneWidth-5), 5);
			end

			self.CPUBrain.Pos = SceneMan:MovePointToGround(self.CPUBrain.Pos, self.CPUBrain.Height*0.3, 2);
			MovableMan:AddActor(self.CPUBrain);
		end

		self.spawnDelay = (6500 - self.Difficulty * 60) * rte.SpawnIntervalScale;
		self.hunterDelay = self.spawnDelay + 10000;

		local spawnStartingActors = function(areaName, numberOfActorsToSpawn, team)
			for i = 1, numberOfActorsToSpawn do
				local guardAreaToUse = areaName .. " " .. i;
				if SceneMan.Scene:HasArea(areaName .. " " .. i) then
					local startingActor;
					if areaName:find("Miner") then
						startingActor = self:CreateEngineer(team);
					else
						startingActor = self:CreateDefender(team);
					end
					if startingActor then
						startingActor.Pos = SceneMan.Scene:GetArea(guardAreaToUse):GetCenterPoint();
						MovableMan:AddActor(startingActor);
					end
				else
					return;
				end
			end
		end
		spawnStartingActors("Red Defender", 7, Activity.TEAM_1);
		spawnStartingActors("Red Miner", 3, Activity.TEAM_1);
		spawnStartingActors("Green Defender", 7, Activity.TEAM_2);
		spawnStartingActors("Green Miner", 3, Activity.TEAM_2);
	end
end

function BrainvsBrain:ResumeLoadedGame()
	if self.CPUTeam ~= Activity.NOTEAM then
		self.bombChance = self:LoadNumber("bombChance");
		self.spawnDelay = self:LoadNumber("spawnDelay");
		self.spawnTimer.ElapsedSimTimeMS = self:LoadNumber("spawnTimerElapsedSimTimeMS");
		self.hunterDelay = self:LoadNumber("hunterDelay");
		self.hunterTimer.ElapsedSimTimeMS = self:LoadNumber("hunterTimerElapsedSimTimeMS");

		if not self.CPUBrain then
			self.CPUBrain = MovableMan:GetUnassignedBrain(self.CPUTeam);
		end
	end

	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) then
			if not self:GetPlayerBrain(player) then
				local team = self:GetTeamOfPlayer(player);
				local foundBrain = MovableMan:GetUnassignedBrain(team);
				if foundBrain then
					--Set the found brain to be the selected actor at start
					self:SetPlayerBrain(foundBrain, player);
					self:SwitchToActor(foundBrain, player, self:GetTeamOfPlayer(player));
					self:SetLandingZone(self:GetPlayerBrain(player).Pos, player);
					--Set the observation target to the brain, so that if/when it dies, the view flies to it in observation mode
					self:SetObservationTarget(self:GetPlayerBrain(player).Pos, player);
				end
			end
		end
	end
end

function BrainvsBrain:EndActivity()
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

function BrainvsBrain:UpdateActivity()
	if self.ActivityState == Activity.OVER then
		return;
	elseif self.ActivityState == Activity.EDITING then
		-- Set all actors to sentry mode
		for actor in MovableMan.AddedActors do
			if actor.ClassName == "AHuman" or actor.ClassName == "ACrab" then
				if actor.AIMode ~= Actor.AIMODE_GOLDDIG then
					actor.AIMode = Actor.AIMODE_SENTRY;
				end
			end
		end
	else
		if self.first_update then
			self.first_update = nil;

			-- Add fog
			if self:GetFogOfWarEnabled() then
				for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
					if self:PlayerActive(player) and self:PlayerHuman(player) then
						SceneMan:MakeAllUnseen(Vector(32, 32), self:GetTeamOfPlayer(player));
					end
				end

				-- Lift the fog around friendly actors
				for Act in MovableMan.AddedActors do
					for ang = 0, math.pi*2, 0.1 do
						SceneMan:CastSeeRay(Act.Team, Act.EyePos, Vector(130+FrameMan.PlayerScreenWidth*0.5, 0):RadRotate(ang), Vector(), 1, 4);
					end
				end
			else -- Lift any fog covering the build areas
				for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
					if self:PlayerActive(player) and self:PlayerHuman(player) then
						SceneMan:RevealUnseenBox(0, 0, SceneMan.SceneWidth-1, SceneMan.SceneHeight-1, self:GetTeamOfPlayer(player));
					end
				end
			end

			-- Set all actors to sentry mode
			for actor in MovableMan.AddedActors do
				if actor.ClassName == "AHuman" or actor.ClassName == "ACrab" then
					if actor.AIMode ~= Actor.AIMODE_GOLDDIG then
						actor.AIMode = Actor.AIMODE_SENTRY;
					end
				end
			end
		else
			if self.winTimer:IsPastRealMS(2000) then
				-- Check win conditions
				self.winTimer:Reset();

				local red_players = 0;
				local green_players = 0;
				if self.CPUTeam ~= Activity.NOTEAM then
					if MovableMan:IsActor(self.CPUBrain) then
						if self.CPUTeam == Activity.TEAM_1 then
							red_players = red_players + 1;
						else
							green_players = green_players + 1;
						end
					end
				end

				for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
					if self:PlayerActive(player) and self:PlayerHuman(player) then
						local Brain = self:GetPlayerBrain(player);
						if not MovableMan:IsActor(Brain) then
							Brain = MovableMan:GetUnassignedBrain(self:GetTeamOfPlayer(player));
							if Brain then
								self:SetPlayerBrain(Brain, player);
								self:SetObservationTarget(Brain.Pos, player);
							else
								-- Need to nullify dead brain or when activity ends it may crash CC
								self:SetPlayerBrain(nil, player);
							end
						end

						if Brain and MovableMan:IsActor(Brain) then
							if self:GetTeamOfPlayer(player) == Activity.TEAM_1 then
								red_players = red_players + 1;
							else
								green_players = green_players + 1;
							end
						else
							self:ResetMessageTimer(player);
							FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
							FrameMan:SetScreenText("Your brain has been destroyed!", self:ScreenOfPlayer(player), 2000, -1, false);
						end
					end
				end

				if red_players < 1 and green_players > 0 then
					self.WinnerTeam = Activity.TEAM_2;
					MovableMan:KillAllEnemyActors(self.WinnerTeam);
					ActivityMan:EndActivity();
					return;
				elseif green_players < 1 and red_players > 0 then
					self.WinnerTeam = Activity.TEAM_1;
					MovableMan:KillAllEnemyActors(self.WinnerTeam);
					ActivityMan:EndActivity();
					return;
				elseif green_players < 1 and red_players < 1 then
					ActivityMan:EndActivity();
					return;
				end
			end

			if self.LZMap then
				self.LZMap:Update();

				if self.hunterTimer:IsPastSimMS(self.hunterDelay) then
					self.hunterTimer:Reset();

					for Hunter in MovableMan.Actors do
						if Hunter.Team == self.CPUTeam and Hunter.AIMode == Actor.AIMODE_GOTO then
							local Pray = Hunter.MOMoveTarget;
							if not MovableMan:IsActor(Pray) then
								Hunter:ClearAIWaypoints();
								Hunter.AIMode = Actor.AIMODE_BRAINHUNT;
							end
						end
					end
				end

				if self:GetTeamFunds(self.CPUTeam) > 0 then
					if self.spawnTimer:IsPastSimMS(self.spawnDelay) and MovableMan:IsActor(self.CPUBrain) then
						if self.AttackActor and MovableMan:IsActor(self.AttackActor) then
							-- We have a target actor, search for a suitable LZ;
							local easyPathLZx, easyPathLZobst, closeLZx, closeLZobst = self.LZMap:FindLZ(self.CPUTeam, self.AttackPos);
							if closeLZx then	-- Search done
								self.spawnTimer:Reset();

								local xPosLZ, obstacleHeight;
								if closeLZobst < 30 and easyPathLZobst < 30 then
									if math.random() < 0.5 then
										xPosLZ = closeLZx;
										obstacleHeight = closeLZobst;
									else
										xPosLZ = easyPathLZx;
										obstacleHeight = easyPathLZobst;
									end
								elseif closeLZobst > 100 and easyPathLZobst < 100 then
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

								if obstacleHeight > 200 and math.random() < 0.4 then
									-- This target is very difficult to reach: cancel this attack and search for another target
									self.AttackActor = nil;
									self.AttackPos = nil;
								else
									-- Attack target
									self.spawnTimer:Reset();
									if MovableMan:IsActor(self.CPUBrain) and SceneMan:ShortestDistance(self.AttackActor.Pos, self.CPUBrain.Pos, false):MagnitudeIsLessThan(1000) then
										-- The player is close to the AI brain so spawn again soon
										self.spawnDelay = (25000 - self.Difficulty * 150) * rte.SpawnIntervalScale;
									else
										self.spawnDelay = (40000 - self.Difficulty * 250) * rte.SpawnIntervalScale;
									end

									if obstacleHeight < 30 then
										self:CreateHeavyDrop(xPosLZ);
									elseif obstacleHeight < 90 then
										self:CreateMediumDrop(xPosLZ);
									elseif obstacleHeight < 180 then
										self:CreateLightDrop(xPosLZ);
									else
										self:CreateScoutDrop(xPosLZ);

										-- This target is very difficult to reach: change target for the next attack
										self.AttackActor = nil;
										self.AttackPos = nil;
									end

									if self.AttackActor then
										if math.random() < 0.4 then
											-- Change target for the next attack
											self.AttackActor = nil;
											self.AttackPos = nil;
										else
											self.AttackPos = Vector(self.AttackActor.Pos.X, self.AttackActor.Pos.Y);
										end
									end
								end
							end
						else
							self.AttackActor = nil;

							-- Find an enemy actor to attack
							local Intruders = {};
							local intruder_tally = 0;
							for Act in MovableMan.Actors do
								if self.OnPlayerTeam[Act.Team] and (Act.ClassName == "AHuman" or Act.ClassName == "ACrab" or Act.ClassName == "Actor") and not SceneMan:IsUnseen(Act.Pos.X, Act.Pos.Y, self.CPUTeam) then
									local distance = 20 * (SceneMan:ShortestDistance(self.CPUBrain.Pos, Act.Pos, false).Largest / SceneMan.SceneWidth);
									table.insert(Intruders, {Act=Act, score=self.LZMap:SurfaceProximity(Act.Pos)+distance+math.random(300)});
									intruder_tally = intruder_tally + 1;
								end
							end

							if intruder_tally < 1 then
								self.spawnTimer:Reset();
								self.spawnDelay = 10000;
							else
								table.sort(Intruders, function(A, B) return A.score > B.score end);	-- the nearest intruder last
								self.AttackActor = table.remove(Intruders).Act;
								self.AttackPos = Vector(self.AttackActor.Pos.X, self.AttackActor.Pos.Y);


								-- try bombing if no targets close to CPU brain
								if math.random() < self.bombChance and (SceneMan:ShortestDistance(self.CPUBrain.Pos, self.AttackPos, false).Largest/SceneMan.SceneWidth) > 0.2 then
									local bombPosX = self.LZMap:FindBombTarget(self.CPUTeam);
									if bombPosX then
										self.spawnTimer:Reset();
										self.spawnDelay = (10000 - self.Difficulty * 70) * rte.SpawnIntervalScale;
										self.bombChance = math.max(self.bombChance*0.9, 0.05);
										self:CreateBombDrop(bombPosX);
									end
								end
							end
						end
					end
				end
			end
		end
	end
end

function BrainvsBrain:CreateHeavyDrop(xPosLZ)
	local Craft = RandomACDropShip("Craft", self.TechName[self.CPUTeam]);	-- Pick a craft to deliver with
	if Craft then
		-- The max allowed weight of this craft plus cargo
		local craftMaxMass = Craft.MaxInventoryMass;
		if craftMaxMass < 0 then
			craftMaxMass = math.huge;
		elseif craftMaxMass < 1 then
			Craft = RandomACDropShip("Craft", 0);	-- MaxMass not defined
			craftMaxMass = Craft.MaxInventoryMass;
		end

		Craft.Team = self.CPUTeam;
		Craft.Pos = Vector(xPosLZ, -30);	-- Set the spawn point of the craft

		for i = 1, Craft.MaxPassengers do
			if math.random() < self:GetCrabToHumanSpawnRatio(PresetMan:GetModuleID(self.TechName[self.CPUTeam])) then
				Passenger = self:CreateCrab(self.CPUTeam);
			elseif RangeRand(0, 105) < self.Difficulty then
				Passenger = self:CreateHeavyInfantry(self.CPUTeam);
			else
				Passenger = self:CreateRandomInfantry(self.CPUTeam);
			end

			if Passenger then
				if self.AttackActor and MovableMan:IsActor(self.AttackActor) then
					Passenger:AddAIMOWaypoint(self.AttackActor);
				else
					Passenger.AIMode = Actor.AIMODE_BRAINHUNT;
				end

				Craft:AddInventoryItem(Passenger);

				if Craft.InventoryMass > craftMaxMass then
					break;
				end
			end
		end

		-- Subtract the total value of the craft+cargo from the CPU team's funds
		self:ChangeTeamFunds(-Craft:GetTotalValue(self.CPUTechID, 2), self.CPUTeam);

		-- Spawn the Craft onto the scene
		MovableMan:AddActor(Craft);
	end
end

function BrainvsBrain:CreateMediumDrop(xPosLZ)
	-- Pick a craft to deliver with
	local Craft;
	if math.random() < self.Difficulty*0.0025 then
		Craft = RandomACRocket("Craft - Crates", self.TechName[self.CPUTeam]);
	elseif math.random() < 0.4 then
		Craft = RandomACDropShip("Craft", self.TechName[self.CPUTeam]);
	else
		Craft = RandomACRocket("Craft", self.TechName[self.CPUTeam]);
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
				Craft = RandomACRocket("Craft", 0);	-- MaxMass not defined
			end
			craftMaxMass = Craft.MaxInventoryMass;
		end

		Craft.Team = self.CPUTeam;
		Craft.Pos = Vector(xPosLZ, -30); -- Set the spawn point of the craft

		for _ = 1, Craft.MaxPassengers do
			if RangeRand(-5, 125) < self.Difficulty then
				Passenger = self:CreateMediumInfantry(self.CPUTeam);
			elseif math.random() < 0.65 then
				Passenger = self:CreateRandomInfantry(self.CPUTeam);
			else
				Passenger = self:CreateLightInfantry(self.CPUTeam);
			end

			if Passenger then
				if self.AttackActor and MovableMan:IsActor(self.AttackActor) then
					Passenger:AddAIMOWaypoint(self.AttackActor);
				else
					Passenger.AIMode = Actor.AIMODE_BRAINHUNT;
				end

				Craft:AddInventoryItem(Passenger);

				if Craft.InventoryMass > craftMaxMass then
					break;
				end
			end
		end

		-- Subtract the total value of the craft+cargo from the CPU team's funds
		self:ChangeTeamFunds(-Craft:GetTotalValue(self.CPUTechID, 2), self.CPUTeam);

		-- Spawn the Craft onto the scene
		MovableMan:AddActor(Craft);
	end
end

function BrainvsBrain:CreateLightDrop(xPosLZ)
	-- Pick a craft to deliver with
	local Craft;
	if math.random() < self.Difficulty*0.004 then
		Craft = RandomACRocket("Craft - Crates", self.TechName[self.CPUTeam]);
	elseif math.random() < 0.5 then
		Craft = RandomACDropShip("Craft", self.TechName[self.CPUTeam]);
	else
		Craft = RandomACRocket("Craft", self.TechName[self.CPUTeam]);
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
				Craft = RandomACRocket("Craft", 0);	-- MaxMass not defined
			end
			craftMaxMass = Craft.MaxInventoryMass;
		end

		Craft.Team = self.CPUTeam;
		Craft.Pos = Vector(xPosLZ, -30); -- Set the spawn point of the craft

		for _ = 1, Craft.MaxPassengers do
			if RangeRand(10, 200) < self.Difficulty then
				Passenger = self:CreateMediumInfantry(self.CPUTeam);
			else
				Passenger = self:CreateLightInfantry(self.CPUTeam);
			end

			if Passenger then
				if self.AttackActor and MovableMan:IsActor(self.AttackActor) then
					Passenger:AddAIMOWaypoint(self.AttackActor);
				else
					Passenger.AIMode = Actor.AIMODE_BRAINHUNT;
				end

				Craft:AddInventoryItem(Passenger);

				if Craft.InventoryMass > craftMaxMass then
					break;
				end
			end
		end

		-- Subtract the total value of the craft+cargo from the CPU team's funds
		self:ChangeTeamFunds(-Craft:GetTotalValue(self.CPUTechID, 2), self.CPUTeam);

		-- Spawn the Craft onto the scene
		MovableMan:AddActor(Craft);
	end
end

function BrainvsBrain:CreateScoutDrop(xPosLZ)
	-- Pick a craft to deliver with
	local Craft;
	if math.random() < self.Difficulty*0.005 then
		Craft = RandomACRocket("Craft - Crates", self.TechName[self.CPUTeam]);
	elseif math.random() < 0.4 then
		Craft = RandomACDropShip("Craft", self.TechName[self.CPUTeam]);
	else
		Craft = RandomACRocket("Craft", self.TechName[self.CPUTeam]);
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

		Craft.Team = self.CPUTeam;
		Craft.Pos = Vector(xPosLZ, -30); -- Set the spawn point of the craft

		for _ = 1, Craft.MaxPassengers do
			if math.random() < 0.3 then
				Passenger = self:CreateLightInfantry(self.CPUTeam);
			else
				Passenger = self:CreateScoutInfantry(self.CPUTeam);
			end

			if Passenger then
				if self.AttackActor and MovableMan:IsActor(self.AttackActor) then
					Passenger:AddAIMOWaypoint(self.AttackActor);
				else
					Passenger.AIMode = Actor.AIMODE_BRAINHUNT;
				end

				Craft:AddInventoryItem(Passenger);

				if Craft.InventoryMass > craftMaxMass then
					break;
				end
			end
		end

		-- Subtract the total value of the craft+cargo from the CPU team's funds
		self:ChangeTeamFunds(-Craft:GetTotalValue(self.CPUTechID, 2), self.CPUTeam);

		-- Spawn the Craft onto the scene
		MovableMan:AddActor(Craft);
	end
end

function BrainvsBrain:CreateBombDrop(bombPosX)
	local Craft = RandomACDropShip("Craft", self.CPUTechID);	-- Pick a craft to deliver with
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
		Craft.Team = self.CPUTeam;
		Craft.Pos = Vector(bombPosX, -30); -- Set the spawn point of the craft

		for _ = 1, math.random(3, 6) do
			Craft:AddInventoryItem(RandomTDExplosive("Bombs - Payloads", self.CPUTechID));

			if Craft.InventoryMass > craftMaxMass then
				break;
			end
		end

		-- Subtract the total value of the craft+cargo from the CPU team's funds
		self:ChangeTeamFunds(-Craft:GetTotalValue(self.CPUTechID, 2), self.CPUTeam);

		-- Spawn the Craft onto the scene
		MovableMan:AddActor(Craft);
	end
end

function BrainvsBrain:CreateCrab(team, mode)
	local Passenger = RandomACrab("Actors - Mecha", self.TechName[team]);
	if Passenger then
		Passenger.AIMode = mode or Actor.AIMODE_GOTO;
		Passenger.Team = team;
		return Passenger;
	end
end

function BrainvsBrain:CreateTurret(team, mode)
	local Passenger = RandomACrab("Turret", self.TechName[team]);
	if Passenger then
		Passenger.AIMode = mode or Actor.AIMODE_SENTRY;
		Passenger.Team = team;
		return Passenger;
	end
end

function BrainvsBrain:CreateRandomInfantry(team, mode)
	local Passenger = RandomAHuman("Actors", self.TechName[team]);
	if Passenger then
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Primary", self.TechName[team]));
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.TechName[team]));

		local rand = math.random();
		if rand < 0.25 then
			Passenger:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", self.TechName[team]));
		elseif rand < 0.50 then
			Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.TechName[team]));
		elseif rand < 0.75 then
			Passenger:AddInventoryItem(RandomHeldDevice("Shields", self.TechName[team]));
		else
			Passenger:AddInventoryItem(CreateHDFirearm("Medikit", "Base.rte"));
		end
		if math.random() < 0.05 then
			Passenger:AddInventoryItem(RandomHDFirearm("Tools - Breaching", self.TechName[team]));
		end

		Passenger.AIMode = mode or Actor.AIMODE_GOTO;
		Passenger.Team = team;
		return Passenger;
	end
end

function BrainvsBrain:CreateLightInfantry(team, mode)
	local Passenger = RandomAHuman("Actors - Light", self.TechName[team]);
	if Passenger.ModuleID ~= PresetMan:GetModuleID(self.TechName[team]) then
		Passenger = RandomAHuman("Actors", self.TechName[team]);
	end

	if Passenger then
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Light", self.TechName[team]));
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.TechName[team]));

		local rand = math.random();
		if rand < 0.5 then
			Passenger:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", self.TechName[team]));
		elseif rand < 0.8 then
			Passenger:AddInventoryItem(CreateHDFirearm("Medikit", "Base.rte"));
		else
			Passenger:AddInventoryItem(RandomHDFirearm("Tools - Breaching", self.TechName[team]));
		end

		Passenger.AIMode = mode or Actor.AIMODE_GOTO;
		Passenger.Team = team;
		return Passenger;
	end
end

function BrainvsBrain:CreateDefender(team)
	local Passenger = RandomAHuman("Actors - Light", self.TechName[team]);
	if Passenger.ModuleID ~= PresetMan:GetModuleID(self.TechName[team]) then
		Passenger = RandomAHuman("Actors", self.TechName[team]);
	end

	if Passenger then
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Light", self.TechName[team]));
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.TechName[team]));

		if math.random() < 0.2 then
			Passenger:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", self.TechName[team]));
		end

		Passenger.AIMode = Actor.AIMODE_SENTRY;
		Passenger.Team = team;
		return Passenger;
	end
end

function BrainvsBrain:CreateHeavyInfantry(team, mode)
	local Passenger = RandomAHuman("Actors - Heavy", self.TechName[team]);
	if Passenger.ModuleID ~= PresetMan:GetModuleID(self.TechName[team]) then
		Passenger = RandomAHuman("Actors", self.TechName[team]);
	end

	if Passenger then
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Heavy", self.TechName[team]));

		if math.random() < 0.3 then
			Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Light", self.TechName[team]));
			if math.random() < 0.25 then
				Passenger:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", self.TechName[team]));
			elseif math.random() < 0.35 then
				Passenger:AddInventoryItem(CreateHDFirearm("Medikit", "Base.rte"));
			end
		else
			Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.TechName[team]));
			if math.random() < 0.3 then
				Passenger:AddInventoryItem(RandomHeldDevice("Shields", self.TechName[team]));
				Passenger:AddInventoryItem(CreateHDFirearm("Medikit", "Base.rte"));
			else
				Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.TechName[team]));
			end
		end

		Passenger.AIMode = mode or Actor.AIMODE_GOTO;
		Passenger.Team = team;
		return Passenger;
	end
end

function BrainvsBrain:CreateMediumInfantry(team, mode)
	local Passenger = RandomAHuman("Actors - Heavy", self.TechName[team]);
	if Passenger.ModuleID ~= PresetMan:GetModuleID(self.TechName[team]) then
		Passenger = RandomAHuman("Actors", self.TechName[team]);
	end

	if Passenger then
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Light", self.TechName[team]));
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.TechName[team]));

		if math.random() < 0.3 then
			Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.TechName[team]));
		else
			Passenger:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", self.TechName[team]));
		end
		if math.random() < 0.5 then
			Passenger:AddInventoryItem(CreateHDFirearm("Medikit", "Base.rte"));
		end

		Passenger.AIMode = mode or Actor.AIMODE_GOTO;
		Passenger.Team = team;
		return Passenger;
	end
end

function BrainvsBrain:CreateScoutInfantry(team, mode)
	local Passenger = RandomAHuman("Actors - Light", self.TechName[team]);
	if Passenger.ModuleID ~= PresetMan:GetModuleID(self.TechName[team]) then
		Passenger = RandomAHuman("Actors", self.TechName[team]);
	end

	if Passenger then
		if math.random() < 0.15 then
			Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Sniper", self.TechName[team]));
		end
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.TechName[team]));

		if math.random() < 0.3 then
			Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.TechName[team]));
		else
			Passenger:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", self.TechName[team]));
			Passenger:AddInventoryItem(CreateHDFirearm("Medikit", "Base.rte"));
		end

		Passenger.AIMode = mode or Actor.AIMODE_GOTO;
		Passenger.Team = team;
		return Passenger;
	end
end

function BrainvsBrain:CreateEngineer(team, mode)
	local Passenger = RandomAHuman("Actors - Light", self.TechName[team]);
	if Passenger.ModuleID ~= PresetMan:GetModuleID(self.TechName[team]) then
		Passenger = RandomAHuman("Actors", self.TechName[team]);
	end

	if Passenger then
		if math.random() < 0.7 then
			Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Light", self.TechName[team]));
		else
			Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.TechName[team]));
			local rand = math.random();
			if rand < 0.2 then
				Passenger:AddInventoryItem(RandomHeldDevice("Shields", self.TechName[team]));
			elseif rand < 0.4 then
				Passenger:AddInventoryItem(CreateHDFirearm("Medikit", "Base.rte"));
			else
				Passenger:AddInventoryItem(RandomTDExplosive("Tools - Breaching", self.TechName[team]));
			end
		end
		Passenger:AddInventoryItem(CreateHDFirearm("Heavy Digger", "Base.rte"));

		Passenger.AIMode = mode or Actor.AIMODE_GOLDDIG;
		Passenger.Team = team;
		return Passenger;
	end
end