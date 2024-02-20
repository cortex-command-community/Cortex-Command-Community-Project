function OneManArmyZeroG:StartActivity(isNewGame)
	SceneMan.Scene:GetArea("OneManArmyZeroGCompatibilityArea");

	self.BuyMenuEnabled = false;

	self.startMessageTimer = Timer();
	self.enemySpawnTimer = Timer();
	self.winTimer = Timer();

	self.CPUTechName = self:GetTeamTech(self.CPUTeam);
	
	self.isDiggersOnly = self.PresetName:find("Diggers") ~= nil;

	if isNewGame then
		self:StartNewGame();
	else
		self:ResumeLoadedGame();
	end
end

function OneManArmyZeroG:OnSave()
	self:SaveNumber("startMessageTimer.ElapsedSimTimeMS", self.startMessageTimer.ElapsedSimTimeMS);
	self:SaveNumber("enemySpawnTimer.ElapsedSimTimeMS", self.enemySpawnTimer.ElapsedSimTimeMS);
	self:SaveNumber("winTimer.ElapsedSimTimeMS", self.winTimer.ElapsedSimTimeMS);

	self:SaveNumber("timeLimit", self.timeLimit);
	self:SaveString("timeDisplay", self.timeDisplay);
	self:SaveNumber("baseSpawnTime", self.baseSpawnTime);
	self:SaveNumber("enemySpawnTimeLimit", self.enemySpawnTimeLimit);
end

function OneManArmyZeroG:StartNewGame()
	self:SetTeamFunds(1000000, self.CPUTeam);
	self:SetTeamFunds(0, Activity.TEAM_1);

	local actorGroup = self.isDiggersOnly and "Actors - Light" or "Actors - Heavy";
	local primaryGroup = self.isDiggersOnly and "Weapons - CQB" or "Weapons - Heavy";
	local secondaryGroup = self.isDiggersOnly and "Weapons - Secondary" or "Weapons - Light";
	local tertiaryGroup = "Weapons - Secondary";

	local timeLimitMinutes;
	local timeLimitText;
	if self.Difficulty <= GameActivity.CAKEDIFFICULTY then
		timeLimitMinutes = self.isDiggersOnly and 5 or 3;
		timeLimitText = self.isDiggersOnly and "five" or "three";
		self.baseSpawnTime = 6000;

		actorGroup = "Actors - Heavy";
		primaryGroup = "Weapons - Heavy";
		secondaryGroup = "Weapons - Explosive";
		tertiaryGroup = "Weapons - Light";
	elseif self.Difficulty <= GameActivity.EASYDIFFICULTY then
		timeLimitMinutes = self.isDiggersOnly and 5 or 4;
		timeLimitText = self.isDiggersOnly and "five" or "four";
		self.baseSpawnTime = 5500;

		actorGroup = "Actors - Heavy";
		secondaryGroup = "Weapons - Light";
	elseif self.Difficulty <= GameActivity.MEDIUMDIFFICULTY then
		timeLimitMinutes = 5;
		timeLimitText = "five";
		self.baseSpawnTime = 5000;

	elseif self.Difficulty <= GameActivity.HARDDIFFICULTY then
		timeLimitMinutes = 6;
		timeLimitText = "six";
		self.baseSpawnTime = 4500;

	elseif self.Difficulty <= GameActivity.NUTSDIFFICULTY then
		timeLimitMinutes = self.isDiggersOnly and 8 or 7;
		timeLimitText = self.isDiggersOnly and "eight" or "seven";
		self.baseSpawnTime = 4000;

		actorGroup = "Actors - Light";
		secondaryGroup = "Weapons - Secondary";
	elseif self.Difficulty <= GameActivity.MAXDIFFICULTY then
		timeLimitMinutes = self.isDiggersOnly and 10 or 9;
		timeLimitText = self.isDiggersOnly and "ten" or "nine";
		self.baseSpawnTime = 3500;

		actorGroup = "Actors - Light";
		primaryGroup = self.isDiggersOnly and "Weapons - CQB" or "Weapons - Primary";
		secondaryGroup = "Weapons - Secondary";
	end
	self.timeLimit = (timeLimitMinutes * 60000) + 5000;
	self.timeDisplay = timeLimitText .. " minutes";
	self.enemySpawnTimeLimit = 500;
	
	local automoverController = CreateActor("Invisible Automover Controller", "Base.rte");
	automoverController.Team = -1;
	automoverController:SetNumberValue("MovementSpeed", 16);
	automoverController:SetNumberValue("ActorUnstickingDisabled", 1);
	automoverController:SetNumberValue("SlowActorVelInNoneMovementDirectionsWhenInZoneBoxDisabled", 1);
	MovableMan:AddActor(automoverController);

	local superNode = CreateMOSRotating("Automover Node 1x1", "Base.rte");
	superNode.Pos = Vector(SceneMan.SceneWidth * 0.5, SceneMan.SceneHeight * 0.5);
	superNode.Team = -1;
	superNode.Scale = 0;
	superNode:SetNumberValue("ZoneWidth", SceneMan.SceneWidth);
	superNode:SetNumberValue("ZoneHeight", SceneMan.SceneHeight);
	MovableMan:AddParticle(superNode);

	self:SetupHumanPlayerBrains(actorGroup, primaryGroup, secondaryGroup, tertiaryGroup);
end

