--[[

*** INSTRUCTIONS ***

This activity can be run on any scene with a "Bunker Breach LZ" and "Brain" area.
The player brains spawn in the "Bunker Breach LZ" area and the AI brain bot in the "Brain" area.
The script will look for player units and send reinforcements to attack them.

When using with randomized bunkers which has multiple brain chambers or other non-Brain Hideout deployments
only one brain at random chamber will be spawned. To avoid wasting MOs for this actors you may define a "Brain Chamber"
area. All actors inside "Brain Chamber" but without a brain nearby will be removed as useless.

Add defender units by placing areas named:
"Sniper1" to "Sniper10"
"Light1" to "Light10"	<-- light actors, Weapons - Light
"Heavy1" to "Heavy10"	<-- heavy actors, Weapons - Heavy
"Crab1" to "Crab10"
"Turret1" to "Turret10"
"Engineer1" to "Engineer10"	<-- light actors, digger, gold dig AI-mode
"Anti-Air1" to "Anti-Air5"
--]]

function Siege:StartActivity()
	self.PlayerTeam = Activity.TEAM_2;

	-- Select a tech for the CPU player
	self.CPUTechName = self:GetTeamTech(self.CPUTeam);	-- Select a tech for the CPU player
	self.PlayerTechName = self:GetTeamTech(self.PlayerTeam);
	gPrevAITech = self.CPUTechName	-- Store the AI tech in a global so we don't pick the same tech again next round
	self.CPUTechID = PresetMan:GetModuleID(self.CPUTechName);

	self:SetTeamFunds(math.ceil((3000 + self.Difficulty * 175) * rte.StartingFundsScale), self.CPUTeam);
	if self.Difficulty == 100 then
		self:SetTeamFunds(50000, self.CPUTeam);
	end
	self.InitialFunds = self:GetTeamFunds(self.CPUTeam);
	self.FundsLastFrame = self.InitialFunds;

	self:SetTeamFunds(self:GetStartingGold(), self.PlayerTeam);

	-- This line will filter out all scenes without any "Bunker Breach LZ" area
	self.InvaderLZ = SceneMan.Scene:GetArea("Bunker Breach LZ");

	-- Timers
	self.WinTimer = Timer();
	self.SpawnTimer = Timer();
	self.spawnDelay = 0;
	self.HunterTimer = Timer();
	self.HunterDelay = self.spawnDelay + 10000;
	self.IntruderAlertTimer = Timer();
	self.IntruderDisbatchDelay = 5000;

	local playerBrainsLocation = nil;


	self.DiggersEssential = false;
	if SceneMan.Scene:HasArea("Diggers Essential") then
		self.DiggersEssential = true;
		print ("Diggers essential");
	end

	-- Remap everything to be player's
	--for actor in MovableMan.AddedActors do
	--	actor.Team = self.PlayerTeam;
	--end

	if SceneMan.Scene:HasArea("Brain") then
		playerBrainsLocation = SceneMan.Scene:GetOptionalArea("Brain"):GetCenterPoint();
	else
		-- Look for a brain among actors created by the deployments
		for actor in MovableMan.AddedActors do
			if actor.Team == self.PlayerTeam and actor:IsInGroup("Brains") then
				playerBrainsLocation = actor.Pos;
				break;
			end
		end
	end

	self.BrainLocations = playerBrainsLocation;

	-- Add player brains
	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) then
			local Brain = MovableMan:GetUnassignedBrain(self.PlayerTeam);

			if not Brain then
				Brain = self:CreateBrainBot(Actor.AIMODE_SENTRY, self.PlayerTeam, self.PlayerTechName);
				MovableMan:AddActor(Brain);
				Brain.Pos = playerBrainsLocation;
				Brain.Team = self.PlayerTeam;
			end

			if Brain then
				self:SetPlayerBrain(Brain, player);
				self:SetObservationTarget(Brain.Pos, player);
			end
		end
	end

	if SceneMan.Scene:HasArea("Brain Chamber") then
		self.BrainChamber = SceneMan.Scene:GetOptionalArea("Brain Chamber");

		-- Set all useless actors, i.e. those who should guard brain in the brain chamber but their brain is in another castle
		-- to delete themselves, because otherwise they are most likely to stand there for the whole battle and waste MOs
		for actor in MovableMan.AddedActors do
			if actor.Team == self.PlayerTeam and self.BrainChamber:IsInside(actor.Pos)
			and SceneMan:ShortestDistance(actor.Pos, playerBrainsLocation, false):MagnitudeIsGreaterThan(200)
			and not actor:HasObjectInGroup("Brains") and (actor.ClassName == "AHuman" or actor.ClassName == "ACrab") then
				actor.ToDelete = true;
			end
		end
	end

	if SceneMan.Scene:HasArea("Perimeter") then
		self.Perimeter = SceneMan.Scene:GetOptionalArea("Perimeter");
		--print ("Perimeter defined");
	end

	local GuardArea = "Sniper";
	for i = 1, 10 do
		if SceneMan.Scene:HasArea(GuardArea..i) then
			local Guard = self:CreateSniper(Actor.AIMODE_SENTRY, self.PlayerTechName);
			if Guard then
				Guard.Team = self.PlayerTeam;
				Guard.Pos = SceneMan.Scene:GetArea(GuardArea..i):GetCenterPoint();
				MovableMan:AddActor(Guard);
			end
		else
			break;
		end
	end

	GuardArea = "Light";
	for i = 1, 10 do
		if SceneMan.Scene:HasArea(GuardArea..i) then
			local Guard = self:CreateLightInfantry(Actor.AIMODE_SENTRY, self.PlayerTechName);
			if Guard then
				Guard.Team = self.PlayerTeam;
				Guard.Pos = SceneMan.Scene:GetArea(GuardArea..i):GetCenterPoint();
				MovableMan:AddActor(Guard);
			end
		else
			break;
		end
	end

	GuardArea = "Heavy";
	for i = 1, 10 do
		if SceneMan.Scene:HasArea(GuardArea..i) then
			local Guard = self:CreateHeavyInfantry(Actor.AIMODE_SENTRY, self.PlayerTechName);
			if Guard then
				Guard.Team = self.PlayerTeam;
				Guard.Pos = SceneMan.Scene:GetArea(GuardArea..i):GetCenterPoint();
				MovableMan:AddActor(Guard);
			end
		else
			break;
		end
	end

	GuardArea = "Crab";
	for i = 1, 10 do
		if SceneMan.Scene:HasArea(GuardArea..i) then
			local Guard = self:CreateCrab(Actor.AIMODE_SENTRY, self.PlayerTechName);
			if Guard then
				Guard.Team = self.PlayerTeam;
				Guard.Pos = SceneMan.Scene:GetArea(GuardArea..i):GetCenterPoint();
				MovableMan:AddActor(Guard);
			end
		else
			break;
		end
	end

	GuardArea = "Turret";
	for i = 1, 10 do
		if SceneMan.Scene:HasArea(GuardArea..i) then
			local Guard = self:CreateTurret(Actor.AIMODE_SENTRY, self.PlayerTechName);
			if Guard then
				Guard.Team = self.PlayerTeam;
				Guard.Pos = SceneMan.Scene:GetArea(GuardArea..i):GetCenterPoint();
				MovableMan:AddActor(Guard);
			end
		else
			break;
		end
	end

	GuardArea = "Engineer";
	for i = 1, 10 do
		if SceneMan.Scene:HasArea(GuardArea..i) then
			local Guard = self:CreateEngineer(Actor.AIMODE_SENTRY, self.PlayerTechName);
			if Guard then
				Guard.Team = self.PlayerTeam;
				Guard.Pos = SceneMan.Scene:GetArea(GuardArea..i):GetCenterPoint();
				MovableMan:AddActor(Guard);
			end
		else
			break;
		end
	end

	GuardArea = "Anti-Air";
	for i = 1, 5 do
		if SceneMan.Scene:HasArea(GuardArea..i) then
			local Guard = self:CreateAntiAir(Actor.AIMODE_SENTRY, self.PlayerTechName);
			if Guard then
				Guard.Team = self.PlayerTeam;
				Guard.Pos = SceneMan.Scene:GetArea(GuardArea..i):GetCenterPoint();
				MovableMan:AddActor(Guard);
			end
		else
			break;
		end
	end

	-- Add fog
	if self:GetFogOfWarEnabled() then
		--SceneMan:MakeAllUnseen(Vector(65, 65), self.CPUTeam);
		--SceneMan:MakeAllUnseen(Vector(25, 25), self.PlayerTeam);
	end

	-- Store data about terrain and enemy actors in the LZ map, use it to pick safe landing zones
	self.LZMap = require("Activities/Utility/LandingZoneMap");
	self.LZMap:Initialize({self.CPUTeam}); -- a list of AI teams

	-- Switch all doors team to player's
	for actor in MovableMan.Actors do
		actor.Team = self.PlayerTeam;
	end

	for actor in MovableMan.AddedActors do
		actor.Team = self.PlayerTeam;
	end
