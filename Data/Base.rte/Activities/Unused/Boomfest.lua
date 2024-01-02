function Boomfest:SceneTest()
--[[ THIS TEST IS DONE AUTOMATICALLY BY THE GAME NOW; IT SCANS THE SCRIPT FOR ANY MENTIONS OF "GetArea" AND TESTS THE SCENES FOR HAVING THOSE USED AREAS DEFINED!
	-- See if the required areas are present in the test scene
	if not (TestScene:HasArea("OMA Spawn") and TestScene:HasArea("LZ Team 1") and TestScene:HasArea("LZ Team 2")) then
		-- If the test scene failed the compatibility test, invalidate it
		TestScene = nil;
	end
--]]
end


function Boomfest:StartActivity()
	print("START! -- Boomfest:StartActivity()!");
	self.Spawn = SceneMan.Scene:GetArea("OMA Spawn");
	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) then
		-- Check if we already have a brain assigned
			if not self:GetPlayerBrain(player) then
				local foundBrain = MovableMan:GetUnassignedBrain(self:GetTeamOfPlayer(player));
				-- If we can't find an unassigned brain in the scene to give each player, then force to go into editing mode to place one
				if not foundBrain then
					foundBrain = CreateAHuman("Soldier Heavy");
					foundBrain:AddInventoryItem(CreateHDFirearm("Rocket Launcher"));
					foundBrain:AddInventoryItem(CreateHDFirearm("Grenade Launcher","Coalition.rte"));
					foundBrain.Pos = self.Spawn:GetCenterPoint();
					MovableMan:AddActor(foundBrain);
					-- Set the found brain to be the selected actor at start
					self:SetPlayerBrain(foundBrain, player);
					self:SwitchToActor(foundBrain, player, self:GetTeamOfPlayer(player));
					self:SetLandingZone(self:GetPlayerBrain(player).Pos, player);
					-- Set the observation target to the brain, so that if/when it dies, the view flies to it in observation mode
					self:SetObservationTarget(self:GetPlayerBrain(player).Pos, player);
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
	end
	self.WList = { "Grenade Launcher", "Spike Launcher", "Flak Cannon", "Bazooka", "Rocket Launcher" };
	self.AList = { "Mia", "Dafred", "Dimitri", "Gordon", "Brutus", "Sandra", "Soldier Light", "Soldier Heavy", "Browncoat Light", "Browncoat Heavy" };
	self.BList = { "Stick Grenade", "Pineapple Grenade", "Grenade", "Explosive Charge", "Cluster Grenade", "Molotov Cocktail", "Incendiary Grenade" };
	self.ESpawnTimer = Timer();
	self.TimeLeft = math.random(10000);
	self.LZ = SceneMan.Scene:GetArea("LZ Team 1");
	self.EnemyLZ = SceneMan.Scene:GetArea("LZ Team 2");
	ActivityMan:GetActivity():SetTeamFunds(0,0);
end

function Boomfest:PauseActivity(pause)
		print("PAUSE! -- Boomfest:PauseActivity()!");
end

function Boomfest:EndActivity()
		print("END! -- Boomfest:EndActivity()!");
end


function Boomfest:UpdateActivity()
	if self.ActivityState ~= Activity.EDITING and self.ActivityState ~= Activity.OVER then
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

		--Spawn the AI.
		if self.ESpawnTimer:LeftTillSimMS(self.TimeLeft) <= 0 then
			local actor = {};
			for x = 0, math.ceil(math.random(3)) do
				actor[x] = CreateAHuman(self.AList[math.random(#self.AList)]);
				actor[x].Team = 1;
				actor[x]:AddInventoryItem(CreateTDExplosive(self.BList[math.random(#self.BList)]));
				actor[x].AIMode = Actor.AIMODE_BRAINHUNT;
				actor[x]:AddInventoryItem(CreateHDFirearm(self.WList[math.random(#self.WList)]));
			end
			local ship = nil;
			local z = math.random();
			if z > 0.90 then
				ship = CreateACRocket("Drop Crate","Dummy.rte");
			elseif z > 0.80 then
				ship = CreateACRocket("Rocklet","Dummy.rte");
			elseif z > 0.60 then
				ship = CreateACDropShip("Drop Ship","Dummy.rte");
			elseif z > 0.40 then
				ship = CreateACDropShip("Drop Ship MK1","Coalition.rte");
			elseif z > 0.20 then
				ship = CreateACRocket("Rocket MK2","Coalition.rte");
			else
				ship = CreateACRocket("Rocket MK1","Coalition.rte");
			end
			for n = 0, #actor do
				ship:AddInventoryItem(actor[n]);
			end
			ship.Team = 1;
			local w = math.random();
			if w > 0.5 then
				ship.Pos = Vector(self.EnemyLZ:GetRandomPoint().X,-50);
			else
				ship.Pos = Vector(self.LZ:GetRandomPoint().X,-50);
			end
			MovableMan:AddActor(ship);
			self.ESpawnTimer:Reset();
			self.TimeLeft = math.random(5000)+5000;
		end
	end
end