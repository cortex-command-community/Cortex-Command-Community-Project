---------------------------------------------------------
-- BRAIN INTEGRITY CHECK
function MetaFight:BrainCheck()

	-- Clear all objective markers, they get re-added each frame
	self:ClearObjectivePoints();
	-- Keep track of which teams we have set objective points for already, since multiple players can be on the same team
	local setTeam = {};
	for team = Activity.TEAM_1, Activity.MAXTEAMCOUNT - 1 do
		setTeam[team] = false;
	end

	-----------------------------------------------------------------
	-- Brain integrity check logic for every player, Human or AI
	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) then

			-- The current player's team
			local team = self:GetTeamOfPlayer(player);

--				if not self.StartTimer:IsPastSimMS(3000) then
--					FrameMan:SetScreenText("Destroy the enemy brain(s)!", self:ScreenOfPlayer(player), 333, 5000, true);
--				end

			-- Only worry about brain death if the player ever had one yet
			if self:PlayerHadBrain(player) then
				-- Check if any player's brain is now dead
				if not MovableMan:IsActor(self:GetPlayerBrain(player)) or not self:GetPlayerBrain(player):HasObjectInGroup("Brains") then
					self:SetPlayerBrain(nil, player);

					-- Try to find a new brain and assign it
					local newBrain = MovableMan:GetUnassignedBrain(team);
					if newBrain then
						self:SetPlayerBrain(newBrain, player);
						self:SwitchToActor(newBrain, player, team);
					-- All hope is lost for this player
					else
						self:ResetMessageTimer(player);
--						FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
--						FrameMan:SetScreenText("Your brain has been destroyed!", self:ScreenOfPlayer(player), 333, -1, false);
						-- Save the position of this player's brain's last known position as the last brain death position
						self.LastBrainDeathPos = self.LastBrainPos[player];

						-- Now see if all brains are dead of this player's team, and if so, check if there's only one team left with brains
						if not MovableMan:GetFirstBrainActor(team) then
							-- If only one team left with any brains, they are the winners!
							if self:OneOrNoneTeamsLeft() then
								-- Only do EndActivity if the winner outcome has changed
								local winnerChanged = self.WinnerTeam ~= self:WhichTeamLeft();
								self.WinnerTeam = self:WhichTeamLeft();
								if winnerChanged then
									ActivityMan:EndActivity();
								end
							end

							-- This whole loser team is done for, so swap its doors to the winner's team, and self-destruct all of its other actors.
							for actor in MovableMan.Actors do
								if actor.Team == team and IsADoor(actor) then
									actor.Team = self.WinnerTeam;
								end
							end
							MovableMan:KillAllTeamActors(team);
							-- Finally, deactive the player entirely -	no don't this is done in AutoResolveOffensive if needed
--							self:DeactivatePlayer(player);
						end

						-- Check if all human-controlled brains are goners, and if so, end the activity also
						if self:HumanBrainCount() == 0 and self.ActivityState ~= Activity.OVER then
							ActivityMan:EndActivity();
						end
					end
				-- We do have a brain
				else
					-- Save the last known position of this player's brain
					self.LastBrainPos[player] = self:GetPlayerBrain(player).Pos;
					-- Continually set the observation target to the brain during play, so that if/when it dies, the view flies to it in observation mode
					if self.ActivityState ~= Activity.OVER and self:GetViewState(player) ~= Activity.OBSERVE then
						self:SetObservationTarget(self:GetPlayerBrain(player).Pos, player);
					end

					if not setTeam[team] then
						-- Add objective points
						self:AddObjectivePoint("Protect!", self:GetPlayerBrain(player).AboveHUDPos, team, GameActivity.ARROWDOWN);
						for otherPlayer = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
							if otherPlayer ~= player and self:PlayerActive(otherPlayer) and MovableMan:IsActor(self:GetPlayerBrain(otherPlayer)) then
								local otherTeam = self:GetTeamOfPlayer(otherPlayer);
								if otherTeam ~= team then
									self:AddObjectivePoint("Destroy!", self:GetPlayerBrain(otherPlayer).AboveHUDPos, team, GameActivity.ARROWDOWN);
								else
									self:AddObjectivePoint("Protect!", self:GetPlayerBrain(otherPlayer).AboveHUDPos, team, GameActivity.ARROWDOWN);
								end
							end
						end
						setTeam[team] = true;
					end
				end
			-- See if we can now find a brain to designate
			else
				local foundBrain = MovableMan:GetUnassignedBrain(team);
				self:SetPlayerBrain(foundBrain, player);
				-- If this is a brain that has landed on the surface and gotten out of his ship, switch to him
				if foundBrain and foundBrain:IsInGroup("Brains") then
					self:SwitchToActor(foundBrain, player, team);
				end
			end

			-- If there's only one active team, it means they are clearing out wildlife in this mission
			if self.TeamCount == 1 then
				-- Check if there's any wildlife left to clear out, and the brain is standing on the surface (and not in a ship still)
				if not MovableMan:GetFirstTeamActor(Activity.NOTEAM, Activity.PLAYER_NONE) and MovableMan:IsActor(self:GetPlayerBrain(player)) and self:GetPlayerBrain(player):IsInGroup("Brains") then
					-- Only do EndActivity if the winner outcome has changed
					local winnerChanged = self.WinnerTeam ~= self:WhichTeamLeft();
					self.WinnerTeam = self:WhichTeamLeft();
					if winnerChanged then
						ActivityMan:EndActivity();
					end
				end
			end
		end
	end
end


---------------------------------------------------------
-- START
function MetaFight:StartActivity()

--[[ DEBUG
	dofile("Base.rte/Actors/AI/NativeHumanAI.lua");
	dofile("Base.rte/Actors/AI/HumanBehaviors.lua");
	dofile("Base.rte/Actors/AI/NativeCrabAI.lua");
	dofile("Base.rte/Actors/AI/CrabBehaviors.lua");
	dofile("Base.rte/Actors/AADrone/AADrone.lua");
	SceneMan:RevealUnseenBox(0,0,SceneMan.SceneWidth,SceneMan.SceneHeight,Activity.TEAM_1);
	--for t=1,4 do ActivityMan:GetActivity():ChangeTeamFunds(900, t-1) end
	for t=1,4 do print("Team "..t.." gold: "..ActivityMan:GetActivity():GetTeamFunds(t-1)) end
-- DEBUG]]

	-- DEBUG
	--SceneMan:RevealUnseenBox(0,0,SceneMan.SceneWidth,SceneMan.SceneHeight,Activity.TEAM_1)

	-- Not editing mode, but a pre-game start mode where players get to place their brains initially
	self.ActivityState = Activity.PREGAME;
	-- Whether there are literally NO brains left in the scene
	self.NoBrainsLeft = false;

	-- Open all doors so we can do pathfinding through them with the brain placement
	MovableMan:OpenAllDoors(true, Activity.NOTEAM);

	-- Make all leftover craft take off and set all guards placed in the build phase in sentry mode
	for actor in MovableMan.AddedActors do
		-- Set all craft to fly away into orbit
		if IsACRocket(actor) or IsACDropShip(actor) then
			actor.AIMode = Actor.AIMODE_RETURN;
		-- And all other actors to sentry mode
		else
			actor.AIMode = Actor.AIMODE_SENTRY;
		end
	end

	if SceneMan.Scene:HasArea(rte.MetabaseArea) then
		self.MetabaseArea = SceneMan.Scene:GetArea(rte.MetabaseArea);
	else
		self.MetabaseArea = nil;
	end

	-- Orbit Scene Scanning vars
	self.ScanStage = { PRESCAN = 0, SCANNING = 1, POSTSCAN = 2, DONESCAN = 3, SCANSTAGECOUNT = 4 };
