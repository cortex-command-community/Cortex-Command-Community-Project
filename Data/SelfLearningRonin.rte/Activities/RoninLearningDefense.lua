-- Ronin Learning Defense
--
-- A Ronin commander holds a base on the far side of the map. The AI keeps part of its army at home to defend and
-- sends the rest out on sorties. Every decision that shapes a sortie is made by a small "contextual bandit" learner
-- (see "Mind" below), and the outcome of the sortie (damage dealt, kills, own losses) is fed back into its memory.
-- The memory is written to SelfLearningRonin.rte/Memory.txt, so the AI keeps getting better from match to match.
--
-- What is learned (every dimension is learned separately for 9 situations: enemy army size x distance to enemy brain):
--   target  - what a sortie goes for first: enemy brain / nearest enemy / weakest enemy / the same two but from behind
--   size    - how many soldiers go out
--   retreat - at which squad survival rate the squad pulls back (never / 34% / 60%)
--   comp    - share of heavy soldiers in the army and in a squad
--   weapon  - which weapon group the soldiers are armed with
--   defense - which share of the army always stays home to defend the commander

local MEMORY_FILE = "SelfLearningRonin.rte/Memory.txt";
local MEMORY_TEMP = "SelfLearningRonin.rte/Memory.tmp";
local MEMORY_VERSION = "RLR1";
local TECH = "Ronin.rte";

local DIMENSIONS = {
	target = {"brain", "nearest", "weakest", "flankBrain", "flankNearest"},
	size = {2, 3, 4, 5, 6},
	retreat = {0, 0.34, 0.6},
	comp = {0, 0.34, 0.67, 1},
	weapon = {"Weapons - Primary", "Weapons - CQB", "Weapons - Heavy", "Weapons - Light", "Weapons - Explosive", "Weapons - Sniper"},
	defense = {0.3, 0.5, 0.7, 0.9},
};

local THINK_INTERVAL_MS = 1000;
local DEFENSE_CYCLE_MS = 45000;
local SORTIE_COOLDOWN_MS = 15000;
local SORTIE_MAX_MS = 150000;
local SORTIE_RETURN_MAX_MS = 40000;
local FIRST_SORTIE_MS = 40000;
local MAX_SQUADS = 2;
local LIGHT_COST = 20;
local HEAVY_COST = 45;
local MAX_MANPOWER = 300;
local HOME_RADIUS = 750;
local SQUAD_SIGHT_RADIUS = 450;

----------------------------------------------------------------------------------------------------------------------
-- Helpers
----------------------------------------------------------------------------------------------------------------------

local function Clamp(value, low, high)
	if value < low then
		return low;
	elseif value > high then
		return high;
	end
	return value;
end

local function Dist(a, b)
	return SceneMan:ShortestDistance(a, b, false).Magnitude;
end

local function IsCombatant(actor)
	return actor.ClassName == "AHuman" or actor.ClassName == "ACrab";
end

-- Returns the actor with this unique ID if it exists and is still alive, otherwise nil.
local function LiveActorById(id)
	local mo = MovableMan:FindObjectByUniqueID(id);
	if mo and MovableMan:IsActor(mo) then
		local actor = ToActor(mo);
		if actor.Status ~= Actor.DEAD and actor.Health > 0 then
			return actor;
		end
	end
	return nil;
end

local function NearestOption(dimension, value)
	local best, bestDiff = nil, math.huge;
	for _, option in ipairs(DIMENSIONS[dimension]) do
		local diff = math.abs(option - value);
		if diff < bestDiff then
			best, bestDiff = option, diff;
		end
	end
	return best;
end

local function Split(text, separator)
	local parts = {};
	local position = 1;
	while position <= #text + 1 do
		local stop = string.find(text, separator, position, true);
		if not stop then
			table.insert(parts, string.sub(text, position));
			break;
		end
		table.insert(parts, string.sub(text, position, stop - 1));
		position = stop + 1;
	end
	return parts;
end

----------------------------------------------------------------------------------------------------------------------
-- Mind: the part that learns
----------------------------------------------------------------------------------------------------------------------

local Mind = {};

function Mind.Reset()
	Mind.data = {matches = 0, wins = 0, losses = 0, sorties = 0, q = {}};
end
Mind.Reset();

local function QKey(ctx, dimension, option)
	return ctx .. "|" .. dimension .. "|" .. tostring(option);
end

function Mind.Get(ctx, dimension, option)
	local entry = Mind.data.q[QKey(ctx, dimension, option)];
	if entry then
		return entry.n, entry.v;
	end
	return 0, 0;
end

-- Moves the value of one option towards the reward. Early on this is a plain average, later on it is a moving average
-- with a floor on the step size so the AI can keep adapting when the player changes his habits.
local function Update(ctx, dimension, option, reward, weight)
	local key = QKey(ctx, dimension, option);
	local entry = Mind.data.q[key];
	if not entry then
		entry = {n = 0, v = 0};
		Mind.data.q[key] = entry;
	end
	entry.n = math.min(entry.n + weight, 100000);
	local step = math.min(1, math.max(0.06, weight / entry.n));
	entry.v = entry.v + step * (reward - entry.v);
