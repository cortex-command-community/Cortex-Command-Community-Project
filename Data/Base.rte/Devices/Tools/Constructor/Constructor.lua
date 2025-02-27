function ConstructorWrapPos(checkPos)
	if SceneMan.SceneWrapsX then
		if checkPos.X > SceneMan.SceneWidth then
			checkPos = Vector(checkPos.X - SceneMan.SceneWidth, checkPos.Y);
		elseif checkPos.X < 0 then
			checkPos = Vector(SceneMan.SceneWidth + checkPos.X, checkPos.Y);
		end
	end

	return checkPos;
end

-- recursive flood filling function
function ConstructorFloodFill(x, y, startnum, maxnum, array, realposition, realspacing)
	array[x][y] = startnum;

	if startnum < maxnum then
		if array[x + 1][y] == -1 or array[x + 1][y] > startnum then
			local checkPos = ConstructorWrapPos(realposition + Vector(realspacing, 0));
			if SceneMan:GetTerrMatter(checkPos.X + (realspacing * 0.5), checkPos.Y + (realspacing * 0.5)) == rte.airID then
				ConstructorFloodFill(x + 1, y, startnum + 1, maxnum, array, checkPos, realspacing);
			end
		end

		if array[x - 1][y] == -1 or array[x - 1][y] > startnum then
			local checkPos = ConstructorWrapPos(realposition + Vector(-realspacing, 0));
			if SceneMan:GetTerrMatter(checkPos.X + (realspacing * 0.5), checkPos.Y + (realspacing * 0.5)) == rte.airID then
				ConstructorFloodFill(x - 1, y, startnum + 1, maxnum, array, checkPos, realspacing);
			end
		end

		if array[x][y + 1] == -1 or array[x][y + 1] > startnum then
			local checkPos = ConstructorWrapPos(realposition + Vector(0, realspacing));
			if SceneMan:GetTerrMatter(checkPos.X + (realspacing * 0.5), checkPos.Y + (realspacing * 0.5)) == rte.airID then
				ConstructorFloodFill(x, y + 1, startnum + 1, maxnum, array, checkPos, realspacing);
			end
		end

		if array[x][y - 1] == -1 or array[x][y - 1] > startnum then
			local checkPos = ConstructorWrapPos(realposition + Vector(0, -realspacing));
			if SceneMan:GetTerrMatter(checkPos.X + (realspacing * 0.5), checkPos.Y + (realspacing * 0.5)) == rte.airID then
				ConstructorFloodFill(x, y - 1, startnum + 1, maxnum, array, checkPos, realspacing);
			end
		end
	end
end

--TODO: Figure out how to snap different sizes properly
function ConstructorSnapPos(checkPos, blockSize)
	return Vector(math.floor((checkPos.X)/blockSize) * blockSize, math.floor((checkPos.Y)/blockSize) * blockSize);
end

function ConstructorTerrainRay(start, trace, skip)
	local hitPos = start + trace;
	SceneMan:CastStrengthRay(start, trace, 0, hitPos, skip, rte.airID, SceneMan.SceneWrapsX);	
	return hitPos;
end

function ConstructorIsInPlan(buildPlan, xPos, yPos)
	for blockSize, blockPlan in pairs(buildPlan) do
		for majorX, column in pairs(blockPlan) do
			for majorY, cell in pairs(column) do
				for blockIndex, block in ipairs(cell) do
					local blockPosX = majorX * blockSize + block.X;
					local blockPosY = majorY * blockSize + block.Y;

					if
							xPos >= blockPosX and xPos <= blockPosX + blockSize - 1
						and yPos >= blockPosY and yPos <= blockPosY + blockSize - 1
					then
						return true;
					end
				end
			end
		end
	end

	return false;
end