--	self.CurrentScanStage = { [Activity.TEAM_1] = self.ScanStage.PRESCAN, [Activity.TEAM_2] = self.ScanStage.PRESCAN, [Activity.TEAM_3] = self.ScanStage.PRESCAN, [Activity.TEAM_4] = self.ScanStage.PRESCAN };
	-- Start by assming no teams have scanning scheduled
	self.CurrentScanStage = self.ScanStage.DONESCAN;
	self.ScanPosX = {};
	self.ScanTimer = {};
	self.StartFunds = {};
	self.ScanEndPos = Vector();

	-- Which are the invading teams
	self.InvadingTeam = {};
	-- Which Teams are managed tactically by the AI? Only teams with NO human players on them
	self.TeamAIActive = {};
	-- Team has given up and is evacuating their brain
	self.TeamEvacuating = {};

	-- A list of AI controlled team numbers
	local CPUTeams = {};
	-- Timers for controlling the AI modes of team members
	self.AIModeTimer = {};

	for team = Activity.TEAM_1, Activity.MAXTEAMCOUNT - 1 do
		self.ScanPosX[team] = -1;
		self.ScanTimer[team] = Timer();
		self.StartFunds[team] = 0;

		if self:TeamActive(team) then
			if SceneMan.Scene:IsScanScheduled(team) then
				-- Yes there is at least one active team that has scanning scheduled
				self.CurrentScanStage = self.ScanStage.PRESCAN;
			end
			-- Start out assuming all teams are all AI invaders, then disprove it
			self.InvadingTeam[team] = true;
			self.TeamAIActive[team] = true;
			self.TeamEvacuating[team] = false;
			-- Inspect all the players of each team
			for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
				if self:PlayerActive(player) and self:GetTeamOfPlayer(player) == team then
					-- Any team with a player that has a brain resident in the scene are DEFENDERS
					local residentBrain = SceneMan.Scene:GetResidentBrain(player);
					if residentBrain then
						ToActor(residentBrain).AIMode = Actor.AIMODE_SENTRY;
						self.InvadingTeam[team] = false;
					end
					-- Disable the tactical team management on any team that has ANY human players on it - the humans get full control
					if self:PlayerHuman(player) then
						self.TeamAIActive[team] = false;
					end
				end
			end

			if self.TeamAIActive[team] then
				self.AIModeTimer[team] = Timer();
				table.insert(CPUTeams, team);
			end
		else
			self.InvadingTeam[team] = false;
			self.TeamAIActive[team] = false;
			self.TeamEvacuating[team] = false;
		end
	end

	-- MetaFight-specific player parameters
	self.InvadingPlayer = {};
	self.Ready = {};
	self.InvadingPlayerCount = 0;
	-- At what funds level the AI starts thinking about going into brain evacuation mode
	self.EvacThreshold = {};
	-- The last known position of all players' active brains
	self.LastBrainPos = {};
	-- The position of the last brain that died
	self.LastBrainDeathPos = Vector();

	local defenderTeam;
	local defenderTeamNativeCostMultiplier = 1.0;

	-- Now init all players
	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		self.Ready[player] = false;
		self.EvacThreshold[player] = 100;
		self.LastBrainPos[player] = Vector();
		if self:PlayerActive(player) then
			-- Reset the timer that will measure the delay between ordering of reinforcements
			-- Determine whether this player is invading or already has a brain to defend here
			local residentBrain = SceneMan.Scene:GetResidentBrain(player);

			-- Human player; init things common to all humans
			if self:PlayerHuman(player) then
				-- Set the Buy menu to reflect the native tech of this metaplayer
				for metaPlayer = Activity.PLAYER_1, MetaMan.PlayerCount - 1 do
					if MetaMan:GetPlayer(metaPlayer).InGamePlayer == player then
						self:GetEditorGUI(player):SetNativeTechModule(MetaMan:GetPlayer(metaPlayer).NativeTechModule);
						self:GetEditorGUI(player):SetForeignCostMultiplier(MetaMan:GetPlayer(metaPlayer).ForeignCostMultiplier);
						self:GetBuyGUI(player):SetMetaPlayer(metaPlayer);
						-- Re-load all the loadouts now that the metaplayer has been set
						self:GetBuyGUI(player):LoadAllLoadoutsFromFile();
					end
				end

				-- Set a temporary LZ
				self:SetLandingZone(Vector(player*(SceneMan.SceneWidth-1)/4, 0), player);
			end

			-- Invading player
			if not residentBrain then
				self.InvadingPlayer[player] = true;
				self.InvadingPlayerCount = self.InvadingPlayerCount + 1;

				-- Human player; init as appropriate for invaders
				if self:PlayerHuman(player) then
					-- Set an initial landing team based on appropriate for his Tech's default brain Loadout
					self:SetOverridePurchaseList("Infantry Brain", player);

					-- Set the mode to LZ select so the player can choose where to land first
					self:SetViewState(Activity.LZSELECT, player);
					FrameMan:SetScreenText("Choose where to land your assault brain", self:ScreenOfPlayer(player), 250, 3500, false);
					self:ResetMessageTimer(player);
				end

			-- Defending player
			else
				self.InvadingPlayer[player] = false;
				defenderTeam = self:GetTeamOfPlayer(player);

				-- Store initial defender team funds to prevent from overflow caused by AI gold cheat
				self.DefenderTeamInitialFunds = self:GetTeamFunds(defenderTeam);

				-- Find out NativeCostMultiplier for defenders, it will be used when we'll have to sell excess actors
				for metaPlayer = Activity.PLAYER_1, MetaMan.PlayerCount - 1 do
					if MetaMan:GetPlayer(metaPlayer).InGamePlayer == player then
						defenderTeamNativeCostMultiplier = MetaMan:GetPlayer(metaPlayer).NativeCostMultiplier;
					end
				end

				-- Human player; init as appropriate for defenders
				if self:PlayerHuman(player) then
					-- Test if the brain is in a valid position
					if not self:GetEditorGUI(player):TestBrainResidence(false) then
						-- Force player to re-place his brain
						self:GetEditorGUI(player):Update();
						FrameMan:SetScreenText("PLACE YOUR BRAIN IN A VALID SPOT FIRST", self:ScreenOfPlayer(player), 250, 3500, false);
						self:ResetMessageTimer(player);
					else
						self.Ready[player] = true;
						-- Place this player's resident brain into the simulation and set it as the player's assigned brain
						SceneMan.Scene:PlaceResidentBrain(player, self);
						FrameMan:SetScreenText("READY to start - wait for others to finish...", self:ScreenOfPlayer(player), 333, -1, false);
						self:ResetMessageTimer(player);

						-- If brain's coords are -1,-1 then it was auto-placed by MetaGameGUI when we captured an empty location
						-- and now we need to relocate it. It does not really matter where, since the Scene should be still empty.
						if residentBrain.Pos.X == -1 and residentBrain.Pos.Y == -1 then
							-- Find some random spot to put our brain
							local pos;
							local success = false;
							-- Make few attempts to find a suitable spot
							for i = 1, 10 do
								local rangeWidth;
								local rangeStart;
								local rangeEnd;

								-- Try to divide the map into pieces and put every brain in separate piece if possible
								if i < 10 then
									rangeWidth = SceneMan.Scene.Width / Activity.MAXPLAYERCOUNT;
									rangeStart = SceneMan.Scene.Width / Activity.MAXPLAYERCOUNT * player;
									rangeEnd = rangeStart + rangeWidth;
								else
									-- Expand the range if we could not find the location
									rangeWidth = SceneMan.Scene.Width;
									rangeStart = 0;
									rangeEnd = rangeStart + rangeWidth;
								end

								--If scene is not wrapped then reduce range to avoid spawning on the edge of the scene
								if not SceneMan.Scene.WrapsX then
									rangeStart = rangeStart + rangeWidth * 0.25;
									rangeEnd = rangeEnd - rangeWidth * 0.25;
								end

								pos = Vector(math.random(rangeStart, rangeEnd), 0);

								success = true;

								-- Measure heights 5 times and verify that we can put brain there
								for j = -2, 2 do
									if SceneMan:FindAltitude(pos + Vector(j * 10, 0), 0, 19) < 25 then
										success = false;
									end
								end

								if success then
									break;
								end
							end

							residentBrain.Pos = SceneMan:MovePointToGround(pos , 20 , 3);
							-- Also order brain to dig itself
							residentBrain.AIMode = Actor.AIMODE_GOLDDIG;

							-- If brain already built some bunkers, but without any brain hideouts, simply move the brain somewhere inside the base
							if self.MetabaseArea and not self.MetabaseArea:IsInside(residentBrain.Pos) then
								local newLocation;
								-- If we can't even find a place inside the base, then at least move brain somewhere close to friendly unit
								-- so it does not look lost and forgotten
								local backupLocation;

								-- If some actor was deployed somewhere, we can assume that this location is safe to move our brain to
								for actor in MovableMan.AddedActors do
									if actor.Team == residentBrain.Team and IsAHuman(actor) then
										if self.MetabaseArea:IsInside(actor.Pos) then
											newLocation = actor.Pos;
										else
											if not actor:IsInGroup("Brains") then
												backupLocation = actor.Pos;
											end
										end
									end
								end

								-- Move brain and cancel gold dig order, is it's already inside the base
								if newLocation then
									residentBrain.AIMode = Actor.AIMODE_SENTRY;
									residentBrain.Pos = newLocation;
								elseif backupLocation then
									residentBrain.AIMode = Actor.AIMODE_SENTRY;
									residentBrain.Pos = backupLocation;
								end
							end
						end
					end

-- Use this in the update later
--					self:SetLandingZone(residentBrain.Pos, player);
					-- Set the observation target to the brain, so that if/when it dies, the view flies to it in observation mode
