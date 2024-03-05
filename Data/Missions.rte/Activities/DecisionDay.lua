require("Constants");
require("Utilities")
require("Scripts/Shared/Activity_SpeedrunHelper")

function DecisionDayDeployBrainPieSliceActivation(pieMenuOwner, pieMenu, pieSlice)
	ActivityMan:GetActivity():SaveNumber("DeployBrain", pieMenuOwner:GetController().Player + 1);
end

function DecisionDayUnDeployBrainPieSliceActivation(pieMenuOwner, pieMenu, pieSlice)
	ActivityMan:GetActivity():SaveNumber("UndeployBrain", pieMenuOwner:GetController().Player + 1);
end

function DecisionDaySwapControlPieSliceActivation(pieMenuOwner, pieMenu, pieSlice)
	ActivityMan:GetActivity():SaveNumber("SwapControl", pieMenuOwner:GetController().Player + 1);
end

function DecisionDay:GetAreaNameForBunker(bunkerNameOrId)
	if bunkerNameOrId == 1 or bunkerNameOrNumber == "frontBunker" then
		return "Front Bunker";
	elseif bunkerNameOrId == 2 or bunkerNameOrNumber == "middleBunker" then
		return "Middle Bunker";
	elseif bunkerNameOrId == 3 or bunkerNameOrNumber == "mainBunker" then
		return "Main Bunker";
	end
end

