local M = {}

function M.onUpdate(self, dt)
    time = game.GetTotalTime()

    trans = self:getComponent("TransformComponent")
    -- trans:Move(Vec3.new(dt * -0.1, 0, 0))

    radius = 2
    trans.Translation = Vec3.new(math.cos(time), 0, math.sin(time)) * radius
    -- trans.Translation = Vec3.new(0, 0, 0)
end

return M