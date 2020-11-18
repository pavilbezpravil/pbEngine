local M = {}

-- todo: move to math_extension.lua
function math.frac(x)
    i, f = math.modf(x)
    return f
end

function M.onUpdate(self, dt)
    time = game.GetTotalTime()

    trans = self:getComponent("TransformComponent")

    -- trans.translation = Vec3.new(0, 0, 0)
    
    -- posiotion can be moved
    -- trans:Move(Vec3.new(dt * -0.1, 0, 0))

    radius = 2.0

    -- angle = math.frac(time)
    angle = time * 4
    
    -- position set
    -- trans.translation = Vec3.new(math.cos(angle), 0, math.sin(angle)) * radius

    -- use quaternion
    -- 2 way
    -- q = Quat.angleAxis(angle, Vec3.Y
    -- trans.translation = q * Vec3.XNeg * radius

    -- q1 = Quat.angleAxis(0, Vec3.Y
    -- q2 = Quat.angleAxis(angle, Vec3.Z

    -- t = math.frac(time * 0.01)
    -- q = Quat.slerp(q1, q2, t)
    -- trans.translation = q * Vec3.new(1, 0, 0) * radius

    -- q1 = Quat.angleAxis(0, Vec3.Y())
    -- q2 = Quat.angleAxis(math.rad(90), Vec3.XNeg())

    -- t = math.sin(time) * 0.5 + 0.5
    -- q = Quat.slerp(q1, q2, t)
    -- trans.translation = q * Vec3.new(0, 1, 0) * radius

    q = Quat.angleAxis(math.rad(90) * dt, Vec3.Y)
    trans.rotation = q * trans.rotation

    local speed = 3
    local velocity = trans.forward * speed
    trans.translation = trans.translation + velocity * dt

    trans.scale = Vec3.new(1, 1, 1) * 0.2 * (math.sin(time * 3.235) + 1.1)

    -- print("forward = " .. tostring(trans.forward))
    -- print("up = " .. tostring(trans.up))
    -- print("right = " .. tostring(trans.right))

    -- t = math.sin(time) * 0.5 + 0.5
    -- q = Quat.slerp(q1, q2, t)
    -- trans.translation = q * Vec3.new(0, 1, 0) * radius
end

return M