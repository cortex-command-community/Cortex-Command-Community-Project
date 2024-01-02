function BulkyDropshipsScript:StartScript()
	self.multiplier = 0.35;
end

function BulkyDropshipsScript:UpdateScript()
	for actor in MovableMan.AddedActors do
		if not actor:NumberValueExists("BulkyDropshipsScript") then
			actor:SetNumberValue("BulkyDropshipsScript", 1);
			if IsACDropShip(actor) then
				local dropship = ToACDropShip(actor);
				dropship.MaxEngineAngle = dropship.MaxEngineAngle * self.multiplier;
				dropship.LateralControlSpeed = dropship.LateralControlSpeed * self.multiplier;
				dropship.HatchDelay = dropship.HatchDelay/self.multiplier;
			end
		end
	end
end