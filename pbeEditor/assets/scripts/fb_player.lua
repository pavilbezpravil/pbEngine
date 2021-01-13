function onCreate(self)
    self.upVelocity = 7.0
end

function onUpdate(self, dt)
    local trans = self:getComponent("TransformComponent")
    if trans.position.y < -10.0 then
        print("game over")
    end

    local rb = self:getComponent("RigidbodyComponent")
    if Input.isKeyPressed(KeyCode.Space) then
        rb.velocity = Vec3.Y * self.upVelocity
    end
end

function onTriggerEnter(self, other)
    print("onTriggerEnter")
    other:destroy()
end
