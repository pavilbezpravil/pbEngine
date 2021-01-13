function onCreate(self)
    self.speed = 5.0
    self.leftCorner = 13.0
end

function onUpdate(self, dt)
    local trans = self:getComponent("TransformComponent")
    trans.position = trans.position +  trans.forward * self.speed * dt
    if trans.position.x < -self.leftCorner then
        trans.position = Vec3.new(self.leftCorner, Random.float(-2.0, 3.0), trans.position.z) 
    end
end
