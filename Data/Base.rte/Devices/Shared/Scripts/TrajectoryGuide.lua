function Create(self)
	self.laserTimer = Timer();
	self.laserTimer:SetSimTimeLimitMS(10);
	self.guideTable = {};

	if IsThrownDevice(self) then
		self.isThrownDevice = true;
		if self.MaxThrowVel > 0 then
			self.projectileVelMax = self.MaxThrowVel;
			self.projectileVelMin = self.MinThrowVel;
		end
		self.projectileGravity = self.GlobalAccScalar;
		self.projectileAirResistance = self.AirResistance;
		self.projectileAirThreshold = self.AirThreshold;

	elseif self.Magazine and self.Magazine.RoundCount ~= 0 then
		self.projectileVel = self.Magazine.NextRound.FireVel;
		self.projectileGravity = self.Magazine.NextRound.NextParticle.GlobalAccScalar;
		self.projectileAirResistance = self.Magazine.NextRound.NextParticle.AirResistance;
		self.projectileAirThreshold = self.Magazine.NextRound.NextParticle.AirThreshold;
	end

	self.guideRadius = self:NumberValueExists("TrajectoryGuideBlastRadius") and self:GetNumberValue("TrajectoryGuideBlastRadius") or 12;
	self.guideAccuracy = self:NumberValueExists("TrajectoryGuideAccuracy") and self:GetNumberValue("TrajectoryGuideAccuracy") or 1;
	self.maxTrajectoryPars = self:NumberValueExists("TrajectoryGuideLength") and self:GetNumberValue("TrajectoryGuideLength") or 60;
	self.guideColor = self:NumberValueExists("TrajectoryGuideColorIndex") and self:GetNumberValue("TrajectoryGuideColorIndex") or 120;
	self.skipLines = self:NumberValueExists("TrajectoryGuideSkipLines") and self:GetNumberValue("TrajectoryGuideSkipLines") or 1;
	self.crossLineCount = self:NumberValueExists("TrajectoryGuideCrossPartCount") and self:GetNumberValue("TrajectoryGuideCrossPartCount") or 0;
	self.crossLineAngle = (math.pi * 2)/self.crossLineCount;
	self.viewCorrection = self:NumberValueExists("TrajectoryGuideViewCorrection") and self:GetNumberValue("TrajectoryGuideViewCorrection") or 0;
	self.drawHitsOnly = self:NumberValueExists("TrajectoryGuideDrawHitsOnly");
	self.includeMOHits = self:NumberValueExists("TrajectoryGuideIncludeMOHits");
end

