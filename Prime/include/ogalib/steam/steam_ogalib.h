/*
ogalib

MIT License

Copyright (c) 2024 Sean Reid (email@seanreid.ca)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#if defined(OGALIB_USING_STEAM)

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <steam/steam_api.h>

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#define OGALIB_STEAM_AUTH_SESSION_TICKET_SIZE 1024

////////////////////////////////////////////////////////////////////////////////
// Enums
////////////////////////////////////////////////////////////////////////////////

namespace ogalib {

typedef enum {
  DataSteamAuthSessionTicketStateNone = 0,
  DataSteamAuthSessionTicketStateWaiting,
  DataSteamAuthSessionTicketStateReady,
  DataSteamAuthSessionTicketStateError,
} DataSteamAuthSessionTicketState;

};

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace ogalib {

class DataSteam {
public:

  bool initializedSteam;
  bool steamAPIInitEnabled;

  u64 appId;

  u64 authSessionTicketAccountId;
  u8 authSessionTicket[OGALIB_STEAM_AUTH_SESSION_TICKET_SIZE];
  u32 authSessionTicketSize;
  std::function<void(const json&)> authSessionTicketCallback;
  HAuthTicket authSessionTicketHandle;
  DataSteamAuthSessionTicketState authSessionTicketState;

  STEAM_CALLBACK(DataSteam, OnGetAuthSessionTicketResponse, GetAuthSessionTicketResponse_t, cbGetAuthSessionTicketResponse);

public:

  DataSteam();

};

};

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

namespace ogalib {

void InitSteam();
void FinalizeSteam();
void ProcessSteam();
void LoginUsingSteam(std::function<void(const json&)> callback);

};

#endif
