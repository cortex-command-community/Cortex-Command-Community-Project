function OneManArmy:StartActivity(isNewGame)
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

function OneManArmy:OnSave()
	self:SaveNumber("startMessageTimer.ElapsedSimTimeMS", self.startMessageTimer.ElapsedSimTimeMS);
	self:SaveNumber("enemySpawnTimer.ElapsedSimTimeMS", self.enemySpawnTimer.ElapsedSimTimeMS);
	self:SaveNumber("winTimer.ElapsedSimTimeMS", self.winTimer.ElapsedSimTimeMS);

	self:SaveNumber("timeLimit", self.timeLimit);
	self:SaveString("timeDisplay", self.timeDisplay);
	self:SaveNumber("baseSpawnTime", self.baseSpawnTime);
	self:SaveNumber("enemySpawnTimeLimit", self.enemySpawnTimeLimit);
end

function OneManArmy:StartNewGame()
	self:SetTeamFunds(1000000, self.CPUTeam);
	self:SetTeamFunds(0, Activity.TEAM_1);

	local actorGroup = "Actors - Light";
	local primaryGroup = "Weapons - Primary";
	local secondaryGroup = "Weapons - Secondary";

	if self.Difficulty <= GameActivity.CAKEDIFFICULTY then
		self.timeLimit = 5 * 60000 + 5000;
		self.timeDisplay = "five minutes";
		self.baseSpawnTime = 6000;

		actorGroup = "Actors - Heavy";
		primaryGroup = "Weapons - Heavy";
		secondaryGroup = "Weapons - Explosive";
	elseif self.Difficulty <= GameActivity.EASYDIFFICULTY then
		self.timeLimit = 5 * 60000 + 5000;
		self.timeDisplay = "five minutes";
		self.baseSpawnTime = 5500;

		actorGroup = "Actors - Heavy";
		primaryGroup = "Weapons - Primary";
		secondaryGroup = "Weapons - Light";
	elseif self.Difficulty <= GameActivity.MEDIUMDIFFICULTY then
		self.timeLimit = 5 * 60000 + 5000;
		self.timeDisplay = "five minutes";
		self.baseSpawnTime = 5000;

	elseif self.Difficulty <= GameActivity.HARDDIFFICULTY then
		self.timeLimit = 6 * 60000 + 5000;
		self.timeDisplay = "six minutes";
		self.baseSpawnTime = 4500;

		primaryGroup = "Weapons - Light";
		secondaryGroup = "Weapons - Secondary";
	elseif self.Difficulty <= GameActivity.NUTSDIFFICULTY then
		self.timeLimit = 8 * 60000 + 5000;
		self.timeDisplay = "eight minutes";
		self.baseSpawnTime = 4000;

		primaryGroup = "Weapons - Secondary";
		secondaryGroup = "Weapons - Secondary";
	elseif self.Difficulty <= GameActivity.MAXDIFFICULTY then
		self.timeLimit = 10 * 60000 + 5000;
		self.timeDisplay = "ten minutes";
		self.baseSpawnTime = 3500;

		primaryGroup = "Weapons - Secondary";
		secondaryGroup = "Tools";
	end
	self.enemySpawnTimeLimit = 500;

	MovableMan:OpenAllDoors(true, -1);
	for actor in MovableMan.AddedActors do
		if actor.ClassName == "ADoor" then
			actor.ToSettle = true;
			actor:GibThis();
		end
	end

	self:SetupHumanPlayerBrains(actorGroup, primaryGroup, secondaryGroup);
end

