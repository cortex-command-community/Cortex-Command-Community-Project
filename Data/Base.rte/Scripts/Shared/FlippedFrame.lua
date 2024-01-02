--Use this script to display a different sprite frame when facing left, instead of the same sprite mirrored
function Update(self)
	self.Frame = self.HFlipped and 1 or 0;
end