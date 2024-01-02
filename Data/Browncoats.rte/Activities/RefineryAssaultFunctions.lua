function RefineryAssault:HandleMessage(message, object)

	self.tacticsHandler:OnMessage(message, object);

	print("activitygotmessage")
	
	print(message)
	print(object)
	
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
			self.saveTable.buyDoorTables.teamAreas[self.aiTeam].LC1 = nil;
			
			for k, v in pairs(self.saveTable.buyDoorTables.LC1) do
				v.Team = self.humanTeam;
			end
		else
			print("NOTHUMAN CAPPED 1");
			print(self.humanTeam .. " team vs object: " .. object);
			self.stage2HoldingLC1 = false;
			self.stage2HoldingBothConsoles = false;
		
			table.insert(self.saveTable.buyDoorTables.teamAreas[self.aiTeam], "LC1");
			self.saveTable.buyDoorTables.teamAreas[self.humanTeam].LC1 = nil;
			
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
			self.saveTable.buyDoorTables.teamAreas[self.aiTeam].LC2 = nil;
			
			for k, v in pairs(self.saveTable.buyDoorTables.LC2) do
				v.Team = self.humanTeam;
			end
		else
			self.stage2HoldingLC2 = false;
			self.stage2HoldingBothConsoles = false;
			
			table.insert(self.saveTable.buyDoorTables.teamAreas[self.aiTeam], "LC2");
			self.saveTable.buyDoorTables.teamAreas[self.humanTeam].LC2 = nil;
			
			for k, v in pairs(self.saveTable.buyDoorTables.LC2) do
				v.Team = self.aiTeam;
			end		
		end
		
		-- as soon as any of the hack consoles are captured, we don't wanna bother with the stage 1 counterattack anymore.
		self.tacticsHandler:RemoveTask("Counterattack", self.aiTeam);
		
	elseif message == "Captured_RefineryS3BuyDoorConsole1" then
		
		table.insert(self.saveTable.buyDoorTables.teamAreas[object], "S3_1");
		-- todo make this team selection better somehow... or maybe It Just Works. dunno. it's ugly.
		self.saveTable.buyDoorTables.teamAreas[(object + 1) % 2].S3_1 = nil;
		
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
		self.saveTable.buyDoorTables.teamAreas[(object + 1) % 2].S3_2 = nil;
		
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
		self.saveTable.buyDoorTables.teamAreas[(object + 1) % 2].S3_3 = nil;
		
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
		
	elseif message == "Captured_RefineryS3BuyDoorConsole4" then
		
		table.insert(self.saveTable.buyDoorTables.teamAreas[object], "S3_4");
		-- todo make this team selection better somehow... or maybe It Just Works. dunno. it's ugly.
		self.saveTable.buyDoorTables.teamAreas[(object + 1) % 2].S3_4 = nil;
		
		for k, v in pairs(self.saveTable.buyDoorTables.S3_4) do
			v.Team = object;
		end
		
		-- dupe code woo
		if object == self.humanTeam then		
			local pos;
			for particle in MovableMan.Particles do
				if particle.PresetName == "Refinery S3 Buy Door Console 4" then
					pos = particle.Pos;
					break;
				end
			end		
			self.tacticsHandler:AddTask("Attack S3 Buy Door Console 4", self.aiTeam, pos, "Attack", 20);		
		else
			self.tacticsHandler:RemoveTask("Attack S3 Buy Door Console 4", self.aiTeam);		
		end
		
	elseif message == "Captured_RefineryS3OilCapturable" then
	
		self.humanAIGoldIncreaseAmount = self.humanAIGoldIncreaseAmount + 20;		
		self.playerGoldIncreaseAmount = self.playerGoldIncreaseAmount + 20;
		
		local soundContainer = CreateSoundContainer("Yskely Refinery Oil Spout Engage", "Browncoats.rte");
		soundContainer:Play(Vector(0, 0));
	
	elseif message == "Captured_RefineryS3FireWeaponryConsole" then	
		
		self.deliveryCreationHandler:AddAvailablePreset(self.humanTeam, "FL-200 Heatlance", "HDFirearm", "Browncoats.rte");
		self.deliveryCreationHandler:AddAvailablePreset(self.humanTeam, "IN-02 Backblast", "HDFirearm", "Browncoats.rte");
		
		self.deliveryCreationHandler:RemoveAvailablePreset(self.aiTeam, "FL-200 Heatlance");
		-- don't remove the IN-02, castrates the brownies too much this early on.
		
	elseif message == "Captured_RefineryS3DrillOverloadConsole" then	
		
		self.HUDHandler:RemoveObjective(self.humanTeam, "S3OverloadDrill");
		self.stage3DrillOverloaded = true;
		
	elseif message == "RefineryAssault_S4DoorsBlownUp" then	
		
		if self.Stage ~= 5 then
		
			self.Stage = 5;
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

		end
		
	end
	
	
	-- DEBUG STAGE SKIPS

	-- ActivityMan:GetActivity():SendMessage("SkipCurrentStage");
	if message == "SkipCurrentStage" then
		message = "SkipStage" .. self.Stage;
	end

	-- ActivityMan:GetActivity():SendMessage("SkipStage1");
	-- ActivityMan:GetActivity():SendMessage("SkipStage2");
	-- ActivityMan:GetActivity():SendMessage("SkipStage3");
	if message == "SkipStage1" then
		for a in MovableMan.Actors do if a.Team == 1 and a.ClassName == "AHuman" and SceneMan.Scene:WithinArea("Mission Stage Area 1", a.Pos) then a.Health = 0 end end
	elseif message == "SkipStage2" then
		self.stage2HoldingBothConsoles = true;
		self.stage2TimeToHoldConsoles = 0;
	elseif message == "SkipStage3" then
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
		
	end
	
	

