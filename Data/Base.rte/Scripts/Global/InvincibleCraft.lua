function InvincibleCraftScript:StartScript()
	self.multiplier = 2;
	self:CheckAllActors(false);
end

function InvincibleCraftScript:EndScript()
	self:CheckAllActors(true);
end

function InvincibleCraftScript:UpdateScript()
	self:CheckAllActors(false);
end

function InvincibleCraftScript:CheckAllActors(reverse)
	for actor in MovableMan.Actors do
		self:CheckCraft(actor, reverse);
	end
	for actor in MovableMan.AddedActors do
		self:CheckCraft(actor, reverse);
	end
end

function InvincibleCraftScript:CheckCraft(actor, reverse)
	if IsACDropShip(actor) or IsACRocket(actor) then
		if actor:NumberValueExists("InvincibleCraftScript") == reverse then
			local multiplier = reverse and 1 / self.multiplier or self.multiplier;
			actor.GibImpulseLimit = actor.GibImpulseLimit * multiplier;
			actor.ImpulseDamageThreshold = actor.ImpulseDamageThreshold * multiplier;
			self:MarkHits(actor, reverse);
		end
		for att in actor.Attachables do
			if att:NumberValueExists("InvincibleCraftScript") == reverse then
				if reverse then
					att.JointStrength = att:GetNumberValue("InvincibleCraftScript_JointStrength");
					att:RemoveNumberValue("InvincibleCraftScript_JointStrength");
				else
					att:SetNumberValue("InvincibleCraftScript_JointStrength", att.JointStrength);
					att.JointStrength = 1000000;
				end
				self:MarkHits(att, reverse);
			end
		end
	end
end

function InvincibleCraftScript:MarkHits(object, reverse)
	if reverse then
		object.HitsMOs = object:GetNumberValue("InvincibleCraftScript_HitsMOs") == 1;
		object.GetsHitByMOs = object:GetNumberValue("InvincibleCraftScript_GetsHitByMOs") == 1;
		object:RemoveNumberValue("InvincibleCraftScript_HitsMOs");
		object:RemoveNumberValue("InvincibleCraftScript_GetsHitByMOs");
		object:RemoveNumberValue("InvincibleCraftScript");
	else
		object:SetNumberValue("InvincibleCraftScript_HitsMOs", object.HitsMOs and 1 or 0);
		object:SetNumberValue("InvincibleCraftScript_GetsHitByMOs", object.GetsHitByMOs and 1 or 0);
		object:SetNumberValue("InvincibleCraftScript", 1);
		object.HitsMOs = false;
		object.GetsHitByMOs = false;
	end
end