end


function Siege:EndActivity()
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

function Siege:UpdateActivity()
	if self.ActivityState == Activity.OVER then
		return;
	end

	self:ClearObjectivePoints(); -- Clear the points, as they are re-added each frame.

	-- Increase CPU income from digging
	local income = self:GetTeamFunds(self.CPUTeam) - self.FundsLastFrame;

	if income > 0 and income < 10 then
		income = income * 5;
		self:SetTeamFunds(self:GetTeamFunds(self.CPUTeam) + income, self.CPUTeam);
	end


	self.FundsLastFrame = self:GetTeamFunds(self.CPUTeam);

	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		local displayValue = self:GetTeamFunds(self.CPUTeam);

		if displayValue < 0 then
			displayValue = 0;
		end

		if self:PlayerActive(player) and self:PlayerHuman(player) then
			FrameMan:SetScreenText("Enemy assault budget: " .. math.floor(displayValue), self:ScreenOfPlayer(player), 0, -1, false);
		end


	end

	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		local Brain = self:GetPlayerBrain(player);
		if Brain and MovableMan:IsActor(Brain) then
			self:AddObjectivePoint("Protect!", Brain.AboveHUDPos, self.PlayerTeam, GameActivity.ARROWDOWN);
		end
	end

	if self.WinTimer:IsPastRealMS(3000) then
		-- Check win conditions
		self.WinTimer:Reset();

		local players = 0;
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self:PlayerActive(player) and self:PlayerHuman(player) then
				local Brain = self:GetPlayerBrain(player);

				-- Look for a new brain
				if not Brain or not MovableMan:ValidMO(Brain) then
					Brain = MovableMan:GetUnassignedBrain(Activity.TEAM_1);
					if Brain then
						self:SetPlayerBrain(Brain, player);
						self:SwitchToActor(Brain, player, self:GetTeamOfPlayer(player));
					else
						self:SetPlayerBrain(nil, player);
					end
				end

				if Brain then
					players = players + 1;
					self:SetObservationTarget(Brain.Pos, player);
					self.BrainLocations = Brain.Pos;
					-- self:AddObjectivePoint("Protect!", Brain.AboveHUDPos, self.PlayerTeam, GameActivity.ARROWDOWN);
				else
					self:ResetMessageTimer(player);
					FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
					FrameMan:SetScreenText("Your brain has been destroyed!", self:ScreenOfPlayer(player), 2000, -1, false);
				end
			end
		end

		if players < 1 then
			self.WinnerTeam = self.CPUTeam;
			MovableMan:KillAllEnemyActors(self.WinnerTeam);
			ActivityMan:EndActivity();
			return;
		end
	elseif self.HunterTimer:IsPastSimMS(self.HunterDelay) then
		self.HunterTimer:Reset();
		self.HunterDelay = 11000;

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

	self.LZMap:Update();

	if self:GetTeamFunds(self.CPUTeam) > 350 then
		if self.SpawnTimer:IsPastSimMS(self.spawnDelay) then
			-- We have a target actor, search for a suitable LZ
			local safePosX = self.LZMap:FindSafeLZ(self.CPUTeam);
			local swatDrop = false;

			local deployType = math.random(4);

			if deployType == 1 then
				if self.BrainLocations then
					safePosX = self.LZMap:FindLZ(self.CPUTeam, self.BrainLocations);
				end
			elseif deployType == 2 then
				safePosX = self.InvaderLZ:GetRandomPoint().X;
			elseif deployType == 3 and self.Perimeter then
				safePosX = self.Perimeter:GetRandomPoint().X;
				swatDrop = true;
			end

			if safePosX then	-- Search done
				self.SpawnTimer:Reset();

				-- Attack target
				self.SpawnTimer:Reset();
				self.spawnDelay = (45000 - self.Difficulty * 250) * rte.SpawnIntervalScale;

				local engineers = 0;

				for actor in MovableMan.Actors do
					if actor.Team == self.CPUTeam and actor.AIMode == Actor.AIMODE_GOLDDIG then
						engineers = engineers + 1;
					end
				end

				if engineers < 4 and self:GetTeamFunds(self.CPUTeam) < self.InitialFunds * 0.15 then
					safePosX = self.LZMap:FindSafeLZ(self.CPUTeam);
					self:CreateEngineerDrop(safePosX, self.CPUTechName);
				elseif swatDrop then
					self:CreateSWATDrop(safePosX, self.CPUTechName);
				else
					local randCap = 3; -- First deploy only scouts

					if self:GetTeamFunds(self.CPUTeam) < self.InitialFunds * 0.40 then
						randCap = 13;
					elseif self:GetTeamFunds(self.CPUTeam) < self.InitialFunds * 0.55 then
						randCap = 9;
					elseif self:GetTeamFunds(self.CPUTeam) < self.InitialFunds * 0.70 then
						randCap = 7;
					elseif self:GetTeamFunds(self.CPUTeam) < self.InitialFunds * 0.85 then
						randCap = 5;
					end

					-- Still add some random to the mix
					--if math.random() < 0.15 then
					--	randCap = 13;
					--end

					local rand = math.random(randCap);

					if rand < 2 then
						self:CreateScoutDrop(safePosX, self.CPUTechName);
					elseif rand < 4 then
						self:CreateLightDrop(safePosX, self.CPUTechName);
					elseif rand < 6 then
						self:CreateMediumDrop(safePosX, self.CPUTechName);
					elseif rand < 8 then
						self:CreateHeavyDrop(safePosX, self.CPUTechName);
					else
						self:CreateArtilleryDrop(safePosX, self.CPUTechName);
					end
				end
			end
		end
	else
		local troops = 0;

		for actor in MovableMan.Actors do
			if actor.Team == self.CPUTeam then
				if actor.ClassName == "AHuman" or actor.ClassName == "ACrab" then
					troops = troops + 1;
					self:AddObjectivePoint("Terminate!", actor.AboveHUDPos, self.PlayerTeam, GameActivity.ARROWDOWN);
				end
			end
		end

		if troops < 2 then
			self.WinnerTeam = self.PlayerTeam;
			MovableMan:KillAllEnemyActors(self.WinnerTeam);
			ActivityMan:EndActivity();
			return;
		end
	end
