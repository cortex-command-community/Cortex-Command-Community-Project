function Create(self)
	self.delayTime = 100;
	
	self.pointCount = 30;
	self.spiralScale = 10;
end

function Update(self)
	if self.Age > self.delayTime then
		for i = 1, self.pointCount do
			local radius = self.spiralScale * math.sqrt(i);
			local checkPos = self.Pos + Vector(radius, 0):RadRotate(i * 2.39996);
			if SceneMan.SceneWrapsX == true then
				if checkPos.X > SceneMan.SceneWidth then
					checkPos = Vector(checkPos.X - SceneMan.SceneWidth, checkPos.Y);
				elseif checkPos.X < 0 then
					checkPos = Vector(SceneMan.SceneWidth + checkPos.X, checkPos.Y);
				end
			end
			
			local moCheck = SceneMan:GetMOIDPixel(checkPos.X, checkPos.Y);
			if moCheck ~= rte.NoMOID then
				local actor = MovableMan:GetMOFromID(MovableMan:GetMOFromID(moCheck).RootID);
				if MovableMan:IsActor(actor) and actor.Team ~= self.Team and actor.Radius > radius then
					self:GibThis();
					break;
				end
			end
		end
	end
end