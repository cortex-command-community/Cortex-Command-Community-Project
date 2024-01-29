package.loaded.Constants = nil; require("Constants");

dofile("Browncoats.rte/Activities/RefineryAssaultFunctions.lua");

function RefineryAssault:OnGlobalMessage(message, object)

	self:HandleMessage(message, object);

end

function RefineryAssault:OnMessage(message, object)

	self:HandleMessage(message, object);

end

function RefineryAssault:SetupBuyDoorAreaTable(self, area)

	-- remove BuyDoorArea_ from the area name to get our table key
	local areaKey = string.sub(area.Name, 13, -1);
	
	--print("area key: " .. areaKey);

	self.saveTable.buyDoorTables[areaKey] = {};
	
	-- does not work, actors are not added properly yet at this stage
	
	-- for box in area.Boxes do
		-- print("onebox")
		-- for mo in MovableMan:GetMOsInBox(box, -1, false) do
			-- print(mo)
			-- if mo.PresetName == "Reinforcement Door" then
				-- table.insert(self.saveTable.buyDoorTables.All, mo)
				-- self.saveTable.buyDoorTables[areaKey][tostring(#self.saveTable.buyDoorTables.All)] = mo;
			-- end
		-- end
	-- end
	
	for mo in MovableMan.AddedActors do
		if mo.PresetName == "Reinforcement Door" and area:IsInside(mo.Pos) then
			table.insert(self.saveTable.buyDoorTables.All, mo)
			self.saveTable.buyDoorTables[areaKey][tonumber(#self.saveTable.buyDoorTables.All)] = mo;
		end
	end
		

end


function RefineryAssault:GetAIFunds(team)

	if team == self.humanTeam then
		return self.humanAIFunds;
	elseif team == self.aiTeam then
		return self:GetTeamFunds(self.aiTeam);
	end

end

function RefineryAssault:ChangeAIFunds(team, changeAmount)

	if team == self.humanTeam then
		self.humanAIFunds = self.humanAIFunds + changeAmount;
	elseif team == self.aiTeam then
		self:ChangeTeamFunds(changeAmount, self.aiTeam);
	end

end

function RefineryAssault:UpdateFunds()

	-- Gold increasing for teams
	
	local playerFunds = self:GetTeamFunds(self.humanTeam);
	local aiTeamFunds = self:GetTeamFunds(self.aiTeam);

	if self.goldTimer:IsPastSimMS(self.goldIncreaseDelay) then
	
		self.goldTimer:Reset();
		
		self:SetTeamFunds(playerFunds + self.playerGoldIncreaseAmount, self.humanTeam);
		self.humanAIFunds = self.humanAIFunds + self.humanAIGoldIncreaseAmount;
		
		self:SetTeamFunds(aiTeamFunds + self.aiTeamGoldIncreaseAmount, self.aiTeam);
	
	end

	-- Debug view
	
	if self.HUDHandler:GetCameraPanEventCount(self.humanTeam) == 0 then
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do	

			if self:PlayerActive(player) and self:PlayerHuman(player) then
			
				local pos = CameraMan:GetOffset(player);
				pos.X = pos.X + FrameMan.PlayerScreenWidth * 0.5;
				--print(pos)
				local yOffset = FrameMan.PlayerScreenHeight * 0.87;
				local xOffset = Vector(FrameMan.PlayerScreenWidth * 0.33, 0);
				pos.Y = pos.Y + yOffset
				
				local textPos = Vector(pos.X, pos.Y - 20);
				--PrimitiveMan:DrawTextPrimitive(textPos, "aiteam: " .. tostring(aiTeamFunds), false, 1)
				
				local textPos = Vector(pos.X - 100, pos.Y - 20);
				--PrimitiveMan:DrawTextPrimitive(textPos, "humanai: " ..  tostring(self.humanAIFunds), false, 1)

				
			end
		end	
	end

end

-----------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------
-- Game functions
-----------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------




-----------------------------------------------------------------------------------------
-- Start Activity
-----------------------------------------------------------------------------------------

function RefineryAssault:StartActivity(newGame)
	print("START! -- RefineryAssault:StartActivity()!");
	
	self.verboseLogging = true;
	
	self.humansAreControllingAlliedActors = false;
	
	self.humanTeam = Activity.TEAM_1;
	self.aiTeam = Activity.TEAM_2;
	self.humanTeamTech = PresetMan:GetModuleID(self:GetTeamTech(self.humanTeam));
	self.aiTeamTech = PresetMan:GetModuleID(self:GetTeamTech(self.aiTeam));
	
	self.humanPlayers = {};
	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) and self:GetTeamOfPlayer(player) == self.humanTeam then
			self.humanPlayers[#self.humanPlayers + 1] = player;
		end
	end
	
	
	self.goldTimer = Timer();
	self.goldIncreaseDelay = 4000;
	
	self.playerGoldIncreaseAmount = 5;	
	self.humanAIGoldIncreaseAmount = 50;
	
	self.aiTeamGoldIncreaseAmount = 0;
	
	self.humanAIFunds = 1;
	
	
	self.saveLoadHandler = require("Activities/Utility/SaveLoadHandler");
	self.saveLoadHandler:Initialize(self.verboseLogging);
	
	self.tacticsHandler = require("Activities/Utility/TacticsHandler");
	self.tacticsHandler:Initialize(self, newGame, 2, 5, 45000, self.verboseLogging);
	
	self.dockingHandler = require("Activities/Utility/DockingHandler");
	self.dockingHandler:Initialize(self, newGame, false, self.verboseLogging);
	
	self.buyDoorHandler = require("Activities/Utility/BuyDoorHandler");
	self.buyDoorHandler:Initialize(self, newGame);
	
	self.deliveryCreationHandler = require("Activities/Utility/DeliveryCreationHandler");
	self.deliveryCreationHandler:Initialize(self, self.verboseLogging);
	
	self.HUDHandler = require("Activities/Utility/HUDHandler");
	self.HUDHandler:Initialize(self, newGame, self.verboseLogging);
	
	self.particleUtility = require("Scripts/Utility/ParticleUtility");
	
	self.MOUtility = require("Scripts/Utility/MOUtility");	
	
	-- Stage stuff
	
	--2
	self.stage2HoldTimer = Timer();
	self.stage2TimeToHoldConsoles = 5000;
	
	self.stageFunctionTable = {};
	table.insert(self.stageFunctionTable, self.MonitorStage1);
	table.insert(self.stageFunctionTable, self.MonitorStage2);
	table.insert(self.stageFunctionTable, self.MonitorStage3);
	table.insert(self.stageFunctionTable, self.MonitorStage4);
	table.insert(self.stageFunctionTable, self.MonitorStage5);
	table.insert(self.stageFunctionTable, self.MonitorStage6);
	table.insert(self.stageFunctionTable, self.MonitorStage7);
	table.insert(self.stageFunctionTable, self.MonitorStage8);
	table.insert(self.stageFunctionTable, self.MonitorStage9);
	table.insert(self.stageFunctionTable, self.MonitorStage10);
	
	-- 6
	
	-- Find stage 6 subcommander door
	for mo in MovableMan.AddedActors do
		if mo.PresetName == "Refinery Subcommander Door" then
			self.stage6SubcommanderDoor = mo;
			mo.Team = 1;
		end
	end

	SceneMan.Scene:AddNavigatableArea("Mission Stage Area 1");
	SceneMan.Scene:AddNavigatableArea("Mission Stage Area 2");
	SceneMan.Scene:AddNavigatableArea("Mission Stage Area 3");
	SceneMan.Scene:AddNavigatableArea("Mission Stage Area 4");
	
	if newGame then
		self.saveTable = {};
		
		self.saveTable.activeDocks = {1, 2};
		self.deliveryCreationHandler:RemoveAvailablePreset(-1, "Tubby Rocket");
		self.deliveryCreationHandler:RemoveAvailablePreset(-1, "Drop Crate");
		
		self.deliveryCreationHandler:RemoveAvailablePreset(self.aiTeam, "Browncoat Boss");
		self.deliveryCreationHandler:RemoveAvailablePreset(self.aiTeam, "MG-85 Manbreaker");
		
		self.saveTable.doorsToConstantlyReset = {};

		for actor in MovableMan.AddedActors do
			if actor:NumberValueExists("BossVaultDoor") then
				table.insert(self.saveTable.doorsToConstantlyReset, actor);
			elseif actor.PresetName == "Refinery Objective Prison Door" then
				self.saveTable.roninPrisonerDoor = actor;
				table.insert(self.saveTable.doorsToConstantlyReset, actor);
			end
			
		end
		
		-- Always active base task for defenders
		self.tacticsHandler:AddTask("Brainhunt", self.aiTeam, Vector(0, 0), "Brainhunt", 2);
	
		-- Set up buy door areas
		-- it would be great to handle this generically, but we have specific
		-- areas and capturing of buy doors within those areas etc and that's
		-- not generic enough anymore
		
		-- the All table holds all buy doors so we can supply them to the handler,
		-- and also get what index to save in the area-specific tables.
		-- so each area-specific table will be made up of the actual indexes
		-- in the All table which we can SendCustomOrder with.
		
		self.saveTable.buyDoorTables = {};
		self.saveTable.buyDoorTables.All = {};
		
		local area = SceneMan.Scene:GetOptionalArea("BuyDoorArea_LC1");
		self:SetupBuyDoorAreaTable(self, area);
		
		area = SceneMan.Scene:GetOptionalArea("BuyDoorArea_LC2");
		self:SetupBuyDoorAreaTable(self, area);
		
		area = SceneMan.Scene:GetOptionalArea("BuyDoorArea_S3_1");
		self:SetupBuyDoorAreaTable(self, area);

		area = SceneMan.Scene:GetOptionalArea("BuyDoorArea_S3_2");
		self:SetupBuyDoorAreaTable(self, area);

		area = SceneMan.Scene:GetOptionalArea("BuyDoorArea_S3_3");
		self:SetupBuyDoorAreaTable(self, area);	

		area = SceneMan.Scene:GetOptionalArea("BuyDoorArea_S4_1");
		self:SetupBuyDoorAreaTable(self, area);
		
		area = SceneMan.Scene:GetOptionalArea("BuyDoorArea_S4_2");
		self:SetupBuyDoorAreaTable(self, area);
		
		area = SceneMan.Scene:GetOptionalArea("BuyDoorArea_S4_3");
		self:SetupBuyDoorAreaTable(self, area);
		
		area = SceneMan.Scene:GetOptionalArea("BuyDoorArea_S4_4");
		self:SetupBuyDoorAreaTable(self, area);
		
		area = SceneMan.Scene:GetOptionalArea("BuyDoorArea_S4_5");
		self:SetupBuyDoorAreaTable(self, area);

		area = SceneMan.Scene:GetOptionalArea("BuyDoorArea_S4_6");
		self:SetupBuyDoorAreaTable(self, area);

		self.buyDoorHandler:ReplaceBuyDoorTable(self.saveTable.buyDoorTables.All);
		
		self.saveTable.buyDoorTables.teamAreas = {};
		self.saveTable.buyDoorTables.teamAreas[self.humanTeam] = {};
		self.saveTable.buyDoorTables.teamAreas[self.aiTeam] = {"LC1", "LC2", "S3_1", "S3_2", "S3_3", "S4_1", "S4_2", "S4_3", "S4_4", "S4_5", "S4_6"};
		
		for k, v in pairs(self.saveTable.buyDoorTables.All) do
			--print(v)
			v.Team = self.aiTeam;
		end
		
		-- Stage function table
		
		self.Stage = 1;
		
		self:SetupFirstStage();

		local automoverController = CreateActor("Invisible Automover Controller", "Base.rte");
		automoverController.Pos = Vector();
		automoverController.Team = self.aiTeam;
		MovableMan:AddActor(automoverController);
		
		-- Tell capturables to deactivate, we'll activate them as we go along
		MovableMan:SendGlobalMessage("DeactivateCapturable_RefineryLCHackConsole1");
		MovableMan:SendGlobalMessage("DeactivateCapturable_RefineryLCHackConsole2");
		MovableMan:SendGlobalMessage("DeactivateCapturable_RefineryS3BuyDoorConsole1");
		MovableMan:SendGlobalMessage("DeactivateCapturable_RefineryS3BuyDoorConsole2");
		MovableMan:SendGlobalMessage("DeactivateCapturable_RefineryS3BuyDoorConsole3");
		MovableMan:SendGlobalMessage("DeactivateCapturable_RefineryS3BuyDoorConsole4");
		MovableMan:SendGlobalMessage("DeactivateCapturable_RefineryS3DrillOverloadConsole");
		MovableMan:SendGlobalMessage("DeactivateCapturable_RefineryS3OilCapturable");
		MovableMan:SendGlobalMessage("DeactivateCapturable_RefineryS4BuyDoorConsole1");
		MovableMan:SendGlobalMessage("DeactivateCapturable_RefineryS4BuyDoorConsole2");
		MovableMan:SendGlobalMessage("DeactivateCapturable_RefineryS4BuyDoorConsole3");
		MovableMan:SendGlobalMessage("DeactivateCapturable_RefineryS4BuyDoorConsole4");
		MovableMan:SendGlobalMessage("DeactivateCapturable_RefineryS4BuyDoorConsole5");
		MovableMan:SendGlobalMessage("DeactivateCapturable_RefineryS7AuxAuthConsole");
		MovableMan:SendGlobalMessage("DeactivateCapturable_RefineryS10FinalConsole");

		-- Stage stuff

		--3
		self.stage3ConsolesBroken = 0;
		
		for particle in MovableMan.Particles do
			if particle.PresetName == "Browncoat Refinery Console Breakable Objective" then
				particle.MissionCritical = true;
			end
		end
		
		for particle in MovableMan.Particles do
			if particle.PresetName == "Browncoat Refinery Generator Breakable Objective" then
				particle.MissionCritical = true;
			end
		end
	
	else
		self:ResumeLoadedGame();
	end
	
	-- for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		-- if self:PlayerActive(player) and self:PlayerHuman(player) then
			-- -- Check if we already have a brain assigned
			-- if not self:GetPlayerBrain(player) then
				-- local foundBrain = MovableMan:GetUnassignedBrain(self:GetTeamOfPlayer(player));
				-- -- If we can't find an unassigned brain in the scene to give each player, then force to go into editing mode to place one
				-- if not foundBrain then
					-- self.ActivityState = Activity.EDITING;
					-- -- Open all doors so we can do pathfinding through them with the brain placement
					-- MovableMan:OpenAllDoors(true, Activity.NOTEAM);
					-- AudioMan:ClearMusicQueue();
					-- AudioMan:PlayMusic("Base.rte/Music/dBSoundworks/ccambient4.ogg", -1, -1);
					-- self:SetLandingZone(Vector(player*SceneMan.SceneWidth/4, 0), player);
				-- else
					-- -- Set the found brain to be the selected actor at start
					-- self:SetPlayerBrain(foundBrain, player);
					-- self:SwitchToActor(foundBrain, player, self:GetTeamOfPlayer(player));
					-- self:SetLandingZone(self:GetPlayerBrain(player).Pos, player);
					-- -- Set the observation target to the brain, so that if/when it dies, the view flies to it in observation mode
					-- self:SetObservationTarget(self:GetPlayerBrain(player).Pos, player);
				-- end
			-- end
		-- end
	-- end	
	
end

function RefineryAssault:ResumeLoadedGame()

	print("loading local refineryassault save table...");
	self.saveTable = self.saveLoadHandler:ReadSavedStringAsTable("saveTable");
	print("loaded local refineryassault save table!");
	
	self.Stage = self:LoadNumber("stage");

	self.goldTimer.ElapsedRealTimeMS = self:LoadNumber("goldTimer");
	
	self.stage2HoldingBothConsoles = self:LoadNumber("stage2HoldingBothConsoles") == 1 and true or false;
	self.stage2HoldTimer.ElapsedRealTimeMS = self:LoadNumber("stage2HoldTimer");
	
	self.stage3ConsolesBroken = self:LoadNumber("stage3ConsolesBroken");
	self.stage3DrillOverloaded = self:LoadNumber("stage3DrillOverloaded") == 1 and true or false;
	
	-- Handlers
	self.tacticsHandler:OnLoad(self.saveLoadHandler);
	self.dockingHandler:OnLoad(self.saveLoadHandler);
	self.buyDoorHandler:OnLoad(self.saveLoadHandler);
	self.deliveryCreationHandler:OnLoad(self.saveLoadHandler);
	self.HUDHandler:OnLoad(self.saveLoadHandler);
	
	self.MOUtility:OnLoad(self.saveLoadHandler);
	
	self.buyDoorHandler:ReplaceBuyDoorTable(self.saveTable.buyDoorTables.All);
		
end

function RefineryAssault:OnSave()
	
	self.saveLoadHandler:SaveTableAsString("saveTable", self.saveTable);
	
	self:SaveNumber("stage", self.Stage);

	self:SaveNumber("goldTimer", self.goldTimer.ElapsedRealTimeMS);
	
	self:SaveNumber("stage2HoldingBothConsoles", self.stage2HoldingBothConsoles and 1 or 0);
	self:SaveNumber("stage2HoldTimer", self.stage2HoldTimer.ElapsedRealTimeMS);
	
	self:SaveNumber("stage3ConsolesBroken", self.stage2ConsolesBroken or 0);
	self:SaveNumber("stage3DrillOverloaded", self.stage2DrillOverloaded and 1 or 0);
	
	-- Handlers
	self.tacticsHandler:OnSave(self.saveLoadHandler);
	self.dockingHandler:OnSave(self.saveLoadHandler);
	self.buyDoorHandler:OnSave(self.saveLoadHandler);
	self.deliveryCreationHandler:OnSave(self.saveLoadHandler);
	self.HUDHandler:OnSave(self.saveLoadHandler);
	
	self.MOUtility:OnSave(self.saveLoadHandler);
	
end

-----------------------------------------------------------------------------------------
-- Pause Activity
-----------------------------------------------------------------------------------------

function RefineryAssault:PauseActivity(pause)
	print("PAUSE! -- RefineryAssault:PauseActivity()!");
end

-----------------------------------------------------------------------------------------
-- End Activity
-----------------------------------------------------------------------------------------

function RefineryAssault:EndActivity()
	print("END! -- RefineryAssault:EndActivity()!");
end

-----------------------------------------------------------------------------------------
-- Update Activity
-----------------------------------------------------------------------------------------

function RefineryAssault:UpdateActivity()

	-- if UInputMan:KeyPressed(Key.F) and UInputMan:KeyHeld(Key.SPACE) then
		-- self.ActivityState = Activity.EDITING;

		-- -- Remove and refund the player's brains (otherwise edit mode does not work)
		-- for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			-- if self:PlayerActive(player) and self:PlayerHuman(player) then
				-- local brain = self:GetPlayerBrain(player);
				-- if MovableMan:IsActor(brain) then
					-- self:ChangeTeamFunds(brain:GetTotalValue(0, 1), self:GetTeamOfPlayer(player));
					-- MovableMan:RemoveActor(brain);
				-- end
			-- end
		-- end
	-- end

	-- Monitor stage objectives
	
	self.stageFunc = self.stageFunctionTable[self.Stage];
	self:stageFunc();


	self:UpdateFunds();
	
	
	-- Seek tasks to create squads for
	-- Only human team uses docks
	
	local team, task = self.tacticsHandler:UpdateTacticsHandler();
	
	if task and self:GetAIFunds(team) > 0 then
		--print("gottask")
		local squad;
		if team == self.aiTeam then
			squad = self:SendBuyDoorDelivery(team, task);
		else
			if math.random() < 0.2 then
				squad = self:SendDockDelivery(team, task);
			else
				squad = self:SendBuyDoorDelivery(team, task);
			end
			if not squad then
				squad = self:SendBuyDoorDelivery(team, task);
			end
		end
		if squad then
			self.tacticsHandler:AddSquad(team, squad, task.Name, true);
		end
	end
	
	-- Update docking craft
	
	self.dockingHandler:UpdateDockingCraft();
	
	-- Update HUD handler
	
	self.HUDHandler:UpdateHUDHandler();
	
	-- Update MOUtility not needed yet
	
	--self.MOUtility:Update();
	

	for k, door in pairs(self.saveTable.doorsToConstantlyReset) do
		if not door or not MovableMan:ValidMO(door) then
		else
			ToADoor(door):ResetSensorTimer();
		end
	end
	
	if self.HUDHandler:GetCameraPanEventCount(self.humanTeam) > 0 and not self.saveTable.brainsInvincible then
		self.saveTable.brainsInvincible = true;
		for k, brain in pairs(self.saveTable.playerBrains) do
			self.MOUtility:SetMOUnhittable(brain, true);
		end
	elseif self.saveTable.brainsInvincible and self.HUDHandler:GetCameraPanEventCount(self.humanTeam) == 0 then
		self.saveTable.brainsInvincible = false;
		for k, brain in pairs(self.saveTable.playerBrains) do
			self.MOUtility:SetMOUnhittable(brain, false);
		end
	end
	
	
	if self.roninPrisonerMessageTimer and not self.roninPrisonerMessageTimer:IsPastSimMS(6000) then
		if self.saveTable.roninPrisonerLeader and MovableMan:ValidMO(self.saveTable.roninPrisonerLeader) then
			PrimitiveMan:DrawTextPrimitive(self.saveTable.roninPrisonerLeader.AboveHUDPos, "We are in your debt.", true, 1);
		end
	else
		if self.saveTable.roninPrisonerDoor and not MovableMan:ValidMO(self.saveTable.roninPrisonerDoor) then
			self:SendMessage("RefineryAssault_RoninPrisonerDoorBroken");
			self.saveTable.roninPrisonerDoor = nil;
		end
	end
	
	
	
	-- Debug
	
	if UInputMan.FlagAltState then

		-- Unlimit camera
		if UInputMan:KeyPressed(Key.KP_8) then
			self.HUDHandler:SetCameraMinimumAndMaximumX(self.humanTeam, 0, 999999);
		end
		
		-- AI funds
		if UInputMan:KeyPressed(Key.KP_7) then
			self:ChangeAIFunds(0, 200);
			self:ChangeAIFunds(1, 200);
		end
		
		if UInputMan:KeyPressed(Key.KP_6) then
			self:SendMessage("Captured_RefineryS3DockConsole")
		end
		
	end
	
	local debugDoorTrigger = UInputMan:KeyPressed(Key.J)	
	
	local debugTrigger = UInputMan:KeyPressed(Key.I)
	
	local debugRocketTrigger = UInputMan:KeyPressed(Key.U)
	
	if debugDoorTrigger then
	
		self:SendBuyDoorDelivery(self.humanTeam);
		
	end
	
	if debugTrigger then
	
		--self:SendDockDelivery(self.humanTeam, false);
		print("tried dropship")
		
	end
	
	if debugRocketTrigger then
	
		--self:SendDockDelivery(self.humanTeam, true);
		print("triedrocket")
		
	end	
end