end

function Siege:PlayerBrainsReachable()
	local reachable = true;

	SceneMan.Scene:UpdatePathFinding();

	for actor in MovableMan.Actors do
		if actor.Team == self.PlayerTeam and actor:IsInGroup("Brains") then
			local pathCost = SceneMan.Scene:CalculatePath(actor.Pos, Vector(actor.Pos.X, 0), false, 0);
			if pathCost > 10000 then
				reachable = false;
				break;
			end
		end
	end

	return reachable;
end

function Siege:CreateHeavyDrop(xPosLZ, techName)
	local Craft = RandomACDropShip("Craft", techName); -- Pick a craft to deliver with
	if Craft then
		-- The max allowed weight of this craft plus cargo
		local craftMaxMass = Craft.MaxInventoryMass;
		if craftMaxMass < 0 then
			craftMaxMass = math.huge;
		elseif craftMaxMass < 1 then
			Craft = RandomACDropShip("Craft", 0); -- MaxMass not defined
			craftMaxMass = Craft.MaxInventoryMass;
		end

		Craft.Team = self.CPUTeam;
		Craft.Pos = Vector(xPosLZ, -30); -- Set the spawn point of the craft

		for i = 1, Craft.MaxPassengers do
			if math.random() < self:GetCrabToHumanSpawnRatio(PresetMan:GetModuleID(techName)) then
				Passenger = self:CreateCrab(Actor.AIMODE_GOTO, techName);
			elseif RangeRand(0, 105) < self.Difficulty then
				Passenger = self:CreateHeavyInfantry(Actor.AIMODE_GOTO,techName);
			else
				Passenger = self:CreateRandomInfantry(Actor.AIMODE_GOTO,techName);
			end

			if Passenger then
				Passenger.AIMode = Actor.AIMODE_BRAINHUNT;

				Craft:AddInventoryItem(Passenger);

				if Craft.InventoryMass > craftMaxMass then
					break;
				end
			end
		end

		-- Subtract the total value of the craft+cargo from the CPU team's funds
		self:ChangeTeamFunds(-Craft:GetTotalValue(PresetMan:GetModuleID(techName), 2), self.CPUTeam);

		-- Spawn the Craft onto the scene
		MovableMan:AddActor(Craft);
	end
