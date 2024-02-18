--[[

*** INSTRUCTIONS ***

This activity can be run on any scene with the "LZ Attacker" and "Main Bunker" areas. It also supports using the "LZ Defender" and "Brain" areas.
The attacking brain spawns in the "LZ Attacker" area and the defending brain in the "Brain" area, or via deployments.
If the defender is a CPU, the script will look for player units and send reinforcements to attack them.

When running the activity on scenes with randomized bunkers which have multiple brain chambers or other non-Brain Hideout deployments, only one brain at random chamber will be spawned.
To avoid wasting MOs for this actors you may define a "Brain Chamber" area. All actors inside "Brain Chamber" but without a brain nearby will be removed as useless.

Add intial defender units by placing areas named:
"Sniper Defenders", "Light Defenders", "Heavy Defenders", "Mecha Defenders", "Turret Defenders", "Engineer Defenders"
Add as many boxes as you want for each area, an appropriate defender will spawn in each of. Keep it reasonable though, unless you want the game to be unplayable.

Add internal reinforcement doors by placing areas named:
"Internal Reinforcements"
You should make sure the area's boxes fit properly in a hallway - specifically you need to position the top left-corner of each box such that a 48 x 48 pixel background door can be safely placed with its top-left corner there. Note that positions will be snapped, to keep this from being too arduous.
--]]

function BunkerBreach:SetupAIVariables()
	self.AI = {};
	self.AI.isAttackerTeam = self.CPUTeam == self.attackerTeam;
	self.AI.isDefenderTeam = self.CPUTeam == self.defenderTeam;

	if self.CPUTeam ~= Activity.NOTEAM then
		self.AI.difficultyRatio = self.Difficulty / Activity.MAXDIFFICULTY;
		self.AI.maxDiggerCount = math.floor(self.AI.difficultyRatio * 5);
		self.AI.spawnTimer = Timer();
		self:CalculateAISpawnDelay(self.AI.isAttackerTeam); -- Make the first attacker spawn show up extra quickly, to minimize dead time at the start of the activity.

		self:SetTeamFunds(4000 + ((8000 * math.floor(self.AI.difficultyRatio * (self.AI.isAttackerTeam and 8 or 4))) / 4), self.CPUTeam);

		if self.AI.isAttackerTeam then
			self.AI.maxCrabCount = 0;
			self.AI.majorAttackTimer = Timer();
			self.AI.initialMajorAttackDelay = self.Difficulty >= Activity.MEDIUMDIFFICULTY and (300000 - (self.AI.difficultyRatio * 200000)) or -1;  -- Only do major attacks if we're at or above medium difficulty.
			self.AI.majorAttackTimer:SetSimTimeLimitMS(self.AI.initialMajorAttackDelay * RangeRand(0.9, 1.1));
			self.AI.isLaunchingMajorAttack = false;
		else
			self.AI.maxCrabCount = math.floor(self.AI.difficultyRatio * 5);
			self.AI.internalReinforcementBudget = self:GetTeamFunds(self.CPUTeam) * self.AI.difficultyRatio;
			self.AI.internalReinforcementDoorsAndActorsToSpawn = {};
		end
	end
end

function BunkerBreach:SetupHumanAttackerBrains()
	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) and self:GetTeamOfPlayer(player) == self.attackerTeam then
			local attackingBrain = self:CreateBrainBot(self:GetTeamOfPlayer(player));

			local brainPosition = Vector(self:GetLZArea(self.attackerTeam):GetRandomPoint().X, 0);
			if SceneMan.SceneWrapsX then
				if brainPosition.X < 0 then
					brainPosition.X = brainPosition.X + SceneMan.SceneWidth;
				elseif brainPosition.X >= SceneMan.SceneWidth then
					brainPosition.X = brainPosition.X - SceneMan.SceneWidth;
				end
			else
				brainPosition.X = math.max(math.min(brainPosition.X, SceneMan.SceneWidth - 50), 50);
			end

			attackingBrain.Pos = SceneMan:MovePointToGround(brainPosition, attackingBrain.Radius * 0.5, 3);
			MovableMan:AddActor(attackingBrain);

			self:SwitchToActor(attackingBrain, player, self.attackerTeam);
			self:SetPlayerBrain(attackingBrain, player);
			self:SetObservationTarget(attackingBrain.Pos, player);
			self:SetLandingZone(attackingBrain.Pos, player);
		end
	end
end

