function onCreate(self)
    self.time = 0.0
end

function onUpdate(self, dt)
    local trans = self:getComponent("TransformComponent")
    local newPosition = trans.position
    newPosition.y = math.sin(self.time * 1.5) * 0.5 + 1.0
    trans.position = newPosition

    self.time = self.time + dt
end