function OneManArmyZeroG:SetupHumanPlayerBrains(actorGroup, primaryGroup, secondaryGroup, tertiaryGroup)
	--Default actors if no tech is chosen
	local defaultActor = self.isDiggersOnly and "Soldier Light" or "Soldier Heavy";
	local defaultPrimary = self.isDiggersOnly and "Ronin/SPAS 12" or "Coalition/Assault Rifle";
	local defaultSecondary = self.isDiggersOnly and "Ronin/.357 Magnum" or "Coalition/Auto Pistol";
	local defaultTertiary = self.isDiggersOnly and "Ronin/.357 Magnum" or "Coalition/Auto Pistol";

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
						local dataModule = PresetMan:GetDataModule(tech);
						local primaryWeapon, secondaryWeapon, tertiaryWeapon, actor;
						for entity in dataModule.Presets do
							local picked;	--Prevent duplicates
							if not primaryWeapon and entity.ClassName == "HDFirearm" and ToMOSRotating(entity):HasObjectInGroup(primaryGroup) and ToMOSRotating(entity).Buyable then
								primaryWeapon = CreateHDFirearm(entity:GetModuleAndPresetName());
								picked = true;
							end
							if not picked and not secondaryWeapon and entity.ClassName == "HDFirearm" and ToMOSRotating(entity):HasObjectInGroup(secondaryGroup) and ToMOSRotating(entity).Buyable then
								secondaryWeapon = CreateHDFirearm(entity:GetModuleAndPresetName());
								picked = true;
							end
							if not picked and not tertiaryWeapon and entity.ClassName == "HDFirearm" and ToMOSRotating(entity):HasObjectInGroup(tertiaryGroup) and ToMOSRotating(entity).Buyable then
								tertiaryWeapon = CreateHDFirearm(entity:GetModuleAndPresetName());
								picked = true;
							end
							if not picked and not actor and entity.ClassName == "AHuman" and ToMOSRotating(entity):HasObjectInGroup(actorGroup) and ToMOSRotating(entity).Buyable then
								actor = CreateAHuman(entity:GetModuleAndPresetName());
							end
						end
						if actor then
							foundBrain = actor;
						end
						local weapons = {primaryWeapon, secondaryWeapon, tertiaryWeapon};
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
						local weapons = {defaultPrimary, defaultSecondary, defaultTertiary};
						for i = 1, #weapons do
							local item = CreateHDFirearm(weapons[i]);
							if item then
								item.GibWoundLimit = item.GibWoundLimit and item.GibWoundLimit * brainToughnessMultiplier or item.GibWoundLimit;
								item.JointStrength = item.JointStrength * brainToughnessMultiplier;
								foundBrain:AddInventoryItem(CreateHDFirearm(weapons[i]));
							end
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
					local medikit = CreateHDFirearm("Base/Medikit");
					if medikit then
						foundBrain:AddInventoryItem(medikit);
					end
					--Reinforce FGArm so that we don't lose it
					--No FGArm = no weapons = no gameplay
					foundBrain.FGArm.GibWoundLimit = 999999;
					foundBrain.FGArm.JointStrength = 999999;

					foundBrain.Pos = Vector(SceneMan.SceneWidth * 0.5, SceneMan.SceneHeight * 0.5);
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

function OneManArmyZeroG:ResumeLoadedGame()
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
	
	for particle in MovableMan.AddedParticles do
		if particle.PresetName == "Automover Node 1x1" then
			particle.Scale = 0;
			break;
		end
	end
	
	for actor in MovableMan.AddedActors do
		if actor.Team ~= Activity.TEAM_1 then
			actor.AIMode = Actor.AIMODE_BRAINHUNT;
		end
	end
end

function OneManArmyZeroG:EndActivity()
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

function OneManArmyZeroG:UpdateActivity()
	if self.ActivityState ~= Activity.OVER then
		ActivityMan:GetActivity():SetTeamFunds(0, 0);
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

					self.WinnerTeam = player;

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
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self:PlayerActive(player) and self:PlayerHuman(player) then
				local playerBrain = self:GetPlayerBrain(player);
				if playerBrain ~= nil then
					if playerBrain.Pos.Y < -5 then
						playerBrain.Pos.Y = SceneMan.SceneHeight;
					elseif playerBrain.Pos.Y > SceneMan.SceneHeight + 5 then
						playerBrain.Pos.Y = 0;
					end
				end
			end
		end
		
		if self.CPUTeam ~= Activity.NOTEAM and self.enemySpawnTimer:LeftTillSimMS(self.enemySpawnTimeLimit) <= 0 then
			for i = 1, math.random(1, 3) do
				local actor = RandomAHuman("Actors - Light", self.CPUTechName);
				if actor.ModuleID ~= PresetMan:GetModuleID(self.CPUTechName) then
					actor = RandomAHuman("Actors", self.CPUTechName);
				end

				if IsAHuman(actor) then
					if self.isDiggersOnly then
						actor:AddInventoryItem(CreateHDFirearm("Light Digger", "Base.rte"));
					else
						actor:AddInventoryItem(RandomHDFirearm("Weapons - Light", self.CPUTechName));
						actor:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.CPUTechName));
					end
				end
				actor.AIMode = Actor.AIMODE_BRAINHUNT;
				actor.Team = self.CPUTeam;
				actor.Pos = Vector(math.random() * SceneMan.SceneWidth, -25);
				actor.Vel = Vector(math.random(-15, 15), math.random(0, 10));
				if math.random() < 0.5 then
					actor.Pos.Y = SceneMan.SceneHeight + 25;
					actor.Vel.Y = -10;
				end
				MovableMan:AddActor(actor);
			end

			self.enemySpawnTimer:Reset();
			self.enemySpawnTimeLimit = (self.baseSpawnTime + math.random(self.baseSpawnTime)) * rte.SpawnIntervalScale;
		end
	end
end