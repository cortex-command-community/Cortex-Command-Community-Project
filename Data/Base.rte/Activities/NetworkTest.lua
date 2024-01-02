package.loaded.Constants = nil; require("Constants");

-----------------------------------------------------------------------------------------
-- Start Activity
-----------------------------------------------------------------------------------------

function NetworkTest:StartActivity()
	print("START! -- NetworkTest:StartActivity()!");

	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) then
			-- Check if we already have a brain assigned
			if not self:GetPlayerBrain(player) then
				local foundBrain = MovableMan:GetUnassignedBrain(self:GetTeamOfPlayer(player));
				-- If we can't find an unassigned brain in the scene to give each player, then force to go into editing mode to place one
				if not foundBrain then
					foundBrain = CreateAHuman("Brain Robot");
					local weapon = CreateHDFirearm("Battle Rifle", "Base.rte");
					foundBrain:AddInventoryItem(weapon);

					local device = CreateHDFirearm("Medium Digger");
					foundBrain:AddInventoryItem(device);

					local device = CreateHDFirearm("Gatling Gun");
					if device then
						foundBrain:AddInventoryItem(device);
					end

					foundBrain.Team = 0;
					foundBrain.Pos = SceneMan:MovePointToGround(Vector(200 + player * 25,0) , 0 , 3);
					MovableMan:AddActor(foundBrain);
				end

				-- Set the found brain to be the selected actor at start
				self:SetPlayerBrain(foundBrain, player);
				self:SwitchToActor(foundBrain, player, self:GetTeamOfPlayer(player));
				self:SetLandingZone(self:GetPlayerBrain(player).Pos, player);
				-- Set the observation target to the brain, so that if/when it dies, the view flies to it in observation mode
				self:SetObservationTarget(self:GetPlayerBrain(player).Pos, player);
			end
		end
	end

	local ship = RandomACDropShip("Dropship MK1", "Base.rte");
	local cargo = CreateAHuman("Fat Culled Clone");
	ship.Pos = SceneMan:MovePointToGround(Vector(0,0) , 75 , 3);
	ship:AddInventoryItem(cargo);
	ship.Team = 0;
	--MovableMan:AddActor(ship);
	--ship:GibThis();

	self.SoundTimer = Timer();

	local presets = {"Default", "Infantry Brain", "Infantry Light", "Infantry Heavy", "Infantry CQB", "Infantry Grenadier", "Infantry Sniper", "Infantry Engineer", "Mecha"};

	--local b = ActivityMan:GetActivity():GetPlayerBrain(0);
	--print (b);
	--MovableMan:AddActor(b);

	for i = 1 , #presets do
		--[[local a = CreateAHuman("Fat Culled Clone");
		a.Team = 0;
		a.Pos = SceneMan:MovePointToGround(Vector(-200 - i * 100,0) , 0 , 3);
		local weapon = CreateHDFirearm("Battle Rifle", "Base.rte");
		a:AddInventoryItem(weapon);
		MovableMan:AddActor(a);--]]--

		local a = PresetMan:GetLoadout(presets[i], "Coalition.rte", false);
		print (type(a));
		print (a);
		--local a = CreateAHuman("Fat Culled Clone");
		--print (type(a));
		--print (a);
		--local a = CreateAHuman("Fat Culled Clone");
		--if IsActor(a) then
			a.Team = 0;
			a.Pos = SceneMan:MovePointToGround(Vector(-200 - i * 50,0) , 0 , 3);
			--MovableMan:AddActor(a);
		--end
	end--]]--

end

-----------------------------------------------------------------------------------------
-- Pause Activity
-----------------------------------------------------------------------------------------

function NetworkTest:PauseActivity(pause)
	print("PAUSE! -- NetworkTest:PauseActivity()!");
end

-----------------------------------------------------------------------------------------
-- End Activity
-----------------------------------------------------------------------------------------

function NetworkTest:EndActivity()
	print("END! -- NetworkTest:EndActivity()!");
end

-----------------------------------------------------------------------------------------
-- Update Activity
-----------------------------------------------------------------------------------------