end

function Siege:CreateSWATDrop(xPosLZ, techName)
	local Craft = RandomACDropShip("Craft", techName); -- Pick a craft to deliver with
	if Craft then
		-- The max allowed weight of this craft plus cargo
		local craftMaxMass = Craft.MaxInventoryMass;
		if craftMaxMass < 0 then
			craftMaxMass = math.huge;
		elseif craftMaxMass < 1 then
			Craft = RandomACDropShip("Craft", 0); -- MaxMass not defined
			craftMaxMass = Craft.MaxInventoryMass;
		end

		Craft.Team = self.CPUTeam;
		Craft.Pos = Vector(xPosLZ, -30); -- Set the spawn point of the craft

		for i = 1, Craft.MaxPassengers do
			Passenger = self:CreateSWATInfantry(Actor.AIMODE_GOTO,techName);

			if Passenger then
				Passenger.AIMode = Actor.AIMODE_BRAINHUNT;

				Craft:AddInventoryItem(Passenger);

				if Craft.InventoryMass > craftMaxMass * 0.75 then
					break;
				end
			end
		end

		-- Subtract the total value of the craft+cargo from the CPU team's funds
		self:ChangeTeamFunds(-Craft:GetTotalValue(PresetMan:GetModuleID(techName), 2), self.CPUTeam);

		-- Spawn the Craft onto the scene
		MovableMan:AddActor(Craft);
	end
