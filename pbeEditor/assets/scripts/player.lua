function onCreate(self)
    print("onCreate from lua")
end

function onUpdate(self, dt)
    print("sdfgsdgdfgdfgdfg")
    trans = self:getComponent("TransformComponent")
    print("sdfgsdgdfgdfgdfg")

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

    mouseDelta = Input.getMouseDelta()
    -- print(tostring(mouseDelta))

    mouseSensitivity = 0.002
    q = Quat.angleAxis(mouseDelta.x * mouseSensitivity, Vec3.YNeg)
    trans.rotation = q * trans.rotation

    local speed = 2
    trans.translation = trans.translation + direction * speed * dt
end