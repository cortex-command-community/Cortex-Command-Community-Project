function Create(self)
	-- Make sure the teleporter list exists.
	if teleporterlista == nil then
		_G["teleporterlista"] = {};
	end

	-- List for storing who can teleport.
	if _G["cantele"] == nil then
		_G["cantele"] = {};
	end
	local cantele = _G["cantele"];

	-- Add self to teleporter list.
	teleporterlista[#teleporterlista + 1] = self;

	-- Stores where on the teleporter list this teleporter is.  Used for assigning a partner.
	self.listnum = #teleporterlista;

	-- Initialize delay timer if it doesn't exist yet.
	if cantele[self.listnum] == nil then
		cantele[self.listnum] = Timer();
	else
	-- Otherwise, reset it.
		cantele[self.listnum]:Reset();
	end

	-- This variable stores which teleporter is this one's "partner", or the one that is linked to it.
	self.partner = nil;

	-- How long it takes between teleports.
	self.portSpeed = 0.5;
	self.porttimemax = 1500;
	self.porttime = self.porttimemax;

	-- Timer to count how long since the last teleportation.
	self.porttimer = Timer();

	-- How long since creation.
	self.creationtimer = Timer();
end

function Update(self)
	local teleporterlistb = _G["teleporterlistb"];
	local cantele = _G["cantele"];
	if teleporterlistb then
		-- A delay so that all teleporters will have been placed by the time the code activates.
		if self.creationtimer:IsPastSimMS(1000) and ActivityMan:GetActivity().ActivityState ~= Activity.EDITING then
			-- Check if the teleporter is linked yet.
			if MovableMan:IsParticle(self.partner) == false then
				-- If not, try to assign a partner.
				self.partner = teleporterlistb[self.listnum];
				-- Turn on spinning effect.
				self:EnableEmission(true);
			elseif cantele[self.listnum]:IsPastSimMS(self.porttime) then
				-- Cycle through all actors.
				local target = nil;
				for actor in MovableMan.Actors do
					if IsActor(actor) then
						local dist = SceneMan:ShortestDistance(self.Pos, actor.Pos, false);
						if dist:MagnitudeIsLessThan(25) and actor.PinStrength == 0 then
							target = actor;
							-- Chargeup.
							self.porttime = self.porttime / (1 + self.portSpeed);
							-- Make the actor glow for a while.
							actor:FlashWhite(10 + self.porttime / 10);
							local glow = CreateMOPixel("Teleporter Glow Short");
							glow.Pos = self.Pos;
							MovableMan:AddParticle(glow);
							if self.porttime < 20 then
								-- Teleport the actor.
								actor.Pos = self.partner.Pos + dist;
								-- Create the teleportation effect for both teleporters in the set.
								local pos = {self.Pos, self.partner.Pos};
								for i = 1, #pos do
									local fx = CreateAEmitter("Teleporter Effect A");
									fx.Pos = pos[i];
									MovableMan:AddParticle(fx);
									local glow = CreateMOPixel("Teleporter Glow");
									glow.Pos = pos[i];
									MovableMan:AddParticle(glow);
								end
							end

							-- Shut off teleportation on this set until the delay is up.
							cantele[self.listnum]:Reset();
						end
					end
				end

				if target == nil then
					self.porttime = self.porttimemax;
				end
			end
		end
		
		self.SpriteAnimDuration = self.porttime;
	end
end

function Destroy(self)
	-- Remove self from teleporter list.
	teleporterlista[self.listnum] = nil;
end