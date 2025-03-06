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
	SceneMan:CastStrengthRay(start, trace, 0, hitPos, skip, rte.airID, true);	
	return hitPos;
end

function ConstructorIsInPlan(buildPlan, xPos, yPos)
	for blockSize, blockPlan in pairs(buildPlan) do
		for majorX, column in pairs(blockPlan) do
			for majorY, cell in pairs(column) do
				-- TODO: This could maybe be made more efficient by skipping columns and cells if they are distant, but only in larger sets I think
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
	11111
	10001
	10001
	10001
	11111
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
	111
	101
	111
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

	-- TODO: This could also be more efficient by only checking necessary pixels.
	-- It's a given that at least 25% of the 8 and 16 neighbors are inside of the block this is currently checking a pixel of.
	-- The fast but bulky solution might be translating an x and y offset from block origin to an index for a cached list of neighbor sets.
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
	-- Timers
	self.displayTimer = Timer();
	self.buildTimer = Timer();
	self.tunnelFillTimer = Timer();

	-- Keep a list of recorded orders.
	self.constructionOrders = {};
	self.excavationOrders = {};

	-- Keep a list of build order sequence as well.
	self.orderSequence = {};

	-- Keep the listings of blocks in various sizes, as well as the corners of each size, and a sequencing list.
	self.blueprintBlocksPerSize = {};
	self.blueprintCornersPerSize = {};
	self.blueprintSizeSequence = {};

	-- Size of constructor action cell, 3 seems to be optimal.
	self.cellSize = 3;

	-- Cost of one cell construction against air is 11% more than actual quantity of concrete required for that area.
	self.cellCost = self.cellSize * self.cellSize + 1;
	self.sprayCost = self.cellCost * 0.5;

	-- Standard construction block in CC.
	self.blockSize = 24;
	self.blockSizeMin = self.blockSize/4;
	self.blockSizeMax = self.blockSize;

	-- A block costs as much as a cell times the number of cells per block.
	self.fullBlock = self.blockSize * self.blockSize / (self.cellSize * self.cellSize) * self.cellCost;
	self.maxResource = 12 * self.fullBlock;
	self.resource = 3 * self.fullBlock;

	-- Not used presently, but nanolyzer among others use it, so remains in INI.
	--self.clearer = CreateMOSRotating("Constructor Terrain Clearer");

	-- The StructuralIntegrity limit of harvestable materials, equal to that of concrete.
	self.digStrength = 200;

	-- The maximum distance from the muzzle which the constructor can dig.
	self.digLength = 40;

	self.spreadRange = math.rad(self.ParticleSpreadRange);
	self.buildSound = CreateSoundContainer("Geiger Click", "Base.rte");

	-- Max distances of build and dig orders respectively, in pixels.
	self.buildDistance = 120;
	self.excavationDistance = 120;

	-- AI operation data. Probably relevant eventually.
	self.minFillDistance = 5;
	self.maxFillDistance = 6;
	self.tunnelFillDelay = 30000 + 30000 * (1 - ActivityMan:GetActivity().Difficulty/GameActivity.MAXDIFFICULTY);

	-- For ignoring inputs until release.
	self.ignorePieMenu = false;
	self.ignorePrimaryWeaponKey = false;

	-- Cursor speed amplification for gamepad input.
	self.cursorMoveSpeed = 2;

	-- How far we allow the cursor to wander from origin (user).
	self.maxCursorDist = Vector(FrameMan.PlayerScreenWidth * 0.5 - 6, FrameMan.PlayerScreenHeight * 0.5 - 6);

	-- Whether the AI is presently pretending to know what it's doing.
	self.operatedByAI = false;

	-- Autobuild for standard units.
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

	-- Autobuild for brain units.
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

		local isDigging = self:GetStringValue("ConstructorMode") == "Dig";

		local pieSliceToAddPresetName = isDigging and "Constructor Spray Mode" or "Constructor Dig Mode";
		local pieSliceToRemovePresetName = isDigging and "Constructor Dig Mode" or "Constructor Spray Mode";
		pieMenu:AddPieSliceIfPresetNameIsUnique(CreatePieSlice(pieSliceToAddPresetName, self.ModuleName), self);
		pieMenu:RemovePieSlicesByPresetName(pieSliceToRemovePresetName);

		pieSliceToAddPresetName = isDigging and "Constructor Order Excavation" or "Constructor Order Construction";
		pieSliceToRemovePresetName = isDigging and "Constructor Order Construction" or "Constructor Order Excavation";
		pieMenu:AddPieSliceIfPresetNameIsUnique(CreatePieSlice(pieSliceToAddPresetName, self.ModuleName), self);
		pieMenu:RemovePieSlicesByPresetName(pieSliceToRemovePresetName);
	end
