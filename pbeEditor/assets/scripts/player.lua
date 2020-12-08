function onCreate(self)
    self.mouseSensitivity = 0.002
end

function onUpdate(self, dt)
    local trans = self:getComponent("TransformComponent")

    local direction = Vec3.new()
    if Input.isKeyPressed(KeyCode.W) then
        direction = direction + trans.forward
    end
    if Input.isKeyPressed(KeyCode.S) then
        direction = direction - trans.forward
    end
    if Input.isKeyPressed(KeyCode.A) then
        direction = direction - trans.right
    end
    if Input.isKeyPressed(KeyCode.D) then
        direction = direction + trans.right
    end
    direction:normalize()

    local mouseDelta = Input.getMouseDelta()
    -- print(tostring(mouseDelta))

    local q = Quat.angleAxis(mouseDelta.x * self.mouseSensitivity, Vec3.YNeg)
    trans.rotation = q * trans.rotation

    local spherePos = trans.translation + trans.forward * 4
    RendPrim.drawSphere(spherePos, 0.5, 16, Color.Red)
    RendPrim.drawLine(spherePos - trans.right * 1.2, spherePos + trans.right * 1.2, Color.Green)

    local speed = 2
    trans.translation = trans.translation + direction * speed * dt
end