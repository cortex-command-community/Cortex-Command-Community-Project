function FasterWalkingScript:StartScript()
	self.multiplier = 1.6;
	self.pushForceDenominator = 1.2;
end

function FasterWalkingScript:UpdateScript()
	for actor in MovableMan.AddedActors do
		if not actor:NumberValueExists("FasterWalkingScript") then
			self:BoostWalkSpeed(actor);
			for item in actor.Inventory do
				self:BoostWalkSpeed(item);
			end
		end
	end
end

function FasterWalkingScript:BoostWalkSpeed(actor)
	local walker;
	if IsAHuman(actor) then
		walker = ToAHuman(actor);
	elseif IsACrab(actor) then
		walker = ToACrab(actor);
	end
	if walker then
		actor:SetNumberValue("FasterWalkingScript", 1);
		walker:SetLimbPathTravelSpeed(Actor.WALK, walker:GetLimbPathTravelSpeed(Actor.WALK) * self.multiplier);
		walker:SetLimbPathTravelSpeed(Actor.RUN, walker:GetLimbPathTravelSpeed(Actor.RUN) * self.multiplier);
		walker:SetLimbPathPushForce(Actor.WALK, walker:GetLimbPathPushForce(Actor.WALK) * self.pushForceDenominator);
		walker:SetLimbPathPushForce(Actor.RUN, walker:GetLimbPathPushForce(Actor.RUN) * self.pushForceDenominator);
	end
end