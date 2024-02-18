package.loaded.Constants = nil;
require("Constants");

function DoainarMission:StartActivity(isNewGame)
	self.caveAreaA = SceneMan.Scene:GetArea("Cave Inside A");
	self.caveAreaB = SceneMan.Scene:GetArea("Cave Inside B");
	self.consoleArea = SceneMan.Scene:GetArea("Console");
	self.pitfallArea = SceneMan.Scene:GetArea("Pitfall");

	self.PlayerTeam = Activity.TEAM_1;
	self.CPUTeam = Activity.TEAM_2;

	self.spawnTimer = Timer();
	self.mamaJumpTimer = Timer();
	self.decipherTimer = Timer();

	self.brainHasLanded = {};
	self.brainDead = {};

	if isNewGame then
		self:StartNewGame();
	else
		self:ResumeLoadedGame();
	end
end

function DoainarMission:OnSave()
	self:SaveNumber("spawnTimer.ElapsedSimTimeMS", self.spawnTimer.ElapsedSimTimeMS);
	self:SaveNumber("mamaJumpTimer.ElapsedSimTimeMS", self.mamaJumpTimer.ElapsedSimTimeMS);
	self:SaveNumber("decipherTimer.ElapsedSimTimeMS", self.decipherTimer.ElapsedSimTimeMS);

	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) then
			self:SaveNumber("brainHasLanded." .. tostring(player), self.brainHasLanded[player] and 1 or 0);
			self:SaveNumber("brainDead." .. tostring(player), self.brainDead[player] and 1 or 0);
		end
	end

	self:SaveNumber("sacDestroyed", self.sacDestroyed and 1 or 0);
	self:SaveNumber("mamaAggressive", self.mamaAggressive and 1 or 0);
	self:SaveNumber("mamaDead", self.mamaDead and 1 or 0);
	self:SaveNumber("passedPitfall", self.passedPitfall and 1 or 0);

	if not self.mamaDead and self.mamaCrab and MovableMan:IsActor(self.mamaCrab) then
		self:SaveNumber("mamaCrab.Status", self.mamaCrab.Status);
	end
end

function DoainarMission:StartNewGame()
	self:SetTeamFunds(math.max(100, self:GetStartingGold()), self.PlayerTeam);

	self.sacDestroyed = false;
	self.mamaAggressive = false;
	self.mamaDead = false;
	self.passedPitfall = false;

	self.eggSac = CreateAEmitter("Eggsac");
	self.eggSac.Pos = Vector(1274, 315);
	self.eggSac.Team = self.CPUTeam;
	MovableMan:AddParticle(self.eggSac);

	self.mamaCrab = CreateACrab("Mega Crab");
	self.mamaCrab.Pos = Vector(1176, 368);
	self.mamaCrab.Team = self.CPUTeam;
	self.mamaCrab.SpriteAnimDuration = self.mamaCrab.SpriteAnimDuration * 10;
	self.mamaCrab.PinStrength = 1;
	MovableMan:AddActor(self.mamaCrab);
	self.mamaCrab.Status = Actor.INACTIVE; -- Note: This must be set after adding the Actor to MovableMan so it doesn't get overwritten by the game.

	--Set all the underground doors to be quiet so that we don't ruin the surprise!
	for actor in MovableMan.AddedActors do
		if actor.ClassName == "ADoor" then
			actor = ToADoor(actor);
			actor.DoorMoveSound.Volume = 0;
			actor.DoorMoveStartSound.Volume = 0;
			actor.DoorMoveEndSound.Volume = 0;
			actor.DoorDirectionChangeSound.Volume = 0;
		end
	end

	SceneMan:MakeAllUnseen(Vector(24, 24), self.PlayerTeam);

	self:SetupHumanPlayerBrains();
end

function DoainarMission:SetupHumanPlayerBrains()
	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		-- Check if we already have a brain assigned
		if self:PlayerActive(player) and self:PlayerHuman(player) and not self:GetPlayerBrain(player) then
			local ship = CreateACRocket("Drop Crate");
			local brain = CreateAHuman("Brain Robot");
			brain:AddInventoryItem(CreateHDFirearm("Assault Rifle"));
			brain.AIMode = Actor.AIMODE_SENTRY;
			ship:AddInventoryItem(brain);
			ship.Pos.X = 50 + player * ship.Radius;
			ship.Vel.X = math.random(8, 12);
			ship.Team = self:GetTeamOfPlayer(player);
			ship:SetControllerMode(Controller.CIM_AI, -1);
			-- Let the spawn into the world, passing ownership
			MovableMan:AddActor(ship);
			-- Set the found brain to be the selected actor at start
			self:SetPlayerBrain(brain, player);
			self:SetViewState(Activity.ACTORSELECT, player);
			CameraMan:SetScroll(self:GetPlayerBrain(player).Pos, player);
			self:SetActorSelectCursor(Vector(3024, 324), player);
			self:SetLandingZone(self:GetPlayerBrain(player).Pos, player);
			-- Set the observation target to the brain, so that if/when it dies, the view flies to it in observation mode
			self:SetObservationTarget(self:GetPlayerBrain(player).Pos, player);
			self.brainHasLanded[player] = false;
			self.brainDead[player] = false;
		end
	end
