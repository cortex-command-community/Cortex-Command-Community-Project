function Create(self)
	self.range = math.sqrt(FrameMan.PlayerScreenWidth^2 + FrameMan.PlayerScreenHeight^2)/2;
	self.penetrationStrength = 170;
	self.strengthVariation = 5;
	--This value tracks the shots and varies the penetration strength to create a "resistance" effect on tougher materials
	self.shotCounter = 0;	--TODO: Rename/describe this variable better
	self.activity = ActivityMan:GetActivity();

	self.cooldown = Timer();
	self.cooldownSpeed = 0.5;

	self.addedParticles = {};

	self.addedWound = nil;
	self.addedWoundOffset = nil;
	self.addedWoundToUniqueID = nil;

	function self.emitSmoke(particleCount)
		for i = 1, particleCount do
			local smoke = CreateMOSParticle("Tiny Smoke Ball 1" .. (math.random() < 0.5 and " Glow Blue" or ""), "Base.rte");
			smoke.Pos = self.MuzzlePos;
			smoke.Lifetime = smoke.Lifetime * RangeRand(0.5, 1.0);
			smoke.Vel = self.Vel * 0.5 + Vector(RangeRand(0, i), 0):RadRotate(RangeRand(-math.pi, math.pi));
			table.insert(self.addedParticles, smoke);
		end
		self:RequestSyncedUpdate();
	end
end

