function Create(self)
	self.rotationSpeed = 0.10;
	self.smoothedRotAngle = self.RotAngle;
end

function Update(self)
	if self.smoothedRotAngle ~= self.RotAngle then
		self.smoothedRotAngle = self.smoothedRotAngle - (self.rotationSpeed * (self.smoothedRotAngle - self.RotAngle));
	end

	self.MountedDeviceRotationOffset = self.smoothedRotAngle - self.RotAngle;
end