end

function DoainarMission:ResumeLoadedGame()
	self.spawnTimer.ElapsedSimTimeMS = self:LoadNumber("spawnTimer.ElapsedSimTimeMS");
	self.mamaJumpTimer.ElapsedSimTimeMS = self:LoadNumber("mamaJumpTimer.ElapsedSimTimeMS");
	self.decipherTimer.ElapsedSimTimeMS = self:LoadNumber("decipherTimer.ElapsedSimTimeMS");

	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) then
			self.brainHasLanded[player] = self:LoadNumber("brainHasLanded." .. tostring(player)) ~= 0;
			self.brainDead[player] = self:LoadNumber("brainDead." .. tostring(player)) ~= 0;
		end
	end

	self.sacDestroyed = self:LoadNumber("sacDestroyed") ~= 0;
	self.mamaAggressive = self:LoadNumber("mamaAggressive") ~= 0;
	self.mamaDead = self:LoadNumber("mamaDead") ~= 0;
	self.passedPitfall = self:LoadNumber("passedPitfall") ~= 0;

	for actor in MovableMan.AddedActors do
		if actor.PresetName == "Mega Crab" then
			self.mamaCrab = ToACrab(actor);
			self.mamaCrab.Status = self:LoadNumber("mamaCrab.Status"); -- Note: This must be handled specially, because inactive status is overwritten when an Actor is added to MovableMan.
			break;
		end
	end

	for particle in MovableMan.AddedParticles do
		if particle.PresetName == "Eggsac" then
			self.eggSac = ToAEmitter(particle);
			break;
		end
	end

	if self.mamaAggressive and not self.mamaDead then
		AudioMan:PlayMusic("Base.rte/Music/dBSoundworks/bossfight.ogg", -1, -1);
	elseif self.passedPitfall then
		AudioMan:PlayMusic("Base.rte/Music/dBSoundworks/ruinexploration.ogg", -1, -1);
	end
end

function DoainarMission:EndActivity()
	-- Temp fix so music doesn't start playing if ending the Activity when changing resolution through the ingame settings.
	if not self:IsPaused() then
		-- Play sad music if no humans are left
		if self:HumanBrainCount() == 0 then
			AudioMan:ClearMusicQueue();
			AudioMan:PlayMusic("Base.rte/Music/dBSoundworks/udiedfinal.ogg", 2, -1.0);
			AudioMan:QueueSilence(10);
			AudioMan:QueueMusicStream("Base.rte/Music/dBSoundworks/ccambient4.ogg");
		else
			-- But if humans are left, then play happy music!
			AudioMan:ClearMusicQueue();
			AudioMan:PlayMusic("Base.rte/Music/dBSoundworks/uwinfinal.ogg", 2, -1.0);
			AudioMan:QueueSilence(10);
			AudioMan:QueueMusicStream("Base.rte/Music/dBSoundworks/ccambient4.ogg");
		end
	end
end

