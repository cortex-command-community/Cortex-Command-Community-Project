function KeepieUppie:StartActivity(isNewGame)
	SceneMan.Scene:GetArea("LZ Team 1");
	SceneMan.Scene:GetArea("LZ All");

	self.BuyMenuEnabled = false;

	self.startMessageTimer = Timer();
	self.enemySpawnTimer = Timer();
	self.winTimer = Timer();

	self.CPUTechName = self:GetTeamTech(self.CPUTeam);

	if isNewGame then
		self:StartNewGame();
	else
		self:ResumeLoadedGame();
	end
end

function KeepieUppie:OnSave()
	self:SaveNumber("playerRocketSpawned", self.playerRocketSpawned and 1 or 0);

	self:SaveNumber("startMessageTimer.ElapsedSimTimeMS", self.startMessageTimer.ElapsedSimTimeMS);
	self:SaveNumber("enemySpawnTimer.ElapsedSimTimeMS", self.enemySpawnTimer.ElapsedSimTimeMS);
	self:SaveNumber("winTimer.ElapsedSimTimeMS", self.winTimer.ElapsedSimTimeMS);

	self:SaveNumber("timeLimit", self.timeLimit);
	self:SaveString("timeDisplay", self.timeDisplay);
	self:SaveNumber("baseSpawnTime", self.baseSpawnTime);
	self:SaveNumber("randomSpawnTime", self.randomSpawnTime);
	self:SaveNumber("enemySpawnTimeLimit", self.enemySpawnTimeLimit);
end

function KeepieUppie:StartNewGame()
	self:SetTeamFunds(1000000, self.CPUTeam);
	self:SetTeamFunds(0, Activity.TEAM_1);

	self.playerRocketSpawned = false;

	if self.Difficulty <= GameActivity.CAKEDIFFICULTY then
		self.timeLimit = 25000;
		self.timeDisplay = "twenty seconds";
		self.baseSpawnTime = 6000;
		self.randomSpawnTime = 8000;
	elseif self.Difficulty <= GameActivity.EASYDIFFICULTY then
		self.timeLimit = 45000;
		self.timeDisplay = "forty seconds";
		self.baseSpawnTime = 5500;
		self.randomSpawnTime = 7000;
	elseif self.Difficulty <= GameActivity.MEDIUMDIFFICULTY then
		self.timeLimit = 65000;
		self.timeDisplay = "one minute";
		self.baseSpawnTime = 5000;
		self.randomSpawnTime = 6000;
	elseif self.Difficulty <= GameActivity.HARDDIFFICULTY then
		self.timeLimit = 95000;
		self.timeDisplay = "one minute and thirty seconds";
		self.baseSpawnTime = 4500;
		self.randomSpawnTime = 5000;
	elseif self.Difficulty <= GameActivity.NUTSDIFFICULTY then
		self.timeLimit = 125000;
		self.timeDisplay = "two minutes and thirty seconds";
		self.baseSpawnTime = 4000;
		self.randomSpawnTime = 4500;
	elseif self.Difficulty <= GameActivity.MAXDIFFICULTY then
		self.timeLimit = 305000;
		self.timeDisplay = "five minutes";
		self.baseSpawnTime = 3500;
		self.randomSpawnTime = 4000;
	end
	self.enemySpawnTimeLimit = 2000;

	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) then
			self:AddOverridePurchase(CreateACRocket("Rocklet"), player);
			self:SetViewState(Activity.LZSELECT, player);
		end
	end
end

function KeepieUppie:ResumeLoadedGame()
	self.playerRocketSpawned = self:LoadNumber("playerRocketSpawned") ~= 0;

	self.startMessageTimer.ElapsedSimTimeMS = self:LoadNumber("startMessageTimer.ElapsedSimTimeMS");
	self.enemySpawnTimer.ElapsedSimTimeMS = self:LoadNumber("enemySpawnTimer.ElapsedSimTimeMS");
	self.winTimer.ElapsedSimTimeMS = self:LoadNumber("winTimer.ElapsedSimTimeMS");

	self.timeLimit = self:LoadNumber("timeLimit");
	self.timeDisplay = self:LoadString("timeDisplay");
	self.baseSpawnTime = self:LoadNumber("baseSpawnTime");
	self.randomSpawnTime = self:LoadNumber("randomSpawnTime");
	self.enemySpawnTimeLimit = self:LoadNumber("enemySpawnTimeLimit");

	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) then
			for actor in MovableMan.Actors do
				if actor.ClassName == "ACRocket" and actor.Team == Activity.TEAM_1 then
					self:SetPlayerBrain(actor, player);
					self:SetObservationTarget(actor.Pos, player);
					self:SwitchToActor(actor, player, player);

					self.playerRocketSpawned = true;
				end
			end
		end
	end
	
	if not self.playerRocketSpawned then
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self:PlayerActive(player) and self:PlayerHuman(player) then
				self:AddOverridePurchase(CreateACRocket("Rocklet"), player);
				self:SetViewState(Activity.LZSELECT, player);
			end
		end
	end
end

function KeepieUppie:EndActivity()
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

