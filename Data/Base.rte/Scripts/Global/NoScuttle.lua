function NoScuttleScript:StartScript()
	self:CheckAllActors(false);
end

function NoScuttleScript:EndScript()
	self:CheckAllActors(true);
end

function NoScuttleScript:UpdateScript()
	self:CheckAllActors(false);
end

function NoScuttleScript:CheckAllActors(reverse)
	for actor in MovableMan.Actors do
		self:CheckCraft(actor, reverse);
	end
	for actor in MovableMan.AddedActors do
		self:CheckCraft(actor, reverse);
	end
end

function NoScuttleScript:CheckCraft(actor, reverse)
	if (IsACDropShip(actor) or IsACRocket(actor)) and actor:NumberValueExists("NoScuttleScript") == reverse then
		local craft = ToACraft(actor);
		if reverse then
			craft.ScuttleOnDeath = craft:GetNumberValue("NoScuttleScript") == 1;
			craft:RemoveNumberValue("NoScuttleScript");
		else
			craft:SetNumberValue("NoScuttleScript", craft.ScuttleOnDeath and 1 or 0);
			craft.ScuttleOnDeath = false;
		end
	end
end
