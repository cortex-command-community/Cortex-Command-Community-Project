function Create(self)
	self.tapTimerAim = Timer();
	self.tapTimerJump = Timer();
	self.tapCounter = 0;
	self.didTap = false;
	self.canTap = false;

	self.tapTime = 200;
	self.tapAmount = 2;
	self.guide = false;

	self.arrow = CreateMOSRotating("Grapple Gun Guide Arrow");
end

function Update(self)
	local parent = self:GetRootParent();
	if parent and IsActor(parent) then
		if IsAHuman(parent) then
			parent = ToAHuman(parent);
		elseif IsACrab(parent) then
			parent = ToACrab(parent);
		else
			parent = ToActor(parent);
		end
		if parent:IsPlayerControlled() and parent.Status < Actor.DYING then
			local controller = parent:GetController();
			local mouse = controller:IsMouseControlled();
			-- Deactivate when equipped in BG arm to allow FG arm shooting
			if parent.EquippedBGItem and parent.EquippedItem then
				if parent.EquippedBGItem.ID == self.ID then
					self:Deactivate();
				end
			end

			if self.Magazine then
				-- Double tapping crouch retrieves the hook
				if self.Magazine.Scale == 1 then
					self.StanceOffset = Vector(ToMOSprite(self:GetParent()):GetSpriteWidth(), 1);
					self.SharpStanceOffset = Vector(ToMOSprite(self:GetParent()):GetSpriteWidth(), 1);
					if controller and controller:IsState(Controller.BODY_CROUCH) then
						if self.canTap then
							controller:SetState(Controller.BODY_CROUCH, false);
							self.tapTimerJump:Reset();
							self.didTap = true;
							self.canTap = false;
							self.tapCounter = self.tapCounter + 1;
						end
					else
						self.canTap = true;
					end

					if self.tapTimerJump:IsPastSimMS(self.tapTime) then
						self.tapCounter = 0;
					else
						if self.tapCounter >= self.tapAmount then
							self:Activate();
							self.tapCounter = 0;
						end
					end
				end

				-- A guide arrow appears at higher speeds
				if (self.Magazine.Scale == 0 and not controller:IsState(Controller.AIM_SHARP)) or parent.Vel:MagnitudeIsGreaterThan(6) then
					self.guide = true;
				else
					self.guide = false;
				end
			end

			if self.guide then
				local frame = 0;
				if parent.Vel:MagnitudeIsGreaterThan(12) then
					frame = 1;
				end
				local startPos = (parent.Pos + parent.EyePos + self.Pos)/3;
				local guidePos = startPos + Vector(parent.AimDistance + (parent.Vel.Magnitude), 0):RadRotate(parent:GetAimAngle(true));
				PrimitiveMan:DrawBitmapPrimitive(ActivityMan:GetActivity():ScreenOfPlayer(controller.Player), guidePos, self.arrow, parent:GetAimAngle(true), frame);
			end
		else
			self:Deactivate();
		end
		
		if self.Magazine then
			self.Magazine.RoundCount = 1;
			self.Magazine.Scale = 1;
			self.Magazine.Frame = 0;
		end
	end
end