function KeepieUppie:UpdateActivity()
	self:ClearObjectivePoints();

	if self.playerRocketSpawned then
		if self.ActivityState ~= Activity.OVER and self.ActivityState ~= Activity.EDITING then
			for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
				if self:PlayerActive(player) and self:PlayerHuman(player) then
					--Display messages.
					self:ResetMessageTimer(player);
					FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
					if self.startMessageTimer:IsPastSimMS(3000) then
						FrameMan:SetScreenText(math.floor(self.winTimer:LeftTillSimMS(self.timeLimit) / 1000) .. " seconds left", self:ScreenOfPlayer(player), 0, 1000, false);
					else
						FrameMan:SetScreenText("Keep the rocket alive for " .. self.timeDisplay .. "!", self:ScreenOfPlayer(player), 333, 5000, true);
					end

					-- The current player's team
					local team = self:GetTeamOfPlayer(player);
					-- Check if any player's brain is dead
					if not MovableMan:IsActor(self:GetPlayerBrain(player)) then
						self:SetPlayerBrain(nil, player);
						self:ResetMessageTimer(player);
						FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
						FrameMan:SetScreenText("Your rocket has been destroyed!", self:ScreenOfPlayer(player), 333, -1, false);
						-- Now see if all brains of self player's team are dead, and if so, end the game
						if not MovableMan:GetFirstBrainActor(team) then
							self.WinnerTeam = self:OtherTeam(team);
							ActivityMan:EndActivity();
						end
					else
						self.HuntPlayer = player;
					end

					--Check if the player has won.
					if self.winTimer:IsPastSimMS(self.timeLimit) then
						self:ResetMessageTimer(player);
						FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
						FrameMan:SetScreenText("You survived!", self:ScreenOfPlayer(player), 333, -1, false);

						self.WinnerTeam = player;

						--Kill all enemies.
						for actor in MovableMan.Actors do
							if actor.Team ~= self.WinnerTeam then
								actor.Health = 0;
							end
						end

						ActivityMan:EndActivity();
					end
				end
			end

			--Spawn the AI.
			if self.CPUTeam ~= Activity.NOTEAM and self.enemySpawnTimer:LeftTillSimMS(self.enemySpawnTimeLimit) <= 0 then
				local ship, actorsInCargo;

				if math.random() < 0.5 then
					-- Set up the ship to deliver this stuff
					ship = RandomACDropShip("Any", self.CPUTechName);
					actorsInCargo = ship.MaxPassengers;
				else
					ship = RandomACRocket("Any", self.CPUTechName);
					actorsInCargo = math.min(ship.MaxPassengers, 2);
				end

				ship.Team = self.CPUTeam;

				-- Set the ship up with a cargo of a few armed and equipped actors
				for i = 1, actorsInCargo do
					-- Get any Actor from the CPU's native tech
					local passenger = nil;
					if math.random() >= self:GetCrabToHumanSpawnRatio(PresetMan:GetModuleID(self.CPUTechName)) then
						passenger = RandomAHuman("Any", self.CPUTechName);
					else
						passenger = RandomACrab("Any", self.CPUTechName);
					end
					-- Equip it with tools and guns if it's a humanoid
					if IsAHuman(passenger) then
						passenger:AddInventoryItem(RandomHDFirearm("Weapons - Primary", self.CPUTechName));
						passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.CPUTechName));
						if math.random() < 0.5 then
							passenger:AddInventoryItem(RandomHDFirearm("Tools - Diggers", self.CPUTechName));
						end
					end
					-- Set AI mode and team so it knows who and what to fight for!
					passenger.AIMode = Actor.AIMODE_BRAINHUNT;
					passenger.Team = self.CPUTeam;
					ship:AddInventoryItem(passenger);
				end

				-- Set the spawn point of the ship from orbit
				if self.HumanCount == 1 then
					for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
						if self:PlayerActive(player) and self:PlayerHuman(player) then
							local sceneChunk = SceneMan.SceneWidth / 3;
							local checkPos = self:GetPlayerBrain(player).Pos.X + (SceneMan.SceneWidth/2) + ( (sceneChunk/2) - (math.random()*sceneChunk) );
							if checkPos > SceneMan.SceneWidth then
								checkPos = checkPos - SceneMan.SceneWidth;
							elseif checkPos < 0 then
								checkPos = SceneMan.SceneWidth + checkPos;
							end
							ship.Pos = Vector(checkPos,-50);
							break;
						end
					end
				else
					if SceneMan.SceneWrapsX then
						ship.Pos = Vector(math.random() * SceneMan.SceneWidth, -50);
					else
						ship.Pos = Vector(RangeRand(100, SceneMan.SceneWidth-100), -50);
					end
				end

				-- Spawn the ship onto the scene
				MovableMan:AddActor(ship);

				self.enemySpawnTimer:Reset();
				self.enemySpawnTimeLimit = (self.baseSpawnTime + math.random(self.randomSpawnTime) * rte.SpawnIntervalScale);
			end
		end
	else
		self.startMessageTimer:Reset();
		self.winTimer:Reset();

		FrameMan:SetScreenText("Order your rocket...", 0, 0, 5000, false);

		--See if the rocket has spawned yet.
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self:PlayerActive(player) and self:PlayerHuman(player) then
				if self:GetDeliveryCount(self:GetTeamOfPlayer(player)) < 1 then
					self:SetViewState(Activity.LZSELECT, player);
				end
				for actor in MovableMan.Actors do
					if actor.ClassName == "ACRocket" and actor.Team == Activity.TEAM_1 then
						self:SetPlayerBrain(actor, player);
						actor:AddToGroup("Brains");
						self:SetObservationTarget(actor.Pos, player);
						self:SwitchToActor(actor, player, Activity.TEAM_1);

						self.playerRocketSpawned = true;
						break;
					end
				end
			end
		end
	end

	self:SetTeamFunds(0, Activity.TEAM_1);

	self:YSortObjectivePoints();
end