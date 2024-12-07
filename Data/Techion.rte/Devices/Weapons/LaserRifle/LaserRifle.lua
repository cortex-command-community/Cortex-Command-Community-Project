local function InsertParticle(var, particle)
	var.addedParticleCount = var.addedParticleCount + 1;
	var.addedParticles[var.addedParticleCount] = particle;
end

local function emitSmoke(self, particleCount)
	local var = self.var;
	local MuzzlePos = self.MuzzlePos;
	for i = 1, particleCount do
		local smoke = CreateMOSParticle("Tiny Smoke Ball 1" .. (math.random() < 0.5 and " Glow Blue" or ""), "Base.rte");
		smoke.Pos = MuzzlePos;
		smoke.Lifetime = smoke.Lifetime * RangeRand(0.5, 1.0);
		smoke.Vel = var.Vel * 0.5 + Vector(RangeRand(0, i), 0):RadRotate(RangeRand(-math.pi, math.pi));
		InsertParticle(var, smoke);
	end
	self:RequestSyncedUpdate();
end

local function drawThiccWobblyLine(screen, startPos, endPos, thiccness, color)
	local dirVector = SceneMan:ShortestDistance(startPos, endPos, true):Perpendicularize()
	local pos1 = startPos + dirVector:SetMagnitude((thiccness - 1) / 2 + math.random(-1, 0))
	local pos2 = startPos - dirVector:SetMagnitude((thiccness - 1) / 2 + math.random(-1, 0))
	local pos3 = endPos + dirVector:SetMagnitude((thiccness - 1) / 2 + math.random(-1, 0))
	local pos4 = endPos - dirVector:SetMagnitude((thiccness - 1) / 2 + math.random(-1, 0))

	PrimitiveMan:DrawTriangleFillPrimitive(screen, pos1, pos2, pos3, color)
	PrimitiveMan:DrawTriangleFillPrimitive(screen, pos3, pos4, pos2, color)
end

function Create(self)
	-- Create local table to store variables for performance
	local var = {};

	-- var.range = math.sqrt(FrameMan.PlayerScreenWidth^2 + FrameMan.PlayerScreenHeight^2)/2;
	var.range = 1000;
	var.penetrationStrength = 170;
	var.strengthVariation = 5;
	--This value tracks the shots and varies the penetration strength to create a "resistance" effect on tougher materials
	var.shotCounter = 0;	--TODO: Rename/describe this variable better
	var.activity = ActivityMan:GetActivity();

	var.cooldown = Timer();
	var.cooldownSpeed = 0.5;

	var.addedParticles = {};
	var.addedParticleCount = 0;

	var.addedWound = nil;
	var.addedWoundOffset = nil;
	var.addedWoundToUniqueID = nil;
	var.Vel = self.Vel;

	self.var = var;
end

