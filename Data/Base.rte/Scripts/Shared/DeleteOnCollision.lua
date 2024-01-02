function OnCollideWithMO(self)
	self.ToDelete = true;
end

function OnCollideWithTerrain(self)
	self.ToDelete = true;
end