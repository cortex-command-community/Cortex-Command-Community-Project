package.loaded.Constants = nil; require("Constants");

function SiteScan:SceneTest()

end

-----------------------------------------------------------------------------------------
-- Start Activity
-----------------------------------------------------------------------------------------

function SiteScan:StartActivity()
	print("START! -- SiteScan:StartActivity()!");

	self.GameOverPeriod = 0;
	
	-- Orbit Scene Scanning vars
	self.ScanStage = { PRESCAN = 0, SCANNING = 1, POSTSCAN = 2, SCANSTAGECOUNT = 3 };
	self.CurrentScanStage = self.ScanStage.PRESCAN;
	self.ScanPosX = { [Activity.TEAM_1] = -1, [Activity.TEAM_2] = -1, [Activity.TEAM_3] = -1, [Activity.TEAM_4] = -1 };
	self.ScanTimer = { [Activity.TEAM_1] = Timer(), [Activity.TEAM_2] = Timer(), [Activity.TEAM_3] = Timer(), [Activity.TEAM_4] = Timer() };
	self.StartFunds = { [Activity.TEAM_1] = 0, [Activity.TEAM_2] = 0, [Activity.TEAM_3] = 0, [Activity.TEAM_4] = 0 };
	self.ScanEndPos = Vector();

	for team = Activity.TEAM_1, Activity.MAXTEAMCOUNT - 1 do
		if self:TeamActive(team) then
			self.StartFunds[team] = self:GetTeamFunds(team);
			self.ScanTimer[team]:Reset();
		end
	end

	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) then
			-- Determine the team that is doing the scanning
			self.ScanTeam = self:GetTeamOfPlayer(player);
			-- Set the view to observation, since that's all we're doing here
			self:SetViewState(Activity.OBSERVE, player);
			-- Clear out the banners, we don't need them showing "GO!" here
			self:GetBanner(GUIBanner.YELLOW, player):ClearText();
			self:GetBanner(GUIBanner.RED, player):ClearText();
		end
	end

	-- Place resident brains into the simulation so they'll be collected properly afterwards
	SceneMan.Scene:PlaceResidentBrains(self);

end

-----------------------------------------------------------------------------------------
-- Pause Activity
-----------------------------------------------------------------------------------------

function SiteScan:PauseActivity(pause)
	print("PAUSE! -- SiteScan:PauseActivity()!");
end

-----------------------------------------------------------------------------------------
-- End Activity
-----------------------------------------------------------------------------------------

function SiteScan:EndActivity()
	print("END! -- SiteScan:EndActivity()!");
	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) then
			FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
			self:GetBanner(GUIBanner.RED, player):ClearText();
			self:GetBanner(GUIBanner.YELLOW, player):ClearText();
		end
	end
end

-----------------------------------------------------------------------------------------
-- Update Activity
-----------------------------------------------------------------------------------------

function SiteScan:UpdateActivity()

	-- Scan the terrain from orbit, revealing all above-ground space for the teams
	if (self.ActivityState == Activity.RUNNING) then
		local scanMessage = "Scanning";
		local messageBlink = 500;

		-- Wait a sec first before RUNNING to scan, so player can get what's going on
		if self.CurrentScanStage == self.ScanStage.PRESCAN then
			scanMessage = "Preparing to scan site from orbit";
			for dotCount = 0, math.floor(self.ScanTimer[self.ScanTeam].ElapsedSimTimeMS / 500) do
				scanMessage = " " .. scanMessage .. ".";
			end
			messageBlink = 0;
--			self:SetObservationTarget(Vector(0, 0), player);
			if self.ScanTimer[self.ScanTeam]:IsPastSimMS(2000) then
				self.CurrentScanStage = self.ScanStage.SCANNING;
			end
		-- Do actual scanning process
		elseif self.CurrentScanStage == self.ScanStage.SCANNING then
			for team = Activity.TEAM_1, Activity.MAXTEAMCOUNT - 1 do
				if self:TeamActive(team) then
					-- Time to do a scan step?
					if self.ScanTimer[team]:IsPastRealMS(SceneMan:GetUnseenResolution(team).X) then
						-- Scan the column, find the end where the ray is blocked
						SceneMan:CastSeeRay(team, Vector(self.ScanPosX[team], 0), Vector(0, SceneMan.Scene.Height), self.ScanEndPos, 50, SceneMan:GetUnseenResolution(team).Y / 2);
						-- Adjust up a bit so one sees more of the sky than blackness
						self.ScanEndPos.Y = self.ScanEndPos.Y - (FrameMan.PlayerScreenHeight / 4);
						-- Also a bit more behind the scanning front so we see more of the terrain
						self.ScanEndPos.X = self.ScanEndPos.X - (FrameMan.PlayerScreenWidth / 4);
						if self.ScanEndPos.X < 0 then
							self.ScanEndPos.X = 0;
						end

						if (self.ScanPosX[team] < SceneMan.Scene.Width) then
							scanMessage = "Scanning";
							messageBlink = 500;
							-- Move on to the next column
							self.ScanPosX[team] = self.ScanPosX[team] + SceneMan:GetUnseenResolution(team).X;
							-- Set the proportionate amount from the team funds to represent payment for the scanning
							self:SetTeamFunds((self.StartFunds[team] * (1.0 - (self.ScanPosX[team] / SceneMan.Scene.Width))) * rte.StartingFundsScale, team);
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
			-- If done scanning ALL TEAMS, move on the the post pause phase
			if self.ScanPosX[self.ScanTeam] > SceneMan.Scene.Width then
				-- Finish off the scanning payment
				self:SetTeamFunds(0, self.ScanTeam);
				self.CurrentScanStage = self.ScanStage.POSTSCAN;
			end
		-- After scan, pause for a second before moving on to gameplay
		elseif self.CurrentScanStage == self.ScanStage.POSTSCAN then
			if self.ScanTimer[self.ScanTeam]:IsPastRealMS(2500) then
				for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
					if self:PlayerActive(player) and self:PlayerHuman(player) then
						FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
						self:GetBanner(GUIBanner.RED, player):ClearText();
						self:GetBanner(GUIBanner.YELLOW, player):ClearText();
					end
				end
				self.WinnerTeam = self.ScanTeam;
				ActivityMan:EndActivity();
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
				if (self.ActivityState == Activity.RUNNING) then
					FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
					FrameMan:SetScreenText(scanMessage, self:ScreenOfPlayer(player), messageBlink, 8000, true);
				end
			end
		end
	end

	-- Scanning is over and done, control how the messages are presented to the player
	if (self.ActivityState == Activity.OVER) then
		-- Manage post-scan messages on screen
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self:PlayerActive(player) and self:PlayerHuman(player) then
				self:GetBanner(GUIBanner.YELLOW, player):ClearText();
				self:GetBanner(GUIBanner.RED, player):ClearText();
			end
		end
	end
end