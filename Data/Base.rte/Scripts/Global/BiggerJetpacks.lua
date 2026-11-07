function BiggerJetpacksScript:StartScript()
	self.multiplier = 2;
	self.bonus = 100;
	self:CheckAllActors(false);
end

function BiggerJetpacksScript:EndScript()
	self:CheckAllActors(true);
end

function BiggerJetpacksScript:UpdateScript()
	self:CheckAllActors(false);
end

function BiggerJetpacksScript:CheckAllActors(reverse)
	for actor in MovableMan.Actors do
		self:CheckActorAndContents(actor, reverse);
	end
	for actor in MovableMan.AddedActors do
		self:CheckActorAndContents(actor, reverse);
	end
end

function BiggerJetpacksScript:CheckActorAndContents(actor, reverse)
	if IsAHuman(actor) then
		self:MarkJetpack(ToAHuman(actor).Jetpack, reverse);
	elseif IsACrab(actor) then
		self:MarkJetpack(ToACrab(actor).Jetpack, reverse);
	end
	for item in actor.Inventory do
		if IsActor(item) then
			self:CheckActorAndContents(ToActor(item), reverse);
		end
	end
end

function BiggerJetpacksScript:MarkJetpack(jetpack, reverse)
	if not jetpack or jetpack:NumberValueExists("BiggerJetpacksScript") ~= reverse then
		return;
	end
	if reverse then
		jetpack.JetTimeTotal = (jetpack.JetTimeTotal - self.bonus) / self.multiplier;
		jetpack:RemoveNumberValue("BiggerJetpacksScript");
	else
		jetpack.JetTimeTotal = jetpack.JetTimeTotal * self.multiplier + self.bonus;
		jetpack:SetNumberValue("BiggerJetpacksScript", 1);
	end
end