end



function RefineryAssault:SendDockDelivery(team, task, forceRocketUsage, squadType)

	local squadCount = math.random(3, 4);

	local craft;
	local order;
	local goldCost;

	if squadType == "Elite" then
		craft, squad, goldCost = self.deliveryCreationHandler:CreateEliteSquadWithCraft(team, forceRocketUsage, squadCount);
	else
		craft, squad, goldCost = self.deliveryCreationHandler:CreateSquadWithCraft(team, forceRocketUsage, squadCount, squadType);
	end
	
	local success = self.dockingHandler:SpawnDockingCraft(craft)
			
	if success then
		self.tacticsHandler:ApplyTaskToSquadActors(squad, task);
		self:ChangeAIFunds(team, -goldCost);
		return squad
	end
	
	return false;
	
end

function RefineryAssault:SendBuyDoorDelivery(team, task, squadType, specificIndex)

	local squadCount = math.random(1, 2);

	local order;
	local goldCost;

	if squadType == "Elite" then
		order, goldCost = self.deliveryCreationHandler:CreateEliteSquad(team, squadCount);
	else
		order, goldCost = self.deliveryCreationHandler:CreateSquad(team, squadCount, squadType);
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
		
		if areaThisIsIn then
			--print(areaThisIsIn.Name)
			
			local randomSelection;
			local usableBuyDoorTable = self.buyDoorHandler:GetAvailableBuyDoorsInArea(areaThisIsIn, team)
			
			if usableBuyDoorTable then
				randomSelection = usableBuyDoorTable[math.random(1, #usableBuyDoorTable)]
			end
			
			if randomSelection then
				local success = self.buyDoorHandler:SendCustomOrder(order, team, randomSelection);
				if success then
					self.tacticsHandler:ApplyTaskToSquadActors(order, task);
					self:ChangeAIFunds(team, -goldCost);
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
		self.tacticsHandler:AddTaskedSquad(self.aiTeam, stage1SquadsTable[k], task.Name);
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
	
	local squad = self:SendDockDelivery(self.humanTeam, task, false, "Elite");
	
	self.tacticsHandler:AddTaskedSquad(self.humanTeam, squad, task.Name);
	
	squad = self:SendDockDelivery(self.humanTeam, task, false, "Elite");
	
	self.tacticsHandler:AddTaskedSquad(self.humanTeam, squad, task.Name);
	
	-- Set up player squad and dropship
	
	local dropShip = self.deliveryCreationHandler:CreateEliteSquadWithCraft(self.humanTeam, false, 5);
	local dropShipPos = SceneMan.Scene:GetOptionalArea("RefineryAssault_HumanBrainSpawn").Center;
	dropShip.Team = self.humanTeam;
	dropShip.Pos = dropShipPos;
	dropShip.AIMode = Actor.AIMODE_SENTRY;
	dropShip.PlayerControllable = true;
	
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
		MovableMan:AddActor(brain);
		--dropShip:AddInventoryItem(brain);
	end
		
	MovableMan:AddActor(dropShip)
	dropShip:OpenHatch();
	
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

end

function RefineryAssault:MonitorStage1()

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
			
			self.tacticsHandler:AddTaskedSquad(self.aiTeam, self.saveTable.enemyActorTables.stage1CounterAttActors, task.Name);
			
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
		
		for k, v in pairs(self.saveTable.buyDoorTables.S3_4) do
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
				print(console)
				print(k)
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
		
		self.HUDHandler:QueueCameraPanEvent(self.humanTeam, "S3DoorSequence", pos, 0.08, 10500, true);

	elseif self.stage3DoorSequenceTimer and self.stage3DoorSequenceTimer:IsPastSimMS(6250) then
	
		if not self.stage3ScreenShake then
			self.stage3ScreenShake = true;
			local pos = SceneMan.Scene:GetOptionalArea("RefineryAssault_S3DoorSequenceArea").Center;
			CameraMan:AddScreenShake(10, pos);
		end
		
		for k, door in pairs(self.saveTable.stage4Door) do
			if MovableMan:ValidMO(door) then
				ToADoor(door):OpenDoor();
			end
		end
		
		for k, door in pairs(self.saveTable.stage3Doors) do
			if MovableMan:ValidMO(door) then
				ToADoor(door):OpenDoor();
			end
		end
		
		if self.stage3DoorSequenceTimer:IsPastSimMS(7250) then
			for k, door in pairs(self.saveTable.stage4Door) do
				if MovableMan:ValidMO(door) then
					ToADoor(door):StopDoor();
					self.Stage = 4;
					local soundContainer = CreateSoundContainer("Yskely Refinery Blast Door Stop", "Browncoats.rte");
					soundContainer:Play(door.Pos);
					CameraMan:AddScreenShake(20, door.Pos);
					
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
			end
		end
		
		self:GetBanner(GUIBanner.YELLOW, 0):ShowText("DOORS OPEN WOW!", GUIBanner.FLYBYLEFTWARD, 1500, Vector(FrameMan.PlayerScreenWidth, FrameMan.PlayerScreenHeight), 0.4, 4000, 0)

		-- Capturables
		
		MovableMan:SendGlobalMessage("ActivateCapturable_RefineryS3BuyDoorConsole4");		
		
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
	end

end

function RefineryAssault:MonitorStage6()

end