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
	return Vector(math.floor((checkPos.X - blockSize/2)/blockSize) * blockSize + blockSize/2, math.floor((checkPos.Y - blockSize/2)/blockSize) * blockSize + blockSize/2);
end

function ConstructorTerrainRay(start, trace, skip)
	local hitPos = start + trace;
	SceneMan:CastStrengthRay(start, trace, 0, hitPos, skip, rte.airID, SceneMan.SceneWrapsX);	
	return hitPos;
end

function Create(self)
	self.displayTimer = Timer();

	self.buildTimer = Timer();
	self.buildList = {};
	self.buildCost = 10;	--How much resource is required per one build 3 x 3 px piece
	self.sprayCost = self.buildCost * 0.5;

	self.buildSize = 24;
	self.buildSizeMin = self.buildSize/4;
	self.buildSizeMax = self.buildSize;
	self.fullBlock = 64 * self.buildCost;	--One full 24x24 block of concrete requires 64 units of resource
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
				self.menu_ignore = false
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
					self.buildList = {};
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
						self.buildList[#self.buildList + 1] = buildThis;
					end
				end
			end

			-- constructor actions if it's AI controlled
			if self.operatedByAI then
				if self.tunnelFillTimer:IsPastSimMS(self.tunnelFillDelay * self.aiSkillRatio) and #self.buildList == 0 then
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
								for i = 1, #self.buildList do
									if self.buildList[i] ~= nil and self.buildList[i][1] == mapX and self.buildList[i][2] == mapY then
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
									self.buildList[#self.buildList + 1] = buildThis;
								end
							end
						end
					end
				end
			end
		else
			self.toAutoBuild = false;
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

			self.buildList = {};
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
		if self.displayTimer:IsPastSimMS(TimerMan.DeltaTimeMS) then
			self.displayTimer:Reset();
			-- flickering colors
			displayColorBlue = 195;
			displayColorYellow = 116;
			displayColorRed = 12;
			displayColorWhite = 252;
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

				if ctrl:IsState(Controller.HOLD_DOWN) or ctrl:IsState(Controller.BODY_CROUCH) then
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
				self.buildSize = self.buildSize/2;
				if self.buildSize < self.buildSizeMin then
					self.buildSize = self.buildSizeMax;
				end
			end

			if cursorMovement:MagnitudeIsGreaterThan(0) then
				self.cursor = self.cursor + (mouseControlled and cursorMovement or cursorMovement:SetMagnitude(self.cursorMoveSpeed * (aiming and 0.5 or 1)));
			end

			local precise = not mouseControlled and aiming;
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
			elseif playerControlled then
				-- add blocks to the build queue if the cursor is firing
				if ctrl:IsState(Controller.WEAPON_FIRE) then
					local freeSlot = true;
					for i = 1, #self.buildList do
						if self.buildList[i] and self.buildList[i][1] == map.X and self.buildList[i][2] == map.Y then
							freeSlot = false;
							break;
						end
					end
					if freeSlot then
						local buildThis = {};
						buildThis[1] = map.X;
						buildThis[2] = map.Y;
						buildThis[3] = 0;
						buildThis[4] = self.buildSize;
						self.buildList[#self.buildList + 1] = buildThis;
					end
				end
				for state = 0, 40 do	-- go through and disable all 41 controller states when moving the build cursor
					ctrl:SetState(state, false);
				end
			else
				self.cursor = nil;
			end
		end

		-- clean up the build list of nil slots and draw the squares to show the build layout
		local tempList = {};
		for i = 1, #self.buildList do
			if self.buildList[i] ~= nil then
				tempList[#tempList + 1] = self.buildList[i];
				if not self.operatedByAI then
					if SceneMan:ShortestDistance(actor.Pos, Vector(self.buildList[i][1], self.buildList[i][2]), SceneMan.SceneWrapsX):MagnitudeIsLessThan(self.buildDistance) then
						PrimitiveMan:DrawBoxPrimitive(screen, Vector(self.buildList[i][1], self.buildList[i][2]), Vector(self.buildList[i][1] + self.buildList[i][4] - 1, self.buildList[i][2] + self.buildList[i][4] - 1), displayColorBlue);
					else
						PrimitiveMan:DrawBoxPrimitive(screen, Vector(self.buildList[i][1], self.buildList[i][2]), Vector(self.buildList[i][1] + self.buildList[i][4] - 1, self.buildList[i][2] + self.buildList[i][4] - 1), displayColorRed);
					end
				end
			end
		end

		self.buildList = tempList;

		-- building up the first block in the build queue
		if self.resource >= self.buildCost and self.buildList[1] then
			if SceneMan:ShortestDistance(actor.Pos, Vector(self.buildList[1][1], self.buildList[1][2]), SceneMan.SceneWrapsX):MagnitudeIsLessThan(self.buildDistance) then
				--TODO: experiment with different cell sizes?
				local cellSize = 3;
				local oneThirdBlock = self.buildList[1][4]/cellSize;
				local cellsPerBlock = oneThirdBlock^2;
				if self.buildList[1][3] < cellsPerBlock then
					local by = math.floor(self.buildList[1][3]/oneThirdBlock);
					local bx = self.buildList[1][3] - (by * oneThirdBlock);
					by = by * cellSize - 1;
					bx = bx * cellSize - 1;

					self.buildList[1][3] = self.buildList[1][3] + 1;
					local totalCost = 0;
					local startPos = ConstructorWrapPos(Vector(bx + self.buildList[1][1], by + self.buildList[1][2]));
					local didBuild = false;
					for x = 1, cellSize do
						for y = 1, cellSize do
							local pos = Vector(startPos.X + x, startPos.Y + y);
							local strengthRatio = SceneMan:GetMaterialFromID(SceneMan:GetTerrMatter(pos.X, pos.Y)).StructuralIntegrity/self.digStrength;
							if strengthRatio < 1 and SceneMan:GetMOIDPixel(pos.X, pos.Y) == rte.NoMOID then
								local name = "";
								if bx + x == 0 or bx + x == self.buildList[1][4] - 1 or by + y == 0 or by + y == self.buildList[1][4] - 1 then
									name = "Base.rte/Constructor Border Tile " .. math.random(4);
								else
									name = "Base.rte/Constructor Tile " .. math.random(16);
								end
								
								local terrainObject = CreateTerrainObject(name);
								terrainObject.Pos = pos;
								SceneMan:AddSceneObject(terrainObject);

								didBuild = true;
								totalCost = 1 - strengthRatio;
							end
						end
					end
					if didBuild then
						self.resource = self.resource - (self.buildCost * totalCost);
						local buildPos = self.Pos + SceneMan:ShortestDistance(self.Pos, Vector(bx + self.buildList[1][1] + (cellSize - 1), by + self.buildList[1][2] + (cellSize - 1)), SceneMan.SceneWrapsX);

						for otherPlayer = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
							local otherScreen = ActivityMan:GetActivity():ScreenOfPlayer(otherPlayer);
							if otherScreen ~= -1 and (otherScreen == screen or not SceneMan:IsUnseen(buildPos.X, buildPos.Y, ActivityMan:GetActivity():GetTeamOfPlayer(otherPlayer))) then
								PrimitiveMan:DrawBoxFillPrimitive(otherScreen, Vector(bx + self.buildList[1][1] + 1, by + self.buildList[1][2] + 1), Vector(bx + self.buildList[1][1] + cellSize, by + self.buildList[1][2] + cellSize), displayColorWhite);
							end
						end

						if screen ~= -1 then
							PrimitiveMan:DrawLinePrimitive(screen, self.Pos, buildPos, displayColorBlue);
						end

						self.buildSound.Volume = totalCost;
						self.buildSound.Pitch = 2 - totalCost;
						self.buildSound:Play(buildPos);

						if self.buildList[1][3] == cellsPerBlock then
							self.buildList[1] = nil;
						end
					end
				else
					self.buildList[1] = nil;
				end
			else
				self.buildList[#self.buildList + 1] = self.buildList[1];
				self.buildList[1] = nil;
			end
		end
		if display then
			self.displayTimer:Reset();
		end
	elseif self.cursor then
		self.cursor = nil;
	end
end