--[[
 
  Lua binding for Tox DNS.
 
   Copyright (C) 2014 Peersuasive Technologies All Rights Reserved.
 
   This file is part of LuaTox.
 
   LuaTox is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
 
   LuaTox is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
 
   You should have received a copy of the GNU General Public License
   along with LuaTox.  If not, see <http://www.gnu.org/licenses/>.
 
--]]


local ToxDNS = require"toxdns"

local args = {...}

local user = assert(arg[1], "Missing username")
local host = arg[2] or "utox.org"
local pub_key = arg[3] or "D3154F65D28A5B41A05D4AC7E4B39C6B1C233CC857FB365C56E8392737462A12"
local port = arg[4] or 53
-- "34B0134182B4817D0B96C8C3A8B310A6A6AA296CE569B4F99661016D8F5F113C"

print(string.format("asking for user %s on %s on port %d with key %s", user, host, port, pub_key))

-- init
-- that's indeed the first test
local dns = nil

local function test_init()
    dns = ToxDNS(pub_key)
    if not(dns) then
        error(string.format("FAILED: instanciate new ToxDNS"))
    end
    print("PASSED: instanciate ToxDNS")
end

local function create_packet(id, str)
    local len = #str + 13 + 16
    local packet = {}
    for i=1, len do
        packet[i] = string.char(0)
    end

    math.randomseed( os.time() *10 )
    for i=1,10 do math.random() end
    local id2 = math.random(1,255)

    packet[1]  = string.char(id)
    packet[2]  = string.char(id2)
    packet[6]  = string.char(1)
    packet[12] = string.char(1)
    packet[13] = "."
    for i=1,#str do
        packet[i+13] = str:sub(i,i)
    end

    local c = 0
    for i=#str+13, 11, -1 do
        if(packet[i] == ".") then
            packet[i] = string.char(c)
            c = 0
        else
            c = c+1
        end
    end

    packet[#str+14 + 2]  = string.char(16)
    packet[#str+14 + 4]  = string.char(1)
    packet[#str+14 + 7]  = string.char(0x29)
    packet[#str+14 + 8]  = string.char(16)
    packet[#str+14 + 12] = string.char(0x80)

    return table.concat(packet)
end

local r_id = nil
local function test_generate_string()
    local len = 98 + #host

    local req = dns:generate(host, user)
    assert( ("table"==type(req)), string.format("FAILED: generate: expected table, got %s\n", type(req)) )
    assert( req[1] > 0, string.format("FAILED: expected key > 0, got %d\n", req[1]) )
    assert( (len == #req[2]), string.format("FAILED: generate: expected string of size %d, got %d\n", len, #req[2]) )

    math.randomseed( os.time() )
    for i=1,10 do math.random() end
    local id = math.random(1,255)
    
    local packet, err = create_packet( id, req[2] )
    assert( (packet), 
            string.format("FAILED: generate string: error %d", err or -1) )

    local out = io.open("out.bin", "wb")
    out:write(packet)
    out:close()
 
    r_id = req[1]
    print("PASSED: generate string")
end

local function test_decrypt_string()
    local f = io.popen("cat out.bin | ./utils/getuserid "..host.." "..port.. " 2>/dev/null")
    local r = f:read("*a")
    f:close()

    assert((r and #r>0), "Can't connect to DNS or user not found")

    local resp = r:match("([^=]+)$")
    assert((resp and #resp==87), string.format("unknown response: %q (expected 87, got %d)", resp, #resp))
    
    local result, raw = dns:decrypt(r_id, resp)

    assert( (result and not(#result==0)), "FAILED: couldn't decrypt response" )
    assert( (#result==76), "FAILED: incorrect decrypted message size" )

    assert( (raw and not(raw==0)), "FAILED: missing decrypted raw message" )
    assert( (#raw==38), "FAILED: incorrect decrypted raw message size" )

    local raw_hex = {}
    for i=1, #raw do
        raw_hex[#raw_hex+1] = string.format( "%02X", string.byte( raw:sub(i,i) ) )
    end
    raw_hex = table.concat(raw_hex)

    assert( ( result == raw_hex ), "FAILED: raw and hex string don't match" )
    --print(string.format("peer id: %s", result))

    print("PASSED: decrypt string")
end

test_init()
test_generate_string()
test_decrypt_string()

print("END")
