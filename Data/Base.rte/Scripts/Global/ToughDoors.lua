function ToughDoorsScript:StartScript()
	self.multiplier = 4;
	self:CheckAllActors(false);
end

function ToughDoorsScript:EndScript()
	self:CheckAllActors(true);
end

function ToughDoorsScript:UpdateScript()
	self:CheckAllActors(false);
end

function ToughDoorsScript:CheckAllActors(reverse)
	for actor in MovableMan.Actors do
		self:CheckDoor(actor, reverse);
	end
	for actor in MovableMan.AddedActors do
		self:CheckDoor(actor, reverse);
	end
end

function ToughDoorsScript:CheckDoor(actor, reverse)
	if IsADoor(actor) then
		local door = ToADoor(actor);
		local multiplier = reverse and 1 / self.multiplier or self.multiplier;
		if door:NumberValueExists("ToughDoorsScript") == reverse then
			door.GibWoundLimit = door.GibWoundLimit * multiplier;
			door.GibImpulseLimit = door.GibImpulseLimit * multiplier;
			self:SetMarked(door, reverse);
		end
		if door.Door and door.Door:NumberValueExists("ToughDoorsScript") == reverse then
			door.Door.GibWoundLimit = door.Door.GibWoundLimit * multiplier;
			door.Door.GibImpulseLimit = door.Door.GibImpulseLimit * multiplier;
			door.Door.JointStrength = door.Door.JointStrength * multiplier;
			self:SetMarked(door.Door, reverse);
		end
	end
end

function ToughDoorsScript:SetMarked(object, reverse)
	if reverse then
		object:RemoveNumberValue("ToughDoorsScript");
	else
		object:SetNumberValue("ToughDoorsScript", 1);
	end
end
