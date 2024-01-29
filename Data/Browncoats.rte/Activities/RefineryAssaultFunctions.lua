function RefineryAssault:HandleMessage(message, object)

	self.tacticsHandler:OnMessage(message, object);

	--print("activitygotmessage")
	
	if self.verboseLogging then
		--print("INFO: Refinery Assault received message: " .. message);
		--print("INFO: Context as follows:")
		--print(object);
	end
	
	--print(message)
	--print(object)
	
	-- this is ugly, but there's no way to avoid this stuff except hiding it away even harder than in this separate script...

	if message == "Captured_RefineryLCHackConsole1" then
	
		self.stage2HoldTimer:Reset();

		print(self.humanTeam .. " team vs object: " .. object);
		
		if object == self.humanTeam then
		
			print("HUMAN CAPTURED 1")
			
			self.stage2HoldingLC1 = true;
		
			-- if we have the other one, we have both, initiate win condition timer
			if self.stage2HoldingLC2 then
				self.stage2HoldingBothConsoles = true;
			end
	
			table.insert(self.saveTable.buyDoorTables.teamAreas[self.humanTeam], "LC1");
			self:RemoveStringFromTable("LC1", self.saveTable.buyDoorTables.teamAreas[self.aiTeam]);
			
			for k, v in pairs(self.saveTable.buyDoorTables.LC1) do
				v.Team = self.humanTeam;
			end
		else
			print("NOTHUMAN CAPPED 1");
			print(self.humanTeam .. " team vs object: " .. object);
			self.stage2HoldingLC1 = false;
			self.stage2HoldingBothConsoles = false;
		
			table.insert(self.saveTable.buyDoorTables.teamAreas[self.aiTeam], "LC1");
			self:RemoveStringFromTable("LC1", self.saveTable.buyDoorTables.teamAreas[self.humanTeam]);
			
			for k, v in pairs(self.saveTable.buyDoorTables.LC1) do
				v.Team = self.aiTeam;
			end		
		end
		
		-- as soon as any of the hack consoles are captured, we don't wanna bother with the stage 1 counterattack anymore.
		self.tacticsHandler:RemoveTask("Counterattack", self.aiTeam);
	

	elseif message == "Captured_RefineryLCHackConsole2" then
	
		self.stage2HoldTimer:Reset();
	
		if object == self.humanTeam then
		
			print("HUMAN CAPTURED 2")
			
			self.stage2HoldingLC2 = true;
		
			-- if we have the other one, we have both, initiate win condition timer
			if self.stage2HoldingLC1 then
				self.stage2HoldingBothConsoles = true;
			end
	
			table.insert(self.saveTable.buyDoorTables.teamAreas[self.humanTeam], "LC2");
			self:RemoveStringFromTable("LC2", self.saveTable.buyDoorTables.teamAreas[self.aiTeam]);
			
			for k, v in pairs(self.saveTable.buyDoorTables.LC2) do
				v.Team = self.humanTeam;
			end
		else
			self.stage2HoldingLC2 = false;
			self.stage2HoldingBothConsoles = false;
			
			table.insert(self.saveTable.buyDoorTables.teamAreas[self.aiTeam], "LC2");
			self:RemoveStringFromTable("LC2", self.saveTable.buyDoorTables.teamAreas[self.humanTeam]);
			
			for k, v in pairs(self.saveTable.buyDoorTables.LC2) do
				v.Team = self.aiTeam;
			end		
		end
		
		-- as soon as any of the hack consoles are captured, we don't wanna bother with the stage 1 counterattack anymore.
		self.tacticsHandler:RemoveTask("Counterattack", self.aiTeam);
		
	elseif message == "Captured_RefineryS3DockConsole" then
	
		if self.verboseLogging then
			print("INFO: Refinery Assault detected capture of S3 dock console. Setting active docks...");
		end
	
		if self.saveTable.activeDocks[1] ~= 5 then -- ghetto check we haven't capped the s4 one
			self.saveTable.activeDocks = {3, 4};
			if self.verboseLogging then
				print("INFO: Refinery Assault set S3 active docks!");
			end
		else
			if self.verboseLogging then
				print("INFO: Refinery Assault detected S4 active docks. Active docks not re-set.");
			end
		end
		
	elseif message == "Captured_RefineryS3BuyDoorConsole1" then
		
		table.insert(self.saveTable.buyDoorTables.teamAreas[object], "S3_1");
		-- todo make this team selection better somehow... or maybe It Just Works. dunno. it's ugly.
		self.saveTable.buyDoorTables.teamAreas[(object + 1) % 2].S3_1 = nil;
		self:RemoveStringFromTable("S3_1", self.saveTable.buyDoorTables.teamAreas[(object + 1) % 2]);
		
		for k, v in pairs(self.saveTable.buyDoorTables.S3_1) do
			v.Team = object;
		end	
		
		-- dupe code woo
		if object == self.humanTeam then		
			local pos;
			for particle in MovableMan.Particles do
				if particle.PresetName == "Refinery S3 Buy Door Console 1" then
					pos = particle.Pos;
					break;
				end
			end		
			self.tacticsHandler:AddTask("Attack S3 Buy Door Console 1", self.aiTeam, pos, "Attack", 20);		
		else
			self.tacticsHandler:RemoveTask("Attack S3 Buy Door Console 1", self.aiTeam);		
		end
		
	elseif message == "Captured_RefineryS3BuyDoorConsole2" then
		
		table.insert(self.saveTable.buyDoorTables.teamAreas[object], "S3_2");
		-- todo make this team selection better somehow... or maybe It Just Works. dunno. it's ugly.
		self:RemoveStringFromTable("S3_2", self.saveTable.buyDoorTables.teamAreas[(object + 1) % 2]);
		
		for k, v in pairs(self.saveTable.buyDoorTables.S3_2) do
			v.Team = object;
		end
		
		-- dupe code woo
		if object == self.humanTeam then		
			local pos;
			for particle in MovableMan.Particles do
				if particle.PresetName == "Refinery S3 Buy Door Console 2" then
					pos = particle.Pos;
					break;
				end
			end		
			self.tacticsHandler:AddTask("Attack S3 Buy Door Console 2", self.aiTeam, pos, "Attack", 20);		
		else
			self.tacticsHandler:RemoveTask("Attack S3 Buy Door Console 2", self.aiTeam);		
		end
		
	elseif message == "Captured_RefineryS3BuyDoorConsole3" then
		
		table.insert(self.saveTable.buyDoorTables.teamAreas[object], "S3_3");
		-- todo make this team selection better somehow... or maybe It Just Works. dunno. it's ugly.
		self:RemoveStringFromTable("S3_3", self.saveTable.buyDoorTables.teamAreas[(object + 1) % 2]);
		
		for k, v in pairs(self.saveTable.buyDoorTables.S3_3) do
			v.Team = object;
		end
		
		-- dupe code woo
		if object == self.humanTeam then		
			local pos;
			for particle in MovableMan.Particles do
				if particle.PresetName == "Refinery S3 Buy Door Console 3" then
					pos = particle.Pos;
					break;
				end
			end		
			self.tacticsHandler:AddTask("Attack S3 Buy Door Console 3", self.aiTeam, pos, "Attack", 20);		
		else
			self.tacticsHandler:RemoveTask("Attack S3 Buy Door Console 3", self.aiTeam);		
		end
		
	elseif message == "Captured_RefineryS3OilCapturable" then
	
		self.humanAIGoldIncreaseAmount = self.humanAIGoldIncreaseAmount + 20;		
		self.playerGoldIncreaseAmount = self.playerGoldIncreaseAmount + 20;
		
		local soundContainer = CreateSoundContainer("Yskely Refinery Oil Spout Engage", "Browncoats.rte");
		soundContainer:Play(Vector(0, 0));
	
	elseif message == "Captured_RefineryS3FireWeaponryConsole" then	
		
		self.deliveryCreationHandler:AddAvailablePreset(self.humanTeam, "FL-200 Heatlance", "HDFirearm", "Browncoats.rte");
		self.deliveryCreationHandler:AddAvailablePreset(self.humanTeam, "IN-02 Backblast", "HDFirearm", "Browncoats.rte");
		
		self.deliveryCreationHandler:RemoveAvailablePreset(self.aiTeam, "IN-02 Backblast");
		
	elseif message == "Captured_RefineryS3GrenadeConsole" then	
		
		self.deliveryCreationHandler:AddAvailablePreset(self.humanTeam, "Fuel Bomb Bandolier", "TDExplosive", "Browncoats.rte");
		self.deliveryCreationHandler:AddAvailablePreset(self.humanTeam, "Fire Bomb Bandolier", "TDExplosive", "Browncoats.rte");
		
		self.deliveryCreationHandler:RemoveAvailablePreset(self.aiTeam, "Fuel Bomb Bandolier");
		
		
	elseif message == "Captured_RefineryS3DrillOverloadConsole" then	
		
		self.HUDHandler:RemoveObjective(self.humanTeam, "S3OverloadDrill");
		self.stage3DrillOverloaded = true;
		
		MovableMan:SendGlobalMessage("RefineryAssault_DrillOverloadBegin");
	
	elseif message == "Captured_RefineryS4FuelPipeConsole" then	
		
		self.deliveryCreationHandler:RemoveAvailablePreset(self.aiTeam, "Fire Bomb Bandolier");
		self.deliveryCreationHandler:RemoveAvailablePreset(self.aiTeam, "FL-200 Heatlance");
		
	elseif message == "Captured_RefineryS4AdvancedGunsConsole" then	
		
		self.deliveryCreationHandler:AddAvailablePreset(self.humanTeam, "BR-76 Stormbringer", "HDFirearm", "Browncoats.rte");
		self.deliveryCreationHandler:AddAvailablePreset(self.humanTeam, "R-500 Extinction", "HDFirearm", "Browncoats.rte");
		
		self.deliveryCreationHandler:RemoveAvailablePreset(self.aiTeam, "BR-76 Stormbringer");
		
	elseif message == "RefineryAssault_S4DoorsBlownUp" then	
		
		if self.Stage ~= 5 then
		
			self.Stage = 5;
			self.HUDHandler:SetCameraMinimumAndMaximumX(self.humanTeam, 0, SceneMan.SceneWidth + 9999);
			self.HUDHandler:RemoveAllObjectives(self.humanTeam);
			
			local pos = SceneMan.Scene:GetOptionalArea("RefineryAssault_S3DoorSequenceArea").Center;
			self.stage4DoorExploSoundContainer = CreateSoundContainer("Yskely Refinery S4 Doors Explo");
			self.stage4DoorExploSoundContainer:Play(pos);
			self.stage4DoorExploDistSoundContainer = CreateSoundContainer("Yskely Refinery S4 Doors Explo Distant");
			self.stage4DoorExploDistSoundContainer:Play(pos);		
			
			CameraMan:AddScreenShake(50, pos);
			
			-- Stage 5 generator stuff
				
			self.saveTable.stage5Generators = {};
			
			local i = 1;
			
			for particle in MovableMan.Particles do
				if particle.PresetName == "Browncoat Refinery Generator Breakable Objective" then
					particle.MissionCritical = false;
					table.insert(self.saveTable.stage5Generators, particle)
					
					self.HUDHandler:AddObjective(self.humanTeam,
					"S5DestroyGenerators" .. i,
					"Destroy",
					"Attack",
					"Destroy backup generators",
					"We need a keycard from one of the sub-commanders. Draw him to you by destroying some generators.",
					particle,
					true,
					true);
					
					i = i + 1;
				end
			end

			self.HUDHandler:AddObjective(self.humanTeam,
			"S5DestroyGenerators",
			"Destroy backup generators",
			"Attack",
			"Destroy backup generators",
			"We need a keycard from one of the sub-commanders. Draw him to you by destroying some generators.",
			nil,
			false,
			true,
			true);
			
			local taskArea = SceneMan.Scene:GetOptionalArea("TacticsPatrolArea_MissionStage4");
			local task = self.tacticsHandler:AddTask("Patrol Stage 4", self.humanTeam, taskArea, "PatrolArea", 10);
			local task = self.tacticsHandler:AddTask("Patrol Stage 4", self.aiTeam, taskArea, "PatrolArea", 10);
			
			self.tacticsHandler:RemoveTask("Patrol Stage 3", self.humanTeam);
			self.tacticsHandler:RemoveTask("Patrol Stage 3", self.aiTeam);
			
			-- Straighten up the buy door situation
			
			self:SendMessage("Captured_RefineryS3BuyDoorConsole1", self.humanTeam);
			self:SendMessage("Captured_RefineryS3BuyDoorConsole2", self.humanTeam);
			self:SendMessage("Captured_RefineryS3BuyDoorConsole3", self.humanTeam);
			
			self.tacticsHandler:RemoveTask("Attack S3 Buy Door Console 1", self.aiTeam);
			self.tacticsHandler:RemoveTask("Attack S3 Buy Door Console 2", self.aiTeam);
			self.tacticsHandler:RemoveTask("Attack S3 Buy Door Console 3", self.aiTeam);
			
			MovableMan:SendGlobalMessage("DeactivateCapturable_RefineryS3BuyDoorConsole1");
			MovableMan:SendGlobalMessage("DeactivateCapturable_RefineryS3BuyDoorConsole2");
			MovableMan:SendGlobalMessage("DeactivateCapturable_RefineryS3BuyDoorConsole3");
			
			-- 1 is already activated when the door first jams
			MovableMan:SendGlobalMessage("ActivateCapturable_RefineryS4BuyDoorConsole2");	
			MovableMan:SendGlobalMessage("ActivateCapturable_RefineryS4BuyDoorConsole3");	
			MovableMan:SendGlobalMessage("ActivateCapturable_RefineryS4BuyDoorConsole4");	
			MovableMan:SendGlobalMessage("ActivateCapturable_RefineryS4BuyDoorConsole5");	
			
			-- Start using buy doors
			
			for k, v in pairs(self.saveTable.buyDoorTables.S4_2) do
				v.Team = self.aiTeam;
			end
			
			for k, v in pairs(self.saveTable.buyDoorTables.S4_3) do
				v.Team = self.aiTeam;
			end

			for k, v in pairs(self.saveTable.buyDoorTables.S4_4) do
				v.Team = self.aiTeam;
			end
			
			for k, v in pairs(self.saveTable.buyDoorTables.S4_5) do
				v.Team = self.aiTeam;
			end
			
			for k, v in pairs(self.saveTable.buyDoorTables.S4_6) do
				v.Team = self.aiTeam;
			end
			
			
			-- Ronin prisoner spawns for the side objective
			
			self.deliveryCreationHandler:AddVirtualTeam(5, "Ronin.rte");
			
			self.saveTable.roninPrisoners = self.deliveryCreationHandler:CreateEliteSquad(5, 5);
			self.saveTable.roninPrisonerLeader = self.saveTable.roninPrisoners[1];
			local area = SceneMan.Scene:GetOptionalArea("RefineryAssault_RoninPrisonersSpawn");
			for k, actor in pairs(self.saveTable.roninPrisoners) do
				actor.HFlipped = true;
				if self.saveTable.roninPrisonerDoorBroken then
					-- failsafe
					actor.Team = self.humanTeam;
					actor.PlayerControllable = true;
				else
					actor.Team = -1;
					actor:DisableScript("Base.rte/AI/HumanAI.lua");
				end
				local pos = area.RandomPoint;
				actor.Pos = pos;
				MovableMan:AddActor(actor);
				actor:UnequipArms();
			end

		end
		
	elseif message == "RefineryAssault_RoninPrisonerDoorBroken" then
	
		self.saveTable.roninPrisonerDoorBroken = true;
		self.roninPrisonerMessageTimer = Timer();
		
		if self.saveTable.roninPrisoners then
			for k, actor in pairs(self.saveTable.roninPrisoners) do
				MovableMan:ChangeActorTeam(actor, self.humanTeam);
				actor:AddScript("Base.rte/AI/HumanAI.lua");
				actor.PlayerControllable = true;
			end
		end
		
	elseif message == "Captured_RefineryS4DockConsole" then
	
		self.saveTable.activeDocks = {5, 6};
		
	elseif message == "Captured_RefineryS4BuyDoorConsole1" then
		
		table.insert(self.saveTable.buyDoorTables.teamAreas[object], "S4_1");
		-- todo make this team selection better somehow... or maybe It Just Works. dunno. it's ugly.
		self:RemoveStringFromTable("S4_1", self.saveTable.buyDoorTables.teamAreas[(object + 1) % 2]);
		
		for k, v in pairs(self.saveTable.buyDoorTables.S4_1) do
			v.Team = object;
		end
		
		-- dupe code woo
		if object == self.humanTeam then		
			local pos;
			for particle in MovableMan.Particles do
				if particle.PresetName == "Refinery S4 Buy Door Console 1" then
					pos = particle.Pos;
					break;
				end
			end		
			self.tacticsHandler:AddTask("Attack S4 Buy Door Console 1", self.aiTeam, pos, "Attack", 20);		
		else
			self.tacticsHandler:RemoveTask("Attack S4 Buy Door Console 1", self.aiTeam);		
		end
		
	elseif message == "Captured_RefineryS4BuyDoorConsole2" then
		
		table.insert(self.saveTable.buyDoorTables.teamAreas[object], "S4_2");
		-- todo make this team selection better somehow... or maybe It Just Works. dunno. it's ugly.
		self:RemoveStringFromTable("S4_2", self.saveTable.buyDoorTables.teamAreas[(object + 1) % 2]);
		
		for k, v in pairs(self.saveTable.buyDoorTables.S4_2) do
			v.Team = object;
		end
		
		-- dupe code woo
		if object == self.humanTeam then		
			local pos;
			for particle in MovableMan.Particles do
				if particle.PresetName == "Refinery S4 Buy Door Console 2" then
					pos = particle.Pos;
					break;
				end
			end		
			self.tacticsHandler:AddTask("Attack S4 Buy Door Console 2", self.aiTeam, pos, "Attack", 20);		
		else
			self.tacticsHandler:RemoveTask("Attack S4 Buy Door Console 2", self.aiTeam);		
		end
		
	elseif message == "Captured_RefineryS4BuyDoorConsole3" then
		
		table.insert(self.saveTable.buyDoorTables.teamAreas[object], "S4_3");
		-- todo make this team selection better somehow... or maybe It Just Works. dunno. it's ugly.
		self:RemoveStringFromTable("S4_3", self.saveTable.buyDoorTables.teamAreas[(object + 1) % 2]);
		
		for k, v in pairs(self.saveTable.buyDoorTables.S4_3) do
			v.Team = object;
		end
		
		-- dupe code woo
		if object == self.humanTeam then		
			local pos;
			for particle in MovableMan.Particles do
				if particle.PresetName == "Refinery S4 Buy Door Console 3" then
					pos = particle.Pos;
					break;
				end
			end		
			self.tacticsHandler:AddTask("Attack S4 Buy Door Console 3", self.aiTeam, pos, "Attack", 20);		
		else
			self.tacticsHandler:RemoveTask("Attack S4 Buy Door Console 3", self.aiTeam);		
		end		
		
	elseif message == "Captured_RefineryS4BuyDoorConsole4" then
		
		table.insert(self.saveTable.buyDoorTables.teamAreas[object], "S4_4");
		-- todo make this team selection better somehow... or maybe It Just Works. dunno. it's ugly.
		self:RemoveStringFromTable("S4_4", self.saveTable.buyDoorTables.teamAreas[(object + 1) % 2]);
		
		for k, v in pairs(self.saveTable.buyDoorTables.S4_4) do
			v.Team = object;
		end
		
		-- dupe code woo
		if object == self.humanTeam then		
			local pos;
			for particle in MovableMan.Particles do
				if particle.PresetName == "Refinery S4 Buy Door Console 4" then
					pos = particle.Pos;
					break;
				end
			end		
			self.tacticsHandler:AddTask("Attack S4 Buy Door Console 4", self.aiTeam, pos, "Attack", 20);		
		else
			self.tacticsHandler:RemoveTask("Attack S4 Buy Door Console 4", self.aiTeam);		
		end
		
	elseif message == "Captured_RefineryS4BuyDoorConsole5" then
		
		table.insert(self.saveTable.buyDoorTables.teamAreas[object], "S4_1");
		-- todo make this team selection better somehow... or maybe It Just Works. dunno. it's ugly.
		self:RemoveStringFromTable("S4_5", self.saveTable.buyDoorTables.teamAreas[(object + 1) % 2]);
		
		for k, v in pairs(self.saveTable.buyDoorTables.S4_5) do
			v.Team = object;
		end
		
		-- dupe code woo
		if object == self.humanTeam then		
			local pos;
			for particle in MovableMan.Particles do
				if particle.PresetName == "Refinery S4 Buy Door Console 5" then
					pos = particle.Pos;
					break;
				end
			end		
			self.tacticsHandler:AddTask("Attack S4 Buy Door Console 5", self.aiTeam, pos, "Attack", 20);		
		else
			self.tacticsHandler:RemoveTask("Attack S4 Buy Door Console 5", self.aiTeam);		
		end		
		
	elseif message == "Captured_RefineryS4BankCapturable" then
	
		self.humanAIGoldIncreaseAmount = self.humanAIGoldIncreaseAmount + 20;		
		self.playerGoldIncreaseAmount = self.playerGoldIncreaseAmount + 20;
		
	elseif message == "Captured_RefineryS4GoldVaultCapturable" then
	
		self.humanAIGoldIncreaseAmount = self.humanAIGoldIncreaseAmount + 4;		
		self.playerGoldIncreaseAmount = self.playerGoldIncreaseAmount + 4;
		
		self:ChangeAIFunds(self.humanTeam, 2000);
		self:ChangeTeamFunds(2000, self.humanTeam); -- player, will also play gold sound
		
	elseif message == "Refinery_S4CameraServerBroken" then
	
		self.saveTable.cameraServerBroken = self.saveTable.cameraServerBroken == nil and 1 or self.saveTable.cameraServerBroken + 1;
		
	elseif message == "RefineryAssault_S7BrainAuthorized" then		
	
		self.HUDHandler:RemoveObjective(self.humanTeam, "S7AuthorizeBrain");
		self.saveTable.stage7BrainAuthorized = true;
		
	elseif message == "Captured_RefineryS7AuxAuthConsole" then	
	
		self.HUDHandler:RemoveObjective(self.humanTeam, "S7AuxAuth");
		self.saveTable.stage7AuxAuthConsoleCaptured = true;
		
	elseif message == "RefineryAssault_S8BossDoorOpened" then	
	
		self.HUDHandler:RemoveObjective(self.humanTeam, "S8OpenBossDoor");
		self.Stage = 9;
		
		MovableMan:SendGlobalMessage("ActivateCapturable_RefineryS10FinalConsole");

		for particle in MovableMan.Particles do
			if particle.PresetName == "Refinery S9 Final Console" then

				self.HUDHandler:AddObjective(self.humanTeam,
				"S9FinalConsole",
				"Capture",
				"Attack",
				"Capture the CNC-center console!",
				"This is it! Get into that console and every Browncoat left on this base is at our mercy!",
				particle.Pos,
				false,
				true,
				true);		
				
				break;
			end
		end

	elseif message == "Captured_RefineryS9BossBankCapturable" then
	
		self.humanAIGoldIncreaseAmount = self.humanAIGoldIncreaseAmount + 20;		
		self.playerGoldIncreaseAmount = self.playerGoldIncreaseAmount + 20;		
		
	elseif message == "Captured_RefineryS9FinalConsole" then	
	
		self.HUDHandler:RemoveObjective(self.humanTeam, "S9FinalConsole");
		self.saveTable.stage9FinalConsoleCaptured = true;

	elseif message == "Refinery_RefineryS10FinalBossDead" then	
		
	end
	
	-- DEBUG STAGE SKIPS

	-- ActivityMan:GetActivity():SetTeamFunds(5000, 0)

	-- ActivityMan:GetActivity():SendMessage("SkipCurrentStage");
	if message == "SkipCurrentStage" then
		message = "SkipStage" .. self.Stage;
		self.HUDHandler:SetCameraMinimumAndMaximumX(self.humanTeam, 0, SceneMan.SceneWidth + 9999);
	end

	-- ActivityMan:GetActivity():SendMessage("SkipStage1");
	-- ActivityMan:GetActivity():SendMessage("SkipStage2");
	-- ActivityMan:GetActivity():SendMessage("SkipStage3");
	if message == "SkipStage1" then
		for a in MovableMan.Actors do if a.Team == 1 and a.ClassName == "AHuman" and SceneMan.Scene:WithinArea("Mission Stage Area 1", a.Pos) then a.Health = 0 end end
	elseif message == "SkipStage2" then
		self.stage2HoldingBothConsoles = true;
		self.stage2TimeToHoldConsoles = 0;
		
		table.insert(self.saveTable.buyDoorTables.teamAreas[self.humanTeam], "LC2");
		self:RemoveStringFromTable("LC2", self.saveTable.buyDoorTables.teamAreas[self.aiTeam]);
		
		for k, v in pairs(self.saveTable.buyDoorTables.LC2) do
			v.Team = self.humanTeam;
		end
		
		table.insert(self.saveTable.buyDoorTables.teamAreas[self.humanTeam], "LC1");
		self:RemoveStringFromTable("LC1", self.saveTable.buyDoorTables.teamAreas[self.aiTeam]);
		
		for k, v in pairs(self.saveTable.buyDoorTables.LC1) do
			v.Team = self.humanTeam;
		end
		
		self.tacticsHandler:RemoveTask("Counterattack", self.aiTeam);

	elseif message == "SkipStage3" then
	
		for k, v in pairs(self.saveTable.buyDoorTables.S3_1) do
			if v.Team == 1 then
				self:SendMessage("Captured_RefineryS3BuyDoorConsole1", self.humanTeam);
			end
			break;
		end

		self.stage3AllConsolesBroken = true;
		self.HUDHandler:RemoveObjective(self.humanTeam, "S3DestroyConsoles");
		self.saveTable.enemyActorTables.stage3FacilityOperator = {};
		self.stage3DrillOverloaded = true;
		self.HUDHandler:RemoveObjective(self.humanTeam, "S3OverloadDrill");
	elseif message == "SkipStage4" then
		for k, door in pairs(self.saveTable.stage4Door) do
			door:GibThis();
		end
		for k, door in pairs(self.saveTable.stage3Doors) do
			door:GibThis();
		end
		self:SendMessage("RefineryAssault_S4DoorsBlownUp");
	elseif message == "SkipStage5" then
		for i, generator in ipairs(self.saveTable.stage5Generators) do
			if not generator or not MovableMan:ValidMO(generator) then
			else
				ToMOSRotating(generator):GibThis();
			end
		end		
	elseif message == "SkipStage6" then
	
		self.HUDHandler:RemoveAllObjectives(self.humanTeam);
		MovableMan:SendGlobalMessage("ActivateCapturable_RefineryS7AuxAuthConsole");
		MovableMan:SendGlobalMessage("ActivateRefineryAuthorizationConsole");
		self.Stage = 7;
		
		for particle in MovableMan.Particles do
			if particle.PresetName == "Refinery Authorization Console" then
	
				self.HUDHandler:AddObjective(self.humanTeam,
				"S7AuthorizeBrain",
				"Authorize yourself",
				"Attack",
				"Authorize your commander using the keycard",
				"With the keycard, you can authorize your commander's physical signature to open the CNC-center blast door at this console.",
				particle.Pos,
				false,
				true,
				true);					
		
			elseif particle.PresetName == "Refinery S7 Auxiliary Authorization Console" then
			
				self.HUDHandler:AddObjective(self.humanTeam,
				"S7AuxAuth",
				"Hack",
				"Attack",
				"Hack the auxiliary authorization console",
				"This console is also responsible for the CNC-center's door authorization list. Hack it.",
				particle.Pos,
				false,
				true,
				true);
				
			end
		end
		
	elseif message == "SkipStage7" then
	
		self:SendMessage("Captured_RefineryS7AuxAuthConsole");
		self:SendMessage("RefineryAssault_S7BrainAuthorized");
		
	elseif message == "SkipStage8" then

		for actor in MovableMan.Actors do
			if actor:NumberValueExists("BossVaultDoor") then
				ToADoor(actor):OpenDoor();
			end
		end
		
		self:SendMessage("RefineryAssault_S8BossDoorOpened");
		
	elseif message == "SkipStage9" then
	
		self:SendMessage("Captured_RefineryS10FinalConsole");
		
	end
	
	