end

-- Learns from one outcome, both for the specific situation and for the "*" (any situation) table.
function Mind.Learn(ctx, dimension, option, reward, weight)
	weight = weight or 1;
	Update(ctx, dimension, option, reward, weight);
	Update("*", dimension, option, reward, weight);
end

-- How adventurous the AI is. Starts high and decays with the number of sorties it has flown, but never reaches zero.
function Mind.Exploration()
	return math.max(0.12, 0.9 * (0.5 ^ (Mind.data.sorties / 120)));
end

function Mind.Value(ctx, dimension, option)
	local nCtx, vCtx = Mind.Get(ctx, dimension, option);
	local nAll, vAll = Mind.Get("*", dimension, option);
	local trust = nCtx / (nCtx + 4);
	return trust * vCtx + (1 - trust) * vAll, nCtx + 0.25 * nAll;
end

-- Picks an option with an upper-confidence-bound rule: promising options are repeated, rarely tried ones get a bonus.
function Mind.Select(ctx, dimension)
	local options = DIMENSIONS[dimension];
	local values, counts, total = {}, {}, 0;
	for i, option in ipairs(options) do
		values[i], counts[i] = Mind.Value(ctx, dimension, option);
		total = total + counts[i];
	end
	local explore = Mind.Exploration();
	local bestIndex, bestScore = 1, -math.huge;
	for i = 1, #options do
		local bonus = explore * math.sqrt(math.log(total + 2) / (counts[i] + 1));
		local score = values[i] + bonus + math.random() * 0.02;
		if score > bestScore then
			bestIndex, bestScore = i, score;
		end
	end
	return options[bestIndex];
end

function Mind.Save()
	local d = Mind.data;
	local file = LuaMan:FileOpen(MEMORY_TEMP, "w");
	if not file or file < 0 then
		if not Mind.warned then
			Mind.warned = true;
			ConsoleMan:PrintString("WARNING: Ronin AI could not write " .. MEMORY_FILE .. ", what it learns will be lost when the game closes.");
		end
		return false;
	end
	LuaMan:FileWriteLine(file, MEMORY_VERSION .. "\n");
	LuaMan:FileWriteLine(file, "stat|matches|" .. d.matches .. "\n");
	LuaMan:FileWriteLine(file, "stat|wins|" .. d.wins .. "\n");
	LuaMan:FileWriteLine(file, "stat|losses|" .. d.losses .. "\n");
	LuaMan:FileWriteLine(file, "stat|sorties|" .. d.sorties .. "\n");
	for key, entry in pairs(d.q) do
		LuaMan:FileWriteLine(file, "q|" .. key .. "|" .. string.format("%.3f", entry.n) .. "|" .. string.format("%.5f", entry.v) .. "\n");
	end
	LuaMan:FileWriteLine(file, "end\n");
	LuaMan:FileClose(file);

	if LuaMan:FileExists(MEMORY_FILE) then
		LuaMan:FileRemove(MEMORY_FILE);
	end
	return LuaMan:FileRename(MEMORY_TEMP, MEMORY_FILE);
end

local function ReadMemoryFile(path)
	if not LuaMan:FileExists(path) then
		return nil;
	end
	local file = LuaMan:FileOpen(path, "r");
	if not file or file < 0 then
		return nil;
	end
	local loaded = {matches = 0, wins = 0, losses = 0, sorties = 0, q = {}};
	local header = LuaMan:FileReadLine(file):gsub("[\r\n]+$", "");
	local complete = false;
	if header == MEMORY_VERSION then
		local guard = 0;
		while not LuaMan:FileEOF(file) and guard < 20000 do
			guard = guard + 1;
			local line = LuaMan:FileReadLine(file):gsub("[\r\n]+$", "");
			if line == "end" then
				complete = true;
				break;
			elseif line ~= "" then
				local parts = Split(line, "|");
				if parts[1] == "stat" and parts[2] and tonumber(parts[3]) then
					loaded[parts[2]] = tonumber(parts[3]);
				elseif parts[1] == "q" and #parts == 6 and tonumber(parts[5]) and tonumber(parts[6]) then
					loaded.q[parts[2] .. "|" .. parts[3] .. "|" .. parts[4]] = {n = tonumber(parts[5]), v = tonumber(parts[6])};
				end
			end
		end
	end
	LuaMan:FileClose(file);
	return complete and loaded or nil;
end

function Mind.Load()
	Mind.Reset();
	-- The temp file only matters if the game died between removing the old file and renaming the new one.
	local loaded = ReadMemoryFile(MEMORY_FILE) or ReadMemoryFile(MEMORY_TEMP);
	if loaded then
		Mind.data = loaded;
	end
end

-- Situation buckets: how many enemy soldiers are around and how far away is the enemy brain.
function Mind.Context(enemyCount, enemyDistance)
	local strength = enemyCount <= 2 and 0 or (enemyCount <= 5 and 1 or 2);
	local distance = enemyDistance < 1000 and 0 or (enemyDistance < 2000 and 1 or 2);
	return "S" .. strength .. "D" .. distance;
end

----------------------------------------------------------------------------------------------------------------------
-- Activity setup
----------------------------------------------------------------------------------------------------------------------

