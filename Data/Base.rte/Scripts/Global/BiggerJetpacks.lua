function BiggerJetpacksScript:StartScript()
	self.multiplier = 2;
end

function BiggerJetpacksScript:UpdateScript()
	for actor in MovableMan.AddedActors do
		if not actor:NumberValueExists("BiggerJetpacksScript") then
			self:BuffJetpack(actor);
			for item in actor.Inventory do
				self:BuffJetpack(item);
			end
		end
	end
end

function BiggerJetpacksScript:BuffJetpack(actor)
	if IsAHuman(actor) then
		actor = ToAHuman(actor);
	elseif IsACrab(actor) then
		actor = ToACrab(actor);
	end
	if actor.Jetpack then
		actor:SetNumberValue("BiggerJetpacksScript", 1);
		actor.Jetpack.JetTimeTotal = actor.Jetpack.JetTimeTotal * self.multiplier + 100;
	end
end