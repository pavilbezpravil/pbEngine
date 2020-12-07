print "Load pbe_sys.lua"

utils = {}
function utils.PrintTable(t)
    print("Print table")

    for k, v in pairs(t) do
        print(k, v)
    end
end

function utils.tablelength(T)
  local count = 0
  for _ in pairs(T) do count = count + 1 end
  return count
end

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

        dofileIntoEnv(modulePath, newenv)
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

pbe_entity = {}
pbe_entity.map = {}
function pbe_entity.addEntity(entityHangler, entity)
  print("entityHandler " .. tostring(entityHangler))
  pbe_entity.map[entityHangler] = entity

  pbe_entity.printEntityMap()
  pbe_entity.printEntityInfo()
end

function pbe_entity.printEntityMap()
  print("print entity map")
  for k, v in pairs(pbe_entity.map) do
    -- print(k, " ", tostring(v))
    print(k, " wtf? ", v)
  end
end

function pbe_entity.printEntityInfo()
  print("printEntityInfo")
  print("map length = " .. tablelength(pbe_entity.map))
end

function pbe_entity.getEntity(entityHangler)
  pbe_entity.printEntityMap()
  
  print("entityHandler " .. tostring(entityHangler))
  entity = pbe_entity.map[entityHangler]
  return entity
end

function pbe_entity.removeEntity(entityHangler)
  pbe_entity.map[entityHangler] = nil
end

function test_e_func()
  return test_e
end