function NetworkTest:UpdateActivity()
	if self.ActivityState == Activity.EDITING then
		-- Game is in editing or other modes, so open all does and reset the game running timer
		MovableMan:OpenAllDoors(true, Activity.NOTEAM);
		-- self.StartTimer:Reset();
	end
	--ConsoleMan:ForceVisibility(true);

	--PrimitiveMan:DrawBoxPrimitive(Vector(1,401), Vector(9,409), 20);

	--PrimitiveMan:DrawBoxPrimitive(-1, Vector(30,430), Vector(50,450), 30);

		--for actor in MovableMan.Actors do
		--	print (actor.DeploymentID);
		--end

	--[[for deployment in SceneMan.Scene.Deployments do
		-- Check if spawned actors still live, if so don't spawn them
		local doSpawn = true;
		for actor in MovableMan.Actors do
			if actor.DeploymentID == deployment.ID then
				doSpawn = false;
				break;
			end
		end
		for actor in MovableMan.AddedActors do
			if actor.DeploymentID == deployment.ID then
				doSpawn = false;
				break;
			end
		end

		if doSpawn then
			print (deployment:GetLoadoutName());
			local spawn = deployment:CreateDeployedActor();
			MovableMan:AddActor(spawn);
		end
	end--]]--

	if self.SoundTimer:IsPastSimMS(1000) then
		local snd = AudioMan:PlaySound("Dummy.rte/Effects/Sounds/BlasterFire.flac", Vector(math.random(), math.random()));
		self.SoundTimer:Reset();
	end

	PrimitiveMan:DrawBoxPrimitive(0, Vector(10,410), Vector(100,449), 5);
	PrimitiveMan:DrawBoxPrimitive(1, Vector(10,450), Vector(100,500), 10);

	PrimitiveMan:DrawTextPrimitive(0, Vector(30, 430), "Player One Sees This!", true, 0);
	PrimitiveMan:DrawTextPrimitive(1, Vector(30, 430), "Player Two Sees This!", true, 0);

	for actor in MovableMan.Actors do
		--[[if actor.ClassName == "ACRocket" then
			local rocket = ToACRocket(actor);
			if rocket then
				local engine = rocket.MainEngine;
				if engine then
					for em in engine.Emissions do
						em.ParticlesPerMinute = 0;
						em.BurstSize = 0;
						em.PushesEmitter = false;
					end
				end
			end
		end--]]--

		if actor.ClassName == "ACDropShip" then
			local ds = ToACDropShip(actor);
			if ds.LeftEngine.HitWhatParticleUniqueID ~= 0 then
				local obj = MovableMan:FindObjectByUniqueID(ds.LeftEngine.HitWhatParticleUniqueID);
				print (ds.LeftEngine.HitWhatParticleUniqueID);
			end
			if ds.RightEngine.HitWhatParticleUniqueID ~= 0 then
				local obj = MovableMan:FindObjectByUniqueID(ds.RightEngine.HitWhatParticleUniqueID);
				print (obj);
				print (ds);
				print (ds.RightEngine.HitWhatParticleUniqueID);
			end
		end--]]--


		--[[if actor.HitWhatMOID ~= rte.NoMOID then
			print (actor.HitWhatMOID);
		end
		if actor.HitWhatTerrMaterial ~= rte.airID then
			print (actor.HitWhatTerrMaterial);
		end--]]--


		--[[if actor.TravelImpulse:MagnitudeIsGreaterThan(0) then
			actor:FlashWhite(75);
		end

		if actor.TravelImpulse:MagnitudeIsGreaterThan(actor.ImpulseDamageThreshold / 2) then
			FrameMan:FlashScreen(0, 10, 250);
		end--]]--
	end

	for p in MovableMan.Particles do
		if p.HitWhatMOID ~= rte.NoMOID then
			--print (p.HitWhatMOID);
		end
		if p.HitWhatTerrMaterial ~= rte.airID then
			--print (p.HitWhatTerrMaterial);
		end
	end--]]--
end