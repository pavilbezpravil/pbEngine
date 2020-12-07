utils = {}
function utils.PrintTable(t)
    print("Print table")

    for k, v in pairs(t) do
        print(k, v)
    end
end

print "Load pbe_sys.lua"

pbe_sys = {}

function setfenv(fn, env)
    local i = 1
    while true do
      local name = debug.getupvalue(fn, i)
      if name == "_ENV" then
        debug.upvaluejoin(fn, i, (function()
          return env
        end), 1)
        break
      elseif not name then
        break
      end
  
      i = i + 1
    end
  
    return fn
end

function getfenv(fn)
    local i = 1
    while true do
      local name, val = debug.getupvalue(fn, i)
      if name == "_ENV" then
        return val
      elseif not name then
        break
      end
      i = i + 1
    end
end

local function dofileIntoEnv(filename, env)
    -- setmetatable ( env, { __index = _G } )
    local status, result = assert(pcall(setfenv(assert(loadfile(filename)), env)))
    -- setmetatable(env, nil)
    return result
end


function pbe_sys.RequireModule(modulePath)
    print("Require module " .. modulePath)

    local M = {}
    do
        local globaltbl = _G
        local newenv = setmetatable({}, {
            __index = function (t, k)
                local v = M[k]
                if v == nil then return globaltbl[k] end
                return v
            end,
            __newindex = M,
        })

        dofileIntoEnv("assets/scripts/player.lua", newenv)
    end

    -- dofileIntoEnv("assets/scripts/player.lua", M)

    return M
end

function pbe_sys.loadModule(moduleName, modulePath)
    print("Load module " .. moduleName)

    _G[moduleName] = pbe_sys.RequireModule(modulePath)
end

function pbe_sys.checkModuleExist(moduleName)
    print("Start checking module " .. moduleName)

    module = _G[moduleName]
    if module then 
        print("module exist")
        print("print module fields")
        utils.PrintTable(module)
    else
        print("cant find module")
    end
end


function someFunc()
  print("some func called ")
end
