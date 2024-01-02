--Self-healing script complete with animations, for AHuman use only
function Create(self)
	self.baseHealDelay = 500;
	self.woundDamageReturnRate = 0.5;
	self.PieMenu:AddPieSlice(CreatePieSlice("Self Heal"), self);
	self.healCrossParticle = CreateMOSParticle("Particle Heal Effect", "Base.rte");
end

function Update(self)
	local controller = self:GetController();
	if self.healing and self.healing.part then
		if self.healing.wound then
			if self.healing.timer:IsPastSimMS(self.healing.delay) then
				self.healing.timer:Reset();
				self.Health = math.min(self.Health + self.healing.wound.BurstDamage * math.max(0, self.healing.part.DamageMultiplier) * self.woundDamageReturnRate, self.MaxHealth);
				self.healing.wound.ToDelete = true;
				self.healing.wound = nil;
				for wound in self.healing.part.Wounds do
					if not wound.ToDelete then
						self.healing.wound = wound;
						self.healing.delay = self.baseHealDelay * math.sqrt(self.healing.wound.Radius);
					end
				end
				local cross = self.healCrossParticle:Clone();
				cross.Pos = self.AboveHUDPos + Vector(0, 4);
				MovableMan:AddParticle(cross);
			else
				local timerRatio = self.healing.timer.ElapsedSimTimeMS/self.healing.delay;
				local arms = {self.FGArm, self.BGArm};
				if self.Head and self.healing.part.ID == self.Head.ID then
					self.Head.RotAngle = self.RotAngle - 0.5 * self.FlipFactor * math.sin(timerRatio * math.pi);
				end
				for _, arm in pairs(arms) do
					if arm then
						if self.healing.part.ID ~= arm.ID then
							local offset = SceneMan:ShortestDistance(arm.JointPos, self.healing.wound.Pos, SceneMan.SceneWrapsX) + Vector(-math.max(self.healing.wound.Radius - 2, 1), 0):RadRotate(math.pi * 2 * timerRatio);
							arm.HandPos = arm.JointPos + Vector(offset.X, offset.Y):SetMagnitude(math.min(offset.Magnitude, arm.MaxLength));
						else
							arm.HandPos = arm.JointPos + Vector(arm.MaxLength * (0.5 + math.sin(timerRatio * math.pi) * 0.5) * self.FlipFactor, 0):RadRotate(self.RotAngle);
						end
					end
				end
				self.healing.wound.Scale = math.min(self.healing.wound.Scale, self.healing.wound.Scale * (1 - timerRatio));
			end
		else	
			self.healing = nil;
		end
		if self.EquippedItem or self.EquippedBGItem or controller:IsState(Controller.BODY_JUMP) or (self.FGArm or self.BGArm) == nil then
			self.healing = nil;
			self:RemoveNumberValue("SelfHeal");
		else
			for state = 1, 15 do
				controller:SetState(state, false);
			end
			controller:SetState(Controller.BODY_CROUCH, true);
		end
	elseif self:NumberValueExists("SelfHeal") then
		self.healing = {};
		local priority = self:GetWoundCount(false, false, false) * self.DamageMultiplier;
		self.healing.part = self;
		for wound in self.Wounds do
			self.healing.wound = wound;
		end
		for attachable in self.Attachables do
			if attachable.DamageMultiplier > 0 then
				local woundCount = attachable:GetWoundCount(false, false, false) * attachable.DamageMultiplier;
				if woundCount > priority then
					priority = woundCount;
					self.healing.part = attachable;
					for wound in attachable.Wounds do
						self.healing.wound = wound;
						break;
					end
				end
				--Find damage-transferring sub-attachables as well
				for attAttachable in attachable.Attachables do
					if attAttachable.DamageMultiplier > 0 then
						local woundCount = attAttachable:GetWoundCount(false, false, false) * attAttachable.DamageMultiplier;
						if woundCount > priority then
							priority = woundCount * attachable.DamageMultiplier;
							self.healing.part = attAttachable;
							for wound in attAttachable.Wounds do
								self.healing.wound = wound;
								break;
							end
						end
					end
				end
			end
		end
		if self.healing.part and self.healing.wound then
			self.healing.timer = Timer();
			self.healing.delay = self.baseHealDelay * math.sqrt(self.healing.wound.Radius);
			self:UnequipArms();
		else
			self.healing = nil;
			controller:SetState(Controller.WEAPON_CHANGE_PREV, true);
			self:RemoveNumberValue("SelfHeal");
		end
	end
end

function WhilePieMenuOpen(self, openedPieMenu)
	openedPieMenu:GetFirstPieSliceByPresetName("Self Heal").Enabled = self.WoundCount > 0 and (self.FGArm or self.BGArm) ~= nil;
end

function SelfHeal(pieMenuOwner, pieMenu, pieSlice)
	if pieMenuOwner and IsAHuman(pieMenuOwner) then 
		pieMenuOwner = ToAHuman(pieMenuOwner);
		if pieMenuOwner.WoundCount > 0 and (pieMenuOwner.FGArm or pieMenuOwner.BGArm) ~= nil then
			pieMenuOwner:SetNumberValue("SelfHeal", 1);
		else
			local errorSound = CreateSoundContainer("Error", "Base.rte");
			errorSound:Play(pieMenuOwner.Pos, pieMenuOwner:GetController().Player);
		end
	end
end