do
	--[[
	01110
	11011
	10001
	11011
	01110
	]]
	local neighbor16 = {
		{ X = -1, Y = -2 },
		{ X =  0, Y = -2 },
		{ X =  1, Y = -2 },
		{ X = -2, Y = -1 },
		{ X = -2, Y = -2 },
		{ X =  2, Y = -2 },
		{ X =  2, Y = -1 },
		{ X = -2, Y =  0 },
		{ X =  2, Y =  0 },
		{ X = -2, Y =  1 },
		{ X = -2, Y =  2 },
		{ X =  2, Y =  2 },
		{ X =  2, Y =  1 },
		{ X = -1, Y =  2 },
		{ X =  0, Y =  2 },
		{ X =  1, Y =  2 },
	};

	--[[
	010
	101
	010
	]]
	local neighbor8 = {
		{ X = 1, Y = 0 },
		{ X = 0, Y = 1 },
		{ X = -1, Y = 0 },
		{ X = 0, Y = -1 },
		{ X = -1, Y = -1 },
		{ X =  1, Y = -1 },
		{ X = -1, Y =  1 },
		{ X =  1, Y =  1 },
	};

	function ConstructorCheckPixelTypeAgainstBuildPlan(buildPlan, xPos, yPos, blockPos, size)
		if xPos <= blockPos.X + 1 or xPos >= blockPos.X + size - 2
		or yPos <= blockPos.Y + 1 or yPos >= blockPos.Y + size - 2 then
			if xPos == blockPos.X or xPos == blockPos.X + size - 1
			or yPos == blockPos.Y or yPos == blockPos.Y + size - 1 then
				for _, neighbor in ipairs(neighbor8) do
					if not ConstructorIsInPlan(buildPlan, xPos + neighbor.X, yPos + neighbor.Y) then
						return 2;
					end
				end
				
				for _, neighbor in ipairs(neighbor16) do
					if not ConstructorIsInPlan(buildPlan, xPos + neighbor.X, yPos + neighbor.Y) then
						return 1;
					end
				end
			else
				for _, neighbor in ipairs(neighbor16) do
					if not ConstructorIsInPlan(buildPlan, xPos + neighbor.X, yPos + neighbor.Y) then
						return 1;
					end
				end
			end
		end

		return 0;
	end
end

function Create(self)
	self.displayTimer = Timer();

	self.buildTimer = Timer();

	self.buildLists = {};
	self.buildInfos = {};
	self.buildSequence = {};

	self.tempLists = {};
	self.tempInfos = {};
	self.tempSequence = {};

	self.cellSize = 2;
	self.buildCost = self.cellSize * self.cellSize + 1;	--How much resource is required per one build cellSize by cellSize px piece
	self.sprayCost = self.buildCost * 0.5;

	self.buildSize = 24;
	self.buildSizeMin = self.buildSize/4;
	self.buildSizeMax = self.buildSize;
	self.fullBlock = 144 * self.buildCost;	--One full 24x24 block of concrete requires 64 units of resource
	self.maxResource = 12 * self.fullBlock;
	self.startResource = 3;
	self.resource = self.startResource * self.fullBlock;
	self.tunnelFillTimer = Timer();

	self.clearer = CreateMOSRotating("Constructor Terrain Clearer");

	self.digStrength = 200;	--The StructuralIntegrity limit of harvestable materials

	self.digLength = 40;
	self.spreadRange = math.rad(self.ParticleSpreadRange);
	self.buildsPerSecond = 100;
	self.buildSound = CreateSoundContainer("Geiger Click", "Base.rte");

	self.buildDistance = 400; -- pixel distance
	self.minFillDistance = 5; -- block distance
	self.maxFillDistance = 6; -- block distance
	self.tunnelFillDelay = 30000 + 30000 * (1 - ActivityMan:GetActivity().Difficulty/GameActivity.MAXDIFFICULTY);

	self.menu_ignore = false; -- ignore the pie menu button until it's released
	self.ignorePrimaryWeaponKey = false; -- ignore the weapon primary button until it's released

	-- don't change these
	self.toAutoBuild = false;
	self.operatedByAI = false;
	self.cursorMoveSpeed = 2;
	self.maxCursorDist = Vector(FrameMan.PlayerScreenWidth * 0.5 - 6, FrameMan.PlayerScreenHeight * 0.5 - 6);

	-- autobuild for standard units
	self.autoBuildList = {
		Vector(-3, 1),
		Vector(-2, 1),
		Vector(-1, 1),
		Vector(2, 1),
		Vector(3, 1),
		Vector(4, 1),

		Vector(-4, -2),
		Vector(-3, -2),
		Vector(0, -2),
		Vector(1, -2),
		Vector(4, -2),
		Vector(5, -2),

		Vector(-3, -3),
		Vector(4, -3),

		Vector(-3, -4),
		Vector(4, -4),

		Vector(-3, -5),
		Vector(-2, -5),
		Vector(-1, -5),
		Vector(2, -5),
		Vector(3, -5),
		Vector(4, -5),

		Vector(-3, -8),
		Vector(-2, -8),
		Vector(-1, -8),
		Vector(0, -8),
		Vector(1, -8),
		Vector(2, -8),
		Vector(3, -8),
		Vector(4, -8)
	};

	-- autobuild for brain units
	self.autoBuildListBrain = {
		Vector(-2, 2),
		Vector(-2, 1),
		Vector(-2, 0),
		Vector(-2, -1),
		Vector(2, 2),
		Vector(2, 1),
		Vector(2, 0),
		Vector(2, -1),

		Vector(-2, -2),
		Vector(-1, -2),
		Vector(0, -2),
		Vector(1, -2),
		Vector(2, -2),

		Vector(-3, 3),
		Vector(-3, 2),
		Vector(-3, 1),
		Vector(-3, 0),
		Vector(-3, -1),

		Vector(3, 3),
		Vector(3, 2),
		Vector(3, 1),
		Vector(3, 0),
		Vector(3, -1),

		Vector(-1, -1),
		Vector(0, -1),
		Vector(1, -1),
	};
