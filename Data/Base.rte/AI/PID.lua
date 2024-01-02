---------------------------------------------------
-- PID controller class
-- http://en.wikipedia.org/wiki/PID_controller
---------------------------------------------------

-- Create a new regulator by specifying the parameters in a table like this:
-- self.myPID = RegulatorPID:New{p=0.1, i=0.1, d=0.1, last_input=0, filter_leak=0.5, integral_max=100}
-- self.myPD = RegulatorPID:New{p=0.2, d=0.5, filter_leak=0.7, integral_max=50}
-- All parameters are optional

RegulatorPID = {};
RegulatorPID.mt = {__index = RegulatorPID};
function RegulatorPID:New(Inputs)
	return setmetatable(
		{
			p = Inputs.p or 0,
			i = Inputs.i or 0,
			d = Inputs.d or 0,
			last_input = Inputs.last_input or 0,
			filter_leak = Inputs.filter_leak or 1,
			filtered_input = Inputs.last_input or 0,
			integral = 0,
			integral_max = Inputs.integral_max or math.huge / 2,
			tick_count = Inputs.tick_count or SettingsMan.AIUpdateInterval
		},
		RegulatorPID.mt);
end

function RegulatorPID:Update(raw_input, target)
	local err;
	local change;
	for i = 1, self.tick_count do 
		self.filtered_input = self.filtered_input * (1-self.filter_leak) + raw_input * self.filter_leak;
		err = self.filtered_input - target;
		change = self.filtered_input - self.last_input;
		self.last_input = self.filtered_input;
		self.integral = math.min(math.max(self.integral + err, -self.integral_max), self.integral_max);
	end
	return self.p*err + self.i*self.integral + self.d*change;
end