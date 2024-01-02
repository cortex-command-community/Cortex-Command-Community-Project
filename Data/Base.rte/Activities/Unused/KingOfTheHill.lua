function KingOfTheHill:SceneTest()
--[[ THIS TEST IS DONE AUTOMATICALLY BY THE GAME NOW; IT SCANS THE SCRIPT FOR ANY MENTIONS OF "GetArea" AND TESTS THE SCENES FOR HAVING THOSE USED AREAS DEFINED!
	-- See if the required areas are present in the test scene
	if not (TestScene:HasArea("KOTH Hold Area") and TestScene:HasArea("LZ Team 1") and TestScene:HasArea("LZ Team 2")) then
		-- If the test scene failed the compatibility test, invalidate it
		TestScene = nil;
	end
--]]
end

function KingOfTheHill:StartActivity()
	print("START! -- KingOfTheHill:StartActivity()!");
	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) then
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
	end

	self.HoldArea = SceneMan.Scene:GetArea("KOTH Hold Area");
	self.Timer1 = Timer();
	self.Timer2 = Timer();
	self.Left1 = 180000; -- 3 minutes.
	self.Left2 = 180000; -- 3 minutes.
	local LZ1 = SceneMan.Scene:GetArea("LZ Team 1");
	local LZ2 = SceneMan.Scene:GetArea("LZ Team 2");
	self:SetLandingZone(LZ1:GetCenterPoint(), Activity.PLAYER_1);
	self:SetLandingZone(LZ2:GetCenterPoint(), Activity.PLAYER_2);
end

function KingOfTheHill:PauseActivity(pause)
		print("PAUSE! -- KingOfTheHill:PauseActivity()!");
end

function KingOfTheHill:EndActivity()
		print("END! -- KingOfTheHill:EndActivity()!");
end


function KingOfTheHill:UpdateActivity()
	if self.ActivityState == Activity.RUNNING then
		local Player1 = nil;
		local Player2 = nil;
		local Player1Hold = false;
		local Player2Hold = false;

		--See who's holding the area.
		local PointHolder = nil;

		for actor in MovableMan.Actors do
			if self.HoldArea:IsInside(actor.Pos) then
			if actor.Team == Activity.TEAM_1 then
				Player1Hold = true;
				Player1 = actor;
			elseif actor.Team == Activity.TEAM_2 then
				Player2Hold = true;
				Player2 = actor;
			end
			 end
		end

		if Player1Hold == true and Player2Hold == false then
			PointHolder = Activity.TEAM_1;
		elseif Player2Hold == true and Player1Hold == false then
			PointHolder = Activity.TEAM_2;
		elseif Player2Hold == true and Player1Hold == true then
			PointHolder = -1;
		end

		--Mark the objectives.
		self:ClearObjectivePoints();
		if self.ActivityState ~= Activity.OVER then
			if PointHolder == nil then
				self:AddObjectivePoint("Hold this point!", self.HoldArea:GetCenterPoint()+Vector(0,-96), Activity.TEAM_2, GameActivity.ARROWDOWN);
				self:AddObjectivePoint("Hold this point!", self.HoldArea:GetCenterPoint()+Vector(0,-96), Activity.TEAM_1, GameActivity.ARROWDOWN);
			end
			if PointHolder == -1 then
				self:AddObjectivePoint("Hold for " .. math.ceil(self.Left1/1000) .. " seconds!", Player1.AboveHUDPos + Vector(0,-4), Activity.TEAM_1, GameActivity.ARROWDOWN);
				self:AddObjectivePoint("Hold for " .. math.ceil(self.Left2/1000) .. " seconds!", Player2.AboveHUDPos + Vector(0,-4), Activity.TEAM_2, GameActivity.ARROWDOWN);
				self:AddObjectivePoint("Kill before " .. math.ceil(self.Left1/1000) .. " seconds are up!", Player1.AboveHUDPos + Vector(0,-4), Activity.TEAM_2, GameActivity.ARROWDOWN);
				self:AddObjectivePoint("Kill before " .. math.ceil(self.Left2/1000) .. " seconds are up!", Player2.AboveHUDPos + Vector(0,-4), Activity.TEAM_1, GameActivity.ARROWDOWN);
				self.Left1 = self.Left1+self.Timer1:LeftTillSimMS(0);
				self.Left2 = self.Left2+self.Timer2:LeftTillSimMS(0);
			end
			if PointHolder == Activity.TEAM_1 then
				self:AddObjectivePoint("Hold for " .. math.ceil(self.Left1/1000) .. " seconds!", Player1.AboveHUDPos + Vector(0,-4), Activity.TEAM_1, GameActivity.ARROWDOWN);
				self:AddObjectivePoint("Kill before " .. math.ceil(self.Left1/1000) .. " seconds are up!", Player1.AboveHUDPos + Vector(0,-4), Activity.TEAM_2, GameActivity.ARROWDOWN);
				self:AddObjectivePoint("Hold this point!", self.HoldArea:GetCenterPoint()+Vector(0,-96), Activity.TEAM_2, GameActivity.ARROWDOWN);
				self.Left1 = self.Left1+self.Timer1:LeftTillSimMS(0);
			end
			if PointHolder == Activity.TEAM_2 then
				self:AddObjectivePoint("Hold for " .. math.ceil(self.Left2/1000) .. " seconds!", Player2.AboveHUDPos + Vector(0,-4), Activity.TEAM_2, GameActivity.ARROWDOWN);
				self:AddObjectivePoint("Kill before " .. math.ceil(self.Left2/1000) .. " seconds are up!", Player2.AboveHUDPos + Vector(0,-4), Activity.TEAM_1, GameActivity.ARROWDOWN);
				self:AddObjectivePoint("Hold this point!", self.HoldArea:GetCenterPoint()+Vector(0,-96), Activity.TEAM_1, GameActivity.ARROWDOWN);
				self.Left2 = self.Left2+self.Timer2:LeftTillSimMS(0);
			end
		end
		self:YSortObjectivePoints();

		self.Timer1:Reset();
		self.Timer2:Reset();

		--Check to see if there's a winner.
		if self.Left1 <= 0 then
			self.WinnerTeam = Activity.TEAM_1;
			ActivityMan:EndActivity();
		end

		if self.Left2 <= 0 then
			self.WinnerTeam = Activity.TEAM_2;
			ActivityMan:EndActivity();
		end
	end
end