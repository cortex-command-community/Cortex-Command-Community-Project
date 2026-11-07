function StrongerJetpacksScript:StartScript()
	self.multiplier = 1.3;
	self:CheckAllActors(false);
end

function StrongerJetpacksScript:EndScript()
	self:CheckAllActors(true);
end

function StrongerJetpacksScript:UpdateScript()
	self:CheckAllActors(false);
end

function StrongerJetpacksScript:CheckAllActors(reverse)
	for actor in MovableMan.Actors do
		self:CheckActorAndContents(actor, reverse);
	end
	for actor in MovableMan.AddedActors do
		self:CheckActorAndContents(actor, reverse);
	end
end

function StrongerJetpacksScript:CheckActorAndContents(actor, reverse)
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

function StrongerJetpacksScript:MarkJetpack(jetpack, reverse)
	if not jetpack or jetpack:NumberValueExists("StrongerJetpacksScript") ~= reverse then
		return;
	end
	-- Burst sizes are whole numbers, so dividing wouldn't get the original back.
	local index = 0;
	for emission in jetpack.Emissions do
		local burstSizeKey = "StrongerJetpacksScript_BurstSize" .. index;
		if reverse then
			emission.ParticlesPerMinute = emission.ParticlesPerMinute / self.multiplier;
			emission.BurstSize = jetpack:GetNumberValue(burstSizeKey);
			jetpack:RemoveNumberValue(burstSizeKey);
		else
			jetpack:SetNumberValue(burstSizeKey, emission.BurstSize);
			emission.ParticlesPerMinute = emission.ParticlesPerMinute * self.multiplier;
			emission.BurstSize = emission.BurstSize * self.multiplier;
		end
		index = index + 1;
	end
	if reverse then
		jetpack:RemoveNumberValue("StrongerJetpacksScript");
	else
		jetpack:SetNumberValue("StrongerJetpacksScript", 1);
	end
end
