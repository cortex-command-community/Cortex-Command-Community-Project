--[[

	*** INSTRUCTIONS ***

	Create the LZmap object in the StartActivity() function with require:
		e.g. self.LZmap = require("Activities/Utility/LandingZoneMap")

	Initialize the internal data about enemies and LZ altitudes etc with a table of team numbers that are contolled by the AI and optionally whether to ignore the fog (defaults to false):
		e.g. self.LZmap:Initialize({self.CPUTeam}) or self.LZmap:Initialize({Activity.TEAM_3, Activity.TEAM_4})

	Call the update function from the UpdateActivity() function every update:
		e.g. self.LZmap:Update()


	Use the funcitons below to find Landing Zones for the AI-controlled team's dropships:

	self.LZmap:FindLZ(team, Destination, [digStrength])
		input: the team # that is looking for a LZ, a Destination vector (a location in the scene) and optionally the maximum material strength this path can dig through (default is 1)
		output: (1) a close LZ x-pos, (2) the highest obstacle along the path to destination, (3) a LZ x-pos with an easy path to the destination and (4) the highest obstacle along that path

	self.LZmap:FindBombTarget(team)
		input: the team # that is looking for a target to bomb
		output: the best LZ x-pos to bomb, or nil if no target was found

	self.LZmap:FindSafeLZ(team, OccupiedLZs)
		input: the team # that is looking for a LZ and an optional table of x-positions to avoid
		output: a LZ x-pos that is far away from enemy brains, or nil if no LZ was found

	self.LZmap:FindStartLZ(team, OccupiedLZs)
		** Very expensive! Only use once at the start of the activity **
		input: the team # that is looking for a LZ and an optional table of x-positions to avoid
		output: a LZ x-pos that is far away from enemy units, or nil if no LZ was found
]]

local LandingZoneMap = {};

function LandingZoneMap:Create(size)
	local Members = {};
	Members.updateMap = 0;
	Members.gridSize = size or 36;
	Members.offsetLZ = Members.gridSize * 2;
	Members.spacingLZ = Members.gridSize * 3;
	Members.widthLZ = Members.gridSize * 5;
	Members.terrainIndex = 0;
	Members.LZs = {};
	Members.LZLookup = {};
	Members.TerrainAlt = {};
	Members.EnemyTeamLOS = {};
	Members.NextTeamLOS = {};
	Members.BombTargets = {};
	Members.BombHistory = {};

	setmetatable(Members, self);
	self.__index = self;

	return Members;
end

function LandingZoneMap:Initialize(AIteams, ignoreFog)
	if not AIteams then
		ConsoleMan:PrintString("LandingZoneMap:Initialize takes a list of AI controlled teams as its argument.");
		return;
	end

	-- add active teams
	for _, team in pairs(AIteams) do
		self.EnemyTeamLOS[team] = {};
		self.NextTeamLOS[team] = {};
		self.BombTargets[team] = {};
	end

	if #AIteams == 1 then -- only one AI team avaliable
		self.teamAI = AIteams[1];
	end

	-- estimate terrain height
	local Pos = Vector(0, -1);
	while self.terrainIndex < math.floor(SceneMan.SceneWidth / self.gridSize) do
		Pos.X = self.terrainIndex * self.gridSize;
		self.TerrainAlt[self.terrainIndex] = SceneMan:FindAltitude(Pos, 0, 19);

		if self.TerrainAlt[self.terrainIndex-2] then -- interpolate
			self.TerrainAlt[self.terrainIndex-1] = (self.TerrainAlt[self.terrainIndex-2] + self.TerrainAlt[self.terrainIndex]) * 0.5;
		end

		self.terrainIndex = self.terrainIndex + 2; -- skip every second square to save time
	end

	self:UpdateAltitudeMap(); -- calculate the fitness for every LZ when every piece of terrain has been updated

	self.SceneActors = {};
	--self.SceneCraft = {};

	-- store all actors placed in the editor
	local residents = 0;
	for Act in MovableMan.AddedActors do
		if Act.ClassName ~= "ADoor" then
			if Act.ClassName == "ACRocket" or Act.ClassName == "ACDropShip" then
				--table.insert(self.SceneCraft, Act);
			else
				table.insert(self.SceneActors, Act);
				residents = residents + 1;
			end
		end
	end

	self.UpdateEnemyData = coroutine.create(self.UpdateEnemies); -- gathers info about all enemy actors

	-- always ignore the fog when looking for the brain LZ
	self.ignoreFog = true;

	-- analyze all enemies (three calls per enemy plus one call to sum up the results)
	for _ = 1, residents*3+1 do
		local _, err = coroutine.resume(self.UpdateEnemyData, self);
		if err then
			ConsoleMan:PrintString("UpdateEnemies error: " .. err); -- print the error message
			self.UpdateEnemyData = coroutine.create(self.UpdateEnemies);
			break;
		end
	end

	self.ignoreFog = ignoreFog or false;
