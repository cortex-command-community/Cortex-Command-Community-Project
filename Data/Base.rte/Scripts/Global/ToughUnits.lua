function ToughUnitsScript:StartScript()
	self.multiplier = 2;
	self:CheckAllActors(false);
end

function ToughUnitsScript:EndScript()
	self:CheckAllActors(true);
end

function ToughUnitsScript:UpdateScript()
	self:CheckAllActors(false);
end

function ToughUnitsScript:CheckAllActors(reverse)
	for actor in MovableMan.Actors do
		self:CheckActorAndContents(actor, reverse);
	end
	for actor in MovableMan.AddedActors do
		self:CheckActorAndContents(actor, reverse);
	end
end

function ToughUnitsScript:CheckActorAndContents(actor, reverse)
	if IsAHuman(actor) or IsACrab(actor) then
		self:MarkBody(actor, reverse);
		for limb in actor.Attachables do
			self:MarkPart(limb, reverse);
			for att in limb.Attachables do
				self:MarkPart(att, reverse);
			end
		end
	end
	for item in actor.Inventory do
		if IsActor(item) then
			self:CheckActorAndContents(ToActor(item), reverse);
		end
	end
end

function ToughUnitsScript:MarkBody(actor, reverse)
	if actor:NumberValueExists("ToughUnitsScript") ~= reverse then
		return;
	end
	local multiplier = reverse and 1 / self.multiplier or self.multiplier;
	actor.GibWoundLimit = actor.GibWoundLimit * multiplier;
	actor.GibImpulseLimit = actor.GibImpulseLimit * multiplier;
	if reverse then
		actor:RemoveNumberValue("ToughUnitsScript");
	else
		actor:SetNumberValue("ToughUnitsScript", 1);
	end
end

function ToughUnitsScript:MarkPart(part, reverse)
	if part:NumberValueExists("ToughUnitsScript") ~= reverse then
		return;
	end
	local multiplier = reverse and 1 / self.multiplier or self.multiplier;
	part.GibWoundLimit = part.GibWoundLimit * multiplier;
	part.JointStrength = part.JointStrength * multiplier;
	if reverse then
		part:RemoveNumberValue("ToughUnitsScript");
	else
		part:SetNumberValue("ToughUnitsScript", 1);
	end
end