function OneManArmy:SetupHumanPlayerBrains(actorGroup, primaryGroup, secondaryGroup)
	-- Default actors if no tech is chosen
	local defaultActor = ("Soldier Light");
	local defaultPrimary = ("Ronin/SPAS 12");
	local defaultSecondary = ("Ronin/.357 Magnum");
	local defaultTertiary = ("Ronin/Molotov Cocktail");

	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) then
			if not self:GetPlayerBrain(player) then
				local team = self:GetTeamOfPlayer(player);
				local foundBrain = MovableMan:GetUnassignedBrain(team);
				-- If we can't find an unassigned brain in the scene to give the player, create one
				if not foundBrain then
					local tech = PresetMan:GetModuleID(self:GetTeamTech(team));
					foundBrain = CreateAHuman(defaultActor);
					-- If a faction was chosen, pick the first item from faction listing
					if tech ~= -1 then
						local module = PresetMan:GetDataModule(tech);
						local primaryWeapon, secondaryWeapon, throwable, actor;
						for entity in module.Presets do
							local picked;	-- Prevent duplicates
							if not primaryWeapon and entity.ClassName == "HDFirearm" and ToMOSRotating(entity):HasObjectInGroup(primaryGroup) and ToMOSRotating(entity).Buyable then
								primaryWeapon = CreateHDFirearm(entity:GetModuleAndPresetName());
								picked = true;
							end
							if not picked and not secondaryWeapon and entity.ClassName == "HDFirearm" and ToMOSRotating(entity):HasObjectInGroup(secondaryGroup) and ToMOSRotating(entity).Buyable then
								secondaryWeapon = CreateHDFirearm(entity:GetModuleAndPresetName());
								picked = true;
							end
							if not picked and not throwable and entity.ClassName == "TDExplosive" and ToMOSRotating(entity):HasObjectInGroup("Bombs - Grenades") and ToMOSRotating(entity).Buyable then
								throwable = CreateTDExplosive(entity:GetModuleAndPresetName());
								picked = true;
							end
							if not picked and not actor and entity.ClassName == "AHuman" and ToMOSRotating(entity):HasObjectInGroup(actorGroup) and ToMOSRotating(entity).Buyable then
								actor = CreateAHuman(entity:GetModuleAndPresetName());
							end
						end
						if actor then
							foundBrain = actor;
						end
						local weapons = {primaryWeapon, secondaryWeapon, throwable};
						for i = 1, #weapons do
							if weapons[i] then
								foundBrain:AddInventoryItem(weapons[i]);
							end
						end
					else	-- If no tech selected, use default items
						local weapons = {defaultPrimary, defaultSecondary};
						for i = 1, #weapons do
							foundBrain:AddInventoryItem(CreateHDFirearm(weapons[i]));
						end
						local item = CreateTDExplosive(defaultTertiary);
						if item then
							foundBrain:AddInventoryItem(item);
						end
					end
					foundBrain.Pos = SceneMan:MovePointToGround(Vector(math.random(0, SceneMan.SceneWidth), 0), 0, 0) + Vector(0, -foundBrain.Radius);
					foundBrain.Team = self:GetTeamOfPlayer(player);
					MovableMan:AddActor(foundBrain);
					-- Set the found brain to be the selected actor at start
					self:SetPlayerBrain(foundBrain, player);
					foundBrain:AddToGroup("Brains");
					self:SwitchToActor(foundBrain, player, self:GetTeamOfPlayer(player));
					self:SetLandingZone(self:GetPlayerBrain(player).Pos, player);
					-- Set the observation target to the brain, so that if/when it dies, the view flies to it in observation mode
					self:SetObservationTarget(self:GetPlayerBrain(player).Pos, player);
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

function OneManArmy:ResumeLoadedGame()
	self.startMessageTimer.ElapsedSimTimeMS = self:LoadNumber("startMessageTimer.ElapsedSimTimeMS");
	self.enemySpawnTimer.ElapsedSimTimeMS = self:LoadNumber("enemySpawnTimer.ElapsedSimTimeMS");
	self.winTimer.ElapsedSimTimeMS = self:LoadNumber("winTimer.ElapsedSimTimeMS");

	self.timeLimit = self:LoadNumber("timeLimit");
	self.timeDisplay = self:LoadString("timeDisplay");
	self.baseSpawnTime = self:LoadNumber("baseSpawnTime");
	self.enemySpawnTimeLimit = self:LoadNumber("enemySpawnTimeLimit");

	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) then
			if not self:GetPlayerBrain(player) then
				local team = self:GetTeamOfPlayer(player);
				local foundBrain = MovableMan:GetUnassignedBrain(team);
				if foundBrain then
					--Set the found brain to be the selected actor at start
					self:SetPlayerBrain(foundBrain, player);
					self:SwitchToActor(foundBrain, player, self:GetTeamOfPlayer(player));
					--Set the observation target to the brain, so that if/when it dies, the view flies to it in observation mode
					self:SetObservationTarget(self:GetPlayerBrain(player).Pos, player);
				end
			end
		end
	end
end

function OneManArmy:EndActivity()
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

