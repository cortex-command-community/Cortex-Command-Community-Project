function ThreadedUpdate(self)
	local gravity = (self.Vel + self.PrevVel)/2 - SceneMan.GlobalAcc * rte.PxTravelledPerFrame;
	self.RotAngle = gravity.AbsRadAngle;
	self.Frame = gravity:MagnitudeIsGreaterThan(5) and 0 or 1;
end