end

function OnAttach(self, newParent)
	local rootParent = self:GetRootParent();
	if IsActor(rootParent) and MovableMan:IsActor(rootParent) then
		local pieMenu = ToActor(rootParent).PieMenu;
		local subPieMenuPieSlice = pieMenu:GetFirstPieSliceByPresetName("Constructor Options");
		if subPieMenuPieSlice ~= nil then
			pieMenu = subPieMenuPieSlice.SubPieMenu;
		end

		local mode = self:GetStringValue("ConstructorMode");
		local pieSliceToAddPresetName = mode == "Dig" and "Constructor Spray Mode" or "Constructor Dig Mode";
		pieMenu:AddPieSliceIfPresetNameIsUnique(CreatePieSlice(pieSliceToAddPresetName, self.ModuleName), self);
	end
end

function Update(self)
	local actor = self:GetRootParent();

	if actor and IsActor(actor) then
		actor = ToActor(actor);
		local ctrl = actor:GetController();
		local playerControlled = actor:IsPlayerControlled();
		local screen = ActivityMan:GetActivity():ScreenOfPlayer(ctrl.Player);

		if playerControlled and self.menu_ignore then
			if not ctrl:IsState(Controller.PIE_MENU_ACTIVE) then
				self.menu_ignore = false;
			end
		end

		if playerControlled and self.ignorePrimaryWeaponKey then
			if not ctrl:IsState(Controller.WEAPON_PRIMARY_HOTKEYSTART) then
				self.ignorePrimaryWeaponKey = false;
			end
		end

		if self.Magazine then
			self.Magazine.RoundCount = math.max(self.resource, 1);

			self.Magazine.Mass = 1 + 29 * (self.resource/self.maxResource);
			self.Magazine.Scale = 0.5 + (self.resource/self.maxResource) * 0.5;

			local parentWidth = ToMOSprite(actor):GetSpriteWidth();
			local parentHeight = ToMOSprite(actor):GetSpriteHeight();
			self.Magazine.Pos = actor.Pos + Vector(-(self.Magazine.Radius * 0.3 + parentWidth * 0.2 - 0.5) * self.FlipFactor, -(self.Magazine.Radius * 0.15 + parentHeight * 0.2)):RadRotate(actor.RotAngle);
			self.Magazine.RotAngle = actor.RotAngle;
		end

		if ctrl:IsState(Controller.PIE_MENU_ACTIVE) then
			PrimitiveMan:DrawTextPrimitive(screen, actor.AboveHUDPos + Vector(0, 26), "Mode: ".. self:GetStringValue("ConstructorMode"), true, 1);
		end

		-- constructor actions if the user is in gold dig mode
		if playerControlled then
			self.operatedByAI = false;
			self.toAutoBuild = true;
		elseif actor.AIMode == Actor.AIMODE_GOLDDIG then
			if self.toAutoBuild == false then
				if self:GetStringValue("ConstructorMode") == "Spray" then
					self:SetStringValue("ConstructorMode", "Dig");
				end
				if ctrl:IsState(Controller.WEAPON_FIRE) and SceneMan:ShortestDistance(actor.Pos, ConstructorTerrainRay(actor.Pos, Vector(0, 50), 3), SceneMan.SceneWrapsX):MagnitudeIsLessThan(30) then
					self.tunnelFillTimer:Reset();
					self.operatedByAI = true;
					self.aiSkillRatio = 1.5 - ActivityMan:GetActivity():GetTeamAISkill(actor.Team)/100;
					self.toAutoBuild = true;
					self.buildLists = {};
					local buildscheme = self.autoBuildList;
					if actor:HasObjectInGroup("Brains") then
						buildscheme = self.autoBuildListBrain;
						self.buildSize = 12;
					else
						self.buildSize = 24;
					end
					local snappos = ConstructorSnapPos(actor.Pos, self.buildSize);
					for i = 1, #buildscheme do
						local temppos = snappos + Vector(buildscheme[i].X * self.buildSize, buildscheme[i].Y * self.buildSize);
						local buildThis = {};
						buildThis[1] = temppos.X;
						buildThis[2] = temppos.Y;
						buildThis[3] = 0;
						buildThis[4] = self.buildSize;
						self.buildLists[#self.buildLists + 1] = buildThis;
					end
				end
			end

			-- constructor actions if it's AI controlled
			if self.operatedByAI then
				if self.tunnelFillTimer:IsPastSimMS(self.tunnelFillDelay * self.aiSkillRatio) and #self.buildLists == 0 then
					self.buildSize = 24;
					self.tunnelFillTimer:Reset();

					-- create an empty 2D array, call cells having -1
					local floodFillListX = {};
					for x = 1, (self.maxFillDistance * 2) + 1 do
						floodFillListX[x] = {};
						for y = 1, (self.maxFillDistance * 2) + 1 do
							floodFillListX[x][y] = -1;
						end
					end

					-- figure out the center of the grid
					local center = math.ceil(((self.maxFillDistance * 2) + 1) * 0.5);

					-- FLOOD FILL!
					ConstructorFloodFill(center, center, 0, self.maxFillDistance, floodFillListX, ConstructorSnapPos(actor.Pos, self.buildSize), self.buildSize);

					-- dump the correctly numbered cells into the build table
					for x = 1, #floodFillListX do
						for y = 1, #floodFillListX do
							if floodFillListX[x][y] >= self.minFillDistance and floodFillListX[x][y] <= self.maxFillDistance then
								local mapX = ConstructorSnapPos(actor.Pos, self.buildSize).X + ((center - x) * -self.buildSize);
								local mapY = ConstructorSnapPos(actor.Pos, self.buildSize).Y + ((center - y) * -self.buildSize);
								local freeSlot = true;
								for i = 1, #self.buildLists do
									if self.buildLists[i] ~= nil and self.buildLists[i][1] == mapX and self.buildLists[i][2] == mapY then
										freeSlot = false;
										break;
									end
								end

								if freeSlot then
									local buildThis = {};
									buildThis[1] = mapX;
									buildThis[2] = mapY;
									buildThis[3] = 0;
									buildThis[4] = self.buildSize;
									self.buildLists[#self.buildLists + 1] = buildThis;
								end
							end
						end
					end
				end
			end
		else
			self.toAutoBuild = false;
		end

		if playerControlled and not self.cursor and ctrl:IsState(Controller.WEAPON_PRIMARY_HOTKEYSTART) then
			self.cursor = Vector(self.MuzzlePos.X, self.MuzzlePos.Y);
			-- If the player actively selected this, ignore the pie menu.
			self.ignorePrimaryWeaponKey = true;
		end

		local mode = self:GetNumberValue("BuildMode");
		if mode == 0 and not self.cursor then
			-- activation
			if ctrl:IsState(Controller.WEAPON_FIRE) then

				local angle = actor:GetAimAngle(true);

				if self:GetStringValue("ConstructorMode") == "Spray" then
					if self.resource >= self.sprayCost then
						local particleCount = 9;
						for i = 1, particleCount do
							local spray = CreateMOPixel("Particle Concrete " .. math.random(4), "Base.rte");
							spray.Pos = self.MuzzlePos;
							spray.Vel = self.Vel + Vector(RangeRand(11, 13), 0):RadRotate(angle + RangeRand(-0.5, 0.5) * self.spreadRange);
							spray.Team = self.Team;
							spray.IgnoresTeamHits = true;
							MovableMan:AddParticle(spray);
						end
						self.resource = self.resource - self.sprayCost;
					else
						self:Deactivate();
					end
				else
					for i = 1, self.RoundsFired do
						local trace = Vector(self.digLength, 0):RadRotate(angle + RangeRand(-1, 1) * self.spreadRange);
						local digPos = ConstructorTerrainRay(self.MuzzlePos, trace, 0);

						if SceneMan:GetTerrMatter(digPos.X, digPos.Y) ~= rte.airID then
							local digWeightTotal = 0;
							local totalVel = Vector();
							local found = 0;

							for x = 1, 3 do
								for y = 1, 3 do
									local checkPos = ConstructorWrapPos(Vector(digPos.X - 2 + x, digPos.Y - 2 + y));
									local terrCheck = SceneMan:GetTerrMatter(checkPos.X, checkPos.Y);
									local material = SceneMan:GetMaterialFromID(terrCheck);

									if material.StructuralIntegrity <= self.digStrength and material.StructuralIntegrity <= self.digStrength * RangeRand(0.5, 1.05) then
										local px = SceneMan:DislodgePixel(checkPos.X, checkPos.Y);

										if px then
											local digWeight = math.sqrt(material.StructuralIntegrity/self.digStrength);
											local speed = 3;

											if terrCheck == rte.goldID then
												--Spawn a glowy gold pixel and delete the original
												px.ToDelete = true;
												px = CreateMOPixel("Gold Particle", "Base.rte");
												px.Pos = checkPos;
												--Sharpness temporarily stores the ID of the target
												px.Sharpness = actor.ID;
												MovableMan:AddParticle(px);
											else
												px.Sharpness = self.ID;
												px.Lifetime = 1000;
												speed = speed + (1 - digWeight) * 5;
												digWeightTotal = digWeightTotal + digWeight;
											end

											px.IgnoreTerrain = true;
											px.Vel = Vector(trace.X, trace.Y):SetMagnitude(-speed):RadRotate(RangeRand(-0.5, 0.5));
											totalVel = totalVel + px.Vel;
											px:AddScript("Base.rte/Devices/Tools/Constructor/ConstructorCollect.lua");
											
											found = found + 1;
										end
									end
								end
							end

							if found > 0 then
								if digWeightTotal > 0 then
									digWeightTotal = digWeightTotal/9;
									self.resource = math.min(self.resource + digWeightTotal * self.buildCost, self.maxResource);
								end

								local collectFX = CreateMOPixel("Particle Constructor Gather Material" .. (digWeightTotal > 0.5 and " Big" or ""));
								collectFX.Vel = totalVel/found;
								collectFX.Pos = Vector(digPos.X, digPos.Y) + collectFX.Vel * rte.PxTravelledPerFrame;

								MovableMan:AddParticle(collectFX);
							else
								self:Deactivate();
							end
						else	-- deactivate if digging air
							self:Deactivate();
							break;
						end
					end
				end
			end
		elseif mode == 1 then	-- cancel
			self:RemoveNumberValue("BuildMode");

			self.buildLists = {};
			self.buildSequence = {};
			self.buildInfos = {};
			self.cursor = nil;
		elseif mode == 2 then	-- build
			self:RemoveNumberValue("BuildMode");

			-- constructor build cursor
			if playerControlled then
				self.cursor = Vector(self.MuzzlePos.X, self.MuzzlePos.Y);
				-- If the player actively selected this, ignore the pie menu.
				if ctrl:IsState(Controller.PIE_MENU_ACTIVE) then
					self.menu_ignore = true;
				end
			end
		end

		local displayColorBlue = 5;
		local displayColorYellow = 120;
		local displayColorRed = 13;
		local displayColorWhite = 254;
		local displayColorGreen = 145;

		if self.displayTimer:IsPastSimMS(100) then
			self.displayTimer:Reset();
			-- flickering colors
			displayColorBlue = 195;
			displayColorYellow = 116;
			displayColorRed = 12;
			displayColorWhite = 252;
			displayColorGreen = 147;
		end

		if self.cursor then
			local cursorMovement = Vector();
			local mouseControlled = ctrl:IsMouseControlled();
			local aiming = false;

			if mouseControlled then
				cursorMovement = cursorMovement + ctrl.MouseMovement;
			else
				aiming = ctrl:IsState(Controller.AIM_SHARP);

				if ctrl:IsState(Controller.HOLD_UP) or ctrl:IsState(Controller.BODY_JUMP) then
					cursorMovement = cursorMovement + Vector(0, -1);
				end

				if ctrl:IsState(Controller.HOLD_DOWN) or ctrl:IsState(Controller.BODY_PRONE) then
					cursorMovement = cursorMovement + Vector(0, 1);
				end

				if ctrl:IsState(Controller.HOLD_LEFT) then
					cursorMovement = cursorMovement + Vector(-1, 0);
				end

				if ctrl:IsState(Controller.HOLD_RIGHT) then
					cursorMovement = cursorMovement + Vector(1, 0);
				end
			end

			if ctrl:IsState(Controller.WEAPON_CHANGE_NEXT) then
				self.buildSize = self.buildSize * 2;

				if self.buildSize > self.buildSizeMax then
					self.buildSize = self.buildSizeMin;
				end
			end

			if ctrl:IsState(Controller.WEAPON_CHANGE_PREV) then
				self.buildSize = self.buildSize / 2;

				if self.buildSize < self.buildSizeMin then
					self.buildSize = self.buildSizeMax;
				end
			end

			if not self.tempLists[self.buildSize] then
				self.tempLists[self.buildSize] = {};
				self.tempInfos[self.buildSize] = {
					leastX = math.huge,
					greatestX = -math.huge,
					leastY = math.huge,
					greatestY = -math.huge,
				};
			end

			if cursorMovement:MagnitudeIsGreaterThan(0) then
				self.cursor = self.cursor + (mouseControlled and cursorMovement or cursorMovement:SetMagnitude(self.cursorMoveSpeed * (aiming and 0.5 or 1)));
			end

			local precise = nil;
			
			if not mouseControlled then
				precise = aiming;
			else
				precise = ctrl:IsState(Controller.WEAPON_AUXILIARY_HOTKEY);
			end

			local map = Vector();

			if precise then
				map = Vector(math.floor(self.cursor.X - self.buildSize/2), math.floor(self.cursor.Y - self.buildSize/2));
				PrimitiveMan:DrawLinePrimitive(screen, self.cursor + Vector(2, 2), self.cursor + Vector(-3, -3), displayColorYellow);
				PrimitiveMan:DrawLinePrimitive(screen, self.cursor + Vector(2, -3), self.cursor + Vector(-3, 2), displayColorYellow);
			else
				map = ConstructorSnapPos(self.cursor, self.buildSize);
				PrimitiveMan:DrawLinePrimitive(screen, self.cursor + Vector(0, 4), self.cursor + Vector(0, -4), displayColorYellow);
				PrimitiveMan:DrawLinePrimitive(screen, self.cursor + Vector(4, 0), self.cursor + Vector(-4, 0), displayColorYellow);
			end

			PrimitiveMan:DrawBoxPrimitive(screen, map, map + Vector(self.buildSize - 1, self.buildSize - 1), displayColorYellow);

			local dist = SceneMan:ShortestDistance(actor.ViewPoint, self.cursor, SceneMan.SceneWrapsX);

			if math.abs(dist.X) > self.maxCursorDist.X then
				self.cursor.X = actor.ViewPoint.X + self.maxCursorDist.X * (dist.X < 0 and -1 or 1);
			end

			if math.abs(dist.Y) > self.maxCursorDist.Y then
				self.cursor.Y = actor.ViewPoint.Y + self.maxCursorDist.Y * (dist.Y < 0 and -1 or 1);
			end

			if (not self.menu_ignore and ctrl:IsState(Controller.PIE_MENU_ACTIVE)) or ctrl:IsState(Controller.ACTOR_NEXT_PREP) or ctrl:IsState(Controller.ACTOR_PREV_PREP) then
				self.cursor = nil;
				self.tempLists = {};
				self.tempInfos = {};
				self.tempSequence = {};
			elseif playerControlled then
				-- add blocks to the build queue if the cursor is firing
				if ctrl:IsState(Controller.WEAPON_FIRE) then
					local freeSlot = true;
					local tempList = self.tempLists[self.buildSize];
					local tempInfo = self.tempInfos[self.buildSize];

					for _, tempList in pairs(self.tempLists) do
						for i = 1, #tempList do
							if
								tempList[i]
								and tempList[i][1] <= map.X and tempList[i][1] + _ >= map.X + self.buildSize
								and tempList[i][2] <= map.Y and tempList[i][2] + _ >= map.Y + self.buildSize
							then
								freeSlot = false;
								break;
							end
						end
					end

					if freeSlot then
						tempInfo.leastX = math.min(tempInfo.leastX, map.X);
						tempInfo.greatestX = math.max(tempInfo.greatestX, map.X);
						tempInfo.leastY = math.min(tempInfo.leastY, map.Y);
						tempInfo.greatestY = math.max(tempInfo.greatestY, map.Y);

						local blockDescription = {};
						blockDescription[1] = map.X;
						blockDescription[2] = map.Y;
						table.insert(tempList, blockDescription);

						table.insert(self.tempSequence, self.buildSize);
					end
				end
			else
				self.cursor = nil;
				self.tempLists = {};
				self.tempInfos = {};
				self.tempSequence = {};
			end

			if not self.ignorePrimaryWeaponKey and ctrl:IsState(Controller.WEAPON_PRIMARY_HOTKEYSTART) then
				-- Use temporary info about block size space to figure out how large a grid can contain requisite block offset data.
				local buildInfos = {};

				for _, tempInfo in pairs(self.tempInfos) do
					local buildInfo = {};

					buildInfo.lowBoundX = math.floor(tempInfo.leastX / _);
					buildInfo.lowBoundY = math.floor(tempInfo.leastY / _);
					buildInfo.highBoundX = math.floor(tempInfo.greatestX / _);
					buildInfo.highBoundY = math.floor(tempInfo.greatestY / _);

					buildInfos[_] = buildInfo;
				end

				-- No longer needed.
				self.tempInfos = {};

				-- The information to build is in the grid.
				self.buildInfos = buildInfos;

				-- Localize for speed, probably.
				local tempLists = self.tempLists;
				local tempSequence = self.tempSequence;

				-- This is the same order we want to actually build it in.
				self.buildSequence = tempSequence;

				-- Iterate through all blocks in the same order they were placed.
				for i = 1, #tempSequence do
					local _ = tempSequence[i];

					-- We can link it right away, tables do not copy assign, shallow or otherwise.
					local buildList = self.buildLists[_] or {};
					self.buildLists[_] = buildList;

					-- Get the block we care about, as well as grid info.
					local blockInfo = table.remove(tempLists[_], 1);
					local buildInfo = buildInfos[_];

					local blockX = math.floor(blockInfo[1] / _);
					local blockY = math.floor(blockInfo[2] / _);
					local remainderX = blockInfo[1] - blockX * _;
					local remainderY = blockInfo[2] - blockY * _;

					buildList[blockX] = buildList[blockX] or {};
					buildList[blockX][blockY] = buildList[blockX][blockY] or {};

					table.insert(buildList[blockX][blockY], {
						X = remainderX,
						Y = remainderY,
						C = 0,
					});

					tempSequence[i] = {_, blockX, blockY, #buildList[blockX][blockY]};
				end
				
				-- Once the loop is done, every temp block has been transformed into a categorized grid positioned and offset block in buildLists.
				self.tempLists = {};
				-- The temp sequences list is also full of tables now, we've linked it to buildSequence as well.
				self.tempSequence = {};
			end
				
			if not ctrl:IsState(Controller.WEAPON_PRIMARY_HOTKEY) then
				local ultraLowBoundX = math.huge;
				local ultraLowBoundY = math.huge;
				local ultraHighBoundX = -math.huge;
				local ultraHighBoundY = -math.huge;

				for _, tempInfo in pairs(self.tempInfos) do
					local tempInfo = self.tempInfos[_];

					ultraLowBoundX = math.min(tempInfo.leastX, ultraLowBoundX);
					ultraLowBoundY = math.min(tempInfo.leastY, ultraLowBoundY);
					ultraHighBoundX = math.max(tempInfo.greatestX + _, ultraHighBoundX);
					ultraHighBoundY = math.max(tempInfo.greatestY + _, ultraHighBoundY);
				end

				local corner = Vector(ultraLowBoundX - 3, ultraLowBoundY - 3);
				PrimitiveMan:DrawTrianglePrimitive(screen, corner, corner + Vector(5, 0), corner + Vector(0, 5), displayColorBlue);
				corner = Vector(ultraLowBoundX - 3, ultraHighBoundY + 2);
				PrimitiveMan:DrawTrianglePrimitive(screen, corner, corner + Vector(5, 0), corner + Vector(0, -5), displayColorBlue);
				corner = Vector(ultraHighBoundX + 2, ultraHighBoundY + 2);
				PrimitiveMan:DrawTrianglePrimitive(screen, corner, corner + Vector(-5, 0), corner + Vector(0, -5), displayColorBlue);
				corner = Vector(ultraHighBoundX + 2, ultraLowBoundY - 3);
				PrimitiveMan:DrawTrianglePrimitive(screen, corner, corner + Vector(-5, 0), corner + Vector(0, 5), displayColorBlue);

				for _, tempList in pairs(self.tempLists) do
					for i, block in pairs(tempList) do
						if not self.operatedByAI then
							if SceneMan:ShortestDistance(actor.Pos, Vector(block[1], block[2]), SceneMan.SceneWrapsX):MagnitudeIsLessThan(self.buildDistance) then
								PrimitiveMan:DrawBoxPrimitive(screen, Vector(block[1], block[2]), Vector(block[1] + _ - 1, block[2] + _ - 1), displayColorBlue);
							else
								PrimitiveMan:DrawBoxPrimitive(screen, Vector(block[1], block[2]), Vector(block[1] + _ - 1, block[2] + _ - 1), displayColorRed);
							end
						end
					end
				end
			end
			
			if not ((not self.menu_ignore and ctrl:IsState(Controller.PIE_MENU_ACTIVE)) or ctrl:IsState(Controller.ACTOR_NEXT_PREP) or ctrl:IsState(Controller.ACTOR_PREV_PREP)) and playerControlled then
				-- go through and disable all 41 controller states when moving the build cursor
				for state = 0, 40 do
					ctrl:SetState(state, false);
				end
			end
		end
		
		for _, blockIndex in ipairs(self.buildSequence) do
			local size = blockIndex[1];
			local majorX = blockIndex[2];
			local majorY = blockIndex[3];
			local cellIndex = blockIndex[4];
			local grid = self.buildLists[size];
			local column = grid[majorX];
			local cell = column[majorY];
			local block = cell[cellIndex];
			local fullX = majorX * size + block.X;
			local fullY = majorY * size + block.Y;
			if not self.operatedByAI then
				if SceneMan:ShortestDistance(actor.Pos, Vector(fullX + size / 2, fullY + size / 2), SceneMan.SceneWrapsX):MagnitudeIsLessThan(self.buildDistance) then
					PrimitiveMan:DrawBoxPrimitive(screen, Vector(fullX, fullY), Vector(fullX + size - 1, fullY + size - 1), displayColorGreen);
				else
					PrimitiveMan:DrawBoxPrimitive(screen, Vector(fullX, fullY), Vector(fullX + size - 1, fullY + size - 1), displayColorRed);
				end
			end
		end

		if self.buildSequence[1] then
			if self.resource >= self.buildCost then
				local blockIndex = self.buildSequence[1];
				local size = blockIndex[1];
				local majorX = blockIndex[2];
				local majorY = blockIndex[3];
				local cellIndex = blockIndex[4];
				local grid = self.buildLists[size];
				local column = grid[majorX];
				local cell = column[majorY];
				local block = cell[cellIndex];
				local fullX = majorX * size + block.X;
				local fullY = majorY * size + block.Y;

				if SceneMan:ShortestDistance(actor.Pos, Vector(fullX + size / 2, fullY + size / 2), SceneMan.SceneWrapsX):MagnitudeIsLessThan(self.buildDistance) then
					local cellSize = self.cellSize;
					local oneThirdBlock = size/cellSize;
					local cellsPerBlock = oneThirdBlock^2;

					if block.C < cellsPerBlock then
						local by = math.floor(block.C/oneThirdBlock);
						local bx = block.C - (by * oneThirdBlock);
						by = by * cellSize - 1;
						bx = bx * cellSize - 1;
						block.C = block.C + 1;
						local totalCost = 0;
						local startPos = ConstructorWrapPos(Vector(bx + fullX, by + fullY));
						local didBuild = false;

						for x = 1, cellSize do
							for y = 1, cellSize do
								local pos = Vector(startPos.X + x, startPos.Y + y);
								local strengthRatio = SceneMan:GetMaterialFromID(SceneMan:GetTerrMatter(pos.X, pos.Y)).StructuralIntegrity/self.digStrength;

								if strengthRatio < 1 and SceneMan:GetMOIDPixel(pos.X, pos.Y) == rte.NoMOID then
									local name = "";

									local whichBorder = ConstructorCheckPixelTypeAgainstBuildPlan(self.buildLists, pos.X, pos.Y, { X = fullX, Y = fullY }, size);

									if whichBorder == 0 then -- Normal
										name = "Base.rte/Constructor Tile " .. math.random(16);
									elseif whichBorder == 1 then -- Inner border
										self.colorCandidates = { 8, 11, 12, 6, 13 };
										name = "Base.rte/Constructor Tile " .. self.colorCandidates[math.random(#self.colorCandidates)];
									else -- Outter border
										name = "Base.rte/Constructor Border Tile " .. math.random(4);
									end
								
									local terrainObject = CreateTerrainObject(name);
									terrainObject.Pos = pos;
									SceneMan:AddSceneObject(terrainObject);
									didBuild = true;
									totalCost = totalCost + (1 - strengthRatio) / cellSize / cellSize;
								end
							end
						end

						if didBuild then
							self.resource = self.resource - (self.buildCost * totalCost);
							local buildPos = self.Pos + SceneMan:ShortestDistance(self.Pos, Vector(bx + fullX + (cellSize - 1), by + fullY + (cellSize - 1)), SceneMan.SceneWrapsX);

							for otherPlayer = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
								local otherScreen = ActivityMan:GetActivity():ScreenOfPlayer(otherPlayer);

								if otherScreen ~= -1 and (otherScreen == screen or not SceneMan:IsUnseen(buildPos.X, buildPos.Y, ActivityMan:GetActivity():GetTeamOfPlayer(otherPlayer))) then
									PrimitiveMan:DrawBoxFillPrimitive(otherScreen, Vector(bx + fullX + 1, by + fullY + 1), Vector(bx + fullX + cellSize, by + fullY + cellSize), displayColorWhite);
								end
							end

							if screen ~= -1 then
								PrimitiveMan:DrawLinePrimitive(screen, self.Pos, buildPos, displayColorBlue);
							end

							self.buildSound.Volume = totalCost;
							self.buildSound.Pitch = 2 - totalCost;
							self.buildSound:Play(buildPos);

							if block.C == cellsPerBlock then
								table.remove(self.buildSequence, 1);
							end
						end
					else
						table.remove(self.buildSequence, 1);
					end
				else
					table.insert(self.buildSequence, table.remove(self.buildSequence, 1));
				end
			end
		else
			self.buildLists = {};
			self.buildSequence = {};
			self.buildInfos = {};
		end

		if display then
			self.displayTimer:Reset();
		end
	elseif self.cursor then
		self.cursor = nil;
	end
end