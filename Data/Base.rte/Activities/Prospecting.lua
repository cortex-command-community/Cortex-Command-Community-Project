package.loaded.Constants = nil; require("Constants");

function Prospecting:SceneTest()

end

-----------------------------------------------------------------------------------------
-- Start Activity
-----------------------------------------------------------------------------------------

function Prospecting:StartActivity()
	print("START! -- Prospecting:StartActivity()!");

	self.ActivityState = Activity.STARTING;

	-- Orbit Scene Scanning vars
	self.ScanStage = { PRESCAN = 0, SCANNING = 1, POSTSCAN = 2, SCANSTAGECOUNT = 3 };
	self.CurrentScanStage = self.ScanStage.PRESCAN;
	self.ScanPosX = { [Activity.TEAM_1] = -1, [Activity.TEAM_2] = -1, [Activity.TEAM_3] = -1, [Activity.TEAM_4] = -1 };
	self.ScanTimer = { [Activity.TEAM_1] = Timer(), [Activity.TEAM_2] = Timer(), [Activity.TEAM_3] = Timer(), [Activity.TEAM_4] = Timer() };
	self.ScanTimer[Activity.TEAM_1]:Reset();
	self.ScanTimer[Activity.TEAM_2]:Reset();
	self.ScanTimer[Activity.TEAM_3]:Reset();
	self.ScanTimer[Activity.TEAM_4]:Reset();
	self.ScanEndPos = Vector();

	self.CPUBrain = nil;
	self.CPUTeam = Activity.TEAM_2;
	self.braindead = {};
	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) then
			self.braindead[player] = false;
		end
	end

	-- Set up the unseen layers
	SceneMan:MakeAllUnseen(Vector(25, 25), Activity.TEAM_1);
	SceneMan:LoadUnseenLayer("Base.rte/Scenes/UnseenTest.png", Activity.TEAM_2);
end

-----------------------------------------------------------------------------------------
-- Pause Activity
-----------------------------------------------------------------------------------------

function Prospecting:PauseActivity(pause)
	print("PAUSE! -- Prospecting:PauseActivity()!");
end

-----------------------------------------------------------------------------------------
-- End Activity
-----------------------------------------------------------------------------------------

function Prospecting:EndActivity()
	print("END! -- Prospecting:EndActivity()!");
end

-----------------------------------------------------------------------------------------
-- Update Activity
-----------------------------------------------------------------------------------------

function Prospecting:UpdateActivity()

	-- Scan the terrain from orbit, revealing all above-ground space for the teams
	if (self.ActivityState == Activity.STARTING) then
		local scanMessage = "Scanning...";
		local messageBlink = 500;

		-- Wait a sec first before starting to scan, so player can get what's going on
		if self.CurrentScanStage == self.ScanStage.PRESCAN then
			scanMessage = "Preparing to scan site from orbit";
			for dotCount = 0, math.floor(self.ScanTimer[Activity.TEAM_1].ElapsedSimTimeMS / 500) do
				scanMessage = " " .. scanMessage .. ".";
			end
			messageBlink = 0;
--			self:SetObservationTarget(Vector(0, 0), player);
			if self.ScanTimer[Activity.TEAM_1]:IsPastSimMS(2000) then
				self.CurrentScanStage = self.ScanStage.SCANNING;
			end
		-- Do actual scanning process
		elseif self.CurrentScanStage == self.ScanStage.SCANNING then
			for team = Activity.TEAM_1, Activity.MAXTEAMCOUNT - 1 do
				if self:TeamActive(team) then
					-- Time to do a scan step?
					if self.ScanTimer[team]:IsPastRealMS(SceneMan:GetUnseenResolution(team).X * 2) then
						-- Scan the column, find the end where the ray is blocked
						SceneMan:CastSeeRay(team, Vector(self.ScanPosX[team], 0), Vector(0, SceneMan.Scene.Height), self.ScanEndPos, 50, SceneMan:GetUnseenResolution(team).Y / 2);
						-- Adjust up a bit so one sees more of the sky than blackness
						self.ScanEndPos.Y = self.ScanEndPos.Y - (FrameMan.PlayerScreenHeight / 4);

						if (self.ScanPosX[team] < SceneMan.Scene.Width) then
							scanMessage = "Scanning...";
							messageBlink = 500;
							-- Move on to the next column
							self.ScanPosX[team] = self.ScanPosX[team] + SceneMan:GetUnseenResolution(team).X;
							self.ScanTimer[team]:Reset();
							-- Set all screens of the teammates to the ray end pos so their screens follow the scanning
							for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
								if self:PlayerActive(player) and self:PlayerHuman(player) then
									if self:GetTeamOfPlayer(player) == team then
										self:SetViewState(Activity.OBSERVE, player);
										self:SetObservationTarget(self.ScanEndPos, player);
									end
								end
							end
						end
					end
				end
			end
			-- If done scanning BOTH TEAMS, move on the the post pause phase
			if self.ScanPosX[Activity.TEAM_1] > SceneMan.Scene.Width and self.ScanPosX[Activity.TEAM_2] > SceneMan.Scene.Width then
				self.CurrentScanStage = self.ScanStage.POSTSCAN;
			end

		-- After scan, pause for a second before moving on to gameplay
		elseif self.CurrentScanStage == self.ScanStage.POSTSCAN then
			if self.ScanTimer[Activity.TEAM_1]:IsPastRealMS(2500) and self.ScanTimer[Activity.TEAM_2]:IsPastRealMS(2500) then
				for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
					if self:PlayerActive(player) and self:PlayerHuman(player) then
						FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
						-- Create the landing crew and add to the override purchase list
						self:SetOverridePurchaseList("Infantry Brain", player);
						self:SetViewState(Activity.LZSELECT, player);
					end
				end
				self.ActivityState = Activity.RUNNING;
			else
				scanMessage = "Complete!";
				messageBlink = 0;
			end
		end

		-- Display the scanning text on all players' screens
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self:PlayerActive(player) and self:PlayerHuman(player) then
				-- The current player's team
				local team = self:GetTeamOfPlayer(player);
				if (self.ActivityState == Activity.STARTING) then
					FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
					FrameMan:SetScreenText(scanMessage, self:ScreenOfPlayer(player), messageBlink, 8000, true);
				end
			end
		end
	end

	--------------------------
	-- Game is RUNNING!

	if self.ActivityState == Activity.RUNNING then
		-- Iterate through all human players
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self:PlayerActive(player) and self:PlayerHuman(player) then
				-- The current player's team
				local team = self:GetTeamOfPlayer(player);

				-- Make sure the game is not already ending
				if not (self.ActivityState == Activity.OVER) then

					-- See if the player has a brain
					local brain = self:GetPlayerBrain(player);

					-- Before the player has even had a brain in the scene, so they can't be called braindead yet
					if not self:PlayerHadBrain(player) then
						local foundBrain = MovableMan:GetFirstBrainActor(team);
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
								FrameMan:SetScreenText("Your brain has been lost!", self:ScreenOfPlayer(player), 333, -1, false);
								self.braindead[player] = true;
								-- Now see if all brains of self player's team are dead, and if so, end the game
								if not MovableMan:GetFirstBrainActor(team) then
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
						FrameMan:SetScreenText("Well done, you retrieved the item!", self:ScreenOfPlayer(player), 0, -1, false);
					else
						FrameMan:SetScreenText("Your brain has been lost!", self:ScreenOfPlayer(player), 0, -1, false);
					end
				end
			end
		end
	end
end