function RoninLearningDefense:StartActivity(isNewGame)
	self.techID = PresetMan:GetModuleID(TECH);

	-- Find the CPU team, add one if the setup screen did not provide it
	self.aiTeam = -1;
	for team = Activity.TEAM_1, Activity.MAXTEAMCOUNT - 1 do
		if self:TeamActive(team) and self:TeamIsCPU(team) then
			self.aiTeam = team;
			break;
		end
	end

	self.humanTeams = {};
	local firstHumanTeam = Activity.TEAM_1;
	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) then
			local team = self:GetTeamOfPlayer(player);
			if next(self.humanTeams) == nil then
				firstHumanTeam = team;
			end
			self.humanTeams[team] = true;
		end
	end

	if self.aiTeam < 0 then
		self.aiTeam = firstHumanTeam + 1;
		if self.aiTeam > Activity.TEAM_4 then
			self.aiTeam = Activity.TEAM_1;
		end
		self.CPUTeam = self.aiTeam;
	end

	self.startTimer = Timer();
	self.thinkTimer = Timer();
	self.spawnTimer = Timer();
	self.sortieTimer = Timer();
	self.matchTimer = Timer();

	self.units = {};		-- id -> {heavy, weapon, state, post}
	self.squads = {};
	self.nextSquadId = 1;
	self.manpower = 100;
	self.needBase = isNewGame;
	self.aiBrainId = nil;
	self.finished = false;
	self.ctx = "S0D1";
	self.defRatio = 0.5;
	self.defCycleStart = 0;
	self.defSeen = {};
	self.defHomeLosses = 0;
	self.defBrainHealthStart = nil;
	self.matchDecisions = {};
	self.lastThinkMS = 0;

	Mind.Load();

	if isNewGame then
		for team = Activity.TEAM_1, Activity.MAXTEAMCOUNT - 1 do
			if self:TeamActive(team) then
				self:SetTeamFunds(team == self.aiTeam and 0 or self:GetStartingGold(), team);
			end
		end
		self:SetupHumanPlayerBrains();
	else
		self.manpower = self:LoadNumber("manpower");
		if self.manpower <= 0 then
			self.manpower = 100;
		end
	end
end

function RoninLearningDefense:OnSave()
	self:SaveNumber("manpower", self.manpower);
end

function RoninLearningDefense:SetupHumanPlayerBrains()
	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) then
			if not self:GetPlayerBrain(player) then
				local foundBrain = MovableMan:GetUnassignedBrain(self:GetTeamOfPlayer(player));
				-- No brain in the scene: switch to editing mode so the player can place one
				if not foundBrain then
					self.ActivityState = Activity.EDITING;
					MusicMan:PlayDynamicSong("Generic Ambient Music");
					self:SetLandingZone(Vector(player * SceneMan.SceneWidth / 4, 0), player);
				else
					MusicMan:PlayDynamicSong("Generic Battle Music");
					self:SetPlayerBrain(foundBrain, player);
					self:SwitchToActor(foundBrain, player, self:GetTeamOfPlayer(player));
					self:SetLandingZone(self:GetPlayerBrain(player).Pos, player);
					self:SetObservationTarget(self:GetPlayerBrain(player).Pos, player);
				end
			end
		end
	end
end

function RoninLearningDefense:EndActivity()
	if not self:IsPaused() then
		if self:HumanBrainCount() == 0 then
			MusicMan:PlayDynamicSong("Generic Defeat Music", "Default", true);
		else
			MusicMan:PlayDynamicSong("Generic Victory Music", "Default", true);
		end
		MusicMan:PlayDynamicSong("Generic Ambient Music");
	end
	-- If the match ended by quitting, keep what was learned during the sorties
	if not self.finished then
		Mind.Save();
	end
end

----------------------------------------------------------------------------------------------------------------------
-- Main update
----------------------------------------------------------------------------------------------------------------------

function RoninLearningDefense:UpdateActivity()
	if self.ActivityState == Activity.OVER then
		return;
	end

	if self.ActivityState == Activity.EDITING then
		self.startTimer:Reset();
		self.matchTimer:Reset();
		self.thinkTimer:Reset();
		self.lastThinkMS = 0;
		return;
	end

	local humansAlive, enemyBrainActors = self:UpdateHumans();

	if not self.startTimer:IsPastSimMS(1500) then
		if self.startTimer:IsPastSimMS(100) then
			for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
				if self:PlayerActive(player) and self:PlayerHuman(player) then
					local d = Mind.data;
					local text = string.format("Ronin AI memory: %d matches (%d won), %d sorties, exploration %d%%", d.matches, d.wins, d.sorties, math.floor(Mind.Exploration() * 100));
					FrameMan:SetScreenText(text, self:ScreenOfPlayer(player), 333, 6000, true);
				end
			end
		end
		return;
	end

	if humansAlive < 1 then
		self:Finish(true);
		return;
	end

	if self.thinkTimer:IsPastSimMS(THINK_INTERVAL_MS) then
		self.thinkTimer:Reset();
		self:Think(enemyBrainActors);
	end
end