end

function Update(self)
	local actor = self:GetRootParent();

	if actor and IsActor(actor) then
		actor = ToActor(actor);

		local muzzlePosition = Vector(self.MuzzlePos.X, self.MuzzlePos.Y);
		local ctrl = actor:GetController();
		local playerControlled = actor:IsPlayerControlled();
		local screen = ActivityMan:GetActivity():ScreenOfPlayer(ctrl.Player);

		if playerControlled and self.ignorePieMenu then
			if not ctrl:IsState(Controller.PIE_MENU_ACTIVE) then
				self.ignorePieMenu = false;
			end
		end

		if playerControlled and self.ignorePrimaryWeaponKey then
			if not ctrl:IsState(Controller.WEAPON_PRIMARY_HOTKEY) then
				self.ignorePrimaryWeaponKey = false;
			end
		end

		local magazine = self.Magazine;

		if not magazine then
			magazine = CreateMagazine("Magazine Constructor", self.ModuleName);
			self.Magazine = magazine;
		end

		magazine.RoundCount = math.max(self.resource, 1);

		magazine.Mass = 1 + 29 * (self.resource/self.maxResource);
		magazine.Scale = 0.5 + (self.resource/self.maxResource) * 0.5;

		local parentWidth = ToMOSprite(actor):GetSpriteWidth();
		local parentHeight = ToMOSprite(actor):GetSpriteHeight();
		magazine.Pos = actor.Pos + Vector(-(magazine.Radius * 0.3 + parentWidth * 0.2 - 0.5) * self.FlipFactor, -(magazine.Radius * 0.15 + parentHeight * 0.2)):RadRotate(actor.RotAngle);
		magazine.RotAngle = actor.RotAngle;

		-- constructor actions if the user is in gold dig mode
		if playerControlled then
			self.operatedByAI = false;
		else
			local isGoingTo = actor.AIMode == Actor.AIMODE_GOTO;
			local isGoldDigging = actor.AIMode == Actor.AIMODE_GOLDDIG;
			
			if (isGoingTo or isGoldDigging) and self:GetStringValue("ConstructorMode") ~= "Dig" then
				self:SetStringValue("ConstructorMode", "Dig");
			end
			
			if (isGoldDigging) then
				if not self.operatedByAI then
					self.operatedByAI = true;
				end

				local presentOrderTunneling = self.orderSequence[1];

				if presentOrderTunneling ~= nil then
					local pressingOrderList = presentOrderTunneling and self.excavationOrders or self.constructionOrders;
					local order = pressingOrderList[1];

					if order then
						local orderInfo = order.orderInfo;
						local topLeftExcavation = Vector(orderInfo.ultraLowBoundX, orderInfo.ultraLowBoundY);
						local topRightExcavation = Vector(orderInfo.ultraHighBoundX, orderInfo.ultraHighBoundY);
						local moveGoal = topLeftExcavation + SceneMan:ShortestDistance(topLeftExcavation, topRightExcavation, true) / 2;

						if actor:GetWaypointListSize() > 0 and actor:SceneWaypoints() ~= moveGoal then
							actor:ClearAIWaypoints();
							actor:AddAISceneWaypoint(moveGoal);
						end
					end
				end
			end

			--[[
			if not self.operatedByAI then
				if ctrl:IsState(Controller.WEAPON_FIRE) and SceneMan:ShortestDistance(actor.Pos, ConstructorTerrainRay(actor.Pos, Vector(0, 50), 3), true):MagnitudeIsLessThan(30) then
					self.operatedByAI = true;

					self.tunnelFillTimer:Reset();
					self.aiSkillRatio = 1.5 - ActivityMan:GetActivity():GetTeamAISkill(actor.Team) / 100;
					self.buildLists = {};
					local buildscheme = self.autoBuildList;

					if actor:HasObjectInGroup("Brains") then
						buildscheme = self.autoBuildListBrain;
						self.blockSize = 12;
					else
						self.blockSize = 24;
					end

					local snappos = ConstructorSnapPos(actor.Pos, self.blockSize);

					for i = 1, #buildscheme do
						local temppos = snappos + Vector(buildscheme[i].X * self.blockSize, buildscheme[i].Y * self.blockSize);
						local buildThis = {};
						buildThis[1] = temppos.X;
						buildThis[2] = temppos.Y;
						buildThis[3] = 0;
						buildThis[4] = self.blockSize;
						self.buildLists[#self.buildLists + 1] = buildThis;
					end
				end
			end

			-- constructor actions if it's AI controlled
			if self.operatedByAI then
				if self.tunnelFillTimer:IsPastSimMS(self.tunnelFillDelay * self.aiSkillRatio) and #self.buildLists == 0 then
					self.blockSize = 24;
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
					ConstructorFloodFill(center, center, 0, self.maxFillDistance, floodFillListX, ConstructorSnapPos(actor.Pos, self.blockSize), self.blockSize);

					-- dump the correctly numbered cells into the build table
					for x = 1, #floodFillListX do
						for y = 1, #floodFillListX do
							if floodFillListX[x][y] >= self.minFillDistance and floodFillListX[x][y] <= self.maxFillDistance then
								local mapX = ConstructorSnapPos(actor.Pos, self.blockSize).X + ((center - x) * -self.blockSize);
								local mapY = ConstructorSnapPos(actor.Pos, self.blockSize).Y + ((center - y) * -self.blockSize);
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
									buildThis[4] = self.blockSize;
									self.buildLists[#self.buildLists + 1] = buildThis;
								end
							end
						end
					end
				end
			end--]]
		end

		if playerControlled and not self.cursor and ctrl:IsState(Controller.WEAPON_PRIMARY_HOTKEYSTART) then
			self.cursor = muzzlePosition;
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
							local spray = CreateMOPixel("Particle Concrete " .. math.random(4), self.ModuleName);
							spray.Pos = muzzlePosition;
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
						local digPos = ConstructorTerrainRay(muzzlePosition, trace, 0);

						if SceneMan:GetTerrMatter(digPos.X, digPos.Y) ~= rte.airID then
							local digWeightTotal = 0;
							local totalVel = Vector();
							local found = 0;

							for x = 1, 3 do
								for y = 1, 3 do
									local checkPos = ConstructorWrapPos(Vector(digPos.X - 1 + x, digPos.Y - 1 + y));
									local terrCheck = SceneMan:GetTerrMatter(checkPos.X, checkPos.Y);
									local material = SceneMan:GetMaterialFromID(terrCheck);

									if material.StructuralIntegrity <= self.digStrength and material.StructuralIntegrity <= self.digStrength * RangeRand(0.5, 1.10) then
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
									self.resource = math.min(self.resource + digWeightTotal * self.cellCost, self.maxResource);
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

			-- Cancel the last given order
			local lastIndexOfSequence = #self.orderSequence;

			if lastIndexOfSequence > 0 then
				local removingExcavation = table.remove(self.orderSequence, lastIndexOfSequence);
				local target = removingExcavation and self.excavationOrders or self.constructionOrders;
				table.remove(target, #target);
			end
		elseif mode == 2 then	-- build
			self:RemoveNumberValue("BuildMode");

			-- constructor build cursor
			if playerControlled then
				self.cursor = muzzlePosition;
				-- If the player actively selected this, ignore the pie menu.
				if ctrl:IsState(Controller.PIE_MENU_ACTIVE) then
					self.ignorePieMenu = true;
				end
			end
		end

		local displayColorBlue = 5;
		local displayColorYellow = 120;
		local displayColorRed = 13;
		local displayColorWhite = 254;
		local displayColorGray = 254;
		local displayColorGreen = 145;
		local displayColorOrange = 47;

		if self.displayTimer:IsPastSimMS(100) then
			self.displayTimer:Reset();
			-- flickering colors
			displayColorBlue = 195;
			displayColorYellow = 116;
			displayColorRed = 12;
			displayColorWhite = 252;
			displayColorGray = 254; -- TODO: Color pick something for this, use it for other player view
			displayColorGreen = 147;
			displayColorOrange = 48;
		end
		
		local precise = nil;
		local map = nil;

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
				self.blockSize = self.blockSize * 2;

				if self.blockSize > self.blockSizeMax then
					self.blockSize = self.blockSizeMin;
				end
			end

			if ctrl:IsState(Controller.WEAPON_CHANGE_PREV) then
				self.blockSize = self.blockSize / 2;

				if self.blockSize < self.blockSizeMin then
					self.blockSize = self.blockSizeMax;
				end
			end

			if not self.blueprintBlocksPerSize[self.blockSize] then
				self.blueprintBlocksPerSize[self.blockSize] = {};
				self.blueprintCornersPerSize[self.blockSize] = {
					leastX = math.huge,
					greatestX = -math.huge,
					leastY = math.huge,
					greatestY = -math.huge,
				};
			end

			if cursorMovement:MagnitudeIsGreaterThan(0) then
				self.cursor = self.cursor + (mouseControlled and cursorMovement or cursorMovement:SetMagnitude(self.cursorMoveSpeed * (aiming and 0.5 or 1)));
			end

			if not mouseControlled then
				precise = aiming;
			else
				precise = ctrl:IsState(Controller.WEAPON_AUXILIARY_HOTKEY);
			end

			if precise then
				map = Vector(math.floor(self.cursor.X - self.blockSize/2), math.floor(self.cursor.Y - self.blockSize/2));
			else
				map = ConstructorSnapPos(self.cursor, self.blockSize);
			end

			local dist = SceneMan:ShortestDistance(actor.ViewPoint, self.cursor, true);

			if math.abs(dist.X) > self.maxCursorDist.X then
				self.cursor.X = actor.ViewPoint.X + self.maxCursorDist.X * (dist.X < 0 and -1 or 1);
			end

			if math.abs(dist.Y) > self.maxCursorDist.Y then
				self.cursor.Y = actor.ViewPoint.Y + self.maxCursorDist.Y * (dist.Y < 0 and -1 or 1);
			end

			if (not self.ignorePieMenu and ctrl:IsState(Controller.PIE_MENU_ACTIVE)) or ctrl:IsState(Controller.ACTOR_NEXT_PREP) or ctrl:IsState(Controller.ACTOR_PREV_PREP) then
				self.cursor = nil;
				self.blueprintBlocksPerSize = {};
				self.blueprintCornersPerSize = {};
				self.blueprintSizeSequence = {};
			elseif playerControlled then
				-- add blocks to the build queue if the cursor is firing
				if ctrl:IsState(Controller.WEAPON_FIRE) then
					local freeSlot = true;
					local tempList = self.blueprintBlocksPerSize[self.blockSize];
					local tempInfo = self.blueprintCornersPerSize[self.blockSize];

					for _, tempList in pairs(self.blueprintBlocksPerSize) do
						for i = 1, #tempList do
							if
								tempList[i]
								and tempList[i][1] <= map.X and tempList[i][1] + _ >= map.X + self.blockSize
								and tempList[i][2] <= map.Y and tempList[i][2] + _ >= map.Y + self.blockSize
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

						table.insert(self.blueprintSizeSequence, self.blockSize);
					end
				end
			else
				self.cursor = nil;
				self.blueprintBlocksPerSize = {};
				self.blueprintCornersPerSize = {};
				self.blueprintSizeSequence = {};
			end

			if not self.ignorePrimaryWeaponKey and #self.blueprintSizeSequence > 0 and ctrl:IsState(Controller.WEAPON_PRIMARY_HOTKEYSTART) then
				local order = {};
				local orderInfo = {};
				
				orderInfo.ultraLowBoundX = math.huge;
				orderInfo.ultraLowBoundY = math.huge;
				orderInfo.ultraHighBoundX = -math.huge;
				orderInfo.ultraHighBoundY = -math.huge;

				for _, tempInfo in pairs(self.blueprintCornersPerSize) do
					local tempInfo = self.blueprintCornersPerSize[_];

					orderInfo.ultraLowBoundX = math.min(tempInfo.leastX, orderInfo.ultraLowBoundX);
					orderInfo.ultraLowBoundY = math.min(tempInfo.leastY, orderInfo.ultraLowBoundY);
					orderInfo.ultraHighBoundX = math.max(tempInfo.greatestX + _, orderInfo.ultraHighBoundX);
					orderInfo.ultraHighBoundY = math.max(tempInfo.greatestY + _, orderInfo.ultraHighBoundY);
				end

				self.blueprintCornersPerSize = {};
				order.orderInfo = orderInfo;

				-- Localize for speed, probably.
				local tempLists = self.blueprintBlocksPerSize;
				local tempSequence = self.blueprintSizeSequence;

				-- This is the same order we want to actually build it in.
				order.orderSequence = tempSequence;
				order.orderBlockSpaces = {};

				-- Iterate through all blocks in the same order they were placed.
				for i = 1, #tempSequence do
					local _ = tempSequence[i];

					-- We can link it right away, tables do not copy assign, shallow or otherwise.
					local buildList = order.orderBlockSpaces[_] or {};
					order.orderBlockSpaces[_] = buildList;

					-- Get the block we care about, as well as grid info.
					local blockInfo = table.remove(tempLists[_], 1);

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
				
				-- Once the loop is done, every temp block has been transformed into a categorized grid positioned and offset block in orderBlockSpaces.
				self.blueprintBlocksPerSize = {};
				-- The temp sequences list is also full of tables now, we've linked it to orderSequence as well.
				self.blueprintSizeSequence = {};

				-- Depending on whether the given orders are to build or excavate, change the list and indicate.
				local isExcavation = self:GetStringValue("ConstructorMode") ~= "Spray";
				table.insert(self.orderSequence, isExcavation);
				table.insert(isExcavation and self.excavationOrders or self.constructionOrders, order);
			end
				
			if not ctrl:IsState(Controller.WEAPON_PRIMARY_HOTKEY) then
				local ultraLowBoundX = math.huge;
				local ultraLowBoundY = math.huge;
				local ultraHighBoundX = -math.huge;
				local ultraHighBoundY = -math.huge;

				for _, tempInfo in pairs(self.blueprintCornersPerSize) do
					local tempInfo = self.blueprintCornersPerSize[_];

					ultraLowBoundX = math.min(tempInfo.leastX, ultraLowBoundX);
					ultraLowBoundY = math.min(tempInfo.leastY, ultraLowBoundY);
					ultraHighBoundX = math.max(tempInfo.greatestX + _, ultraHighBoundX);
					ultraHighBoundY = math.max(tempInfo.greatestY + _, ultraHighBoundY);
				end

				local bluePrintDisplayColor = self:GetStringValue("ConstructorMode") ~= "Spray" and displayColorRed or displayColorBlue;

				local corner = Vector(ultraLowBoundX - 3, ultraLowBoundY - 3);
				PrimitiveMan:DrawLinePrimitive(screen, corner, corner + Vector( 5,  0), bluePrintDisplayColor);
				PrimitiveMan:DrawLinePrimitive(screen, corner, corner + Vector( 0,  5), bluePrintDisplayColor);
				corner = Vector(ultraLowBoundX - 3, ultraHighBoundY + 2);
				PrimitiveMan:DrawLinePrimitive(screen, corner, corner + Vector( 5,  0), bluePrintDisplayColor);
				PrimitiveMan:DrawLinePrimitive(screen, corner, corner + Vector( 0, -5), bluePrintDisplayColor);
				corner = Vector(ultraHighBoundX + 2, ultraHighBoundY + 2);
				PrimitiveMan:DrawLinePrimitive(screen, corner, corner + Vector(-5,  0), bluePrintDisplayColor);
				PrimitiveMan:DrawLinePrimitive(screen, corner, corner + Vector( 0, -5), bluePrintDisplayColor);
				corner = Vector(ultraHighBoundX + 2, ultraLowBoundY - 3);
				PrimitiveMan:DrawLinePrimitive(screen, corner, corner + Vector(-5,  0), bluePrintDisplayColor);
				PrimitiveMan:DrawLinePrimitive(screen, corner, corner + Vector( 0,  5), bluePrintDisplayColor);

				for _, tempList in pairs(self.blueprintBlocksPerSize) do
					for i, block in pairs(tempList) do
						PrimitiveMan:DrawBoxPrimitive(screen, Vector(block[1], block[2]), Vector(block[1] + _ - 1, block[2] + _ - 1), bluePrintDisplayColor);
					end
				end
			end
			
			if not ((not self.ignorePieMenu and ctrl:IsState(Controller.PIE_MENU_ACTIVE)) or ctrl:IsState(Controller.ACTOR_NEXT_PREP) or ctrl:IsState(Controller.ACTOR_PREV_PREP)) and playerControlled then
				-- go through and disable all 41 controller states when moving the build cursor
				for state = 0, 40 do
					ctrl:SetState(state, false);
				end
			end
		end

		-- Draw all build orders
		for i, constructionOrder in pairs(self.constructionOrders) do
			constructionSequence = constructionOrder.orderSequence;
			constructionBlockSpaces = constructionOrder.orderBlockSpaces;
			constructionInfo = constructionOrder.orderInfo;

			local displayColor = displayColorGreen;
			local unavailableColor = displayColorRed;
			
			for _, blockIndex in ipairs(constructionSequence) do
				local size = blockIndex[1];
				local majorX = blockIndex[2];
				local majorY = blockIndex[3];
				local cellIndex = blockIndex[4];
				local grid = constructionBlockSpaces[size];
				local column = grid[majorX];
				local cell = column[majorY];
				local block = cell[cellIndex];
				local fullX = majorX * size + block.X;
				local fullY = majorY * size + block.Y;

				if not self.operatedByAI then
					if SceneMan:ShortestDistance(muzzlePosition, Vector(fullX + size / 2, fullY + size / 2), true):MagnitudeIsLessThan(self.buildDistance) then
						PrimitiveMan:DrawBoxPrimitive(screen, Vector(fullX, fullY), Vector(fullX + size - 1, fullY + size - 1), displayColor);
					else
						PrimitiveMan:DrawBoxPrimitive(screen, Vector(fullX, fullY), Vector(fullX + size - 1, fullY + size - 1), displayColorRed);
					end
				end
			end

			local corner = Vector(constructionInfo.ultraLowBoundX - 3, constructionInfo.ultraLowBoundY - 3);
			PrimitiveMan:DrawTriangleFillPrimitive(screen, corner + Vector( 2,  0), corner, corner + Vector( 0,  2), displayColor);

			corner = Vector(constructionInfo.ultraLowBoundX - 3, constructionInfo.ultraHighBoundY + 2);
			PrimitiveMan:DrawTriangleFillPrimitive(screen, corner + Vector( 2,  0), corner, corner + Vector( 0, -2), displayColor);

			corner = Vector(constructionInfo.ultraHighBoundX + 2, constructionInfo.ultraHighBoundY + 2);
			PrimitiveMan:DrawTriangleFillPrimitive(screen, corner + Vector(-2,  0), corner, corner + Vector( 0, -2), displayColor);

			corner = Vector(constructionInfo.ultraHighBoundX + 2, constructionInfo.ultraLowBoundY - 3);
			PrimitiveMan:DrawTriangleFillPrimitive(screen, corner + Vector(-2,  0), corner, corner + Vector( 0,  2), displayColor);

			-- But only act upon the first item listed
			if i == 1 then
				if constructionSequence[1] then
					if self.resource >= self.cellCost then
						local blockIndex = constructionSequence[1];
						local size = blockIndex[1];
						local majorX = blockIndex[2];
						local majorY = blockIndex[3];
						local cellIndex = blockIndex[4];
						local grid = constructionBlockSpaces[size];
						local column = grid[majorX];
						local cell = column[majorY];
						local block = cell[cellIndex];
						local fullX = majorX * size + block.X;
						local fullY = majorY * size + block.Y;

						if SceneMan:ShortestDistance(muzzlePosition, Vector(fullX + size / 2, fullY + size / 2), true):MagnitudeIsLessThan(self.buildDistance) then
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

											local whichBorder = ConstructorCheckPixelTypeAgainstBuildPlan(constructionBlockSpaces, pos.X, pos.Y, { X = fullX, Y = fullY }, size);

											if whichBorder == 0 then -- Normal
												name = "Constructor Tile " .. math.random(16);
											elseif whichBorder == 1 then -- Inner border
												self.colorCandidates = { 8, 11, 12, 6, 13 };
												name = "Constructor Tile " .. self.colorCandidates[math.random(#self.colorCandidates)];
											else -- Outter border
												name = "Constructor Border Tile " .. math.random(4);
											end
								
											local terrainObject = CreateTerrainObject(name, self.ModuleName);
											terrainObject.Pos = pos;
											SceneMan:AddSceneObject(terrainObject);
											didBuild = true;
											totalCost = totalCost + (1 - strengthRatio) / cellSize / cellSize;
										end
									end
								end

								if didBuild then
									self.resource = self.resource - (self.cellCost * totalCost);
									local buildPos = self.Pos + SceneMan:ShortestDistance(self.Pos, Vector(bx + fullX + (cellSize - 1), by + fullY + (cellSize - 1)), true);

									for otherPlayer = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
										local otherScreen = ActivityMan:GetActivity():ScreenOfPlayer(otherPlayer);

										if otherScreen ~= -1 and (otherScreen == screen or not SceneMan:IsUnseen(buildPos.X, buildPos.Y, ActivityMan:GetActivity():GetTeamOfPlayer(otherPlayer))) then
											PrimitiveMan:DrawBoxFillPrimitive(otherScreen, Vector(bx + fullX + 1, by + fullY + 1), Vector(bx + fullX + cellSize, by + fullY + cellSize), displayColorGray);
										end
									end

									if screen ~= -1 then
										PrimitiveMan:DrawLinePrimitive(screen, self.Pos, buildPos, displayColor);
									end

									self.buildSound.Volume = totalCost;
									self.buildSound.Pitch = 2 - totalCost;
									self.buildSound:Play(buildPos);

									if block.C == cellsPerBlock then
										table.remove(constructionSequence, 1);
									end
								end
							else
								table.remove(constructionSequence, 1);
							end
						else
							table.insert(constructionSequence, table.remove(constructionSequence, 1));
						end
					end
				else
					table.remove(self.constructionOrders, 1);

					for i, orderIsExcavation in ipairs(self.orderSequence) do
						if not orderIsExcavation then
							if i == 1 then
								actor:ClearAIWaypoints();
							end

							table.remove(self.orderSequence, i);
							break;
						end
					end
				end
			end
		end

		for i, digOrder in pairs(self.excavationOrders) do
			excavationSequence = digOrder.orderSequence;
			excavationBlockSpaces = digOrder.orderBlockSpaces;
			excavationInfo = digOrder.orderInfo;

			local displayColor = displayColorOrange;
			local unavailableColor = displayColorRed;
			
			for _, blockIndex in ipairs(excavationSequence) do
				local size = blockIndex[1];
				local majorX = blockIndex[2];
				local majorY = blockIndex[3];
				local cellIndex = blockIndex[4];
				local grid = excavationBlockSpaces[size];
				local column = grid[majorX];
				local cell = column[majorY];
				local block = cell[cellIndex];
				local fullX = majorX * size + block.X;
				local fullY = majorY * size + block.Y;
				if not self.operatedByAI then
					if SceneMan:ShortestDistance(muzzlePosition, Vector(fullX + size / 2, fullY + size / 2), true):MagnitudeIsLessThan(self.excavationDistance) then
						PrimitiveMan:DrawBoxPrimitive(screen, Vector(fullX, fullY), Vector(fullX + size - 1, fullY + size - 1), displayColor);
					else
						PrimitiveMan:DrawBoxPrimitive(screen, Vector(fullX, fullY), Vector(fullX + size - 1, fullY + size - 1), unavailableColor);
					end
				end
			end

			local corner = Vector(excavationInfo.ultraLowBoundX - 3, excavationInfo.ultraLowBoundY - 3);
			PrimitiveMan:DrawTriangleFillPrimitive(screen, corner + Vector( 2,  0), corner, corner + Vector( 0,  2), displayColor);

			corner = Vector(excavationInfo.ultraLowBoundX - 3, excavationInfo.ultraHighBoundY + 2);
			PrimitiveMan:DrawTriangleFillPrimitive(screen, corner + Vector( 2,  0), corner, corner + Vector( 0, -2), displayColor);

			corner = Vector(excavationInfo.ultraHighBoundX + 2, excavationInfo.ultraHighBoundY + 2);
			PrimitiveMan:DrawTriangleFillPrimitive(screen, corner + Vector(-2,  0), corner, corner + Vector( 0, -2), displayColor);

			corner = Vector(excavationInfo.ultraHighBoundX + 2, excavationInfo.ultraLowBoundY - 3);
			PrimitiveMan:DrawTriangleFillPrimitive(screen, corner + Vector(-2,  0), corner, corner + Vector( 0,  2), displayColor);

			-- only act upon the first item in the list
			if i == 1 then
				if excavationSequence[1] then
					local cellSize = self.cellSize;

					local closest = 1;
					local leastDistSquared = math.huge;

					for i, blockIndex in pairs(excavationSequence) do
						local size = blockIndex[1];
						local majorX = blockIndex[2];
						local majorY = blockIndex[3];
						local cellIndex = blockIndex[4];

						local grid = excavationBlockSpaces[size];
						local column = grid[majorX];
						local cell = column[majorY];
						local block = cell[cellIndex];

						local fullX = majorX * size + block.X + size / 2;
						local fullY = majorY * size + block.Y + size / 2;
						
						local distSquared = math.pow(fullX - muzzlePosition.X, 2) + math.pow(fullY - muzzlePosition.Y, 2);

						if distSquared < leastDistSquared then
							leastDistSquared = distSquared;
							closest = i;
						end
					end
					
					local blockIndex = excavationSequence[closest];

					local size = blockIndex[1];
					local majorX = blockIndex[2];
					local majorY = blockIndex[3];
					local cellIndex = blockIndex[4];

					local grid = excavationBlockSpaces[size];
					local column = grid[majorX];
					local cell = column[majorY];
					local block = cell[cellIndex];

					local fullX = majorX * size + block.X;
					local fullY = majorY * size + block.Y;
						
					if SceneMan:ShortestDistance(muzzlePosition, Vector(fullX + size / 2, fullY + size / 2), true):MagnitudeIsLessThan(self.excavationDistance) then
						local oneThirdBlock = size/cellSize;
						local cellsPerBlock = oneThirdBlock^2;

						if block.C < cellsPerBlock then
							local by = math.floor(block.C/oneThirdBlock);
							local bx = block.C - (by * oneThirdBlock);
							by = by * cellSize - 1;
							bx = bx * cellSize - 1;
							local totalCost = 0;
							local startPos = ConstructorWrapPos(Vector(bx + fullX + math.ceil(cellSize / 2), by + fullY + math.ceil(cellSize / 2)));
							local digWeightTotal = 0;
							local totalVel = Vector();
							local found = 0;
							local clear = true;
							local digThreshold = RangeRand(0.5, 4) * self.digStrength;

							for xOffset = -math.ceil(cellSize / 2) + 1, math.floor(cellSize / 2) do
								for yOffset = -math.ceil(cellSize / 2) + 1, math.floor(cellSize / 2) do
									local checkPos = ConstructorWrapPos(Vector(startPos.X + xOffset, startPos.Y + yOffset));
									local terrCheck = SceneMan:GetTerrMatter(checkPos.X, checkPos.Y);
									local material = SceneMan:GetMaterialFromID(terrCheck);

									if material.ID ~= rte.airID and material.StructuralIntegrity <= self.digStrength then
										clear = false;
									end

									if material.StructuralIntegrity <= self.digStrength and material.StructuralIntegrity <= RangeRand(0.5, 1.05) * self.digStrength and material.StructuralIntegrity <= digThreshold then
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
												digThreshold = digThreshold - material.StructuralIntegrity;
											end

											px.IgnoreTerrain = true;
											px.Vel = (px.Pos - self.Pos):SetMagnitude(-speed):RadRotate(RangeRand(-0.5, 0.5));
											totalVel = totalVel + px.Vel;
											px:AddScript("Base.rte/Devices/Tools/Constructor/ConstructorCollect.lua");
											
											found = found + 1;
										end
									end
								end
							end

							if clear then
								block.C = block.C + 1;
							end

							if found > 0 then
								if digWeightTotal > 0 then
									digWeightTotal = digWeightTotal/9;
									self.resource = math.min(self.resource + digWeightTotal * self.cellCost, self.maxResource);
								end

								local collectFX = CreateMOPixel("Particle Constructor Gather Material" .. (digWeightTotal > 0.5 and " Big" or ""));
								collectFX.Vel = totalVel/found;
								collectFX.Pos = Vector(startPos.X + math.random(cellSize), startPos.Y + math.random(cellSize)) + collectFX.Vel * rte.PxTravelledPerFrame;

								MovableMan:AddParticle(collectFX);

								self.resource = self.resource - (self.cellCost * totalCost);
								local digPos = self.Pos + SceneMan:ShortestDistance(self.Pos, Vector(bx + fullX + (cellSize - 1), by + fullY + (cellSize - 1)), true);

								for otherPlayer = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
									local otherScreen = ActivityMan:GetActivity():ScreenOfPlayer(otherPlayer);

									if otherScreen ~= -1 and (otherScreen == screen or not SceneMan:IsUnseen(digPos.X, digPos.Y, ActivityMan:GetActivity():GetTeamOfPlayer(otherPlayer))) then
										PrimitiveMan:DrawBoxFillPrimitive(otherScreen, Vector(bx + fullX + 1, by + fullY + 1), Vector(bx + fullX + cellSize, by + fullY + cellSize), displayColorGray);
									end
								end

								if screen ~= -1 then
									PrimitiveMan:DrawLinePrimitive(screen, muzzlePosition, digPos, displayColor);
								end

								if block.C == cellsPerBlock then
									table.remove(excavationSequence, closest);
								end
							end
						else
							table.remove(excavationSequence, closest);
						end
					end
				else
					table.remove(self.excavationOrders, 1);
					
					for i, orderIsExcavation in ipairs(self.orderSequence) do
						if orderIsExcavation then
							if i == 1 then
								actor:ClearAIWaypoints();
							end

							table.remove(self.orderSequence, i);
							break;
						end
					end
				end
			end
		end

		if ctrl:IsState(Controller.PIE_MENU_ACTIVE) then
			PrimitiveMan:DrawTextPrimitive(screen, actor.AboveHUDPos + Vector(0, 26), "Mode: ".. self:GetStringValue("ConstructorMode"), true, 1);
		end

		if self.cursor then
			local cursorColor = self:GetStringValue("ConstructorMode") ~= "Spray" and displayColorYellow or displayColorWhite;

			if precise then
				PrimitiveMan:DrawLinePrimitive(screen, self.cursor + Vector(2, 2), self.cursor + Vector(-3, -3), cursorColor);
				PrimitiveMan:DrawLinePrimitive(screen, self.cursor + Vector(2, -3), self.cursor + Vector(-3, 2), cursorColor);
			else
				PrimitiveMan:DrawLinePrimitive(screen, self.cursor + Vector(0, 4), self.cursor + Vector(0, -4), cursorColor);
				PrimitiveMan:DrawLinePrimitive(screen, self.cursor + Vector(4, 0), self.cursor + Vector(-4, 0), cursorColor);
			end

			PrimitiveMan:DrawBoxPrimitive(screen, map, map + Vector(self.blockSize - 1, self.blockSize - 1), cursorColor);
		end

		if display then
			self.displayTimer:Reset();
		end
	elseif self.cursor then
		self.cursor = nil;
	end
end