function DoainarMission:UpdateActivity()
	-- Clear all objective markers, they get re-added each frame
	self:ClearObjectivePoints();
	if self.mamaCrab and MovableMan:IsActor(self.mamaCrab) then
		if not MovableMan:IsActor(self.target) then
			self.target = MovableMan:GetClosestTeamActor(self.PlayerTeam, Activity.PLAYER_NONE, self.mamaCrab.Pos, SceneMan.SceneWidth, Vector(), self.mamaCrab);
		end
	else
		self.mamaCrab = nil;
	end

	local crabcount = 0;
	local crabsOutside = 0;
	for _, actorCollection in ipairs({MovableMan.AddedActors, MovableMan.Actors}) do
		for actor in actorCollection do
			if actor.PresetName == "Crab" then
				crabcount = crabcount + 1;
				if not self.mamaCrab then
					self:AddObjectivePoint("Kill!", actor.AboveHUDPos, self.PlayerTeam, GameActivity.ARROWDOWN);
				elseif not self.mamaAggressive and not self.caveAreaA:IsInside(actor.Pos) and not self.caveAreaB:IsInside(actor.Pos) then
					self:AddObjectivePoint("Kill!", actor.AboveHUDPos, self.PlayerTeam, GameActivity.ARROWDOWN);
					crabsOutside = crabsOutside + 1;
				end
				if actor.Age < TimerMan.DeltaTimeMS then
					actor.AIMode = math.random() < 0.5 and Actor.AIMODE_BRAINHUNT or Actor.AIMODE_PATROL;
				end
			end
		end
	end

	local playerInsideConsoleArea = 0;
	local invest2obj = 0;
	if not (self.ActivityState == Activity.OVER) then
		-- Iterate through all human players
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self:PlayerActive(player) and self:PlayerHuman(player) then
				-- The current player's team
				local team = self:GetTeamOfPlayer(player);
				-- Make sure the game is not already ending
				-- Check if any player's brain is dead
				local brain = self:GetPlayerBrain(player);
				if not brain or not MovableMan:IsActor(brain) or not brain:HasObjectInGroup("Brains") then
					self:SetPlayerBrain(nil, player);
					-- Try to find a new unasigned brain this player can use instead, or if his old brain entered a craft
					local newBrain = MovableMan:GetUnassignedBrain(team);
					-- Found new brain actor, assign it and keep on truckin'
					if newBrain and self.brainDead[player] == false then
						self:SetPlayerBrain(newBrain, player);
						self:SwitchToActor(newBrain, player, team);
					else
						FrameMan:SetScreenText("Your brain has been lost!", self:ScreenOfPlayer(player), 333, -1, true);
						self.brainDead[player] = true;
						-- Now see if all brains of self player's team are dead, and if so, end the game
						if not MovableMan:GetFirstBrainActor(team) then
							self.WinnerTeam = self:OtherTeam(team);
							ActivityMan:EndActivity();
						end
					end
				else
					-- Update the observation target to the brain, so that if/when it dies, the view flies to it in observation mode
					self:SetObservationTarget(brain.Pos, player);
				end
				if self.brainDead[player] == false then
					if (-self.spawnTimer:LeftTillSimMS(0) >= 25) then
						--Check if all the outside crabs have died, and if so, enter the "aggression" stage, where all the crabs try to kill you.
						if self.mamaAggressive == false then
							if MovableMan:IsActor(self.mamaCrab) and (crabsOutside == 0 or self.mamaCrab.PinStrength == 0 or self.mamaCrab.Health < self.mamaCrab.MaxHealth) then
								self.mamaCrab.SpriteAnimDuration = self.mamaCrab.SpriteAnimDuration * 0.1;
								self.mamaAggressive = true;

								self.mamaCrab.PinStrength = 0;
								self.mamaCrab.Status = Actor.UNSTABLE;

								self.target = MovableMan:GetClosestTeamActor(self.PlayerTeam, Activity.PLAYER_NONE, self.mamaCrab.Pos, SceneMan.SceneWidth, Vector(), self.mamaCrab);

								self:ResetMessageTimer(player);
								FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
								FrameMan:SetScreenText("Uh oh, looks like you angered the mother crab!  Kill it before it kills you!", self:ScreenOfPlayer(player), 0, 7500, true);
								AudioMan:PlayMusic("Base.rte/Music/dBSoundworks/bossfight.ogg", -1, -1);
							end
						end

						if MovableMan:IsActor(brain) and self.brainHasLanded[player] == false then
							self:SwitchToActor(brain, player, team);
							self.brainHasLanded[player] = true;
							self:ResetMessageTimer(player);
							FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
							FrameMan:SetScreenText("Looks like there's a crab den down here.  We'll have to clear them out first.", self:ScreenOfPlayer(player), 0, 7500, true);
						end

						if not self.mamaCrab and self.mamaDead == false and self.mamaAggressive == true then
							self.mamaDead = true;
							self:ResetMessageTimer(player);
							FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
							FrameMan:SetScreenText("That was a close one.  Go finish off their den!", self:ScreenOfPlayer(player), 0, 7500, true);
							AudioMan:PlayMusic("Base.rte/Music/dBSoundworks/cc2g.ogg", -1, -1);
						end

						if MovableMan:IsParticle(self.eggSac) == false and self.sacDestroyed == false then
							self:ResetMessageTimer(player);
							FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
							FrameMan:SetScreenText("Looks like a cave-in happened down here.  Dig through that sand, there might be something under it.", self:ScreenOfPlayer(player), 0, 7500, true);
							self.sacDestroyed = true;
						end

						if MovableMan:IsActor(self:GetControlledActor(player)) then
							if self.pitfallArea:IsInside(self:GetControlledActor(player).Pos) and self.passedPitfall == false then
								self:ResetMessageTimer(player);
								FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
								FrameMan:SetScreenText("What the...?  It's some kind of ancient bunker?  There seems to be a control panel inside, go see what's on it...", self:ScreenOfPlayer(player), 0, 7500, true);
								AudioMan:ClearMusicQueue();
								AudioMan:PlayMusic("Base.rte/Music/dBSoundworks/ruinexploration.ogg", -1, -1);
								self.passedPitfall = true;

								for actor in MovableMan.Actors do
									if actor.ClassName == "ADoor" then
										actor = ToADoor(actor);
										actor.DoorMoveSound.Volume = 1;
										actor.DoorMoveStartSound.Volume = 1;
										actor.DoorMoveEndSound.Volume = 1;
										actor.DoorDirectionChangeSound.Volume = 1;
									end
								end
							end
							if self.consoleArea:IsInside(self:GetControlledActor(player).Pos) then
								self:GetControlledActor(player).HUDVisible = false;
								playerInsideConsoleArea = 1;
							else
								self:GetControlledActor(player).HUDVisible = true;
							end
						end

						if self.sacDestroyed == true and self.passedPitfall == false then
							invest2obj = 1;
						end

						if MovableMan:IsActor(brain) and self.passedPitfall == false and playerInsideConsoleArea == 0 then
							self:AddObjectivePoint("Protect!", brain.AboveHUDPos + Vector(0, -8), self.PlayerTeam, GameActivity.ARROWDOWN);
						end
					end
				end
			end
		end
	end

	if self.mamaDead == false and MovableMan:IsActor(self.mamaCrab) and self.mamaCrab.Status ~= Actor.INACTIVE then
		self:AddObjectivePoint("Kill!", self.mamaCrab.AboveHUDPos+Vector(0, -16), self.PlayerTeam, GameActivity.ARROWDOWN);
		if self.mamaJumpTimer:LeftTillSimMS(3000) < 0 and MovableMan:IsActor(self.target) and SceneMan:GetTerrMatter(self.mamaCrab.Pos.X, self.mamaCrab.Pos.Y + self.mamaCrab:GetSpriteHeight() * 0.5) then
			local jumpVector = Vector((self.target.Pos.X - self.mamaCrab.Pos.X) * 0.045, -15 + ((self.target.Pos.Y - self.mamaCrab.Pos.Y) * 0.025));
			self.mamaCrab.Vel = self.mamaCrab.Vel + jumpVector:SetMagnitude(math.min(jumpVector.Magnitude, 30));
			self.mamaCrab.Status = Actor.UNSTABLE;
			self.mamaJumpTimer:Reset();
		end
	elseif self.mamaAggressive == true and MovableMan:IsParticle(self.eggSac) then
		self:AddObjectivePoint("Destroy!", self.eggSac.Pos+Vector(0,-16), self.PlayerTeam, GameActivity.ARROWDOWN);
	end
	if self.eggSac and MovableMan:IsParticle(self.eggSac) then
		self.eggSac.Throttle = 2/(1 + (crabcount + self.eggSac.WoundCount) * 0.1) - 1;
		self.eggSac.SpriteAnimDuration = 600 - 300 * self.eggSac.Throttle;
	else
		self.eggSac = nil;
	end

	--Reading the console
	if playerInsideConsoleArea == 1 or self.WinnerTeam == self.PlayerTeam then
		if MovableMan:IsParticle(self.litscreen) == false then
			self.litscreen = CreateAEmitter("Lit Screen");
			self.litscreen:EnableEmission(true);
			self.litscreen.Pos = Vector(1104, 612);
			MovableMan:AddParticle(self.litscreen);
		end
		if self.decipherTimer:LeftTillSimMS(3000) > 0 then
			self:AddObjectivePoint("Loading... " .. math.ceil(self.decipherTimer:LeftTillSimMS(3000)/1000) .. " seconds left.", Vector(1104, 600), self.PlayerTeam, GameActivity.ARROWDOWN);
		elseif self.decipherTimer:IsPastSimMS(3000) then
			local textTime = 5000;
			for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
				if self:PlayerActive(player) and self:PlayerHuman(player) then
					FrameMan:SetScreenText("These are cartesian coordinates...  Where could they possibly lead to?", self:ScreenOfPlayer(player), 0, textTime, true);
				end
			end
			self.WinnerTeam = self.PlayerTeam;
			self:ClearObjectivePoints();
			if self.decipherTimer:IsPastSimMS(3000 + textTime) then
				ActivityMan:EndActivity();
			end
		end
	else
		self.decipherTimer:Reset();
		if MovableMan:IsParticle(self.litscreen) then
			self.litscreen.Lifetime = 1;
		end
	end

	if self.passedPitfall == true and playerInsideConsoleArea == 0 then
		self:AddObjectivePoint("Investigate!", Vector(1104, 600), self.PlayerTeam, GameActivity.ARROWDOWN);
	end

	if invest2obj == 1 then
		self:AddObjectivePoint("Get a digging tool and dig!", Vector(1400, 400), self.PlayerTeam, GameActivity.ARROWDOWN);
	end
	 --Sort the objective points
	self:YSortObjectivePoints();
end