-- Keeps the human brains assigned, draws objective markers. Returns the number of living human brains
-- and a list of those brains.
function RoninLearningDefense:UpdateHumans()
	self:ClearObjectivePoints();
	local alive = 0;
	local brains = {};
	local aiBrain = self.aiBrainId and LiveActorById(self.aiBrainId) or nil;

	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) then
			local team = self:GetTeamOfPlayer(player);
			-- If the brain is gone, maybe the player just entered a craft; look for another one
			if not MovableMan:IsActor(self:GetPlayerBrain(player)) then
				local newBrain = MovableMan:GetUnassignedBrain(team);
				if newBrain then
					self:SetPlayerBrain(newBrain, player);
					self:SwitchToActor(newBrain, player, team);
					self:SetLandingZone(self:GetPlayerBrain(player).Pos, player);
					self:SetObservationTarget(self:GetPlayerBrain(player).Pos, player);
				end
			end

			if not MovableMan:IsActor(self:GetPlayerBrain(player)) then
				self:SetPlayerBrain(nil, player);
				self:ResetMessageTimer(player);
				local screen = self:ScreenOfPlayer(player);
				FrameMan:ClearScreenText(screen);
				FrameMan:SetScreenText("Your brain has been destroyed!", screen, 333, -1, false);
			else
				alive = alive + 1;
				local brain = self:GetPlayerBrain(player);
				table.insert(brains, brain);
				self:AddObjectivePoint("Protect!", brain.AboveHUDPos, team, GameActivity.ARROWDOWN);
				if aiBrain then
					self:AddObjectivePoint("Destroy!", aiBrain.AboveHUDPos, team, GameActivity.ARROWDOWN);
				end
			end
		end
	end
	return alive, brains;
end

----------------------------------------------------------------------------------------------------------------------
-- The AI's once-per-second thinking
----------------------------------------------------------------------------------------------------------------------

function RoninLearningDefense:Clock()
	return self.matchTimer.ElapsedSimTimeMS;
end

function RoninLearningDefense:Think(humanBrains)
	local clock = self:Clock();
	local dt = math.max(0, (clock - self.lastThinkMS) / 1000);
	self.lastThinkMS = clock;

	-- Look at the battlefield
	local enemies = {};
	local mine = {};
	local aiBrain = nil;
	for actor in MovableMan.Actors do
		if IsCombatant(actor) and actor.Status ~= Actor.DEAD and actor.Health > 0 then
			if actor.Team == self.aiTeam then
				if actor:IsInGroup("Brains") then
					aiBrain = actor;
				else
					table.insert(mine, actor);
				end
			else
				table.insert(enemies, actor);
			end
		end
	end

	-- Build the base the first time we get here
	if self.needBase then
		if aiBrain then
			self.needBase = false;
		else
			aiBrain = self:BuildBase(humanBrains);
			self.needBase = false;
			return;
		end
	end

	if aiBrain then
		self.aiBrainId = aiBrain.UniqueID;
	elseif self.aiBrainId then
		-- The commander is dead: the humans win
		self:Finish(false);
		return;
	else
		return;
	end

	self:SyncUnits(mine);

	-- Work out the situation we are in
	local enemyBrain = nil;
	local enemyBrainDist = math.huge;
	for _, brain in ipairs(humanBrains) do
		local d = Dist(aiBrain.Pos, brain.Pos);
		if d < enemyBrainDist then
			enemyBrain, enemyBrainDist = brain, d;
		end
	end
	local nearestEnemy, nearestEnemyDist = nil, math.huge;
	local threatsAtHome = 0;
	for _, enemy in ipairs(enemies) do
		local d = Dist(aiBrain.Pos, enemy.Pos);
		if d < nearestEnemyDist then
			nearestEnemy, nearestEnemyDist = enemy, d;
		end
		if d < HOME_RADIUS then
			threatsAtHome = threatsAtHome + 1;
		end
	end
	self.ctx = Mind.Context(#enemies, enemyBrain and enemyBrainDist or nearestEnemyDist);
	self.threatsAtHome = threatsAtHome;

	self:UpdateDefenseCycle(aiBrain, enemies, clock);
	self:EconomyAndSpawning(aiBrain, mine, dt, clock);
	self:UpdateSquads(aiBrain, enemies, enemyBrain, nearestEnemy, clock);
	self:TryLaunchSortie(aiBrain, mine, enemies, enemyBrain, clock);
	self:ManageDefenders(aiBrain, clock);
end

-- Keeps self.units in step with the actors that exist: adopts new ones, forgets dead ones.
function RoninLearningDefense:SyncUnits(mine)
	local present = {};
	for _, actor in ipairs(mine) do
		local id = actor.UniqueID;
		present[id] = true;
		if not self.units[id] then
			self.units[id] = {heavy = actor.Mass > 150, weapon = "Weapons - Primary", state = "defender"};
		end
	end
	for id, unit in pairs(self.units) do
		if not present[id] then
			if unit.state == "defender" or unit.state == "returning" then
				self.defHomeLosses = self.defHomeLosses + 1;
			end
			self.units[id] = nil;
		end
	end
end

----------------------------------------------------------------------------------------------------------------------
-- Base, economy, spawning
----------------------------------------------------------------------------------------------------------------------

-- Creates the commander far away from the human brain and a small garrison.
function RoninLearningDefense:BuildBase(humanBrains)
	local width = SceneMan.SceneWidth;
	local refX = width * 0.25;
	if humanBrains[1] then
		refX = humanBrains[1].Pos.X;
	end

	local x;
	if SceneMan.SceneWrapsX then
		x = (refX + width * 0.5) % width;
	else
		x = width - refX;
		if math.abs(x - refX) < width * 0.35 then
			x = refX < width * 0.5 and width - 250 or 250;
		end
		x = Clamp(x, 200, width - 200);
	end

	local ground = SceneMan:MovePointToGround(Vector(x, 0), 20, 5);
	local brain = RandomAHuman("Brains", self.techID);
	if not brain then
		ConsoleMan:PrintString("ERROR: Ronin Learning Defense could not create a Ronin commander (is Ronin.rte loaded?).");
		return false;
	end
	local brainPos = Vector(ground.X, ground.Y - 20);
	self:Equip(brain, "Weapons - Primary");
	brain:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.techID));
	brain.Team = self.aiTeam;
	brain.Pos = brainPos;
	brain.AIMode = Actor.AIMODE_SENTRY;
	self.aiBrainId = brain.UniqueID;
	MovableMan:AddActor(brain);

	for i = 1, 4 do
		self:SpawnUnit(brainPos, false);
	end
	self.manpower = 60;
	self.spawnTimer:Reset();
	self.sortieTimer:Reset();
	self.defCycleStart = self:Clock();
	self.defRatio = Mind.Select(self.ctx, "defense");
	self:Remember(self.ctx, "defense", self.defRatio);
	return true;
