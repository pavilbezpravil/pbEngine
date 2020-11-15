local M = {}

function M.onUpdate(self, dt)
    print("OnUpdate SpotLightBlink")
    spotLight = self:getComponent("SpotLightComponent")
    print(tostring(spotLight))
    print(spotLight.Enable)
    print(spotLight.Color)
    print(spotLight.Multiplier)
    print(spotLight.CutOff)

    spotLight.Multiplier = 100
 
    time = game.GetTotalTime()
    spotLight.Multiplier = (1 + math.sin(time * 3)) * 25

    trans = self:getComponent("TransformComponent")
    trans:Move(Vec3.new(dt * 0.1, 0, 0))
end

return M