--					self:SetObservationTarget(residentBrain.Pos, player);

				-- AI defending player; init its brain which should already exist and signal ready
				else
					-- AI brain should be ready to rock
					self.Ready[player] = true;
					SceneMan.Scene:PlaceResidentBrain(player, self);

					-- If brain's coords are -1,-1 then it was auto-placed by MetaGameGUI when we captured an empty location
					-- and now we need to relocate it. It does not really matter where, since the Scene should be still empty.
					if residentBrain.Pos.X == -1 and residentBrain.Pos.Y == -1 then
						-- Find some random spot to put our brain
						local pos;
						local success = false;
						-- Make few attempts to find a suitable spot
						for i = 1, 20 do
							local rangeWidth;
							local rangeStart;
							local rangeEnd;

							-- Try to divide the map into pieces and put every brain in separate piece if possible
							if i < 10 then
								rangeWidth = SceneMan.Scene.Width / Activity.MAXPLAYERCOUNT;
								rangeStart = SceneMan.Scene.Width / Activity.MAXPLAYERCOUNT * player;
								rangeEnd = rangeStart + rangeWidth;
							else
								-- Expand the range if we could not find the location
								rangeWidth = SceneMan.Scene.Width;
								rangeStart = 0;
								rangeEnd = rangeStart + rangeWidth;
							end

							--If scene is not wrapped then reduce range to avoid spawning on the edge of the scene
							if not SceneMan.Scene.WrapsX then
								rangeStart = rangeStart + rangeWidth * 0.25;
								rangeEnd = rangeEnd - rangeWidth * 0.25;
							end

							pos = Vector(math.random(rangeStart, rangeEnd), 0);

							success = true;

							-- Measure heights 5 times and verify that we can put brain there
							for j = -2, 2 do
								if SceneMan:FindAltitude(pos + Vector(j * 10, 0), 0, 19) < 25 then
									success = false;
								end
							end

							if success then
								break;
							end
						end

						residentBrain.Pos = SceneMan:MovePointToGround(pos , 20 , 3);
						-- Also order brain to dig itself
						residentBrain.AIMode = Actor.AIMODE_GOLDDIG;

						-- If brain already built some bunkers, but without any brain hideouts, simply move the brain somewhere inside the base
						if self.MetabaseArea and not self.MetabaseArea:IsInside(residentBrain.Pos) then
							local newLocation;
							-- If we can't even find a place inside the base, then at least move brain somewhere close to friendly unit
							-- so it does not look lost and forgotten
							local backupLocation;

							-- If some actor was deployed somewhere, we can assume that this location is safe to move our brain to
							for actor in MovableMan.AddedActors do
								if actor.Team == residentBrain.Team and IsAHuman(actor) then
									if self.MetabaseArea:IsInside(actor.Pos) then
										newLocation = actor.Pos;
									else
										if not actor:IsInGroup("Brains") then
											backupLocation = actor.Pos;
										end
									end
								end
							end

							-- Move brain and cancel gold dig order, is it's already inside the base
							if newLocation then
								residentBrain.AIMode = Actor.AIMODE_SENTRY;
								residentBrain.Pos = newLocation;
							elseif backupLocation then
								residentBrain.AIMode = Actor.AIMODE_SENTRY;
								residentBrain.Pos = backupLocation;
							end
						end
					end

					-- Still no brain of this player? Last ditch effort to find one and assign it to this player
					if not self:GetPlayerBrain(player) then
						self:SetPlayerBrain(MovableMan:GetUnassignedBrain(self:GetTeamOfPlayer(player)), player);
					end
					-- Something went wrong.. recover
					if not self:GetPlayerBrain(player) then
-- TODO: spawn a new brain in an appropriate spot
					end
				end
			end
		end
	end

	-- Starting in pregame, so play some appropriate music
	if self.ActivityState == Activity.PREGAME then
		AudioMan:ClearMusicQueue();
		AudioMan:PlayMusic("Base.rte/Music/dBSoundworks/ccambient4.ogg", -1, -1);
	end

	-- Disable AI if we are editing pregame
	self:DisableAIs(self.ActivityState == Activity.PREGAME, Activity.NOTEAM);

	-- Move any brains resident in the Scene to the MovableMan
-- No, this is taken care of by GameActivity::UpdateEditing
--	SceneMan.Scene:PlaceResidentBrains(self);

	--self.StartTimer = Timer();

	if #CPUTeams > 0 then
		-- Store data about terrain and enemy actors in the LZ map, use it to pick safe landing zones
		self.AI = {};
		self.AI.LZmap = require("Activities/Utility/LandingZoneMap");
		self.AI.LZmap:Initialize(CPUTeams);
		self.AI.SpawnTimer = Timer();
		self.AI.SpawnTimer:SetSimTimeLimitMS(16000);
		self.AI.AttackTarget = {};
		self.AI.AttackPos = {};
		self.AI.Defender = {};
	end

	-- Clear data about actors controlled by external scripts. If scripts are active they'll grab their actors back
	-- but if not, actors will remain uncontrolled.
	for actor in MovableMan.Actors do
		actor:RemoveStringValue("ScriptControlled");
	end

	for actor in MovableMan.AddedActors do
		actor:RemoveStringValue("ScriptControlled");
	end
end

---------------------------------------------------------
-- PAUSE
function MetaFight:PauseActivity(pause)

end

---------------------------------------------------------
-- END
function MetaFight:EndActivity()

	-- If there's no brains left in the scene at all after game over, then re-do the outcome
	self.NoBrainsLeft = self:NoTeamLeft();

	-- Temp fix so music doesn't start playing if ending the Activity when changing resolution through the ingame settings.
	if not self:IsPaused() then
		-- This is now no-man's land
		if self.NoBrainsLeft then
			self.WinnerTeam = Activity.NOTEAM;
			SceneMan.Scene.TeamOwnership = Activity.NOTEAM;
			-- Should not clear blueprints because this wipes all placed loadouts info
			--SceneMan.Scene:ClearPlacedObjectSet(Scene.BLUEPRINT);
			-- Sad music
			AudioMan:ClearMusicQueue();
			AudioMan:PlayMusic("Base.rte/Music/dBSoundworks/udiedfinal.ogg", 2, -1.0);
			AudioMan:QueueSilence(10);
			AudioMan:QueueMusicStream("Base.rte/Music/dBSoundworks/ccambient4.ogg");
		-- Also play sad music if no humans are left
		elseif self:HumanBrainCount() == 0 then
			AudioMan:ClearMusicQueue();
			AudioMan:PlayMusic("Base.rte/Music/dBSoundworks/udiedfinal.ogg", 2, -1.0);
			AudioMan:QueueSilence(10);
			AudioMan:QueueMusicStream("Base.rte/Music/dBSoundworks/ccambient4.ogg");
		-- But if humans are left, then play happy music!
		else
			-- Win music!
			AudioMan:ClearMusicQueue();
			AudioMan:PlayMusic("Base.rte/Music/dBSoundworks/uwinfinal.ogg", 2, -1.0);
			AudioMan:QueueSilence(10);
			AudioMan:QueueMusicStream("Base.rte/Music/dBSoundworks/ccambient4.ogg");
		end
	end

	-- Display appropriate message for each player, winner or loser
	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) then
			-- Winners
			if self:GetTeamOfPlayer(player) == self.WinnerTeam then
				-- Owned the site/base at start
				if SceneMan.Scene.TeamOwnership == self:GetTeamOfPlayer(player) then
					FrameMan:SetScreenText("Your team has successfully defended this site!", self:ScreenOfPlayer(player), 0, -1, false);
				else
					FrameMan:SetScreenText("Your team has successfully taken over this site!", self:ScreenOfPlayer(player), 0, -1, false);
				end
			-- Losers
			else
				-- Owned the site/base at start
				if SceneMan.Scene.TeamOwnership == self:GetTeamOfPlayer(player) then
					FrameMan:SetScreenText("Your team's brains have been destroyed and therefore lost this site!", self:ScreenOfPlayer(player), 0, -1, false);
				else
					FrameMan:SetScreenText("Your attempt to take over this site has failed!", self:ScreenOfPlayer(player), 0, -1, false);
				end
			end
			-- Set the observation targets if everyone to the last brain's death
			if not self.LastBrainDeathPos:IsZero() then
				self:SetViewState(Activity.OBSERVE, player);
				self:SetObservationTarget(self.LastBrainDeathPos, player);
			end
			self:ResetMessageTimer(player);
		end
	end

	-- Give the victorious team ownership of the played scene
	if self.WinnerTeam ~= Activity.NOTEAM then
		-- If these are new owners, delete the previous team's blueprint building plans for this base
		if SceneMan.Scene.TeamOwnership ~= self.WinnerTeam then
			-- Should not clear blueprints because this wipes all placed loadouts info
			--SceneMan.Scene:ClearPlacedObjectSet(Scene.BLUEPRINT);
		end

		-- If there was only one active team to begin with, it means they are clearing out wildlife in this mission
		if self.TeamCount == 1 then
			-- Play less exctatic music
--			AudioMan:ClearMusicQueue();
--			AudioMan:PlayMusic("Base.rte/Music/dBSoundworks/ccambient4.ogg", -1, -1.0);
		else
			-- Win music!
--			AudioMan:ClearMusicQueue();
--			AudioMan:PlayMusic("Base.rte/Music/dBSoundworks/uwinfinal.ogg", 2, -1.0);
--			AudioMan:QueueSilence(10);
--			AudioMan:QueueMusicStream("Base.rte/Music/dBSoundworks/ccambient4.ogg");
		end

		-- Actually change ownership
		SceneMan.Scene.TeamOwnership = self.WinnerTeam;
	end

	-- Open all doors
	MovableMan:OpenAllDoors(true, Activity.NOTEAM);

