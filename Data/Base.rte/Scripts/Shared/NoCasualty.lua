function Destroy(self)
	ActivityMan:GetActivity():ReportDeath(self.Team, -1);
end