end


function LandingZoneMap:Update()
	self.updateMap = not self.updateMap;
	if self.updateMap then
		self:UpdateAltitudeMap();
	else
		local _, err = coroutine.resume(self.UpdateEnemyData, self);
		if err then
			ConsoleMan:PrintString("UpdateEnemies error: " .. err); -- print the error message
			self.UpdateEnemyData = coroutine.create(self.UpdateEnemies);
		end
	end
end


function LandingZoneMap.GetNeighborHeightValue(self, index, offset)
	if self.TerrainAlt[index+offset] then
		local height = self.TerrainAlt[index+offset];
		if height < 100 or height >= SceneMan.SceneHeight then
			return -50000; -- cannot land here
		end

		return height;
	elseif offset > 0 then
		offset = offset - 1;
		return self.GetNeighborHeightValue(self, 0, offset);
	elseif offset < 0 then
		offset = offset + 1;
		return self.GetNeighborHeightValue(self, #self.TerrainAlt, offset);
	end
end


function LandingZoneMap.GetNeighborHeightValueNoWrap(self, index, offset)
	if self.TerrainAlt[index+offset] then
		local height = self.TerrainAlt[index+offset];
		if height < 100 or height >= SceneMan.SceneHeight then
			return -50000; -- cannot land here
		end

		return height;
	end

	return -50000;
end

-- update a single terrain value every time this function is called
function LandingZoneMap:UpdateAltitudeMap()
	-- measure the altitude for a single piece of terrain
	if self.terrainIndex < math.floor(SceneMan.SceneWidth/self.gridSize) then
		self.TerrainAlt[self.terrainIndex] = SceneMan:FindAltitude(Vector(self.terrainIndex*self.gridSize, -1), 0, 9);
		self.terrainIndex = self.terrainIndex + 1;
	else	-- calculate the fitness for every LZ when every piece of terrain has been updated
		self.terrainIndex = 0;

		local GetNeighborVal;
		if SceneMan.SceneWrapsX then
			GetNeighborVal = self.GetNeighborHeightValue;
		else
			GetNeighborVal = self.GetNeighborHeightValueNoWrap;
		end

		local value, altitude;
		local minVal = -1;
		local indexLZ = 1;
		for i = 2, #self.TerrainAlt, 3 do
			altitude = self.TerrainAlt[i];
			if altitude >= SceneMan.SceneHeight or altitude < 100 then
				-- there is no room to land here
				self.LZs[indexLZ] = nil;
			else
				value = 0;
				for offset = -2, -1 do	-- difference in altitude from terrain pieces to the left
					value = value - math.abs(altitude - GetNeighborVal(self, i, offset));
				end

				for offset = 1, 2 do	-- difference in altitude from terrain pieces to the right
					value = value - math.abs(altitude - GetNeighborVal(self, i, offset));
				end

				if self.LZs[indexLZ] then
					self.LZs[indexLZ].X = i * self.gridSize;
					self.LZs[indexLZ].Y = altitude;
					self.LZs[indexLZ].value = value;
				else
					self.LZs[indexLZ] = {X=i*self.gridSize, Y=altitude, value=value};
				end

				if value < minVal and value > -500 then
					minVal = value;
				end

				self.LZLookup[self.LZs[indexLZ].X] = indexLZ;
				indexLZ = indexLZ + 1;
			end
		end

		-- delete any left over data after the last LZ in the table (just in case the # of LZs have shrunk)
		while self.LZs[indexLZ] do
			self.LZLookup[self.LZs[indexLZ].X] = nil;
			self.LZs[indexLZ] = nil;
			indexLZ = indexLZ + 1;
		end

		-- normalize the score
		for k, DataLZ in pairs(self.LZs) do
			self.LZs[k].value = 1 - DataLZ.value / minVal;
		end
	end
end

-- estimate LOS for enemy actors
function LandingZoneMap.UpdateEnemies(self)
	while true do
		-- store all actors in tables
		if not self.SceneActors then
			self.SceneActors = {};
			--self.SceneCraft = {};

			if self.teamAI then
				-- only one AI team, ignore all AI actors
				for Act in MovableMan.Actors do
					if Act.ClassName ~= "ADoor" then
						if Act.ClassName == "ACRocket" or Act.ClassName == "ACDropShip" then
							--table.insert(self.SceneCraft, Act);
						elseif Act.Team ~= self.teamAI then
							table.insert(self.SceneActors, Act);
						end
					end
				end
			else
				for Act in MovableMan.Actors do
					if Act.ClassName ~= "ADoor" then
						if Act.ClassName == "ACRocket" or Act.ClassName == "ACDropShip" then
							--table.insert(self.SceneCraft, Act);
						else
							table.insert(self.SceneActors, Act);
						end
					end
				end
			end
		else
			-- analyze one actor per update
			local Act = table.remove(self.SceneActors);
			if not Act then	-- no actors left to analyze
				self.SceneActors = nil;

				-- activate the updated LOS table
				for k, _ in pairs(self.LZs) do
					for team, _ in pairs(self.EnemyTeamLOS) do
						self.NextTeamLOS[team][k] = (self.NextTeamLOS[team][k] or 0) * 0.5;
						self.EnemyTeamLOS[team][k] = (self.EnemyTeamLOS[team][k] or 0) * 0.5 + self.NextTeamLOS[team][k];
					end
				end

				-- slowly reduce activity in the BombTargets table since actors will move around, die etc.
				local Prune = {};
				for team, _ in pairs(self.BombTargets) do
					for x, value in pairs(self.BombTargets[team]) do
						if value > 0.1 then
							self.BombTargets[team][x] = value * 0.85;
						else
							table.insert(Prune, {team=team, x=x}); -- remove this entry later
						end
					end
				end

				for _, Data in pairs(Prune) do
					self.BombTargets[Data.team][Data.x] = nil;
				end
			elseif MovableMan:IsActor(Act) and not Act:IsDead() and Act.Vel.Largest < 12 and not(self.teamAI and Act.Team == self.teamAI) then
				-- ignore this actor if it is on the only AI team, if it is moving to fast or it is dead

				-- check which team can see this actor
				local VisibleToTeam = {};
				for team, _ in pairs(self.EnemyTeamLOS) do
					if Act.Team ~= team and (self.ignoreFog or not SceneMan:IsUnseen(Act.Pos.X, Act.Pos.Y, team)) then
						table.insert(VisibleToTeam, team);
					end
				end

				if #VisibleToTeam > 0 then
					-- check if the actor is on the surface
					if Act.Vel.Largest < 3 then
						local index = math.floor(Act.Pos.X / self.gridSize);
						if self.TerrainAlt[index] and self.TerrainAlt[index] > 400 then	-- the altitude is known to be suitable for bombing
							if math.abs(self.TerrainAlt[index] - Act.Pos.Y) < Act.Height*0.3 then	-- the actor is close to the ground
								local x = index * self.gridSize;
								for _, team in pairs(VisibleToTeam) do
									if Act.Team ~= team then
										local score = self.BombTargets[team][x] or 1;
										if score < 10 then
											self.BombTargets[team][x] = score * 1.25; -- mark this terrain piece as a potential target for bombing
										end
									end
								end
							end
						end
					end

					local viewRange = FrameMan.PlayerScreenWidth * 0.5 + Act.AimDistance + 100; -- DropShip radius is ~100
					if Act.EquippedItem then
						viewRange = viewRange + Act.EquippedItem.SharpLength; -- add the SharpLength of any weapon
					end

					viewRange = math.ceil(viewRange/self.offsetLZ) * self.offsetLZ; -- round

					-- stay far away from AA-units
					if Act:HasObjectInGroup("Anti-Air") then
						viewRange = math.max(1100, viewRange);
					end

					-- cast rays to the right and left of the actor
					local actorTeam = Act.IgnoresWhichTeam;
					local Origin = Vector(Act.EyePos.X, Act.EyePos.Y);
					local Free = Vector();
					local SeePos, Dist, mag, pixels;
					local noise = RangeRand(0, 0.2); -- cast the rays in sligtly different angles every time
					local start_ang = {0+noise, 2.356-noise};
					local end_ang = {0.786+noise, 3.142-noise};
					for i = 1, 2 do
						for ang = start_ang[i], end_ang[i], 0.785 do
							pixels = SceneMan:CastObstacleRay(Origin, Vector(viewRange*math.cos(ang), -viewRange*math.sin(ang)), Vector(), Free, 255, actorTeam, rte.grassID, 18);
							if pixels < 0 or pixels > self.gridSize then
								Dist = SceneMan:ShortestDistance(Origin, Free, false);
								mag = Dist.Magnitude;

								for range = self.gridSize, mag, self.offsetLZ do
									SeePos = Origin + Dist * (range / mag);

									-- assign a score to this XY-position, lower means worse LZ
									local lzX = self:PosToClosestLZ(SeePos.X);
									if lzX then
										local lzIndex = self.LZLookup[lzX];
										if lzIndex and self.LZs[lzIndex] and SeePos.Y <= self.LZs[lzIndex].Y then
											-- write LOS data to a temporary table
											for _, team in pairs(VisibleToTeam) do
												if actorTeam ~= team then
													self.NextTeamLOS[team][lzIndex] = (self.NextTeamLOS[team][lzIndex] or 0) + 15;
												end
											end
										end
									end
								end
							end
						end

						coroutine.yield(); -- wait until next frame
						Act = nil; -- this pointer is no longer safe to access
					end

					-- cast a ray upwards
					pixels = SceneMan:CastObstacleRay(Origin, Vector(0, -self.spacingLZ), Vector(), Free, rte.NoMOID, actorTeam, rte.grassID, 14);
					if pixels < 0 or pixels > self.gridSize then
						local lz1, lz2 = self:PosToLZs(Origin.X);
						if lz1 then
							Dist = SceneMan:ShortestDistance(Origin, Free, false);
							mag = Dist.Largest;
							for range = self.gridSize, mag, self.offsetLZ do
								local y = Origin.Y + Dist.Y * (range / mag);

								-- assign a score to this XY-position, lower means worse LZ
								if lz2 then
									-- this point belongs to two LZs
									local lzIndex = self.LZLookup[lz1];
									if lzIndex and self.LZs[lzIndex] and y <= self.LZs[lzIndex].Y then
										-- write LOS data to a temporary table
										for _, team in pairs(VisibleToTeam) do
											if actorTeam ~= team then
												self.NextTeamLOS[team][lzIndex] = (self.NextTeamLOS[team][lzIndex] or 0) + 15;
											end
										end
									end

									lzIndex = self.LZLookup[lz2];
									if lzIndex and self.LZs[lzIndex] and y <= self.LZs[lzIndex].Y then
										-- write LOS data to a temporary table
										for _, team in pairs(VisibleToTeam) do
											if actorTeam ~= team then
												self.NextTeamLOS[team][lzIndex] = (self.NextTeamLOS[team][lzIndex] or 0) + 15;
											end
										end
									end
								else
									local lzIndex = self.LZLookup[lz1];
									if lzIndex and self.LZs[lzIndex] and y <= self.LZs[lzIndex].Y then
										-- write LOS data to a temporary table
										for _, team in pairs(VisibleToTeam) do
											if actorTeam ~= team then
												self.NextTeamLOS[team][lzIndex] = (self.NextTeamLOS[team][lzIndex] or 0) + 15;
											end
										end
									end
								end
							end
						end
					end
				end
			end
		end

		coroutine.yield(); -- wait until next frame
	end
end

-- Estimate the distance
function LandingZoneMap:SurfaceProximity(Pos)
	local y = self.TerrainAlt[math.floor(Pos.X/self.gridSize)];
	if y then
		return math.abs(Pos.Y - y);
	end

	return SceneMan.SceneHeight;
end

-- Evaluates one LZ every time this function is called. Returns two LZs when the search is done.
function LandingZoneMap:FindLZ(team, Destination, digStrenght)
	if self.LZSearchRoutine then
		local msg, close_x, close_obst, path_x, path_obst = coroutine.resume(self.LZSearchRoutine, self, team, Destination, (digStrenght or 1));
		if not msg then
			self.LZSearchRoutine = nil;
			ConsoleMan:PrintString("Error in LandingZoneMap.SearchForLZ: "..close_x); -- print the error message
		elseif close_x then
			self.LZSearchRoutine = nil;
			return close_x, close_obst, path_x, path_obst;
		end
	else
		self.LZSearchRoutine = coroutine.create(LandingZoneMap.SearchForLZ);
	end
end

-- A coroutine that search for a LZ. Input is team and destination. Output is LZ x-pos and obstacle height for two LZs, one close and one with an easy path
function LandingZoneMap.SearchForLZ(self, team, Destination, digStrenght)
	-- estimate how visible our descent is to the enemy
	local LOSgrid = self.EnemyTeamLOS[team];
	local GoodLZs = {};
	local totalLZs = 0;
	for k, LZ in pairs(self.LZs) do
		-- give LZs that are inside the fog a worse score
		local score = LZ.value * 9;
		if not self.ignoreFog and SceneMan:IsUnseen(LZ.X, LZ.Y, team) then
			score = score - 100;
		end

		table.insert(GoodLZs, {X=LZ.X, Y=LZ.Y, score=score-(LOSgrid[k] or 0)});
		totalLZs = totalLZs + 1;
	end

	-- avoid existing craft
	self:AddCraftScore(GoodLZs, self.LZLookup);

	-- remove the worst half of the LZs
	table.sort(GoodLZs, function(A, B) return A.score > B.score end); -- the best LZ first
	if totalLZs > 7 then
		local limit = math.max(math.floor(math.floor(SceneMan.SceneWidth/self.spacingLZ)*0.5), 1);
		while totalLZs > limit do
			table.remove(GoodLZs);
			totalLZs = totalLZs - 1;
		end
	end

	local pathRequestsCompleted = 0;
	local completedPathRequests = {};

	-- measure the distance to the destination
	for k, LZ in pairs(GoodLZs) do
		SceneMan.Scene:CalculatePathAsync(
			function(pathRequest)
				pathRequestsCompleted = pathRequestsCompleted + 1;
				completedPathRequests[k] = pathRequest;
			end, 
			Vector(LZ.X, LZ.Y), Destination, false, digStrenght, team);
	end

	while pathRequestsCompleted < #GoodLZs do
		coroutine.yield(); -- wait until all paths are complete
	end

	for k, LZ in pairs(GoodLZs) do
		pathRequest = completedPathRequests[k];
		if pathRequest.PathLength > -1 then
			local NextWpt, PrevWpt, deltaY;
			local height = 0;
			local pathObstMaxHeight = 0;

			for Wpt in pathRequest.Path do
				NextWpt = SceneMan:MovePointToGround(Wpt, 20, 12);

				if PrevWpt then
					deltaY = PrevWpt.Y - NextWpt.Y;
					if deltaY > 20 then	-- Wpt is more than n pixels above PrevWpt in the scene
						if (deltaY / math.abs(SceneMan:ShortestDistance(PrevWpt, NextWpt, false).X+0.1)) > 1 then	-- the slope is more than 45 degrees
							height = height + (PrevWpt.Y - NextWpt.Y);
							pathObstMaxHeight = math.max(pathObstMaxHeight, height);
						else
							height = 0;
						end
					else
						height = 0;
					end
				end

				PrevWpt = NextWpt;
			end

			GoodLZs[k].terrainScore = LZ.score;
			GoodLZs[k].pathLength = pathRequest.PathLength;
			GoodLZs[k].pathObstMaxHeight = pathObstMaxHeight;
			GoodLZs[k].score = LZ.score - (pathRequest.PathLength * 0.2 + math.floor(pathObstMaxHeight/15) * 12); -- recalculate the score so we can find a safe LZ that has an easy path to the destination
		else
			-- unknown path
			GoodLZs[k].terrainScore = LZ.score;
			GoodLZs[k].pathLength = 200;
			GoodLZs[k].pathObstMaxHeight = 200;
			GoodLZs[k].score = LZ.score - 100;
		end
	end

	table.sort(GoodLZs, function(A, B) return A.score > B.score end); -- the best LZ first
	local MobilityLZ, selected_index = self:SelectLZ(GoodLZs, 12);
	if selected_index then
		table.remove(GoodLZs, selected_index); -- don't select this LZ again
	end

	-- recalculate the score so we can find a safe LZ that is close to the destination
	for k, LZ in pairs(GoodLZs) do
		GoodLZs[k].score = LZ.terrainScore - (LZ.pathLength * 0.7 + math.floor(LZ.pathObstMaxHeight/20) * 8);
	end

	table.sort(GoodLZs, function(A, B) return A.score > B.score end); -- the best LZ first
	local CloseLZ = self:SelectLZ(GoodLZs, 10);

	return MobilityLZ.X, MobilityLZ.pathObstMaxHeight, CloseLZ.X, CloseLZ.pathObstMaxHeight;
end

-- input: the team # that is looking for a target to bomb
function LandingZoneMap:FindBombTarget(team)
	local minimumDistanceRequiredFromEnemyBrain = 500;

	local enemyBrainPositions = {};
	local gameActivity = ActivityMan:GetActivity();
	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if gameActivity:PlayerActive(player) and gameActivity:GetTeamOfPlayer(player) ~= team then
			local brain = gameActivity:GetPlayerBrain(player);
			if brain and MovableMan:IsActor(brain) and brain.Team ~= team then
				table.insert(enemyBrainPositions, brain.Pos.X);
			end
		end
	end

	local bombTargets = {};
	for x, score in pairs(self.BombTargets[team]) do
		if score > 1.5 then
			for _, enemyBrainPositionX in pairs(enemyBrainPositions) do
				if SceneMan:ShortestDistance(Vector(x, 0), Vector(enemyBrainPositionX, 0), SceneMan.SceneWrapsX):MagnitudeIsGreaterThan(minimumDistanceRequiredFromEnemyBrain) then
					if self.BombHistory[x] then
						table.insert(bombTargets, {score=score+self.BombHistory[x], X=x});
					else
						table.insert(bombTargets, {score=score, X=x});
					end
				end
			end
		end
	end

	for x, value in pairs(self.BombHistory) do
		self.BombHistory[x] = value * 0.7;
	end

	--TODO don't pick an LZ within some reasonable distance of a brain

	if #bombTargets > 0 then
		-- pick one of the best LZs
		local TargetLZ = self:SelectLZ(bombTargets, 5);
		if TargetLZ then
			self.BombHistory[TargetLZ.X] = -2;	-- punish this position in the future so we don't bomb the same place again right away
			return TargetLZ.X;
		end
	end
end

-- input: the team # that is looking for a LZ
function LandingZoneMap:FindStartLZ(team, OccupiedLZs)
	local LOSgrid = self.EnemyTeamLOS[team];

	-- store enemy actor locations
	local EnemyLocations = {};
	for Act in MovableMan.Actors do
		if Act.Team ~= team and (Act.ClassName == "Actor" or Act.ClassName == "ACrab" or Act.ClassName == "AHuman") then
			table.insert(EnemyLocations, Vector(Act.Pos.X, Act.Pos.Y));
		end
	end

	for Act in MovableMan.AddedActors do
		if Act.Team ~= team and (Act.ClassName == "Actor" or Act.ClassName == "ACrab" or Act.ClassName == "AHuman") then
			table.insert(EnemyLocations, Vector(Act.Pos.X, Act.Pos.Y));
		end
	end

	if OccupiedLZs then
		for _, x in pairs(OccupiedLZs) do
			local y = self.TerrainAlt[math.floor(x/self.gridSize)] or SceneMan.SceneHeight * 0.5;
			table.insert(EnemyLocations, Vector(x, y));
		end
	end

	-- estimate how visible our descent is to the enemy
	local GoodLZs = {};
	for k, LZ in pairs(self.LZs) do
		local tmp = LZ.value * 0.5 - (LOSgrid[k] or 0);
		table.insert(GoodLZs, {X=LZ.X, Y=LZ.Y, score=tmp});
	end

	-- avoid existing craft
	self:AddCraftScore(GoodLZs, self.LZLookup);

	table.sort(GoodLZs, function(A, B) return A.score > B.score end); -- the best LZs first
	while #GoodLZs > 12 do
		table.remove(GoodLZs); -- keep the n best LZs
	end

	-- calculate the distance from the best LZs to all enemy actors
	local distance;
	local bestProxScore = 1;
	for k, LZ in pairs(GoodLZs) do
		distance = nil;
		local PosLZ = Vector(LZ.X, LZ.Y);
		for _, PosEnemy in pairs(EnemyLocations) do
			local wpts = SceneMan.Scene:CalculatePath(PosEnemy, PosLZ, false, 1, team);
			if wpts > -1 then
				if distance then
					distance = math.min(wpts, distance);
				else
					distance = wpts;
				end
			end
		end

		if distance then
			GoodLZs[k].prox = distance;
		else
			local Dist = SceneMan:ShortestDistance(PosLZ, PosEnemy, false);
			GoodLZs[k].prox = math.floor((math.abs(Dist.X) + Dist.Magnitude * 0.2)/20);
		end

		bestProxScore = math.max(distance, bestProxScore);
	end

	-- add the proximity to the enemy actors to the score
	for k, LZ in pairs(GoodLZs) do
		GoodLZs[k].score = LZ.score + 2 * (LZ.prox / bestProxScore)^2; -- normalize the proximity score
	end

	table.sort(GoodLZs, function(A, B) return A.score > B.score end); -- the best LZs first

	local TargetLZ = self:SelectLZ(GoodLZs, 7);
	if TargetLZ then
		return TargetLZ.X;
	end
end

-- input: the team # that is looking for a LZ
function LandingZoneMap:FindSafeLZ(team, OccupiedLZs)
	local LOSgrid = self.EnemyTeamLOS[team];

	-- store brain locations
	local BrainLocations = {};
	local GmActiv = ActivityMan:GetActivity();
	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if GmActiv:PlayerActive(player) and GmActiv:GetTeamOfPlayer(player) ~= team then
			local Brain = GmActiv:GetPlayerBrain(player);
			if Brain and MovableMan:IsActor(Brain) then
				table.insert(BrainLocations, Vector(Brain.Pos.X, Brain.Pos.Y));
			end
		end
	end

	if OccupiedLZs then
		for _, x in pairs(OccupiedLZs) do
			local y = self.TerrainAlt[math.floor(x/self.gridSize)] or SceneMan.SceneHeight * 0.5;
			table.insert(BrainLocations, Vector(x, y));
		end
	end

	-- estimate the distance from the LZs to all enemy brains
	local score;
	local bestProxScore = 1;
	local BrainProxScore = {};
	for k, LZ in pairs(self.LZs) do
		score = nil;
		local PosLZ = Vector(LZ.X, LZ.Y);
		for _, PosBrain in pairs(BrainLocations) do
			local Dist = SceneMan:ShortestDistance(PosLZ, PosBrain, false);
			if score then
				score = math.min(score, math.abs(Dist.X)+Dist.Magnitude*0.2);
			else
				score = math.abs(Dist.X) + Dist.Magnitude * 0.2;
			end
		end

		if score then
			BrainProxScore[k] = score;
			bestProxScore = math.max(score, bestProxScore);
		else
			BrainProxScore[k] = 1;
		end
	end

	-- estimate how visible our descent is to the enemy
	local GoodLZs = {};
	for k, LZ in pairs(self.LZs) do
		local proximityBias = 2 * (BrainProxScore[k] / bestProxScore)^2; -- normalize the brain proximity
		table.insert(GoodLZs, {X=LZ.X, Y=LZ.Y, score=LZ.value*0.5-(LOSgrid[k] or 0)*3+proximityBias});
	end

	-- avoid existing craft
	self:AddCraftScore(GoodLZs, self.LZLookup);

	local TempLZs = {};
	table.sort(GoodLZs, function(A, B) return A.score < B.score end); -- the best LZ last

	for i = 1, 20 do	-- pick one of the n best LZs
		local LZ = table.remove(GoodLZs);
		if LZ then
			TempLZs[i] = LZ; -- the best LZ first
		else
			break;
		end
	end

	local TargetLZ = self:SelectLZ(TempLZs, 7);
	if TargetLZ then
		return TargetLZ.X;
	end
end

-- reduce the score around any existing craft to avoid collisions
function LandingZoneMap:AddCraftScore(LZs, LZlookup)
	local x_pos, tmp_pos;
	local SceneCraft = {};

	for Act in MovableMan.AddedActors do
		if Act.ClassName == "ACRocket" or Act.ClassName == "ACDropShip" then
			table.insert(SceneCraft, Act);
		end
	end

	for Act in MovableMan.Actors do
		if Act.ClassName == "ACRocket" or Act.ClassName == "ACDropShip" then
			table.insert(SceneCraft, Act);
		end
	end

	for _, Craft in pairs(SceneCraft) do
		if MovableMan:ValidMO(Craft) then
			x_pos = self:PosToClosestLZ(Craft.Pos.X); -- align the craft position with a LZ
			if x_pos then
				if x_pos >= SceneMan.SceneWidth then
					if SceneMan.SceneWrapsX then
						x_pos = self.offsetLZ;
					else
						x_pos = x_pos - self.spacingLZ;
					end
				end

				if LZlookup[x_pos] and LZs[LZlookup[x_pos]] then
					LZs[LZlookup[x_pos]].score = LZs[LZlookup[x_pos]].score - 300;

					-- left side
					tmp_pos = x_pos - self.widthLZ;
					if LZs[LZlookup[tmp_pos]] then
						LZs[LZlookup[tmp_pos]].score = LZs[LZlookup[tmp_pos]].score - 150;
						if Craft.Diameter > self.widthLZ then
							tmp_pos = x_pos - self.widthLZ*2;
							if LZs[LZlookup[tmp_pos]] then
								LZs[LZlookup[tmp_pos]].score = LZs[LZlookup[tmp_pos]].score - 100;
							elseif LZs[#LZs] then
								LZs[#LZs].score = LZs[#LZs].score - 100;
							end
						end
					else
						tmp_pos = #LZs;
						LZs[tmp_pos].score = LZs[tmp_pos].score - 150;
						if Craft.Diameter > self.widthLZ then
							tmp_pos = tmp_pos - 1;
							if LZs[tmp_pos] then
								LZs[tmp_pos].score = LZs[tmp_pos].score - 100;
							end
						end
					end

					-- right side
					tmp_pos = x_pos + self.widthLZ;
					if LZs[LZlookup[tmp_pos]] then
						LZs[LZlookup[tmp_pos]].score = LZs[LZlookup[tmp_pos]].score - 150;
						if Craft.Diameter > self.widthLZ then
							tmp_pos = x_pos + self.widthLZ*2;
							if LZs[LZlookup[tmp_pos]] then
								LZs[LZlookup[tmp_pos]].score = LZs[LZlookup[tmp_pos]].score - 100;
							elseif LZs[1] then
								LZs[1].score = LZs[1].score - 100;
							end
						end
					elseif LZs[1] then
						LZs[1].score = LZs[1].score - 150;
						if Craft.Diameter > self.widthLZ and LZs[2] then
							LZs[2].score = LZs[2].score - 100;
						end
					end
				end
			end
		end
	end
end


-- pick the LZ semi-randomly, with a larger probablility for LZs with a higher score
function LandingZoneMap:SelectLZ(LZs, temperature)
	if #LZs > 1 then
		local temp = temperature or 10; -- a higher temperature means less random selection
		local sum = 0;
		local bestScore = LZs[1].score;
		local worstScore = LZs[#LZs].score;

		-- normalize the score
		for i, LZ in pairs(LZs) do
			LZs[i].chance = temp * ((LZ.score - worstScore) / (bestScore - worstScore));
			sum = sum + math.exp(LZs[i].chance);
		end

		-- use Softmax to pick one of the n best LZs
		if sum > 0 then
			local pick = math.random() * sum;
			sum = 0;
			for k, LZ in pairs(LZs) do
				sum = sum + math.exp(LZ.chance);
				if sum >= pick then
					return LZ, k;
				end
			end
		else
			return LZs[1], 1;
		end
	elseif #LZs == 1 then
		return LZs[1], 1;
	end
end

function LandingZoneMap:PosToClosestLZ(x)
	if x < 0 then
		if SceneMan.SceneWrapsX then
			x = x + SceneMan.SceneWidth;
		else
			return;
		end
	elseif x >= SceneMan.SceneWidth then
		if SceneMan.SceneWrapsX then
			x = x - SceneMan.SceneWidth;
		else
			return;
		end
	end

	return math.floor(math.max(x-self.offsetLZ, 0)/self.spacingLZ+0.5) * self.spacingLZ + self.offsetLZ;
end

function LandingZoneMap:PosToLZs(x)
	if x < 0 then
		if SceneMan.SceneWrapsX then
			x = x + SceneMan.SceneWidth;
		else
			return;
		end
	elseif x >= SceneMan.SceneWidth then
		if SceneMan.SceneWrapsX then
			x = x - SceneMan.SceneWidth;
		else
			return;
		end
	end

	local new_x = math.max(x-self.offsetLZ, 0);
	local lz1 = math.floor(new_x/self.spacingLZ) * self.spacingLZ + self.offsetLZ;
	local lz2 = math.ceil(new_x/self.spacingLZ) * self.spacingLZ + self.offsetLZ;
	if lz1 ~= lz2 then
		return lz1, lz2;
	end

	return lz1;
end

return LandingZoneMap:Create();