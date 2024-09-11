function Create(self)
	self.detonateTimer = Timer();
	self.fuseTriggered = false;

	self.smallAngle = math.pi/6;
	self.angleList = {};

	self.initFuse = math.random(4000, 5000);
	self.clusterCount = 8;
	self.clusterFuse = 1000;
end

function Update(self)
	if not self.fuseTriggered then
		if self:IsActivated() then
			self.fuseTriggered = true;
			self.detonateTimer:Reset();
		end
	else
		if self.detonateTimer:IsPastSimMS(self.initFuse) then
			self.angleList = {};
			for i = 1, 12 do
				local angleCheck = self.smallAngle * i;
				for i = 1, 5 do
					local checkPos = self.Pos + Vector(i, 0):RadRotate(angleCheck);
					if SceneMan.SceneWrapsX == true then
						if checkPos.X > SceneMan.SceneWidth then
							checkPos = Vector(checkPos.X - SceneMan.SceneWidth, checkPos.Y);
						elseif checkPos.X < 0 then
							checkPos = Vector(SceneMan.SceneWidth + checkPos.X, checkPos.Y);
						end
					end
					local terrCheck = SceneMan:GetTerrMatter(checkPos.X, checkPos.Y);
					if terrCheck ~= rte.airID then
						break;
					end
					if i == 5 then
						self.angleList[#self.angleList + 1] = angleCheck;
					end
				end
			end

			for i = 1, self.clusterCount do
				local minibomb = CreateAEmitter("Imperatus Scatter Bomb Piece");
				minibomb.Pos = self.Pos;
				minibomb.Sharpness = self.clusterFuse + 75 * i;
				if #self.angleList > 0 then
					minibomb.Vel = Vector(10 + math.random(10), 0):RadRotate(self.angleList[math.random(#self.angleList)] + RangeRand(-0.1, 0.1));
				else
					minibomb.Vel = Vector(10 + math.random(10), 0):DegRotate(45 * i + ((math.random() * 15) - 7.5));
				end
				MovableMan:AddParticle(minibomb);
			end
			
			self:GibThis();
		end
	end
end