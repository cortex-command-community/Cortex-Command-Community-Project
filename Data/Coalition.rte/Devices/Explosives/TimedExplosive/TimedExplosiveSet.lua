function Create(self)
	self.lifeTimer = Timer();
	self.blinkTimer = Timer();
	self.blipTimer = Timer();

	self.actionPhase = 0;
	self.changeCounter = 0;
	self.stuck = false;
	self.blipdelay = 1000;

	self.minBlipDelay = 100;
	self.medBlipDelay = 250;
	self.maxBlipDelay = 500;

	self.detonateDelay = self:NumberValueExists("DetonationDelay") and self:GetNumberValue("DetonationDelay") or 11000;
	self.Frame = 1;

	if TimedExplosiveTable == nil then
		TimedExplosiveTable = {};
	end

	self.tableNum = #TimedExplosiveTable + 1;
	TimedExplosiveTable[self.tableNum] = self;

	self.activateSound = CreateSoundContainer("Explosive Device Activate", "Base.rte");
	self.blipSound = CreateSoundContainer("Timed Explosive Blip", "Coalition.rte");

	RemoteExplosiveStick(self);
end

function Update(self)
	if TimedExplosiveTable == nil then
		TimedExplosiveTable = {};
		TimedExplosiveTable[self.tableNum] = self;
	end

	RemoteExplosiveStick(self);

	if self.stuck then
		if self.lifeTimer:IsPastSimMS(self.detonateDelay) then
			self:GibThis();
		else
			self.ToDelete = false;
			self.ToSettle = false;

			local number = math.ceil((self.detonateDelay - self.lifeTimer.ElapsedSimTimeMS) * 0.01) * 0.1;
			local text = "".. number;
			if number == math.ceil(number) then
				text = text ..".0";
			end
			for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do

				local screen = ActivityMan:GetActivity():ScreenOfPlayer(player);
				if screen ~= -1 and not SceneMan:IsUnseen(self.Pos.X, self.Pos.Y, ActivityMan:GetActivity():GetTeamOfPlayer(player)) then
					PrimitiveMan:DrawTextPrimitive(screen, self.Pos + Vector(-5, -self.Diameter), text, true, 0);
				end
			end
		end

		if self.blipTimer:IsPastSimMS(50) then
			self.Frame = 0;
		else
			self.Frame = 1;
		end

		if self.blipTimer:IsPastSimMS(self.blipdelay) then
			self.blipTimer:Reset();
			self.blinkTimer:Reset();
			self.blipSound:Play(self.Pos);

			if self.changeCounter == 0 and self.lifeTimer.ElapsedSimTimeMS > (self.detonateDelay * 0.85 - 5000) then
				self.changeCounter = 1;
				self.blipdelay = self.maxBlipDelay;
			end

			if self.changeCounter == 1 and self.lifeTimer.ElapsedSimTimeMS > (self.detonateDelay * 0.90 - 3000) then
				self.changeCounter = 2;
				self.blipdelay = self.medBlipDelay;
			end

			if self.changeCounter == 2 and self.lifeTimer.ElapsedSimTimeMS > (self.detonateDelay * 0.95 - 1000) then
				self.changeCounter = 3;
				self.blipdelay = self.minBlipDelay;
			end
		end
	end
	if self.Sharpness == 1 then
		self.ToDelete = true;
	end
end

function Destroy(self)
	TimedExplosiveTable[self.tableNum] = nil;
end