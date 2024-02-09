dofile("Base.rte/Constants.lua")
require("AI/NativeCrabAI")

function Create(self)
	self.AI = NativeCrabAI:Create(self)
	self.SearchTimer = Timer()
	self.radarRange = 1024
	self.ValidTargets = {}
	self.Frame = 1

	-- The "AA-Drone Ammo Counter" object tracks ammo across battles and reduce gold cost of the carrier
	if self:HasObject("AA-Drone Ammo Counter") then
		if self.InventorySize == 1 then
			self.Frame = 2 -- One SAM left
		else
			self.Frame = 0 -- Out of ammo
		end
	end

	function self.UpdateInertSAM(self)
		if self.SearchTimer:IsPastSimMS(300) then
			local ArmedSAM = CreateAEmitter("Armed SAM", "Base.rte")
			if ArmedSAM then
				self.SAM.ToDelete = true
				self.SAM.HitsMOs = false
				self.SAM.GetsHitByMOs = false

				ArmedSAM.Pos = self.SAM.Pos
				ArmedSAM.Vel = self.SAM.Vel
				ArmedSAM.Team = self.SAM.Team
				ArmedSAM.IgnoresTeamHits = true
				ArmedSAM.RotAngle = self.SAM.RotAngle
				ArmedSAM.AngularVel = self.SAM.AngularVel

				self.proximityFuze = nil -- Reset the proximity fuze

				-- Don't target the center of a craft
				if self.SAM_Target.ClassName == "ACDropShip" then
					if self.SAM_Target.Pos.X > self.SAM.Pos.X then
						self.SAM_Offset = Vector(-self.SAM_Target.Radius, 0) -- Left
					else
						self.SAM_Offset = Vector(self.SAM_Target.Radius, 0) -- Right
					end
				else
					self.SAM_Offset = Vector(0, self.SAM_Target.Radius * 0.5) -- Below
				end

				-- Initialize missile velocity history and targeting data
				self.SAM_AimPos = self.SAM_Target.Pos + self.SAM_Target:RotateOffset(self.SAM_Offset)
				self.SAM_LastVel = Vector(self.SAM.Vel.X, self.SAM.Vel.Y)

				self.SAM = ArmedSAM
				MovableMan:AddMO(self.SAM)

				self.UpdateSAM = self.UpdateArmedSAM -- Use the UpdateArmedSAM-function from now on
			else
				self.SAM = nil
			end
		else
			local angError = math.asin(Vector(0, -1):Cross(self.SAM:RotateOffset(Vector(1, 0)))) -- The angle between missile facing and straight up
			self.SAM.RotAngle = self.SAM.RotAngle + math.min(math.max(angError, -0.02), 0.02)
			self.SAM.AngularVel = self.SAM.AngularVel * 0.95
		end
	end

	function self.UpdateArmedSAM(self)
		-- Find the velocity vector that will take the missile to the target
		local FutureVel = self.SAM.Vel + (self.SAM.Vel - self.SAM_LastVel) * 10
		local OptimalVel = SceneMan:ShortestDistance(self.SAM.Pos, self.SAM_AimPos, false)
		local angError = math.asin(OptimalVel.Normalized:Cross(FutureVel.Normalized)) -- The angle between FutureVel and OptimalVel

		-- Gradually turn towards the optimal velocity vector
		self.SAM.RotAngle = self.SAM.RotAngle + math.min(math.max(angError, -0.04), 0.04)

		-- Gradually return the thruster to the starting position if the missile is facing the target
		if math.abs(angError) < 0.15 then
			self.SAM.EmitAngle = self.SAM.EmitAngle * 0.8 + math.pi * 0.2
		else
			self.SAM.EmitAngle = math.max(math.min(self.SAM.EmitAngle + angError * 0.1, 4.14), 2.14) -- Vector thrust
		end

		-- Detonate the missile when appropriate
		local range = SceneMan:ShortestDistance(
			self.SAM.Pos,
			self.SAM_Target.Pos + self.SAM_Target:RotateOffset(self.SAM_Offset),
			false
		).Magnitude
		if self.proximityFuze then
			if range < 30 then
				self.SAM:GibThis() -- The target is close enough; detonate
			elseif math.abs(angError) > 1.5 and range > self.proximityFuze then -- The missile is moving away from the target: detonate
				self.SAM:GibThis()
			else
				self.proximityFuze = range
			end
		elseif range < 120 then -- The target is close: arm the proximity fuze
			self.proximityFuze = range
		end

		self.SAM.AngularVel = self.SAM.AngularVel * 0.96 + (self.SAM.EmitAngle - math.pi) * 0.05 -- The vector thrust will cause the SAM to rotate
		self.SAM_LastVel = self.SAM_LastVel * 0.6 + self.SAM.Vel * 0.4 -- Used to calculate the acceleration of the missile
		self.SAM_AimPos = self.SAM_AimPos * 0.6
			+ (
					self.SAM_Target.Pos
					+ self.SAM_Target:RotateOffset(self.SAM_Offset)
					+ self.SAM_Target.Vel * math.min(range / 50, 20)
				)
				* 0.4; -- Filter the AimPos to reduce noise
	end
