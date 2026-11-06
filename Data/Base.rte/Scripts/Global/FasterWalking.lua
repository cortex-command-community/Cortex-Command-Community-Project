function FasterWalkingScript:StartScript()
	self.speedMultiplier = 1.6;
	self.pushForceMultiplier = 1.2;
	self:CheckAllActors(false);
end

function FasterWalkingScript:EndScript()
	self:CheckAllActors(true);
end

function FasterWalkingScript:UpdateScript()
	self:CheckAllActors(false);
end

function FasterWalkingScript:CheckAllActors(reverse)
	for actor in MovableMan.Actors do
		self:CheckActorAndContents(actor, reverse);
	end
	for actor in MovableMan.AddedActors do
		self:CheckActorAndContents(actor, reverse);
	end
end

function FasterWalkingScript:CheckActorAndContents(actor, reverse)
	if IsAHuman(actor) then
		self:MarkWalker(ToAHuman(actor), reverse);
	elseif IsACrab(actor) then
		self:MarkWalker(ToACrab(actor), reverse);
	end
	for item in actor.Inventory do
		if IsActor(item) then
			self:CheckActorAndContents(ToActor(item), reverse);
		end
	end
end

function FasterWalkingScript:MarkWalker(walker, reverse)
	if walker:NumberValueExists("FasterWalkingScript") ~= reverse then
		return;
	end
	local speedMultiplier = reverse and 1 / self.speedMultiplier or self.speedMultiplier;
	local pushForceMultiplier = reverse and 1 / self.pushForceMultiplier or self.pushForceMultiplier;
	walker:SetLimbPathTravelSpeed(Actor.WALK, walker:GetLimbPathTravelSpeed(Actor.WALK) * speedMultiplier);
	walker:SetLimbPathTravelSpeed(Actor.RUN, walker:GetLimbPathTravelSpeed(Actor.RUN) * speedMultiplier);
	walker:SetLimbPathPushForce(Actor.WALK, walker:GetLimbPathPushForce(Actor.WALK) * pushForceMultiplier);
	walker:SetLimbPathPushForce(Actor.RUN, walker:GetLimbPathPushForce(Actor.RUN) * pushForceMultiplier);
	if reverse then
		walker:RemoveNumberValue("FasterWalkingScript");
	else
		walker:SetNumberValue("FasterWalkingScript", 1);
	end
end
