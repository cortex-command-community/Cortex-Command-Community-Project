function Create(self)
	self.laserTimer = Timer();
	self.laserCheckDelay = 30;
	self.laserLength = self.SharpLength + FrameMan.PlayerScreenWidth * 0.3;
	self.laserSpaceCheck = 8; --For optimization purposes. Smaller value means a more accurate but slower check.

	self.laserDensity = math.ceil(self.laserLength/self.laserSpaceCheck);

	-- Cached value for the synced update
	self.laserPar = nil;
end

function ThreadedUpdate(self)
	if not self:IsReloading() and self.laserTimer:IsPastSimMS(self.laserCheckDelay) then
		self.laserTimer:Reset();
		local actor = self:GetRootParent();
		if IsActor(actor) and ToActor(actor):GetController():IsState(Controller.AIM_SHARP) then
			local actor = ToActor(actor);
			local aimAngle = actor:GetAimAngle(true);
			local roughLandPos = self.MuzzlePos + Vector(self.laserLength, 0):RadRotate(aimAngle);
			for i = 0, self.laserDensity do
				local checkPos = self.MuzzlePos + Vector(self.laserSpaceCheck * i, 0):RadRotate(aimAngle);
				if SceneMan.SceneWrapsX == true then
					if checkPos.X > SceneMan.SceneWidth then
						checkPos = Vector(checkPos.X - SceneMan.SceneWidth, checkPos.Y);
					elseif checkPos.X < 0 then
						checkPos = Vector(SceneMan.SceneWidth + checkPos.X, checkPos.Y);
					end
				end
				local terrCheck = SceneMan:GetTerrMatter(checkPos.X, checkPos.Y);
				if terrCheck == rte.airID then
					local moCheck = SceneMan:GetMOIDPixel(checkPos.X, checkPos.Y);
					if moCheck ~= rte.NoMOID and moCheck ~= self.ID then
						local mo = ToMOSRotating(MovableMan:GetMOFromID(moCheck));
						if mo and (mo.Team ~= actor.Team or (mo.WoundCount > 0 and mo.PresetName ~= "Nano Rifle")) then
							roughLandPos = checkPos;
							break;
						end
					end
				else
					roughLandPos = checkPos;
					break;
				end
			end

			local checkRoughLandPos = roughLandPos + Vector(self.laserSpaceCheck * -1, 0):RadRotate(aimAngle);
			for i = 0, self.laserSpaceCheck do
				local checkPos = checkRoughLandPos + Vector(i, 0):RadRotate(aimAngle);
				if SceneMan.SceneWrapsX == true then
					if checkPos.X > SceneMan.SceneWidth then
						checkPos = Vector(checkPos.X - SceneMan.SceneWidth,checkPos.Y);
					elseif checkPos.X < 0 then
						checkPos = Vector(SceneMan.SceneWidth + checkPos.X, checkPos.Y);
					end
				end
				local terrCheck = SceneMan:GetTerrMatter(checkPos.X, checkPos.Y);
				if terrCheck == rte.airID then
					local moCheck = SceneMan:GetMOIDPixel(checkPos.X, checkPos.Y);
					if moCheck ~= rte.NoMOID and moCheck ~= self.ID then
						if actor:IsPlayerControlled() then
							local mo = MovableMan:GetMOFromID(moCheck);
							PrimitiveMan:DrawCirclePrimitive(ActivityMan:GetActivity():ScreenOfPlayer(actor:GetController().Player), mo.Pos, mo.Radius, 5);
						end
						break;
					end
				else
					break;
				end
				local laserPar = CreateMOPixel("Nano Rifle Laser Sight Glow");
				laserPar.Pos = checkPos;
				laserPar.Lifetime = self.laserCheckDelay * 2;
				self.laserPar = laserPar;
				self:RequestSyncedUpdate();
			end
		end
	end
end

function SyncedUpdate(self)
	if self.laserPar then
		MovableMan:AddParticle(self.laserPar);
		self.laserPar = nil;
	end
end