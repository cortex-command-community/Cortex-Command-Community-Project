local setupMinionAIMode = function(self, minion)
	if self.minionsShouldGather then
		minion.AIMode = Actor.AIMODE_GOTO;
		minion:ClearMovePath();
		minion:AddAIMOWaypoint(self);
	else
		minion.AIMode = Actor.AIMODE_SENTRY;
		local random = math.random();
		if random < 0.25 then
			minion.AIMode = Actor.AIMODE_PATROL;
		elseif random < 0.35 then
			local target = MovableMan:GetClosestEnemyActor(minion.Team, self.Pos, self.enemySearchRadius, Vector());
			if target then
				minion.AIMode = Actor.AIMODE_GOTO;
				minion:ClearMovePath();
				minion:AddAIMOWaypoint(target);
			end
		elseif random < 0.45 then
			minion.AIMode = Actor.AIMODE_BRAINHUNT;
		end
	end
end

local handleMinionSpawning = function(self)
	if self.spawnTimer:IsPastSimTimeLimit() then
		self.spawnTimer:Reset();
		self.spawnTimer:SetSimTimeLimitMS(self.baseSpawnTime);
		local funds = ActivityMan:GetActivity():GetTeamFunds(self.Team);
		local enemyCount = 0;
		if funds >= self.spawnCost or self.GoldCarried >= self.spawnCost then
			local spawnPos = self.Pos + Vector(0, self.Radius + RangeRand(0, self.spawnRadius)):RadRotate(RangeRand(-1, 1));
			if SceneMan:CastMaxStrengthRay(self.Pos, spawnPos, 8) < self.spawnTerrainTolerance * 2 then
				local pointCount = 10;
				local radius = self.minionTemplate.Radius/math.sqrt(pointCount);
				local totalSurroundingTerrain = 0;
				local totalTolerance = self.spawnTerrainTolerance * pointCount;
				
				for i = 0, pointCount do
					local checkPos = spawnPos + Vector(math.sqrt(i) * radius, 0):RadRotate(i * 2.39996);
					local terrCheck = SceneMan:GetTerrMatter(checkPos.X, checkPos.Y);
					if terrCheck == rte.airID then
						totalSurroundingTerrain = totalSurroundingTerrain + self.spawnTerrainTolerance * 2;	--Treat air as unsuitable terrain to spawn from
					else
						totalSurroundingTerrain = totalSurroundingTerrain + SceneMan:GetMaterialFromID(terrCheck).StructuralIntegrity;
					end
					if totalSurroundingTerrain > totalTolerance then
						break;
					end
				end

				if totalSurroundingTerrain < totalTolerance then
					local weapon = self.weapons[math.random(#self.weapons)]:Clone();
					weapon.DeleteWhenRemovedFromParent = true;
					
					local newMinion = self.minionTemplate:Clone();
					newMinion:AddInventoryItem(weapon);
					newMinion.Pos = spawnPos;
					newMinion.HFlipped = self.HFlipped;
					newMinion.RotAngle = RangeRand(-1.5, 1.5);
					self:setupMinionAIMode(newMinion);
					MovableMan:AddActor(newMinion);
					table.insert(self.minions, newMinion);
		
					if self.GoldCarried >= self.spawnCost then
						self.GoldCarried = self.GoldCarried - self.spawnCost;
					else
						ActivityMan:GetActivity():SetTeamFunds(funds - self.spawnCost, self.Team);
					end

					self.spawnTimer:SetSimTimeLimitMS(self.baseSpawnTime * #self.minions);
							
					local spawnSlice = self.minionManagementSubPieMenu:GetFirstPieSliceByPresetName(self.enableMinionSpawning and "DisableMinionSpawning" or "EnableMinionSpawning");
					if spawnSlice then
						spawnSlice.Description = self.enableMinionSpawning and "Let the dead sleep..." or "Raise the living dead!";
					end

					return;
				end
			end
		end
		if #self.minions == 0 and #self.frenziedMinions == 0 then
			local spawnSlice = self.minionManagementSubPieMenu:GetFirstPieSliceByPresetName(self.enableMinionSpawning and "DisableMinionSpawning" or "EnableMinionSpawning");
			if spawnSlice then
				spawnSlice.Description = self.enableMinionSpawning and "From dust they came..." or "...To dust they return.";
			end
		end
	end
end

local cleanupDeadMinions = function(self)
	for i = 1, #self.minions do
		if not MovableMan:IsActor(self.minions[i]) or self.minions[i].Health <= 0 then
			table.remove(self.minions, i);
		end
	end

	for i = 1, #self.frenziedMinions do
		if not MovableMan:IsActor(self.frenziedMinions[i]) or self.frenziedMinions[i].Health <= 0 then
			table.remove(self.frenziedMinions, i);
		end
	end
end

local updateMinions = function(self)
	if self.updateMinionTimer:IsPastSimTimeLimit() then
		self.updateMinionTimer:Reset();
			
		self:cleanupDeadMinions();
		self.minionsFrenzyPieSlice.Enabled = false;
		
		local enemyCount = 0;
		for actor in MovableMan.Actors do
			if actor.Team ~= self.Team and actor.Team ~= Activity.NOTEAM and (actor.ClassName == "AHuman" or actor.ClassName == "ACrab") then
				enemyCount = enemyCount + 1;
			end
		end
		
		for i = 1, #self.minions do
			local minion = self.minions[i];
			
			if SceneMan:ShortestDistance(self.Pos, minion.Pos, SceneMan.SceneWrapsX).Magnitude > self.minionDecayRadius then
				minion.Health = minion.Health - 1;
				for attachable in minion.Attachables do
					local smoke = CreateMOSParticle("Small Smoke Ball 1");
					smoke.Pos = attachable.Pos;
					smoke.Lifetime = smoke.Lifetime * RangeRand(0.25, 1.5);
					MovableMan:AddParticle(smoke);
					if minion.Health < math.random(10) then
						minion:RemoveAttachable(attachable, true, true);
						if minion.Health > 0 then
							break;
						end
					end
				end
				if minion.Health <= 0 then
					ActivityMan:GetActivity():ReportDeath(minion.Team, -1);
				end
			end
			self.minionsFrenzyPieSlice.Enabled = true;
			
			if self.isAIControlled and #self.minions > enemyCount then
				self:SetNumberValue("MinionsFrenzy", 1);
			end
		end

		self:updateFrenziedMinions();
	end
	if self:IsPlayerControlled() and self.HUDVisible then
		for _, minion in pairs(self.minions) do
			if minion.Age > 1000 then
				PrimitiveMan:DrawBitmapPrimitive(ActivityMan:GetActivity():ScreenOfPlayer(self:GetController().Player), minion.AboveHUDPos + Vector(0, math.sin(self.Age * 0.01) * 2 - 3), self.indicatorArrow, self.Team, 0, false, false);
			end
		end
	end
end

local updateFrenziedMinions = function(self)
	for i = 1, #self.frenziedMinions do
		local frenziedMinion = self.frenziedMinions[i];
		local isBrainhunting = frenziedMinion.AIMode == Actor.AIMODE_BRAINHUNT;
		
		if math.random() < 0.25 then
			frenziedMinion.AIMode = isBrainhunting and Actor.AIMODE_GOTO or Actor.AIMODE_BRAINHUNT;
			isBrainhunting = not isBrainhunting;
			if not isBrainhunting then
				local target = MovableMan:GetClosestEnemyActor(frenziedMinion.Team, self.Pos, self.enemySearchRadius, Vector());
				if target then
					frenziedMinion.AIMode = Actor.AIMODE_GOTO;
					frenziedMinion:AddAIMOWaypoint(target);
				else
					frenziedMinion.AIMode = Actor.AIMODE_BRAINHUNT;
				end
			end
		end

		if not isBrainhunting then
			if not frenziedMinion.MOMoveTarget or not MovableMan:IsActor(frenziedMinion.MOMoveTarget) then
				frenziedMinion.AIMode = Actor.AIMODE_BRAINHUNT;
				isBrainhunting = true;
			end
		end
	end
end

function Create(self)
	self.setupMinionAIMode = setupMinionAIMode;
	self.handleMinionSpawning = handleMinionSpawning;
	self.cleanupDeadMinions = cleanupDeadMinions;
	self.updateMinions = updateMinions;
	self.updateFrenziedMinions = updateFrenziedMinions;

	self.isAIControlled = not ActivityMan:GetActivity():PlayerHuman(self:GetController().Player);
	if self.isAIControlled then
		self.enableMinionSpawning = true;
		self:RemoveNumberValue("EnableNecromancy");
		self.minionsShouldGather = false;
		self:RemoveNumberValue("MinionsGather");
	end

	self.minionTemplate = CreateAHuman("Skeleton", "Uzira.rte");
	self.minionTemplate.Team = self.Team;
	self.minionTemplate.HUDVisible = false;
	self.minionTemplate.PlayerControllable = false;
	self.minionTemplate.PinStrength = self.minionTemplate.Mass;
	self.minionTemplate:AddScript("Uzira.rte/Actors/Shared/Undead.lua");

	self.weapons = {CreateHDFirearm("Blunderpop", "Uzira.rte"), CreateHDFirearm("Blunderbuss", "Uzira.rte"), CreateHDFirearm("Musket", "Uzira.rte"), CreateHDFirearm("Boomstick", "Uzira.rte"), CreateHDFirearm("Crossbow", "Uzira.rte")};
	
	self.spawnRadius = math.max(FrameMan.PlayerScreenWidth, FrameMan.PlayerScreenHeight) * 0.1;
	self.minionDecayRadius = self.spawnRadius * 4 + self.minionTemplate.Radius;
	self.enemySearchRadius = self.minionDecayRadius;
	self.spawnTerrainTolerance = 70;
	self.spawnCost = self.minionTemplate:GetGoldValue(0, 1, 1);
	
	self.minions = {};
	self.frenziedMinions = {};

	self.spawnTimer = Timer();
	self.baseSpawnTime = 750;
	self.spawnTimer:SetSimTimeLimitMS(self.baseSpawnTime);
	
	self.updateMinionTimer = Timer();
	self.updateMinionTimer:SetSimTimeLimitMS(500);
	
	self.minionManagementSubPieMenu = self.PieMenu:GetFirstPieSliceByPresetName("MinionManagement").SubPieMenu;
	
	self.enableMinionSpawning = self:NumberValueExists("EnableMinionSpawning") and (self:GetNumberValue("EnableMinionSpawning") ~= 0) or true;
	self.minionManagementSubPieMenu:RemovePieSlicesByPresetName((self.enableMinionSpawning and "EnableMinionSpawning" or "DisableMinionSpawning"));
	
	self.minionsShouldGather = self:NumberValueExists("MinionsGather") and (self:GetNumberValue("MinionsGather") ~= 0) or false;
	self.minionManagementSubPieMenu:RemovePieSlicesByPresetName((self.minionsShouldGather and "MinionsGather" or "MinionsStandby"));
	
	self.minionsFrenzyPieSlice = self.minionManagementSubPieMenu:GetFirstPieSliceByPresetName("MinionsFrenzy");
	self.minionsFrenzyPieSlice.Enabled = false;
	
	self.indicatorArrow = CreateMOSParticle("Indicator Arrow", "Uzira.rte");
end

function Update(self)
	if self.isAIControlled and self:IsPlayerControlled() then
		self.isAIControlled = false;
	end
	
	if self:NumberValueExists("EnableMinionSpawning") then
		self.enableMinionSpawning = self:GetNumberValue("EnableMinionSpawning") ~= 0;
		self:RemoveNumberValue("EnableMinionSpawning");
		
		local sliceNameToRemove = self.enableMinionSpawning and "EnableMinionSpawning" or "DisableMinionSpawning";
		local sliceNameToAdd = self.enableMinionSpawning and "DisableMinionSpawning" or "EnableMinionSpawning";
		self.minionManagementSubPieMenu:ReplacePieSlice(self.minionManagementSubPieMenu:GetFirstPieSliceByPresetName(sliceNameToRemove), CreatePieSlice(sliceNameToAdd, "Uzira.rte"));

		local newSlice = self.minionManagementSubPieMenu:GetFirstPieSliceByPresetName(sliceNameToAdd);
		if #self.minions == 0 and #self.frenziedMinions == 0 then
			newSlice.Description = self.enableMinionSpawning and "From dust they came..." or "...To dust they return.";
		else
			newSlice.Description = self.enableMinionSpawning and "Let the dead sleep..." or "Raise the living dead!";
		end
	end
	
	if self:NumberValueExists("MinionsGather") then
		self.minionsShouldGather = self:GetNumberValue("MinionsGather") ~= 0;
		self:RemoveNumberValue("MinionsGather");
		
		local sliceNameToRemove = self.minionsShouldGather and "MinionsGather" or "MinionsStandby";
		local sliceNameToAdd = self.minionsShouldGather and "MinionsStandby" or "MinionsGather";
		self.minionManagementSubPieMenu:ReplacePieSlice(self.minionManagementSubPieMenu:GetFirstPieSliceByPresetName(sliceNameToRemove), CreatePieSlice(sliceNameToAdd, "Uzira.rte"));
		
		self:cleanupDeadMinions();
		for i = 1, #self.minions do
			self:setupMinionAIMode(self.minions[i]);
		end
	end
	
	if self:NumberValueExists("MinionsFrenzy") then
		self:RemoveNumberValue("MinionsFrenzy");
	
		self:cleanupDeadMinions();
		for i = 1, #self.minions do
			self.frenziedMinions[#self.frenziedMinions + 1] = self.minions[i];
		end
		
		self.minions = {};
		self:updateFrenziedMinions();
	end
	
	if self.enableMinionSpawning then
		self:handleMinionSpawning();
	else
		self.spawnTimer:Reset();
	end
	
	self:updateMinions();
end