end

function Update(self)
	if self.SAM then -- Check if any old missile is alive
		if MovableMan:ValidMO(self.SAM) then
			if self.SAM_Target and MovableMan:ValidMO(self.SAM_Target) then
				self.UpdateSAM(self)
			else
				-- The target is not valid any more: replace the missile with an intert one
				if self.SAM.PresetName == "Armed SAM" then
					local InertSAM = CreateAEmitter("Inert SAM", "Base.rte")
					if InertSAM then
						self.SAM.ToDelete = true
						self.SAM.HitsMOs = false
						self.SAM.GetsHitByMOs = false

						InertSAM.Pos = self.SAM.Pos
						InertSAM.Vel = self.SAM.Vel
						InertSAM.RotAngle = self.SAM.RotAngle
						InertSAM.AngularVel = self.SAM.AngularVel
						InertSAM.Team = self.Team
						InertSAM.IgnoresTeamHits = true
						MovableMan:AddMO(InertSAM)
					end
				end

				self.SAM = nil
				self.SAM_Target = nil
			end
		else
			self.SAM = nil
		end
	elseif self.Frame > 0 and self.Vel.Largest < 12 then -- we have SAMs left
		if #self.ValidTargets < 1 then -- Find valid targets
			if self.SearchTimer:IsPastSimMS(100) then
				self.SearchTimer:Reset() -- Only search a few times/sec to reduce calculations per update

				-- Only look for targets if there are no obstacles above us
				local Trace = self:RotateOffset(Vector(0, -200))
				if not SceneMan:CastStrengthRay(self.Pos, Trace, 5, Vector(), 9, -1, true) then -- Terrain str 5
					local obstructed = false
					local ID = SceneMan:CastMORay(self.AboveHUDPos, Trace, self.ID, self.IgnoresWhichTeam, 0, true, 15)
					if ID < rte.NoMOID then
						local MO = MovableMan:GetMOFromID(ID)
						if ID ~= MO.RootID then
							MO = MovableMan:GetMOFromID(MO.RootID)
						end

						if MO.Team == self.Team then
							obstructed = true -- The MO above us is on our team: don't shoot
						end
					end

					if not obstructed then
						local Dist, range, angle
						for Act in MovableMan.Actors do
							if Act.Team ~= self.Team and not Act:IsDead() and Act:HasObjectInGroup("Craft") then
								Dist = SceneMan:ShortestDistance(self.Pos, Act.Pos + Act.Vel * 9, false)
								if
									Act.Vel.Y > 0 or ((Dist.X > 0 and Act.Vel.X < -5) or (Dist.X < 0 and Act.Vel.X > 5))
								then -- Only shoot at craft moving down, or moving towards us
									range = Dist.Magnitude - Act.Radius
									if range < self.radarRange then -- Shoot at enemy craft within radarRange pixels
										angle = math.abs(
											math.asin(Dist.Normalized:Cross(self:RotateOffset(Vector(0, -1))))
										)
										if angle < 1.7 then -- Search in a ~200 degree arc above us
											table.insert(
												self.ValidTargets,
												{
													Actor = Act,
													priority = angle / 3
														+ range / 300
														+ (3 - Act.Health / 100)
														- math.abs(Act.AngularVel),
												}
											) -- prioritize close, damaged targets that are straight above us that does not spin
										end
									end
								end
							end
						end
					end
				end
			end

			-- Sort the targets in ascending order
			if #self.ValidTargets > 1 then
				table.sort(self.ValidTargets, function(A, B)
					return A.priority > B.priority
				end)
			end

			-- Store brain location
			local GmActiv = ActivityMan:GetActivity()
			for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
				if GmActiv:PlayerActive(player) and GmActiv:GetTeamOfPlayer(player) == self.Team then
					local Brain = GmActiv:GetPlayerBrain(player)
					if Brain and MovableMan:IsActor(Brain) then
						self.MyBrainPos = Vector(Brain.Pos.X, Brain.Pos.Y)
					end

					break
				end
			end
		else -- Check if the missile have a clear line of sight to any of the selected targets
			local NewTarget = table.remove(self.ValidTargets).Actor -- Only check one target to reduce calculations per update
			if NewTarget and MovableMan:ValidMO(NewTarget) and not NewTarget:IsDead() then
				local Trace = SceneMan:ShortestDistance(self.AboveHUDPos, NewTarget.Pos, false)
				-- Don't shoot at targets that are out of reach
				if Trace.Magnitude < self.radarRange then
					-- Don't shoot at targets that are very close to the brain
					if
						SceneMan:ShortestDistance(self.MyBrainPos or self.Pos, NewTarget.Pos, false).Largest
							- NewTarget.Radius
						> 120
					then
						-- First do a very inexact scan of half the distance to the target for friendly dropships and terrain
						if
							SceneMan:CastObstacleRay(
								self.AboveHUDPos,
								Trace * 0.5,
								Vector(),
								Vector(),
								self.ID,
								self.IgnoresWhichTeam,
								0,
								25
							) < 0
						then
							-- If nothing was found, do a more exact scan for terrain all the way to the target
							if not SceneMan:CastStrengthRay(self.AboveHUDPos, Trace, 5, Vector(), 9, -1, true) then -- Terrain str 5
								self.SearchTimer:Reset()

								-- Spawn the SAM
								self.SAM = CreateAEmitter("Inert SAM", "Base.rte")
								if self.SAM then
									local SpawnOffset = Vector(0, -15)
									if self.Frame < 2 then
										self.Frame = 2 -- Remove the left SAM
										SpawnOffset.X = -10
									else
										self.Frame = 0 -- Remove the right SAM
										SpawnOffset.X = 10
									end

									self.SAM.Team = self.Team
									self.SAM.RotAngle = self.RotAngle + 1.571
									self.SAM.AngularVel = self.AngularVel
									self.SAM.Pos = self.Pos + self:RotateOffset(SpawnOffset)
									self.SAM.Vel = self.Vel + self:RotateOffset(Vector(0, -17))
									self.SAM.IgnoresTeamHits = true
									self.SAM:TriggerBurst()
									MovableMan:AddMO(self.SAM)

									self.armedSAM = false
									self.SAM_Target = NewTarget

									-- Call this function to update the missile
									self.UpdateSAM = self.UpdateInertSAM
								end

								-- Add an invisible object to the inventory to track ammo
								local AmmoCounter = CreateMOSRotating("AA-Drone Ammo Counter", "Base.rte")
								if AmmoCounter then
									self:AddInventoryItem(AmmoCounter)
								end
							end
						end
					end
				end
			end
		end
	end
end

function UpdateAI(self)
	self.AI:Update(self)
end

function Destroy(self)
	self.AI:Destroy(self)
end