end

---------------------------------------------------------
-- UPDATE
function MetaFight:UpdateActivity()
	--------------------------------------------------------
	-- Immediately do scanning for teams who have scheduled it
	local scanMessage = "Scanning";
	local messageBlink = 500;

	-- DEBUG
	--[[if SceneMan.Scene:HasArea(rte.MetabaseArea) then
		self.Metabase = SceneMan.Scene:GetArea(rte.MetabaseArea);
		for actor in MovableMan.Actors do
			if self.Metabase:IsInside(actor.Pos) then
				actor:FlashWhite(25);
			end
		end
	end--]]--

	-- Wait a sec first before starting to scan, so player can get what's going on
	if self.CurrentScanStage == self.ScanStage.PRESCAN then
		scanMessage = "Preparing to scan site from orbit";
		for dotCount = 0, math.floor(self.ScanTimer[Activity.TEAM_1].ElapsedSimTimeMS / 500) do
			scanMessage = " " .. scanMessage .. ".";
		end
		messageBlink = 0;
--			self:SetObservationTarget(Vector(0, 0), player);
		if self.ScanTimer[Activity.TEAM_1]:IsPastRealMS(2000) then
			self.CurrentScanStage = self.ScanStage.SCANNING;
		end
	-- Do actual scanning process for those teams that have scheduled it
	elseif self.CurrentScanStage == self.ScanStage.SCANNING then
		local doneScan = true;
		for team = Activity.TEAM_1, Activity.MAXTEAMCOUNT - 1 do
			if self:TeamActive(team) and SceneMan.Scene:IsScanScheduled(team) then
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
-- This has already been paid when the scan was scheduled in the metagame GUI
--							self:SetTeamFunds((self.StartFunds[team] * (1.0 - (self.ScanPosX[team] / SceneMan.Scene.Width))) * rte.StartingFundsScale, team);
						self.ScanTimer[team]:Reset();
						-- Set all screens of the teammates to the ray end pos so their screens follow the scanning
--[[
						for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
							if self:PlayerActive(player) and self:PlayerHuman(player) then
								if self:GetTeamOfPlayer(player) == team then
-- Not applicable in metafight; just scan while the player does placing etc
--										self:SetViewState(Activity.OBSERVE, player);
--										self:SetObservationTarget(self.ScanEndPos, player);
								end
							end
						end
--]]
					end
				end
				-- Check if anyone isn't done scanning yet
				if self.ScanPosX[team] < SceneMan.Scene.Width then
					doneScan = false;
				end
			end
		end

		-- If done scanning ALL TEAMS, move on the the post pause phase
		if doneScan then
			self.CurrentScanStage = self.ScanStage.POSTSCAN;
		end
	-- After scan, pause for a second before moving on to gameplay
	elseif self.CurrentScanStage == self.ScanStage.POSTSCAN then
		if self.ScanTimer[Activity.TEAM_1]:IsPastRealMS(2500) then
			for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
				if self:PlayerActive(player) and self:PlayerHuman(player) and SceneMan.Scene:IsScanScheduled(self:GetTeamOfPlayer(player)) then
					FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
				end
			end
			self.CurrentScanStage = self.ScanStage.DONESCAN;
			-- Reset all scanning scheduling since it has now been completed
			for team = Activity.TEAM_1, Activity.MAXTEAMCOUNT - 1 do
				if self:TeamActive(team) then
					SceneMan.Scene:SetScheduledScan(team, false);
				end
			end
		else
			scanMessage = "Complete!";
			messageBlink = 0;
		end
	end

	-- Display the scanning text on all players' screens
	if self.CurrentScanStage < self.ScanStage.DONESCAN then
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self:PlayerActive(player) and self:PlayerHuman(player) then
				-- The current player's team
				local team = self:GetTeamOfPlayer(player);
	--				if (self.ActivityState == Activity.RUNNING) then
				if (SceneMan.Scene:IsScanScheduled(team)) then
					FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
					FrameMan:SetScreenText(scanMessage, self:ScreenOfPlayer(player), messageBlink, 8000, false);
				end
			end
		end
	end

	-----------------------------------------------
	-- Players are placing brains and initial invading forces
	if self.ActivityState == Activity.PREGAME then
		-- See if players are getting done with placing etc. and ready to start
		local allReady = true;

		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self:PlayerActive(player) and self:PlayerHuman(player) then
				----------------------------------
				-- DEFENDERS
				-- Update the editor if we're placing a brain
				if not self.InvadingPlayer[player] and not self.Ready[player] then
					self:GetEditorGUI(player):Update();
					-- This defending player appears to be done placing his brain
					if self:GetEditorGUI(player).EditorMode == SceneEditorGUI.PICKINGOBJECT or self:GetEditorGUI(player).EditorMode == SceneEditorGUI.DONEEDITING then
						-- Check if we're really ready - if not, TestBrainRes will kick us back to installing brain mode
						self.Ready[player] = self:GetEditorGUI(player):TestBrainResidence(false);
					end
				end

				----------------------------------
				-- INVADERS
				if self.InvadingPlayer[player] and not self.Ready[player] then
					-- If not in LZSelect anymore, then this guy must be done placing his initial landing spot
					if self:GetViewState(player) ~= Activity.LZSELECT then
						-- Hm, no deliveries coming? then go back and try again, punk
						if self:GetDeliveryCount(self:GetTeamOfPlayer(player)) < 1 then
							-- Set the mode to LZ selct so the player can choose where to land first
							self:SetViewState(Activity.LZSELECT, player);
							FrameMan:SetScreenText("Choose where to land your assault brain", self:ScreenOfPlayer(player), 250, 3500, false);
							self:ResetMessageTimer(player);
						else
							-- Ok, done for real
							self.Ready[player] = true;
						end
					end
				end

				-- Keep showing ready message for those who are
				if self.Ready[player] then
					FrameMan:SetScreenText("READY to start - wait for others to finish...", self:ScreenOfPlayer(player), 333, -1, false);
				else
					allReady = false;
				end
			end
		end

		-- YES, we are allegedly all ready to stop editing and start the game!
		if allReady then
			for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
				if self:PlayerActive(player) and self:PlayerHuman(player) then
					-- Place all defending human brains into the simulation
					if not self.InvadingPlayer[player] then
						-- If not already done, place this player's resident brain into the simulation and set it as the player's assigned brain
						SceneMan.Scene:PlaceResidentBrain(player, self);
						-- Still no brain of this player? Last ditch effort to find one and assign it to this player
						if not self:GetPlayerBrain(player) then
							self:SetPlayerBrain(MovableMan:GetUnassignedBrain(self:GetTeamOfPlayer(player)), player);
						end
						-- Something went wrong.. we're not done placing brains after all?
						if not self:GetPlayerBrain(player) then
							allReady = false;
							-- Get the brains back into residency so the players who are OK are still so
							SceneMan.Scene:RetrieveResidentBrains(self);
							break;
						end

						-- Set the brain to be the selected actor at start
						self:SwitchToActor(self:GetPlayerBrain(player), player, self:GetTeamOfPlayer(player));
						self:SetLandingZone(self:GetPlayerBrain(player).Pos, player);
						-- Update the observation target to the brain, so that if/when it dies, the view flies to it in observation mode
						self:SetObservationTarget(self:GetPlayerBrain(player).Pos, player);
						-- Clear the messages before starting the game
						self:ResetMessageTimer(player);
						FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
						-- Reset the screen occlusion if any players are still in menus
						CameraMan:SetScreenOcclusion(Vector(), self:ScreenOfPlayer(player));
					end
				end
			end
		end

		-- Still good to go? then START!
		if allReady then
			-- START the game
			self.ActivityState = Activity.RUNNING;
			-- Close all doors after placing brains so our fortresses are secure
			MovableMan:OpenAllDoors(false, Activity.NOTEAM);
			-- Activate the AIs
			self:DisableAIs(false, Activity.NOTEAM);
			self:InitAIs()
			-- Reset the mouse value and pathfinding so it'll know about the newly placed stuff
			UInputMan:SetMouseValueMagnitude(0, -1);
			SceneMan.Scene:ResetPathFinding();
			-- Start the in-game music track
			AudioMan:ClearMusicQueue();
			AudioMan:PlayMusic("Base.rte/Music/dBSoundworks/cc2g.ogg", 0, -1.0);
			AudioMan:QueueSilence(30);
			AudioMan:QueueMusicStream("Base.rte/Music/Watts/Last Man.ogg");
			AudioMan:QueueSilence(30);
			AudioMan:QueueMusicStream("Base.rte/Music/dBSoundworks/cc2g.ogg");

			-- Find LZs for the AI teams
			self:DesignateLZs();
			self.AI.SpawnTimer:Reset();
			self.AI.SpawnTimer:SetSimTimeLimitMS(20000 - 10000 * (self.Difficulty/GameActivity.MAXDIFFICULTY));

			-- Spawn escort when attacking bunkers
			if self.hasDefender then
				for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
					if self:PlayerActive(player) and self.InvadingPlayer[player] then
						local lzX = math.floor(self:GetLandingZone(player).X);
						self:OrderEscortLoadout(self:WrapPos(lzX-250), player, self:GetTeamOfPlayer(player));
						self:OrderEscortLoadout(self:WrapPos(lzX+250), player, self:GetTeamOfPlayer(player));
					end
				end
			end
		end
	end

	-------------------------------------
	-- Game is RUNNING
	if self.ActivityState == Activity.RUNNING then
		-------------------------------------
		-- Tactical AI team management
		for team = Activity.TEAM_1, Activity.MAXTEAMCOUNT - 1 do
			-- Only allow three orders per second, per AI team
			if self:TeamActive(team) and self.TeamAIActive[team] and self.AIModeTimer[team]:IsPastRealMS(330) then
				self.AIModeTimer[team]:Reset();

				-- Check if this team have selected a target to attack
				local target;
				if MovableMan:IsActor(self.AI.AttackTarget[team]) then
					target = self.AI.AttackTarget[team];
				end

				-- INVADERS
				if self.InvadingTeam[team] then
					-- Give all existing team actors appropriate AI orders
					for actor in MovableMan.Actors do
						if actor.Team == team and not actor:StringValueExists("ScriptControlled") then
							-- The brain should try to dig itself into the ground to fortify itself against counterattack
							if actor:IsInGroup("Brains") then
								actor.AIMode = Actor.AIMODE_GOLDDIG;
								break;
							-- Actors start in sentry mode, send them towards the enemy target
							elseif actor.AIMode == Actor.AIMODE_SENTRY then
								if not actor:IsInGroup("Anti-Air") then
									if target then
										actor.AIMode = Actor.AIMODE_GOTO;
										actor:ClearAIWaypoints();
										actor:AddAIMOWaypoint(target);
									else
										actor.AIMode = Actor.AIMODE_BRAINHUNT;
									end

									break;
								end
							-- This actor have been ordered to attack a target, check that the target still exist
							elseif actor.AIMode == Actor.AIMODE_GOTO then
								if not actor.MOMoveTarget or not MovableMan:IsActor(actor.MOMoveTarget) then
									if target then
										actor:ClearAIWaypoints();
										actor:AddAIMOWaypoint(target);
									else
										actor.AIMode = Actor.AIMODE_BRAINHUNT;
									end

									break;
								end
							end
						end
					end
				-- DEFENDERS
				else
					-- Cheat and give the defending team 300 gold/minute at normal difficulty
					-- but do not allow to accumulate AI more gold it has before the fight, as it looks weird
					-- when after a fierce battle AI gets a ton of gold.
					if self:GetTeamFunds(team) < self.DefenderTeamInitialFunds * 0.5 then
						self:ChangeTeamFunds(self.Difficulty*TimerMan.DeltaTimeSecs*2, team);
					end

					-- Give all existing team actors appropriate AI orders
					for actor in MovableMan.Actors do
						if actor.Team == team	 and not actor:StringValueExists("ScriptControlled") then
							if actor.AIMode == Actor.AIMODE_SENTRY then
								-- The brain should stay put, presumably in a safe spot as dictated by the base plan
								-- Not a brain, so can this actor go hunt the enemy brain?
								-- If so, check if it was ordered during the fight as opposed to placed beforehand as defensive sentry
								if actor.PlacedByPlayer == Activity.PLAYER_NONE and not actor:IsInGroup("Anti-Air") and not actor:IsInGroup("Brains") then
									actor.AIMode = Actor.AIMODE_BRAINHUNT;
									break;
								end
							-- This actor have been ordered to attack a target, check that the target still exist
							elseif actor.AIMode == Actor.AIMODE_GOTO then
								if not MovableMan:IsActor(actor.MOMoveTarget) then
									if target then
										actor:ClearAIWaypoints();
										actor:AddAIMOWaypoint(target);
									else
										actor.AIMode = Actor.AIMODE_BRAINHUNT;
									end

									break;
								end
							end
						end
					end
				end
			end
		end

		-----------------------------------------------------------------
		-- Brain integrity check logic for every player, Human or AI
		MetaFight:BrainCheck();

		-----------------------------------------------------------
		-- AI players on AI-controlled teams order in reinforcements while they can afford them
		if self.AI then
			self.AI.LZmap:Update();

			if self.AI.SpawnTimer:IsPastSimTimeLimit() then
				local activeAITeams = 0;

				if not self.AI.spawnForPlayer then
					self.AI.SpawnTimer:Reset();

					-- find the first AI team
					for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
						local team = self:GetTeamOfPlayer(player);
						if self.TeamAIActive[team] and self:PlayerActive(player) and not self:PlayerHuman(player) and self:GetPlayerBrain(player) then
							if not self.AI.spawnForPlayer then
								self.AI.spawnForPlayer = player;
							end
							activeAITeams = activeAITeams + 1;
						end
					end
				end

				if self.AI.spawnForPlayer then
					local team = self:GetTeamOfPlayer(self.AI.spawnForPlayer);

					if self:SearchLZ(self.AI.spawnForPlayer) then	-- returns true when the search is complete. may or may not spawn a unit
						self.AI.SpawnTimer:Reset();

						-- find the next AI team (if any)
						local oldPlayer = self.AI.spawnForPlayer;
						self.AI.spawnForPlayer = nil;
						for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
							if player > oldPlayer then
								if self:PlayerActive(player) and not self:PlayerHuman(player) and self:GetPlayerBrain(player) and self.TeamAIActive[self:GetTeamOfPlayer(player)] then
									self.AI.spawnForPlayer = player;
									break;
								end
							end
						end

						if self.AI.spawnForPlayer then
							self.AI.SpawnTimer:SetSimTimeLimitMS(1000);
						else	-- all AI teams have spawned, wait a little longer before we try to spawn troops again
							self.AI.SpawnTimer:SetSimTimeLimitMS(6000);
						end
					end
				else
					self.AI.SpawnTimer:Reset();
				end
			end
		end
	--else
		--self.StartTimer:Reset();
	end

	-----------------------------------------------
	-- Game is OVER
	if self.ActivityState == Activity.OVER then
		-- Continue to make sure any remaining brains which were declared victorious are still okay.. and if not, change the outcome
		if not self.NoBrainsLeft then
			-- Keep checking the health of remaining brains
			MetaFight:BrainCheck();
		end
	elseif self.AI then
		self.AI.LZmap:Update();	-- Update info about landing zones and actors
	end
