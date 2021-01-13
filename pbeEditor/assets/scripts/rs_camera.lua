function onCreate(self)
    self.distance = 20.0
end

function onUpdate(self, dt)
    local trans = self:getComponent("TransformComponent")

    local player = Scene.findEntityByTag("Player")
    if player:isValid() then
        local playerTrans = player:getComponent("TransformComponent")
        trans.position = playerTrans.position - trans.forward * self.distance
    end
end