end

-- Puts a weapon from the given group into the actor's inventory. Returns the group that was actually used.
function RoninLearningDefense:Equip(actor, weaponGroup)
	local gun = RandomHDFirearm(weaponGroup, self.techID);
	if not gun then
		weaponGroup = "Weapons - Primary";
		gun = RandomHDFirearm(weaponGroup, self.techID);
	end
	if gun then
		actor:AddInventoryItem(gun);
	end
	return weaponGroup;
end

function RoninLearningDefense:PostPosition(brainPos)
	local x = brainPos.X + (math.random() * 2 - 1) * 260;
	return SceneMan:MovePointToGround(Vector(x, brainPos.Y - 150), 20, 5);
end

-- Creates one Ronin soldier at the base. The heavy/light mix and the weapon group are chosen by the learner.
function RoninLearningDefense:SpawnUnit(brainPos, payCost)
	local heavy = math.random() < Mind.Select(self.ctx, "comp");
	local actor = RandomAHuman(heavy and "Actors - Heavy" or "Actors - Light", self.techID);
	if not actor or actor.ModuleID ~= self.techID then
		actor = RandomAHuman("Actors", self.techID);
	end
	if not actor then
		return false;
	end

	local weapon = Mind.Select(self.ctx, "weapon");
	weapon = self:Equip(actor, weapon);
	if math.random() < 0.5 then
		actor:AddInventoryItem(RandomHDFirearm("Weapons - Secondary", self.techID));
	end
	local roll = math.random();
	if roll < 0.3 then
		actor:AddInventoryItem(RandomTDExplosive("Bombs - Grenades", self.techID));
	elseif roll < 0.5 then
		actor:AddInventoryItem(CreateHDFirearm("Medikit", "Base.rte"));
	end

	local post = self:PostPosition(brainPos);
	actor.Team = self.aiTeam;
	actor.Pos = post;
	actor.AIMode = Actor.AIMODE_SENTRY;

	-- The Lua object stops being valid once it is handed to MovableMan, so grab everything first
	local id = actor.UniqueID;
	self.units[id] = {heavy = heavy, weapon = weapon, state = "defender", post = post};
	MovableMan:AddActor(actor);
	if payCost then
		self.manpower = self.manpower - (heavy and HEAVY_COST or LIGHT_COST);
	end
	return true;
end

function RoninLearningDefense:EconomyAndSpawning(aiBrain, mine, dt, clock)
	local income = 1.6 + self.Difficulty * 0.03;
	self.manpower = math.min(MAX_MANPOWER, self.manpower + income * dt);
	local cap = 8 + math.floor(self.Difficulty / 8);
	if #mine < cap and self.manpower >= LIGHT_COST and self.spawnTimer:IsPastSimMS(3500) then
		if self:SpawnUnit(aiBrain.Pos, true) then
			self.spawnTimer:Reset();
		end
	end
end

----------------------------------------------------------------------------------------------------------------------
-- Defense: keep a share of the army at home, learn which share works
----------------------------------------------------------------------------------------------------------------------

function RoninLearningDefense:Remember(ctx, dimension, option)
	table.insert(self.matchDecisions, {ctx = ctx, dimension = dimension, option = option});
	if #self.matchDecisions > 80 then
		table.remove(self.matchDecisions, 1);
	end
end

