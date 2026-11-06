function ToughCraftScript:StartScript()
	self.multiplier = 4;
	self:CheckAllActors(false);
end

function ToughCraftScript:EndScript()
	self:CheckAllActors(true);
end

function ToughCraftScript:UpdateScript()
	self:CheckAllActors(false);
end

function ToughCraftScript:CheckAllActors(reverse)
	for actor in MovableMan.Actors do
		self:CheckCraft(actor, reverse);
	end
	for actor in MovableMan.AddedActors do
		self:CheckCraft(actor, reverse);
	end
end

function ToughCraftScript:CheckCraft(actor, reverse)
	if IsACDropShip(actor) or IsACRocket(actor) then
		local multiplier = reverse and 1 / self.multiplier or self.multiplier;
		if actor:NumberValueExists("ToughCraftScript") == reverse then
			actor.GibWoundLimit = actor.GibWoundLimit * multiplier;
			actor.GibImpulseLimit = actor.GibImpulseLimit * multiplier;
			self:SetMarked(actor, reverse);
		end
		for att in actor.Attachables do
			if att:NumberValueExists("ToughCraftScript") == reverse then
				att.GibWoundLimit = att.GibWoundLimit * multiplier;
				att.JointStrength = att.JointStrength * multiplier;
				self:SetMarked(att, reverse);
			end
		end
	end
end

function ToughCraftScript:SetMarked(object, reverse)
	if reverse then
		object:RemoveNumberValue("ToughCraftScript");
	else
		object:SetNumberValue("ToughCraftScript", 1);
	end
end