function DecisionDay:SetupInternalReinforcementsData()
	self.internalReinforcementsDoorParticle = CreateMOSRotating("Background Door", "Base.rte");

	local scene = SceneMan.Scene;
	self.internalReinforcementsData = {};
	self.internalReinforcementsData.doorsAndActorsToSpawn = {};

	for _, bunkerId in pairs(self.bunkerIds) do
		self.internalReinforcementsData[bunkerId] = {};
		self.internalReinforcementsData[bunkerId].enabled = false;

		self.internalReinforcementsData[bunkerId].positions = {};
		self.internalReinforcementsData[bunkerId].area = scene:GetArea(self:GetAreaNameForBunker(bunkerId) .. " Internal Reinforcements");
	end

	-- Note: Bunker region internal reinforcement areas are also counted based on the bunker they're in.
	for bunkerRegionName, bunkerRegionData in pairs(self.bunkerRegions) do
		if bunkerRegionData.internalReinforcementsArea then
			for box in bunkerRegionData.internalReinforcementsArea.Boxes do
				local shouldAddBox = true;
				for internalReinforcementsAreaBox in self.internalReinforcementsData[bunkerRegionData.bunkerId].area.Boxes do
					if internalReinforcementsAreaBox:IsWithinBox(box.Center) then
						shouldAddBox = false;
						break;
					end
				end
				if shouldAddBox then
					self.internalReinforcementsData[bunkerRegionData.bunkerId].area:AddBox(box);
				end
			end
		end
	end

	for _, bunkerId in pairs(self.bunkerIds) do
		for internalReinforcementsBox in self.internalReinforcementsData[bunkerId].area.Boxes do
			local backgroundDoor = CreateTerrainObject("Module Back Middle E", "Base.rte");
			backgroundDoor.Pos = SceneMan:SnapPosition(internalReinforcementsBox.Corner, true);
			self.internalReinforcementsData[bunkerId].positions[#self.internalReinforcementsData[bunkerId].positions + 1] = backgroundDoor.Pos + Vector(24, 24);
			SceneMan:AddSceneObject(backgroundDoor);
		end
	end
end

function DecisionDay:StartActivity(isNewGame)
	if self.Difficulty <= Activity.MINDIFFICULTY then
		self.difficultyRatio = 0.5;
	elseif self.Difficulty <= Activity.CAKEDIFFICULTY then
		self.difficultyRatio = 0.625;
	elseif self.Difficulty <= Activity.EASYDIFFICULTY then
		self.difficultyRatio = 0.75;
	elseif self.Difficulty <= Activity.MEDIUMDIFFICULTY then
		self.difficultyRatio = 1;
	elseif self.Difficulty <= Activity.HARDDIFFICULTY then
		self.difficultyRatio = 1.5;
	elseif self.Difficulty <= Activity.NUTSDIFFICULTY then
		self.difficultyRatio = 1.75;
	elseif self.Difficulty <= Activity.MAXDIFFICULTY then
		self.difficultyRatio = 2;
	end

	self.stages = { followInitialDropShip = 1, showInitialText = 2, attackFrontBunker = 3, frontBunkerCaptured = 4, deployBrain = 5, attackMiddleBunker = 6, middleBunkerCaptured = 7, findTunnel = 8, captureDoorControls = 9, captureMainBunker = 10, attackBrain = 11 };
	self.bunkerIds = { frontBunker = 1, middleBunker = 2, mainBunker = 3 };

	local scene = SceneMan.Scene;
	self.initialDeadBodiesArea = scene:GetArea("Initial Dead Bodies");
	self.initialExtraFOWReveal = scene:GetArea("Initial Extra FOW Reveal");
	self.initialHumanFOWArea = scene:GetArea("Initial Human FOW Area");
	self.initialHumanSpawnArea = scene:GetArea("Initial Human Spawn");
	self.initialDropShipSpawnArea = scene:GetArea("Initial DropShip Spawn");

	self.bunkerAreas = {};
	self.popoutTurretsData = {};
	for _, bunkerId in pairs(self.bunkerIds) do
		local bunkerAreaName = self:GetAreaNameForBunker(bunkerId);
		self.bunkerAreas[bunkerId] = {};
		self.bunkerAreas[bunkerId].totalArea = scene:GetArea(bunkerAreaName);
		self.bunkerAreas[bunkerId].leftDefendersArea = scene:GetArea(bunkerAreaName .. " Left Defenders");
		self.bunkerAreas[bunkerId].rightDefendersArea = scene:GetArea(bunkerAreaName .. " Right Defenders");
		self.bunkerAreas[bunkerId].internalDefendersArea = scene:GetArea(bunkerAreaName .. " Internal Defenders");
		self.bunkerAreas[bunkerId].internalTurretsArea = scene:GetArea(bunkerAreaName .. " Internal Turrets");
		self.bunkerAreas[bunkerId].lzArea = scene:GetArea(bunkerAreaName .. " LZ");

		self.popoutTurretsData[bunkerId] = {};
		self.popoutTurretsData[bunkerId].enabled = false;
		self.popoutTurretsData[bunkerId].totalArea = scene:GetArea(bunkerAreaName .. " External Popout Turrets");
		self.popoutTurretsData[bunkerId].activationArea = scene:GetArea(bunkerAreaName .. " External Popout Turrets Activation");
		self.popoutTurretsData[bunkerId].turretsActivated = false;
		self.popoutTurretsData[bunkerId].deactivationDelayTimer = Timer(2000);
		self.popoutTurretsData[bunkerId].boxData = {};
		for box in self.popoutTurretsData[bunkerId].totalArea.Boxes do
			self.popoutTurretsData[bunkerId].boxData[box] = {};
			self.popoutTurretsData[bunkerId].boxData[box].movementTimer = Timer(1000);
			self.popoutTurretsData[bunkerId].boxData[box].respawnTimer = Timer(10000, 10000);
			self.popoutTurretsData[bunkerId].boxData[box].movementSound = CreateSoundContainer("Door Movement Loop", "Base.rte");
			self.popoutTurretsData[bunkerId].boxData[box].movementSound.Volume = 0.5;
			self.popoutTurretsData[bunkerId].boxData[box].movementSound.Loops = -1;
			self.popoutTurretsData[bunkerId].boxData[box].movementSound.PitchVariation = 0.1;
			self.popoutTurretsData[bunkerId].boxData[box].movementSound.AttenuationStartDistance = 50;
		end
	end
	self.bunkerAreas[self.bunkerIds.mainBunker].frontDoors = scene:GetArea(self:GetAreaNameForBunker(self.bunkerIds.mainBunker) .. " Front Doors");
	self.bunkerAreas[self.bunkerIds.mainBunker].brainDoors = scene:GetArea(self:GetAreaNameForBunker(self.bunkerIds.mainBunker) .. " Brain Doors");
	self.bunkerAreas[self.bunkerIds.mainBunker].rearLZArea = scene:GetArea(self:GetAreaNameForBunker(self.bunkerIds.mainBunker) .. " Rear LZ");

	self.hiddenTunnelArea = scene:GetArea("Hidden Tunnel");
	self.hiddenTunnelBlockingCrateArea = scene:GetArea("Hidden Tunnel Blocking Crate");

	self.humanTeam = Activity.TEAM_1;
	self.aiTeam = Activity.TEAM_2;
	self.humanTeamTech = PresetMan:GetModuleID(self:GetTeamTech(self.humanTeam));
	self.aiTeamTech = PresetMan:GetModuleID(self:GetTeamTech(self.aiTeam));

	local bunkerRegionNames = {
		"Front Bunker Operations",
		"Front Bunker Small Vault",
		"Middle Bunker Operations",
		"Main Bunker Door Controls",
		"Main Bunker Security Tower",
		"Main Bunker Barracks",
		"Main Bunker Armory",
		"Main Bunker Air Traffic Control",
		"Main Bunker Shield Generator",
		"Main Bunker Command Center",
		"Main Bunker Small Vault",
		"Main Bunker Medium Vault",
		"Main Bunker Large Vault"
	}
	local bunkerRegionRecaptureWeights = {};
	bunkerRegionRecaptureWeights["Main Bunker Small Vault"] = 1;
	bunkerRegionRecaptureWeights["Main Bunker Door Controls"] = 2;
	bunkerRegionRecaptureWeights["Main Bunker Medium Vault"] = 3;
	bunkerRegionRecaptureWeights["Main Bunker Large Vault"] = 4;
	bunkerRegionRecaptureWeights["Main Bunker Security Tower"] = 5;
	bunkerRegionRecaptureWeights["Main Bunker Armory"] = 6;
	bunkerRegionRecaptureWeights["Main Bunker Barracks"] = 7;
	bunkerRegionRecaptureWeights["Main Bunker Air Traffic Control"] = 8;

	self.bunkerRegions = {};
	self.captureDisplayScreenTemplate = CreateMOSParticle("Login Screen", "Missions.rte");
	self.fauxdanDisplayScreenTemplate = CreateMOSParticle("Fauxdan Screen", "Missions.rte");
	for _, bunkerRegionName in ipairs(bunkerRegionNames) do
		self.bunkerRegions[bunkerRegionName] = {
			enabled = false,
			bunkerRegionName = bunkerRegionName,
			bunkerId = bunkerRegionName:find("Front Bunker") and self.bunkerIds.frontBunker or (bunkerRegionName:find("Middle Bunker") and self.bunkerIds.middleBunker or self.bunkerIds.mainBunker);
			totalArea = scene:GetArea(bunkerRegionName),
			captureArea = scene:GetArea(bunkerRegionName .. " Capture"),
			captureDisplayArea = scene:GetArea(bunkerRegionName .. " Capture Display"),
			captureDisplayScreens = {},
			internalReinforcementsArea = scene:HasArea(bunkerRegionName .. " Internal Reinforcements") and scene:GetOptionalArea(bunkerRegionName .. " Internal Reinforcements") or nil,
			defenderArea = scene:GetArea(bunkerRegionName .. " Defenders"),
			ownerTeam = self.aiTeam,
			hasBeenCapturedAtLeastOnceByHumanTeam = false,
			captureCount = 0,
			captureLimit = 600 * self.difficultyRatio,
			aiRegionDefenseTimer = Timer(60000 / self.difficultyRatio, 60000 / self.difficultyRatio),
			aiRegionAttackTimer = Timer(90000 / self.difficultyRatio),
			aiRecaptureWeight = bunkerRegionRecaptureWeights[bunkerRegionName] or 0,
			fauxdanDisplayArea = scene:HasArea(bunkerRegionName .. " Fauxdan Display") and scene:GetOptionalArea(bunkerRegionName .. " Fauxdan Display") or nil,
			fauxdanDisplayScreens = {},
			shieldedArea = scene:HasArea(bunkerRegionName .. " Shield") and scene:GetOptionalArea(bunkerRegionName .. " Shield") or nil,
			brainDoor = scene:HasArea(bunkerRegionName .. " Brain Door") and scene:GetOptionalArea(bunkerRegionName .. " Brain Door") or nil,
			brain = scene:HasArea(bunkerRegionName .. " Shield") and scene:GetOptionalArea(bunkerRegionName .. " Brain") or nil,
		};
		if bunkerRegionName:find("Vault") then
			self.bunkerRegions[bunkerRegionName].incomeMultiplier = bunkerRegionName:find("Large") and 2 or (bunkerRegionName:find("Medium") and 1.5 or 1);
		end
	end

	self.controlledBunkerRegionComputerTerrainObjects = {
		[self.humanTeam] = {
			["left"] = CreateTerrainObject("Decision Day Computer Left On", "Missions.rte"),
			["right"] = CreateTerrainObject("Decision Day Computer Right On", "Missions.rte")
		},
		[self.aiTeam] = {
			["left"] = CreateTerrainObject("Decision Day Computer Left Off", "Missions.rte"),
			["right"] = CreateTerrainObject("Decision Day Computer Right Off", "Missions.rte")
		}
	}

	self:SetupInternalReinforcementsData();

	self.humanPlayers = {};
	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) and self:GetTeamOfPlayer(player) == self.humanTeam then
			self.humanPlayers[#self.humanPlayers + 1] = player;
		end
	end

	self.currentStage = self.stages.followInitialDropShip;
	self.currentMessageNumber = 1;
	self.numberOfMessagesForStage = 1;
	self.cameraMinimumX = self.initialDropShipSpawnArea.Center.X - 50;

	self.messageTimer = Timer(10000);
	self.vaultIncomeTimer = Timer(1000);
	self.mainBunkerShieldedAreaFOWTimer = Timer(10000);

	local createNewActorDataTable = function()
		return { count = 0 };
	end

	self.alliedData = {};
	self.alliedData.spawnsEnabled = false;
	self.alliedData.spawnTimer = Timer(3000, 2500);
	self.alliedData.actors = {};
	self.alliedData.actors.sentries = createNewActorDataTable();
	self.alliedData.actors.attackers = createNewActorDataTable();
	self.alliedData.attackerLimit = 10;

	self.aiData = {};
	self.aiData.externalSpawnsEnabled = false;
	self.aiData.externalSpawnTimer = Timer(120000 / self.difficultyRatio);
	self.aiData.internalReinforcementsEnabled = false;
	self.aiData.internalReinforcementsTimer = Timer(100000 / self.difficultyRatio);
	self.aiData.internalReinforcementLimit = 12 * self.difficultyRatio;
	self.aiData.numberOfInternalReinforcementsCreated = 0;
	self.aiData.internalReinforcementPositionsCalculationCoroutines = {};
	self.aiData.bunkerRegionDefenseRange = math.max(500, math.min(750 * self.difficultyRatio, 1000));
	self.aiData.attackerLimit = 10 * self.difficultyRatio;
	self.aiData.attackersPerSpawn = 4 * self.difficultyRatio;
	self.aiData.attackTarget = nil;
	self.aiData.attackRetargetTimer = Timer(15000);

	self.aiData.brainSpawned = false;
	self.aiData.brainDefenderSpawnTimer = Timer(10000 / self.difficultyRatio);
	self.aiData.brainDefenderReplenishTimer = Timer(30000 / self.difficultyRatio);
	self.aiData.brainDefendersTotal = 20 * self.difficultyRatio;
	self.aiData.brainDefendersRemaining = self.aiData.brainDefendersTotal;

	self.aiData.actors = {};
	self.aiData.actors.sentries = createNewActorDataTable();
	self.aiData.actors.attackers = createNewActorDataTable();
	self.aiData.actors.internalReinforcements = createNewActorDataTable();
	self.aiData.actors.internalTurrets = createNewActorDataTable();
	self.aiData.actors.externalPopoutTurrets = createNewActorDataTable();

	self.aiData.enemiesInsideBunkers = {};
	for _, bunkerId in pairs(self.bunkerIds) do
		self.aiData.enemiesInsideBunkers[bunkerId] = {};
	end

	self.vaultCaptureIncome = 1500;
	self.vaultTickIncome = 6;

	self.initialDropShipDestroyed = false;
	self.anyHumanHasSeenObjectives = false;
	self.anyHumanHasDeployedABrain = false;
	self.tunnelHasBeenEntered = false;

	self.humansAreControllingAlliedActors = false;

	self.frontBunkerAlliedDefendersSpawned = false;
	self.middleBunkerAlliedDefendersSpawned = false;

	self.initialDropShipsAndVelocities = {};
	self.previousCraftLZInfo = {};

	self.deployBrainPieSlice = CreatePieSlice("Land", "Base.rte");
	self.deployBrainPieSlice.PresetName = "Deploy Brain";
	self.deployBrainPieSlice.Description = "Deploy Brain";
	self.deployBrainPieSlice.Type = PieSlice.NoType;
	self.deployBrainPieSlice.Direction = Directions.Up;
	self.deployBrainPieSlice.ScriptPath = "Missions.rte/Activities/DecisionDay.lua";
	self.deployBrainPieSlice.FunctionName = "DecisionDayDeployBrainPieSliceActivation";

	self.undeployBrainPieSlice = CreatePieSlice("Launch", "Base.rte");
	self.undeployBrainPieSlice.PresetName = "Undeploy Brain";
	self.undeployBrainPieSlice.Description = "Undeploy Brain";
	self.undeployBrainPieSlice.Type = PieSlice.NoType;
	self.undeployBrainPieSlice.Direction = Directions.Up;
	self.undeployBrainPieSlice.ScriptPath = "Missions.rte/Activities/DecisionDay.lua";
	self.undeployBrainPieSlice.FunctionName = "DecisionDayUnDeployBrainPieSliceActivation";

	self.buyMenuPieSlice = CreatePieSlice("BuyMenu", "Base.rte");
	self.buyMenuPieSlice.Direction = Directions.Up;

	self.swapControlPieSlice = CreatePieSlice("Cycle", "Base.rte");
	self.swapControlPieSlice.PresetName = "Swap Troop Control";
	self.swapControlPieSlice.Description = "Swap Troop Control";
	self.swapControlPieSlice.Type = PieSlice.NoType;
	self.swapControlPieSlice.Direction = Directions.Up;
	self.swapControlPieSlice.ScriptPath = "Missions.rte/Activities/DecisionDay.lua";
	self.swapControlPieSlice.FunctionName = "DecisionDaySwapControlPieSliceActivation";

	self.popoutTurretTemplate = CreateACrab("Special TradeStar Ceiling Turret", "Missions.rte");
	self.popoutTurretTemplate:EnableDeepCheck(true);
	self.popoutTurretTemplate.Turret:EnableDeepCheck(true);
	self.popoutTurretTemplate.Team = self.aiTeam;
	self.popoutTurretTemplate.RotAngle = math.pi * 0.25;
	self.popoutTurretTemplate.AimRangeUpperLimit = 0;
	self.popoutTurretTemplate.AimRangeLowerLimit = math.pi * 0.75;

	-- Hangar doors are set to inactive so they don't randomly open and close, since they're useless.
	for actor in MovableMan.AddedActors do
		if IsADoor(actor) and actor.PresetName == "Door Rotate Long" then
			actor.Status = Actor.INACTIVE;
		end
	end

	self.keysToSaveAndLoadValuesOf = {
		"currentStage", "currentMessageNumber", "numberOfMessagesForStage", "cameraMinimumX",
		"messageTimer", "vaultIncomeTimer", "mainBunkerShieldedAreaFOWTimer",
		"alliedData.spawnsEnabled", "alliedData.spawnTimer", "alliedData.attackerLimit",
		"aiData.externalSpawnsEnabled", "aiData.externalSpawnTimer", "aiData.internalReinforcementsEnabled", "aiData.internalReinforcementsTimer", "aiData.brainSpawned", "aiData.brainDefenderSpawnTimer", "aiData.brainDefenderReplenishTimer", "aiData.brainDefendersRemaining",
		"initialDropShipDestroyed", "anyHumanHasSeenObjectives", "anyHumanHasDeployedABrain", "tunnelHasBeenEntered",
		"humansAreControllingAlliedActors",
		"frontBunkerAlliedDefendersSpawned", "middleBunkerAlliedDefendersSpawned",
	}
	for bunkerRegionName, bunkerRegionData in pairs(self.bunkerRegions) do
		local bunkerRegionKeyPrefix = "bunkerRegions." .. bunkerRegionName .. ".";
		self.keysToSaveAndLoadValuesOf[#self.keysToSaveAndLoadValuesOf + 1] = bunkerRegionKeyPrefix .. "enabled";
		self.keysToSaveAndLoadValuesOf[#self.keysToSaveAndLoadValuesOf + 1] = bunkerRegionKeyPrefix .. "ownerTeam";
		self.keysToSaveAndLoadValuesOf[#self.keysToSaveAndLoadValuesOf + 1] = bunkerRegionKeyPrefix .. "hasBeenCapturedAtLeastOnceByHumanTeam";
		self.keysToSaveAndLoadValuesOf[#self.keysToSaveAndLoadValuesOf + 1] = bunkerRegionKeyPrefix .. "captureCount";
		self.keysToSaveAndLoadValuesOf[#self.keysToSaveAndLoadValuesOf + 1] = bunkerRegionKeyPrefix .. "aiRegionDefenseTimer";
		self.keysToSaveAndLoadValuesOf[#self.keysToSaveAndLoadValuesOf + 1] = bunkerRegionKeyPrefix .. "aiRegionAttackTimer";
	end
	for _, bunkerId in pairs(self.bunkerIds) do
		self.keysToSaveAndLoadValuesOf[#self.keysToSaveAndLoadValuesOf + 1] = "internalReinforcementsData." .. tostring(bunkerId) .. ".enabled";

		local popoutTurretDataPrefix = "popoutTurretsData." .. tostring(bunkerId) .. ".";
		self.keysToSaveAndLoadValuesOf[#self.keysToSaveAndLoadValuesOf + 1] = popoutTurretDataPrefix .. "enabled";
		self.keysToSaveAndLoadValuesOf[#self.keysToSaveAndLoadValuesOf + 1] = popoutTurretDataPrefix .. "turretsActivated";
		self.keysToSaveAndLoadValuesOf[#self.keysToSaveAndLoadValuesOf + 1] = popoutTurretDataPrefix .. "deactivationDelayTimer";
	end

	if isNewGame then
		self:StartNewGame();
	else
		self:ResumeLoadedGame();
	end
end

function DecisionDay:OnSave()
	local function getTableValueByKey(tableOrSuperTableToGetValueFrom, key)
		local firstPeriodPosition = key:find("%.");
		if firstPeriodPosition ~= nil then
			local trimmedTableKey = key:sub(1, firstPeriodPosition - 1);
			local trimmedValueKey = key:sub(firstPeriodPosition + 1);

			local tableToGetValueFrom = tableOrSuperTableToGetValueFrom[tonumber(trimmedTableKey) or trimmedTableKey];
			return getTableValueByKey(tableToGetValueFrom, tonumber(trimmedValueKey) or trimmedValueKey);
		else
			return tableOrSuperTableToGetValueFrom[key];
		end
	end

	for _, key in pairs(self.keysToSaveAndLoadValuesOf) do
		local value = getTableValueByKey(self, key);

		if type(value) == "number" then
			self:SaveNumber(key, value);
		elseif type(value) == "string" then
			self:SaveString(key, value);
		elseif type(value) == "boolean" then
			self:SaveNumber(key, value and 1 or 0);
		elseif type(value) == "table" then
			print("Saving Error: Tables are not supported, use . for subkeys!");
		elseif type(value) == "userdata" then
			if value.ElapsedSimTimeMS ~= nil then
				self:SaveNumber(key .. ".SimTimeLimitMS", value:GetSimTimeLimitMS());
				self:SaveNumber(key .. ".ElapsedSimTimeMS", value.ElapsedSimTimeMS);
			else
				print("Saving Error: The only supported userdata type is Timer!")
			end
		end
	end

	while #self.aiData.internalReinforcementPositionsCalculationCoroutines > 0 do
		self:UpdateAIInternalReinforcements(true);
	end
end

function DecisionDay:StartNewGame()
	self.speedrunData = ActivitySpeedrunHelper.Setup(self, self.DoSpeedrunMode);
	
	self:SetTeamFunds(0, self.humanTeam);
	self.BuyMenuEnabled = false;

	self:SetupFogOfWar();

	self:SetupStorageCrateInventories();

	local automoverController = CreateActor("Invisible Automover Controller", "Base.rte");
	automoverController.Pos = Vector();
	automoverController.Team = self.aiTeam;
	--automoverController:SetNumberValue("HumansRemainUpright", 1);
	MovableMan:AddActor(automoverController);

	for box in self.initialDeadBodiesArea.Boxes do
		local deadBody = RandomAHuman("Actors", self.aiTeamTech);
		deadBody.Team = self.aiTeam;
		deadBody.Pos = box.Center;
		deadBody.Vel.X = -2;
		if deadBody.DeathSound then 
			deadBody.DeathSound.Volume = 0; 
		end
		if deadBody.PainSound then 
			deadBody.PainSound.Volume = 0; 
		end
		deadBody.HFlipped = true;
		deadBody.Health = 0;
		MovableMan:AddActor(deadBody);

		local weapon = RandomHDFirearm("Weapons - Light", self.aiTeamTech);
		weapon.Pos = box.Center  + Vector(-50, 6);
		weapon.HFlipped = true;
		weapon.RotAngle = 0.45;
		weapon.Vel = Vector();
		MovableMan:AddItem(weapon);
		weapon.ToSettle = true;
	end

	local hiddenTunnelBlockingCrate = CreateTerrainObject("Metal Crate Small A FG", "Base.rte");
	hiddenTunnelBlockingCrate.Pos = SceneMan:SnapPosition(self.hiddenTunnelBlockingCrateArea.FirstBox.Corner, true);
	SceneMan:AddSceneObject(hiddenTunnelBlockingCrate);

	self:DoInitialHumanSpawns();

	self:SpawnAreaDefinedAIDefenders();
end

function DecisionDay:SetupFogOfWar()
	if self:GetFogOfWarEnabled() then
		SceneMan:MakeAllUnseen(Vector(20, 20), self.humanTeam);
		SceneMan:MakeAllUnseen(Vector(20, 20), self.aiTeam);

		-- Reveal above ground for everyone.
		for x = 0, SceneMan.SceneWidth - 1, 20 do
			SceneMan:CastSeeRay(self.humanTeam, Vector(x, 0), Vector(0, SceneMan.SceneHeight), Vector(), 1, 9);
			SceneMan:CastSeeRay(self.aiTeam, Vector(x, 0), Vector(0, SceneMan.SceneHeight), Vector(), 1, 9);
		end

		-- Reveal extra areas - roofs and such that don't get handled by the vertical rays.
		for box in self.initialExtraFOWReveal.Boxes do
			SceneMan:RevealUnseenBox(box.Corner.X, box.Corner.Y, box.Width, box.Height, self.humanTeam);
			SceneMan:RevealUnseenBox(box.Corner.X, box.Corner.Y, box.Width, box.Height, self.aiTeam);
		end

		--Reveal the starting area for the human team and hide it for the ai.
		for box in self.initialHumanFOWArea.Boxes do
			SceneMan:RevealUnseenBox(box.Corner.X, box.Corner.Y, box.Width, box.Height, self.humanTeam);
			SceneMan:RestoreUnseenBox(box.Corner.X, box.Corner.Y, box.Width, box.Height, self.aiTeam);
		end

		-- Reveal the dead bodies for the human team.
		SceneMan:RevealUnseenBox(self.initialDeadBodiesArea.FirstBox.Center.X - 150, self.initialDeadBodiesArea.FirstBox.Center.Y - 150, 200, 420, self.humanTeam);

		-- Reveal the bunkers for the AI and hide them for the player.
		for _, bunkerArea in ipairs(self.bunkerAreas) do
			for box in bunkerArea.totalArea.Boxes do
				SceneMan:RevealUnseenBox(box.Corner.X, box.Corner.Y, box.Width, box.Height, self.aiTeam);
				SceneMan:RestoreUnseenBox(box.Corner.X, box.Corner.Y, box.Width, box.Height, self.humanTeam);
			end
		end

		-- Reveal a circle around actors.
		for actor in MovableMan.AddedActors do
			for angle = 0, math.pi * 2, 0.05 do
				SceneMan:CastSeeRay(actor.Team, actor.EyePos, Vector(150 + FrameMan.PlayerScreenWidth * 0.5, 0):RadRotate(angle), Vector(), 1, 4);
			end
		end
	end
end

function DecisionDay:SetupStorageCrateInventories()
	for actor in MovableMan.AddedActors do
		if actor.PresetName:find("Decision Day Storage Crate") then
			for i = 1, 99 do
				local inventoryTypeString = "Inventory" .. tostring(i) .. "Type";

				if actor:StringValueExists(inventoryTypeString) then
					local inventoryType = actor:GetStringValue(inventoryTypeString);

					local inventoryPresetNameString = "Inventory" .. tostring(i) .. "PresetName";
					local inventoryPresetName = actor:StringValueExists(inventoryPresetNameString) and actor:GetStringValue(inventoryPresetNameString) or nil;

					local inventoryGroupString = "Inventory" .. tostring(i) .. "Group";
					local inventoryGroup = actor:StringValueExists(inventoryGroupString) and actor:GetStringValue(inventoryGroupString) or nil;

					local inventoryInfantryTypeString = "Inventory" .. tostring(i) .. "InfantryType";
					local inventoryInfantryType = actor:StringValueExists(inventoryInfantryTypeString) and actor:GetStringValue(inventoryInfantryTypeString) or nil;

					local inventoryCountString = "Inventory" .. tostring(i) .. "Count";
					local inventoryCount = actor:NumberValueExists(inventoryCountString) and actor:GetNumberValue(inventoryCountString) or 1;
					for _ = 1, inventoryCount do
						local inventoryItem;
						if inventoryPresetName ~= nil then
							inventoryItem = _G["Create" .. inventoryType](inventoryPresetName);
						elseif inventoryGroup ~= nil then
							inventoryItem = _G["Random" .. inventoryType](inventoryGroup, self.humanTeamTech);
						elseif inventoryInfantryTypeString ~= nil then
							inventoryItem = self:CreateInfantry(self.humanTeam, inventoryInfantryType);
							inventoryItem.PlayerControllable = true;
						end

						if inventoryItem ~= nil then
							inventoryItem.Team = self.humanTeam;
							actor:AddInventoryItem(inventoryItem);
						end
					end
				else
					break;
				end
			end
		end
	end
end

function DecisionDay:DoInitialHumanSpawns()
	local firstActorPos = Vector(self.initialHumanSpawnArea.FirstBox.Corner.X, self.initialHumanSpawnArea.Center.Y);
	local initialActorFunds = 1300 / self.difficultyRatio;
	local spawnedActors = {};
	while initialActorFunds > 0 do
		local actor = self:SpawnInfantry(self.humanTeam, #spawnedActors < 3 and "CQB" or "Heavy", firstActorPos, Actor.AIMODE_SENTRY, true);
		spawnedActors[#spawnedActors + 1] = actor;
		actor.PlayerControllable = true;
		if #spawnedActors < 3 then
			actor:AddInventoryItem(RandomTDExplosive("Tools - Breaching", self.humanTeamTech));
		elseif math.random() < 0.65 then
			actor:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", self.humanTeamTech));
		end
		initialActorFunds = initialActorFunds - actor:GetTotalValue(self.humanTeamTech, 1);
	end
	for _, player in pairs(self.humanPlayers) do
		local brain = PresetMan:GetLoadout("Infantry Brain", self.humanTeamTech, false);
		spawnedActors[#spawnedActors + 1] = brain;
		if brain then
			brain:RemoveInventoryItem("Constructor");
		else
			brain = RandomAHuman("Brains", self.humanTeamTech);
			brain:AddToGroup("Brain " .. tostring(player));
			brain:AddInventoryItem(RandomHDFirearm("Weapons - Light", self.humanTeamTech));
			brain:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.humanTeamTech));
		end
		brain.Pos = firstActorPos;
		brain.Team = self.humanTeam;
		brain.AIMode = Actor.AIMODE_SENTRY;
		MovableMan:AddActor(brain);
		self:SetPlayerBrain(brain, player);
		self:SetObservationTarget(brain.Pos, player);
	end
	
	local actorXOffset = math.min(30, 275 / #spawnedActors);
	for index, actor in ipairs(spawnedActors) do
		actor.Pos.X = actor.Pos.X + (actorXOffset * (index - 1));
		actor.Pos.Y = 0;
		actor.Pos = SceneMan:MovePointToGround(actor.Pos, actor.Radius, 5);
	end
end

function DecisionDay:SpawnAreaDefinedAIDefenders()
	local bunkerId = self.bunkerIds.frontBunker;
	if self.currentStage == self.stages.deployBrain then
		bunkerId = self.bunkerIds.middleBunker;
	elseif self.currentStage == self.stages.middleBunkerCaptured then
		bunkerId = self.bunkerIds.mainBunker;
	end

	local defenderType = self.difficultyRatio >= 1 and "Sniper" or "Light";
	for box in self.bunkerAreas[bunkerId].leftDefendersArea.Boxes do
		local actor = self:SpawnInfantry(self.aiTeam, defenderType, box.Center, Actor.AIMODE_SENTRY, true);
		actor:AddToGroup("AI Sentries");
		self.aiData.actors.sentries[actor.UniqueID] = actor;
		self.aiData.actors.sentries.count = self.aiData.actors.sentries.count + 1;
	end

	for box in self.bunkerAreas[bunkerId].rightDefendersArea.Boxes do
		local actor = self:SpawnInfantry(self.aiTeam, defenderType, box.Center, Actor.AIMODE_SENTRY, false);
		actor:AddToGroup("AI Sentries");
		self.aiData.actors.sentries[actor.UniqueID] = actor;
		self.aiData.actors.sentries.count = self.aiData.actors.sentries.count + 1;
	end

	local areaCenterPointX = self.bunkerAreas[bunkerId].totalArea.Center.X
	defenderType = self.difficultyRatio >= 1 and "CQB" or "Light";
	for box in self.bunkerAreas[bunkerId].internalDefendersArea.Boxes do
		local actor = self:SpawnInfantry(self.aiTeam, defenderType, box.Center, Actor.AIMODE_SENTRY, box.Center.X > areaCenterPointX);
		actor:AddToGroup("AI Sentries");
		self.aiData.actors.sentries[actor.UniqueID] = actor;
		self.aiData.actors.sentries.count = self.aiData.actors.sentries.count + 1;
	end

	for box in self.bunkerAreas[bunkerId].internalTurretsArea.Boxes do
		local actor = self:CreateCrab(self.aiTeam, true);
		actor.Pos = SceneMan:SnapPosition(box.Center, true);
		actor.AIMode = Actor.AIMODE_SENTRY;
		actor.HFlipped = box.Center.X > areaCenterPointX;
		actor.PinStrength = 0;
		actor:AddToGroup("AI Internal Turrets");
		self.aiData.actors.internalTurrets[actor.UniqueID] = actor;
		self.aiData.actors.internalTurrets.count = self.aiData.actors.internalTurrets.count + 1;
		MovableMan:AddActor(actor);
	end

	for bunkerRegionName, bunkerRegionData in pairs(self.bunkerRegions) do
		if bunkerRegionName:find(self:GetAreaNameForBunker(bunkerId)) then
			areaCenterPointX = bunkerRegionData.totalArea.Center.X;
			for box in bunkerRegionData.defenderArea.Boxes do
				local actor = self:SpawnInfantry(self.aiTeam, defenderType, box.Center, Actor.AIMODE_SENTRY, box.Center.X > areaCenterPointX);
				actor:AddToGroup("AI Region Defenders");
				actor:AddToGroup("AI Region Defenders - " .. bunkerRegionName);
				self.aiData.actors.sentries[actor.UniqueID] = actor;
				self.aiData.actors.sentries.count = self.aiData.actors.sentries.count + 1;
			end
		end
	end
end

function DecisionDay:SetTableValueByKey(tableToSetValueFor, keyToLoad)
	local tableKey = keyToLoad;
	if tableKey:find("%.") then
		tableKey = tableKey:sub(1 - (tableKey:reverse()):find("%."));
		tableKey = tonumber(tableKey) or tableKey;
	end

	local existingValue = tableToSetValueFor[tableKey];
	local existingValueType = type(existingValue);

	if existingValueType == "nil" then
		print("Loading Error: Tried to load " .. tableKey .." but there's no existing value for it!");
	end

	if existingValueType == "number" then
		tableToSetValueFor[tableKey] = self:LoadNumber(keyToLoad);
	elseif existingValueType == "string" then
		tableToSetValueFor[tableKey] = self:LoadString(keyToLoad);
	elseif existingValueType == "boolean" then
		tableToSetValueFor[tableKey] = self:LoadNumber(keyToLoad) ~= 0;
	elseif existingValueType == "table" then
		print("Loading Error: Tables are not supported, use . for subkeys!")
	elseif existingValueType == "userdata" then
		if existingValue.ElapsedSimTimeMS ~= nil then
			tableToSetValueFor[tableKey]:SetSimTimeLimitMS(self:LoadNumber(keyToLoad .. ".SimTimeLimitMS"));
			tableToSetValueFor[tableKey].ElapsedSimTimeMS = self:LoadNumber(keyToLoad .. ".ElapsedSimTimeMS");
		else
			print("Loading Error: The only supported userdata type is Timer!");
		end
	end
end

function DecisionDay:ResumeLoadedGame()
	local function getTableForFullKey(tableOrSuperTableToGetValueFrom, fullKey)
		local firstPeriodPosition = fullKey:find("%.");
		if firstPeriodPosition ~= nil then
			local trimmedTableKey = fullKey:sub(1, firstPeriodPosition - 1);
			local trimmedValueKey = fullKey:sub(firstPeriodPosition + 1);

			local tableToGetValueFrom = tableOrSuperTableToGetValueFrom[tonumber(trimmedTableKey) or trimmedTableKey];
			return getTableForFullKey(tableToGetValueFrom, tonumber(trimmedValueKey) or trimmedValueKey);
		else
			return tableOrSuperTableToGetValueFrom;
		end
	end

	for _, key in pairs(self.keysToSaveAndLoadValuesOf) do
		local tableToSetValueFor = getTableForFullKey(self, tonumber(key) or key);
		self:SetTableValueByKey(tableToSetValueFor, key);
	end
	self.aiData.attackRetargetTimer.ElapsedSimTimeMS = self.aiData.attackRetargetTimer:GetSimTimeLimitMS(); -- We don't save ai attack target, so make sure we pick an attack target right away.

	local function handleActorTableEntry(actor, tableToAddActorTo)
		tableToAddActorTo[actor.UniqueID] = IsAHuman(actor) and ToAHuman(actor) or ToACrab(actor);
		tableToAddActorTo.count = tableToAddActorTo.count + 1;
	end

	for actor in MovableMan.AddedActors do
		if self.currentStage < self.stages.frontBunkerCaptured and actor.Team == self.humanTeam and IsACDropShip(actor) then
			actor.ImpulseDamageThreshold = 1;
			actor.GlobalAccScalar = 1.75;
			self.initialDropShipsAndVelocities[#self.initialDropShipsAndVelocities + 1] = { dropShip = ToACDropShip(actor), velX = actor.Vel.X };
		elseif actor:IsInGroup("Brains") and actor.Team == self.humanTeam then
			for _, player in pairs(self.humanPlayers) do
				if actor:IsInGroup("Brain " .. tostring(player)) then
					self:SetPlayerBrain(actor, player);
				elseif actor:IsInGroup("Deployed Brain " .. tostring(player)) then
					self:SetPlayerBrain(actor, player);

					local undeployBrainPieSlice = self.undeployBrainPieSlice:Clone();
					undeployBrainPieSlice.Direction = Directions.Left;
					actor.PieMenu:AddPieSliceIfPresetNameIsUnique(undeployBrainPieSlice, self);

					local swapControlPieSlice = self.swapControlPieSlice:Clone();
					swapControlPieSlice.Direction = Directions.Right;
					actor.PieMenu:AddPieSliceIfPresetNameIsUnique(swapControlPieSlice, self);
				elseif actor:IsInGroup("Empty Brain Body " .. tostring(player)) then
					actor = ToAHuman(actor);
					actor.Head.Scale = 0;
					actor:GetController().InputMode = Controller.CIM_DISABLED;
				end
			end
		elseif actor:IsInGroup("Allied Sentries") then
			handleActorTableEntry(actor, self.alliedData.actors.sentries);
		elseif actor:IsInGroup("Allied Attackers") then
			handleActorTableEntry(actor, self.alliedData.actors.attackers)
		elseif actor:IsInGroup("AI Sentries") or actor:IsInGroup("AI Region Defenders") then
			handleActorTableEntry(actor, self.aiData.actors.sentries);
		elseif actor:IsInGroup("AI Internal Turrets") then
			handleActorTableEntry(actor, self.aiData.actors.internalTurrets);
		elseif actor:IsInGroup("AI Internal Reinforcements") then
			handleActorTableEntry(actor, self.aiData.actors.internalReinforcements);
		elseif actor:IsInGroup("AI Attackers") then
			handleActorTableEntry(actor, self.aiData.actors.attackers);
		elseif IsACDropShip(actor) or IsACRocket(actor) then
			if actor.PresetName:find("Decision Day Storage Crate") then
				for inventory in actor.Inventory do
					inventory.Team = self.humanTeam;
				end
			else
				actor.AIMode = actor:IsInventoryEmpty() and Actor.AIMODE_RETURN or Actor.AIMODE_DELIVER;
				for inventory in actor.Inventory do
					if inventory:IsInGroup("Allied Attackers") then
						handleActorTableEntry(inventory, self.alliedData.actors.attackers);
					elseif inventory:IsInGroup("AI Attackers") then
						handleActorTableEntry(inventory, self.aiData.actors.attackers);
					end
				end
			end
		elseif actor.PresetName == self.popoutTurretTemplate.PresetName then
			actor.ToDelete = true;
		elseif IsADoor(actor) then
			if self.bunkerAreas[self.bunkerIds.mainBunker].frontDoors.FirstBox:IsWithinBox(actor.Pos) then
				actor = ToADoor(actor);
				local bunkerRegion = self.bunkerRegions["Main Bunker Door Controls"];
				if actor.Team ~= bunkerRegion.ownerTeam then
					MovableMan:ChangeActorTeam(actor, bunkerRegion.ownerTeam);
				end
				if bunkerRegion.ownerTeam == self.humanTeam then
					actor.Status = Actor.INACTIVE;
					actor:OpenDoor();
				else
					actor.Status = Actor.STABLE;
				end
			elseif self.bunkerRegions["Main Bunker Command Center"].brainDoor.FirstBox:IsWithinBox(actor.Pos) then
				actor.Status = Actor.INACTIVE;
				if self.bunkerRegions["Main Bunker Command Center"].ownerTeam == self.humanTeam then
					ToADoor(actor):OpenDoor();
				else
					ToADoor(actor):CloseDoor();
				end
			end
		end
	end

	for particle in MovableMan.AddedParticles do
		if particle.PresetName == self.fauxdanDisplayScreenTemplate.PresetName or particle.PresetName == self.captureDisplayScreenTemplate.PresetName then
			particle.ToDelete = true;
		end
	end

	if self.currentStage >= self.stages.frontBunkerCaptured then
		self:UpdateLZAreas();
		self:UpdateAlliedAttackersWaypoint();
	end
end

function DecisionDay:DoGameOverCheck()
	if self.WinnerTeam == Activity.NOTEAM then
		if not MovableMan:GetFirstBrainActor(self.humanTeam) then
			self.WinnerTeam = self.aiTeam;
			self.messageTimer:Reset();
		elseif self.aiData.brainSpawned and not MovableMan:GetFirstBrainActor(self.aiTeam) then
			self.WinnerTeam = self.humanTeam;
			for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
				self:SetObservationTarget(self.bunkerRegions["Main Bunker Command Center"].brain.Center, player);
			end
			self.messageTimer:Reset();
		end
	end
	
	if self.speedrunData and self.WinnerTeam == self.humanTeam then
		self.messageTimer:SetSimTimeLimitMS(10000);
		ActivitySpeedrunHelper.CompleteSpeedrun(self.speedrunData);
	end

	if self.WinnerTeam ~= Activity.NOTEAM and self.messageTimer:IsPastSimTimeLimit() then
		if self.WinnerTeam == self.humanTeam then
			for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
				if self:PlayerActive(player) and self:PlayerHuman(player) then
					self:GetBanner(GUIBanner.RED, player):ClearText();
				end
			end
		end
		ActivityMan:EndActivity();
	end
end

function DecisionDay:DoSpeedrunMode()
	self.Difficulty = Activity.MAXDIFFICULTY;
	self.difficultyRatio = 2;
	for _, bunkerRegionData in pairs(self.bunkerRegions) do
		bunkerRegionData.captureLimit = 600 * self.difficultyRatio;
		bunkerRegionData.aiRegionDefenseTimer:SetSimTimeLimitMS(60000 / self.difficultyRatio);
		bunkerRegionData.aiRegionDefenseTimer.ElapsedSimTimeMS = 60000 / self.difficultyRatio;
		bunkerRegionData.aiRegionAttackTimer:SetSimTimeLimitMS(90000 / self.difficultyRatio);
	end
	
	self.aiData.externalSpawnTimer:SetSimTimeLimitMS(120000 / self.difficultyRatio);
	self.aiData.internalReinforcementsTimer:SetSimTimeLimitMS(100000 / self.difficultyRatio);
	self.aiData.internalReinforcementLimit = 12 * self.difficultyRatio;
	self.aiData.bunkerRegionDefenseRange = math.max(500, math.min(750 * self.difficultyRatio, 1000));
	self.aiData.attackerLimit = 10 * self.difficultyRatio;
	self.aiData.attackersPerSpawn = 4 * self.difficultyRatio;

	self.aiData.brainDefenderSpawnTimer:SetSimTimeLimitMS(10000 / self.difficultyRatio);
	self.aiData.brainDefenderReplenishTimer:SetSimTimeLimitMS(30000 / self.difficultyRatio);
	self.aiData.brainDefendersTotal = 20 * self.difficultyRatio;
	self.aiData.brainDefendersRemaining = self.aiData.brainDefendersTotal;
	
	self:SetTeamFunds(0, self.humanTeam);
	
	self:SetTeamTech(self.humanTeam, "-All-");
	self.humanTeamTech = PresetMan:GetModuleID(self:GetTeamTech(self.humanTeam));
	self:SetTeamTech(self.aiTeam, "-All-");
	self.aiTeamTech = PresetMan:GetModuleID(self:GetTeamTech(self.aiTeam));
	
	AudioMan:PlayMusic("Base.rte/Music/dBSoundworks/bossfight.ogg", -1, -1);
	
	self.messageTimer:SetSimTimeLimitMS(1);
	
	self.currentStage = self.stages.attackFrontBunker;

	self.aiData.internalReinforcementsEnabled = true;
	self.internalReinforcementsData[self.bunkerIds.frontBunker].enabled = true;

	self.bunkerRegions["Front Bunker Operations"].enabled = true;
	self.bunkerRegions["Front Bunker Small Vault"].enabled = true;
end

function DecisionDay:UpdateCurrentStage()
	local previousStage = self.currentStage;

	if self.currentStage == self.stages.followInitialDropShip and self.initialDropShipDestroyed then
		self.currentStage = self.stages.showInitialText;
	elseif self.currentStage == self.stages.showInitialText and self.messageTimer:IsPastSimTimeLimit() then
		self.currentStage = self.stages.attackFrontBunker;

		self.aiData.internalReinforcementsEnabled = true;
		self.internalReinforcementsData[self.bunkerIds.frontBunker].enabled = true;

		self.bunkerRegions["Front Bunker Operations"].enabled = true;
		self.bunkerRegions["Front Bunker Small Vault"].enabled = true;
	elseif self.currentStage == self.stages.attackFrontBunker and self.bunkerRegions["Front Bunker Operations"].ownerTeam == self.humanTeam then
		self.currentStage = self.stages.frontBunkerCaptured;

		for box in self.bunkerAreas[self.bunkerIds.frontBunker].totalArea.Boxes do
			SceneMan:RevealUnseenBox(box.Corner.X, box.Corner.Y, box.Width, box.Height, self.humanTeam);
		end

		self.internalReinforcementsData[self.bunkerIds.frontBunker].enabled = false;
	elseif self.currentStage == self.stages.frontBunkerCaptured and self.messageTimer:IsPastSimTimeLimit() then
		self.currentStage = self.stages.deployBrain;
		self.numberOfMessagesForStage = 2;
		self.cameraMinimumX = 4200;

		self.internalReinforcementsData[self.bunkerIds.middleBunker].enabled = true;

		self:SpawnAreaDefinedAIDefenders();

		self.alliedData.spawnTimer:SetSimTimeLimitMS(10000);
		self.alliedData.spawnTimer:Reset();
		self.alliedData.spawnsEnabled = true;
		self.aiData.externalSpawnsEnabled = true;

		for actor in MovableMan.Actors do
			if actor.Team == self.humanTeam and actor.PlayerControllable and not actor:IsPlayerControlled() and not self.bunkerAreas[self.bunkerIds.frontBunker].totalArea:IsInside(actor.Pos) then
				if actor:IsInGroup("Brains") then
					actor:AddAISceneWaypoint(Vector(6060, 1160));
					actor:AddAISceneWaypoint(Vector(6204, 1060));
					actor:AddAISceneWaypoint(Vector(6060, 960));
					actor:AddAISceneWaypoint(self.bunkerRegions["Front Bunker Operations"].totalArea.Center);
				else
					actor:AddAISceneWaypoint(SceneMan:MovePointToGround(self.initialDropShipSpawnArea.FirstBox.Corner, 20, 20));
				end
				actor.AIMode = Actor.AIMODE_GOTO;
			end
		end
	elseif self.currentStage == self.stages.deployBrain and self.anyHumanHasDeployedABrain then
		self.currentStage = self.stages.attackMiddleBunker;
		self.numberOfMessagesForStage = 1;
		self.cameraMinimumX = 3660;

		self.bunkerRegions["Middle Bunker Operations"].enabled = true;
	elseif self.currentStage == self.stages.attackMiddleBunker and self.bunkerRegions["Middle Bunker Operations"].ownerTeam == self.humanTeam then
		self.currentStage = self.stages.middleBunkerCaptured;
		self.cameraMinimumX = 0;

		local groundLevel = SceneMan:MovePointToGround(self.bunkerAreas[self.bunkerIds.middleBunker].lzArea.FirstBox.Corner, 10, 10).Y;
		for box in self.bunkerAreas[self.bunkerIds.middleBunker].totalArea.Boxes do
			if box.Corner.Y + box.Height < groundLevel then
				SceneMan:RevealUnseenBox(box.Corner.X, box.Corner.Y, box.Width, box.Height, self.humanTeam);
			end
		end

		local hiddenTunnelUnblocker = CreateTerrainObject("Empty 1X", "Base.rte");
		hiddenTunnelUnblocker.Pos = SceneMan:SnapPosition(self.hiddenTunnelBlockingCrateArea.FirstBox.Corner, true);
		SceneMan:AddSceneObject(hiddenTunnelUnblocker);

		self:SpawnAreaDefinedAIDefenders();
		self.popoutTurretsData[self.bunkerIds.mainBunker].enabled = true;
		self:UpdateAlliedAttackersWaypoint();
	elseif self.currentStage == self.stages.middleBunkerCaptured and self.messageTimer:IsPastSimTimeLimit() then
		self.currentStage = self.stages.findTunnel;
	elseif self.currentStage == self.stages.findTunnel and self.tunnelHasBeenEntered then
		self.currentStage = self.stages.captureDoorControls;

		self.bunkerRegions["Main Bunker Door Controls"].enabled = true;
		self.bunkerRegions["Main Bunker Small Vault"].enabled = true;
	elseif self.currentStage == self.stages.captureDoorControls and self.bunkerRegions["Main Bunker Door Controls"].ownerTeam == self.humanTeam then
		self.currentStage = self.stages.captureMainBunker;
		self.numberOfMessagesForStage = 2;

		self.internalReinforcementsData[self.bunkerIds.mainBunker].enabled = true;

		self.bunkerRegions["Main Bunker Door Controls"].enabled = false;

		self.bunkerRegions["Main Bunker Security Tower"].enabled = true;
		self.bunkerRegions["Main Bunker Barracks"].enabled = true;
		self.bunkerRegions["Main Bunker Armory"].enabled = true;
		self.bunkerRegions["Main Bunker Air Traffic Control"].enabled = true;
		self.bunkerRegions["Main Bunker Shield Generator"].enabled = true;
		self.bunkerRegions["Main Bunker Medium Vault"].enabled = true;
		self.bunkerRegions["Main Bunker Large Vault"].enabled = true;

		self:DoAlliedSpawns(true);
		self:UpdateAlliedAttackersWaypoint();
		self.alliedData.spawnTimer:SetSimTimeLimitMS(10000);
		self.alliedData.attackerLimit = 5;
	elseif self.currentStage == self.stages.captureMainBunker and self.bunkerRegions["Main Bunker Shield Generator"].ownerTeam == self.humanTeam then
		self.currentStage = self.stages.attackBrain;
		self.numberOfMessagesForStage = 1;

		for movableObject in MovableMan:GetMOsInBox(self.bunkerAreas[self.bunkerIds.mainBunker].brainDoors.FirstBox, self.humanTeam, true) do
			if IsADoor(movableObject) then
				movableObject.ToDelete = true;
			end
		end
		for movableObject in MovableMan:GetMOsInBox(self.bunkerRegions["Main Bunker Command Center"].brainDoor.FirstBox, -1, true) do
			if IsADoor(movableObject) then
				MovableMan:ChangeActorTeam(ToActor(movableObject), self.aiTeam);
				ToADoor(movableObject).Status = Actor.INACTIVE;
				ToADoor(movableObject):CloseDoor();
			end
		end

		self.aiData.brainSpawned = true;
		local aiBrain = CreateActor("Brain Case", "Base.rte");
		aiBrain.Team = self.aiTeam;
		aiBrain.Pos = self.bunkerRegions["Main Bunker Command Center"].brain.Center;
		MovableMan:AddActor(aiBrain);

		self.bunkerRegions["Main Bunker Shield Generator"].enabled = false;
		self.bunkerRegions["Main Bunker Command Center"].enabled = true;
	end

	if previousStage ~= self.currentStage then
		self.messageTimer:Reset();
		self.currentMessageNumber = 1;
	end
end

function DecisionDay:UpdateCamera()
	for _, player in pairs(self.humanPlayers) do
		local adjustedCameraMinimumX = self.cameraMinimumX + (0.5 * (FrameMan.PlayerScreenWidth - 960))
		if CameraMan:GetScrollTarget(player).X < adjustedCameraMinimumX then
			CameraMan:SetScrollTarget(Vector(adjustedCameraMinimumX, CameraMan:GetScrollTarget(player).Y), 0.25, 0);
		end
	end
	
	if self.speedrunData and ActivitySpeedrunHelper.SpeedrunActive(self.speedrunData) then
		return;
	end

	local slowScroll = 0.0125;
	local mediumScroll = 0.05;
	local fastScroll = 0.05;
	local veryFastScroll = 0.075;

	local scrollTargetAndSpeed;
	if self.currentStage <= self.stages.showInitialText then
		if self.currentStage == self.stages.showInitialText and self.messageTimer.SimTimeLimitProgress > 0.75 then
			scrollTargetAndSpeed = {nil, fastScroll};
		else
			local dropShipToFollow = #self.initialDropShipsAndVelocities > 0 and self.initialDropShipsAndVelocities[1].dropShip or nil;
			if dropShipToFollow then
				scrollTargetAndSpeed = {dropShipToFollow.Pos, veryFastScroll};
			else
				scrollTargetAndSpeed = {self.initialDropShipSpawnArea.Center, veryFastScroll};
			end
		end
	elseif self.currentStage == self.stages.frontBunkerCaptured then
		if self.bunkerRegions["Front Bunker Small Vault"].ownerTeam == self.aiTeam and self.messageTimer.SimTimeLimitProgress > 0.25 and self.messageTimer.SimTimeLimitProgress < 0.75 then
			scrollTargetAndSpeed = {self.bunkerRegions["Front Bunker Small Vault"].captureArea.Center, mediumScroll};
		elseif self.messageTimer.SimTimeLimitProgress > 0.5 and self.messageTimer.SimTimeLimitProgress < 1 then
			scrollTargetAndSpeed = {self.bunkerRegions["Front Bunker Operations"].captureArea.Center, mediumScroll};
		end
	elseif self.currentStage == self.stages.deployBrain and self.currentMessageNumber == 1 then
		local groundLevel = SceneMan:MovePointToGround(self.initialDropShipSpawnArea.FirstBox.Corner, 10, 10).Y;
		if self.messageTimer.SimTimeLimitProgress < 0.025 then
			scrollTargetAndSpeed = {Vector(self.initialDropShipSpawnArea.FirstBox.Corner.X, groundLevel), mediumScroll};
		elseif self.messageTimer.SimTimeLimitProgress < 0.5 then
			scrollTargetAndSpeed = {Vector(self.bunkerAreas[self.bunkerIds.middleBunker].totalArea.Center.X + 200, groundLevel), slowScroll};
		elseif self.messageTimer.SimTimeLimitProgress < 0.75 then
			scrollTargetAndSpeed = {self.bunkerAreas[self.bunkerIds.middleBunker].totalArea.Center + Vector(200, -600), slowScroll};
		elseif self.messageTimer.SimTimeLimitProgress < 1 then
			scrollTargetAndSpeed = {self.bunkerAreas[self.bunkerIds.frontBunker].totalArea.Center, mediumScroll};
		end
	elseif self.currentStage == self.stages.middleBunkerCaptured then
		local groundLevel = SceneMan:MovePointToGround(self.initialDropShipSpawnArea.FirstBox.Corner, 10, 10).Y;
		if self.messageTimer.SimTimeLimitProgress < 0.25 then
			scrollTargetAndSpeed = {Vector(2630, groundLevel), slowScroll};
		elseif self.messageTimer.SimTimeLimitProgress < 0.5 then
			scrollTargetAndSpeed = {Vector(2630, groundLevel - 600), slowScroll};
		elseif self.messageTimer.SimTimeLimitProgress < 0.75 then
			scrollTargetAndSpeed = {Vector(1980, groundLevel - 200), slowScroll};
		elseif self.messageTimer.SimTimeLimitProgress < 1 then
			scrollTargetAndSpeed = {self.bunkerRegions["Middle Bunker Operations"].totalArea.Center, mediumScroll};
		end
	elseif self.currentStage == self.stages.captureMainBunker then
		if self.messageTimer.SimTimeLimitProgress < 1 then
			local cameraTargets = {
				self.bunkerRegions["Main Bunker Door Controls"],
				self.bunkerRegions["Main Bunker Small Vault"],
				self.bunkerRegions["Main Bunker Armory"],
				self.bunkerRegions["Main Bunker Shield Generator"],
				self.bunkerRegions["Main Bunker Large Vault"],
				self.bunkerRegions["Main Bunker Medium Vault"],
				self.bunkerRegions["Main Bunker Barracks"],
				self.bunkerRegions["Main Bunker Air Traffic Control"],
				self.bunkerRegions["Main Bunker Security Tower"],
				self.bunkerRegions["Main Bunker Door Controls"]
			};

			local currentCameraTarget;
			if self.currentMessageNumber == 1 then
				if self.messageTimer.SimTimeLimitProgress < 0.05 then
					currentCameraTarget = 1;
				elseif self.messageTimer.SimTimeLimitProgress < 0.25 then
					currentCameraTarget = 2;
				elseif self.messageTimer.SimTimeLimitProgress < 0.5 then
					currentCameraTarget = 3;
				elseif self.messageTimer.SimTimeLimitProgress < 0.75 then
					currentCameraTarget = 4;
				elseif self.messageTimer.SimTimeLimitProgress < 1 then
					currentCameraTarget = 5;
				end
			elseif self.currentMessageNumber == 2 then
				if self.messageTimer.SimTimeLimitProgress < 0.15 then
					currentCameraTarget = 6;
				elseif self.messageTimer.SimTimeLimitProgress < 0.4 then
					currentCameraTarget = 7;
				elseif self.messageTimer.SimTimeLimitProgress < 0.65 then
					currentCameraTarget = 8;
				elseif self.messageTimer.SimTimeLimitProgress < 0.9 then
					currentCameraTarget = 9;
				elseif self.messageTimer.SimTimeLimitProgress < 1 then
					currentCameraTarget = 10;
				end
			end
			scrollTargetAndSpeed = {cameraTargets[currentCameraTarget].captureArea.FirstBox.Center, slowScroll};
		end
	elseif self.currentStage == self.stages.attackBrain then
		if self.messageTimer.SimTimeLimitProgress < 0.05 then
			scrollTargetAndSpeed = {self.bunkerRegions["Main Bunker Shield Generator"].totalArea.Center, mediumScroll};
		elseif self.messageTimer.SimTimeLimitProgress < 0.25 then
			scrollTargetAndSpeed = {self.bunkerRegions["Main Bunker Command Center"].totalArea.Center, mediumScroll};
		elseif self.messageTimer.SimTimeLimitProgress < 0.5 then
			scrollTargetAndSpeed = {self.bunkerRegions["Main Bunker Shield Generator"].totalArea.Center, mediumScroll};
		end
	end

	if scrollTargetAndSpeed then
		for _, player in pairs(self.humanPlayers) do
			if not scrollTargetAndSpeed[1] then
				brain = self:GetPlayerBrain(player)
				if brain then
					CameraMan:SetScrollTarget(brain.pos, scrollTargetAndSpeed[2], player)
				end
			else
				CameraMan:SetScrollTarget(scrollTargetAndSpeed[1], scrollTargetAndSpeed[2], player);
			end
		end
	end
	self.cameraIsPanning = scrollTargetAndSpeed ~= nil;
end

function DecisionDay:UpdateMessages()
	if self.speedrunData and ActivitySpeedrunHelper.SpeedrunActive(self.speedrunData) then
		FrameMan:ClearScreenText(0);
		FrameMan:SetScreenText(ActivitySpeedrunHelper.GetSpeedrunDuration(self.speedrunData), 0, 0, 1, ActivitySpeedrunHelper.SpeedrunCompleted(self.speedrunData));
		return;
	end

	if self.messageTimer:IsPastSimTimeLimit() then
		if self.currentMessageNumber < self.numberOfMessagesForStage then
			self.currentMessageNumber = self.currentMessageNumber + 1;
			self.messageTimer:Reset();
		else
			return;
		end
	end

	local brainString = #self.humanPlayers == 1 and "brain" or "brains";
	for _, player in pairs(self.humanPlayers) do
		local messageText;
		local textCentered = true;
		local blinkTime = 0;

		if self.WinnerTeam == self.humanTeam then
			messageText = "Finally, we have revenge for the loss of our Maginot bunker. With the loss of this fortress, the enemy is in tatters, we can crush them with ease."
			if self.messageTimer.SimTimeLimitProgress > 0.25 then
				messageText = messageText .. "\nYour outstanding contributions have been noted, we'll see to it that you receive a hefty year-end performance bonus!";
			end
		elseif self.WinnerTeam == self.aiTeam then
			messageText = "With the loss of your brain, the assault has failed catastrophically. Your faction now faces complete destruction at the hands of your enemies.";
		elseif self.currentStage == self.stages.showInitialText then
			messageText = "Good to see your forces have made it over land. As you can see, this installation is impregnable by air thanks to their EMP defenses."
			if self.messageTimer.SimTimeLimitProgress > 0.25 then
				messageText = messageText .. "\nGet into the bunker and disable the AA defenses so we can land our forces and begin the real assault!";
			end
		elseif self.currentStage == self.stages.attackFrontBunker and not self.anyHumanHasSeenObjectives then
			messageText = "Enter actor select mode to see objectives!";
			blinkTime = 1000;
		elseif self.currentStage == self.stages.frontBunkerCaptured then
			messageText = "Excellent work! With these anti-air defences disabled, we can start our attack in earnest.\n";
			if self.messageTimer.SimTimeLimitProgress > 0.25 then
				if self.bunkerRegions["Front Bunker Small Vault"].ownerTeam == self.aiTeam then
					messageText = messageText .. "We'll begin the assault shortly, capture the vault to get some funds, and deploy your " .. brainString .. " in the Operations Center so you can assist us!";
				else
					messageText = messageText .. "We'll begin the assault shortly, deploy your " .. brainString .. " in the Operations Center so you can assist us!";
				end
			end
		elseif self.currentStage == self.stages.deployBrain then
			if self.currentMessageNumber == 1 then
				if self.messageTimer.SimTimeLimitProgress > 0.1 then
					messageText = "As you can see, there's another small bunker to deal with before we can assault the fortress proper. Our forces should make short work of it.";
				end
				if self.messageTimer.SimTimeLimitProgress > 0.35 then
					messageText = messageText .. "\nOnce your brain is deployed, you're welcome to take control of some of our forces while you build up your own.";
				end
			elseif self.currentMessageNumber == 2 then
				messageText = "It should be noted that sub-surface scans indicate that this area is devoid of gold, it seems that every ounce of it has been dug up and stored in vaults.";
				if self.messageTimer.SimTimeLimitProgress > 0.25 then
					messageText = messageText .. "\nYou'll have to rely on capturing these vaults for income. As a bonus, every vault we capture will also harm our enemy's reinforcement capabilities.";
				end
			end
		elseif self.currentStage == self.stages.attackMiddleBunker then
			messageText = "Excellent, with your brain deployed you can contribute to the assault."
			if self.messageTimer.SimTimeLimitProgress > 0.25 then
				messageText = messageText .. "\nOur troops will push straight to the main bunker, you should follow along behind us and capture the middle bunker's operations room.";
			end
		elseif self.currentStage == self.stages.middleBunkerCaptured then
			messageText = "Good job capturing the middle bunker, it'll let us land troops closer and focus on the main bunker."
			if self.messageTimer.SimTimeLimitProgress > 0.25 then
				messageText = messageText .. "\nUnfortunately, we've got our work cut out for us, the main bunker is incredibly well defended.";
			end
		elseif self.currentStage == self.stages.findTunnel then
			messageText = "Sub-surface scans suggest there's an old mining tunnel that you can use to infiltrate the main bunker and help us from the inside.";
			if self.messageTimer.SimTimeLimitProgress > 0.25 then
				messageText = messageText .. "\nLook around below the bottom of the middle bunker and find a way in. Our forces will keep them locked down so they don't retake our territory, but move quickly!";
			end
		elseif self.currentStage == self.stages.captureDoorControls then
			messageText = "Excellent, you found the tunnel and it looks abandoned! With any luck, this will get you straight to the main bunker.";
			if self.messageTimer.SimTimeLimitProgress > 0.25 then
				messageText = messageText .. "\nGet in there and capture the door and turret controls as soon as possible, our forces are dwindling.";
			end
		elseif self.currentStage == self.stages.captureMainBunker then
			if self.currentMessageNumber == 1 then
				messageText = "Excellent work, and just in time, we've almost run out of reinforcements.";
				if self.messageTimer.SimTimeLimitProgress > 0.25 then
					messageText = messageText .. "\nWe'll use our remaining forces to hold the door controls and the other bunkers - buy more troops and take over the bunker, sector-by-sector.";
				end
			elseif self.currentMessageNumber == 2 then
				messageText = "Your main goal is to capture the shield generator so you can get access to the brain vault and destroy the enemy brain.";
				if self.messageTimer.SimTimeLimitProgress > 0.25 then
					messageText = messageText .. "\nThat said, every region you capture will damage the enemy's capabilities, so it might be wise to capture as much as you can.";
				end
			end
		elseif self.currentStage == self.stages.attackBrain then
			messageText = "Great work, the shield generator is disabled. This is the last stretch, get your forces to the brain vault and finish the fight!";
		end

		if messageText then
			FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
			FrameMan:SetScreenText(messageText, self:ScreenOfPlayer(player), blinkTime, 0, textCentered);
		end
	end
end

function DecisionDay:SpawnAndUpdateInitialDropShips()
	if self.currentStage < self.stages.attackFrontBunker and self.alliedData.spawnTimer:IsPastSimTimeLimit() then
		local craft = RandomACDropShip("Craft", self.humanTeamTech);
		if not craft or craft.MaxInventoryMass <= 0 then
			craft = RandomACDropShip("Craft", "Base.rte");
		end

		craft.Pos = Vector(self.initialDropShipSpawnArea:GetRandomPoint().X, -50);
		craft.Team = self.humanTeam;
		craft.PlayerControllable = false;
		craft.ImpulseDamageThreshold = 1;
		for i = 1, 2 do
			local actor = RandomAHuman("Actors", self.humanTeamTech);
			actor.Team = self.humanTeam;
			actor.Health = 0;
			craft:AddInventoryItem(actor);
		end
		craft.GlobalAccScalar = 1.75;
		MovableMan:AddActor(craft);
		self.initialDropShipsAndVelocities[#self.initialDropShipsAndVelocities + 1] = { dropShip = craft, velX = math.random(-5, 5) };

		self.alliedData.spawnTimer:Reset();
	end

	for i = #self.initialDropShipsAndVelocities, 1, -1 do
		local initialDropShipAndVelocity = self.initialDropShipsAndVelocities[i];
		if not MovableMan:ValidMO(initialDropShipAndVelocity.dropShip) then
			table.remove(self.initialDropShipsAndVelocities, i);
			if i == 1 and not self.initialDropShipDestroyed then
				self.initialDropShipDestroyed = true;
			end
		else
			initialDropShipAndVelocity.dropShip.Vel.X = initialDropShipAndVelocity.velX;
			if initialDropShipAndVelocity.dropShip.TravelImpulse.Magnitude > 10 or initialDropShipAndVelocity.dropShip.Age > 100000 then
				initialDropShipAndVelocity.dropShip:GibThis();
			end
		end
	end
end

function DecisionDay:UpdateObjectiveArrowsAndRegionVisuals()
	local showStrategicVisuals = false;
	for _, player in pairs(self.humanPlayers) do
		if self:GetViewState(player) == Activity.ACTORSELECT then
			showStrategicVisuals = true;
			break;
		end
	end

	self:ClearObjectivePoints();

	if showStrategicVisuals or self.cameraIsPanning then
		self.anyHumanHasSeenObjectives = true;

		if self.currentStage == self.stages.deployBrain then
			self:AddObjectivePoint("Use the Pie Menu and deploy your brain to join the assault", self.bunkerRegions["Front Bunker Operations"].totalArea.Center, self.humanTeam, GameActivity.ARROWDOWN);
		elseif self.currentStage == self.stages.findTunnel then
			self:AddObjectivePoint("Find and enter the abandoned tunnel", self.bunkerAreas[self.bunkerIds.middleBunker].totalArea.Center + Vector(84, 185), self.humanTeam, GameActivity.ARROWDOWN);
		end

		for bunkerRegionName, bunkerRegionData in pairs(self.bunkerRegions) do
			if bunkerRegionData.enabled then
				if bunkerRegionData.ownerTeam ~= self.humanTeam then
					local objectiveString = bunkerRegionData.hasBeenCapturedAtLeastOnceByHumanTeam and "Recapture " or "Capture ";
					if bunkerRegionName:find("Operations") then
						objectiveString = objectiveString .. "to secure the bunker";
					elseif bunkerRegionName:find("Vault") then
						objectiveString = objectiveString .. "to gain funds";
					elseif bunkerRegionName:find("Door Controls") then
						objectiveString = objectiveString .. "to breach the bunker";
					elseif bunkerRegionName:find("Security Tower") then
						objectiveString = objectiveString .. "to access cameras and turrets";
					elseif bunkerRegionName:find("Barracks") then
						objectiveString = objectiveString .. "to slow enemy internal reinforcements";
						if self.currentStage == self.stages.attackBrain then
							objectiveString = objectiveString .. " and command center defenders";
						end
					elseif bunkerRegionName:find("Armory") then
						objectiveString = objectiveString .. "to limit enemy equipment";
					elseif bunkerRegionName:find("Air Traffic Control") then
						objectiveString = objectiveString .. "to control the LZ";
					elseif bunkerRegionName:find("Shield Generator") then
						objectiveString = objectiveString .. "to reach the enemy brain";
					elseif bunkerRegionName:find("Command Center") then
						objectiveString = objectiveString .. "to destroy the enemy brain";
						if self.aiData.brainDefendersRemaining > 0 then
							local reinforcementsRemainString = self.aiData.brainDefendersRemaining == 1 and " reinforcement remains" or " reinforcements remain";
							objectiveString = objectiveString .. "\n" .. tostring(self.aiData.brainDefendersRemaining) .. reinforcementsRemainString;
						end
					end
					self:AddObjectivePoint(objectiveString, bunkerRegionData.captureArea.Center - Vector(0, 30), self.humanTeam, GameActivity.ARROWDOWN);
				end

				for _, player in pairs(self.humanPlayers) do
					if self:GetViewState(player) == Activity.ACTORSELECT then
						if math.abs((bunkerRegionData.totalArea.Center - CameraMan:GetScrollTarget(player)).X) < FrameMan.PlayerScreenWidth * 0.75 then
							local boxFillPrimitives = {};
							for box in bunkerRegionData.totalArea.Boxes do
								boxFillPrimitives[#boxFillPrimitives + 1] = BoxFillPrimitive(player, box.Corner, box.Corner + Vector(box.Width, box.Height), bunkerRegionData.ownerTeam == self.humanTeam and 147 or 13);
							end
							PrimitiveMan:DrawPrimitives(75, boxFillPrimitives);

							if bunkerRegionData.ownerTeam == self.humanTeam then
								local capturedRegionDescription = "This region is ";
								if bunkerRegionName:find("Operations") then
									capturedRegionDescription = capturedRegionDescription .. "keeping the bunker and LZ secured";
								elseif bunkerRegionName:find("Vault") then
									capturedRegionDescription = capturedRegionDescription .. "providing you with funds";
								elseif bunkerRegionName:find("Door Controls") then
									capturedRegionDescription = capturedRegionDescription .. "giving you control of the front doors"
								elseif bunkerRegionName:find("Security Tower") then
									capturedRegionDescription = capturedRegionDescription .. "giving you control of the cameras and turrets"
								elseif bunkerRegionName:find("Barracks") then
									capturedRegionDescription = capturedRegionDescription .. "slowing enemy internal reinforcements"
								elseif bunkerRegionName:find("Armory") then
									capturedRegionDescription = capturedRegionDescription .. "limiting enemy weapons"
								elseif bunkerRegionName:find("Air Traffic Control") then
									capturedRegionDescription = capturedRegionDescription .. "giving you control of the LZ"
								elseif bunkerRegionName:find("Shield Generator") then
									capturedRegionDescription = capturedRegionDescription .. "giving you access to the enemy brain"
								end
								PrimitiveMan:DrawTextPrimitive(player, bunkerRegionData.totalArea.Center, capturedRegionDescription, false, 1);
							end
						end
					end
				end
			end
		end

		self:YSortObjectivePoints();
	end
end

function DecisionDay:UpdateLZAreas()
	if self.bunkerRegions["Front Bunker Operations"].ownerTeam == self.humanTeam then
		self.alliedData.lzArea = self.bunkerAreas[self.bunkerIds.frontBunker].lzArea;
		self.aiData.lzArea = self.bunkerAreas[self.bunkerIds.middleBunker].lzArea;
	end

	if self.bunkerRegions["Middle Bunker Operations"].ownerTeam == self.humanTeam then
		self.alliedData.lzArea = self.bunkerAreas[self.bunkerIds.middleBunker].lzArea;
		self.aiData.lzArea = self.bunkerAreas[self.bunkerIds.mainBunker].lzArea;
	end

	if self.bunkerRegions["Main Bunker Air Traffic Control"].ownerTeam == self.humanTeam then
		self.alliedData.lzArea = self.bunkerAreas[self.bunkerIds.mainBunker].lzArea;
		self.aiData.lzArea = self.bunkerAreas[self.bunkerIds.mainBunker].rearLZArea;
	end

	self:SetLZArea(self.humanTeam, self.alliedData.lzArea);
end

function DecisionDay:UpdateRegionCapturing()
	for bunkerRegionName, bunkerRegionData in pairs(self.bunkerRegions) do
		if bunkerRegionData.enabled then
			local captureBox = bunkerRegionData.captureArea.FirstBox;
			if captureBox then
				local capturingTeam = bunkerRegionData.ownerTeam == self.aiTeam and self.humanTeam or self.aiTeam;
				local capturingTeamActorInArea = false;
				for movableObject in MovableMan:GetMOsInBox(captureBox, bunkerRegionData.ownerTeam, true) do
					if (IsAHuman(movableObject) or IsACrab(movableObject)) and ToActor(movableObject).Health > 0 then
						capturingTeamActorInArea = true;
						break;
					end
				end

				local ownerTeamActorInTotalAreaBlockingCapture = false;
				if capturingTeamActorInArea then
					for captureBlockingBox in bunkerRegionData.totalArea.Boxes do
						for movableObject in MovableMan:GetMOsInBox(captureBlockingBox, capturingTeam, true) do
							if (IsAHuman(movableObject) or IsACrab(movableObject)) and ToActor(movableObject).Health > 0 then
								ownerTeamActorInTotalAreaBlockingCapture = true;
								break;
							end
						end
						if ownerTeamActorInTotalAreaBlockingCapture then
							break;
						end
					end
				end

				if bunkerRegionData.captureCount > 0 or (capturingTeamActorInArea and not ownerTeamActorInTotalAreaBlockingCapture) then
					bunkerRegionData.captureCount = bunkerRegionData.captureCount + (capturingTeamActorInArea and 1 or -1);

					if bunkerRegionData.captureCount >= bunkerRegionData.captureLimit then
						bunkerRegionData.ownerTeam = capturingTeam;
						bunkerRegionData.captureCount = 0;

						for _, captureDisplayScreen in ipairs(bunkerRegionData.captureDisplayScreens) do
							captureDisplayScreen.Lifetime = 2000;
						end

						local useLeftReplacementComputer = captureBox.Center.X >= bunkerRegionData.totalArea.Center.X;
						local replacementComputer = self.controlledBunkerRegionComputerTerrainObjects[capturingTeam][useLeftReplacementComputer and "left" or "right"]:Clone();
						replacementComputer.Pos = SceneMan:SnapPosition(captureBox.Center, true);
						SceneMan:AddSceneObject(replacementComputer);

						for box in bunkerRegionData.totalArea.Boxes do
							SceneMan:RevealUnseenBox(box.Corner.X, box.Corner.Y, box.Width, box.Height, capturingTeam);
						end

						-- Armory and Shield Generator are handled elsewhere.
						if bunkerRegionName:find("Operations") or bunkerRegionName:find("Air Traffic Control") then
							self:UpdateLZAreas();
						elseif bunkerRegionName == "Main Bunker Door Controls" then
							for movableObject in MovableMan:GetMOsInBox(self.bunkerAreas[self.bunkerIds.mainBunker].frontDoors.FirstBox, capturingTeam, true) do
								if IsADoor(movableObject) then
									local movableObjectAsADoor = ToADoor(movableObject);
									MovableMan:ChangeActorTeam(movableObjectAsADoor, bunkerRegionData.ownerTeam);
									if bunkerRegionData.ownerTeam == self.humanTeam then
										movableObjectAsADoor.Status = Actor.INACTIVE;
										movableObjectAsADoor:OpenDoor();
									else
										movableObjectAsADoor.Status = Actor.STABLE;
									end
								end
							end
							for box, boxData in pairs(self.popoutTurretsData[bunkerRegionData.bunkerId].boxData) do
								if boxData.actor and MovableMan:ValidMO(boxData.actor) then
									boxData.actor:GibThis();
								end
							end
						elseif bunkerRegionName:find("Vault") and bunkerRegionData.ownerTeam == self.humanTeam and not bunkerRegionData.hasBeenCapturedAtLeastOnceByHumanTeam then
							self:ChangeTeamFunds(self.vaultCaptureIncome * bunkerRegionData.incomeMultiplier / self.difficultyRatio, self.humanTeam);
						elseif bunkerRegionName:find("Security Tower") then
							if bunkerRegionData.ownerTeam == self.humanTeam and not bunkerRegionData.hasBeenCapturedAtLeastOnceByHumanTeam then
								for box in self.bunkerAreas[self.bunkerIds.mainBunker].totalArea.Boxes do
									SceneMan:RevealUnseenBox(box.Corner.X, box.Corner.Y, box.Width, box.Height, self.humanTeam);
								end
								if self.bunkerRegions["Main Bunker Shield Generator"].ownerTeam == self.aiTeam then
									self.mainBunkerShieldedAreaFOWTimer.ElapsedSimTimeMS = self.mainBunkerShieldedAreaFOWTimer:GetSimTimeLimitMS();
								end
							end
							self:UpdateAndCleanupDataTables(bunkerRegionData.ownerTeam == self.humanTeam and self.aiTeam or self.humanTeam);
							for key, actor in pairs(self.aiData.actors.internalTurrets) do
								if key ~= "count" then
									MovableMan:ChangeActorTeam(actor, bunkerRegionData.ownerTeam);
								end
							end
						elseif bunkerRegionName:find("Barracks") then
							self.aiData.internalReinforcementsTimer:SetSimTimeLimitMS(self.aiData.internalReinforcementsTimer:GetSimTimeLimitMS() * (bunkerRegionData.ownerTeam == self.humanTeam and 2 or 0.5));
						elseif bunkerRegionName:find("Command Center") then
							for movableObject in MovableMan:GetMOsInBox(bunkerRegionData.brainDoor.FirstBox, -1, true) do
								if IsADoor(movableObject) then
									if bunkerRegionData.ownerTeam == self.humanTeam then
										ToADoor(movableObject):OpenDoor();
									else
										ToADoor(movableObject):CloseDoor();
									end
								end
							end
						end

						bunkerRegionData.hasBeenCapturedAtLeastOnceByHumanTeam = bunkerRegionData.hasBeenCapturedAtLeastOnceByHumanTeam or bunkerRegionData.ownerTeam == self.humanTeam;
					end
				end
			end
		end
	end
end

function DecisionDay:UpdateRegionScreens()
	for bunkerRegionName, bunkerRegionData in pairs(self.bunkerRegions) do
		if bunkerRegionData.enabled then
			local currentFauxdanDisplayFrameString;
			local currentLoginScreenFrameString;
			
			if bunkerRegionData.fauxdanDisplayArea ~= nil and bunkerRegionData.ownerTeam == self.aiTeam and self.currentStage <= self.stages.attackBrain then
				for box in bunkerRegionData.fauxdanDisplayArea.Boxes do
					local boxCenterPos = box.Center;
					local fauxdanDisplayScreenKey = tostring(boxCenterPos.FlooredX) .. "," .. tostring(boxCenterPos.FlooredY);

					local boxBlockedByCaptureDisplay = false;
					if bunkerRegionData.captureCount > 0 then
						for captureDisplayBox in bunkerRegionData.captureDisplayArea.Boxes do
							if captureDisplayBox.Center.Floored == boxCenterPos.Floored then
								boxBlockedByCaptureDisplay = true;
								break;
							end
						end
					end

					if not boxBlockedByCaptureDisplay and bunkerRegionData.fauxdanDisplayScreens[fauxdanDisplayScreenKey] == nil then
						local fauxdanDisplayScreen = self.fauxdanDisplayScreenTemplate:Clone();
						fauxdanDisplayScreen.Pos = boxCenterPos;
						MovableMan:AddParticle(fauxdanDisplayScreen);
						bunkerRegionData.fauxdanDisplayScreens[fauxdanDisplayScreenKey] = fauxdanDisplayScreen;
					elseif boxBlockedByCaptureDisplay and bunkerRegionData.fauxdanDisplayScreens[fauxdanDisplayScreenKey] ~= nil then
						bunkerRegionData.fauxdanDisplayScreens[fauxdanDisplayScreenKey].ToDelete = true;
						bunkerRegionData.fauxdanDisplayScreens[fauxdanDisplayScreenKey] = nil;
					end
				end
			elseif next(bunkerRegionData.fauxdanDisplayScreens) ~= nil then
				for _, fauxdanDisplayScreen in pairs(bunkerRegionData.fauxdanDisplayScreens) do
					fauxdanDisplayScreen.ToDelete = true;
				end
				bunkerRegionData.fauxdanDisplayScreens = {};
			end

			if bunkerRegionData.captureCount > 0 then
				if #bunkerRegionData.captureDisplayScreens == 0 then
					for box in bunkerRegionData.captureDisplayArea.Boxes do
						local captureDisplayScreen = self.captureDisplayScreenTemplate:Clone();
						captureDisplayScreen.Pos = box.Center;
						MovableMan:AddParticle(captureDisplayScreen);
						bunkerRegionData.captureDisplayScreens[#bunkerRegionData.captureDisplayScreens + 1] = captureDisplayScreen;
					end
				end
				for index, captureDisplayScreen in ipairs(bunkerRegionData.captureDisplayScreens) do
					if MovableMan:ValidMO(captureDisplayScreen) then
						captureDisplayScreen.Frame = math.floor((bunkerRegionData.captureCount / bunkerRegionData.captureLimit) * (captureDisplayScreen.FrameCount));
						captureDisplayScreen.Age = 0;
					end
				end
			elseif #bunkerRegionData.captureDisplayScreens > 0 then
				bunkerRegionData.captureDisplayScreens = {};
			end
		end
	end
end

function DecisionDay:UpdateVaultTickIncome()
	if self.vaultIncomeTimer:IsPastSimTimeLimit() then
		self:ChangeTeamFunds(self.vaultTickIncome, self.aiTeam); -- Note: AI always gets one free vault worth of income, to keep their external spawns coming.
		for bunkerRegionName, bunkerRegionData in pairs(self.bunkerRegions) do
			if bunkerRegionName:find("Vault") and bunkerRegionData.enabled then
				local vaultTickIncome = self.vaultTickIncome * bunkerRegionData.incomeMultiplier;
				if bunkerRegionData.ownerTeam == self.humanTeam then
					vaultTickIncome = vaultTickIncome / self.difficultyRatio;
				end
				self:SetTeamFunds(self:GetTeamFunds(bunkerRegionData.ownerTeam) + vaultTickIncome, bunkerRegionData.ownerTeam); -- Note: Not using ChangeTeamFunds here cause it keeps playing noise, which is a pain for steady income like this.
			end
		end

		self.vaultIncomeTimer:Reset();
	end
end

function DecisionDay:UpdateAndCleanupDataTables(teamToCleanup)
	local cleanupDataTables = function(actorsSupertable)
		for actorsTableName, actorsTable in pairs(actorsSupertable) do
			for key, actor in pairs(actorsTable) do
				if key ~= "count" and actor.UniqueID ~= key then
					actorsTable[key] = nil;
					actorsTable.count = actorsTable.count - 1;
				end
			end
		end
	end

	local bunkerOperationsOwnedByAITeam = function(self, bunkerId)
		if bunkerId == self.bunkerIds.frontBunker then
			return self.bunkerRegions["Front Bunker Operations"].ownerTeam == self.aiTeam;
		elseif bunkerId == self.bunkerIds.middleBunker then
			return self.bunkerRegions["Middle Bunker Operations"].ownerTeam == self.aiTeam;
		end
		return true;
	end

	if teamToCleanup ~= self.aiTeam then
		cleanupDataTables(self.alliedData.actors);
	end
	if teamToCleanup ~= self.humanTeam then
		cleanupDataTables(self.aiData.actors);

		for _, bunkerId in pairs(self.bunkerIds) do
			self.aiData.enemiesInsideBunkers[bunkerId] = {};
			if self.internalReinforcementsData[bunkerId].enabled and bunkerOperationsOwnedByAITeam(self, bunkerId) then
				for bunkerBox in self.bunkerAreas[bunkerId].totalArea.Boxes do
					for movableObject in MovableMan:GetMOsInBox(bunkerBox, self.aiTeam, true) do
						if IsAHuman(movableObject) or IsACrab(movableObject) then
							self.aiData.enemiesInsideBunkers[bunkerId][#self.aiData.enemiesInsideBunkers[bunkerId] + 1] = ToActor(movableObject);
						end
					end
				end
			end
		end
	end
end

function DecisionDay:UpdateAIInternalReinforcements(forceInstantSpawning)
	for index, internalReinforcementPositionsCalculationCoroutine in ipairs(self.aiData.internalReinforcementPositionsCalculationCoroutines) do
		local _, internalReinforcementPositionsToEnemyTargets, maxNumberOfInternalReinforcementsToCreate, maxFundsForInternalReinforcements = coroutine.resume(internalReinforcementPositionsCalculationCoroutine, self);

		if coroutine.status(internalReinforcementPositionsCalculationCoroutine) == "dead" then
			table.remove(self.aiData.internalReinforcementPositionsCalculationCoroutines, index);
			local numberOfReinforcementsCreated, remainingReinforcementFunds = self:CreateInternalReinforcements("CQB", internalReinforcementPositionsToEnemyTargets, maxNumberOfInternalReinforcementsToCreate - self.aiData.numberOfInternalReinforcementsCreated, maxFundsForInternalReinforcements);
			self.aiData.numberOfInternalReinforcementsCreated = self.aiData.numberOfInternalReinforcementsCreated + numberOfReinforcementsCreated;
			maxFundsForInternalReinforcements = remainingReinforcementFunds;

			if #self.aiData.internalReinforcementPositionsCalculationCoroutines == 0 and (self.aiData.numberOfInternalReinforcementsCreated / maxNumberOfInternalReinforcementsToCreate < 0.5) then
				self.aiData.internalReinforcementsTimer.ElapsedSimTimeMS = self.aiData.internalReinforcementsTimer:GetSimTimeLimitMS() * 0.25;
			end
		end
	end

	if self.aiData.internalReinforcementsTimer:IsPastSimTimeLimit() and #self.aiData.internalReinforcementPositionsCalculationCoroutines == 0 then
		self:UpdateAndCleanupDataTables(self.aiTeam);
		self.aiData.numberOfInternalReinforcementsCreated = 0;

		if self.aiData.actors.internalReinforcements.count < self.aiData.internalReinforcementLimit then
			for _, bunkerId in pairs(self.bunkerIds) do
				if self.internalReinforcementsData[bunkerId].enabled then
					local maxNumberOfInternalReinforcementsToCreate = math.ceil(self.difficultyRatio * 2 * bunkerId * RangeRand(0.5, 1.5));
					local maxFundsForInternalReinforcements = math.ceil(self.difficultyRatio * 350 * bunkerId * RangeRand(0.75, 1.25));
					if self.internalReinforcementsData[bunkerId].enabled and #self.aiData.enemiesInsideBunkers[bunkerId] > 0 then
						table.insert(self.aiData.internalReinforcementPositionsCalculationCoroutines, coroutine.create(self.CalculateInternalReinforcementPositionsToEnemyTargets));
						coroutine.resume(self.aiData.internalReinforcementPositionsCalculationCoroutines[#self.aiData.internalReinforcementPositionsCalculationCoroutines], self, bunkerId, maxNumberOfInternalReinforcementsToCreate, maxFundsForInternalReinforcements);
					end
				end
			end
			self.aiData.internalReinforcementsTimer:Reset();
		else
			self.aiData.internalReinforcementsTimer.ElapsedSimTimeMS = self.aiData.internalReinforcementsTimer:GetSimTimeLimitMS() * 0.5;
		end
	end

	for internalReinforcementDoor, actorsToSpawn in pairs(self.internalReinforcementsData.doorsAndActorsToSpawn) do
		if MovableMan:ValidMO(internalReinforcementDoor) and (forceInstantSpawning or internalReinforcementDoor.Frame == internalReinforcementDoor.FrameCount - 1) then
			for _, actorToSpawn in pairs(actorsToSpawn) do
				actorToSpawn.Team = internalReinforcementDoor.Team;
				actorToSpawn:AddToGroup("AI Internal Reinforcements");
				self.aiData.actors.internalReinforcements[actorToSpawn.UniqueID] = actorToSpawn;
				self.aiData.actors.internalReinforcements.count = self.aiData.actors.internalReinforcements.count + 1;
				MovableMan:AddActor(actorToSpawn);
			end
			self.internalReinforcementsData.doorsAndActorsToSpawn[internalReinforcementDoor] = nil;
		end
	end
end

function DecisionDay:UpdateAIDecisions()
	local bunkerRegionForAIToRecapture;
	local aiOwnedBunkerRegions = {};
	local humanOwnedBunkerRegions = {};

	for bunkerRegionName, bunkerRegionData in pairs(self.bunkerRegions) do
		if bunkerRegionData.enabled then
			if bunkerRegionData.ownerTeam == self.aiTeam then
				aiOwnedBunkerRegions[#aiOwnedBunkerRegions + 1] = bunkerRegionData;
				if bunkerRegionData.captureCount > 0 and bunkerRegionData.aiRegionDefenseTimer:IsPastSimTimeLimit() then
					local captureAreaCenter = bunkerRegionData.captureArea.Center;

					for movableObject in MovableMan:GetMOsInRadius(captureAreaCenter, self.aiData.bunkerRegionDefenseRange, self.humanTeam, true) do
						if (IsAHuman(movableObject) or IsACrab(movableObject)) and (not movableObject:IsInGroup("AI Region Defenders") or movableObject:IsInGroup("AI Region Defenders - " .. bunkerRegionName)) and movableObject.PinStrength == 0 and not movableObject:IsInGroup("Actors - Turrets")  then
							--TODO when we have calculate path async with limited max path length, use it here. Will have to do coroutine, etc.
							--local pathLengthToCaptureArea = SceneMan.Scene:CalculatePath(movableObject.Pos, captureAreaCenter, false, GetPathFindingDefaultDigStrength(), self.aiTeam) * 20;
							--if pathLengthToCaptureArea < self.aiData.bunkerRegionDefenseRange then
								local actor = ToActor(movableObject);
								actor.AIMode = Actor.AIMODE_GOTO;
								actor:AddAISceneWaypoint(captureAreaCenter);
							--end
						end
					end
					if self.aiData.internalReinforcementsEnabled and bunkerRegionData.internalReinforcementsArea and self.internalReinforcementsData[bunkerRegionData.bunkerId].enabled then
						local internalReinforcementPositionsToEnemyTargets = {};
						for box in bunkerRegionData.internalReinforcementsArea.Boxes do
							for _, internalReinforcementPosition in pairs(self.internalReinforcementsData[bunkerRegionData.bunkerId].positions) do
								if box:IsWithinBox(internalReinforcementPosition) then
									internalReinforcementPositionsToEnemyTargets[internalReinforcementPosition] = {};
									for i = 1, 5 do
										table.insert(internalReinforcementPositionsToEnemyTargets[internalReinforcementPosition], captureAreaCenter);
									end
									break;
								end
							end
						end
						self:CreateInternalReinforcements(math.random() < self.difficultyRatio * 0.75 and "CQB" or "Light", internalReinforcementPositionsToEnemyTargets, nil, 450 * self.difficultyRatio);
					end

					bunkerRegionData.aiRegionDefenseTimer:Reset();
				end
			elseif bunkerRegionData.ownerTeam == self.humanTeam then
				bunkerRegionData.aiRegionDefenseTimer:Reset();

				humanOwnedBunkerRegions[#humanOwnedBunkerRegions + 1] = bunkerRegionData;
				if bunkerRegionData.aiRecaptureWeight > 0 and (bunkerRegionForAIToRecapture == nil or bunkerRegionData.aiRecaptureWeight > bunkerRegionForAIToRecapture.aiRecaptureWeight) and bunkerRegionData.aiRegionAttackTimer:IsPastSimTimeLimit() then
					bunkerRegionForAIToRecapture = bunkerRegionData;
				end
			end
		end
	end

	if self.aiData.attackRetargetTimer:IsPastSimTimeLimit() then
		self.aiData.attackTarget = bunkerRegionForAIToRecapture ~= nil and bunkerRegionForAIToRecapture.captureArea.Center or nil;

		self:UpdateAndCleanupDataTables(self.aiTeam);

		for key, actor in pairs(self.aiData.actors.attackers) do
			if key ~= "count" and actor.HasEverBeenAddedToMovableMan then
				local shouldChangeTarget = false;
				if actor.AIMode == Actor.AIMODE_BRAINHUNT then
					shouldChangeTarget = true;
				elseif actor.AIMode == Actor.AIMODE_GOTO then
					local currentWaypointTarget = actor:GetLastAIWaypoint();
					for _, bunkerRegionData in pairs(aiOwnedBunkerRegions) do
						if bunkerRegionData.captureArea:IsInside(currentWaypointTarget) then
							shouldChangeTarget = true;
							break;
						end
					end
				else
					shouldChangeTarget = true;
					for _, bunkerRegionData in pairs(humanOwnedBunkerRegions) do
						if bunkerRegionData.totalArea:IsInside(actor.Pos) then
							shouldChangeTarget = false;
							if not bunkerRegionData.captureArea:IsInside(actor.Pos) then
								actor:AddAISceneWaypoint(bunkerRegionData.captureArea.Center);
								actor.AIMode = Actor.AIMODE_GOTO;
							end
							break;
						end
					end
				end
				if shouldChangeTarget then
					if self.aiData.attackTarget ~= nil then
						actor:AddAISceneWaypoint(self.aiData.attackTarget);
						actor.AIMode = Actor.AIMODE_GOTO;
					else
						actor.AIMode = Actor.AIMODE_BRAINHUNT;
					end
				end
			end
		end
		if bunkerRegionForAIToRecapture then
			bunkerRegionForAIToRecapture.aiRegionAttackTimer:Reset();
		end
		self.aiData.attackRetargetTimer:Reset();
	end
end

function DecisionDay:SpawnBunkerAlliedDefenders(bunkerId)
	local craft = RandomACDropShip("Craft", self.humanTeamTech);
	if not craft or craft.MaxInventoryMass <= 0 then
		craft = RandomACDropShip("Craft", "Base.rte");
	end
	craft.Pos = SceneMan:MovePointToGround(self.bunkerAreas[bunkerId].lzArea.Center, 100, 20);
	craft.Team = self.humanTeam;
	craft.PlayerControllable = false;
	craft.HUDVisible = false;
	craft.AIMode = Actor.AIMODE_RETURN;
	craft:SetGoldValue(0);
	MovableMan:AddActor(craft);

	for item in MovableMan.Items do
		if self.bunkerAreas[bunkerId].totalArea:IsInside(item.Pos) then
			item.ToSettle = true;
		end
	end
	for movableObject in MovableMan:GetMOsInRadius(self.bunkerAreas[bunkerId].leftDefendersArea.Center, 300, self.humanTeam, true) do
		if IsAHuman(movableObject) or IsACrab(movableObject) then
			movableObject.ToDelete = true;
		end
	end

	for box in self.bunkerAreas[bunkerId].leftDefendersArea.Boxes do
		local spawnedActor = self:SpawnInfantry(self.humanTeam, defenderType, box.Center, Actor.AIMODE_SENTRY, true);
		spawnedActor.PlayerControllable = false;
		spawnedActor.HUDVisible = false;
		spawnedActor:AddToGroup("Allied Sentries");
		self.alliedData.actors.sentries[spawnedActor.UniqueID] = spawnedActor;
		self.alliedData.actors.sentries.count = self.alliedData.actors.sentries.count + 1;
	end
	if bunkerId == self.bunkerIds.frontBunker then
		self.frontBunkerAlliedDefendersSpawned = true;
	elseif bunkerId == self.bunkerIds.middleBunker then
		self.middleBunkerAlliedDefendersSpawned = true;
	end
end

function DecisionDay:UpdateAlliedAttackersWaypoint(optionalSpecificActorToUpdate)
	local targetPosition = self.bunkerAreas[self.bunkerIds.middleBunker].lzArea.FirstBox.Corner + Vector(100, 50);
	if self.currentStage >= self.stages.captureMainBunker then
		targetPosition = self.bunkerRegions["Main Bunker Door Controls"].captureArea.Center;
	elseif self.currentStage >= self.stages.middleBunkerCaptured then
		targetPosition = SceneMan:MovePointToGround(self.bunkerAreas[self.bunkerIds.mainBunker].frontDoors.FirstBox.Center + Vector(30, 0), 10, 10);
	end
	if optionalSpecificActorToUpdate then
		optionalSpecificActorToUpdate:ClearAIWaypoints();
		optionalSpecificActorToUpdate:AddAISceneWaypoint(targetPosition + Vector(math.random(-25, 25), 0));
		optionalSpecificActorToUpdate.AIMode = Actor.AIMODE_GOTO;
	else
		self:UpdateAndCleanupDataTables(self.humanTeam);
		for key, actor in pairs(self.alliedData.actors.attackers) do
			if key ~= "count" then
				actor:ClearAIWaypoints();
				actor:AddAISceneWaypoint(targetPosition + Vector(math.random(-25, 25), 0));
				actor.AIMode = Actor.AIMODE_GOTO;
			end
		end
	end
end

function DecisionDay:DoHumanBrainPieSliceHandling()
	for _, player in pairs(self.humanPlayers) do
		local brain = self:GetPlayerBrain(player);
		if brain and MovableMan:ValidMO(brain) and not brain:IsInGroup("Deployed Brain " .. player) then
			local pieSlice = brain.PieMenu:GetFirstPieSliceByPresetName(self.deployBrainPieSlice.PresetName);
			if not pieSlice then
				brain.PieMenu:AddPieSlice(self.deployBrainPieSlice:Clone(), self);
				pieSlice = brain.PieMenu:GetFirstPieSliceByPresetName(self.deployBrainPieSlice.PresetName);
			end
			pieSlice.Enabled = self.bunkerRegions["Front Bunker Operations"].totalArea:IsInside(brain.Pos);
		end
	end

	local activatedDeployBrainPieSlicePlayer = self:LoadNumber("DeployBrain") - 1;
	if activatedDeployBrainPieSlicePlayer >= 0 then
		self:DeployHumanBrain(activatedDeployBrainPieSlicePlayer);
		self:SaveNumber("DeployBrain", 0);
	end
	local activatedUndeployBrainPieSlicePlayer = self:LoadNumber("UndeployBrain") - 1;
	if activatedUndeployBrainPieSlicePlayer >= 0 then
		self:UndeployHumanBrain(activatedUndeployBrainPieSlicePlayer);
		self:SaveNumber("UndeployBrain", 0);
	end
	local activatedSwapControlPieSlicePlayer = self:LoadNumber("SwapControl") - 1;
	if activatedSwapControlPieSlicePlayer >= 0 then
		self:SaveNumber("SwapControl", 0);
		self.humansAreControllingAlliedActors = not self.humansAreControllingAlliedActors;
		for actor in MovableMan.Actors do
			if actor.Team == self.humanTeam and (IsAHuman(actor) or IsACrab(actor)) and not actor:IsInGroup("Brains") and not actor:IsInGroup("Allied Sentries") and actor.PlayerControllable == self.humansAreControllingAlliedActors then
				actor.PlayerControllable = not self.humansAreControllingAlliedActors;
				actor.HUDVisible = not self.humansAreControllingAlliedActors;
				actor:GetController().InputMode = self.humansAreControllingAlliedActors and Controller.CIM_DISABLED or Controller.CIM_AI;
			end
		end
		self:UpdateAndCleanupDataTables(self.humanTeam);
		for key, actor in pairs(self.alliedData.actors.attackers) do
			if key ~= "count" then
				actor.PlayerControllable = self.humansAreControllingAlliedActors;
				actor.HUDVisible = self.humansAreControllingAlliedActors;
				if not self.humansAreControllingAlliedActors then
					self:UpdateAlliedAttackersWaypoint(actor);
				end
			end
		end
	end
end

function DecisionDay:DeployHumanBrain(player)
	if not self.BuyMenuEnabled then
		self.BuyMenuEnabled = true;
		for actor in MovableMan.Actors do
			if actor.Team == self.humanTeam and actor.PlayerControllable then
				actor.PieMenu:AddPieSliceIfPresetNameIsUnique(self.buyMenuPieSlice, self);
			end
		end
	end

	local existingBrain = self:GetPlayerBrain(player);
	if existingBrain then
		existingBrain:SetAimAngle(-0.78);
		existingBrain.PinStrength = 100;
		existingBrain.Pos = SceneMan:MovePointToGround(existingBrain.Pos, existingBrain.Radius, 0);
		existingBrain:FlashWhite(100);

		if existingBrain:IsMechanical() and IsAHuman(existingBrain) then
			existingBrain = ToAHuman(existingBrain);
			existingBrain.Head.Scale = 0;
			existingBrain.PlayerControllable = false;
			existingBrain.HUDVisible = false;
			existingBrain:GetController().InputMode = Controller.CIM_DISABLED;
			existingBrain:AddToGroup("Empty Brain Body " .. player);

			local brainCase = CreateActor("Brain Case", "Base.rte");
			brainCase.Pos = Vector(existingBrain.Pos.X, self.bunkerRegions["Front Bunker Operations"].totalArea.FirstBox.Corner.Y);
			brainCase.Team = self.humanTeam;
			brainCase:FlashWhite(100);
			brainCase:AddToGroup("Deployed Brain " .. player);

			local undeployBrainPieSlice = self.undeployBrainPieSlice:Clone();
			undeployBrainPieSlice.Direction = Directions.Left;
			brainCase.PieMenu:AddPieSliceIfPresetNameIsUnique(undeployBrainPieSlice, self);

			local swapControlPieSlice = self.swapControlPieSlice:Clone();
			swapControlPieSlice.Direction = Directions.Right;
			brainCase.PieMenu:AddPieSliceIfPresetNameIsUnique(swapControlPieSlice, self);

			MovableMan:AddActor(brainCase);
			self:SetPlayerBrain(brainCase, player);
			self:SwitchToActor(brainCase, player, self.humanTeam);
		else
			existingBrain.Status = Actor.INACTIVE;
			existingBrain:GetController().InputMode = Controller.CIM_DISABLED;
			existingBrain.Pos.Y = existingBrain.Pos.Y - 20;
			existingBrain.PieMenu:RemovePieSlicesByPresetName(self.deployBrainPieSlice.PresetName);
			existingBrain.PieMenu:AddPieSliceIfPresetNameIsUnique(self.undeployBrainPieSlice, self);
			existingBrain.PieMenu:AddPieSliceIfPresetNameIsUnique(self.swapControlPieSlice, self);
			existingBrain:AddToGroup("Deployed Brain " .. player);
		end

		self.anyHumanHasDeployedABrain = true;
	end
end

function DecisionDay:UndeployHumanBrain(player)
	local existingBrain = self:GetPlayerBrain(player);
	if existingBrain then
		if self.humansAreControllingAlliedActors then
			self:SaveNumber("SwapControl", 1);
		end

		if existingBrain:IsMechanical() then
			local emptyBrainBody;
			for movableObject in MovableMan:GetMOsInRadius(existingBrain.Pos, 200, self.aiTeam, true) do
				if movableObject:IsInGroup("Empty Brain Body " .. player) and IsAHuman(movableObject) then
					emptyBrainBody = ToAHuman(movableObject);
					break;
				end
			end
			if emptyBrainBody then
				emptyBrainBody.PinStrength = 0;
				emptyBrainBody.Head.Scale = 1;
				emptyBrainBody.PlayerControllable = true;
				emptyBrainBody.HUDVisible = true;
				emptyBrainBody:FlashWhite(100);
				emptyBrainBody:RemoveFromGroup("Empty Brain Body " .. player);

				self:SetPlayerBrain(emptyBrainBody, player);
				self:SwitchToActor(emptyBrainBody, player, self.humanTeam);
				existingBrain.ToDelete = true;
			end
		else
			existingBrain.Status = Actor.STABLE;
			existingBrain.PinStrength = 0;
			existingBrain:FlashWhite(100);
			existingBrain.PieMenu:RemovePieSlicesByPresetName(self.undeployBrainPieSlice.PresetName);
			existingBrain.PieMenu:RemovePieSlicesByPresetName(self.swapControlPieSlice.PresetName);
			existingBrain:RemoveFromGroup("Deployed Brain " .. player);
		end
	end
end

function DecisionDay:DoAlliedSpawns(forceSpawn)
	if forceSpawn or self.alliedData.spawnTimer:IsPastSimTimeLimit() then
		self:UpdateAndCleanupDataTables(self.humanTeam);

		if forceSpawn or self.alliedData.actors.attackers.count < self.alliedData.attackerLimit then
			local alliedCraft = self:SpawnCraft(self.humanTeam, -1, math.random() > 0.5, nil, nil, nil);
			for actor in alliedCraft.Inventory do
				actor = ToActor(actor);
				actor:AddToGroup("Allied Attackers");
				actor.HUDVisible = self.humansAreControllingAlliedActors;
				self.alliedData.actors.attackers[actor.UniqueID] = actor;
				self.alliedData.actors.attackers.count = self.alliedData.actors.attackers.count + 1;
				self:UpdateAlliedAttackersWaypoint(actor);
			end
		end

		self.alliedData.spawnTimer:Reset();
	end
end

function DecisionDay:DoAIExternalSpawns()
	if self.aiData.externalSpawnTimer:IsPastSimTimeLimit() and self.aiData.actors.attackers.count < self.aiData.attackerLimit and self:GetTeamFunds(self.aiTeam) > 0 then
		self:UpdateAndCleanupDataTables(self.aiTeam);
		self.previousCraftLZInfo[self.aiTeam] = nil;

		local numberOfActorsSpawned = 0;
		while numberOfActorsSpawned < self.aiData.attackersPerSpawn and self:GetTeamFunds(self.aiTeam) > 0 do
			local aiCraft = self:SpawnCraft(self.aiTeam, true, true, nil, self.aiData.attackersPerSpawn - numberOfActorsSpawned, nil);
			self:ChangeTeamFunds(-aiCraft:GetTotalValue(self.aiTeamTech, 1), self.aiTeam);
			for actor in aiCraft.Inventory do
				numberOfActorsSpawned = numberOfActorsSpawned + 1;
				actor = ToActor(actor);
				actor:AddToGroup("AI Attackers");
				self.aiData.actors.attackers[actor.UniqueID] = actor;
				self.aiData.actors.attackers.count = self.aiData.actors.attackers.count + 1;

				if self.aiData.attackTarget ~= nil then
					actor:AddAISceneWaypoint(self.aiData.attackTarget);
					actor.AIMode = Actor.AIMODE_GOTO;
				else
					actor.AIMode = Actor.AIMODE_BRAINHUNT;
				end
			end
		end

		self.aiData.externalSpawnTimer:Reset();
	end
end

function DecisionDay:UpdateMainBunkerExternalPopoutTurrets()
	local updateMovementTimerForActivationChange = function(movementTimer)
		if movementTimer:IsPastSimTimeLimit() then
			movementTimer:Reset();
		else
			movementTimer.ElapsedSimTimeMS = movementTimer:GetSimTimeLimitMS() * (1 - movementTimer.SimTimeLimitProgress);
		end
	end

	local bunkerId = self.bunkerIds.mainBunker;
	if self.popoutTurretsData[bunkerId].enabled then
		for box, boxData in pairs(self.popoutTurretsData[bunkerId].boxData) do
			if boxData.actor and not MovableMan:ValidMO(boxData.actor) then
				boxData.actor = nil;
				boxData.respawnTimer:Reset();
			elseif not boxData.actor and boxData.respawnTimer:IsPastSimTimeLimit() then
				local popoutTurret = self.popoutTurretTemplate:Clone();
				popoutTurret.Status = Actor.INACTIVE;
				popoutTurret.Pos = box.Center;
				boxData.actor = popoutTurret;
				boxData.movementTimer:Reset();
				MovableMan:AddActor(popoutTurret);
			end
		end

		local popoutTurretsShouldActivate = false;
		for movableObject in MovableMan:GetMOsInBox(self.popoutTurretsData[bunkerId].activationArea.FirstBox, self.aiTeam, true) do
			if IsAHuman(movableObject) or IsACrab(movableObject) then
				popoutTurretsShouldActivate = true;
				break;
			end
		end
		if popoutTurretsShouldActivate then
			self.popoutTurretsData[bunkerId].deactivationDelayTimer:Reset();
			if not self.popoutTurretsData[bunkerId].turretsActivated then
				self.popoutTurretsData[bunkerId].turretsActivated = true;

				for _, boxData in pairs(self.popoutTurretsData[bunkerId].boxData) do
					updateMovementTimerForActivationChange(boxData.movementTimer);
				end
			end
		elseif not popoutTurretsShouldActivate and self.popoutTurretsData[bunkerId].turretsActivated and self.popoutTurretsData[bunkerId].deactivationDelayTimer:IsPastSimTimeLimit() then
			self.popoutTurretsData[bunkerId].turretsActivated = false;
			for _, boxData in pairs(self.popoutTurretsData[bunkerId].boxData) do
				updateMovementTimerForActivationChange(boxData.movementTimer);
			end
		end

		for box, boxData in pairs(self.popoutTurretsData[bunkerId].boxData) do
			if not boxData.movementTimer:IsPastSimTimeLimit() and boxData.actor then
				local startPos = self.popoutTurretsData[bunkerId].turretsActivated and box.Center or box.Center + Vector(25, 25);
				local endPos = self.popoutTurretsData[bunkerId].turretsActivated and box.Center + Vector(25, 25) or box.Center;
				boxData.actor.Pos.X = Lerp(0, 1, startPos.X, endPos.X, boxData.movementTimer.SimTimeLimitProgress);
				boxData.actor.Pos.Y = Lerp(0, 1, startPos.Y, endPos.Y, boxData.movementTimer.SimTimeLimitProgress);
				if boxData.movementSound:IsBeingPlayed() then
					boxData.movementSound.Pos = boxData.actor.Pos;
				else
					boxData.movementSound:Play(boxData.actor.Pos);
				end
			elseif boxData.movementTimer:IsPastSimTimeLimit() then
				boxData.movementSound:Stop();
			end
		end
	end
end

function DecisionDay:UpdateBrainDefenderSpawning()
	if self.aiData.brainDefenderReplenishTimer:IsPastSimTimeLimit() then
		if self.aiData.brainDefendersRemaining < self.aiData.brainDefendersTotal and self.bunkerRegions["Main Bunker Barracks"].ownerTeam == self.aiTeam then
			self.aiData.brainDefendersRemaining = self.aiData.brainDefendersRemaining + 1;
		end
		self.aiData.brainDefenderReplenishTimer:Reset();
	end

	if self.aiData.brainDefendersRemaining > 0 and self.aiData.brainDefenderSpawnTimer:IsPastSimTimeLimit() then
		local bunkerRegionData = self.bunkerRegions["Main Bunker Command Center"];
		local enemiesInsideCommandCenter = {};
		for box in bunkerRegionData.totalArea.Boxes do
			for movableObject in MovableMan:GetMOsInBox(box, self.aiTeam, true) do
				if IsAHuman(movableObject) or IsACrab(movableObject) then
					enemiesInsideCommandCenter[#enemiesInsideCommandCenter + 1] = movableObject;
				end
			end
		end

		if #enemiesInsideCommandCenter > 0 then
			local infantryType = "CQB";
			for box in bunkerRegionData.internalReinforcementsArea.Boxes do
				local internalReinforcementPositionsToEnemyTargets = {};
				for _, internalReinforcementPosition in pairs(self.internalReinforcementsData[bunkerRegionData.bunkerId].positions) do
					if box:IsWithinBox(internalReinforcementPosition) then
						internalReinforcementPositionsToEnemyTargets[internalReinforcementPosition] = {};
						local targetPos = math.random() < 0.25 and bunkerRegionData.captureArea.Center or enemiesInsideCommandCenter[math.random(#enemiesInsideCommandCenter)].Pos;
						table.insert(internalReinforcementPositionsToEnemyTargets[internalReinforcementPosition], targetPos);

						if self.aiData.brainDefendersRemaining > 1 then
							targetPos = math.random() < 0.25 and bunkerRegionData.captureArea.Center or enemiesInsideCommandCenter[math.random(#enemiesInsideCommandCenter)].Pos;
							table.insert(internalReinforcementPositionsToEnemyTargets[internalReinforcementPosition], targetPos);
						end

						if self.difficultyRatio >= 1 and self.aiData.brainDefendersRemaining > 2 then
							targetPos = math.random() < 0.25 and bunkerRegionData.captureArea.Center or enemiesInsideCommandCenter[math.random(#enemiesInsideCommandCenter)].Pos;
							table.insert(internalReinforcementPositionsToEnemyTargets[internalReinforcementPosition], targetPos);
						end
						break;
					end
				end
				self.aiData.brainDefendersRemaining = self.aiData.brainDefendersRemaining - self:CreateInternalReinforcements(infantryType, internalReinforcementPositionsToEnemyTargets);
				infantryType = "Heavy";
				if self.aiData.brainDefendersRemaining <= 0 then
					break;
				end
			end
		end

		self.aiData.brainDefenderSpawnTimer:Reset();
	end
end

function DecisionDay:UpdateActivity()
	self:ClearObjectivePoints();

	if (self.ActivityState == Activity.OVER) then
		return;
	end

	self:DoGameOverCheck();

	self:UpdateCurrentStage();

	if self.WinnerTeam == -1 then
		self:UpdateCamera();
	end

	self:UpdateMessages();

	if self.WinnerTeam ~= Activity.NOTEAM then
		return;
	end
	
	if self.speedrunData and not ActivitySpeedrunHelper.SpeedrunActive(self.speedrunData) then
		if self.currentStage == self.stages.followInitialDropShip then
			ActivitySpeedrunHelper.CheckForSpeedrun(self.speedrunData);
		else
			self.speedrunData = nil;
		end
	end

	if self.currentStage < self.stages.frontBunkerCaptured then
		self:SpawnAndUpdateInitialDropShips();
	end

	if self.currentStage >= self.stages.attackFrontBunker then
		self:UpdateObjectiveArrowsAndRegionVisuals();
	end

	self:UpdateRegionCapturing();

	self:UpdateRegionScreens();

	self:UpdateVaultTickIncome();

	if self.aiData.internalReinforcementsEnabled then
		self:UpdateAIInternalReinforcements();
	end

	if self.alliedData.spawnsEnabled then
		self:DoAlliedSpawns();
	end

	if self.aiData.externalSpawnsEnabled then
		self:DoAIExternalSpawns();
	end

	if self.currentStage >= self.stages.attackFrontBunker then
		self:UpdateAIDecisions();
	end

	if self.currentStage >= self.stages.deployBrain then
		if self.currentStage == self.stages.deployBrain and not self.frontBunkerAlliedDefendersSpawned and self.messageTimer.SimTimeLimitProgress > 0.65 then
			self:SpawnBunkerAlliedDefenders(self.bunkerIds.frontBunker);
			self.alliedData.spawnTimer.ElapsedSimTimeMS = self.alliedData.spawnTimer:GetSimTimeLimitMS() * 0.85;
			self.aiData.externalSpawnTimer.ElapsedSimTimeMS = self.aiData.externalSpawnTimer:GetSimTimeLimitMS() * 0.5;
		elseif self.currentStage == self.stages.middleBunkerCaptured and not self.middleBunkerAlliedDefendersSpawned and self.messageTimer.SimTimeLimitProgress > 0.65 then
			self:SpawnBunkerAlliedDefenders(self.bunkerIds.middleBunker);
		end

		if self.currentStage > self.stages.deployBrain or (self.currentStage == self.stages.deployBrain and self.currentMessageNumber > 1) then
			self:DoHumanBrainPieSliceHandling();
		end

		if self.currentStage >= self.stages.middleBunkerCaptured then
			if self.bunkerRegions["Main Bunker Shield Generator"].ownerTeam == self.aiTeam and self.mainBunkerShieldedAreaFOWTimer:IsPastSimTimeLimit() then
				for box in self.bunkerRegions["Main Bunker Command Center"].shieldedArea.Boxes do
					SceneMan:RestoreUnseenBox(box.Corner.X, box.Corner.Y, box.Width, box.Height, self.humanTeam);
				end
				self.mainBunkerShieldedAreaFOWTimer:Reset();
			end

			if self.currentStage < self.stages.captureMainBunker then
				self:UpdateMainBunkerExternalPopoutTurrets();
			end
		end

		if self.currentStage == self.stages.findTunnel then
			for hiddenTunnelBox in self.hiddenTunnelArea.Boxes do
				for movableObject in MovableMan:GetMOsInBox(hiddenTunnelBox, self.aiTeam, true) do
					if IsAHuman(movableObject) or IsACrab(movableObject) then
						self.tunnelHasBeenEntered = true;
						break;
					end
				end
				if self.tunnelHasBeenEntered then
					break;
				end
			end
		end

		if self.currentStage == self.stages.attackBrain then
			self:UpdateBrainDefenderSpawning();
		end
	end
end

function DecisionDay:EndActivity()
	-- Temp fix so music doesn't start playing if ending the Activity when changing resolution through the ingame settings.
	if not self:IsPaused() then
		if self:HumanBrainCount() == 0 then
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

function DecisionDay:SpawnInfantry(team, infantryType, pos, aimode, hflipped)
	local actor = self:CreateInfantry(team, infantryType);
	actor.Pos = pos;
	actor.Team = team;
	actor.AIMode = aimode;
	actor.HFlipped = hflipped;
	MovableMan:AddActor(actor);
	return actor;
end

function DecisionDay:SpawnCraft(team, avoidPreviousCraftPos, useRocketsInsteadOfDropShips, infantryType, passengerCount)
	local tech = team == self.humanTeam and self.humanTeamTech or self.aiTeamTech;
	local crabToHumanSpawnRatio = self:GetCrabToHumanSpawnRatio(tech);
	crabToHumanSpawnRatio = 0;

	local craft = useRocketsInsteadOfDropShips and RandomACRocket("Craft", tech) or RandomACDropShip("Craft", tech);
	if not craft or craft.MaxInventoryMass <= 0 then
		craft = useRocketsInsteadOfDropShips and RandomACRocket("Craft", "Base.rte") or RandomACDropShip("Craft", "Base.rte");
	end
	craft.Team = team;
	craft.PlayerControllable = false;
	craft.HUDVisible = team ~= self.humanTeam;
	if team == self.humanTeam then
		craft:SetGoldValue(0);
	end

	local lzArea = team == self.humanTeam and self.alliedData.lzArea or self.aiData.lzArea;
	craft.Pos = Vector(lzArea.RandomPoint.X, -50);
	local craftSpriteWidth = craft:GetSpriteWidth();
	if avoidPreviousCraftPos ~= 0 and avoidPreviousCraftPos ~= false and self.previousCraftLZInfo[team] ~= nil then
		local spaceToLeaveBetweenCrafts = (craftSpriteWidth + self.previousCraftLZInfo[team].craftSpriteWidth) * 1.5;
		if math.abs(SceneMan:ShortestDistance(craft.Pos, Vector(self.previousCraftLZInfo[team].posX, 0), false).X) < spaceToLeaveBetweenCrafts then
			if avoidPreviousCraftPos == true then
				avoidPreviousCraftPos = math.random() < 0.5 and -1 or 1;
			end
			craft.Pos.X = self.previousCraftLZInfo[team].posX + (avoidPreviousCraftPos * spaceToLeaveBetweenCrafts);
			if craft.Pos.X >= (SceneMan.SceneWidth - craftSpriteWidth) then
				craft.Pos.X = self.previousCraftLZInfo[team] - spaceToLeaveBetweenCrafts;
			end
		end
	end
	self.previousCraftLZInfo[team] = {posX = craft.Pos.X, craftSpriteWidth = craftSpriteWidth};

	if passengerCount == nil then
		passengerCount = math.random(math.ceil(craft.MaxPassengers * 0.5), craft.MaxPassengers);
	end
	passengerCount = math.min(passengerCount, craft.MaxPassengers);
	for i = 1, passengerCount do
		local actor;
		if infantryType then
			passenger = self:CreateInfantry(team, infantryType);
		elseif math.random() < crabToHumanSpawnRatio then
			passenger = self:CreateCrab(team);
		else
			passenger = self:CreateInfantry(team);
		end

		if passenger then
			passenger.Team = team;
			craft:AddInventoryItem(passenger);
			if craft.InventoryMass > craft.MaxInventoryMass then
				break;
			end
		end
	end
	MovableMan:AddActor(craft);
	return craft;
end

function DecisionDay:CalculateInternalReinforcementPositionsToEnemyTargets(bunkerId, maxNumberOfInternalReinforcementsToCreate, maxFundsForInternalReinforcements)
	local enemiesToTarget = {};
	for i = 1, maxNumberOfInternalReinforcementsToCreate do
		if enemiesToTarget[i] == nil then
			enemiesToTarget[i] = self.aiData.enemiesInsideBunkers[bunkerId][math.random(1, #self.aiData.enemiesInsideBunkers[bunkerId])];
		end
	end
	local internalReinforcementPositionsToEnemyTargets = {};

	if coroutine.running() then
		coroutine.yield(); -- Yield after initial setup, so we can set up our coroutines separately from running them.
	end

	for _, enemyToTarget in ipairs(enemiesToTarget) do
		if MovableMan:ValidMO(enemyToTarget) then
			local internalReinforcementPositionForEnemy;
			local shortestPathCoroutine = coroutine.create(FindStartPositionWithShortestPathToEndPosition);
			while coroutine.status(shortestPathCoroutine) ~= "dead" do
				local _, result = coroutine.resume(shortestPathCoroutine, self.internalReinforcementsData[bunkerId].positions, enemyToTarget.Pos, self.aiTeam);
				if result then
					internalReinforcementPositionForEnemy = result.position;
				else
					coroutine.yield();
				end
			end

			if internalReinforcementPositionForEnemy then
				if not internalReinforcementPositionsToEnemyTargets[internalReinforcementPositionForEnemy] then
					internalReinforcementPositionsToEnemyTargets[internalReinforcementPositionForEnemy] = {};
				end
				table.insert(internalReinforcementPositionsToEnemyTargets[internalReinforcementPositionForEnemy], enemyToTarget);
			end
		end
	end

	return internalReinforcementPositionsToEnemyTargets, maxNumberOfInternalReinforcementsToCreate, maxFundsForInternalReinforcements;
end

function DecisionDay:CreateInternalReinforcements(loadout, internalReinforcementPositionsToEnemyTargets, maxNumberOfInternalReinforcementsToCreate, maxFundsForInternalReinforcements)
	if loadout == "Any" then
		loadout = nil;
	end
	if maxNumberOfInternalReinforcementsToCreate == nil then
		maxNumberOfInternalReinforcementsToCreate = 999;
	end
	if maxFundsForInternalReinforcements == nil then
		maxFundsForInternalReinforcements = 999999;
	end
	if maxFundsForInternalReinforcements <= 0 then
		return {}, maxFundsForInternalReinforcements;
	end
	local crabToHumanSpawnRatio = self:GetCrabToHumanSpawnRatio(self.aiTeamTech);
	crabToHumanSpawnRatio = 0;

	local numberOfReinforcementsCreated = 0;
	for internalReinforcementPosition, enemyTargetsForPosition in pairs(internalReinforcementPositionsToEnemyTargets) do
		if numberOfReinforcementsCreated < maxNumberOfInternalReinforcementsToCreate and maxFundsForInternalReinforcements > 0 then
			local doorParticle = self.internalReinforcementsDoorParticle:Clone();
			doorParticle.Pos = internalReinforcementPosition;
			doorParticle.Team = self.aiTeam;
			MovableMan:AddParticle(doorParticle);
			self.internalReinforcementsData.doorsAndActorsToSpawn[doorParticle] = {};

			local numberOfInternalReinforcementsToCreateAtPosition = math.min(#enemyTargetsForPosition, 5);
			if numberOfInternalReinforcementsToCreateAtPosition == 1 and math.random() < (self.difficultyRatio * 0.5) then
				numberOfInternalReinforcementsToCreateAtPosition = 2;
				if math.random() < (self.difficultyRatio * 0.1) then
					numberOfInternalReinforcementsToCreateAtPosition = 3;
				end
			end

			for i = 1, numberOfInternalReinforcementsToCreateAtPosition do
				local internalReinforcement;
				if loadout then
					internalReinforcement = self:CreateInfantry(self.aiTeam, loadout);
				elseif math.random() < crabToHumanSpawnRatio then
					local createTurretReinforcement = math.random() < 0.05;
					internalReinforcement = self:CreateCrab(self.aiTeam, createTurretReinforcement);
				else
					internalReinforcement = self:CreateInfantry(self.aiTeam);
				end
				internalReinforcement.Team = self.aiTeam;
				internalReinforcement.Pos = internalReinforcementPosition;
				if numberOfInternalReinforcementsToCreateAtPosition > 1 then
					local leftmostSpawnOffset = 20;
					internalReinforcement.Pos.X = internalReinforcement.Pos.X - leftmostSpawnOffset + ((i - 1) * ((leftmostSpawnOffset * 2) / (numberOfInternalReinforcementsToCreateAtPosition - 1)));
				end
				if internalReinforcement:IsInGroup("Actors - Turrets") then
					internalReinforcement.AIMode = Actor.AIMODE_SENTRY;
				else
					internalReinforcement.AIMode = Actor.AIMODE_GOTO;
					local internalReinforcementTarget = enemyTargetsForPosition[math.random(#enemyTargetsForPosition)];
					if internalReinforcementTarget.ClassName == "Vector" then
						internalReinforcement:AddAISceneWaypoint(internalReinforcementTarget);
					else
						internalReinforcement:AddAIMOWaypoint(internalReinforcementTarget);
					end
				end

				table.insert(self.internalReinforcementsData.doorsAndActorsToSpawn[doorParticle], internalReinforcement);

				numberOfReinforcementsCreated = numberOfReinforcementsCreated + 1;
				maxFundsForInternalReinforcements = maxFundsForInternalReinforcements - internalReinforcement:GetTotalValue(self.aiTeamTech, 1);
				if numberOfReinforcementsCreated >= maxNumberOfInternalReinforcementsToCreate or maxFundsForInternalReinforcements <= 0 then
					break;
				end
			end
		end
	end
	return numberOfReinforcementsCreated, maxFundsForInternalReinforcements;
end

function DecisionDay:CreateInfantry(team, infantryType)
	local tech = team == self.humanTeam and self.humanTeamTech or self.aiTeamTech;
	if infantryType == nil then
		local infantryTypes = {"Light", "Sniper", "Heavy", "CQB"};
		infantryType = infantryTypes[math.random(#infantryTypes)];
	end
	local allowAdvancedEquipment = team == self.humanTeam or self.bunkerRegions["Main Bunker Armory"].ownerTeam == team;
	if not allowAdvancedEquipment and self.difficultyRatio > 1 then
		allowAdvancedEquipment = math.random() < (1 - (4 / (self.difficultyRatio * 3)));
	end

	local actorType = (infantryType == "Heavy" or infantryType == "CQB") and "Actors - Heavy" or "Actors - Light";
	if infantryType == "CQB" and math.random() < 0.25 then
		actorType = "Actors - Light";
	end
	local actor = RandomAHuman(actorType, tech);
	if actor.ModuleID ~= tech then
		actor = RandomAHuman("Actors", tech);
	end
	actor.Team = team;
	actor.PlayerControllable = self.humansAreControllingAlliedActors;

	if infantryType == "Light" then
		actor:AddInventoryItem(RandomHDFirearm("Weapons - Light", tech));
		actor:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", tech));
		if allowAdvancedEquipment then
			if math.random() < 0.5 then
				actor:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", tech));
			elseif math.random() < 0.1 then
				actor:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", tech));
			elseif math.random() < 0.3 then
				actor:AddInventoryItem(CreateHDFirearm("Medikit", "Base.rte"));
			end
		end
	elseif infantryType == "Sniper" then
		if allowAdvancedEquipment then
			actor:AddInventoryItem(RandomHDFirearm("Weapons - Sniper", tech));
		else
			actor:AddInventoryItem(RandomHDFirearm("Weapons - Light", tech));
		end
		actor:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", tech));
		if allowAdvancedEquipment then
			if math.random() < 0.3 then
				actor:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", tech));
			elseif math.random() < 0.5 then
				actor:AddInventoryItem(CreateHDFirearm("Medikit", "Base.rte"));
			end
		end
	elseif infantryType == "Heavy" then
		if allowAdvancedEquipment then
			actor:AddInventoryItem(RandomHDFirearm("Weapons - Heavy", tech));
		else
			actor:AddInventoryItem(RandomHDFirearm("Weapons - Primary", tech));
		end
		actor:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", tech));
		if allowAdvancedEquipment and math.random() < 0.3 then
			if math.random() < 0.5 then
				actor:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", tech));
				if math.random() < 0.1 then
					actor:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", tech));
				end
			else
				actor:AddInventoryItem(RandomHeldDevice("Shields", tech));
				if math.random() < 0.3 then
					actor:AddInventoryItem(CreateHDFirearm("Medikit", "Base.rte"));
				end
			end
		end
	elseif infantryType == "CQB" then
		actor:AddInventoryItem(RandomHDFirearm("Weapons - CQB", tech));
		actor:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", tech));
		if allowAdvancedEquipment then
			if math.random() < 0.3 then
				actor:AddInventoryItem(RandomHeldDevice("Shields", tech));
				if math.random() < 0.3 then
					actor:AddInventoryItem(CreateHDFirearm("Medikit", "Base.rte"));
				end
			else
				actor:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", tech));
				if math.random() < 0.1 then
					actor:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", tech));
				end
			end
		end
	end

	return actor;
end

function DecisionDay:CreateCrab(team, createTurret)
	local tech = team == self.humanTeam and self.humanTeamTech or self.aiTeamTech;
	local crabToHumanSpawnRatio = self:GetCrabToHumanSpawnRatio(tech);
	local group = createTurret and "Actors - Turrets" or "Actors - Mecha";

	local actor;
	if crabToHumanSpawnRatio > 0 then
		actor = RandomACrab(group, tech);
	end
	if actor == nil or (createTurret and not actor:IsInGroup("Actors - Turrets")) then
		if createTurret then
			actor = CreateACrab("TradeStar Turret", "Base.rte");
		else
			return self:CreateInfantry(team, "Heavy");
		end
	end
	actor.Team = team;
	actor.PlayerControllable = createTurret or self.humansAreControllingAlliedActors;
	return actor;
end