function BunkerBreach:SetupDefenderBrains()
	local defenderBrain;

	-- Add defender brains, either using the Brain area or picking randomly from those created by deployments.
	if SceneMan.Scene:HasArea("Brain") then
		for actor in MovableMan.AddedActors do
			if actor:IsInGroup("Brains") then
				actor.ToDelete = true;
			end
		end

		defenderBrain = self:CreateBrainBot(self.defenderTeam);
		defenderBrain.Pos = SceneMan.Scene:GetOptionalArea("Brain"):GetCenterPoint();
		MovableMan:AddActor(defenderBrain);
	else
		-- Pick the defender brain randomly from among those created by deployments, then delete the others and clean up most of their guards.
		local deploymentBrains = {};
		for actor in MovableMan.AddedActors do
			if actor.Team == self.defenderTeam and actor:IsInGroup("Brains") then
				deploymentBrains[#deploymentBrains + 1] = actor;
			end
		end
		local brainIndexToChoose = math.random(1, #deploymentBrains);
		defenderBrain = deploymentBrains[brainIndexToChoose];
		table.remove(deploymentBrains, brainIndexToChoose);

		if SceneMan.Scene:HasArea("Brain Chamber") then
			self.brainChamber = SceneMan.Scene:GetOptionalArea("Brain Chamber");
		end
		for _, unchosenDeploymentBrain in pairs(deploymentBrains) do
			unchosenDeploymentBrain.ToDelete = true;
			for actor in MovableMan.AddedActors do
				if actor.Team == self.defenderTeam and math.random() < 0.75 and self.brainChamber:IsInside(actor.Pos) and (actor.ClassName == "AHuman" or actor.ClassName == "ACrab") and SceneMan:ShortestDistance(actor.Pos, defenderBrain.Pos, false):MagnitudeIsGreaterThan(200) then
					actor.ToDelete = true;
				end
			end
		end
	end

	-- Make sure all defending human players have brains.
	if not self.AI.isDefenderTeam then
		local playerDefenderBrainsAssignedCount = 0;
		local brainToAssignToPlayer;
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self:PlayerActive(player) and self:PlayerHuman(player) and self:GetTeamOfPlayer(player) == self.defenderTeam then
				if playerDefenderBrainsAssignedCount == 0 then
					brainToAssignToPlayer = defenderBrain;
				else
					brainToAssignToPlayer = self:CreateBrainBot(self.defenderTeam);
					brainToAssignToPlayer.Pos = defenderBrain.Pos + Vector(playerDefenderBrainsAssignedCount * 10 * defenderBrain.FlipFactor, 0);
					MovableMan:AddActor(brainToAssignToPlayer);
				end
				playerDefenderBrainsAssignedCount = playerDefenderBrainsAssignedCount + 1;

				self:SwitchToActor(brainToAssignToPlayer, player, self.defenderTeam);
				self:SetPlayerBrain(brainToAssignToPlayer, player);
				self:SetObservationTarget(brainToAssignToPlayer.Pos, player);
				self:SetLandingZone(brainToAssignToPlayer.Pos, player);
			end
		end
	elseif self.AI.isDefenderTeam then
		self.AI.brain = defenderBrain;
	end
end

function BunkerBreach:SetupDefenderActors()
	local techID = PresetMan:GetModuleID(self:GetTeamTech(self.defenderTeam));
	local crabToHumanSpawnRatio = self:GetCrabToHumanSpawnRatio(techID);

	local loadoutNames = {"Light", "Heavy", "Sniper", "Engineer", "Mecha", "Turret"};
	
	local hasSpawnAreas = false;
	for _, loadoutName in pairs(loadoutNames) do
		if SceneMan.Scene:HasArea(loadoutName .. " Defenders") then
			hasSpawnAreas = true;
		end
	end
	
	for actor in MovableMan.AddedActors do
		if not actor:IsInGroup("Brains") and not actor:IsInGroup("Bunker Systems") then
			if hasSpawnAreas then
				actor.ToDelete = true;
			elseif actor.Team ~= self.defenderTeam then
				MovableMan:ChangeActorTeam(actor, self.defenderTeam);
			end
		elseif IsADoor(actor) then
			-- Give every door to the defender team
			actor.Team = self.defenderTeam;
		end
	end
	
	if hasSpawnAreas then
		for _, loadoutName in pairs({"Light", "Heavy", "Sniper", "Engineer", "Mecha", "Turret"}) do
			if SceneMan.Scene:HasArea(loadoutName .. " Defenders") then
				hasSpawnAreas = true;
				local defenderArea = SceneMan.Scene:GetOptionalArea(loadoutName .. " Defenders");
				if defenderArea ~= nil then
					for defenderBox in defenderArea.Boxes do
						local guard;
						if loadoutName == "Mecha" or loadoutName == "Turret" then
							guard = crabToHumanSpawnRatio > 0 and self:CreateCrab(techID, loadoutName == "Turret") or self:CreateInfantry(techID, "Heavy");
						else
							guard = self:CreateInfantry(techID, loadoutName);
						end
						if guard then
							guard.Pos = defenderBox.Center;
							guard.Team = self.defenderTeam;
							guard.AIMode = Actor.AIMODE_SENTRY;
							if loadoutName == "Engineer" then
								guard.AIMode = Actor.AIMODE_GOLDDIG;
							end
							MovableMan:AddActor(guard);
						end
					end
				end
			end
		end
	end
end

function BunkerBreach:SetupFogOfWar()
	if self:GetFogOfWarEnabled() then
		SceneMan:MakeAllUnseen(Vector(20, 20), self.attackerTeam);
		SceneMan:MakeAllUnseen(Vector(20, 20), self.defenderTeam);

		-- Reveal outside areas for the attacker.
		for x = 0, SceneMan.SceneWidth - 1, 20 do
			SceneMan:CastSeeRay(self.attackerTeam, Vector(x, 0), Vector(0, SceneMan.SceneHeight), Vector(), 1, 9);
		end

		-- Reveal the main bunker area for the defender.
		for mainBunkerBox in self.mainBunkerArea.Boxes do
			SceneMan:RevealUnseenBox(mainBunkerBox.Corner.X, mainBunkerBox.Corner.Y, mainBunkerBox.Width, mainBunkerBox.Height, self.defenderTeam);
		end

		-- Reveal a circle around actors, so they're not standing in the dark.
		for actor in MovableMan.AddedActors do
			for angle = 0, math.pi * 2, 0.05 do
				SceneMan:CastSeeRay(actor.Team, actor.EyePos, Vector(150 + FrameMan.PlayerScreenWidth * 0.5, 0):RadRotate(angle), Vector(), 1, 4);
			end
		end
	end
end

function BunkerBreach:SetupDefenderInternalReinforcementAreas()
	if self.AI.isDefenderTeam then
		local internalReinforcementsArea = SceneMan.Scene:GetOptionalArea("Internal Reinforcements");
		if internalReinforcementsArea ~= nil then
			self.AI.internalReinforcementsDoorParticle = CreateMOSRotating("Background Door", "Base.rte");
			self.AI.internalReinforcementPositions = {};
			for internalReinforcementsBox in internalReinforcementsArea.Boxes do
				local backgroundDoor = CreateTerrainObject("Module Back Middle E", "Base.rte");
				backgroundDoor.Pos = SceneMan:SnapPosition(internalReinforcementsBox.Corner, true);
				self.AI.internalReinforcementPositions[#self.AI.internalReinforcementPositions + 1] = backgroundDoor.Pos + Vector(24, 24);
				SceneMan:AddSceneObject(backgroundDoor);
			end
		end
	end
end

function BunkerBreach:StartActivity(isNewGame)
	self.attackerTeam = Activity.TEAM_1;
	self.defenderTeam = Activity.TEAM_2;

	-- Setup LZ and main bunker areas, and also filter out any scenes without the "LZ Attacker" area from being usable in this Activity.
	local attackerLZ = SceneMan.Scene:GetArea("LZ Attacker");
	self:SetLZArea(self.attackerTeam, attackerLZ);
	if SceneMan.Scene:HasArea("LZ Defender") then
		self:SetLZArea(self.defenderTeam, SceneMan.Scene:GetOptionalArea("LZ Defender"));
	end
	self.mainBunkerArea = SceneMan.Scene:GetArea("Main Bunker");

	self.winConditionCheckTimer = Timer();
	self.winConditionCheckTimer:SetSimTimeLimitMS(1000);

	-- Get rid of the wonky default scene launch text.
	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) then
			FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
		end
	end

	if isNewGame then
		-- Because of game oddities, we need to set funds to match starting gold manually.
		self:SetTeamFunds(self:GetStartingGold(), self.defenderTeam);
		self:SetTeamFunds(self:GetStartingGold(), self.attackerTeam);

		self:SetupAIVariables();

		self:SetupDefenderBrains();

		self:SetupDefenderActors();

		self:SetupHumanAttackerBrains();

		self:SetupFogOfWar();
	else
		self:ResumeLoadedGame();
	end
	self:SetupDefenderInternalReinforcementAreas();
end

function BunkerBreach:OnSave()
	self:SaveNumber("AI.isAttackerTeam", self.AI.isAttackerTeam and 1 or 0);
	self:SaveNumber("AI.isDefenderTeam", self.AI.isDefenderTeam and 1 or 0);

	self:SaveNumber("AI.difficultyRatio", self.AI.difficultyRatio);
	self:SaveNumber("AI.maxDiggerCount", self.AI.maxDiggerCount);
	self:SaveNumber("AI.spawnTimer.ElapsedSimTimeMS", self.AI.spawnTimer.ElapsedSimTimeMS);
	self:SaveNumber("AI.spawnTimer.SimTimeLimitMS", self.AI.spawnTimer:GetSimTimeLimitMS());

	self:SaveNumber("AI.maxCrabCount", self.AI.maxCrabCount);
	self:SaveNumber("AI.majorAttackTimer.ElapsedSimTimeMS", self.AI.spawnTimer.ElapsedSimTimeMS);
	self:SaveNumber("AI.majorAttackTimer.SimTimeLimitMS", self.AI.spawnTimer:GetSimTimeLimitMS());
	self:SaveNumber("AI.initialMajorAttackDelay", self.AI.initialMajorAttackDelay or 0);
	self:SaveNumber("AI.isLaunchingMajorAttack", self.AI.isLaunchingMajorAttack and 1 or 0);

	self:SaveNumber("AI.internalReinforcementBudget", self.AI.internalReinforcementBudget or 0);

	-- If any internal reinforcements are queued to spawn, we can't save them, so spawn them right away.
	if self.AI.isDefenderTeam then
		self:UpdateInternalReinforcementSpawning(true);
	end
end

function BunkerBreach:ResumeLoadedGame()
	self.AI = {};

	self.AI.isAttackerTeam = self:LoadNumber("AI.isAttackerTeam") ~= 0;
	self.AI.isDefenderTeam = self:LoadNumber("AI.isDefenderTeam") ~= 0;

	if self.CPUTeam ~= Activity.NOTEAM then
		self.AI.difficultyRatio = self:LoadNumber("AI.difficultyRatio");
		self.AI.maxDiggerCount = self:LoadNumber("AI.maxDiggerCount");

		self.AI.spawnTimer = Timer();
		self.AI.spawnTimer.ElapsedSimTimeMS = self:LoadNumber("AI.spawnTimer.ElapsedSimTimeMS");
		self.AI.spawnTimer:SetSimTimeLimitMS(self:LoadNumber("AI.spawnTimer.SimTimeLimitMS"));

		self.AI.maxCrabCount = self:LoadNumber("AI.maxCrabCount");
		self.AI.majorAttackTimer = Timer();
		self.AI.majorAttackTimer.ElapsedSimTimeMS = self:LoadNumber("AI.majorAttackTimer.ElapsedSimTimeMS");
		self.AI.majorAttackTimer:SetSimTimeLimitMS(self:LoadNumber("AI.majorAttackTimer.SimTimeLimitMS"));
		self.AI.initialMajorAttackDelay = self:LoadNumber("AI.initialMajorAttackDelay");
		self.AI.isLaunchingMajorAttack = self:LoadNumber("AI.isLaunchingMajorAttack") ~= 0

		self.AI.internalReinforcementBudget = self:LoadNumber("AI.internalReinforcementBudget") ~= 0;
	end
end

function BunkerBreach:EndActivity()
	-- Temp fix so music doesn't start playing if ending the Activity when changing resolution through the ingame settings.
	if not self:IsPaused() then
		if self.WinnerTeam == self.CPUTeam then
			AudioMan:ClearMusicQueue();
			AudioMan:PlayMusic("Base.rte/Music/dBSoundworks/udiedfinal.ogg", 2, -1.0);
			AudioMan:QueueSilence(10);
			AudioMan:QueueMusicStream("Base.rte/Music/dBSoundworks/ccambient4.ogg");
		else
			AudioMan:ClearMusicQueue();
			AudioMan:PlayMusic("Base.rte/Music/dBSoundworks/uwinfinal.ogg", 2, -1.0);
			AudioMan:QueueSilence(10);
			AudioMan:QueueMusicStream("Base.rte/Music/dBSoundworks/ccambient4.ogg");
		end
	end
end

function BunkerBreach:CheckWinConditions()
	if not MovableMan:GetFirstBrainActor(self.defenderTeam) then
		self.WinnerTeam = self.attackerTeam;
	elseif self.AI.isAttackerTeam then
		if self.AI.funds <= 0 then
			local firstAITeamActor = MovableMan:GetNextTeamActor(self.attackerTeam, nil);
			local nextAITeamActor = firstAITeamActor ~= nil and MovableMan:GetNextTeamActor(self.attackerTeam, firstAITeamActor) or nil;
			if not nextAITeamActor and (not firstAITeamActor or (firstAITeamActor.Health / firstAITeamActor.MaxHealth) < 0.5) then
				self.WinnerTeam = self.defenderTeam;
			end
		end
	elseif not MovableMan:GetFirstBrainActor(self.attackerTeam) then
		self.WinnerTeam = self.defenderTeam;
	end
end

function BunkerBreach:UpdatePlayerObjectiveArrowsAndScreenText()
	if self.AI.isAttackerTeam then
		if self.AI.funds <= 0 then
			self:ClearObjectivePoints();
			local objectiveArrowsShown = 0;
			for _, friendlyUnitTable in ipairs({self.AI.friendlyUnitsInsideBunker, self.AI.friendlyUnitsOutsideBunker}) do
				for _, friendlyUnit in pairs(friendlyUnitTable) do
					if MovableMan:IsActor(friendlyUnit) then
						self:AddObjectivePoint("Destroy!", friendlyUnit.AboveHUDPos, self.defenderTeam, GameActivity.ARROWDOWN);
					end
					objectiveArrowsShown = objectiveArrowsShown + 1;
					if objectiveArrowsShown >= 10 then
						break;
					end
				end
			end
			self:YSortObjectivePoints();
		else
			local screenText = "Remaining Enemy Budget: " .. math.floor(self.AI.funds) .. " oz";
			if self.AI.isLaunchingMajorAttack then
				screenText = screenText .. "\n ALERT: The Enemy is Launching a Major Offensive!"
			end
			for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
				if self:PlayerActive(player) and self:PlayerHuman(player) then
					FrameMan:SetScreenText(screenText, self:ScreenOfPlayer(player), 0, 2500, false);
				end
			end
		end
	elseif self.AI.isDefenderTeam and self.AI.enemyHumanIsRamboing then
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self:PlayerActive(player) and self:PlayerHuman(player) then
				FrameMan:SetScreenText("ALERT: Enemy Alarms Have Been Triggered!", self:ScreenOfPlayer(player), 0, 2500, false);
			end
		end
	end
end

function BunkerBreach:UpdateInternalReinforcementSpawning(forceInstantSpawning)
	for internalReinforcementDoor, actorsToSpawn in pairs(self.AI.internalReinforcementDoorsAndActorsToSpawn) do
		if MovableMan:IsParticle(internalReinforcementDoor) and (forceInstantSpawning or internalReinforcementDoor.Frame == internalReinforcementDoor.FrameCount - 1) then
			for _, actorToSpawn in pairs(actorsToSpawn) do
				actorToSpawn.Team = internalReinforcementDoor.Team;
				MovableMan:AddActor(actorToSpawn);
			end
			self.AI.internalReinforcementDoorsAndActorsToSpawn[internalReinforcementDoor] = nil;
		end
	end
end

function BunkerBreach:UpdateAIDecisionData()
	self.AI.friendlyUnitsInsideBunker = {};
	self.AI.friendlyUnitsOutsideBunker = {};
	self.AI.crabCount = 0;
	self.AI.breacherUnits = {};
	self.AI.diggerCount = 0;
	self.AI.enemyUnitsInsideBunker = {};
	self.AI.enemyUnitsOutsideBunker = {};
	self.AI.enemyHumanIsRamboing = false;

	local friendlyUnitTotalValue = 0;
	local enemyUnitInsideBunkerTotalValue = 0;
	local enemyUnitOutsideBunkerTotalValue = 0;
	for actor in MovableMan.Actors do
		if actor.ClassName == "AHuman" or actor.ClassName == "ACrab" then
			if actor.Team == self.CPUTeam then
				if self.mainBunkerArea:IsInside(actor.Pos) then
					self.AI.friendlyUnitsInsideBunker[#self.AI.friendlyUnitsInsideBunker + 1] = actor;
				else
					self.AI.friendlyUnitsOutsideBunker[#self.AI.friendlyUnitsOutsideBunker + 1] = actor;
				end
				friendlyUnitTotalValue = friendlyUnitTotalValue + actor:GetTotalValue(actor.ModuleID, 1);

				if actor.ClassName == "ACrab" then
					self.AI.crabCount = self.AI.crabCount + 1;
				elseif actor:HasObjectInGroup("Tools - Breaching") then
					self.AI.breacherUnits[#self.AI.breacherUnits + 1] = actor;
				elseif actor:HasObjectInGroup("Tools - Diggers") and actor.AIMode == Actor.AIMODE_GOLDDIG then
					self.AI.diggerCount = self.AI.diggerCount + 1;
				end
			else
				if self.mainBunkerArea:IsInside(actor.Pos) then
					self.AI.enemyUnitsInsideBunker[#self.AI.enemyUnitsInsideBunker + 1] = actor;
					enemyUnitInsideBunkerTotalValue = enemyUnitInsideBunkerTotalValue + actor:GetTotalValue(actor.ModuleID, 1);
					if actor:IsInGroup("Brains") and actor:IsPlayerControlled() then
						self.AI.enemyHumanIsRamboing = true;
					end
				else
					self.AI.enemyUnitsOutsideBunker[#self.AI.enemyUnitsOutsideBunker + 1] = actor;
					enemyUnitOutsideBunkerTotalValue = enemyUnitOutsideBunkerTotalValue + actor:GetTotalValue(actor.ModuleID, 1);
				end
			end
		end
	end
	self.AI.friendlyUnitsToEnemyUnitsValueRatio = friendlyUnitTotalValue / math.max(enemyUnitInsideBunkerTotalValue + enemyUnitOutsideBunkerTotalValue, 1);
	self.AI.enemyUnitsInsideToOutsideValueRatio = enemyUnitInsideBunkerTotalValue / math.max(enemyUnitOutsideBunkerTotalValue, 1);
	if self.AI.isDefenderTeam and #self.AI.enemyUnitsInsideBunker <= (2 * self.HumanCount) then
		for _, enemyUnitInsideBunker in pairs(self.AI.enemyUnitsInsideBunker) do
			if enemyUnitInsideBunker:IsPlayerControlled() then
				self.AI.enemyHumanIsRamboing = true;
				break;
			end
		end
	end

	if self.AI.shouldSpawnDiggers then
		self.AI.shouldSpawnDiggers = false;
	elseif self.AI.diggerCount < self.AI.maxDiggerCount and (self.AI.funds < 500 or math.random() < 0.1) then
		self.AI.shouldSpawnDiggers = true;
	end

	if self.AI.isAttackerTeam and self.AI.funds > 0 and self.AI.majorAttackTimer:IsPastSimTimeLimit() then
		self.AI.isLaunchingMajorAttack = not self.AI.isLaunchingMajorAttack;
		if self.AI.isLaunchingMajorAttack then
			self.AI.majorAttackTimer:SetSimTimeLimitMS(self.AI.spawnTimer:GetSimTimeLimitMS() * math.random(0.75, 1.5));
		else
			self.AI.majorAttackTimer:SetSimTimeLimitMS(self.AI.initialMajorAttackDelay * RangeRand(0.9, 1.1))
		end
		self.AI.majorAttackTimer:Reset();
	end
end

function BunkerBreach:CalculateAISpawnDelay(useShorterDelay)
	if self.AI.friendlyUnitsToEnemyUnitsValueRatio == nil then
		self.AI.friendlyUnitsToEnemyUnitsValueRatio = 0;
	end

	local spawnDelayMultiplier = 1;
	if useShorterDelay then
		spawnDelayMultiplier = spawnDelayMultiplier - 0.25
	end

	self.AI.spawnTimer:SetSimTimeLimitMS(math.min(150000, math.floor((40000 - (self.AI.difficultyRatio * 20000) + (self.AI.friendlyUnitsToEnemyUnitsValueRatio * (self.AI.isDefenderTeam and 7500 or 10000)) + (self.AI.isAttackerTeam and 10000 or 0)) * rte.SpawnIntervalScale * spawnDelayMultiplier)));
end

function BunkerBreach:SendDefenderGuardsAtEnemiesInsideBunker()
	local maxGuardsToSend = math.ceil(math.random(1, self.AI.difficultyRatio * 5));

	local sentGuardCount = 0;
	for _, enemyUnitInsideBunker in pairs(self.AI.enemyUnitsInsideBunker) do
		if MovableMan:IsActor(enemyUnitInsideBunker) and sentGuardCount < maxGuardsToSend then
			local closestFriendlyUnitData = {};
			for _, friendlyUnitInsideBunker in pairs(self.AI.friendlyUnitsInsideBunker) do
				if not friendlyUnitInsideBunker:IsInGroup("Brains") then
					local pathLengthFromFriendlyUnitToEnemy = SceneMan.Scene:CalculatePath(friendlyUnitInsideBunker.Pos, enemyUnitInsideBunker.Pos, false, GetPathFindingDefaultDigStrength(), self.CPUTeam);
					if closestFriendlyUnitData.pathLengthToEnemy == nil or pathLengthFromFriendlyUnitToEnemy < closestFriendlyUnitData.pathLengthToEnemy then
						closestFriendlyUnitData.pathLengthToEnemy = pathLengthFromFriendlyUnitToEnemy;
						closestFriendlyUnitData.actor = friendlyUnitInsideBunker;

						if closestFriendlyUnitData.pathLengthToEnemy < 5 then
							break;
						end
					end
				end
			end

			if closestFriendlyUnitData.actor and closestFriendlyUnitData.actor.AIMode ~= Actor.AIMODE_GOLDDIG and (closestFriendlyUnitData.actor.AIMode ~= Actor.AIMODE_SENTRY or math.random() < 0.1) then
				closestFriendlyUnitData.actor:AddAIMOWaypoint(enemyUnitInsideBunker);
				closestFriendlyUnitData.actor.AIMode = Actor.AIMODE_GOTO;
				sentGuardCount = sentGuardCount + 1;
			end
		end
	end
end

function BunkerBreach:UpdateAISpawns()
	if self.AI.isAttackerTeam and self.AI.funds > 0 then
		if self.AI.shouldSpawnDiggers then
			self:CreateDrop("Engineer", Actor.AIMODE_GOLDDIG, self.AI.maxDiggerCount - self.AI.diggerCount);
		elseif self.AI.isLaunchingMajorAttack or self.AI.friendlyUnitsToEnemyUnitsValueRatio < (3 * math.max(self.AI.difficultyRatio, 0.1)) then
			self:CreateDrop("Any", Actor.AIMODE_BRAINHUNT, self.AI.isLaunchingMajorAttack and 999 or nil, self.AI.isLaunchingMajorAttack);

			if self.AI.isLaunchingMajorAttack and math.random() < (self.AI.difficultyRatio * 0.9) then
				self:CreateDrop("Any", Actor.AIMODE_BRAINHUNT, 999, true);
			end

			self:CalculateAISpawnDelay(self.AI.isLaunchingMajorAttack);
		else
			self.AI.spawnTimer:SetSimTimeLimitMS(self.AI.spawnTimer:GetSimTimeLimitMS() * 0.9);
		end
	else
		if self.AI.difficultyRatio > 0 and (self.AI.enemyHumanIsRamboing or self.AI.enemyUnitsInsideToOutsideValueRatio > RangeRand(0.25, 1.5)) and math.random() < 0.75 then
			self:SendDefenderGuardsAtEnemiesInsideBunker();

			local numberOfInternalReinforcementsToSpawn = 0;
			if self.AI.internalReinforcementBudget > 0 then
				if self.AI.enemyHumanIsRamboing then
					numberOfInternalReinforcementsToSpawn = 1 + math.ceil(4 * self.AI.difficultyRatio * math.random());
				else
					numberOfInternalReinforcementsToSpawn = math.ceil(math.min(5 * self.AI.difficultyRatio, self.AI.enemyUnitsInsideToOutsideValueRatio * self.AI.difficultyRatio * 2));
				end
			end
			if #self.AI.internalReinforcementPositions > 0 then
				self:CreateInternalReinforcements("Any", numberOfInternalReinforcementsToSpawn);
			elseif self.AI.funds > 0 then
				self:CreateDrop("Any", Actor.AIMODE_BRAINHUNT, 999);
			end
			self:CalculateAISpawnDelay(true);
		elseif self.AI.funds > 0 then
			if self.AI.shouldSpawnDiggers then
				self:CreateDrop("Engineer", Actor.AIMODE_GOLDDIG, self.AI.maxDiggerCount - self.AI.diggerCount);
				self:CalculateAISpawnDelay(true);
			else
				self:CreateDrop("Any", Actor.AIMODE_BRAINHUNT);
				self:CalculateAISpawnDelay();
			end
		end
	end
end

function BunkerBreach:UpdateActivity()
	if self.ActivityState == Activity.OVER then
		return;
	end

	if self.winConditionCheckTimer:IsPastSimTimeLimit() then
		self:CheckWinConditions();
		if self.WinnerTeam ~= -1 then
			for actor in MovableMan.Actors do
				if actor.Team ~= self.WinnerTeam then
					local randomResult = math.random();
					if randomResult < 0.33 then
						actor:GibThis();
					elseif randomResult < 0.66 then
						actor.Health = 0;
					else
						actor.Status = Actor.INACTIVE;
					end
				end
			end
			ActivityMan:EndActivity();
			return;
		end
		self.winConditionCheckTimer:Reset();
	end

	if self.CPUTeam ~= -1 then
		self.AI.funds = self:GetTeamFunds(self.CPUTeam);

		self:UpdatePlayerObjectiveArrowsAndScreenText();

		if self.AI.internalReinforcementDoorsAndActorsToSpawn then
			self:UpdateInternalReinforcementSpawning();
		end

		if self.AI.spawnTimer:IsPastSimTimeLimit() then
			self.AI.spawnTimer:Reset();

			self:UpdateAIDecisionData();

			if self.AI.funds > 0  or (self.AI.isDefenderTeam and self.AI.internalReinforcementBudget > 0) then
				self:UpdateAISpawns();
			else
				self.AI.spawnTimer:SetSimTimeLimitMS(5000);
			end
		end
	end
end

function BunkerBreach:CalculateInternalReinforcementPositionsToEnemyTargets(numberOfReinforcementsToCreate)
	local enemiesToTarget = {};
	for i = 1, numberOfReinforcementsToCreate do
		if enemiesToTarget[i] == nil then
			enemiesToTarget[i] = self.AI.enemyUnitsInsideBunker[math.random(1, #self.AI.enemyUnitsInsideBunker)];
		end
	end

	local internalReinforcementPositionsToEnemyTargets = {};
	for _, enemyToTarget in ipairs(enemiesToTarget) do
		local internalReinforcementPositionForEnemy;
		local pathLengthFromClosestInternalReinforcementPositionToEnemy = SceneMan.SceneWidth * SceneMan.SceneHeight;
		for _, internalReinforcementPosition in pairs(self.AI.internalReinforcementPositions) do
			local pathLengthFromInternalReinforcementPositionToEnemy = SceneMan.Scene:CalculatePath(internalReinforcementPosition, enemyToTarget.Pos, false, GetPathFindingDefaultDigStrength(), self.CPUTeam);
			if pathLengthFromInternalReinforcementPositionToEnemy < pathLengthFromClosestInternalReinforcementPositionToEnemy then
				internalReinforcementPositionForEnemy = internalReinforcementPosition;
				pathLengthFromClosestInternalReinforcementPositionToEnemy = pathLengthFromInternalReinforcementPositionToEnemy;
			end
		end
		if internalReinforcementPositionForEnemy then
			if not internalReinforcementPositionsToEnemyTargets[internalReinforcementPositionForEnemy] then
				internalReinforcementPositionsToEnemyTargets[internalReinforcementPositionForEnemy] = {};
			end
			table.insert(internalReinforcementPositionsToEnemyTargets[internalReinforcementPositionForEnemy], enemyToTarget);
		end
	end

	return internalReinforcementPositionsToEnemyTargets;
end

function BunkerBreach:CreateInternalReinforcements(loadout, numberOfReinforcementsToCreate)
	if loadout == "Any" then
		loadout = nil;
	end
	local team = self.CPUTeam;
	local techID = PresetMan:GetModuleID(self:GetTeamTech(team));
	local crabToHumanSpawnRatio = self:GetCrabToHumanSpawnRatio(techID);

	local internalReinforcementPositionsToEnemyTargets = self:CalculateInternalReinforcementPositionsToEnemyTargets(numberOfReinforcementsToCreate);

	local numberOfReinforcementsCreated = 0;
	for internalReinforcementPosition, enemyTargetsForPosition in pairs(internalReinforcementPositionsToEnemyTargets) do
		if numberOfReinforcementsCreated < numberOfReinforcementsToCreate and self.AI.internalReinforcementBudget > 0 and #enemyTargetsForPosition > 0 then
			local doorParticle = self.AI.internalReinforcementsDoorParticle:Clone();
			doorParticle.Pos = internalReinforcementPosition;
			doorParticle.Team = team;
			MovableMan:AddParticle(doorParticle);
			self.AI.internalReinforcementDoorsAndActorsToSpawn[doorParticle] = {};

			local numberOfInternalReinforcementsToCreateAtPosition = math.min(#enemyTargetsForPosition, 3);
			if numberOfInternalReinforcementsToCreateAtPosition == 1 and math.random() < (self.AI.difficultyRatio * 0.75) then
				numberOfInternalReinforcementsToCreateAtPosition = 2;
				if math.random() < (self.AI.difficultyRatio * 0.5) then
					numberOfInternalReinforcementsToCreateAtPosition = 3;
				end
			end
			
			for i = 1, numberOfInternalReinforcementsToCreateAtPosition do
				local internalReinforcement;
				if loadout then
					internalReinforcement = self:CreateInfantry(techID, loadout);
				elseif math.random() < crabToHumanSpawnRatio and self.AI.crabCount < self.AI.maxCrabCount then
					local createTurretReinforcement = math.random() < 0.05;
					internalReinforcement = self:CreateCrab(techID, createTurretReinforcement);
				else
					internalReinforcement = self:CreateInfantry(techID);
				end
				internalReinforcement.Team = team;
				internalReinforcement.Pos = internalReinforcementPosition;
				if numberOfInternalReinforcementsToCreateAtPosition == 2 then
					internalReinforcement.Pos.X = internalReinforcement.Pos.X + (i == 1 and -10 or 10);
				elseif numberOfInternalReinforcementsToCreateAtPosition == 3 then
					internalReinforcement.Pos.X = internalReinforcement.Pos.X - 30 + (i * 15);
				end
				if internalReinforcement:IsInGroup("Actors - Turrets") then
					internalReinforcement.AIMode = Actor.AIMODE_SENTRY;
				else
					internalReinforcement.AIMode = Actor.AIMODE_GOTO;
					internalReinforcement:AddAIMOWaypoint(enemyTargetsForPosition[math.random(#enemyTargetsForPosition)]);
				end
				table.insert(self.AI.internalReinforcementDoorsAndActorsToSpawn[doorParticle], internalReinforcement);

				self.AI.internalReinforcementBudget = self.AI.internalReinforcementBudget - internalReinforcement:GetTotalValue(techID, 2);
				numberOfReinforcementsCreated = numberOfReinforcementsCreated + 1;
				if numberOfReinforcementsCreated >= numberOfReinforcementsToCreate or self.AI.internalReinforcementBudget <= 0 then
					break;
				end
			end
		end
	end
end

function BunkerBreach:CreateDrop(loadout, aiMode, passengerCount, avoidPreviousCraftPos)
	if loadout == "Any" then
		loadout = nil;
	end
	local team = self.CPUTeam;
	local techID = PresetMan:GetModuleID(self:GetTeamTech(team));
	local crabToHumanSpawnRatio = self:GetCrabToHumanSpawnRatio(techID);

	local craft = RandomACDropShip("Craft", techID);
	if not craft or craft.MaxInventoryMass <= 0 then
		craft = RandomACDropShip("Craft", "Base.rte");
	end
	craft.Team = team;

	craft.Pos = Vector(0, -30);
	local landingZoneArea = self:GetLZArea(team);
	if landingZoneArea then
		craft.Pos.X = landingZoneArea:GetRandomPoint().X;
	elseif self.AI.isDefenderTeam and self.AI.brain then
		craft.Pos.X = math.max(math.min(self.AI.brain.Pos.X + math.random(-100, 100), SceneMan.SceneWidth - 100), 100);
	else
		craft.Pos.X = math.random(100, SceneMan.SceneWidth - 100);
	end
	local craftSpriteWidth = craft:GetSpriteWidth();
	if avoidPreviousCraftPos and self.AI.previousCraftLZInfo then
		local spaceToLeaveBetweenCrafts = (craftSpriteWidth + self.AI.previousCraftLZInfo.craftSpriteWidth) * 1.2;
		if math.abs(SceneMan:ShortestDistance(craft.Pos, Vector(self.AI.previousCraftLZInfo.posX, 0), SceneMan.SceneWrapsX).X) < spaceToLeaveBetweenCrafts then
			craft.Pos.X = self.AI.previousCraftLZInfo.posX + spaceToLeaveBetweenCrafts;
			if craft.Pos.X >= (SceneMan.SceneWidth - craftSpriteWidth) then
				craft.Pos.X = self.AI.previousCraftLZInfo - spaceToLeaveBetweenCrafts;
			end
		end
	end
	self.AI.previousCraftLZInfo = {posX = craft.Pos.X, craftSpriteWidth = craftSpriteWidth};

	if passengerCount == nil then
		passengerCount = math.random(math.ceil(craft.MaxPassengers * 0.5), craft.MaxPassengers);
	end
	passengerCount = math.min(passengerCount, craft.MaxPassengers);
	for i = 1, passengerCount do
		local actor;
		if loadout then
			passenger = self:CreateInfantry(techID, loadout);
		elseif math.random() < crabToHumanSpawnRatio and self.AI.crabCount < self.AI.maxCrabCount then
			passenger = self:CreateCrab(techID);
		else
			passenger = self:CreateInfantry(techID);
		end

		if passenger then
			passenger.Team = team;
			if aiMode then
				passenger.AIMode = aiMode;
			else
				passenger.AIMode = Actor.AIMODE_BRAINHUNT;
			end
			if passenger:IsInGroup("Actors - Turrets") then
				passenger.AIMode = Actor.AIMODE_SENTRY;
			elseif IsACrab(passenger) and passenger.AIMode == Actor.AIMODE_GOLDDIG then
				passenger.AIMode = Actor.Actor.AIMODE_PATROL;
			end
			craft:AddInventoryItem(passenger);

			if craft.InventoryMass > craft.MaxInventoryMass then
				break;
			end
		end
	end
	self:ChangeTeamFunds(-craft:GetTotalValue(techID, 2), team);
	MovableMan:AddActor(craft);
end

function BunkerBreach:CreateInfantry(techID, loadout)
	if loadout == nil then
		local infantryLoadouts = {"Light", "Heavy", "Sniper"};
		loadout = infantryLoadouts[math.random(#infantryLoadouts)];
	end

	local loadoutToGroupNameTable = {
		Light = "Actors - Light",
		Heavy = "Actors - Heavy",
		Sniper = "Actors - Light",
		Engineer = "Actors - Light",
	};
	local actor = RandomAHuman(loadoutToGroupNameTable[loadout] or "Actors", techID);
	if actor.ModuleID ~= techID then
		actor = RandomAHuman("Actors", techID);
	end

	if loadout == "Light" then
		actor:AddInventoryItem(RandomHDFirearm("Weapons - Light", techID));
		actor:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", techID));
		if math.random() < 0.5 then
			actor:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", techID));
		elseif math.random() < 0.8 then
			actor:AddInventoryItem(CreateHDFirearm("Medikit", "Base.rte"));
		end
		if self.AI.isAttackerTeam and math.random() < 0.25 then
			actor:AddInventoryItem(RandomHDFirearm("Tools - Breaching", techID));
		end
	elseif loadout == "Heavy" then
		actor:AddInventoryItem(RandomHDFirearm("Weapons - Heavy", techID));
		if math.random() < 0.3 then
			actor:AddInventoryItem(RandomHDFirearm("Weapons - Light", techID));
			if math.random() < 0.25 then
				actor:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", techID));
			elseif math.random() < 0.35 then
				actor:AddInventoryItem(CreateHDFirearm("Medikit", "Base.rte"));
			end
		else
			actor:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", techID));
			if math.random() < 0.3 then
				actor:AddInventoryItem(RandomHeldDevice("Shields", techID));
				actor:AddInventoryItem(CreateHDFirearm("Medikit", "Base.rte"));
			else
				actor:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", techID));
			end
		end
	elseif loadout == "Sniper" then
		actor:AddInventoryItem(RandomHDFirearm("Weapons - Sniper", techID));
		actor:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", techID));
		if math.random() < 0.3 then
			actor:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", techID));
		else
			actor:AddInventoryItem(CreateHDFirearm("Medikit", "Base.rte"));
		end
	elseif loadout == "Engineer" then
		if math.random() < 0.7 then
			actor:AddInventoryItem(RandomHDFirearm("Weapons - Light", techID));
		else
			actor:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", techID));
			if math.random() < 0.2 then
				actor:AddInventoryItem(RandomHeldDevice("Shields", techID));
			elseif math.random() < 0.6 then
				actor:AddInventoryItem(CreateHDFirearm("Medikit", "Base.rte"));
			end
		end
		actor:AddInventoryItem(RandomHDFirearm("Tools - Diggers", techID));
		if self.AI.isAttackerTeam and math.random() < 0.5 then
			actor:AddInventoryItem(RandomTDExplosive("Tools - Breaching", techID));
		end
	else
		actor = RandomAHuman("Actors", techID);
		actor:AddInventoryItem(RandomHDFirearm("Weapons - Primary", techID));
		actor:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", techID));

		if math.random() < 0.25 then
			actor:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", techID));
		elseif math.random() < 0.5 then
			actor:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", techID));
		elseif math.random() < 0.5 then
			actor:AddInventoryItem(RandomHeldDevice("Shields", techID));
		else
			actor:AddInventoryItem(CreateHDFirearm("Medikit", "Base.rte"));
		end
		if self.AI.isAttackerTeam and math.random() < 0.15 then
			actor:AddInventoryItem(RandomTDExplosive("Tools - Breaching", techID));
		end
	end

	return actor;
end

function BunkerBreach:CreateCrab(techID, createTurret)
	local group = createTurret and "Actors - Turrets" or "Actors - Mecha";
	local actor = RandomACrab(group, techID);
	return actor;
end

function BunkerBreach:CreateBrainBot(team, techID)
	local techID = PresetMan:GetModuleID(self:GetTeamTech(team));
	local actor;
	if techID ~= -1 and team == self.attackerTeam then
		actor = PresetMan:GetLoadout("Infantry Brain", techID, false);
	else
		actor = RandomAHuman("Brains", techID);
		actor:AddInventoryItem(RandomHDFirearm("Weapons - Light", techID));
		if team == self.attackerTeam then
			actor:AddInventoryItem(CreateHDFirearm("Constructor", "Base.rte"));
		else
			actor:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", techID));
			actor:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", techID));
		end
	end
	actor.AIMode = Actor.AIMODE_SENTRY;
	actor.Team = team;
	return actor;
end
