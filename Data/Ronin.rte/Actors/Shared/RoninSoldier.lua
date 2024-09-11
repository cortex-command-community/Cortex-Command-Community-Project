require("Loadouts");

function Create(self)
	self.updateTimer = Timer();
	if self.Head then
		if not self:NumberValueExists("Identity") then
			self.face = math.random(0, (self.Head.FrameCount * 0.5) - 1);
			if self.Head then
				self.Head.Frame = self.face;
			end
			self:SetNumberValue("Identity", self.face);
			self:SetGoldValue(self:GetGoldValue(0, 1, 1) * 0.4);
		else
			self.face = self:GetNumberValue("Identity");
			if self.Head then
				self.Head.Frame = self.face;
			end
		end
	end

	--Equip loadout actors with random weapons
	if not self:NumberValueExists("Equipped") then
		local headgear;
		if string.find(self.PresetName, "Ronin") then
			local loadoutName = string.gsub(self.PresetName, "Ronin ", "");
			if RoninLoadouts[loadoutName] then
				local unit = RoninLoadouts[loadoutName];
				--Pick a random item out of each set of items
				local firearms = {unit["Primary"], unit["Secondary"], unit["Tertiary"]};
				for class = 1, #firearms do
					if firearms[class] then
						local firearmToCreate = firearms[class][math.random(#firearms[class])];
						local firearm;
						if firearmToCreate == "Riot Shield" then
							firearm = CreateHeldDevice(firearmToCreate, self.ModuleName);
						else
							firearm = CreateHDFirearm(firearmToCreate, self.ModuleName);
						end
						firearm:SetGoldValue(firearm:GetGoldValue(0, 1, 1) * 0.6);
						self:AddInventoryItem(firearm);
					end
				end
				if unit["Throwable"] then
					self:AddInventoryItem(CreateTDExplosive(unit["Throwable"][math.random(#unit["Throwable"])], self.ModuleName));
				end
				if unit["Headgear"] and self.Head then
					headgear = CreateAttachable("Ronin ".. unit["Headgear"][math.random(#unit["Headgear"])], self.ModuleName);
					self.Head:AddAttachable(headgear);
				end
			end
		elseif math.random() < 0.01 and self.Head then
			headgear = CreateAttachable("Ronin Crab Helmet", "Ronin.rte");
			self.Head:AddAttachable(headgear);
		elseif self.PresetName == "Raider" and self.Head then
			headgear = CreateAttachable("Ronin ".. RoninLoadouts["Machinegunner"]["Headgear"][math.random(#RoninLoadouts["Machinegunner"]["Headgear"])], self.ModuleName);
			self.Head:AddAttachable(headgear);
		end

		if self.Head and (self.face == 1 or self.face == 4) then	--Female
			self.DeathSound.Pitch = 1.2;
			self.PainSound.Pitch = 1.2;
			if not headgear then
				self.Head:AddAttachable(CreateAttachable("Ronin " .. (self.face == 1 and "Black" or "Blonde") .. " Hair", self.ModuleName));
			end
		end
		
		self:SetNumberValue("Equipped", 1);
	end
end

function ThreadedUpdate(self)
	self.controller = self:GetController();
	local damaged = self.Health < self.PrevHealth - 1;
	if self.updateTimer:IsPastSimMS(1000) or damaged then
		self.updateTimer:Reset();
		self.aggressive = self.Health < (self.MaxHealth * 0.5);
		if self.Head and self.face then
			if self.aggressive or damaged or (self.controller and self.controller:IsState(Controller.WEAPON_FIRE)) then
				self.Head.Frame = self.face + (self.Head.FrameCount * 0.5);
			else
				self.Head.Frame = self.face;
			end
		end
	end
end