function FasterWalkingScript:StartScript()
	self.speedDenominator = 1.3;
	self.forceDenominator = 3;

	-- this is maybee not even close to an accurate use of this term
	self.vtable = {};
end

function FasterWalkingScript:EndScript()
	for _, walker in ipairs(self.vtable) do
		if walker and IsActor(walker) and MovableMan:ValidMO(walker) then
			local walkerSpeedMod = walker:GetNumberValue("FasterWalking_speedProportionIncrease") + 1;
			local walkerForceMod = walker:GetNumberValue("FasterWalking_forceProportionIncrease") + 1;
		
			walker:SetLimbPathTravelSpeed(Actor.WALK, walker:GetLimbPathTravelSpeed(Actor.WALK) / walkerSpeedMod);
			walker:SetLimbPathPushForce(Actor.WALK, walker:GetLimbPathPushForce(Actor.WALK) / walkerForceMod);
			walker:SetLimbPathTravelSpeed(Actor.RUN, walker:GetLimbPathTravelSpeed(Actor.RUN) / walkerSpeedMod);
			walker:SetLimbPathPushForce(Actor.RUN, walker:GetLimbPathPushForce(Actor.RUN) / walkerForceMod);

			walker:RemoveNumberValue("FasterWalking_speedProportionIncrease");
			walker:RemoveNumberValue("FasterWalking_forceProportionIncrease");
		end
	end
end

function FasterWalkingScript:UpdateScript()
	for actor in MovableMan.AddedActors do
		self:CheckActorAndContents(actor);
	end
end

function FasterWalkingScript:CheckActorAndContents(actor)
	local speedCorrection = self.speedDenominator / (actor:GetNumberValue("FasterWalking_speedProportionIncrease") + 1);
	local forceCorrection = self.forceDenominator / (actor:GetNumberValue("FasterWalking_forceProportionIncrease") + 1);

	if 0.001 < math.abs(speedCorrection - 1) + math.abs(forceCorrection - 1) then
		self:BoostWalkSpeed(actor, speedCorrection, forceCorrection);
	end

	for item in actor.Inventory do
		if IsActor(item) then
			self:CheckActorAndContents(ToActor(item));
		end
	end
end

function FasterWalkingScript:BoostWalkSpeed(actor, speedCorrection, forceCorrection)
	local walker;

	if IsAHuman(actor) then
		walker = ToAHuman(actor);
	elseif IsACrab(actor) then
		walker = ToACrab(actor);
	end

	if walker then
		table.insert(self.vtable, walker);

		local presentSpeedMod = walker:GetNumberValue("FasterWalking_speedProportionIncrease") + 1;
		local presentForceMod = walker:GetNumberValue("FasterWalking_forceProportionIncrease") + 1;

		walker:SetNumberValue("FasterWalking_speedProportionIncrease", self.speedDenominator - 1);
		walker:SetNumberValue("FasterWalking_forceProportionIncrease", self.forceDenominator - 1);

		walker:SetLimbPathTravelSpeed(Actor.WALK, walker:GetLimbPathTravelSpeed(Actor.WALK) * speedCorrection);
		walker:SetLimbPathPushForce(Actor.WALK, walker:GetLimbPathPushForce(Actor.WALK) * forceCorrection);
		walker:SetLimbPathTravelSpeed(Actor.RUN, walker:GetLimbPathTravelSpeed(Actor.RUN) * speedCorrection);
		walker:SetLimbPathPushForce(Actor.RUN, walker:GetLimbPathPushForce(Actor.RUN) * forceCorrection);
	end
end