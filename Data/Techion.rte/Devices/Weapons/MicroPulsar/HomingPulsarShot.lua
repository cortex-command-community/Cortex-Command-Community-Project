function Create(self)
	self.adjustmentAmount = 1;

	self.delayTimer = Timer();

	local longDist = 800;
	local shortDist = 98;
	--To-do: rewrite this garbage targeting system?
	for mo in MovableMan:GetMOsInRadius(self.Pos, longDist, self.Team, true) do
		if mo and IsMOSRotating(mo) then
			local distCheck = SceneMan:ShortestDistance(self.Pos, mo.Pos, SceneMan.SceneWrapsX);
			if distCheck.Magnitude - mo.Radius < longDist then

				local toCheckPos = Vector(distCheck.Magnitude, 0):RadRotate(self.Vel.AbsRadAngle);
				local checkPos = self.Pos + toCheckPos;
				if SceneMan.SceneWrapsX == true then
					if checkPos.X > SceneMan.SceneWidth then
						checkPos = Vector(checkPos.X - SceneMan.SceneWidth, checkPos.Y);
					elseif checkPos.X < 0 then
						checkPos = Vector(SceneMan.SceneWidth + checkPos.X, checkPos.Y);
					end
				end

				local distCheck2 = SceneMan:ShortestDistance(checkPos, mo.Pos, SceneMan.SceneWrapsX);

				if distCheck2.Magnitude - mo.Radius < shortDist then

					if SceneMan:CastStrengthRay(self.Pos, toCheckPos, 0, Vector(), 3, rte.airID, SceneMan.SceneWrapsX) == false and SceneMan:CastStrengthRay(checkPos, distCheck2:SetMagnitude(distCheck2.Magnitude - mo.Radius), 0, Vector(), 3, 0, SceneMan.SceneWrapsX) == false then
						self.target = mo;
						longDist = distCheck.Magnitude - mo.Radius;
						shortDist = distCheck2.Magnitude - mo.Radius;
					end
				end
			end
		end
	end
end

function Update(self)
	if self.delayTimer:IsPastSimMS(25) and self.target and self.target.ID ~= rte.NoMOID then
		local checkVel = SceneMan:ShortestDistance(self.Pos, self.target.Pos, SceneMan.SceneWrapsX);
		checkVel = checkVel:SetMagnitude(checkVel.Magnitude - self.target.Radius);
		if SceneMan:CastStrengthRay(self.Pos, checkVel, 0, Vector(), 3, rte.airID, SceneMan.SceneWrapsX) == false then
			local aimVel = Vector(self.Vel.X, self.Vel.Y):SetMagnitude(1) + SceneMan:ShortestDistance(self.Pos, self.target.Pos, SceneMan.SceneWrapsX):SetMagnitude(self.adjustmentAmount);
			self.Vel = Vector(self.Vel.Magnitude, 0):RadRotate(aimVel.AbsRadAngle);
		end
	end
end