end


function Siege:CreateArtilleryDrop(xPosLZ, techName)
	local Craft = RandomACDropShip("Craft", techName); -- Pick a craft to deliver with
	if Craft then
		-- The max allowed weight of this craft plus cargo
		local craftMaxMass = Craft.MaxInventoryMass;
		if craftMaxMass < 0 then
			craftMaxMass = math.huge;
		elseif craftMaxMass < 1 then
			Craft = RandomACDropShip("Craft", 0); -- MaxMass not defined
			craftMaxMass = Craft.MaxInventoryMass;
		end

		Craft.Team = self.CPUTeam;
		Craft.Pos = Vector(xPosLZ, -30); -- Set the spawn point of the craft

		for i = 1, Craft.MaxPassengers do
			Passenger = self:CreateArtilleryInfantry(Actor.AIMODE_BRAINHUNT, techName);
			if Passenger then
				Passenger.AIMode = Actor.AIMODE_BRAINHUNT;

				Craft:AddInventoryItem(Passenger);

				if Craft.InventoryMass > craftMaxMass then
					break;
				end
			end
		end

		-- Subtract the total value of the craft+cargo from the CPU team's funds
		self:ChangeTeamFunds(-Craft:GetTotalValue(PresetMan:GetModuleID(techName), 2), self.CPUTeam);

		-- Spawn the Craft onto the scene
		MovableMan:AddActor(Craft);
	end
