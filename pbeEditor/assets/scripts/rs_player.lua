function onCreate(self)
    self.speed = 5.0
end

function onUpdate(self, dt)
    local trans = self:getComponent("TransformComponent")

    local camera = Scene.findEntityByTag("Camera")
    local cameraTrans = camera:getComponent("TransformComponent")

    local direction = Vec3.new()
    if Input.isKeyPressed(KeyCode.W) then
        direction = direction + cameraTrans.forward
    end
    if Input.isKeyPressed(KeyCode.S) then
        direction = direction - cameraTrans.forward
    end
    if Input.isKeyPressed(KeyCode.A) then
        direction = direction - cameraTrans.right
    end
    if Input.isKeyPressed(KeyCode.D) then
        direction = direction + cameraTrans.right
    end
    direction:normalize()

    local rb = self:getComponent("RigidbodyComponent")
    -- rb.velocity = direction * self.speed
    rb:addForce(direction * self.speed)
end

function onTriggerEnter(self, other)
    print("collect coin")
    other:destroy()
end