function ThreadedUpdate(self)
	local var = self.var;
	if self.FiredFrame then
		local actor = self:GetRootParent();
		local range = var.range + math.random(8);
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
			local strengthFactor = math.max(1 - rayLength/var.range, math.random()) * (var.shotCounter + 1)/var.strengthVariation;

			local addedWoundToMOID = SceneMan:GetMOIDPixel(hitPos.X, hitPos.Y);
			if addedWoundToMOID ~= rte.NoMOID and addedWoundToMOID ~= self.ID then
				local mo = ToMOSRotating(MovableMan:GetMOFromID(addedWoundToMOID));
				if var.penetrationStrength * strengthFactor >= mo.Material.StructuralIntegrity then
					local moAngle = -mo.RotAngle * mo.FlipFactor;

					local woundName = mo:GetEntryWoundPresetName();
					if woundName ~= "" then
						local wound = CreateAEmitter(woundName);

						local dist = SceneMan:ShortestDistance(mo.Pos, hitPos, SceneMan.SceneWrapsX);
						local woundOffset = Vector(dist.X * mo.FlipFactor, dist.Y):RadRotate(moAngle):SetMagnitude(dist.Magnitude - (wound.Radius - 1) * wound.Scale);
						wound.InheritedRotAngleOffset = woundOffset.AbsRadAngle;
						woundOffset = woundOffset:RadRotate(-mo.RotAngle);
						var.addedWound = wound;
						var.addedWoundOffset = woundOffset;
						var.addedWoundToUniqueID = mo.UniqueID;
						self:RequestSyncedUpdate();
					end
				end
			end

			local smoke = CreateMOSParticle("Tiny Smoke Ball 1" .. (math.random() < 0.5 and " Glow Blue" or ""), "Base.rte");
			smoke.Pos = gapPos;
			smoke.Vel = Vector(-trace.X, -trace.Y):SetMagnitude(math.random(3, 6)):RadRotate(RangeRand(-1.5, 1.5));
			smoke.Lifetime = smoke.Lifetime * strengthFactor;
			InsertParticle(var, smoke);

			local pix = CreateMOPixel("Laser Rifle Glow " .. math.floor(strengthFactor * 4 + 0.5), "Techion.rte");
			pix.Pos = gapPos;
			pix.Sharpness = var.penetrationStrength/6;
			pix.Vel = Vector(trace.X, trace.Y):SetMagnitude(6);
			InsertParticle(var, pix);
		end
		if rayLength ~= 0 then
			trace = SceneMan:ShortestDistance(startPos, gapPos, SceneMan.SceneWrapsX);
			for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
				local team = var.activity:GetTeamOfPlayer(player);
				local screen = var.activity:ScreenOfPlayer(player);
				if screen ~= -1 and not (SceneMan:IsUnseen(startPos.X, startPos.Y, team) or SceneMan:IsUnseen(hitPos.X, hitPos.Y, team)) then
					drawThiccWobblyLine(screen, startPos, startPos + trace, 3, 198);
					PrimitiveMan:DrawLinePrimitive(screen, startPos, startPos + trace, 254);
				end
			end
		end
		var.shotCounter = (var.shotCounter + 1) % var.strengthVariation;
		var.cooldown:Reset();
	end

	if self.Magazine and self.Magazine.RoundCount > 0 then
		local ammoRatio = 1 - self.Magazine.RoundCount/self.Magazine.Capacity;
		emitSmoke(self, math.floor(ammoRatio * RangeRand(0.5, 2.0) + RangeRand(0.25, 0.50)));
		var.cooldown:SetSimTimeLimitMS(self.ReloadTime * ammoRatio);

		local cooldownRate = math.floor(var.cooldown.ElapsedSimTimeMS/(60000/(self.RateOfFire * var.cooldownSpeed)));
		if ammoRatio ~= 0 and cooldownRate >= 1 then
			self.Magazine.RoundCount = math.min(self.Magazine.RoundCount + cooldownRate, self.Magazine.Capacity);
			var.cooldown:Reset();
		end
		self.FireSound.Pitch = (1.0 - ammoRatio * 0.1)^2;
	elseif self:IsReloading() then
		emitSmoke(self, math.floor((var.cooldown:LeftTillSimTimeLimitMS()/self.ReloadTime) * RangeRand(0.5, 2.0) + RangeRand(0.50, 0.75)));
	elseif self.RoundInMagCount >= 0 then
		self:Reload();
	end
end

-- Localize function to improve performance
local AddParticle = MovableMan.AddParticle;

function SyncedUpdate(self)
	local var = self.var;
	if var.addedWound then
		local mo = MovableMan:FindObjectByUniqueID(var.addedWoundToUniqueID);
		if mo then
			mo = ToMOSRotating(mo);
			mo:AddWound(var.addedWound, var.addedWoundOffset, true);
		end
		
		var.addedWound = nil;
		var.addedWoundOffset = nil;
		var.addedWoundToUniqueID = nil;
	end

	if (var.addedParticleCount > 0) then
		for i = 1, var.addedParticleCount do
			-- Remember to add function caller as first argument to localized function
			AddParticle(MovableMan, var.addedParticles[i]);
		end
		var.addedParticles = {};
		var.addedParticleCount = 0;
	end
end