end

function Siege:CreateMediumDrop(xPosLZ, techName)
	-- Pick a craft to deliver with
	local Craft;
	if math.random() < 0.6 then
		Craft = RandomACDropShip("Craft", techName);
	else
		Craft = RandomACRocket("Craft", techName);
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

		for i = 1, Craft.MaxPassengers do
			if RangeRand(-5, 125) < self.Difficulty then
				Passenger = self:CreateMediumInfantry(Actor.AIMODE_GOTO,techName);
			elseif math.random() < 0.65 then
				Passenger = self:CreateRandomInfantry(Actor.AIMODE_GOTO,techName);
			else
				Passenger = self:CreateLightInfantry(Actor.AIMODE_GOTO,techName);
			end

			if Passenger then
				Passenger.AIMode = Actor.AIMODE_BRAINHUNT;

				Craft:AddInventoryItem(Passenger);

				if Craft.InventoryMass > craftMaxMass then
					break;
				end
			end
		end

		-- Subtract the total value of the craft+cargo from the CPU team's funds
		self:ChangeTeamFunds(-Craft:GetTotalValue(PresetMan:GetModuleID(techName), 2), self.CPUTeam);

		-- Spawn the Craft onto the scene
		MovableMan:AddActor(Craft);
	end
end

function Siege:CreateLightDrop(xPosLZ, techName)
	-- Pick a craft to deliver with
	local Craft;
	if math.random() < 0.6 then
		Craft = RandomACDropShip("Craft", techName);
	else
		Craft = RandomACRocket("Craft", techName);
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

		for i = 1, Craft.MaxPassengers do
			if RangeRand(10, 200) < self.Difficulty then
				Passenger = self:CreateMediumInfantry(Actor.AIMODE_GOTO,techName);
			else
				Passenger = self:CreateLightInfantry(Actor.AIMODE_GOTO,techName);
			end

			if Passenger then
				Passenger.AIMode = Actor.AIMODE_BRAINHUNT;

				Craft:AddInventoryItem(Passenger);

				if Craft.InventoryMass > craftMaxMass then
					break;
				end
			end
		end

		-- Subtract the total value of the craft+cargo from the CPU team's funds
		self:ChangeTeamFunds(-Craft:GetTotalValue(PresetMan:GetModuleID(techName), 2), self.CPUTeam);

		-- Spawn the Craft onto the scene
		MovableMan:AddActor(Craft);
	end
end

function Siege:CreateEngineerDrop(xPosLZ, techName)
	-- Pick a craft to deliver with
	local Craft;
	if math.random() < 0.6 then
		Craft = RandomACDropShip("Craft", techName);
	else
		Craft = RandomACRocket("Craft", techName);
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

		for i = 1, Craft.MaxPassengers do
			Passenger = self:CreateEngineer(Actor.AIMODE_GOTO,techName);

			if Passenger then
				Passenger.AIMode = Actor.AIMODE_GOLDDIG;

				Craft:AddInventoryItem(Passenger);

				if Craft.InventoryMass > craftMaxMass then
					break;
				end
			end
		end

		-- Subtract the total value of the craft+cargo from the CPU team's funds
		self:ChangeTeamFunds(-Craft:GetTotalValue(PresetMan:GetModuleID(techName), 2), self.CPUTeam);

		-- Spawn the Craft onto the scene
		MovableMan:AddActor(Craft);
	end
end


function Siege:CreateScoutDrop(xPosLZ, techName)
	-- Pick a craft to deliver with
	local Craft, actorsInCargo;
	if math.random() < 0.6 then
		Craft = RandomACDropShip("Craft", techName);
	else
		Craft = RandomACRocket("Craft", techName);
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

		for i = 1, Craft.MaxPassengers do
			if math.random() < 0.3 then
				Passenger = self:CreateLightInfantry(Actor.AIMODE_GOTO,techName);
			else
				Passenger = self:CreateScoutInfantry(Actor.AIMODE_GOTO,techName);
			end

			if Passenger then
				Passenger.AIMode = Actor.AIMODE_BRAINHUNT;

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