end

function RefineryAssault:RemoveStringFromTable(str, tab)

	for k, v in pairs(tab) do
		if v == str then
			table.remove(tab, k);
		end
	end
	
end

function RefineryAssault:SendDockDelivery(team, task, forceRocketUsage, squadType)

	local squadCount = math.random(3, 4);

	local craft;
	local squad;
	local goldCost;
	
	if not forceRocketUsage then
		forceRocketUsage = math.random() < 0.3 and true or false;
	end
	
	if squadType == "Elite" then
		-- we already always go elite, avoid giving DeliveryCreationHandler an invalid infantry type
		squadType = nil;
	end

	-- Dock deliveries are always elite
	craft, squad, goldCost = self.deliveryCreationHandler:CreateEliteSquadWithCraft(team, forceRocketUsage, squadCount, squadType);
	
	for k, actor in pairs(squad) do
		actor.PlayerControllable = self.humansAreControllingAlliedActors;
		if team == self.humanTeam then
			actor.HUDVisible = self.humansAreControllingAlliedActors;
		end
	end
	
	craft.PlayerControllable = self.humansAreControllingAlliedActors;
	craft.HUDVisible = self.humansAreControllingAlliedActors;
	craft:SetGoldValue(0);
	
	-- reminder that only player team sends to dock in this activity.
	
	if self.verboseLogging then
		print("INFO: Refinery Assault is sending a dock delivery. Active docks:")
		for k, v in ipairs(self.saveTable.activeDocks) do
			print("INFO: ACTIVE DOCK: " .. v);
		end
	end
	
	local tempActiveDocks = {};
	
	for k, v in ipairs(self.saveTable.activeDocks) do
		tempActiveDocks[k] = v;
	end
	
	local specificDock;
	local success;
	
	while #tempActiveDocks > 0 and not success do
		local rand = math.random(1, #tempActiveDocks);
		specificDock = tempActiveDocks[rand];
		success = self.dockingHandler:SpawnDockingCraft(craft, specificDock);
		table.remove(tempActiveDocks, rand);
	end
	
	if success then
		self:ChangeAIFunds(team, -goldCost);
		if self.verboseLogging then
			print("INFO: RefineryAssault sent a dock delivery for team " .. team);
		end
		return squad;
	end
	
	return false;
	
end

function RefineryAssault:SendBuyDoorDelivery(team, task, squadType, specificIndex)

	local squadCount = math.random(2, 3);

	local order;
	local goldCost;

	if squadType == "Elite" then
		order, goldCost = self.deliveryCreationHandler:CreateEliteSquad(team, squadCount);
	else
		order, goldCost = self.deliveryCreationHandler:CreateSquad(team, squadCount, squadType);
	end
	
	for k, actor in pairs(order) do
		actor.PlayerControllable = self.humansAreControllingAlliedActors;
		if actor.Team == self.humanTeam then
			actor.HUDVisible = self.humansAreControllingAlliedActors;
		end
	end
	
	--print("tried order for team: " .. team);
	
	if order then
		local taskPos;
		if task then
			
			taskPos = task.Position.PresetName and task.Position.Pos or task.Position; -- ghetto MO check
			if taskPos.Name then -- ghetto-er Area check
				taskPos = taskPos.RandomPoint;
			end
			-- check if it's in an area this team owns
			local areaThisIsIn
			for i = 1, #self.saveTable.buyDoorTables.teamAreas[team] do
				local area = SceneMan.Scene:GetOptionalArea("BuyDoorArea_" .. self.saveTable.buyDoorTables.teamAreas[team][i]);
				if area:IsInside(taskPos) then
					areaThisIsIn = area;
					--print("is inside teamowned area: " .. area.Name);
					break;
				end
			end
		else
			return false;
		end
		
		if not areaThisIsIn or not self.buyDoorHandler:GetAvailableBuyDoorsInArea(areaThisIsIn, team) then

			-- loop through all owned buy doors, continually selecting the area with the closest buy door.
			-- the one we're left with is in the closest area.
			-- might be ineffective, but who cares.
			
			local closestDist = false;
			if #self.saveTable.buyDoorTables.teamAreas[team] > 0 then
				for k, area in pairs(self.saveTable.buyDoorTables.teamAreas[team]) do
					--print(area)
					for k, buyDoor in pairs(self.saveTable.buyDoorTables[area]) do
						local dist = SceneMan:ShortestDistance(taskPos, buyDoor.Pos, SceneMan.SceneWrapsX).Magnitude;
						if not closestDist then
							closestDist = dist;
							areaThisIsIn = area;
						elseif dist < closestDist then
							closestDist = dist;
							areaThisIsIn = area;
						end
					end
				end
				--print("found closest area to task:");
				--print(area);
				-- actually get the Area
				areaThisIsIn = SceneMan.Scene:GetOptionalArea("BuyDoorArea_" .. areaThisIsIn);
			else
				--print("team " .. team .. " doesn't have a backup area");
			end
		end
		
		--print(team)
		--print(areaThisIsIn.Name)
		
		if areaThisIsIn then
			--print(areaThisIsIn.Name)
			
			local randomSelection;
			local usableBuyDoorTable = self.buyDoorHandler:GetAvailableBuyDoorsInArea(areaThisIsIn, team)
			
			if usableBuyDoorTable then
				randomSelection = usableBuyDoorTable[math.random(1, #usableBuyDoorTable)]
			end
			
			if randomSelection then
				if team == self.aiTeam and self.saveTable.cameraServersBroken and self.saveTable.cameraServersBroken >= 2 then
					self.buyDoorHandler:ChangeCooldownTime(randomSelection, 15000);
				else
					self.buyDoorHandler:ChangeCooldownTime(randomSelection, 5000);
				end
				local success = self.buyDoorHandler:SendCustomOrder(order, team, randomSelection);
				if success then
					self:ChangeAIFunds(team, -goldCost);
					if self.verboseLogging then
						print("INFO: RefineryAssault sent a buydoor delivery for team " .. team);
					end
					return order;
				end
			end
		end
	end
	
	return false;
	
end

function RefineryAssault:SetupStartingActors()

	self.saveTable.enemyActorTables = {};
	
	local AHumanTable = {};
	-- brownies don't really have acrabs, do they?
	local ACrabTable = {};

	for actor in MovableMan.AddedActors do
		-- any actors that are just Actor are likely buy doors or
		-- other misc objects. however, ahumans and acrabs are only actual units
		-- we care about.
		if IsAHuman(actor) then
			table.insert(AHumanTable, actor);
		elseif IsACrab(actor) then
			table.insert(ACrabTable, actor);
		end
	end
	
	self.saveTable.enemyActorTables.stage1 = {};
	self.saveTable.enemyActorTables.stage1CounterAttActors = {};
	
	-- locals used just to set up tasks
	local stage1SquadsTable = {};
	stage1SquadsTable[0] = {};
	stage1SquadsTable[1] = {};
	stage1SquadsTable[2] = {};
	
	for i, actor in ipairs(AHumanTable) do
	
		
		if SceneMan.Scene:WithinArea("Mission Stage Area 1", actor.Pos) and actor.Team == self.aiTeam then
			table.insert(self.saveTable.enemyActorTables.stage1, actor);
			-- divvy up into squads of 3
			-- bonus: it ends up 0-indexed!!!
			table.insert(stage1SquadsTable[i % 3], actor);
			
			-- Set up HUD handler objectives
			
			self.HUDHandler:AddObjective(self.humanTeam,
			"S1KillEnemies" .. i,
			"Kill",
			"Attack",
			"Clear the first hanging building of enemies",
			"Secure an FOB for us to stage further attacks from.",
			actor,
			true,
			true);
			
			-- Test
			
			--self.HUDHandler:QueueCameraPanEvent(self.humanTeam, "S1KillEnemies" .. i, actor, 0.1, 2500, false);
			
		end
		
		if SceneMan.Scene:WithinArea("RefineryAssault_S1CounterAttActors", actor.Pos) and actor.Team == self.aiTeam then
			table.insert(self.saveTable.enemyActorTables.stage1CounterAttActors, actor);
			actor.HFlipped = true; -- look the right way numbnuts
		end		
		
	end

	for k, v in pairs(stage1SquadsTable) do
		local task = self.tacticsHandler:PickTask(self.aiTeam);
		self.tacticsHandler:AddSquad(self.aiTeam, stage1SquadsTable[k], task.Name, true);
	end
	
	self.saveTable.enemyActorTables.stage3FacilityOperator = {};
	
	-- note index access, we get a table back
	local facilityOperator = self.deliveryCreationHandler:CreateEliteSquad(self.aiTeam, 1, "Heavy")[1];
	local area = SceneMan.Scene:GetOptionalArea("RefineryAssault_S3FacilityOperator");
	local pos = SceneMan:MovePointToGround(area.Center, 50, 3);
	
	facilityOperator.Pos = pos;
	MovableMan:AddActor(facilityOperator);
	facilityOperator.AIMode = Actor.AIMODE_SENTRY;
	
	table.insert(self.saveTable.enemyActorTables.stage3FacilityOperator, facilityOperator);
	
	-- Stage 3 and 4 door stuff, might as well save it
	
	self.saveTable.stage3Doors = {};
	self.saveTable.stage4Door = {};
	
	for actor in MovableMan.AddedActors do
		if actor:NumberValueExists("BlastDoorOpening") then
			table.insert(self.saveTable.stage3Doors, actor);
		elseif actor:NumberValueExists("BlastDoorStuck") then
			table.insert(self.saveTable.stage4Door, actor);
		end
	end
	
end

function RefineryAssault:SetupFirstStage()

	-- Unique function just to hide away init stuff - every other stage setup is immediately done upon completion of its
	-- Monitor function
	
	-- Intro cinematics
	
	self.saveTable.introTimer = Timer();
	self.saveTable.introLastRocketSpawnTime = 0;
	
	local cameraPos = SceneMan.Scene:GetOptionalArea("RefineryAssault_IntroCameraPan1").Center;
	self.HUDHandler:QueueCameraPanEvent(self.humanTeam, "S1IntroPan1", cameraPos, 1, 500, true, true, true);
	
	local cameraPos = SceneMan.Scene:GetOptionalArea("RefineryAssault_IntroCameraPan2").Center;
	self.HUDHandler:QueueCameraPanEvent(self.humanTeam, "S1IntroPan2", cameraPos, 0.01, 6000, true, true, true);
	
	local cameraPos = SceneMan.Scene:GetOptionalArea("RefineryAssault_IntroCameraPan3").Center;
	self.HUDHandler:QueueCameraPanEvent(self.humanTeam, "S1IntroPan3", cameraPos, 0.01, 3500, true, true, true);
	
	local cameraPos = SceneMan.Scene:GetOptionalArea("RefineryAssault_IntroCameraPan4").Center;

	-- Fog of war-causing callback if it's enabled, but none if it's not
	if self:GetFogOfWarEnabled() then
		self.HUDHandler:QueueCameraPanEvent(self.humanTeam, "S1IntroPan4", cameraPos, 0.01, 5000, true, true, true, function() SceneMan:MakeAllUnseen(Vector(20, 20), self.humanTeam); end);
	else
		self.HUDHandler:QueueCameraPanEvent(self.humanTeam, "S1IntroPan4", cameraPos, 0.01, 5000, true, true, true);
	end
	
	-- Disable all buy doors, not using them quite yet
	
	for k, v in pairs(self.saveTable.buyDoorTables.All) do
		v.Team = -1
	end
	
	-- Set up stage 1 enemy actors
	
	self.tacticsHandler:AddTask("Sentry", self.aiTeam, Vector(0, 0), "Sentry", 10);
	local taskArea = SceneMan.Scene:GetOptionalArea("TacticsPatrolArea_MissionStage1");
	self.tacticsHandler:AddTask("Patrol Stage 1", self.aiTeam, taskArea, "PatrolArea", 5);
	
	self:SetupStartingActors();
	
	-- Set up the 2 dock squads
	
	taskArea = SceneMan.Scene:GetOptionalArea("TacticsPatrolArea_MissionStage1");
	local task = self.tacticsHandler:AddTask("Search And Destroy", self.humanTeam, taskArea, "PatrolArea", 10);
	
	local squad = self:SendDockDelivery(self.humanTeam, task, true, "Elite");
	
	self.tacticsHandler:AddSquad(self.humanTeam, squad, task.Name, true);
	
	squad = self:SendDockDelivery(self.humanTeam, task, true, "Elite");
	
	self.tacticsHandler:AddSquad(self.humanTeam, squad, task.Name, true);
	
	-- Set up player squad and dropship
	
	local dropShip = self.deliveryCreationHandler:CreateEliteSquadWithCraft(self.humanTeam, false, 5);
	local dropShipPos = SceneMan.Scene:GetOptionalArea("RefineryAssault_HumanBrainSpawn").Center;
	dropShip.Team = self.humanTeam;
	dropShip.Pos = dropShipPos;
	dropShip.AIMode = Actor.AIMODE_SENTRY;
	dropShip.PlayerControllable = self.humansAreControllingAlliedActors;
	dropShip:SetGoldValue(0);
	
	self.saveTable.playerBrains = {};
	
	for i, player in pairs(self.humanPlayers) do
		local brain = PresetMan:GetLoadout("Infantry Brain", self.humanTeamTech, false);
		if brain then
			brain:RemoveInventoryItem("Constructor");
		else
			brain = RandomAHuman("Brains", self.humanTeamTech);
			brain:AddToGroup("Brain " .. tostring(player));
			brain:AddInventoryItem(RandomHDFirearm("Weapons - Light", self.humanTeamTech));
			brain:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.humanTeamTech));
		end
		brain.Team = self.humanTeam;
		brain.AIMode = Actor.AIMODE_SENTRY;
		self:SetPlayerBrain(brain, player);
		self:SetObservationTarget(brain.Pos, player);
		self:SwitchToActor(brain, player, self.humanTeam);
		brain.Pos = dropShip.Pos + Vector(0 + (10 * i), 180);
		table.insert(self.saveTable.playerBrains, brain);
		MovableMan:AddActor(brain);
		--dropShip:AddInventoryItem(brain);
	end
	
	self.saveTable.stage1InitialDropship = dropShip;
	
	MovableMan:AddActor(dropShip)
	--dropShip:OpenHatch();
	
	-- HUD handler listed objective
	
	self.HUDHandler:AddObjective(self.humanTeam,
	"S1KillEnemies",
	"Kill",
	"Attack",
	"Clear the first hanging building of enemies",
	"Secure an FOB for us to stage further attacks from.",
	nil,
	false,
	true);
	
	-- HUD handler camera limits
	
	self.HUDHandler:SetCameraMinimumAndMaximumX(self.humanTeam, 0, 2500);
	

end

function RefineryAssault:MonitorStage1()

	if not self.saveTable.introTimer:IsPastSimMS(2000) then
		if self.saveTable.introTimer:IsPastSimMS(self.saveTable.introLastRocketSpawnTime + 100) then
			self.saveTable.introLastRocketSpawnTime = self.saveTable.introTimer.ElapsedSimTimeMS;
			local particle = CreateAEmitter("Particle Rocket Launcher", "Base.rte");
			particle.Pos = SceneMan.Scene:GetOptionalArea("RefineryAssault_IntroRocketSpawns").RandomPoint;
			particle.Vel = Vector(math.random(-5, 5), -70);
			particle.RotAngle = math.pi/2;
			particle.Team = self.humanTeam;
			MovableMan:AddParticle(particle);
			particle:EnableEmission(true);
		end
	end
		

	-- Send away the initial dropship once it's empty
	if self.saveTable.stage1InitialDropship then
		local craft = ToACDropShip(self.saveTable.stage1InitialDropship);
		if not craft or not MovableMan:ValidMO(craft) or craft:IsDead() then
			self.saveTable.stage1InitialDropship = nil;
		else
			craft = ToACDropShip(self.saveTable.stage1InitialDropship);
			if self.saveTable.stage1InitialDropshipToReturn then
				if craft.AIMode == Actor.AIMODE_SENTRY then
					craft.AIMode = Actor.AIMODE_RETURN;
					self.saveTable.stage1InitialDropship = nil;
					self.saveTable.stage1InitialDropshipToReturn = nil;
				end
			elseif craft:IsInventoryEmpty() then
				local pos = SceneMan.Scene:GetOptionalArea("RefineryAssault_HumanBrainSpawn").Center;
				craft:ClearAIWaypoints();
				craft:AddAISceneWaypoint(Vector(pos.X - 300, pos.Y));
				craft.DeliveryState = ACraft.LAUNCH;
				craft.AIMode = Actor.AIMODE_GOTO;
				craft:CloseHatch();
				self.saveTable.stage1InitialDropshipToReturn = true;
			elseif self.saveTable.introTimer:IsPastSimMS(8000) then
				craft:OpenHatch();
			end
		end
	end

	local noActors = true;

	for i, actor in ipairs(self.saveTable.enemyActorTables.stage1) do
		if not actor or not MovableMan:ValidMO(actor) or actor:IsDead() then
			self.saveTable.enemyActorTables.stage1[i] = false;
			self.HUDHandler:RemoveObjective(self.humanTeam, "S1KillEnemies" .. i);
		else
			noActors = false;
		end
	end
	
	if noActors then
		-- stage completion!
		self.Stage = 2;
		
		self:GetBanner(GUIBanner.YELLOW, 0):ShowText("STAGE 1 DONE!", GUIBanner.FLYBYLEFTWARD, 1500, Vector(FrameMan.PlayerScreenWidth, FrameMan.PlayerScreenHeight), 0.4, 4000, 0)
		
		-- Start using buydoors
		
		for k, v in pairs(self.saveTable.buyDoorTables.LC1) do
			v.Team = self.aiTeam;
		end
		
		for k, v in pairs(self.saveTable.buyDoorTables.LC2) do
			v.Team = self.aiTeam;
		end
		
		-- Capturable setup
		
		MovableMan:SendGlobalMessage("ActivateCapturable_RefineryLCHackConsole1");
		MovableMan:SendGlobalMessage("ActivateCapturable_RefineryLCHackConsole2");
		
		-- Task setup
		
		local taskPos = SceneMan.Scene:GetOptionalArea("CaptureArea_RefineryLCHackConsole1").Center;
		
		self.tacticsHandler:AddTask("Attack Hack Console 1", self.humanTeam, taskPos, "Attack", 10);
		self.tacticsHandler:AddTask("Defend Hack Console 1", self.aiTeam, taskPos, "Defend", 10);
		
		taskPos = SceneMan.Scene:GetOptionalArea("CaptureArea_RefineryLCHackConsole2").Center;
		
		self.tacticsHandler:AddTask("Attack Hack Console 2", self.humanTeam, taskPos, "Attack", 10);
		self.tacticsHandler:AddTask("Defend Hack Console 2", self.aiTeam, taskPos, "Defend", 10);
		
		local taskArea = SceneMan.Scene:GetOptionalArea("TacticsPatrolArea_MissionStage2");
		local task = self.tacticsHandler:AddTask("Patrol Stage 2", self.humanTeam, taskArea, "PatrolArea", 2);
		local task = self.tacticsHandler:AddTask("Patrol Stage 2", self.aiTeam, taskArea, "PatrolArea", 4);
		
		self.tacticsHandler:RemoveTask("Sentry", self.aiTeam);
		self.tacticsHandler:RemoveTask("Patrol Stage 1", self.aiTeam);
		self.tacticsHandler:RemoveTask("Search And Destroy", self.humanTeam);
		
		-- Send the counterattack by setting up squad
		
		-- First check they still exist, could be dealing with a wise guy
		
		for k, actor in pairs(self.saveTable.enemyActorTables.stage1CounterAttActors) do
			if not actor or not MovableMan:ValidMO(actor) or actor:IsDead() then
				table.remove(self.saveTable.enemyActorTables.stage1CounterAttActors, k);
			end
		end
		
		if #self.saveTable.enemyActorTables.stage1CounterAttActors > 0 then
		
			local taskArea = SceneMan.Scene:GetOptionalArea("TacticsPatrolArea_MissionStage1");
			local task = self.tacticsHandler:AddTask("Counterattack", self.aiTeam, taskArea, "PatrolArea", 10);
			
			self.tacticsHandler:AddSquad(self.aiTeam, self.saveTable.enemyActorTables.stage1CounterAttActors, task.Name, true);
			
		end
		
		--Monies
		
		self.humanAIFunds = math.max(self.humanAIFunds, 0);
		
		self.aiTeamGoldIncreaseAmount = self.aiTeamGoldIncreaseAmount + 100;
		self.humanAIGoldIncreaseAmount = self.humanAIGoldIncreaseAmount + 30;
		
		-- HUD handler
		
		self.HUDHandler:RemoveAllObjectives(self.humanTeam);
		
		self.HUDHandler:AddObjective(self.humanTeam,
		"S2HackConsoles",
		"Hack and hold",
		"Attack",
		"Hack and hold the logistics center control consoles",
		"Gain control of the facility's logistics computers and hold them until we finish downloading crucial intelligence.",
		nil,
		false,
		true);
		
		local objPos = SceneMan.Scene:GetOptionalArea("CaptureArea_RefineryLCHackConsole1").Center;
		
		self.HUDHandler:AddObjective(self.humanTeam,
		"S2HackConsole1",
		"Hack and hold",
		"Attack",
		"",
		"",
		objPos,
		true,
		true);
		
		local objPos = SceneMan.Scene:GetOptionalArea("CaptureArea_RefineryLCHackConsole2").Center;
		
		self.HUDHandler:AddObjective(self.humanTeam,
		"S2HackConsole2",
		"Hack and hold",
		"Attack",
		"",
		"",
		objPos,
		true,
		true);
		
		self.HUDHandler:SetCameraMinimumAndMaximumX(self.humanTeam, 0, 6300);
	
	end	
	
end

function RefineryAssault:MonitorStage2()

	--print("stage 2 timer: " .. self.stage2HoldTimer.ElapsedSimTimeMS);
	--print(self.stage2HoldingBothConsoles)

	if self.stage2HoldingBothConsoles == true and self.stage2HoldTimer:IsPastSimMS(self.stage2TimeToHoldConsoles) then
		self:GetBanner(GUIBanner.YELLOW, 0):ShowText("YOU'RE S2 WINNER!", GUIBanner.FLYBYLEFTWARD, 1500, Vector(FrameMan.PlayerScreenWidth, FrameMan.PlayerScreenHeight), 0.4, 4000, 0)
		self.Stage = 3;
		
		-- Capturable setup
		
		MovableMan:SendGlobalMessage("DeactivateCapturable_RefineryLCHackConsole1");
		MovableMan:SendGlobalMessage("DeactivateCapturable_RefineryLCHackConsole2");
		
		MovableMan:SendGlobalMessage("ActivateCapturable_RefineryS3BuyDoorConsole1");
		MovableMan:SendGlobalMessage("ActivateCapturable_RefineryS3BuyDoorConsole2");
		MovableMan:SendGlobalMessage("ActivateCapturable_RefineryS3BuyDoorConsole3");
		
		MovableMan:SendGlobalMessage("ActivateCapturable_RefineryS3DrillOverloadConsole");
		
		MovableMan:SendGlobalMessage("ActivateCapturable_RefineryS3OilCapturable");
		
		-- Setup stage 3 consoles
		
		self.saveTable.stage3Consoles = {};
		
		local i = 1;
		
		for particle in MovableMan.Particles do
			if particle.PresetName == "Browncoat Refinery Console Breakable Objective" then
				particle.MissionCritical = false;
				table.insert(self.saveTable.stage3Consoles, particle)
				self.tacticsHandler:AddTask("Defend Refinery Console " .. i, self.aiTeam, particle, "Defend", 10);
				self.tacticsHandler:AddTask("Attack Refinery Console " .. i, self.humanTeam, particle, "Attack", 10);
				i = i + 1;
				print("found refinery breakable console and added task")
			end
		end
		
		--Monies
		
		self.humanAIFunds = math.max(self.humanAIFunds, 0);
		
		self.aiTeamGoldIncreaseAmount = self.aiTeamGoldIncreaseAmount + 200;
		self.humanAIGoldIncreaseAmount = self.humanAIGoldIncreaseAmount + 100;
		
		-- Task stuff
		
		local taskArea = SceneMan.Scene:GetOptionalArea("TacticsPatrolArea_MissionStage3");
		local task = self.tacticsHandler:AddTask("Patrol Stage 3", self.humanTeam, taskArea, "PatrolArea", 10);
		local task = self.tacticsHandler:AddTask("Patrol Stage 3", self.aiTeam, taskArea, "PatrolArea", 10);
		
		self.tacticsHandler:RemoveTask("Patrol Stage 2", self.humanTeam);
		self.tacticsHandler:RemoveTask("Patrol Stage 2", self.aiTeam);
		
		self.tacticsHandler:RemoveTask("Attack Hack Console 1", self.humanTeam);
		self.tacticsHandler:RemoveTask("Defend Hack Console 1", self.aiTeam);
		
		self.tacticsHandler:RemoveTask("Attack Hack Console 2", self.humanTeam);
		self.tacticsHandler:RemoveTask("Defend Hack Console 2", self.aiTeam);
		
		-- Start using buy doors
		
		for k, v in pairs(self.saveTable.buyDoorTables.S3_1) do
			v.Team = self.aiTeam;
		end
		
		for k, v in pairs(self.saveTable.buyDoorTables.S3_2) do
			v.Team = self.aiTeam;
		end

		for k, v in pairs(self.saveTable.buyDoorTables.S3_3) do
			v.Team = self.aiTeam;
		end
		
		for k, v in pairs(self.saveTable.buyDoorTables.S4_1) do
			v.Team = self.aiTeam;
		end
		
		-- HUD handler
		
		self.HUDHandler:RemoveAllObjectives(self.humanTeam);
		
		self.HUDHandler:AddObjective(self.humanTeam,
		"S3OpenDoors",
		"Open blast doors",
		"Attack",
		"Open the blast doors blocking our path",
		"The path forwards is blocked by three blast doors. Open them by triggering the facility failsafes.",
		nil,
		false,
		true,
		true);
		
		local objPos = SceneMan.Scene:GetOptionalArea("CaptureArea_RefineryS3DrillOverloadConsole").Center;
		
		self.HUDHandler:AddObjective(self.humanTeam,
		"S3OverloadDrill",
		"Sabotage the main drill",
		"Attack",
		"Sabotage the main drill",
		"Overload the drill and destroy it. You'll need to get your commander to do it.",
		objPos,
		false,
		true,
		true);
		
		self.HUDHandler:AddObjective(self.humanTeam,
		"S3DestroyConsoles",
		"Find and destroy control centers",
		"Attack",
		"Find and destroy the refinery control centers",
		"Destroying the facility's control centers should contribute to triggering its failsafes. Find them.",
		nil,
		false,
		true,
		true);
		
		self.HUDHandler:AddObjective(self.humanTeam,
		"S3DefeatOperator",
		"Defeat facility operator",
		"Attack",
		"Defeat the refinery operator",
		"Defeat the operator monitoring the refinery. It can't hurt.",
		nil,
		false,
		true,
		true);
		
		self.HUDHandler:SetCameraMinimumAndMaximumX(self.humanTeam, 0, 12000);
		
	end
	
end

function RefineryAssault:MonitorStage3()

	if not self.stage3FacilityOperatorKilled then

		for k, actor in pairs(self.saveTable.enemyActorTables.stage3FacilityOperator) do
			if not actor or not MovableMan:ValidMO(actor) or actor:IsDead() then
				table.remove(self.saveTable.enemyActorTables.stage3FacilityOperator, k);
			end
		end
		
		if #self.saveTable.enemyActorTables.stage3FacilityOperator == 0 then
			self.HUDHandler:RemoveObjective(self.humanTeam, "S3DefeatOperator");
			self.stage3FacilityOperatorKilled = true;
		end
		
	end
	
	if not self.stage3AllConsolesBroken then

		for k, console in pairs(self.saveTable.stage3Consoles) do
			if not console or not MovableMan:ValidMO(console) then
				self.saveTable.stage3Consoles[k] = nil;
				
				self.tacticsHandler:RemoveTask("Defend Refinery Console " .. k, self.aiTeam);
				self.tacticsHandler:RemoveTask("Attack Refinery Console " .. k, self.humanTeam);

			else
				--print(console)
				--print(k)
			end
		end
		
		if #self.saveTable.stage3Consoles == 0 then
			self.stage3AllConsolesBroken = true;
			self.HUDHandler:RemoveObjective(self.humanTeam, "S3DestroyConsoles");
		end
		
	end

	if self.stage3AllConsolesBroken and self.stage3FacilityOperatorKilled and self.stage3DrillOverloaded and not self.stage3DoorSequenceTimer then
	
		-- initiate scripted sequence
		self.stage3DoorSequenceTimer = Timer();
	
		self.HUDHandler:RemoveObjective(self.humanTeam, "S3OpenDoors");
		
		local pos = SceneMan.Scene:GetOptionalArea("RefineryAssault_S3DoorSequenceArea").Center;
		
		local soundContainer = CreateSoundContainer("Yskely Refinery Blast Door Alarm", "Browncoats.rte");
		soundContainer:Play(pos);
		
		self.HUDHandler:QueueCameraPanEvent(self.humanTeam, "S3DoorSequence", pos, 0.08, 15000, true, true, true);

	elseif self.stage3DoorSequenceTimer and self.stage3DoorSequenceTimer:IsPastSimMS(6250) then
	
		if not self.stage3ScreenShake then
			self.stage3ScreenShake = true;
			local pos = SceneMan.Scene:GetOptionalArea("RefineryAssault_S3DoorSequenceArea").Center;
			CameraMan:AddScreenShake(10, pos);
		end
		
		-- thou shalt deal with this bad code
		if MovableMan:ValidMO(self.saveTable.stage3Doors[1]) then
			ToADoor(self.saveTable.stage3Doors[1]):OpenDoor();
		end
		
		if self.stage3DoorSequenceTimer:IsPastSimMS(7500) then
			if MovableMan:ValidMO(self.saveTable.stage3Doors[2]) then
				ToADoor(self.saveTable.stage3Doors[2]):OpenDoor();
				if not self.stage3ScreenShake2 then
					self.stage3ScreenShake2 = true;
					local pos = SceneMan.Scene:GetOptionalArea("RefineryAssault_S3DoorSequenceArea").Center;
					CameraMan:AddScreenShake(10, pos);
				end
			end
		end
		
		if not self.stage3PlayedDoorStopSound == true and self.stage3DoorSequenceTimer:IsPastSimMS(8750) then
			if MovableMan:ValidMO(self.saveTable.stage4Door[1]) then
				ToADoor(self.saveTable.stage4Door[1]):OpenDoor();
				if not self.stage3ScreenShake3 then
					self.stage3ScreenShake3 = true;
					local pos = SceneMan.Scene:GetOptionalArea("RefineryAssault_S3DoorSequenceArea").Center;
					CameraMan:AddScreenShake(10, pos);
				end
			end
		end
		
		if self.stage3DoorSequenceTimer:IsPastSimMS(10000) then
			if MovableMan:ValidMO(self.saveTable.stage4Door[1]) then
				local door = self.saveTable.stage4Door[1];
				ToADoor(door):ResetSensorTimer();
				if not self.stage3PlayedDoorStopSound then
					ToADoor(door):StopDoor();
					local soundContainer = CreateSoundContainer("Yskely Refinery Blast Door Stop", "Browncoats.rte");
					soundContainer:Play(door.Pos);
					CameraMan:AddScreenShake(20, door.Pos);
					self.stage3PlayedDoorStopSound = true;
				end
			end
		end

		if self.stage3DoorSequenceTimer:IsPastSimMS(12500) then
			self.Stage = 4;
			self.HUDHandler:SetCameraMinimumAndMaximumX(self.humanTeam, 0, 14500);
			self.HUDHandler:RemoveAllObjectives(self.humanTeam);		
			self.HUDHandler:AddObjective(self.humanTeam,
				"S4DestroyDoor",
				"Find a way to open the door",
				"Attack",
				"Find a way to open the door",
				"They've jammed the last door! Find a way around and let our main force through.",
				nil,
				false,
				true);
		end
		
		self:GetBanner(GUIBanner.YELLOW, 0):ShowText("DOORS OPEN WOW!", GUIBanner.FLYBYLEFTWARD, 1500, Vector(FrameMan.PlayerScreenWidth, FrameMan.PlayerScreenHeight), 0.4, 4000, 0)

		-- Capturables
		
		MovableMan:SendGlobalMessage("ActivateCapturable_RefineryS4BuyDoorConsole1");		
		
	end
	
end

function RefineryAssault:MonitorStage4()

	for k, door in pairs(self.saveTable.stage4Door) do
		if not door or not MovableMan:ValidMO(door) then
		else
			ToADoor(door):ResetSensorTimer();
		end
	end
	
	for k, door in pairs(self.saveTable.stage3Doors) do
		if not door or not MovableMan:ValidMO(door) then
		else
			ToADoor(door):ResetSensorTimer();
		end
	end

end

function RefineryAssault:MonitorStage5()

	local noGenerators = true;

	for i, generator in ipairs(self.saveTable.stage5Generators) do
		if not generator or not MovableMan:ValidMO(generator) then
			self.saveTable.stage5Generators[i] = false;
			self.HUDHandler:RemoveObjective(self.humanTeam, "S5DestroyGenerators" .. i);
		else
			noGenerators = false;
		end
	end
	
	if noGenerators then
		self.Stage = 6;
		self.HUDHandler:RemoveAllObjectives(self.humanTeam);
		
		-- Subcommander door spawn
		
		local squadTypeTable = {"Heavy", "CQB", "Heavy", "Sniper"};
		
		self.saveTable.enemyActorTables.stage6SubCommanderSquad = self.deliveryCreationHandler:CreateSquad(self.aiTeam, squadTypeTable);

		-- note index access, we get a table back
		self.saveTable.stage6subCommander = self.deliveryCreationHandler:CreateEliteSquad(self.aiTeam, 1, "Heavy")[1];
		self.saveTable.stage6Keycard = CreateHeldDevice("Browncoat Military Keycard", "Browncoats.rte");
		self.saveTable.stage6subCommander:AddInventoryItem(self.saveTable.stage6Keycard);
		
		table.insert(self.saveTable.enemyActorTables.stage6SubCommanderSquad, self.saveTable.stage6subCommander);
		
		self.tacticsHandler:AddSquad(self.aiTeam, self.saveTable.enemyActorTables.stage6SubCommanderSquad, "Brainhunt");
		
		for k, item in pairs(self.saveTable.enemyActorTables.stage6SubCommanderSquad) do
			self.stage6SubcommanderDoor:AddInventoryItem(item);
		end
		
		self.stage6SubcommanderDoor:SendMessage("BuyDoor_CustomTableOrder");
		
		self.HUDHandler:QueueCameraPanEvent(self.humanTeam, "S6SubcommanderView", self.stage6SubcommanderDoor.Pos, 0.05, 5000, true);
		
		self.HUDHandler:AddObjective(self.humanTeam,
		"S6GetKeycard",
		"Get the keycard",
		"Attack",
		"Get the subcommander's keycard",
		"That's the commander with the keycard we need. Get it to your own commander.",
		self.saveTable.stage6Keycard,
		false,
		true);
		
		self.HUDHandler:AddObjective(self.humanTeam,
		"S6KillSubcommander",
		"Kill",
		"Attack",
		"Kill the subcommander.",
		"Kill the subcommander for his keycard.",
		self.saveTable.stage6subCommander,
		true,
		true);
		
	end

end

function RefineryAssault:MonitorStage6()

	-- if not self.saveTable.stage6subCommanderKilled then

		-- if not self.saveTable.stage6subCommander or not MovableMan:ValidMO(self.saveTable.stage6subCommander) or self.saveTable.stage6subCommander:IsDead() then
			-- self.HUDHandler:RemoveObjective(self.humanTeam, "S6KillSubcommander");
			-- self.saveTable.stage6subCommanderKilled = true;
		-- end

	-- end	
	
	-- NOTE: on first frame when the keycard disappears it is, for some reason, not in any actor's inventory
	-- so we have to wait a frame
	
	if not self.saveTable.stage6Keycard or (self.saveTable.stage6Keycard.HasEverBeenAddedToMovableMan and not MovableMan:ValidMO(self.saveTable.stage6Keycard)) then
		
		if self.stage7FrameWaited then
		
			local keyCardLost = true;
		
			-- inefficient if misc. actor is holding onto it... and obj arrow disappears
			-- hopefully AI doesn't ever randomly pick it up
			-- alternative: auto drop it, force brain to pick it up first
			for actor in MovableMan.Actors do
				for item in actor.Inventory do
					if item.PresetName == "Browncoat Military Keycard" then
						keyCardLost = false;
						if actor.Team == self.humanTeam and actor:IsInGroup("Brains") then
							-- player brain got it
							actor:RemoveInventoryItem("Browncoat Military Keycard");
							self.HUDHandler:RemoveAllObjectives(self.humanTeam);
							MovableMan:SendGlobalMessage("ActivateCapturable_RefineryS7AuxAuthConsole");
							self.Stage = 7;
							
							for particle in MovableMan.Particles do
								if particle.PresetName == "Refinery Authorization Console" then
								
									particle:SendMessage("ActivateRefineryAuthorizationConsole");
						
									self.HUDHandler:AddObjective(self.humanTeam,
									"S7AuthorizeBrain",
									"Authorize yourself",
									"Attack",
									"Authorize your commander using the keycard",
									"With the keycard, you can authorize your commander's physical signature to open the CNC-center blast door at this console.",
									particle.Pos,
									false,
									true,
									true);					
							
								elseif particle.PresetName == "Refinery S7 Auxiliary Authorization Console" then
								
									self.HUDHandler:AddObjective(self.humanTeam,
									"S7AuxAuth",
									"Hack",
									"Attack",
									"Hack the auxiliary authorization console",
									"This console is also responsible for the CNC-center's door authorization list. Hack it.",
									particle.Pos,
									false,
									true,
									true);
									
								end
							end
							
							return;
						end
					end
				end
			end
			
			if keyCardLost then
				-- spawn a new one
				self.saveTable.stage6Keycard = CreateHeldDevice("Browncoat Military Keycard", "Browncoats.rte");
				self.saveTable.stage6Keycard.Pos = self.stage6SubcommanderDoor.Pos
				MovableMan:AddItem(self.saveTable.stage6Keycard);
			end
		else
			self.stage7FrameWaited = true;
		end
		self.stage7FrameWaited = false;
	end

end

function RefineryAssault:MonitorStage7()

	if self.saveTable.stage7BrainAuthorized and self.saveTable.stage7AuxAuthConsoleCaptured then

		self.Stage = 8;
		
		for particle in MovableMan.Particles do
			if particle.PresetName == "Refinery Boss Door Console" then
	
				particle:SendMessage("ActivateRefineryBossDoorConsole");
	
				self.HUDHandler:AddObjective(self.humanTeam,
				"S8OpenBossDoor",
				"Open the door",
				"Attack",
				"Open the CNC-center door",
				"We're in the home stretch. This is the last door to the main control console of the entire facility. Open it with your authorized commander.",
				particle.Pos,
				false,
				true,
				true);					
				
			end
		end		
		
	end
	
end

function RefineryAssault:MonitorStage8()

	-- nothing to actually do here... handled in messages

end

function RefineryAssault:MonitorStage9()

	if self.saveTable.stage9FinalConsoleCaptured and not self.stage9FinalTimer then
	
		self.stage9FinalTimer = Timer();
		
	elseif self.stage9FinalTimer and self.stage9FinalTimer:IsPastSimMS(5000) then
	
		self.Stage = 10;
		MovableMan:SendGlobalMessage("Refinery_S10SpawnBoss", self.aiTeam);
	
	end


end

function RefineryAssault:MonitorStage10()


end