function Create(self)
	self.turnSpeed = 0.015;	--Speed of the turret turning, in rad per frame
	self.searchRange = self:NumberValueExists("TurretSearchRange") and self:GetNumberValue("TurretSearchRange") or 250;	--Detection area diameter or max ray distance to search enemies from, in px
	--Whether turret searches for enemies in a larger area. Default mode is a narrower ray detection
	self.areaMode = true;
	--Toggle visibility of the aim area / trace to see how the detection works
	self.showAim = false;
	--Angle alteration variables (do not touch)
	self.rotation = 0;	--Left / Right movement affecting turret angle
	self.verticalFactor = 0;	--How Up / Down movement affects turret angle

	self.fireTimer = Timer();
	self.fireTimer:SetSimTimeLimitMS(500);
end

function Update(self)
	local parent = self:GetParent();
	if parent then
		--Aim away from parent according to offset
		self.InheritedRotAngleOffset = (math.pi * 0.5 * self.verticalFactor + self.ParentOffset.AbsRadAngle)/(1 + self.verticalFactor) - self.rotation;
		if IsActor(parent) then
			parent = ToActor(parent);
			if parent.Status ~= Actor.STABLE then
				return;
			end
			local controller = parent:GetController();

			if controller:IsState(Controller.MOVE_RIGHT) then
				self.rotation = self.rotation + self.turnSpeed;
			end
			if controller:IsState(Controller.MOVE_LEFT) then
				self.rotation = self.rotation - self.turnSpeed;
			end
			--Spread / tighten aim when moving up / down
			if controller:IsState(Controller.MOVE_DOWN) and self.ParentOffset.X ~= 0 then
				self.verticalFactor = self.verticalFactor - self.turnSpeed;
			end
		end

		if math.abs(self.rotation) > 0.001 then
			self.rotation = self.rotation/(1 + self.turnSpeed * 2);
		else
			self.rotation = 0;
		end

		if math.abs(self.verticalFactor) > 0.001 then
			self.verticalFactor = self.verticalFactor/(1 + self.turnSpeed * 4);
		else
			self.verticalFactor = 0;
		end

		if self.areaMode then	--Area Mode
			local aimPos = self.Pos + Vector((self.searchRange * 0.5), 0):RadRotate(self.RotAngle);
			--Debug: visualize aim area
			if self.showAim then
				PrimitiveMan:DrawCirclePrimitive(self.Team, aimPos, (self.searchRange * 0.5), 13);
			end
			local aimTarget = MovableMan:GetClosestEnemyActor(self.Team, aimPos, (self.searchRange * 0.5), Vector());
			if aimTarget and aimTarget.Status < Actor.INACTIVE then
				--Debug: visualize search trace
				if self.showAim then
					PrimitiveMan:DrawLinePrimitive(self.Team, aimPos, aimTarget.Pos, 13);
				end
				--Check that the target isn't obscured by terrain
				local aimTrace = SceneMan:ShortestDistance(self.Pos, aimTarget.Pos, SceneMan.SceneWrapsX);
				if not SceneMan:CastStrengthRay(self.Pos, aimTrace, 30, Vector(), 5, 0, SceneMan.SceneWrapsX) then
					self.RotAngle = aimTrace.AbsRadAngle;
					--Debug: visualize aim trace
					if self.showAim then
						PrimitiveMan:DrawLinePrimitive(self.Team, self.Pos, aimTarget.Pos, 254);
					end
					self:EnableEmission(true);
					self:TriggerBurst();
					self.fireTimer:Reset();
				end
			end
		else	--Default Mode
			local target;
			local aimTrace = Vector(self.searchRange, 0):RadRotate(self.RotAngle);
			--Search for MOs directly in line of sight of two rays
			local moCheck1 = SceneMan:CastMORay(self.Pos, aimTrace:RadRotate(1/math.sqrt(self.searchRange)), parent.ID, self.Team, rte.airID, false, 5);
			if moCheck1 ~= rte.NoMOID then
				target = MovableMan:GetMOFromID(MovableMan:GetMOFromID(moCheck1).RootID);
			else
				local moCheck2 = SceneMan:CastMORay(self.Pos, aimTrace:RadRotate(-1/math.sqrt(self.searchRange)), parent.ID, self.Team, rte.airID, false, 5);
				if moCheck2 ~= rte.NoMOID then
					target = MovableMan:GetMOFromID(MovableMan:GetMOFromID(moCheck2).RootID);
				end
			end

			local color = 13;	--Debug trace color: red
			if target and IsActor(target) and ToActor(target).Status < Actor.INACTIVE then
				self:EnableEmission(true);
				self:TriggerBurst();
				self.fireTimer:Reset();

				if self:IsSetToBurst() then
					color = 254;	--Debug trace color: white
				end
			end

			--Debug: visualize aim traces
			if self.showAim then
				PrimitiveMan:DrawLinePrimitive(self.Team, self.Pos, self.Pos + aimTrace:RadRotate(1/math.sqrt(self.searchRange)), color);
				PrimitiveMan:DrawLinePrimitive(self.Team, self.Pos, self.Pos + aimTrace:RadRotate(-1/math.sqrt(self.searchRange)), color);
			end
		end
	end
	
	if self.fireTimer:IsPastSimTimeLimit() then
		self:EnableEmission(false);
	end
end