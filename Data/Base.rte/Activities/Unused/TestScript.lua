-----------------------------------------------------------------------------------------
-- Test a Scene for Compatibility
-----------------------------------------------------------------------------------------

function TestMission:SceneTest()
--[[ THIS TEST IS DONE AUTOMATICALLY BY THE GAME NOW; IT SCANS THE SCRIPT FOR ANY MENTIONS OF "GetArea" AND TESTS THE SCENES FOR HAVING THOSE USED AREAS DEFINED!
	-- See if the required areas are present in the test scene
	if not (TestScene:HasArea("KOTH Hold Area") and TestScene:HasArea("LZ Team 1") and TestScene:HasArea("LZ Team 2")) then
		-- If the test scene failed the compatibility test, invalidate it
		TestScene = nil;
	end
--]]
end

-----------------------------------------------------------------------------------------
-- Start Activity
-----------------------------------------------------------------------------------------

function TestMission:StartActivity()
	print("START! -- TestMission:StartActivity()!");

	self.ActivityState = Activity.STARTING;

	-- Orbit Scene Scanning vars
	self.ScanStage = { PRESCAN = 0, SCANNING = 1, POSTSCAN = 2, SCANSTAGECOUNT = 3 };
	self.CurrentScanStage = self.ScanStage.PRESCAN;
	self.ScanPosX = { [0] = -1, [1] = -1 };
	self.ScanTimer = Timer();
	self.ScanTimer:Reset();
	self.ScanEndPos = Vector();

	self.CPUBrain = nil;
	self.CPUTeam = Activity.TEAM_2;
	self.braindead = {};
	for player = 0, self.PlayerCount - 1 do
		self.braindead[player] = false;
	end
end

-----------------------------------------------------------------------------------------
-- Pause Activity
-----------------------------------------------------------------------------------------

function TestMission:PauseActivity(pause)
	print("PAUSE! -- TestMission:PauseActivity()!");
end

-----------------------------------------------------------------------------------------
-- End Activity
-----------------------------------------------------------------------------------------

function TestMission:EndActivity()
	print("END! -- TestMission:EndActivity()!");
end

-----------------------------------------------------------------------------------------
-- Update Activity
-----------------------------------------------------------------------------------------

function TestMission:UpdateActivity()

	-- Scan the terrain from orbit, revealing all above-ground space for the teams
	if (self.ActivityState == Activity.STARTING) then
		local scanMessage = "Scanning...";
		local messageBlink = 500;

		-- Wait a sec first before starting to scan, so player can get what's going on
		if self.CurrentScanStage == self.ScanStage.PRESCAN then
			scanMessage = "Preparing to scan site from orbit";
			for dotCount = 0, math.floor(self.ScanTimer.ElapsedSimTimeMS / 500) do
				scanMessage = " " .. scanMessage .. ".";
			end
			messageBlink = 0;
