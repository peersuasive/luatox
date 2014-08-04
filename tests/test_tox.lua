--[[
 
  Lua binding for Tox.
 
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


local Tox = require"tox"

-- init
local tox  = Tox()
local tox2 = Tox()
local tox3 = Tox()

local function accept_friend_request(pub, data)
    if (#data == 6) and ("Gentoo"==data) then
        local req, err = tox2:addFriendNorequest(pub)
        if not(req)then
            print("FAILED: accept friend request", err)
            return
        end
    else
        print(
        string.format(
        "FAILED: CallbackFriendRequest: failed to receive data: expected 6 | Gentoo, received: %d | %s", 
        #data, data)
        )
    end
    print("PASSED: accept friend")
end

local function test_add_friends()
    print"******** ADD FRIENDS *********"

    tox2:callbackFriendRequest(accept_friend_request)

    local address = tox2:getAddress()

    -- TODO ajouter les codes d'erreur dans la classe
    local friend, err = tox3:addFriend(address, "Gentoo")
    assert(friend , "FAILED: add friend: "..(err or "[no msg]") )

    local off = 1
    while true do
        tox:toxDo()
        tox2:toxDo()
        tox3:toxDo()

        if (tox:isConnected() and tox2:isConnected() and tox3:isConnected() and off) then
            off = nil
            print("all connected")
        end

        if (tox2:isOnline(0)) and (tox3:isOnline(0)) then
            break
        end

        os.execute("sleep 1")
    end
    print("PASSED: add friend")
end

local function test_send_message()
    print"******** SEND MESSAGE ********"

    local messages_received = nil
    local function print_message(friendnumber, string)
        if("G"==string)then
            messages_received = 1
        else
            print(string.format("FAILED: print message: data not matching: expexted: G, got: %s\n", srtring))
        end
    end

    tox3:callbackFriendMessage(print_message)
    local size = tox2:sendMessage(0, "G")
    assert(size, "FAILED: Friend doesn't exist!" )

    while true do
        tox:toxDo()
        tox2:toxDo()
        tox3:toxDo()

        if(messages_received)then
            break
        end
        os.execute("sleep 1")
    end
    print("PASSED: send/receive message")
end

local function test_name_change()
    print"******** CHANGE NAME *********"
    local name_changes = false
    local function print_nickchange(friendnumber, string)
        if("Gentoo"==string)then
            name_changes = true
        else
            print(string.format("FAILED: change name: data not matching: expexted: Gentoo, got: %s", string))
        end
    end

    tox3:callbackNameChange(print_nickchange)
    assert( tox2:setName("Gentoo"), "FAILED: can't to set name")

    while true do
        tox:toxDo()
        tox2:toxDo()
        tox3:toxDo()

        if(name_changes)then
            break
        end
        os.execute("sleep 1")
    end

    local temp_name = tox3:getName(0)
    assert( "Gentoo"==temp_name,
        string.format("FAILED: name is not correctly received: expected: Gentoo, got: %s", temp_name))
    print("PASSED: change name")
end

local function test_is_typing()
    print"******** IS TYPING ***********"
    local typing_changes = 0
    local function print_typingchange(friendnumber, is_typing)
        if(is_typing)then
            typing_changes = 2
        else
            typing_changes = 1
        end
    end
    tox2:callbackTypingChange(print_typingchange);

    assert(tox3:setUserIsTyping(0, true), "FAILED: is typing: can't change status")
    while true do
        tox:toxDo()
        tox2:toxDo()
        tox3:toxDo()

        if(typing_changes==2)then
            break
        end
        os.execute("sleep 1")
    end
    assert( tox2:getIsTyping(0), "FAILED: couldn't get is typing status from friend when true")
    print("PASSED: got friend is typing status")

    assert(tox3:setUserIsTyping(0, false), "FAILED: is typing: can't change status")
    while true do
        tox:toxDo()
        tox2:toxDo()
        tox3:toxDo()

        if(typing_changes==1)then
            break
        end
        os.execute("sleep 1")
    end
    assert( not(tox2:getIsTyping(0)), "FAILED: couldn't get is typing status from friend when false")
    print("PASSED: got friend is not typing status")
end

local size_recv, num = 0, 0
local function write_file(friendnumber, filenumber, data)
    local f_data = string.rep(string.char(num), #data)

    if(#f_data==#data)and(f_data==data)then
        size_recv = size_recv + #data
    else
        print(string.format("FILE_CORRUPTED: got %d, expected: %d, %s", #data, #f_data, tostring(data==f_data)))
        error("FILE CORRUPTED")
    end
    num = num+1
    if(num>255)then num = 0 end
end

local file_sent, sendf_ok = 0, 0
local function file_print_control(friendnumber, send_receive, filenumber, control_type, data)
    if (send_receive == 0) and (control_type == Tox.control.FINISHED) then
        file_sent = 1
    end

    if (send_receive == 1) and (control_type == Tox.control.ACCEPT) then
        sendf_ok = 1
    end
end

local filenum, file_accepted, file_size = 0, 0, 0
local function file_request_accept(friendnumber, filenumber, filesize, filename)
    local pat = "Gentoo.exe"
    if (#pat == #filename) and (pat==filename) then
        file_accepted = file_accepted + 1
    else
        print("ERROR: file rejected")
        error("ERROR")
    end

    file_size = filesize
    local r = tox3:fileSendControl(friendnumber, 1, filenumber, Tox.control.ACCEPT, nil)
    if not(r) then
        print("ERROR: couldn't send file control from callback!")
        error("ERROR")
    end
end

local function test_send_file()
    filenum = 0
    file_accepted = 0
    file_size = 0
    file_sent = 0
    sendf_ok = 0
    size_recv = 0

    tox3:callbackFileData(write_file)
    tox2:callbackFileControl(file_print_control)
    tox3:callbackFileControl(file_print_control)
    tox3:callbackFileSendRequest(file_request_accept)

    local totalf_size = 100 * 1024 * 1024
    local fnum = tox2:newFileSender(0, totalf_size, "Gentoo.exe")
    assert(fnum, "tox_new_file_sender fail")

    local fpiece_size = tox2:fileDataSize(0)

    local num = 0
    local f_data = string.rep(string.char(num), fpiece_size)

    print("sending 100Mb file...")
    local start = os.time()
    while (1) do
        file_sent = 0
        tox:toxDo()
        tox2:toxDo()
        tox3:toxDo()

        if (sendf_ok>0) then
            while(
                tox2:fileSendData(0, fnum, f_data, (fpiece_size<totalf_size) and fpiece_size or totalf_size)
            ) do
                if (totalf_size <= fpiece_size) then
                    sendf_ok = 0
                    local r = tox2:fileSendControl(0, 0, fnum, Tox.control.FINISHED, nil)
                    if not(r) then
                        print("ERROR: couldn't send FINISH signal!")
                        error("ERROR")
                    end
                end
                num = num+1
                if(num>255)then num = 0 end
                f_data = string.rep(string.char(num), fpiece_size)
                totalf_size = totalf_size - fpiece_size
                if(totalf_size<0)then totalf_size =0 end
            end
        end

        if (file_sent>0) and (size_recv == file_size) then
            break
        end

        local tox1_interval = tox:toxDoInterval()
        local tox2_interval = tox2:toxDoInterval()
        local tox3_interval = tox3:toxDoInterval()

        if (tox2_interval > tox3_interval) then
            os.execute("sleep "..(tox3_interval/1000))
        else
            os.execute("sleep "..(tox2_interval/1000))
        end
    end
    print(string.format("100MB file sent in %d sec", os.time() - start))
end


local function test_many_clients()
    local NUM_TOXES   = 66
    local NUM_FRIENDS = 20
    local NUM_TOXES   = 2
    local NUM_FRIENDS = 2

    local toxes = {}
    local i, j

    for i=1, NUM_TOXES do
        local tox = Tox()
        assert(tox, string.format("failed to create tox instance %d", i))
        tox:callbackFriendRequest(accept_friend_request)
        toxes[#toxes+1] = tox
    end

    local tpairs = {}

    local function computeToxes(i)
        local id1, id2
        while true do
            math.randomseed(os.time())
            math.random(); math.random(); math.random()
            id1 = math.random(1, NUM_TOXES)
            id2 = math.max(1, ( (id1 + math.random(1, NUM_TOXES - 1) + 1) % NUM_TOXES ))
            local ok = true
            for j=1, i-1 do
                if (id1==id2) or ((tpairs[j].tox2 == id1) and (tpairs[j].tox1 == id2)) then
                    ok = false
                    break
                end
            end
            if(id1==id2)then ok=false end
            print(id1, id2)
            if(ok)then break end
        end
        return id1, id2
    end

    print("adding friends...")
    for i=1,NUM_FRIENDS do
        local tox_id1, tox_id2 = computeToxes(i)
        print("i:", i, tox_id1, tox_id2)
        local test, err = nil
        while(true)do
            local tox1, tox2 = toxes[tox_id1], toxes[tox_id2]
            local address = tox1:getAddress()
            test, err = tox2:addFriend(address, "Gentoo")
            if not(test) and (err ~= Tox.errors.ALREADYSENT) then
                print("OK", tox_id1, tox_id2)
                break
            else
                tox_id1, tox_id2 = computeToxes(i)
            end
        end
        tpairs[i] = {
            tox1 = tox_id1,
            tox2 = tox_id2
        }
        assert(test, string.format("Failed to add friend error code: %i", err))
    end

    print("waiting for status...")
    while (1) do
        local counter = 0
        for i = 1, NUM_TOXES do
            local tox = toxes[i]
            for j=1, tox:countFriendlist() do
                if (tox:getFriendConnectionStatus(j) == 1) then
                    counter = counter+1
                end
            end
        end

        if (counter == (NUM_FRIENDS*2)) then
            break
        end

        for i = 1, NUM_TOXES do
            local tox = toxes[i]
            tox:toxDo()
        end

        io:write(".")
        os.execute("sleep 1")
    end

    print("test_many_clients succeeded")
end


test_add_friends()
test_send_message()
test_name_change()
test_is_typing()
test_send_file()

-- test_many_clients()
print("END")
