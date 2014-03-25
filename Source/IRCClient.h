/*
 * Copyright (C) 2011 Fredi Machado <https://github.com/Fredi>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _IRCCLIENT_H
#define _IRCCLIENT_H

#include <string>
#include <vector>
#include <list>
#include "IRCSocket.h"
#include "StringOperations.h"

class IRCClient;

struct IRCCommandPrefix
{
    void Parse(const std::string& data)
    {
        if (data == "")
            return;

        prefix = data.substr(1, data.find(" ") - 1);
        std::vector<std::string> tokens;

        if (prefix.find("@") != std::string::npos)
        {
            tokens = split(prefix, '@');
            nick = tokens.at(0);
            host = tokens.at(1);
        }
        if (nick != "" && nick.find("!") != std::string::npos)
        {
            tokens = split(nick, '!');
            nick = tokens.at(0);
            user = tokens.at(1);
        }
    };

    std::string prefix;
    std::string nick;
    std::string user;
    std::string host;
};

struct IRCMessage
{
    IRCMessage();
    IRCMessage(const std::string& cmd, IRCCommandPrefix p, const std::vector<std::string>& params) :
        command(cmd), prefix(p), parameters(params) {};

    std::string command;
    IRCCommandPrefix prefix;
    std::vector<std::string> parameters;
};

struct IRCCommandHook
{
    IRCCommandHook() : function(NULL) {};

    std::string command;
    void (*function)(const IRCMessage& /*message*/, IRCClient& /*client*/);
};

class IRCClient
{
public:
    IRCClient() : _debug(false) {};

    bool InitSocket();
    bool Connect(const std::string& /*host*/, int /*port*/);
    void Disconnect();
    bool Connected() { return _socket.Connected(); };

    bool SendIRC(const std::string& /*data*/);

    bool Login(const std::string& /*nick*/, const std::string& /*user*/);

    void ReceiveData();

    void HookIRCCommand(const std::string& /*command*/, void (*function)(const IRCMessage& /*message*/, IRCClient& /*client*/));

    void Parse(const std::string& /*data*/);

    void HandleCTCP(const IRCMessage& /*message*/);

    // Default internal handlers
    void HandlePrivMsg(const IRCMessage& /*message*/);
    void HandleNotice(const IRCMessage& /*message*/);
    void HandleChannelJoinPart(const IRCMessage& /*message*/);
    void HandleUserNickChange(const IRCMessage& /*message*/);
    void HandleUserQuit(const IRCMessage& /*message*/);
    void HandleChannelNamesList(const IRCMessage& /*message*/);
    void HandleNicknameInUse(const IRCMessage& /*message*/);
    void HandleServerMessage(const IRCMessage& /*message*/);

    void Debug(bool debug) { _debug = debug; };

private:
    void HandleCommand(const IRCMessage& /*message*/);
    void CallHook(const std::string& /*command*/, const IRCMessage& /*message*/);

    IRCSocket _socket;

    std::list<IRCCommandHook> _hooks;

    std::string _nick;
    std::string _user;

    bool _debug;
};

#endif