function RoninLearningDefense:UpdateDefenseCycle(aiBrain, enemies, clock)
	-- Watch enemies that come close to the base
	for _, enemy in ipairs(enemies) do
		if Dist(aiBrain.Pos, enemy.Pos) < HOME_RADIUS then
			local id = enemy.UniqueID;
			local seen = self.defSeen[id];
			if not seen then
				self.defSeen[id] = {h0 = enemy.Health, hmin = enemy.Health, max = math.max(1, enemy.MaxHealth)};
			elseif enemy.Health < seen.hmin then
				seen.hmin = enemy.Health;
			end
		end
	end
	if not self.defBrainHealthStart then
		self.defBrainHealthStart = aiBrain.Health;
	end

	if clock - self.defCycleStart < DEFENSE_CYCLE_MS then
		return;
	end

	local damage, kills = 0, 0;
	for id, seen in pairs(self.defSeen) do
		local actor = LiveActorById(id);
		if not actor then
			kills = kills + 1;
			damage = damage + 1;
		else
			damage = damage + Clamp((seen.h0 - math.min(seen.hmin, actor.Health)) / seen.max, 0, 1);
		end
	end
	local brainLoss = Clamp((self.defBrainHealthStart - aiBrain.Health) / math.max(1, aiBrain.MaxHealth), 0, 1);
	local reward = 0.8 * damage + 0.4 * kills - 0.5 * self.defHomeLosses - 3 * brainLoss;
	reward = Clamp(reward, -2.5, 4) / 2.5;
	Mind.Learn(self.ctx, "defense", self.defRatio, reward, 1);

	-- Start the next cycle
	self.defSeen = {};
	self.defHomeLosses = 0;
	self.defBrainHealthStart = aiBrain.Health;
	self.defCycleStart = clock;
	self.defRatio = Mind.Select(self.ctx, "defense");
	self:Remember(self.ctx, "defense", self.defRatio);
end

-- Defenders stand guard around the commander; units coming back from a sortie walk to a post first.
function RoninLearningDefense:ManageDefenders(aiBrain, clock)
	for id, unit in pairs(self.units) do
		if unit.state == "returning" then
			local actor = LiveActorById(id);
			if actor then
				if Dist(actor.Pos, unit.post) < 90 or clock - (unit.returnStart or clock) > 25000 then
					actor.AIMode = Actor.AIMODE_SENTRY;
					unit.state = "defender";
				end
			end
		end
	end
end

----------------------------------------------------------------------------------------------------------------------
-- Sorties
----------------------------------------------------------------------------------------------------------------------

function RoninLearningDefense:SendUnit(actor, waypoints)
	actor:ClearAIWaypoints();
	actor.AIMode = Actor.AIMODE_GOTO;
	for _, waypoint in ipairs(waypoints) do
		actor:AddAISceneWaypoint(waypoint);
	end
end

-- Turns the target type chosen by the learner into waypoints.
function RoninLearningDefense:PlanWaypoints(targetType, aiBrain, enemies, enemyBrain, nearestEnemy)
	local targetPos = nil;
	local basePlan = targetType;
	if targetType == "flankBrain" then
		basePlan = "brain";
	elseif targetType == "flankNearest" then
		basePlan = "nearest";
	end

	if basePlan == "brain" then
		targetPos = enemyBrain and Vector(enemyBrain.Pos.X, enemyBrain.Pos.Y) or nil;
	elseif basePlan == "nearest" then
		targetPos = nearestEnemy and Vector(nearestEnemy.Pos.X, nearestEnemy.Pos.Y) or nil;
	elseif basePlan == "weakest" then
		local weakest, weakestHealth = nil, math.huge;
		for _, enemy in ipairs(enemies) do
			local near = Dist(aiBrain.Pos, enemy.Pos) < 2500;
			local health = enemy.Health + (near and 0 or 1000);
			if health < weakestHealth then
				weakest, weakestHealth = enemy, health;
			end
		end
		targetPos = weakest and Vector(weakest.Pos.X, weakest.Pos.Y) or nil;
	end

	if not targetPos then
		-- Fall back to anything that exists
		if enemyBrain then
			targetPos = Vector(enemyBrain.Pos.X, enemyBrain.Pos.Y);
		elseif nearestEnemy then
			targetPos = Vector(nearestEnemy.Pos.X, nearestEnemy.Pos.Y);
		else
			return nil;
		end
	end

	local waypoints = {};
	if targetType == "flankBrain" or targetType == "flankNearest" then
		-- Walk past the target and come back at it from the far side
		local direction = SceneMan:ShortestDistance(aiBrain.Pos, targetPos, false).X >= 0 and 1 or -1;
		local behind = SceneMan:MovePointToGround(Vector(targetPos.X + direction * 450, targetPos.Y - 200), 20, 5);
		table.insert(waypoints, behind);
	end
	table.insert(waypoints, targetPos);
	return waypoints;
end