function ThreadedUpdate(self)
	local actor = self:GetRootParent();
	if IsActor(actor) and MovableMan:ValidMO(actor) and ToActor(actor):IsPlayerControlled() and not self:IsReloading() then
		local actor = ToActor(actor);
		local controller = actor:GetController();
		if not self.isThrownDevice and (self:DoneReloading() or self.FiredFrame) and self.Magazine and self.Magazine.RoundCount ~= 0 then
			self.projectileVel = self.Magazine.NextRound.FireVel;
			self.projectileGravity = self.Magazine.NextRound.NextParticle.GlobalAccScalar;
		end
		local hitPos;
		if (self.isThrownDevice and (actor.SharpAimProgress > 0.5 or controller:IsState(Controller.WEAPON_FIRE))) or (not self.isThrownDevice and actor.SharpAimProgress > 0.1) then
			if self.laserTimer:IsPastSimTimeLimit() then

				local guideParPos, guideParVel;
				if self.isThrownDevice and IsAHuman(actor) then
					--Display detonation point if a scripted fuze is active
					if self.fuze and self.fuzeDelay then
						self.maxTrajectoryPars = (self.fuzeDelay - self.fuze.ElapsedSimTimeMS - self.laserTimer.ElapsedSimTimeMS)/TimerMan.DeltaTimeMS * rte.PxTravelledPerFrame;
					end
					actor = ToAHuman(actor);
					local throwProgress = controller:IsState(Controller.WEAPON_FIRE) and actor.ThrowProgress or actor.SharpAimProgress;
					local maxVel = self.projectileVelMax or (actor.FGArm.ThrowStrength + math.abs(actor.AngularVel * 0.5))/math.sqrt(math.abs(self.Mass) + 1);
					local minVel = self.projectileVelMin or maxVel * 0.2;
					--The following offset is as found in the source code (TODO: utilize EndThrowOffset properly instead)
					guideParPos = actor.Pos + actor.Vel * rte.PxTravelledPerFrame + Vector((actor.FGArm.ParentOffset.X + actor.FGArm.MaxLength) * actor.FlipFactor, actor.FGArm.ParentOffset.Y - actor.FGArm.MaxLength * 0.5):RadRotate(actor:GetAimAngle(false) * actor.FlipFactor);
					guideParVel = Vector(minVel + (maxVel - minVel) * throwProgress, 0):RadRotate(actor:GetAimAngle(true));
				elseif not self.isThrownDevice then
					guideParPos = self.MuzzlePos;
					guideParVel = Vector(self.projectileVel, 0):RadRotate(actor:GetAimAngle(true));
				end

				self.guideTable = {};
				self.guideTable[1] = Vector(guideParPos.X, guideParPos.Y);

				for i = 1, self.maxTrajectoryPars do
					guideParVel = guideParVel + Vector(SceneMan.GlobalAcc.X, SceneMan.GlobalAcc.Y)/GetPPM() * self.projectileGravity;
					if self.projectileAirResistance ~= 0 and guideParVel.Largest >= self.projectileAirThreshold then
						guideParVel:SetMagnitude(guideParVel.Magnitude * (1 - (self.projectileAirResistance * TimerMan.DeltaTimeSecs * 2.5)));	--To-do: replace "* 2.5" with something more intangible
					end
					local roughHit = false;
					for i = 1, self.guideAccuracy do
						guideParPos = guideParPos + guideParVel/self.guideAccuracy;
						local moCheck = self.includeMOHits and SceneMan:GetMOIDPixel(guideParPos.X, guideParPos.Y) or rte.NoMOID;
						if SceneMan:GetTerrMatter(guideParPos.X, guideParPos.Y) == rte.airID and (moCheck == rte.NoMOID or MovableMan:GetMOFromID(moCheck).RootID == self.RootID) then
							self.guideTable[#self.guideTable + 1] = guideParPos;
						else
							roughHit = true;
							break;
						end
					end
					if roughHit then
						hitPos = Vector(self.guideTable[#self.guideTable].X, self.guideTable[#self.guideTable].Y);
						if self.includeMOHits then
							SceneMan:CastObstacleRay(self.guideTable[#self.guideTable], SceneMan:ShortestDistance(self.guideTable[#self.guideTable], guideParPos, false), Vector(), hitPos, self.ID, -2, rte.airID, 3);
						else
							SceneMan:CastStrengthRay(self.guideTable[#self.guideTable], SceneMan:ShortestDistance(self.guideTable[#self.guideTable], guideParPos, false), 0, hitPos, 3, rte.airID, false);
						end
						self.guideTable[#self.guideTable + 1] = hitPos;
						break;
					end
				end
				self.laserTimer:Reset();
			end
		else
			self.guideTable = {};
		end
		if #self.guideTable > 1 and (not self.drawHitsOnly or hitPos) then
			local screen = ActivityMan:GetActivity():ScreenOfPlayer(controller.Player);
			local angleDirection = #self.guideTable > 2 and SceneMan:ShortestDistance(self.guideTable[#self.guideTable - 2], self.guideTable[#self.guideTable - 1], SceneMan.SceneWrapsX).AbsRadAngle or self.RotAngle + (self.HFlipped and math.pi or 0);
			if self.skipLines > 0 then
				for i = 1, #self.guideTable - 1 do
					if self.skipLines == 0 or i % (self.skipLines + 1) == 0 then
						PrimitiveMan:DrawLinePrimitive(screen, self.guideTable[i], self.guideTable[i + 1], self.guideColor);
					end
				end
			end
			PrimitiveMan:DrawCirclePrimitive(screen, self.guideTable[#self.guideTable], self.guideRadius, self.guideColor);
			local lineVector = Vector(-self.guideRadius, 0):RadRotate(angleDirection);
			for i = 0, self.crossLineCount - 1 do
				PrimitiveMan:DrawLinePrimitive(screen, self.guideTable[#self.guideTable] + lineVector:RadRotate(self.crossLineAngle) * 0.5, self.guideTable[#self.guideTable] + lineVector * 1.5, self.guideColor);
			end
			if self.viewCorrection > 0 then
				--This part will offset the actor's view to point closer to the guide end position.
				local viewPoint = actor.ViewPoint + SceneMan:ShortestDistance(actor.ViewPoint, self.guideTable[#self.guideTable], SceneMan.SceneWrapsX):CapMagnitude(self.SharpLength) * self.viewCorrection * actor.SharpAimProgress;
				CameraMan:SetScrollTarget(viewPoint, 0.1, screen);
			end
		end
	end
end