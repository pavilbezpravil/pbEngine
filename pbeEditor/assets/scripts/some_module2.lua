local M = {}
if setfenv then
	setfenv(1, M) -- for 5.1
else
	_ENV = M -- for 5.2
end

function foo()
    return 13
end

return M