function RoninLearningDefense:TryLaunchSortie(aiBrain, mine, enemies, enemyBrain, clock)
	if clock < FIRST_SORTIE_MS or #self.squads >= MAX_SQUADS or not self.sortieTimer:IsPastSimMS(SORTIE_COOLDOWN_MS) then
		return;
	end
	if #enemies == 0 and not enemyBrain then
		return;
	end

	-- How many soldiers may leave while the learned share stays at home
	local total, inSquads, ready = #mine, 0, {};
	for _, actor in ipairs(mine) do
		local unit = self.units[actor.UniqueID];
		if unit then
			if unit.state == "squad" then
				inSquads = inSquads + 1;
			elseif unit.state == "defender" then
				table.insert(ready, {actor = actor, unit = unit});
			end
		end
	end
	local keepHome = math.ceil(total * self.defRatio);
	local allowed = math.min(#ready, total - keepHome - inSquads);
	if allowed < 2 or self.threatsAtHome >= 3 then
		return;
	end

	local ctx = self.ctx;
	local targetType = Mind.Select(ctx, "target");
	local sizeOption = Mind.Select(ctx, "size");
	local retreat = Mind.Select(ctx, "retreat");
	local compOption = Mind.Select(ctx, "comp");
	local size = math.min(sizeOption, allowed);

	local nearestEnemy, nearestDist = nil, math.huge;
	for _, enemy in ipairs(enemies) do
		local d = Dist(aiBrain.Pos, enemy.Pos);
		if d < nearestDist then
			nearestEnemy, nearestDist = enemy, d;
		end
	end
	local waypoints = self:PlanWaypoints(targetType, aiBrain, enemies, enemyBrain, nearestEnemy);
	if not waypoints then
		return;
	end

	-- Pick the soldiers: the learned share of heavies first, then fill up
	local wantHeavy = math.floor(size * compOption + 0.5);
	local picked, heavyCount = {}, 0;
	for _, entry in ipairs(ready) do
		if #picked < size and entry.unit.heavy and heavyCount < wantHeavy then
			table.insert(picked, entry);
			entry.taken = true;
			heavyCount = heavyCount + 1;
		end
	end
	for _, entry in ipairs(ready) do
		if #picked < size and not entry.taken and (not entry.unit.heavy or heavyCount < wantHeavy) then
			table.insert(picked, entry);
			entry.taken = true;
		end
	end
	for _, entry in ipairs(ready) do
		if #picked < size and not entry.taken then
			table.insert(picked, entry);
			entry.taken = true;
		end
	end
	if #picked < 2 then
		return;
	end

	local squad = {
		id = self.nextSquadId,
		members = {},
		size = #picked,
		heavy = 0,
		weapons = {},
		targetType = targetType,
		retreat = retreat,
		ctx = ctx,
		defRatio = self.defRatio,
		startMS = clock,
		lastOrderMS = clock,
		state = "out",
		seen = {},
		waypoints = waypoints,
		currentTarget = waypoints[#waypoints],
		lost = 0,
	};
	self.nextSquadId = self.nextSquadId + 1;
	for _, entry in ipairs(picked) do
		local id = entry.actor.UniqueID;
		table.insert(squad.members, id);
		table.insert(squad.weapons, entry.unit.weapon);
		if entry.unit.heavy then
			squad.heavy = squad.heavy + 1;
		end
		entry.unit.state = "squad";
		entry.unit.squad = squad.id;
		self:SendUnit(entry.actor, waypoints);
	end
	table.insert(self.squads, squad);
	self.sortieTimer:Reset();

	self:Remember(ctx, "target", targetType);
	self:Remember(ctx, "size", NearestOption("size", squad.size));
	self:Remember(ctx, "retreat", retreat);
	self:Remember(ctx, "comp", NearestOption("comp", squad.heavy / squad.size));
end

function RoninLearningDefense:UpdateSquads(aiBrain, enemies, enemyBrain, nearestEnemy, clock)
	for index = #self.squads, 1, -1 do
		local squad = self.squads[index];
		local alive = {};
		local centroid = Vector(0, 0);
		for _, id in ipairs(squad.members) do
			local actor = LiveActorById(id);
			if actor then
				table.insert(alive, actor);
				centroid = centroid + actor.Pos;
			end
		end
		squad.lost = squad.size - #alive;

		if #alive == 0 then
			self:FinishSortie(index, "wiped out");
		else
			centroid = centroid / #alive;

			-- Remember the enemies this squad has been fighting
			for _, enemy in ipairs(enemies) do
				if Dist(centroid, enemy.Pos) < SQUAD_SIGHT_RADIUS then
					local id = enemy.UniqueID;
					local seen = squad.seen[id];
					if not seen then
						squad.seen[id] = {h0 = enemy.Health, hmin = enemy.Health, max = math.max(1, enemy.MaxHealth)};
					elseif enemy.Health < seen.hmin then
						seen.hmin = enemy.Health;
					end
				end
			end

			local elapsed = clock - squad.startMS;
			if squad.state == "out" then
				local survival = #alive / squad.size;
				local retreatNow = squad.retreat > 0 and survival <= squad.retreat;
				local recalled = self.threatsAtHome >= 3 and elapsed > 20000;
				if retreatNow or recalled or elapsed > SORTIE_MAX_MS then
					squad.state = "return";
					squad.returnStartMS = clock;
					squad.endReason = retreatNow and "retreated" or (recalled and "recalled" or "timed out");
					for _, actor in ipairs(alive) do
						self:SendUnit(actor, {Vector(aiBrain.Pos.X, aiBrain.Pos.Y)});
					end
				elseif Dist(centroid, squad.currentTarget) < 160 or clock - squad.lastOrderMS > 25000 then
					-- Arrived (or stuck): go for whatever enemy is closest now, else for the enemy brain
					local nextTarget = nil;
					local best = 1800;
					for _, enemy in ipairs(enemies) do
						local d = Dist(centroid, enemy.Pos);
						if d < best then
							best = d;
							nextTarget = Vector(enemy.Pos.X, enemy.Pos.Y);
						end
					end
					if not nextTarget and enemyBrain then
						nextTarget = Vector(enemyBrain.Pos.X, enemyBrain.Pos.Y);
					end
					if nextTarget then
						squad.currentTarget = nextTarget;
						for _, actor in ipairs(alive) do
							self:SendUnit(actor, {nextTarget});
						end
					end
					squad.lastOrderMS = clock;
				end
			else
				if Dist(centroid, aiBrain.Pos) < 220 or clock - squad.returnStartMS > SORTIE_RETURN_MAX_MS then
					self:FinishSortie(index, squad.endReason or "returned");
				end
			end
		end
	end
end

-- Scores a sortie and teaches the result to the learner.
function RoninLearningDefense:FinishSortie(index, reason)
	local squad = self.squads[index];
	table.remove(self.squads, index);

	local damage, kills = 0, 0;
	for id, seen in pairs(squad.seen) do
		local actor = LiveActorById(id);
		if not actor then
			kills = kills + 1;
			damage = damage + 1;
		else
			damage = damage + Clamp((seen.h0 - math.min(seen.hmin, actor.Health)) / seen.max, 0, 1);
		end
	end

	local lost = 0;
	for _, id in ipairs(squad.members) do
		if not LiveActorById(id) then
			lost = lost + 1;
		end
	end

	local reward;
	if damage < 0.05 and lost == 0 then
		if reason == "match ended" then
			return; -- the match was over before this squad met anybody: nothing to learn from
		end
		reward = -0.15; -- a walk in the park that achieved nothing
	else
		reward = 0.8 * damage + 0.4 * kills - 0.6 * lost;
	end
	reward = Clamp(reward, -2.5, 4) / 2.5;

	local ctx = squad.ctx;
	Mind.Learn(ctx, "target", squad.targetType, reward);
	Mind.Learn(ctx, "size", NearestOption("size", squad.size), reward);
	Mind.Learn(ctx, "retreat", squad.retreat, reward);
	Mind.Learn(ctx, "comp", NearestOption("comp", squad.heavy / squad.size), reward);
	Mind.Learn(ctx, "defense", squad.defRatio, reward, 0.5);
	for _, weapon in ipairs(squad.weapons) do
		Mind.Learn(ctx, "weapon", weapon, reward, 0.5);
	end
	Mind.data.sorties = Mind.data.sorties + 1;

	-- Survivors go back to guard duty
	local brain = self.aiBrainId and LiveActorById(self.aiBrainId) or nil;
	for _, id in ipairs(squad.members) do
		local unit = self.units[id];
		local actor = LiveActorById(id);
		if unit and actor and brain then
			unit.state = "returning";
			unit.squad = nil;
			unit.post = self:PostPosition(brain.Pos);
			unit.returnStart = self:Clock();
			self:SendUnit(actor, {unit.post});
		elseif unit then
			unit.state = "defender";
			unit.squad = nil;
		end
	end

	ConsoleMan:PrintString(string.format("Ronin AI sortie (%s, %s, %d men, retreat at %d%%): %s - damage %.1f, kills %d, lost %d, reward %+.2f",
		squad.ctx, squad.targetType, squad.size, squad.retreat * 100, reason, damage, kills, lost, reward));
	Mind.Save();
end

----------------------------------------------------------------------------------------------------------------------
-- End of match
----------------------------------------------------------------------------------------------------------------------

-- aiWon is true when the human brains are gone, false when the Ronin commander died.
function RoninLearningDefense:Finish(aiWon)
	if self.finished then
		return;
	end
	self.finished = true;

	-- Open sorties count with what they have achieved so far
	for index = #self.squads, 1, -1 do
		self:FinishSortie(index, "match ended");
	end

	-- The result of the whole match is a weak signal for every decision taken during it
	local result = aiWon and 1 or -1;
	for _, decision in ipairs(self.matchDecisions) do
		Mind.Learn(decision.ctx, decision.dimension, decision.option, result, 0.15);
	end
	local d = Mind.data;
	d.matches = d.matches + 1;
	if aiWon then
		d.wins = d.wins + 1;
	else
		d.losses = d.losses + 1;
	end
	Mind.Save();

	if aiWon then
		self.WinnerTeam = self.aiTeam;
	else
		for team in pairs(self.humanTeams) do
			self.WinnerTeam = team;
			break;
		end
	end
	ConsoleMan:PrintString(string.format("Ronin AI memory saved: %d matches, %d won, %d lost, %d sorties.", d.matches, d.wins, d.losses, d.sorties));
	ActivityMan:EndActivity();
end

-- Exposed so the learner can be tested outside of the game.
RoninLearningDefense.Mind = Mind;
