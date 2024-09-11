function Create(self)
	self.bashWindupSound = CreateSoundContainer("Metal Shield Foley", "Base.rte");
	self.bashSwingSound = CreateSoundContainer("Browncoat RS-04 Bash Swing", "Browncoats.rte");
	self.bashImpactSound = CreateSoundContainer("Browncoat RS-04 Bash Impact", "Browncoats.rte");

	self.bashTimer = Timer();
	self.bashCoolDown = false;
	self.Bashing = false;
	self.bashActive = false;
	
	self.bashWindupTime = 225;
	self.bashActiveTime = 300;
	self.bashCoolDownTime = 500;
	
	self.bashWindupStanceOffset = Vector(5, 2);
	self.bashActiveStanceOffset = Vector(18, -1);
	
	self.origStanceOffset = Vector(self.StanceOffset.X*self.FlipFactor, self.StanceOffset.Y);
	self.origSharpStanceOffset = Vector(self.SharpStanceOffset.X, self.SharpStanceOffset.Y);

	self.equippedInMainHand = false;
	self:SetOneHanded(true);
	local rootParent = self:GetRootParent();
	if IsAHuman(rootParent) and ToAHuman(rootParent).EquippedItem and ToAHuman(rootParent).EquippedItem.UniqueID == self.UniqueID then
		ToAHuman(rootParent):UnequipBGArm();
		self.equippedInMainHand = true;
		self:SetOneHanded(false);
	end
end

function OnAttach(self, newParent)
	self.equippedInMainHand = false;
	self:SetOneHanded(true);
	self.Bashing = false;
	self.bashActive = false;
	self.HitsMOs = false	local rootParent = self:GetRootParent();
	if IsAHuman(rootParent) and ToAHuman(rootParent).EquippedItem and ToAHuman(rootParent).EquippedItem.UniqueID == self.UniqueID then
		ToAHuman(rootParent):UnequipBGArm();
		self.equippedInMainHand = true;
		self:SetOneHanded(false);
	end
end

function ThreadedUpdate(self)
	self.hitActor = nil;

	self.bashWindupSound.Pos = self.Pos;
	self.bashSwingSound.Pos = self.Pos;

	if self.equippedInMainHand then
		if not self.bashCoolDown then
			if self:IsActivated() then
				if not self.Bashing then
					self.bashWindupSound:Play(self.Pos);
					self.bashSwingSound:Play(self.Pos);
					self.hitMOTable = {};
					self.Bashing = true;
					self.bashTimer:Reset();
					self.StanceOffset = self.bashWindupStanceOffset;
					self.SharpStanceOffset = self.bashWindupStanceOffset;
				end
			end
		elseif self.bashTimer:IsPastSimMS(self.bashCoolDownTime) then
			self.bashCoolDown = false;
		end	
		
		if self.Bashing then
			if self.bashTimer:IsPastSimMS(self.bashWindupTime + self.bashActiveTime) then
				self.Bashing = false;
				self.bashActive = false;
				self.bashTimer:Reset();
				self.bashCoolDown = true;
				self.StanceOffset = self.origStanceOffset;
				self.SharpStanceOffset = self.origSharpStanceOffset;
			elseif self.bashTimer:IsPastSimMS(self.bashWindupTime) then
				local topRayPos = self.Pos + Vector(0, 12):RadRotate(self.RotAngle);
				local midRayPos = self.Pos;
				local bottomRayPos = self.Pos + Vector(0, -12):RadRotate(self.RotAngle);
				
				local trace = Vector(8 * self.FlipFactor, 0):RadRotate(self.RotAngle);
				
				--PrimitiveMan:DrawLinePrimitive(0, topRayPos, topRayPos + trace, 255);
				--PrimitiveMan:DrawLinePrimitive(0, midRayPos, midRayPos + trace, 255);
				--PrimitiveMan:DrawLinePrimitive(0, bottomRayPos, bottomRayPos + trace, 255);
				
				local rayID = SceneMan:CastMORay(topRayPos, trace, self.ID, self.Team, rte.grassID, false, 2);
				local rayID = rayID == rte.NoMOID and SceneMan:CastMORay(midRayPos, trace, self.ID, self.Team, rte.grassID, false, 2) or rayID;
				local rayID = rayID == rte.NoMOID and SceneMan:CastMORay(bottomRayPos, trace, self.ID, self.Team, rte.grassID, false, 2) or rayID;
				if rayID ~= rte.NoMOID then
					local MO = MovableMan:GetMOFromID(rayID);
					local rootMO = MO:GetRootParent();
					if not self.hitMOTable[rootMO.UniqueID] and IsActor(rootMO) then
						self.hitMOTable[rootMO.UniqueID] = true;
						self.hitActor = ToActor(rootMO);
						self.bashImpactSound:Play(self.Pos);
						self:RequestSyncedUpdate();
					end
				end

				if not self.bashActive then
					self.bashActive = true;
				end
				
				self.StanceOffset = self.bashActiveStanceOffset;
				self.SharpStanceOffset = self.bashActiveStanceOffset;
			end
		end
	end
end

function SyncedUpdate(self)
	if self.hitActor and MovableMan:ValidMO(self.hitActor) then
		local ctrl = self.hitActor:GetController();
		
		self.hitActor.Status = math.max(1, self.hitActor.Status);
		self.hitActor.AngularVel = self.hitActor.AngularVel - 5*self.FlipFactor;
		for i = 0, 29 do --Go through and disable the gameplay-related controller states
			ctrl:SetState(i, false);
		end
		
		if self.hitActor.BodyHitSound then
			self.hitActor.BodyHitSound:Play(self.hitActor.Pos);
		end
		
		local attachablesTriggered = 0;
		
		for att in self.Attachables do
			if math.random(0, 100) > 25 then
				attachablesTriggered = attachablesTriggered + 1;
				local width = math.floor(ToMOSprite(att):GetSpriteWidth() * 0.5 + 0.5);
				local offset = Vector(math.random(-width, width), math.random(-width, width));
				local setAngle = self.RotAngle;
				local woundName = att:GetEntryWoundPresetName();
				local wound;
				if woundName ~= "" then
					wound = CreateAEmitter(woundName);
					wound.RotAngle = setAngle;
					if wound.BurstSound then
						wound.BurstSound.Volume = math.max(0, 1 - (0.4*attachablesTriggered));
					end
					
					att:AddWound(wound, offset, true);
				end
			end
		end
	end
end