-----------------------------------------------------------------------------------------
-- Test a Scene for Compatibility
-----------------------------------------------------------------------------------------

function ZombiePitMission:SceneTest()
--[[ THIS TEST IS DONE AUTOMATICALLY BY THE GAME NOW; IT SCANS THE SCRIPT FOR ANY MENTIONS OF "GetArea" AND TESTS THE SCENES FOR HAVING THOSE USED AREAS DEFINED!
	-- See if the required areas are present in the test scene
	if not (TestScene:HasArea("OMA Spawn") and TestScene:HasArea("LZ Team 1") and TestScene:HasArea("LZ Team 2")) then
		-- If the test scene failed the compatibility test, invalidate it
		TestScene = nil;
	end
--]]
end

-----------------------------------------------------------------------------------------
-- Start Activity
-----------------------------------------------------------------------------------------

function ZombiePitMission:StartActivity()
	print("START! -- ZombiePitMission:StartActivity()!");

	self.FightStage = { NOFIGHT = 0, DEFENDING = 1, ATTACK = 2, FIGHTSTAGECOUNT = 3 };

	self.AreaTimer = Timer();
	self.StepTimer = Timer();
	self.SpawnTimer = Timer();
	self.ScreenChange = false;

	self.CurrentFightStage = self.FightStage.NOFIGHT;
	self.CPUBrain = nil;

	--------------------------
	-- Set up teams

	-- Team 2 is always CPU
	self.CPUTeam = Activity.TEAM_2;

--	for team = 0, self.TeamCount - 1 do

--	end

	--------------------------
	-- Set up players

	for player = 0, self.PlayerCount - 1 do
		-- Check if we already have a brain assigned
		if not self:GetPlayerBrain(player) then
			local foundBrain = MovableMan:GetUnassignedBrain(self:GetTeamOfPlayer(player));
			-- If we can't find an unassigned brain in the scene to give each player, then force to go into editing mode to place one
			if not foundBrain then
				self.ActivityState = Activity.EDITING;
				AudioMan:ClearMusicQueue();
				AudioMan:PlayMusic("Base.rte/Music/dBSoundworks/ccambient4.ogg", -1, -1);
			else
				-- Set the found brain to be the selected actor at start
				self:SetPlayerBrain(foundBrain, player);
				self:SwitchToActor(foundBrain, player, self:GetTeamOfPlayer(player));
				self:SetLandingZone(self:GetPlayerBrain(player).Pos, player);
				-- Set the observation target to the brain, so that if/when it dies, the view flies to it in observation mode
				self:SetObservationTarget(self:GetPlayerBrain(player).Pos, player);
			end
		end
	end

	-- Set up AI modes
	for actor in MovableMan.Actors do
		if actor.Team == self.CPUTeam then
			actor:SetControllerMode(Controller.CIM_AI, -1);
			actor.AIMode = Actor.AIMODE_BRAINHUNT;
		end
	end

	--------------------------
	-- Set up tutorial

	self.AreaTimer:Reset();
	self.StepTimer:Reset();
	self.SpawnTimer:Reset();

end

-----------------------------------------------------------------------------------------
-- Pause Activity
-----------------------------------------------------------------------------------------

function ZombiePitMission:PauseActivity(pause)
	print("PAUSE! -- ZombiePitMission:PauseActivity()!");
end

-----------------------------------------------------------------------------------------
-- End Activity
-----------------------------------------------------------------------------------------

function ZombiePitMission:EndActivity()
	print("END! -- ZombiePitMission:EndActivity()!");
end

-----------------------------------------------------------------------------------------
-- Update Activity
-----------------------------------------------------------------------------------------

function ZombiePitMission:UpdateActivity()
	-- Avoid game logic when we're editing
	if (self.ActivityState == Activity.EDITING) then
-- Don't update the editing, it's being done already by GameActivity
--		self:UpdateEditing();
		return;
	end

	--------------------------
	-- Iterate through all human players

	for player = 0, self.PlayerCount - 1 do
		-- The current player's team
		local team = self:GetTeamOfPlayer(player);

		-- Make sure the game is not already ending
		if not (self.ActivityState == Activity.OVER) then
			-- Check if any player's brain is dead
			if not MovableMan:IsActor(self:GetPlayerBrain(player)) then
				self:SetPlayerBrain(nil, player);
				FrameMan:SetScreenText("Your brain has been destroyed!", player, 333, -1, false);

				-- Now see if all brains of self player's team are dead, and if so, end the game
				if not MovableMan:GetClosestBrainActor(team) then
					self.WinnerTeam = self:OtherTeam(team);
					ActivityMan:EndActivity();
				end

				self:ResetMessageTimer(player);
			end
		-- Game over, show the appropriate messages until a certain time
		elseif not self.GameOverTimer:IsPastSimMS(self.GameOverPeriod) then
