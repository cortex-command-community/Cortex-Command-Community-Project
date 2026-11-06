function ToughBrainsScript:StartScript()
	self.multiplier = 3;
	self:CheckAllActors(false);
end

function ToughBrainsScript:EndScript()
	self:CheckAllActors(true);
end

function ToughBrainsScript:UpdateScript()
	self:CheckAllActors(false);
end

function ToughBrainsScript:CheckAllActors(reverse)
	for actor in MovableMan.Actors do
		self:CheckActorAndContents(actor, reverse);
	end
	for actor in MovableMan.AddedActors do
		self:CheckActorAndContents(actor, reverse);
	end
end

function ToughBrainsScript:CheckActorAndContents(actor, reverse)
	if actor:IsInGroup("Brains") then
		self:MarkBody(actor, reverse);
		for limb in actor.Attachables do
			self:MarkPart(limb, reverse);
			for att in limb.Attachables do
				self:MarkPart(att, reverse);
			end
		end
		if IsACrab(actor) and ToACrab(actor).EquippedItem then
			self:MarkPart(ToACrab(actor).EquippedItem, reverse);
		end
	end
	for item in actor.Inventory do
		if IsActor(item) then
			self:CheckActorAndContents(ToActor(item), reverse);
		end
	end
end

function ToughBrainsScript:MarkBody(actor, reverse)
	if actor:NumberValueExists("ToughBrainsScript") ~= reverse then
		return;
	end
	local multiplier = reverse and 1 / self.multiplier or self.multiplier;
	actor.GibWoundLimit = actor.GibWoundLimit * multiplier;
	actor.GibImpulseLimit = actor.GibImpulseLimit * multiplier;
	if reverse then
		actor:RemoveNumberValue("ToughBrainsScript");
	else
		actor:SetNumberValue("ToughBrainsScript", 1);
	end
end

function ToughBrainsScript:MarkPart(part, reverse)
	if part:NumberValueExists("ToughBrainsScript") ~= reverse then
		return;
	end
	local multiplier = reverse and 1 / self.multiplier or self.multiplier;
	part.GibWoundLimit = part.GibWoundLimit * multiplier;
	part.JointStrength = part.JointStrength * multiplier;
	if reverse then
		part:RemoveNumberValue("ToughBrainsScript");
	else
		part:SetNumberValue("ToughBrainsScript", 1);
	end
end
