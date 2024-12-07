--------------------------------------- Instructions ---------------------------------------

------- Require this in your script like so: 

-- self.GameIntensityCalculator = require("Activities/Utility/GameIntensityCalculator");
-- self.GameIntensityCalculator:Initialize(Activity, bool newGame, bool verboseLogging, number intensityValuePerHPBarLost, number intensityDegradationRatePerSecond);

-- Call self.GameIntensityCalculator:UpdateGameIntensityCalculator() every frame.

-- The main update function will return an intensity number between 0 and 1.
-- This corresponds to how much damage has been dealt directly within all the player's views.
-- Use the two number arguments to control how the intensity value rises and falls.
-- One "HP Bar" is considered a standard 100 HP.

------- Saving/Loading

-- Saving and loading requires you to also have the SaveLoadHandler ready.
-- Simply run OnSave(instancedSaveLoadHandler) and OnLoad(instancedSaveLoadHandler) when appropriate.

--------------------------------------- Misc. Information ---------------------------------------

--




local GameIntensityCalculator = {};

function GameIntensityCalculator:Create()
	local Members = {};

	setmetatable(Members, self);
	self.__index = self;

	return Members;
end

function GameIntensityCalculator:Initialize(activity, newGame, intensityValuePerHPBarLost, intensityDegradationRatePerSecond, verboseLogging)
	self.verboseLogging = verboseLogging
	
	self.Activity = activity;
	
	if not intensityValuePerHPBarLost then
		intensityValuePerHPBarLost = 0.2;
	end
	if not intensityDegradationRatePerSecond then
		intensityDegradationRatePerSecond = 0.01;
	end
	
	-- One "HP Bar" is considered 100 HP
	self.intensityValuePerHPBarLost = intensityValuePerHPBarLost;
	
	self.intensityDegradationRatePerSecond = intensityDegradationRatePerSecond;
	
	if newGame then
		self.saveTable = {};
		self.saveTable.CurrentIntensity = -0.5;
	end
	
	print("INFO: GameIntensityCalculator initialized!")
end

function GameIntensityCalculator:OnLoad(saveLoadHandler)
	print("INFO: GameIntensityCalculator loading...");
	self.saveTable = saveLoadHandler:ReadSavedStringAsTable("GameIntensityCalculatorMainTable");
	print("INFO: GameIntensityCalculator loaded!");
end

function GameIntensityCalculator:OnSave(saveLoadHandler)
	print("INFO: GameIntensityCalculator saving...");
	saveLoadHandler:SaveTableAsString("GameIntensityCalculatorMainTable", self.saveTable);	
	print("INFO: GameIntensityCalculator saved!");
end

function GameIntensityCalculator:GetCurrentIntensity()
	return math.min(1, self.saveTable.CurrentIntensity, math.max(0, self.saveTable.CurrentIntensity));
end

function GameIntensityCalculator:UpdateGameIntensityCalculator()
	self.saveTable.CurrentIntensity = math.max(-0.5, self.saveTable.CurrentIntensity - (self.intensityDegradationRatePerSecond * TimerMan.DeltaTimeSecs));

	local healthLostThisFrame = 0;

	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self.Activity:PlayerActive(player) and self.Activity:PlayerHuman(player) then
			local screenBox = Box(CameraMan:GetOffset(player), CameraMan:GetOffset(player) + Vector(FrameMan.PlayerScreenWidth, FrameMan.PlayerScreenHeight));
			for mo in MovableMan:GetMOsInBox(screenBox, -1, true) do
				if IsActor(mo) then
					mo = ToActor(mo);
					healthLostThisFrame = math.max(0, healthLostThisFrame + mo.PrevHealth - mo.Health);
				end
			end
		end
	end
	
	if healthLostThisFrame > 0 then
		--print("health lost this frame: " .. healthLostThisFrame);
	end
	
	self.saveTable.CurrentIntensity = math.min(1.25, self.saveTable.CurrentIntensity + self.intensityValuePerHPBarLost * (healthLostThisFrame/100))
	
	--print("intensity: " .. self.saveTable.CurrentIntensity);
	
	return math.min(1, self.saveTable.CurrentIntensity, math.max(0, self.saveTable.CurrentIntensity));
end


return GameIntensityCalculator:Create();