function BulkyDropshipsScript:StartScript()
	self.multiplier = 0.35;
	self:CheckAllActors(false);
end

function BulkyDropshipsScript:EndScript()
	self:CheckAllActors(true);
end

function BulkyDropshipsScript:UpdateScript()
	self:CheckAllActors(false);
end

function BulkyDropshipsScript:CheckAllActors(reverse)
	for actor in MovableMan.Actors do
		self:CheckDropship(actor, reverse);
	end
	for actor in MovableMan.AddedActors do
		self:CheckDropship(actor, reverse);
	end
end

function BulkyDropshipsScript:CheckDropship(actor, reverse)
	if not IsACDropShip(actor) or actor:NumberValueExists("BulkyDropshipsScript") ~= reverse then
		return;
	end
	local dropship = ToACDropShip(actor);
	if reverse then
		dropship.MaxEngineAngle = dropship.MaxEngineAngle / self.multiplier;
		dropship.LateralControlSpeed = dropship.LateralControlSpeed / self.multiplier;
		-- The hatch delay is a whole number, so multiplying wouldn't get the original back.
		dropship.HatchDelay = dropship:GetNumberValue("BulkyDropshipsScript");
		dropship:RemoveNumberValue("BulkyDropshipsScript");
	else
		dropship:SetNumberValue("BulkyDropshipsScript", dropship.HatchDelay);
		dropship.MaxEngineAngle = dropship.MaxEngineAngle * self.multiplier;
		dropship.LateralControlSpeed = dropship.LateralControlSpeed * self.multiplier;
		dropship.HatchDelay = dropship.HatchDelay / self.multiplier;
	end
end