--			self:SetObservationTarget(Vector(0, 0), player);
			if self.ScanTimer:IsPastSimMS(2000) then
				self.CurrentScanStage = self.ScanStage.SCANNING;
			end
		-- Do actual scanning process
		elseif self.CurrentScanStage == self.ScanStage.SCANNING then
			doScanStep = self.ScanTimer:IsPastRealMS(SceneMan:GetUnseenResolution(team).X * 2);
			for team = 0, self.TeamCount - 1 do
				if doScanStep then
					-- Scan the column, find the end where the ray is blocked
					SceneMan:CastSeeRay(team, Vector(self.ScanPosX[team], 0), Vector(0, SceneMan.Scene.Height), self.ScanEndPos, 50, SceneMan:GetUnseenResolution(team).Y / 2);
					-- Adjust up a bit so one sees more of the sky than blackness
					self.ScanEndPos.Y = self.ScanEndPos.Y - (FrameMan.PlayerScreenHeight / 4);

					if (self.ScanPosX[team] < SceneMan.Scene.Width) then
						scanMessage = "Scanning...";
						messageBlink = 500;
						-- Move on to the next column
						self.ScanPosX[team] = self.ScanPosX[team] + SceneMan:GetUnseenResolution(team).X;
						self.ScanTimer:Reset();
						-- Set all screens of the teammates to the ray end pos so their screens follow the scanning
						for player = 0, self.PlayerCount - 1 do
							if self:GetTeamOfPlayer(player) == team then
								self:SetViewState(Activity.OBSERVE, player);
								self:SetObservationTarget(self.ScanEndPos, player);
							end
						end
					end
				end

				-- If done scanning, move on the the post pause phase
				if self.ScanPosX[team] > SceneMan.Scene.Width then
					self.CurrentScanStage = self.ScanStage.POSTSCAN;
				end
			end
		-- After scan, pause for a second before moving on to gameplay
		elseif self.CurrentScanStage == self.ScanStage.POSTSCAN then
			if self.ScanTimer:IsPastRealMS(2500) then
				for player = 0, self.PlayerCount - 1 do
					FrameMan:ClearScreenText(player);
					-- Create the landing crew and add to the override purchase list
					self:AddOverridePurchase(CreateACRocket("Rocket MK1"), player);
					self:AddOverridePurchase(CreateAHuman("Brain Robot"), player);
					self:AddOverridePurchase(CreateHDFirearm("Pistol"), player);
					self:SetViewState(Activity.LZSELECT, player);
					self.ActivityState = Activity.RUNNING;
				end
			else
				scanMessage = "Complete!";
				messageBlink = 0;
			end
		end

		-- Display the scanning text on all players' screens
		for player = 0, self.PlayerCount - 1 do
			-- The current player's team
			local team = self:GetTeamOfPlayer(player);
			if (self.ActivityState == Activity.STARTING) then
				FrameMan:ClearScreenText(player);
				FrameMan:SetScreenText(scanMessage, player, messageBlink, 8000, true);
			end
		end
	end

	--------------------------
	-- Game is RUNNING!

	if self.ActivityState == Activity.RUNNING then
		-- Iterate through all human players
		for player = 0, self.PlayerCount - 1 do
			-- The current player's team
			local team = self:GetTeamOfPlayer(player);

			-- Make sure the game is not already ending
			if not (self.ActivityState == Activity.OVER) then

				-- See if the player has a brain
				local brain = self:GetPlayerBrain(player);

				-- Before the player has even had a brain in the scene, so they can't be called braindead yet
				if not self:PlayerHadBrain(player) then
					local foundBrain = MovableMan:GetClosestBrainActor(team);
					-- If we found a brain for this guy to use, then assign and switch to it
					if foundBrain then
						self:SetPlayerBrain(foundBrain, player);
						self:SetViewState(Activity.NORMAL, player);
						self:SwitchToActor(newBrain, player, team);
					end
				else
					-- Check if any player's brain is dead, after they've had one
					if not brain or not MovableMan:IsActor(brain) or not brain:HasObjectInGroup("Brains") then
						self:SetPlayerBrain(nil, player);
						-- Try to find a new unassigned brain this player can use instead, or if his old brain entered a craft
						local newBrain = MovableMan:GetUnassignedBrain(team);
						-- Found new brain actor, assign it and keep on truckin'
						if newBrain and self.braindead[player] == false then
							self:SetPlayerBrain(newBrain, player);
							if MovableMan:IsActor(newBrain) then
								self:SwitchToActor(newBrain, player, team);
							end
						else
							FrameMan:SetScreenText("Your brain has been lost!", player, 333, -1, false);
							self.braindead[player] = true;
							-- Now see if all brains of self player's team are dead, and if so, end the game
							if not MovableMan:GetClosestBrainActor(team) then
								self.WinnerTeam = self:OtherTeam(team);
								ActivityMan:EndActivity();
							end
							self:ResetMessageTimer(player);
						end
					else
						-- Update the observation target to the brain, so that if/when it dies, the view flies to it in observation mode
						self:SetObservationTarget(brain.Pos, player);
					end
				end

--[[
				if self:GetViewState(player) == Activity.OBSERVE and not brain then
					local foundBrain = MovableMan:GetUnassignedBrain(team);
					if foundBrain then
						self:SetPlayerBrain(foundBrain, player);
						self:SetViewState(Activity.NORMAL, player);
					end
				end
]]--

			-- Game over, show the appropriate messages until a certain time
			elseif not self.GameOverTimer:IsPastSimMS(self.GameOverPeriod) then
-- TODO: make more appropriate messages here for run out of funds endings
				if team == self.WinnerTeam then
					FrameMan:SetScreenText("Well done, you retrieved the item!", player, 0, -1, false);
				else
					FrameMan:SetScreenText("Your brain has been lost!", player, 0, -1, false);
				end
			end
		end
	end
end