function Siege:CreateCrab(mode, techName)
	local Passenger = RandomACrab("Actors - Mecha", techName);
	if Passenger then
		-- Set AI mode and team so it knows who and what to fight for!
		Passenger.AIMode = mode or Actor.AIMODE_GOTO;
		Passenger.Team = self.CPUTeam;
		return Passenger;
	end
end

function Siege:CreateTurret(mode, techName)
	local Passenger = RandomACrab("Turret", techName);
	if Passenger then
		-- Set AI mode and team so it knows who and what to fight for!
		Passenger.AIMode = mode or Actor.AIMODE_SENTRY;
		Passenger.Team = self.CPUTeam;
		return Passenger;
	end
end

-- Get any Actor from the CPU's native tech
function Siege:CreateRandomInfantry(mode, techName)
	local	Passenger = RandomAHuman("Actors", techName);
	if Passenger then
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Primary", techName));
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", techName));

		if math.random() < 0.4 then
			Passenger:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", techName));
			if math.random() < 0.5 then
				Passenger:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", techName));
			end
		elseif not self:PlayerBrainsReachable() then
			Passenger:AddInventoryItem(RandomHDFirearm("Tools - Diggers", techName));
		else
			if self.DiggersEssential then
				Passenger:AddInventoryItem(CreateHDFirearm("Light Digger", "Base.rte"));
			end
		end

		-- Set AI mode and team so it knows who and what to fight for!
		Passenger.AIMode = mode or Actor.AIMODE_GOTO;
		Passenger.Team = self.CPUTeam;
		return Passenger;
	end
end

function Siege:CreateSWATInfantry(mode, techName)
	local	Passenger = RandomAHuman("Actors - Light", techName);
	if Passenger then
		Passenger:AddInventoryItem(RandomHDFirearm("Shields", techName));
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", techName));

		if math.random() < 0.2 then
			Passenger:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", techName));
		end
		if math.random() < 0.2 then
			Passenger:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", techName));
		end
		if math.random() < 0.2 then
			Passenger:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", techName));
		end

		if self.DiggersEssential then
			Passenger:AddInventoryItem(CreateHDFirearm("Light Digger", "Base.rte"));
		end

		-- Set AI mode and team so it knows who and what to fight for!
		Passenger.AIMode = mode or Actor.AIMODE_GOTO;
		Passenger.Team = self.CPUTeam;
		return Passenger;
	end
end

function Siege:CreateLightInfantry(mode, techName)
	local	Passenger = RandomAHuman("Actors - Light", techName);
	if Passenger then
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Light", techName));
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", techName));

		if math.random() < 0.2 then
			Passenger:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", techName));
		end

		if self.DiggersEssential then
			Passenger:AddInventoryItem(CreateHDFirearm("Light Digger", "Base.rte"));
		end

		-- Set AI mode and team so it knows who and what to fight for!
		Passenger.AIMode = mode or Actor.AIMODE_GOTO;
		Passenger.Team = self.CPUTeam;
		return Passenger;
	end
end

function Siege:CreateHeavyInfantry(mode, techName)
	local	Passenger = RandomAHuman("Actors - Heavy", techName);
	if Passenger then
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Heavy", techName));
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", techName));

		if math.random() < 0.6 then
			Passenger:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", techName));
			Passenger:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", techName));
			if math.random() < 0.4 then
				Passenger:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", techName));
			end
		end

		if not self:PlayerBrainsReachable() then
			Passenger:AddInventoryItem(RandomHDFirearm("Tools - Diggers", techName));
		else
			if self.DiggersEssential then
				Passenger:AddInventoryItem(CreateHDFirearm("Light Digger", "Base.rte"));
			end
		end

		-- Set AI mode and team so it knows who and what to fight for!
		Passenger.AIMode = mode or Actor.AIMODE_BRAINHUNT;
		Passenger.Team = self.CPUTeam;
		return Passenger;
	end
end