function OneManArmy:UpdateActivity()
	if self.ActivityState ~= Activity.OVER then
		ActivityMan:GetActivity():SetTeamFunds(0, Activity.TEAM_1);
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self:PlayerActive(player) and self:PlayerHuman(player) then
				--Display messages.
				if self.startMessageTimer:IsPastSimMS(3000) then
					FrameMan:SetScreenText(math.floor(self.winTimer:LeftTillSimMS(self.timeLimit) / 1000) .. " seconds left", self:ScreenOfPlayer(player), 0, 1000, false);
				else
					FrameMan:SetScreenText("Survive for " .. self.timeDisplay .. "!", self:ScreenOfPlayer(player), 333, 5000, true);
				end

				local team = self:GetTeamOfPlayer(player);
				-- Check if any player's brain is dead
				if not MovableMan:IsActor(self:GetPlayerBrain(player)) then
					self:SetPlayerBrain(nil, player);
					self:ResetMessageTimer(player);
					FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
					FrameMan:SetScreenText("Your brain has been destroyed!", self:ScreenOfPlayer(player), 333, -1, false);
					-- Now see if all brains of self player's team are dead, and if so, end the game
					if not MovableMan:GetFirstBrainActor(team) then
						self.WinnerTeam = self:OtherTeam(team);
						ActivityMan:EndActivity();
					end
				end

				--Check if the player has won.
				if self.winTimer:IsPastSimMS(self.timeLimit) then
					self:ResetMessageTimer(player);
					FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
					FrameMan:SetScreenText("You survived!", self:ScreenOfPlayer(player), 333, -1, false);

					self.WinnerTeam = team;

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
			-- Set up the ship to deliver this stuff
			local ship = RandomACRocket("Any", self.CPUTechName);
			local actorsInCargo = math.min(ship.MaxPassengers, 2);

			ship.Team = self.CPUTeam;

			-- The max allowed weight of this craft plus cargo
			local craftMaxMass = ship.MaxInventoryMass;
			if craftMaxMass < 0 then
				craftMaxMass = math.huge;
			elseif craftMaxMass < 1 then
				ship = RandomACRocket("Any", 0);
				craftMaxMass = ship.MaxInventoryMass;
			end
			local totalInventoryMass = 0;

			-- Set the ship up with a cargo of a few armed and equipped actors
			for i = 1, actorsInCargo do
				local passenger = RandomAHuman("Actors - " .. ((self.Difficulty > 75 and math.random() > 0.5) and "Heavy" or "Light"), self.CPUTechName);
				if passenger.ModuleID ~= PresetMan:GetModuleID(self.CPUTechName) then
					passenger = RandomAHuman("Actors", self.CPUTechName);
				end

				passenger:AddInventoryItem(CreateHDFirearm("Light Digger", "Base.rte"));

				-- Set AI mode and team so it knows who and what to fight for!
				passenger.AIMode = Actor.AIMODE_BRAINHUNT;
				passenger.Team = self.CPUTeam;

				-- Check that we can afford to buy and to carry the weight of this passenger
				if (ship:GetTotalValue(0,3) + passenger:GetTotalValue(0,3)) <= self:GetTeamFunds(self.CPUTeam) and (totalInventoryMass + passenger.Mass) <= craftMaxMass then
					-- Yes we can; so add it to the cargo hold
					ship:AddInventoryItem(passenger);
					totalInventoryMass = totalInventoryMass + passenger.Mass;
					passenger = nil;
				else
					-- Nope; just delete the nixed passenger and stop adding new ones
					-- This doesn't need to be explicitly deleted here, teh garbage collection would do it eventually..
					-- but since we're so sure we don't need it, might as well go ahead and do it here right away
					DeleteEntity(passenger);
					passenger = nil;

					if i < 2 then	-- Don't deliver empty craft
						DeleteEntity(ship);
						ship = nil;
					end

					break;
				end
			end

			if ship then
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

				-- Double-check if the computer can afford this ship and cargo, then subtract the total value from the team's funds
				local shipValue = ship:GetTotalValue(0,3);
				if shipValue <= self:GetTeamFunds(self.CPUTeam) then
					-- Subtract the total value of the ship+cargo from the CPU team's funds
					self:ChangeTeamFunds(-shipValue, self.CPUTeam);
					-- Spawn the ship onto the scene
					MovableMan:AddActor(ship);
				else
					-- The ship and its contents is deleted if it can't be afforded
					-- This doesn't need to be explicitly deleted here, teh garbage collection would do it eventually..
					-- but since we're so sure we don't need it, might as well go ahead and do it here right away
					DeleteEntity(ship);
					ship = nil;
				end
			end

			self.enemySpawnTimer:Reset();
			self.enemySpawnTimeLimit = (self.baseSpawnTime + math.random(self.baseSpawnTime) * rte.SpawnIntervalScale);
		end
	end
end
