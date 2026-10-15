--[[

A battle between AI teams that plays out by itself, for watching or for testing things with.

Every active team gets a brain, and reinforcements that keep coming for free and go after the other teams' brains.
A team is out when its brain is gone, and the last team left wins.
Human players don't get to control anything, they can only look around.

--]]

function AIBattle:StartActivity()
	-- How long a team waits between its reinforcements, and how many units it can have before it stops getting more.
	self.reinforcementInterval = 20000;
	self.maxUnitsPerTeam = 8;

	self.deliveryCreationHandler = require("Activities/Utility/DeliveryCreationHandler");
	self.deliveryCreationHandler:Initialize(self);

	local teams = {};
	for team = Activity.TEAM_1, Activity.MAXTEAMCOUNT - 1 do
		if self:TeamActive(team) then
			table.insert(teams, team);
		end
	end

	self.brains = {};
	self.reinforcementTimers = {};
	for index, team in ipairs(teams) do
		local brain = PresetMan:GetLoadout("Infantry Brain", PresetMan:GetModuleID(self:GetTeamTech(team)), false);
		brain.Team = team;
		brain.AIMode = Actor.AIMODE_SENTRY;
		-- The brains start out evenly spread over the parts of the scene that are open to the sky.
		local openXs = self:FindXsOpenToSky(brain.Height);
		local brainX = #openXs > 0 and openXs[math.ceil(#openXs * (index - 0.5) / #teams)] or SceneMan.SceneWidth * (index - 0.5) / #teams;
		brain.Pos = SceneMan:MovePointToGround(Vector(brainX, 0), brain.Height * 0.3, 2);
		MovableMan:AddActor(brain);
		self.brains[team] = brain;

		self.reinforcementTimers[team] = Timer();
		self:SendReinforcements(team);
	end

	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) then
			-- Observing is what keeps the player from being given an actor to control, which would then stand around waiting for input.
			self:SetViewState(Activity.OBSERVE, player);
			self:SetObservationTarget(self.brains[self:GetTeamOfPlayer(player)].Pos, player);
		end
	end

	MusicMan:PlayDynamicSong("Generic Battle Music");
end

function AIBattle:UpdateActivity()
	if self.ActivityState == Activity.OVER then
		return;
	end

	local teamsLeft = 0;
	local lastTeamLeft = Activity.NOTEAM;
	for team = Activity.TEAM_1, Activity.MAXTEAMCOUNT - 1 do
		if self.brains[team] then
			if MovableMan:IsActor(self.brains[team]) then
				teamsLeft = teamsLeft + 1;
				lastTeamLeft = team;
				if self.reinforcementTimers[team]:IsPastSimMS(self.reinforcementInterval) then
					self.reinforcementTimers[team]:Reset();
					self:SendReinforcements(team);
				end
			else
				self.brains[team] = nil;
				MovableMan:KillAllTeamActors(team);
			end
		end
	end

	if teamsLeft <= 1 then
		self.WinnerTeam = lastTeamLeft;
		ActivityMan:EndActivity();
	end
end

-- Finds the X positions along the scene that have at least the given height of open air below the top of the scene, in order.
-- Where the terrain reaches the top of the scene, a brain would otherwise be put on top of it, outside the scene, where nothing can path to or from it.
-- Being open to the sky also lets the team's reinforcements be dropped off near their brain.
function AIBattle:FindXsOpenToSky(height)
	local openXs = {};
	for x = 0, SceneMan.SceneWidth - 1, 10 do
		if SceneMan:FindAltitude(Vector(x, 0), height, 2) >= height then
			table.insert(openXs, x);
		end
	end
	return openXs;
end

function AIBattle:SendReinforcements(team)
	local unitCount = 0;
	for actor in MovableMan.Actors do
		if actor.Team == team and (IsAHuman(actor) or IsACrab(actor)) then
			unitCount = unitCount + 1;
		end
	end
	if unitCount >= self.maxUnitsPerTeam then
		return;
	end

	-- The units come set to hunt for brains. Nothing is paid for them, so no team can run out of funds.
	local craft = self.deliveryCreationHandler:CreateSquadWithCraft(team);
	-- They get dropped off somewhere around their own brain.
	local landingX = self.brains[team].Pos.X + math.random(-200, 200);
	craft.Pos = Vector(math.max(100, math.min(SceneMan.SceneWidth - 100, landingX)), -30);
	MovableMan:AddActor(craft);
end