end

-----------------------------------------------
-- Make sure that all AI players have a LZ
function MetaFight:DesignateLZs()
	local OccupiedLZs = {};

	-- First, get the player LZs
	local hasResidentBrain = false;
	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerHuman(player) then
			-- The players have placed the LZs by now, insert them in to the occupied list
			local PlayerLZ = self:GetLandingZone(player);
			table.insert(OccupiedLZs, math.floor(PlayerLZ.X));
		end

		if SceneMan.Scene:GetResidentBrain(player) then
			hasResidentBrain = true;
		end
	end

	-- Now search for AI LZs
	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and not self:PlayerHuman(player) and self.InvadingPlayer[player] then
			local team = self:GetTeamOfPlayer(player);
			if self.TeamAIActive[team] then
				-- Find a good spot to land
				local lzX = self.AI.LZmap:FindStartLZ(team, OccupiedLZs) or math.random(0, SceneMan.SceneWidth-1); -- was FindSafeLZ
				-- Mark this LZ as occupied
				table.insert(OccupiedLZs, lzX);
				-- Set the LZ so the delivery will pop up there
				self:SetLandingZone(Vector(lzX, 0), player);
				-- Set an initial landing team based on appropriate for his Tech's default brain Loadout
				self:SetOverridePurchaseList("Infantry Brain", player);
				-- Create the actual delivery so it'll come down soon
				self:CreateDelivery(player);
			end
		end
	end
end