function ThreadedUpdate(self)
	if self.FiredFrame then
		local actor = self:GetRootParent();
		local range = self.range + math.random(8);
		if IsActor(actor) then
			actor = ToActor(actor);
			range = range + actor.AimDistance;
			if actor:GetController():IsState(Controller.AIM_SHARP) then
				range = range + self.SharpLength * actor.SharpAimProgress;
			end
		end

		local startPos = self.MuzzlePos + Vector(0, RangeRand(-0.5, 0.5)):RadRotate(self.RotAngle);
		local hitPos = Vector(startPos.X, startPos.Y);
		local gapPos = Vector(startPos.X, startPos.Y);
		local trace = Vector(range * self.FlipFactor, 0):RadRotate(self.RotAngle);
		--Use higher pixel skip first to find a rough estimate
		local skipPx = 4;
		local rayLength = SceneMan:CastObstacleRay(startPos, trace, hitPos, gapPos, actor.ID, self.Team, rte.airID, skipPx);

		if rayLength >= 0 then
			gapPos = gapPos - Vector(trace.X, trace.Y):SetMagnitude(skipPx);
			skipPx = 1;
			local shortRay = SceneMan:CastObstacleRay(gapPos, Vector(trace.X, trace.Y):SetMagnitude(range - rayLength + skipPx), hitPos, gapPos, actor.ID, self.Team, rte.airID, skipPx);
			gapPos = gapPos - Vector(trace.X, trace.Y):SetMagnitude(skipPx);
			local strengthFactor = math.max(1 - rayLength/self.range, math.random()) * (self.shotCounter + 1)/self.strengthVariation;

			self.addedWoundToMOID = SceneMan:GetMOIDPixel(hitPos.X, hitPos.Y);
			if self.addedWoundToMOID ~= rte.NoMOID and self.addedWoundToMOID ~= self.ID then
				local mo = ToMOSRotating(MovableMan:GetMOFromID(self.addedWoundToMOID));
				if self.penetrationStrength * strengthFactor >= mo.Material.StructuralIntegrity then
					local moAngle = -mo.RotAngle * mo.FlipFactor;

					local woundName = mo:GetEntryWoundPresetName();
					if woundName ~= "" then
						local wound = CreateAEmitter(woundName);

						local dist = SceneMan:ShortestDistance(mo.Pos, hitPos, SceneMan.SceneWrapsX);
						local woundOffset = Vector(dist.X * mo.FlipFactor, dist.Y):RadRotate(moAngle):SetMagnitude(dist.Magnitude - (wound.Radius - 1) * wound.Scale);
						wound.InheritedRotAngleOffset = woundOffset.AbsRadAngle;
						woundOffset = woundOffset:RadRotate(-mo.RotAngle);
						self.addedWound = wound;
						self.addedWoundOffset = woundOffset;
						self.addedWoundToUniqueID = mo.UniqueID;
						self:RequestSyncedUpdate();
					end
				end
			end

			local smoke = CreateMOSParticle("Tiny Smoke Ball 1" .. (math.random() < 0.5 and " Glow Blue" or ""), "Base.rte");
			smoke.Pos = gapPos;
			smoke.Vel = Vector(-trace.X, -trace.Y):SetMagnitude(math.random(3, 6)):RadRotate(RangeRand(-1.5, 1.5));
			smoke.Lifetime = smoke.Lifetime * strengthFactor;
			table.insert(self.addedParticles, smoke);

			local pix = CreateMOPixel("Laser Rifle Glow " .. math.floor(strengthFactor * 4 + 0.5), "Techion.rte");
			pix.Pos = gapPos;
			pix.Sharpness = self.penetrationStrength/6;
			pix.Vel = Vector(trace.X, trace.Y):SetMagnitude(6);
			table.insert(self.addedParticles, pix);
		end
		if rayLength ~= 0 then
			trace = SceneMan:ShortestDistance(startPos, gapPos, SceneMan.SceneWrapsX);
			for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
				local team = self.activity:GetTeamOfPlayer(player);
				local screen = self.activity:ScreenOfPlayer(player);
				if screen ~= -1 and not (SceneMan:IsUnseen(startPos.X, startPos.Y, team) or SceneMan:IsUnseen(hitPos.X, hitPos.Y, team)) then
					PrimitiveMan:DrawLinePrimitive(screen, startPos, startPos + trace, 254);
				end
			end
			local particleCount = trace.Magnitude * RangeRand(0.4, 0.8);
			for i = 0, particleCount do
				local pix = CreateMOPixel("Laser Rifle Glow 0", "Techion.rte");
				pix.Pos = startPos + trace * i/particleCount;
				pix.Vel = self.Vel;
				table.insert(self.addedParticles, pix);
			end
		end
		self.shotCounter = (self.shotCounter + 1) % self.strengthVariation;
		self.cooldown:Reset();
	end

	if self.Magazine and self.Magazine.RoundCount > 0 then
		local ammoRatio = 1 - self.Magazine.RoundCount/self.Magazine.Capacity;
		self.emitSmoke(math.floor(ammoRatio * RangeRand(0.5, 2.0) + RangeRand(0.25, 0.50)));
		self.cooldown:SetSimTimeLimitMS(self.ReloadTime * ammoRatio);

		local cooldownRate = math.floor(self.cooldown.ElapsedSimTimeMS/(60000/(self.RateOfFire * self.cooldownSpeed)));
		if ammoRatio ~= 0 and cooldownRate >= 1 then
			self.Magazine.RoundCount = math.min(self.Magazine.RoundCount + cooldownRate, self.Magazine.Capacity);
			self.cooldown:Reset();
		end
		self.FireSound.Pitch = (1.0 - ammoRatio * 0.1)^2;
	elseif self:IsReloading() then
		self.emitSmoke(math.floor((self.cooldown:LeftTillSimTimeLimitMS()/self.ReloadTime) * RangeRand(0.5, 2.0) + RangeRand(0.50, 0.75)));
	elseif self.RoundInMagCount >= 0 then
		self:Reload();
	end
end

function SyncedUpdate(self)
	if self.addedWound then
		local mo = MovableMan:FindObjectByUniqueID(self.addedWoundToUniqueID);
		if mo then
			mo = ToMOSRotating(mo);
			mo:AddWound(self.addedWound, self.addedWoundOffset, true);
		end
		
		self.addedWound = nil;
		self.addedWoundOffset = nil;
		self.addedWoundToMOID = nil;
		self.addedWoundToUniqueID = nil;
	end

	for i = 1, #self.addedParticles do
		MovableMan:AddParticle(self.addedParticles[i]);
	end
	self.addedParticles = {};
end