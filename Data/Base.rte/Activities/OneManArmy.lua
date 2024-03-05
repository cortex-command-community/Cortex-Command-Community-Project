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

	local actorGroup = "Actors - Heavy";
	local primaryGroup = "Weapons - Heavy";
	local secondaryGroup = "Weapons - Light";

	if self.Difficulty <= GameActivity.CAKEDIFFICULTY then
		self.timeLimit = 3 * 60000 + 5000;
		self.timeDisplay = "three minutes";
		self.baseSpawnTime = 6000;

		primaryGroup = "Weapons - Heavy";
		secondaryGroup = "Weapons - Explosive";
	elseif self.Difficulty <= GameActivity.EASYDIFFICULTY then
		self.timeLimit = 4 * 60000 + 5000;
		self.timeDisplay = "four minutes";
		self.baseSpawnTime = 5500;

	elseif self.Difficulty <= GameActivity.MEDIUMDIFFICULTY then
		self.timeLimit = 5 * 60000 + 5000;
		self.timeDisplay = "five minutes";
		self.baseSpawnTime = 5000;

	elseif self.Difficulty <= GameActivity.HARDDIFFICULTY then
		self.timeLimit = 6 * 60000 + 5000;
		self.timeDisplay = "six minutes";
		self.baseSpawnTime = 4500;

	elseif self.Difficulty <= GameActivity.NUTSDIFFICULTY then
		self.timeLimit = 7 * 60000 + 5000;
		self.timeDisplay = "seven minutes";
		self.baseSpawnTime = 4000;

		actorGroup = "Actors - Light";
		primaryGroup = "Weapons - Primary";
		secondaryGroup = "Weapons - Secondary";
	elseif self.Difficulty <= GameActivity.MAXDIFFICULTY then
		self.timeLimit = 9 * 60000 + 5000;
		self.timeDisplay = "nine minutes";
		self.baseSpawnTime = 3500;

		actorGroup = "Actors - Light";
		primaryGroup = "Weapons - Secondary";
		secondaryGroup = "Weapons - Secondary";
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
	--Default actors if no tech is chosen
	local defaultActor = ("Soldier Heavy");
	local defaultPrimary = ("Coalition/Assault Rifle");
	local defaultSecondary = ("Coalition/Auto Pistol");
	local defaultTertiary = ("Coalition/Frag Grenade");

	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) then
			if not self:GetPlayerBrain(player) then
				local team = self:GetTeamOfPlayer(player);
				local foundBrain = MovableMan:GetUnassignedBrain(team);

				local brainToughnessMultiplier = math.ceil(11 - (self.Difficulty * 0.1));	--Med = 6, Max = 11, Min = 1

				--If we can't find an unassigned brain in the scene to give the player, create one
				if not foundBrain then
					local tech = PresetMan:GetModuleID(self:GetTeamTech(team));
					foundBrain = CreateAHuman(defaultActor);
					--If a faction was chosen, pick the first item from faction listing
					if tech ~= -1 then
						local module = PresetMan:GetDataModule(tech);
						local primaryWeapon, secondaryWeapon, throwable, actor;
						for entity in module.Presets do
							local picked;	--Prevent duplicates
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
							local item = weapons[i];
							if item then
								item.GibWoundLimit = item.GibWoundLimit * brainToughnessMultiplier;
								item.JointStrength = item.JointStrength * brainToughnessMultiplier;
								foundBrain:AddInventoryItem(weapons[i]);
							end
						end
					else
						--If no tech selected, use default items
						local weapons = {defaultPrimary, defaultSecondary};
						for i = 1, #weapons do
							local item = CreateHDFirearm(weapons[i]);
							if item then
								item.GibWoundLimit = item.GibWoundLimit and item.GibWoundLimit * brainToughnessMultiplier or item.GibWoundLimit;
								item.JointStrength = item.JointStrength * brainToughnessMultiplier;
								foundBrain:AddInventoryItem(CreateHDFirearm(weapons[i]));
							end
						end
						local item = CreateTDExplosive(defaultTertiary);
						if item then
							foundBrain:AddInventoryItem(item);
						end
					end
					--Reinforce the brain actor
					local parts = {foundBrain, foundBrain.Head, foundBrain.FGArm, foundBrain.BGArm, foundBrain.FGLeg, foundBrain.BGLeg};
					for i = 1, #parts do
						local part = parts[i];
						if part then
							part.GibWoundLimit = math.ceil(part.GibWoundLimit * brainToughnessMultiplier);
							part.DamageMultiplier = part.DamageMultiplier/brainToughnessMultiplier;
							if IsAttachable(part) then
								ToAttachable(part).JointStrength = ToAttachable(part).JointStrength * brainToughnessMultiplier;
							else
								part.GibImpulseLimit = foundBrain.GibImpulseLimit * brainToughnessMultiplier;
								part.ImpulseDamageThreshold = foundBrain.GibImpulseLimit * brainToughnessMultiplier;
							end
							for att in part.Attachables do
								att.GibWoundLimit = math.ceil(att.GibWoundLimit * brainToughnessMultiplier);
								att.JointStrength = att.JointStrength * brainToughnessMultiplier;
							end
						end
					end
					local medikit = CreateHDFirearm("Base/Medikit");
					if medikit then
						foundBrain:AddInventoryItem(medikit);
					end
					--Reinforce FGArm so that we don't lose it
					--No FGArm = no weapons = no gameplay
					foundBrain.FGArm.GibWoundLimit = 999999;
					foundBrain.FGArm.JointStrength = 999999;

					foundBrain.Pos = SceneMan:MovePointToGround(Vector(math.random(0, SceneMan.SceneWidth), 0), 0, 0) + Vector(0, -foundBrain.Radius);
					foundBrain.Team = self:GetTeamOfPlayer(player);
					MovableMan:AddActor(foundBrain);
					--Set the found brain to be the selected actor at start
					self:SetPlayerBrain(foundBrain, player);
					foundBrain:AddToGroup("Brains");
					self:SwitchToActor(foundBrain, player, self:GetTeamOfPlayer(player));
					self:SetLandingZone(self:GetPlayerBrain(player).Pos, player);
					--Set the observation target to the brain, so that if/when it dies, the view flies to it in observation mode
					self:SetObservationTarget(self:GetPlayerBrain(player).Pos, player);
				else
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
		--Play sad music if no humans are left
		if self:HumanBrainCount() == 0 then
			AudioMan:ClearMusicQueue();
			AudioMan:PlayMusic("Base.rte/Music/dBSoundworks/udiedfinal.ogg", 2, -1.0);
			AudioMan:QueueSilence(10);
			AudioMan:QueueMusicStream("Base.rte/Music/dBSoundworks/ccambient4.ogg");
		else
			--But if humans are left, play happy music!
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
				--Display messages
				if self.startMessageTimer:IsPastSimMS(3000) then
					FrameMan:SetScreenText(math.floor(self.winTimer:LeftTillSimMS(self.timeLimit) * 0.001) .. " seconds left", self:ScreenOfPlayer(player), 0, 1000, false);
				else
					FrameMan:SetScreenText("Survive for " .. self.timeDisplay .. "!", self:ScreenOfPlayer(player), 333, 5000, true);
				end

				local team = self:GetTeamOfPlayer(player);
				--Check if any player's brain is dead
				if not MovableMan:IsActor(self:GetPlayerBrain(player)) then
					self:SetPlayerBrain(nil, player);
					self:ResetMessageTimer(player);
					FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
					FrameMan:SetScreenText("Your brain has been destroyed!", self:ScreenOfPlayer(player), 333, -1, false);
					--Now see if all brains of self player's team are dead, and if so, end the game
					if not MovableMan:GetFirstBrainActor(team) then
						self.WinnerTeam = self:OtherTeam(team);
						ActivityMan:EndActivity();
					end
				end

				--Check if the player has won
				if self.winTimer:IsPastSimMS(self.timeLimit) then
					self:ResetMessageTimer(player);
					FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
					FrameMan:SetScreenText("You survived!", self:ScreenOfPlayer(player), 333, -1, false);

					self.WinnerTeam = team;

					--Kill all enemies
					for actor in MovableMan.Actors do
						if actor.Team ~= self.WinnerTeam then
							actor.Health = 0;
						end
					end

					ActivityMan:EndActivity();
				end
			end
		end

		--Spawn the AI
		if self.CPUTeam ~= Activity.NOTEAM and self.enemySpawnTimer:LeftTillSimMS(self.enemySpawnTimeLimit) <= 0 then
			local ship, actorsInCargo;

			if math.random() < 0.5 then
				--Set up the ship to deliver this stuff
				ship = RandomACDropShip("Any", self.CPUTechName);
				--If we can't afford this dropship, then try a rocket instead
				if ship:GetTotalValue(0, 3) > self:GetTeamFunds(self.CPUTeam) then
					DeleteEntity(ship);
					ship = RandomACRocket("Any", self.CPUTechName);
				end
				actorsInCargo = math.min(ship.MaxPassengers, 3);
			else
				ship = RandomACRocket("Any", self.CPUTechName);
				actorsInCargo = math.min(ship.MaxPassengers, 2);
			end

			ship.Team = self.CPUTeam;

			--The max allowed weight of this craft plus cargo
			local craftMaxMass = ship.MaxInventoryMass;
			if craftMaxMass < 0 then
				craftMaxMass = math.huge;
			elseif craftMaxMass < 1 then
				ship = RandomACDropShip("Any", 0);
				craftMaxMass = ship.MaxInventoryMass;
			end
			local totalInventoryMass = 0;

			--Set the ship up with a cargo of a few armed and equipped actors
			for i = 1, actorsInCargo do
				--Get any Actor from the CPU's native tech
				local passenger = nil;
				if math.random() >= self:GetCrabToHumanSpawnRatio(PresetMan:GetModuleID(self.CPUTechName)) then
					passenger = RandomAHuman("Actors - Light", self.CPUTechName);
					if passenger.ModuleID ~= PresetMan:GetModuleID(self.CPUTechName) then
						passenger = RandomAHuman("Actors", self.CPUTechName);
					end
				else
					passenger = RandomACrab("Actors - Mecha", self.CPUTechName);
				end

				--Equip it with tools and guns if it's a humanoid
				if IsAHuman(passenger) then
					passenger:AddInventoryItem(RandomHDFirearm("Weapons - Light", self.CPUTechName));
					passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.CPUTechName));
					if math.random() < 0.5 then
						passenger:AddInventoryItem(RandomHDFirearm("Tools - Diggers", self.CPUTechName));
					end
				end
				--Set AI mode and team so it knows who and what to fight for!
				passenger.AIMode = Actor.AIMODE_BRAINHUNT;
				passenger.Team = self.CPUTeam;

				--Check that we can afford to buy and to carry the weight of this passenger
				if (ship:GetTotalValue(0, 3) + passenger:GetTotalValue(0, 3)) <= self:GetTeamFunds(self.CPUTeam) and (totalInventoryMass + passenger.Mass) <= craftMaxMass then
					--Yes we can; so add it to the cargo hold
					ship:AddInventoryItem(passenger);
					totalInventoryMass = totalInventoryMass + passenger.Mass;
					passenger = nil;
				else
					--Nope; just delete the nixed passenger and stop adding new ones
					--This doesn't need to be explicitly deleted here, the garbage collection would do it eventually,
					--but since we're so sure we don't need it, might as well go ahead and do it right away
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
				--Set the spawn point of the ship from orbit
				if self.HumanCount == 1 then
					for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
						if self:PlayerActive(player) and self:PlayerHuman(player) then
							local sceneChunk = SceneMan.SceneWidth * 0.3;
							local checkPos = self:GetPlayerBrain(player).Pos.X + (SceneMan.SceneWidth * 0.5) + ((sceneChunk * 0.5) - (math.random() * sceneChunk));
							if checkPos > SceneMan.SceneWidth then
								checkPos = checkPos - SceneMan.SceneWidth;
							elseif checkPos < 0 then
								checkPos = SceneMan.SceneWidth + checkPos;
							end
							ship.Pos = Vector(checkPos, -50);
							break;
						end
					end
				else
					if SceneMan.SceneWrapsX then
						ship.Pos = Vector(math.random() * SceneMan.SceneWidth, -50);
					else
						ship.Pos = Vector(RangeRand(100, SceneMan.SceneWidth - 100), -50);
					end
				end

				--Double-check if the computer can afford this ship and cargo, then subtract the total value from the team's funds
				local shipValue = ship:GetTotalValue(0, 3);
				if shipValue <= self:GetTeamFunds(self.CPUTeam) then
					--Subtract the total value of the ship+cargo from the CPU team's funds
					self:ChangeTeamFunds(-shipValue, self.CPUTeam);
					--Spawn the ship onto the scene
					MovableMan:AddActor(ship);
				else
					--The ship and its contents is deleted if it can't be afforded
					DeleteEntity(ship);
					ship = nil;
				end
			end

			self.enemySpawnTimer:Reset();
			self.enemySpawnTimeLimit = (self.baseSpawnTime + math.random(self.baseSpawnTime)) * rte.SpawnIntervalScale;
		end
	end
end