-----------------------------------------------
-- Use the LZmap to search for a good LZ to drop troops
function MetaFight:SearchLZ(player)
	local team = self:GetTeamOfPlayer(player);
	if self:GetTeamFunds(team) <= self.EvacThreshold[player] then
		return true;
	end

	-- Spawn LZ defender
	if self.InvadingPlayer[player] and (not self.AI.Defender[team] or not MovableMan:IsActor(self.AI.Defender[team]) or self.AI.Defender[team]:IsDead()) then
		local BrainPos = self:GetPlayerBrainPos(player);
		if BrainPos then
			-- Mark all LZs to the left of our brain as occupied
			local OccupiedLZs = {};
			for x = 0, BrainPos.X-400, 120 do
				table.insert(OccupiedLZs, x);
			end

			-- Mark all LZs to the right of our brain as occupied
			for x = BrainPos.X+400, SceneMan.SceneWidth-1, 120 do
				table.insert(OccupiedLZs, x);
			end

			-- Find a decent LZ close to our brain
			local lzX = self.AI.LZmap:FindSafeLZ(team, OccupiedLZs);
			if lzX then
				local MetaPlayer = MetaMan:GetMetaPlayerOfInGamePlayer(player);
				if MetaPlayer ~= 0 then
					local Craft = RandomACDropShip("Craft", MetaPlayer.NativeTechModule);
					if Craft then
						local Passenger = RandomAHuman("Actors - Light", MetaPlayer.NativeTechModule);
						if Passenger.ModuleID ~= MetaPlayer.NativeTechModule then
							Passenger = RandomAHuman("Actors", MetaPlayer.NativeTechModule);
						end
						if Passenger then
							Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Light", MetaPlayer.NativeTechModule));
							Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", MetaPlayer.NativeTechModule));
							Passenger:AddAISceneWaypoint(SceneMan:MovePointToGround(Vector(lzX, 0), Passenger.Height*0.25, 9));
							Passenger.AIMode = Actor.AIMODE_GOTO;
							Passenger.Team = team;
							self.AI.Defender[team] = Passenger;
							Craft:AddInventoryItem(Passenger);
						end

						-- Set team, location, deduct cost and spawn
						Craft.Team = team;
						Craft.Pos = Vector(lzX, -20);
						self:ChangeTeamFunds(-Craft:GetTotalValue(MetaPlayer.NativeTechModule, MetaPlayer.ForeignCostMultiplier, MetaPlayer.NativeCostMultiplier), team);
						MovableMan:AddActor(Craft);

						return true;
					end
				end
			end
		end
	end

	if self.AI.AttackPos[team] then	-- Search for a LZ from where to attack the target
		local easyPathLZx, easyPathLZobst, closeLZx, closeLZobst = self.AI.LZmap:FindLZ(team, self.AI.AttackPos[team]);
		if easyPathLZx then	-- Search done

			-- pick either the easy or the closeset LZ
			local lzX, obstacleHeight;
			if closeLZobst < 25 and easyPathLZobst < 25 then
				if math.random() < 0.5 then
					lzX = closeLZx;
					obstacleHeight = closeLZobst;
				else
					lzX = easyPathLZx;
					obstacleHeight = easyPathLZobst;
				end
			elseif closeLZobst > 100 then
				lzX = easyPathLZx;
				obstacleHeight = easyPathLZobst;
			else
				if math.random() < 0.4 then
					lzX = closeLZx;
					obstacleHeight = closeLZobst;
				else
					lzX = easyPathLZx;
					obstacleHeight = easyPathLZobst;
				end
			end

			if obstacleHeight > 200 and math.random() < 0.25 then
				-- This target is very difficult to reach: cancel this attack and search for another target again soon
				self.AI.AttackTarget[team] = nil;
				self.AI.AttackPos[team] = nil;
			else
				-- Land a mix of default and random loadouts and then set all units in sentry-mode (or in goto-mode with a dead MOMoveTarget) to move towards the AttackTarget

				-- Set an appropriate Loadout from this player's Tech
				local orderOK = false;
				if obstacleHeight < 30 then
					if math.random() > self:GetCrabToHumanSpawnRatio(MetaMan:GetMetaPlayerOfInGamePlayer(player).NativeTechModule) then
						if math.random() > 0.2 then
							if math.random() < self.Difficulty/100 then
								orderOK = self:OrderHeavyLoadout(player, team);
							else
								orderOK = self:OrderMediumLoadout(player, team);
							end
						else
							local cost = self:SetOverridePurchaseList("Infantry Heavy", player);
							if cost > 0 then
								orderOK = true;
							end
						end
					else
						local cost = self:SetOverridePurchaseList("Mecha", player);
						if cost > 0 then
							orderOK = true;
						end
					end
				elseif obstacleHeight < 80 then
					if math.random() > 0.2 then
						orderOK = self:OrderMediumLoadout(player, team);
					else
						local cost = self:SetOverridePurchaseList("Infantry CQB", player);
						if cost > 0 then
							orderOK = true;
						end
					end
				elseif obstacleHeight < 200 then
					if math.random() > 0.2 then
						orderOK = self:OrderLightLoadout(player, team);
					else
						local cost = self:SetOverridePurchaseList("Infantry Light", player);
						if cost > 0 then
							orderOK = true;
						end
					end
				else
					orderOK = self:OrderScoutLoadout(player, team);

					-- This target is very difficult to reach: change target for the next attack
					self.AI.AttackTarget[team] = nil;
					self.AI.AttackPos[team] = nil;
				end

				if orderOK then
					-- Set the LZ so the delivery will pop up there
					self:SetLandingZone(SceneMan:MovePointToGround(Vector(lzX, -1), 10, 3), player);

					-- Create the actual delivery so it'll come down soon
					if self.AI.AttackTarget[team] and MovableMan:IsActor(self.AI.AttackTarget[team]) then
						self:CreateDelivery(player, Actor.AIMODE_GOTO, self.AI.AttackTarget[team]);
					else
						self:CreateDelivery(player, Actor.AIMODE_BRAINHUNT);
					end

					if not MovableMan:IsActor(self.AI.AttackTarget[team]) or math.random() < 0.4 then
						-- Change target for the next attack
						self.AI.AttackPos[team] = nil;
					else
						self.AI.AttackPos[team] = Vector(self.AI.AttackTarget[team].Pos.X, self.AI.AttackTarget[team].Pos.Y);
					end
				else
					self:ClearOverridePurchase(player);
				end
			end

			return true;
		end
	else	-- Select an enemy actor as a target for the next attack
		local SafePos = self:GetPlayerBrainPos(player);
		if SafePos then
			local TargetActors = {};
			for actor in MovableMan.Actors do
				if actor.Team ~= team and ((actor.ClassName == "AHuman" and actor.PresetName ~= "Find Path") or actor.ClassName == "ACrab" or actor.ClassName == "Actor") and not actor:IsDead() then
					local distance = SceneMan:ShortestDistance(SafePos, actor.Pos, false).Largest;
					if actor:HasObjectInGroup("Brains") then
						distance = distance * 0.7; -- Prioritize brains
					end

					table.insert(TargetActors, {Act=actor, score=distance});
				end
			end

			self.AI.AttackTarget[team] = self:SelectTarget(TargetActors); -- Select the target based on distance from our brain
			if self.AI.AttackTarget[team] then
				self.AI.AttackPos[team] = Vector(self.AI.AttackTarget[team].Pos.X, self.AI.AttackTarget[team].Pos.Y);
			else
				-- No target found
				return true;
			end
		else
			-- No brain?
			return true;
		end
	end
end

-----------------------------------------------
-- Return the pos of this player's brain, if any
function MetaFight:GetPlayerBrainPos(player)
	local Brain = self:GetPlayerBrain(player);
	if Brain and MovableMan:IsActor(Brain) then
		return Brain.Pos;
	end
end

