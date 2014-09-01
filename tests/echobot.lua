--
-- A simple echo bot for LuaTox
--
-- Usage: lua echobot.lua <datadir>
--
-- where <datadir> is a Tox data configuration directory with a save
-- file named "data" (as per Single Tox Standard Draft v0.1.0).
-- It can be created with 'toxic' or other client.
--

local tox = require("tox")
local posix = require("posix")


local function load_boostrap_data(fname)
    local fp = io.open(fname, "rb")
    if not fp then
        return nil
    end
    local data = fp:read("*a")
    fp:close()
    return data
end


local function run(datadir)
    if not datadir then
        print("Usage: " .. arg[0] .. " <datadir>")
        return
    end

    local bootstrap_file = datadir .. "/data"
    local bootstrap_data = load_boostrap_data(bootstrap_file)
    if not bootstrap_data then
        print("No bootstrap data found in " .. bootstrap_file)
        return
    end

    local t = tox.new()
    t:load(bootstrap_data)

    -- Accepts friend requests automatically.
    t:callbackFriendRequest(function(pub, msg, userdata)
        print("Friend request received: " .. (msg or "No message"))
        local req, err = t:addFriendNorequest(pub)
        if not req then
            print("Error: " .. (err or "Unknown"))
        end
    end)

    t:callbackFriendMessage(function(friendnumber, msg, userdata)
        print("Message from friend #" .. friendnumber)
        print(msg)
        t:sendMessage(friendnumber, "Echo: " .. (msg or "nil"))
    end)

    print("Waiting for connection...")
    local echoed = false
    while true do
        if t:isConnected() then
            if not echoed then
                print("Connected")
                echoed = true
            end
        elseif echoed then
            print("Disconnected")
            echoed = false
        end
        t:toxDo()
        posix.sleep(1)
    end

end

run(arg[1])
