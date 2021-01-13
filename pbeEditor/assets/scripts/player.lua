function onCreate(self)
    self.mouseSensitivity = 0.002
    self.destroyTest = true
    self.destroyTimerPeriod = 1.0
    self.destroyTimer = self.destroyTimerPeriod
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

    local speed = 2
    trans.position = trans.position + direction * speed * dt

    local spherePos = trans.position + trans.forward * 4
    RendPrim.drawSphere(spherePos, 0.5, 16, Color.Red)
    RendPrim.drawLine(spherePos - trans.right * 1.2, spherePos + trans.right * 1.2, Color.Green)

    if self.destroyTest then
        self.destroyTimer = self.destroyTimer - dt
        if self.destroyTimer < 0.0 then
            self.destroyTimer = self.destroyTimer + self.destroyTimerPeriod
            local entityForDestroy = Scene.findEntityByTag("Cube")
            entityForDestroy:destroy()
            -- if entityForDestroy:isValid() then
            --     entityForDestroy:destroy()
            -- end
        end
    end
end