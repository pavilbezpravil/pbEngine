local M = {}

print("hello from lua")


frame = 0

function M.onUpdate(self, dt)
    frame = frame + 1
    print("OnUpdate from lua")
    print("frame = " .. frame)
    print("dt = " .. dt)
    print("entity UUID = " .. type(self:getUUID()))
    print("entity UUID = " .. tostring(self:getUUID()))
    print("entity UUID = " .. tostring(self))

    v1 = Vec3.new(1, 0, 0);
    v2 = Vec3.new(1.3, 0.228, 0);

    print("v1 = " .. tostring(v1))
    print("v2 = " .. tostring(v2))
    print("v2:normalize() = " .. tostring(v2:normalize()))
    print("v2:length() = " .. v2:length())
    print("v1:dot(v2) = " .. v1:dot(v2))
    print("v1:cross(v2) = " .. tostring(v1:cross(v2)))
    print("v1 + v2 = " .. tostring(v1 + v2))
    print("v1 - v2 = " .. tostring(v1 - v2))
    print("int = " .. 1)
    print("float = " .. 1.0)
end

package.loaded["assets/scripts/some_module"] = nil
local mymodule = require "assets/scripts/some_module"

package.loaded["assets/scripts/some_module2"] = nil
local mymodule2 = require "assets/scripts/some_module2"

function M.onTest()
    print("onTest567")
end

function onTestModule()
    print("onTestModule")
    print(mymodule)
    for k, v in pairs(mymodule) do
        print(k .. " -> " .. tostring(v))
    end
    print("module  v = " .. mymodule.foo())
    -- print("module  v = " .. foo())
    print("module2 v = " .. mymodule2.foo())
    -- local v = mymodule.foo()
    -- print("v = " .. v)
    -- print(type(package.loaded))
    -- for k, v in pairs(package.loaded) do
    --     print(k .. " -> " .. tostring(v))
    -- end
end

return M