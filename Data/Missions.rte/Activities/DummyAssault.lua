package.loaded.Constants = nil;
require("Constants");

function DummyAssault:StartActivity(isNewGame)
	self.alarmZone = SceneMan.Scene:GetArea("Dummy Base Alarm");
	self.factoryZone = SceneMan.Scene:GetArea("Dummy Factory Invasion");
	self.searchArea = SceneMan.Scene:GetArea("Search Area");

	self.spawnTimer = Timer();
	self.brainDead = {};
	self.spawnTime = 30000 * math.exp(self.Difficulty * -0.011); -- Scale spawn time from 30s to 10s. Normal is ~17s

	self.CPUTeam = Activity.TEAM_2;
	self.CPUTech = self:GetTeamTech(Activity.TEAM_2);

	if isNewGame then
		self:StartNewGame();
	else
		self:ResumeLoadedGame();
	end
end

function DummyAssault:OnSave()
	self:SaveNumber("spawnTimer.ElapsedSimTimeMS", self.spawnTimer.ElapsedSimTimeMS);

	self:SaveNumber("alarmTriggered", self.alarmTriggered and 1 or 0);
end

function DummyAssault:StartNewGame()
	self:SetTeamFunds(self:GetStartingGold(), Activity.TEAM_1);

	if self:GetFogOfWarEnabled() then
		-- Make the scene unseen for the player team
		SceneMan:MakeAllUnseen(Vector(20, 20), Activity.TEAM_1);
		for x = SceneMan.SceneWidth - 1000, SceneMan.SceneWidth - 20, 25 do	-- Reveal the LZ
			SceneMan:CastSeeRay(Activity.TEAM_1, Vector(x,-25), Vector(0,SceneMan.SceneHeight), Vector(), 4, 20);
		end

		-- Hide the player LZ for the AI
		SceneMan:MakeAllUnseen(Vector(50, 50), Activity.TEAM_2);
		for y = 0, SceneMan.SceneHeight, 50 do
			for x = 0, SceneMan.SceneWidth-1000, 50 do
				SceneMan:RevealUnseen(x, y, Activity.TEAM_2);
			end
		end
	end

	-- Set up AI modes for the Actors that have been added to the scene by the scene definition
	for actor in MovableMan.AddedActors do
		if actor.Team == self.CPUTeam then
			actor.AIMode = Actor.AIMODE_SENTRY;
			if actor.PresetName == "Dummy Controller" then
				self.CPUBrain = actor;
			end
		end
	end

	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) then
			-- Check if we already have a brain assigned
			if not self:GetPlayerBrain(player) then
				self.brainDead[player] = false;
				local brain = CreateActor("Brain Case", "Base.rte");
				brain.Pos = Vector(3348, 1128-player*24);
				brain.RotAngle = math.rad(270);

				-- Let the spawn into the world, passing ownership
				MovableMan:AddActor(brain);

				-- Set the found brain to be the selected actor at start
				self:SetPlayerBrain(brain, player);
				CameraMan:SetScroll(brain.Pos, player);
				self:SetLandingZone(brain.Pos, player);

				-- Set the observation target to the brain, so that if/when it dies, the view flies to it in observation mode
				self:SetObservationTarget(brain.Pos, player);
			end
		end
	end
end

function DummyAssault:ResumeLoadedGame()
	self.spawnTimer.ElapsedSimTimeMS = self:LoadNumber("spawnTimer.ElapsedSimTimeMS");

	self.alarmTriggered = self:LoadNumber("alarmTriggered") ~= 0;

	for actor in MovableMan.AddedActors do
		if actor.Team == self.CPUTeam and actor.PresetName == "Dummy Controller" then
			self.CPUBrain = actor;
			break;
		end
	end
end

function DummyAssault:EndActivity()
	if self.humanTeam == self.WinnerTeam then
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self:PlayerActive(player) and self:PlayerHuman(player) then
				FrameMan:ClearScreenText(self:ScreenOfPlayer(player));
				FrameMan:SetScreenText("Congratulations, you've destroyed the enemy base!", self:ScreenOfPlayer(player), 0, -1, false);
			end
		end
	end

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

function DummyAssault:UpdateActivity()
	self:ClearObjectivePoints();

	if self.ActivityState ~= Activity.OVER then
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self:PlayerActive(player) and self:PlayerHuman(player) then
				local team = self:GetTeamOfPlayer(player);

				local brain = self:GetPlayerBrain(player);
				if not brain or not MovableMan:IsActor(brain) or not brain:HasObjectInGroup("Brains") then
					self:SetPlayerBrain(nil, player);
					-- Try to find a new unasigned brain this player can use instead, or if his old brain entered a craft
					local newBrain = MovableMan:GetUnassignedBrain(team);
					-- Found new brain actor, assign it and keep on truckin'
					if newBrain and not self.brainDead[player] then
						self:SetPlayerBrain(newBrain, player);
						self:SwitchToActor(newBrain, player, team);
						self:GetBanner(GUIBanner.RED, player):ClearText();
					else
						FrameMan:SetScreenText("Your brain has been lost!", self:ScreenOfPlayer(player), 333, -1, false);
						self.brainDead[player] = true;
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
		end

		if MovableMan:IsActor(self.CPUBrain) then
			self:AddObjectivePoint("Destroy!", self.CPUBrain.AboveHUDPos+Vector(0,-16), Activity.TEAM_1, GameActivity.ARROWDOWN);
		else
			self.WinnerTeam = Activity.TEAM_1;
			ActivityMan:EndActivity();
		end
	end

	if self.alarmTriggered then
		if self.spawnTimer:IsPastSimMS(self.spawnTime) then
			self.spawnTimer:Reset();

			local actor;
			if math.random() > self:GetCrabToHumanSpawnRatio(PresetMan:GetModuleID(self.CPUTech)) then
				actor = RandomAHuman("Actors - Light", self.CPUTech);
				if math.random() > 0.5 then
					actor.AIMode = Actor.AIMODE_BRAINHUNT;
					if math.random() > 0.5 then
						actor:AddInventoryItem(RandomHDFirearm("Tools - Diggers", self.CPUTech));
					end
				else
					actor.AIMode = Actor.AIMODE_GOTO;
					actor:ClearAIWaypoints();
					actor:AddAISceneWaypoint(self.searchArea:GetRandomPoint());
				end

				actor:AddInventoryItem(RandomHDFirearm("Weapons - Primary", self.CPUTech));
				if math.random() > 0.1 then
					actor:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", self.CPUTech));
				end
			else
				actor = RandomACrab("Actors - Mecha", self.CPUTech);
				actor.AIMode = Actor.AIMODE_BRAINHUNT;
			end

			actor.Team = Activity.TEAM_2;
			actor.Pos = Vector(5, 550);
			MovableMan:AddActor(actor);
		end
	elseif self.spawnTimer:IsPastSimMS(1000) then
		self.spawnTimer:Reset();

		for actor in MovableMan.Actors do
			if actor.Team == Activity.TEAM_1 and self.alarmZone:IsInside(actor.Pos) then
				self.alarmTriggered = true;
				break;
			end
		end
	end

	self:YSortObjectivePoints();
end