function Siege:CreateArtilleryInfantry(mode, techName)
	local	Passenger = RandomAHuman("Actors - Heavy", techName);
	if Passenger then
		local weapon = RandomHDFirearm("Weapons - Explosive", techName);
		if weapon then
			Passenger:AddInventoryItem(weapon);
		end
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Heavy", techName));

		if not self:PlayerBrainsReachable() then
			Passenger:AddInventoryItem(RandomHDFirearm("Tools - Diggers", techName));
		else
			if self.DiggersEssential then
				Passenger:AddInventoryItem(CreateHDFirearm("Light Digger", "Base.rte"));
			end
		end

		Passenger.SightDistance = Passenger.SightDistance * 10;
		Passenger.AimDistance = Passenger.AimDistance * 10;

		-- Set AI mode and team so it knows who and what to fight for!
		Passenger.AIMode = mode or Actor.AIMODE_BRAINHUNT;
		Passenger.Team = self.CPUTeam;
		return Passenger;
	end
end


function Siege:CreateMediumInfantry(mode, techName)
	local	Passenger = RandomAHuman("Actors - Heavy", techName);
	if Passenger then
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Light", techName));
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", techName));

		if self.DiggersEssential then
			Passenger:AddInventoryItem(CreateHDFirearm("Light Digger", "Base.rte"));
		end

		-- Set AI mode and team so it knows who and what to fight for!
		Passenger.AIMode = mode or Actor.AIMODE_BRAINHUNT;
		Passenger.Team = self.CPUTeam;
		return Passenger;
	end
end

function Siege:CreateScoutInfantry(mode, techName)
	local	Passenger = RandomAHuman("Actors - Light", techName);
	if Passenger then
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", techName));

		if math.random() < 0.6 then
			Passenger:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", techName));
		else
			Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", techName));
		end

		if self.DiggersEssential then
			Passenger:AddInventoryItem(CreateHDFirearm("Light Digger", "Base.rte"));
		end

		-- Set AI mode and team so it knows who and what to fight for!
		Passenger.AIMode = mode or Actor.AIMODE_BRAINHUNT;
		Passenger.Team = self.CPUTeam;
		return Passenger;
	end
end

function Siege:CreateSniper(mode, techName)
	local	Passenger;
	if math.random() < 0.7 then
		Passenger = RandomAHuman("Actors - Light", techName);
	else
		Passenger = RandomAHuman("Actors - Heavy", techName);
	end

	if Passenger then
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Sniper", techName));
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", techName));

		if self.DiggersEssential then
			Passenger:AddInventoryItem(CreateHDFirearm("Light Digger", "Base.rte"));
		end

		-- Set AI mode and team so it knows who and what to fight for!
		Passenger.AIMode = mode or Actor.AIMODE_BRAINHUNT;
		Passenger.Team = self.CPUTeam;
		return Passenger;
	end
end

function Siege:CreateEngineer(mode, techName)
	local Passenger = RandomAHuman("Actors - Light", techName);
	if Passenger then
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Light", techName));
		Passenger:AddInventoryItem(CreateHDFirearm("Medium Digger", "Base.rte"));

		-- Set AI mode and team so it knows who and what to fight for!
		Passenger.AIMode = mode or Actor.AIMODE_GOLDDIG;
		Passenger.Team = self.CPUTeam;
		return Passenger;
	end
end

function Siege:CreateAntiAir(mode, techName)
	local Passenger = RandomACrab("Anti-Air", techName);
	if Passenger then
		-- Set AI mode and team so it knows who and what to fight for!
		Passenger.AIMode = mode or Actor.AIMODE_SENTRY;
		Passenger.Team = self.CPUTeam;
		return Passenger;
	end
end

function Siege:CreateBrainBot(mode, team, techName)
	local Act = RandomAHuman("Brains", techName);
	if Act then
		Act:AddInventoryItem(RandomHDFirearm("Weapons - Light", techName));
		Act:AddInventoryItem(CreateHDFirearm("Medium Digger", "Base.rte"));

		if math.random() < 0.5 then
			Act:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", techName));
		end

		-- Set AI mode and team so it knows who and what to fight for!
		Act.AIMode = mode or Actor.AIMODE_SENTRY;
		Act.Team = team;
		return Act;
	end
end
