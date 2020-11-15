local M = {}

function M.onUpdate(self, dt)
    spotLight = self:getComponent("SpotLightComponent")
    
    time = game.GetTotalTime()
    spotLight.Multiplier = (1 + math.sin(time * 3)) * 25
end

return M