-- TODO: make more appropriate messages here for run out of funds endings
			if team == self.WinnerTeam then
				FrameMan:SetScreenText("Your competition's wetware is mush!", player, 0, -1, false);
			else
				FrameMan:SetScreenText("Your brain has been destroyed!", player, 0, -1, false);
			end
		end
	end

	------------------------------------------------
	-- Iterate through all teams

	for team = 0, self.TeamCount - 1 do

		-------------------------------------------
		-- Check for victory conditions

		-- Make sure the game isn't already ending
		if (not (self.ActivityState == Activity.OVER)) and (not (team == self.CPUTeam)) then

		end
	end

	--------------------------------------------
	-- Battle logic

	if self:GetControlledActor(Activity.PLAYER_1) then
		-- Triggered defending stage
		if self.CurrentFightStage == self.FightStage.NOFIGHT and SceneMan.Scene:WithinArea("AIAttack", self:GetControlledActor(Activity.PLAYER_1).Pos) then
			-- Take over control of screen messages
			self:ResetMessageTimer(Activity.PLAYER_1);
			-- Display the text of the current step
			FrameMan:ClearScreenText(Activity.PLAYER_1);
			FrameMan:SetScreenText("DEFEND YOUR BRAIN AGAINST THE INCOMING FORCES!", Activity.PLAYER_1, 500, 8000, true);
			-- self will make all the enemy team AI's go into brain hunt mode
			for actor in MovableMan.Actors do
				if actor.Team == self.CPUTeam then
					actor:SetControllerMode(Controller.CIM_AI, -1);
					actor.AIMode = Actor.AIMODE_BRAINHUNT;
				end
			end
			-- Advance the stage
			self.CurrentFightStage = self.FightStage.DEFENDING;
		end
	end

	if self.SpawnTimer:IsPastSimMS(4000) then
--		local ship = CreateACDropShip("Drop Ship MK1");
		local zombie;
		which = math.random();
		if which > 0.76 then
			zombie = CreateAHuman("Skeleton");
		elseif which > 0.66 then
			zombie = CreateAHuman("Zombie Thin");
		elseif which > 0.33 then
			zombie = CreateAHuman("Zombie Medium");
		else
			zombie = CreateAHuman("Zombie Fat");
		end

		local bomb = CreateTDExplosive("Blue Bomb");
		zombie:AddInventoryItem(bomb);

--		local gun = CreateHDFirearm("AK-47");
--		zombie:AddInventoryItem(gun);
--		ship:AddInventoryItem(zombie);
		-- See if there is a designated LZ Area for attackers, and only land over it
		local attackLZ = SceneMan.Scene:GetArea("Zombie Spawn 1");
		if attackLZ then
			zombie.Pos = attackLZ:GetRandomPoint();
		else
			-- Will appear anywhere when there is no designated LZ
			zombie.Pos = Vector(SceneMan.Scene.Width * math.random(), 0);
		end
		zombie.Team = self.CPUTeam;
		zombie:SetControllerMode(Controller.CIM_AI, -1);
		-- Let the spawn into the world, passing ownership
		MovableMan:AddActor(zombie);
		-- Wait another period for next spawn
		self.SpawnTimer:Reset();
	end

	-- TEST KILL ZONE
--[[
	for actor in MovableMan.Actors do
		if SceneMan.Scene:WithinArea("Kill Zone", actor.Pos) then
			actor:GibThis();
		end
	end
--]]
	-----------------------------------------------------
	-- Check for victory conditions
--[[
	-- Check if the CPU brain is dead
	if not MovableMan:IsActor(self.CPUBrain) and ActivityMan:ActivityRunning() then
		self.CPUBrain = nil;
		-- Proclaim player winner end
		self.WinnerTeam = Activity.TEAM_1;
		-- Finito!
		ActivityMan:EndActivity();
	end
--]]
end