--[[
How To Use the Activity Speedrun Helper:
1. In your Activity's Create function, call ActivitySpeedrunHelper.Setup. The first argument should be self, the second should be the function you want to run when then player enters speedrun mode (see the SignalHunt Activity for an example).
	This will return speedrun data, which you should keep, since you'll need to use it elsewhere.
2. In your Activity's Update function, call ActivitySpeedrunHelper.CheckForSpeedrun, until the speedrun has begun. The only argument is the speedrun data which was returned by Setup.
	This will return true if the speedrun has begun or nothing went wrong, or false if something went wrong. If it returns false, you should delete your speedrun data, since it will no longer work.
3. When you want to mark a speedrun as completed, call ActivitySpeedrunHelper.CompleteSpeedrun. The only argument is the speedrun data which was returned by Setup.

To check whether a run is active, call ActivitySpeedrunHelper.SpeedrunActive. The only argument is the speedrun data which was returned by Setup.
To check whether a run is completed, call ActivitySpeedrunHelper.SpeedrunCompleted. The only argument is the speedrun data which was returned by Setup.
To get duration of a run (active or completed), formatted in seconds and milliseconds, call ActivitySpeedrunHelper.GetSpeedrunDuration. The only argument is the speedrun data which was returned by Setup.
--]]

require("Scripts/Shared/SecretCodeEntry");

ActivitySpeedrunHelper = {};

function ActivitySpeedrunHelper.Setup(activityLuaObject, activitySpeedrunInitializationCallbackFunction)
	if activityLuaObject.HumanCount == 1 then
		local speedrunData = {};
		
		speedrunData.activitySpeedrunInitializationCallbackFunction = activitySpeedrunInitializationCallbackFunction;
		speedrunData.activityLuaObject = activityLuaObject;
		speedrunData.secretCodeEntryDataIndex = SecretCodeEntry.Setup(ActivitySpeedrunHelper.InitializeSpeedrun, speedrunData, 1);
		
		speedrunData.speedrunActive = false;
		speedrunData.speedrunTimer = Timer();
		speedrunData.speedrunCompletionTime = -1;
		
		return speedrunData;
	end
end

function ActivitySpeedrunHelper:CheckForSpeedrun()
	if self.speedrunActive then
		return true;
	end
	
	if not SecretCodeEntry.IsValid(self.secretCodeEntryDataIndex) then
		return false;
	end
	
	SecretCodeEntry.Update(self.secretCodeEntryDataIndex);
		
	return true;
end

function ActivitySpeedrunHelper:InitializeSpeedrun()
	self.speedrunActive = true;
	if self.activitySpeedrunInitializationCallbackFunction ~= nil then
		self.activitySpeedrunInitializationCallbackFunction(self.activityLuaObject);
	end
end

function ActivitySpeedrunHelper:CompleteSpeedrun()
	if self.speedrunCompletionTime == -1 then
		self.speedrunCompletionTime = self.speedrunTimer.ElapsedRealTimeMS;
	end
end

function ActivitySpeedrunHelper:SpeedrunActive()
	return self.speedrunActive;
end

function ActivitySpeedrunHelper:SpeedrunCompleted()
	return self.speedrunCompletionTime > -1;
end

function ActivitySpeedrunHelper:GetSpeedrunDuration()
	if self.speedrunCompletionTime == -1 then
		return "Speedrun Duration: " .. RoundFloatToPrecision(self.speedrunTimer.ElapsedRealTimeMS / 1000, 3, 1) .. " seconds";
	else
		return "Speedrun Completed In: " .. RoundFloatToPrecision(self.speedrunCompletionTime / 1000, 3, 1) .. " seconds";
	end
end