function StrongerJetpacksScript:StartScript()
	self.multiplier = 1.3;
end

function StrongerJetpacksScript:UpdateScript()
	for actor in MovableMan.AddedActors do
		if not actor:NumberValueExists("StrongerJetpacksScript") then
			self:BuffJetpack(actor);
			for item in actor.Inventory do
				self:BuffJetpack(item);
			end
		end
	end
end

function StrongerJetpacksScript:BuffJetpack(actor)
	if IsAHuman(actor) then
		actor = ToAHuman(actor);
	elseif IsACrab(actor) then
		actor = ToACrab(actor);
	end
	if actor.Jetpack then
		actor:SetNumberValue("StrongerJetpacksScript", 1);
		for em in actor.Jetpack.Emissions do
			em.ParticlesPerMinute = em.ParticlesPerMinute * self.multiplier;
			em.BurstSize = em.BurstSize * self.multiplier;
		end
	end
end