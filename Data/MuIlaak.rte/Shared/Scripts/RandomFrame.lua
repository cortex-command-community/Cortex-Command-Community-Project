function Create(self)
    if self.Randomized == nil then
        self.Frame = math.random(0, self.FrameCount - 1);
        self.Randomized = true;
    end
end