-----------------------------------------------
-- Pick an actor semi-randomly, with a larger probability for actors with a lower score
function MetaFight:SelectTarget(TargetActors)
	if #TargetActors > 1 then
		table.sort(TargetActors, function(A, B) return A.score < B.score end); -- Actors closer to the surface first

		local temperature = 6; -- a higher temperature means less random selection
		local sum = 0;
		local worstScore = TargetActors[#TargetActors].score;

		-- normalize the score
		for i, Data in pairs(TargetActors) do
			TargetActors[i].chance = 1 - Data.score / worstScore;
			sum = sum + math.exp(temperature*TargetActors[i].chance);
		end

		-- use Softmax to pick one of the n best LZs
		if sum > 0 then
			local pick = math.random() * sum;
			sum = 0;
			for _, Data in pairs(TargetActors) do
				sum = sum + math.exp(temperature*Data.chance);
				if sum >= pick then
					return Data.Act;
				end
			end
		end

		return TargetActors[1].Act;
	elseif #TargetActors == 1 then
		return TargetActors[1].Act;
	end
end

-----------------------------------------------
-- Pick a craft to deliver with
function MetaFight:PickCraft(MetaPlayer)
	local Craft;

	-- Find a drop ship with MaxPassengers > 0
	if math.random() < 0.4 then
		for i = 1, 3 do
			Craft = RandomACDropShip("Craft", MetaPlayer.NativeTechModule);
			if Craft.MaxPassengers == 0 then
				Craft = nil;
			else
				break;
			end
		end

		-- not found, try a rocket
		if not Craft then
			for i = 1, 3 do
				Craft = RandomACRocket("Craft", MetaPlayer.NativeTechModule);
				if Craft.MaxPassengers == 0 then
					Craft = nil;
				else
					break;
				end
			end
		end
	else -- Find a rocket with MaxPassengers > 0
		for i = 1, 3 do
			Craft = RandomACRocket("Craft", MetaPlayer.NativeTechModule);
			if Craft.MaxPassengers == 0 then
				Craft = nil;
			else
				break;
			end
		end

		-- not found, try a drop ship
		if not Craft then
			for i = 1, 3 do
				Craft = RandomACDropShip("Craft", MetaPlayer.NativeTechModule);
				if Craft.MaxPassengers == 0 then
					Craft = nil;
				else
					break;
				end
			end
		end
	end

	-- Use base crafts as a fall-back
	if not Craft then
		if math.random() < 0.5 then
			Craft = RandomACDropShip("Craft", MetaPlayer.NativeTechModule);
		else
			Craft = RandomACRocket("Craft", MetaPlayer.NativeTechModule);
		end
	end

	return Craft;
end

-----------------------------------------------
-- Functions for creating drop-craft with random actors
function MetaFight:OrderHeavyLoadout(player, team)
	local MetaPlayer = MetaMan:GetMetaPlayerOfInGamePlayer(player);

	-- Pick a craft to deliver with
	local Craft = MetaFight:PickCraft(MetaPlayer);
	local passengerLimit = Craft.MaxPassengers;

	-- Limit passengers based on difficulty
	if passengerLimit > 1 then
		if self.Difficulty < 90 then
			passengerLimit = SelectRand(1, passengerLimit);
		end

		if passengerLimit > 1 and self.Difficulty < 50 then
			passengerLimit = passengerLimit - 1;
		end
	end

	if Craft then
		self:AddOverridePurchase(Craft, player);

		-- The max allowed weight of this craft plus cargo
		local craftMaxMass = Craft.MaxInventoryMass;
		if craftMaxMass < 0 then
			craftMaxMass = math.huge;
		elseif craftMaxMass < 1 then
			if math.random() < 0.5 then
				Craft = RandomACDropShip("Craft", MetaPlayer.NativeTechModule);
			else
				Craft = RandomACRocket("Craft", MetaPlayer.NativeTechModule);
			end
			craftMaxMass = Craft.MaxInventoryMass;
		end

		local totalMass = 0;
		local diggers = 0;
		for actorsInCargo = 1, 10 do
			if math.random() < 0.8 then
				local d, m = self:PurchaseHeavyInfantry(player, MetaPlayer.NativeTechModule);
				diggers = diggers + d;
				totalMass = totalMass + m;
			else
				local d, m = self:PurchaseMediumInfantry(player, MetaPlayer.NativeTechModule);
				diggers = diggers + d;
				totalMass = totalMass + m;
			end

			-- Stop adding actors when the cargo limit is reached
			if actorsInCargo >= passengerLimit or totalMass > craftMaxMass then
				break;
			end
		end

		if diggers < 1 then
			self:AddOverridePurchase(CreateHDFirearm("Light Digger", "Base.rte"), player);
		end

		return true;
	end
end

function MetaFight:OrderMediumLoadout(player, team)
	local MetaPlayer = MetaMan:GetMetaPlayerOfInGamePlayer(player);

	-- Pick a craft to deliver with
	local Craft = MetaFight:PickCraft(MetaPlayer);
	local passengerLimit = Craft.MaxPassengers;

	-- Limit passengers based on difficulty
	if passengerLimit > 1 then
		if self.Difficulty < 80 then
			passengerLimit = SelectRand(1, passengerLimit);
		end

		if passengerLimit > 1 and self.Difficulty < 40 then
			passengerLimit = passengerLimit - 1;
		end
	end

	if Craft then
		self:AddOverridePurchase(Craft, player);

		-- The max allowed weight of this craft plus cargo
		local craftMaxMass = Craft.MaxInventoryMass;
		if craftMaxMass < 0 then
			craftMaxMass = math.huge;
		elseif craftMaxMass < 1 then
			if math.random() < 0.5 then
				Craft = RandomACDropShip("Craft", MetaPlayer.NativeTechModule);
			else
				Craft = RandomACRocket("Craft", MetaPlayer.NativeTechModule);
			end
			craftMaxMass = Craft.MaxInventoryMass;
		end

		local totalMass = 0;
		local diggers = 0;
		for actorsInCargo = 1, 10 do
			if math.random() < 0.8 then
				local d, m = self:PurchaseLightInfantry(player, MetaPlayer.NativeTechModule);
				diggers = diggers + d;
				totalMass = totalMass + m;
			else
				local d, m = self:PurchaseMediumInfantry(player, MetaPlayer.NativeTechModule);
				diggers = diggers + d;
				totalMass = totalMass + m;
			end

			-- Stop adding actors when the cargo limit is reached
			if actorsInCargo >= passengerLimit or totalMass > craftMaxMass then
				break;
			end
		end

		if diggers < 1 then
			self:AddOverridePurchase(CreateHDFirearm("Light Digger", "Base.rte"), player);
		end

		return true;
	end
end

function MetaFight:OrderLightLoadout(player, team)
	local MetaPlayer = MetaMan:GetMetaPlayerOfInGamePlayer(player);

	-- Pick a craft to deliver with
	local Craft = MetaFight:PickCraft(MetaPlayer);
	local passengerLimit = Craft.MaxPassengers;

	-- Limit passengers based on difficulty
	if passengerLimit > 1 then
		if self.Difficulty < 70 then
			passengerLimit = SelectRand(1, passengerLimit);
		end

		if passengerLimit > 1 and self.Difficulty < 30 then
			passengerLimit = passengerLimit - 1;
		end
	end

	if Craft then
		self:AddOverridePurchase(Craft, player);

		-- The max allowed weight of this craft plus cargo
		local craftMaxMass = Craft.MaxInventoryMass;
		if craftMaxMass < 0 then
			craftMaxMass = math.huge;
		elseif craftMaxMass < 1 then
			if math.random() < 0.5 then
				Craft = RandomACDropShip("Craft", MetaPlayer.NativeTechModule);
			else
				Craft = RandomACRocket("Craft", MetaPlayer.NativeTechModule);
			end
			craftMaxMass = Craft.MaxInventoryMass;
		end

		local totalMass = 0;
		local diggers = 0;
		for actorsInCargo = 1, 10 do
			local d, m = self:PurchaseLightInfantry(player, MetaPlayer.NativeTechModule);
			diggers = diggers + d;
			totalMass = totalMass + m;

			-- Stop adding actors when the cargo limit is reached
			if actorsInCargo >= passengerLimit or totalMass > craftMaxMass then
				break;
			end
		end

		if diggers < 1 then
			self:AddOverridePurchase(CreateHDFirearm("Light Digger", "Base.rte"), player);
		end

		return true;
	end
end

function MetaFight:OrderScoutLoadout(player, team)
	local MetaPlayer = MetaMan:GetMetaPlayerOfInGamePlayer(player);

	-- Pick a craft to deliver with
	local Craft = MetaFight:PickCraft(MetaPlayer);
	local passengerLimit = Craft.MaxPassengers;

	-- Limit passengers based on difficulty
	if passengerLimit > 1 then
		if self.Difficulty < 60 then
			passengerLimit = SelectRand(1, passengerLimit);
		end

		if passengerLimit > 1 and self.Difficulty < 20 then
			passengerLimit = passengerLimit - 1;
		end
	end

	if Craft then
		self:AddOverridePurchase(Craft, player);

		-- The max allowed weight of this craft plus cargo
		local craftMaxMass = Craft.MaxInventoryMass;
		if craftMaxMass < 0 then
			craftMaxMass = math.huge;
		elseif craftMaxMass < 1 then
			if math.random() < 0.5 then
				Craft = RandomACDropShip("Craft", MetaPlayer.NativeTechModule);
			else
				Craft = RandomACRocket("Craft", MetaPlayer.NativeTechModule);
			end
			craftMaxMass = Craft.MaxInventoryMass;
		end

		local totalMass = 0;
		local diggers = 0;
		for actorsInCargo = 1, 10 do
			if math.random() < 0.8 then
				local d, m = self:PurchaseScoutInfantry(player, MetaPlayer.NativeTechModule);
				diggers = diggers + d;
				totalMass = totalMass + m;
			else
				local d, m = self:PurchaseLightInfantry(player, MetaPlayer.NativeTechModule);
				diggers = diggers + d;
				totalMass = totalMass + m;
			end

			-- Stop adding actors when the cargo limit is reached
			if actorsInCargo >= passengerLimit or totalMass > craftMaxMass then
				break;
			end
		end

		if diggers < 1 then
			self:AddOverridePurchase(CreateHDFirearm("Light Digger", "Base.rte"), player);
		end

		return true;
	end
end

function MetaFight:OrderEscortLoadout(xPosLZ, player, team)
	local MetaPlayer = MetaMan:GetMetaPlayerOfInGamePlayer(player);

	-- Pick a craft to deliver with
	local Craft = RandomACRocket("Craft", MetaPlayer.NativeTechModule);
	if Craft then
		if self:GetCrabToHumanSpawnRatio(MetaPlayer.NativeTechModule) < 1 then
			local Passenger = self:CreateLightInfantry(Actor.AIMODE_SENTRY, MetaPlayer.NativeTechModule);
			if Passenger then
				Passenger.Team = team;
				Craft:AddInventoryItem(Passenger);
			end
			local Passenger = self:CreateMediumInfantry(Actor.AIMODE_SENTRY, MetaPlayer.NativeTechModule);
			if Passenger then
				Passenger.Team = team;
				Craft:AddInventoryItem(Passenger);
			end
		else
			local Passenger = RandomACrab("Actors - Mecha", MetaPlayer.NativeTechModule);
			if Passenger then
				Passenger.Team = team;
				Craft:AddInventoryItem(Passenger);
			end
		end

		-- Subtract the total value of the craft+cargo from the team's funds
		self:ChangeTeamFunds(-Craft:GetTotalValue(MetaPlayer.NativeTechModule, MetaPlayer.ForeignCostMultiplier, MetaPlayer.NativeCostMultiplier) * 0.33, team);

		Craft.Team = team;
		Craft.Pos = Vector(xPosLZ, -30); -- Set the spawn point of the craft
		MovableMan:AddActor(Craft); -- Spawn the Craft onto the scene
	end
end


-----------------------------------------------
-- Functions for purchasing random actors
function MetaFight:PurchaseHeavyInfantry(player, techID)
	local mass = 0;
	local digger = 0;
	local Cargo = RandomAHuman("Actors - Heavy", techID);
	if Cargo.ModuleID ~= techID then
		Cargo = RandomAHuman("Actors", techID);
	end

	if Cargo then
		self:AddOverridePurchase(Cargo, player);
		mass = mass + Cargo.Mass;

		Cargo = RandomHDFirearm("Weapons - Heavy", techID);
		if Cargo then
			self:AddOverridePurchase(Cargo, player);
			mass = mass + Cargo.Mass;
		end

		Cargo = RandomHDFirearm("Weapons - Secondary", techID);
		if Cargo then
			self:AddOverridePurchase(Cargo, player);
			mass = mass + Cargo.Mass;
		end

		if math.random() < rte.DiggersRate then
			if math.random() < 0.2 then
				if math.random() < 0.3 then
					Cargo = RandomHDFirearm("Tools - Breaching", techID);
					digger = 0.5;
				else
					Cargo = RandomTDExplosive("Tools - Breaching", techID);
				end
				self:AddOverridePurchase(Cargo, player);
				mass = mass + Cargo.Mass;
			else
				Cargo = RandomHDFirearm("Tools - Diggers", techID);
				self:AddOverridePurchase(Cargo, player);
				mass = mass + Cargo.Mass;
				digger = 1;
			end
		else
			Cargo = RandomTDExplosive("Bombs - Grenades", techID);
			if Cargo then
				self:AddOverridePurchase(Cargo, player);
				mass = mass + Cargo.Mass;
			end
		end
	end

	return digger, mass;
end

function MetaFight:PurchaseMediumInfantry(player, techID)
	local mass = 0;
	local digger = 0;
	local Cargo = RandomAHuman("Actors - Heavy", techID);
	if Cargo.ModuleID ~= techID then
		Cargo = RandomAHuman("Actors", techID);
	end

	if Cargo then
		self:AddOverridePurchase(Cargo, player);
		mass = mass + Cargo.Mass;

		Cargo = RandomHDFirearm("Weapons - Light", techID);
		if Cargo then
			self:AddOverridePurchase(Cargo, player);
			mass = mass + Cargo.Mass;
		end

		if math.random() < rte.DiggersRate then
			if math.random() < 0.4 then
				if math.random() < 0.5 then
					Cargo = RandomHDFirearm("Tools - Breaching", techID);
					digger = 0.5;
				else
					Cargo = RandomTDExplosive("Tools - Breaching", techID);
				end
				self:AddOverridePurchase(Cargo, player);
				mass = mass + Cargo.Mass;
			else
				Cargo = RandomHDFirearm("Tools - Diggers", techID);
				self:AddOverridePurchase(Cargo, player);
				mass = mass + Cargo.Mass;
				digger = 1;
			end
		else
			Cargo = RandomHDFirearm("Weapons - Secondary", techID);
			if Cargo then
				self:AddOverridePurchase(Cargo, player);
				mass = mass + Cargo.Mass;
			end
		end
	end

	return digger, mass;
end

function MetaFight:PurchaseLightInfantry(player, techID)
	local mass = 0;
	local digger = 0;
	local Cargo = RandomAHuman("Actors - Light", techID);
	if Cargo.ModuleID ~= techID then
		Cargo = RandomAHuman("Actors", techID);
	end

	if Cargo then
		self:AddOverridePurchase(Cargo, player);
		mass = mass + Cargo.Mass;

		Cargo = RandomHDFirearm("Weapons - Light", techID);
		if Cargo then
			self:AddOverridePurchase(Cargo, player);
			mass = mass + Cargo.Mass;
		end

		if math.random() < rte.DiggersRate then
			if math.random() < 0.6 then
				if math.random() < 0.7 then
					Cargo = RandomHDFirearm("Tools - Breaching", techID);
					digger = 0.5;
				else
					Cargo = RandomTDExplosive("Tools - Breaching", techID);
				end
				self:AddOverridePurchase(Cargo, player);
				mass = mass + Cargo.Mass;
			else
				Cargo = RandomHDFirearm("Tools - Diggers", techID);
				self:AddOverridePurchase(Cargo, player);
				mass = mass + Cargo.Mass;
				digger = 1;
			end
		else
			Cargo = RandomHDFirearm("Weapons - Secondary", techID);
			if Cargo then
				self:AddOverridePurchase(Cargo, player);
				mass = mass + Cargo.Mass;
			end
		end
	end

	return digger, mass;
end

function MetaFight:PurchaseScoutInfantry(player, techID)
	local mass = 0;
	local digger = 0;
	local Cargo = RandomAHuman("Actors - Light", techID);
	if Cargo.ModuleID ~= techID then
		Cargo = RandomAHuman("Actors", techID);
	end

	if Cargo then
		self:AddOverridePurchase(Cargo, player);
		mass = mass + Cargo.Mass;

		if math.random() < 0.6 then
			Cargo = RandomHDFirearm("Weapons - Secondary", techID);
			if Cargo then
				self:AddOverridePurchase(Cargo, player);
				mass = mass + Cargo.Mass;
			end
		else
			Cargo = RandomHDFirearm("Weapons - Light", techID);
			if Cargo then
				self:AddOverridePurchase(Cargo, player);
				mass = mass + Cargo.Mass;
			end
		end
	end

	return digger, mass;
end


-----------------------------------------------
-- Functions for creating random actors
function MetaFight:CreateHeavyInfantry(mode, techID)
	local Passenger = RandomAHuman("Actors - Heavy", techID);
	if Passenger.ModuleID ~= techID then
		Passenger = RandomAHuman("Actors", techID);
	end

	if Passenger then
		Passenger.AIMode = mode;
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Heavy", techID));
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", techID));

		if math.random() < rte.DiggersRate then
			Passenger:AddInventoryItem(RandomHDFirearm("Tools - Diggers", techID));
		else
			Passenger:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", techID));
			if math.random() < 0.5 then
				Passenger:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", techID));
			end
		end

		return Passenger;
	end
end

function MetaFight:CreateMediumInfantry(mode, techID)
	local Passenger = RandomAHuman("Actors - Heavy", techID);
	if Passenger.ModuleID ~= techID then
		Passenger = RandomAHuman("Actors", techID);
	end

	if Passenger then
		Passenger.AIMode = mode;
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Light", techID));
		if math.random() < rte.DiggersRate then
			Passenger:AddInventoryItem(RandomHDFirearm("Tools - Diggers", techID));
		else
			Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", techID));
		end

		return Passenger;
	end
end

function MetaFight:CreateLightInfantry(mode, techID)
	local Passenger = RandomAHuman("Actors - Light", techID);
	if Passenger.ModuleID ~= techID then
		Passenger = RandomAHuman("Actors", techID);
	end

	if Passenger then
		Passenger.AIMode = mode;
		Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Light", techID));
		if math.random() < rte.DiggersRate then
			Passenger:AddInventoryItem(RandomHDFirearm("Tools - Diggers", techID));
		else
			Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", techID));
		end

		return Passenger;
	end
end

function MetaFight:CreateScoutInfantry(mode, techID)
	local Passenger = RandomAHuman("Actors - Light", techID);
	if Passenger.ModuleID ~= techID then
		Passenger = RandomAHuman("Actors", techID);
	end

	if Passenger then
		Passenger.AIMode = mode;
		if math.random() < 0.6 then
			Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", techID));
			if math.random() < 0.6 then
				Passenger:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", techID));
			else
				Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", techID));
			end
		else
			Passenger:AddInventoryItem(RandomHDFirearm("Weapons - Light", techID));
		end

		return Passenger;
	end
end

function MetaFight:WrapPos(lzX)
	if SceneMan.SceneWrapsX then
		if lzX >= SceneMan.SceneWidth then
			lzX = lzX - SceneMan.SceneWidth;
		elseif lzX < 0 then
			lzX = SceneMan.SceneWidth + lzX;
		end
	else
		lzX = math.max(math.min(lzX, SceneMan.SceneWidth-50), 50);
	end

	return lzX;
end
