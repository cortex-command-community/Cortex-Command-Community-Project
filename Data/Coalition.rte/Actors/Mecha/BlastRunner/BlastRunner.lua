function Update(self)
--[[
	if self:GetController():IsState(Controller.WEAPON_FIRE) then
		BlastRunnerDetonate(self);
	end
]]--
	if self:GetController():IsState(Controller.MOVE_LEFT) then
		self.AngularVel = math.min(25,self.AngularVel+5);
	end

	if self:GetController():IsState(Controller.MOVE_RIGHT) then
		self.AngularVel = math.max(-25,self.AngularVel-5);
	end

	local rayHitPos = Vector(0,0);
	if self:GetController():IsState(Controller.BODY_JUMP) and SceneMan:CastStrengthRay(self.Pos,Vector(0,(self.Radius+5)),0,rayHitPos,0,0,SceneMan.SceneWrapsX) == true then
		self.Vel = self.Vel + Vector(0,-2);
	end
end