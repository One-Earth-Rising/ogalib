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

#if defined(OGALIB_USING_STEAM)

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Windows.h>
#include <Shellapi.h>
#include <winhttp.h>
#include <ogalib/ogalib.h>
#include <ogalib/ogalib_steam.h>

using namespace ogalib;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

extern ogalib::Data ogalibData;
ogalib::DataSteam ogalibDataSteam;

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

DataSteam::DataSteam():
initializedSteam(false),
steamAPIInitEnabled(true),
appId(0),
authSessionTicketAccountId(0),
authSessionTicketHandle(k_HAuthTicketInvalid),
authSessionTicketState(DataSteamAuthSessionTicketStateNone),
cbGetAuthSessionTicketResponse(this, &DataSteam::OnGetAuthSessionTicketResponse) {

}

void DataSteam::OnGetAuthSessionTicketResponse(GetAuthSessionTicketResponse_t* param) {
  if(param->m_eResult == k_EResultOK) {
    ogalibDataSteam.authSessionTicketState = DataSteamAuthSessionTicketStateReady;
  }
  else {
    ogalibDataSteam.authSessionTicketState = DataSteamAuthSessionTicketStateError;
  }
}

void ogalib::InitSteam() {
  if(auto it = ogalibData.initParams.find("Steam.APIInitEnabled")) {
    auto& value = it.value();
    if(value.IsBool()) {
      ogalibDataSteam.steamAPIInitEnabled = value.GetBool();
    }
  }

  if(ogalibDataSteam.steamAPIInitEnabled) {
    ogalibDataSteam.initializedSteam = SteamAPI_Init();
  }

  HSteamUser hSteamUser = GetHSteamUser();
  HSteamPipe hSteamPipe = GetHSteamPipe();
  ISteamClient* steamClient = SteamClient();
  if(steamClient) {
    ISteamUtils* steamUtils = steamClient->GetISteamUtils(hSteamPipe, STEAMUTILS_INTERFACE_VERSION);

    ogalibDataSteam.appId = steamUtils->GetAppID();
  }
}

void ogalib::FinalizeSteam() {
  if(ogalibDataSteam.steamAPIInitEnabled) {
    if(ogalibDataSteam.initializedSteam) {
      SteamAPI_Shutdown();
    }
  }
}

void ogalib::ProcessSteam() {
  SteamAPI_RunCallbacks();

  if(ogalibData.loginInProgress) {
    if(ogalibDataSteam.authSessionTicketState == DataSteamAuthSessionTicketStateReady) {
      auto callback = ogalibDataSteam.authSessionTicketCallback;
      ogalibDataSteam.authSessionTicketState = DataSteamAuthSessionTicketStateNone;

      char ticketBuffer[OGALIB_STEAM_AUTH_SESSION_TICKET_SIZE * 2 + 1];
      u32 useTicketSize = min(ogalibDataSteam.authSessionTicketSize, OGALIB_STEAM_AUTH_SESSION_TICKET_SIZE);

      for(u32 i = 0; i < useTicketSize; i++) {
        u32 index = i << 1;
        snprintf(&ticketBuffer[index], 3, "%02X", (int) ogalibDataSteam.authSessionTicket[i]);
      }
      ticketBuffer[useTicketSize << 1] = 0;

      std::string params = string_printf("?network=steam&steamAccountId=%llu&steamAppId=%llu&steamAuthSessionTicket=%s", ogalibDataSteam.authSessionTicketAccountId, ogalibDataSteam.appId, ticketBuffer);
      if(ogalibData.encodeURLRequests)
        params = EncodeURL(params.c_str());

      std::string url = string_printf("%s/Login/v1/%s", ogalibData.baseAPI.c_str(), params.c_str()).c_str();

      json sendURLParams;
      sendURLParams["ignoreSSLErrors"] = true;

      SendURL(url.c_str(), sendURLParams, [=](const json& response) {
        ogalibData.loginInProgress = false;
        auto callback = ogalibDataSteam.authSessionTicketCallback;
        ogalibDataSteam.authSessionTicketCallback = nullptr;

        if(auto it = response.find("error")) {
          if(callback) {
            callback({
              {"error", it.c_str()},
            });
          }
        }
        else if(auto it = response.find("response")) {
          json loginResponse;
          if(loginResponse.parse(it.str())) {
            if(auto itError = loginResponse.find("error")) {
              if(callback) {
                callback({
                  {"error", itError.c_str()},
                });
              }
            }
            else if(auto itResp = loginResponse.find("resp")) {
              auto resp = itResp.str();
              if(resp == "ok") {
                if(auto itId = loginResponse.find("id")) {
                  auto& id = itId.value();
                  if(id.IsNumber()) {
                    ogalibData.userId = itId.value().GetUint64();
                  }
                  else {
                    ogalibData.userId = 0;
                  }
                }
                else {
                  ogalibData.userId = 0;
                }

                if(auto itToken = loginResponse.find("token")) {
                  auto& token = itToken.value();
                  if(token.IsNumber()) {
                    ogalibData.token = token.GetUint64();
                  }
                  else {
                    ogalibData.token = 0;
                  }
                }
                else {
                  ogalibData.token = 0;
                }
              }

              if(ogalibData.userId && ogalibData.token) {
                if(callback) {
                  json result;
                  result["success"] = true;
                  callback(result);
                }
              }
              else {
                if(callback) {
                  callback({
                    {"error", "Invalid user."},
                  });
                }
              }
            }
            else {
              if(callback) {
                callback({
                  {"error", "Unknown response."},
                });
              }
            }
          }
          else {
            if(callback) {
              callback({
                {"error", loginResponse.error()},
              });
            }
          }
        }
        else {
          if(callback) {
            callback({
              {"error", "Could not find response."},
            });
          }
        }
      });
    }
    else if(ogalibDataSteam.authSessionTicketState == DataSteamAuthSessionTicketStateError) {
      ogalibDataSteam.authSessionTicketState = DataSteamAuthSessionTicketStateNone;
      ogalibData.loginInProgress = false;
      auto callback = ogalibDataSteam.authSessionTicketCallback;
      ogalibDataSteam.authSessionTicketCallback = nullptr;
      if(callback) {
        callback({
          {"error", "Failure in obtaining Steam authorization ticket."},
        });
      }
    }
  }
}

void ogalib::LoginUsingSteam(std::function<void(const json&)> callback) {
  memset(ogalibDataSteam.authSessionTicket, 0, sizeof(ogalibDataSteam.authSessionTicket));
  ogalibDataSteam.authSessionTicketSize = sizeof(ogalibDataSteam.authSessionTicket);
  ogalibDataSteam.authSessionTicketCallback = callback;
  ogalibDataSteam.authSessionTicketState = DataSteamAuthSessionTicketStateNone;

  HSteamUser hSteamUser = GetHSteamUser();
  HSteamPipe hSteamPipe = GetHSteamPipe();
  ISteamClient* steamClient = SteamClient();
  if(steamClient) {
    ISteamUser* steamUser = steamClient->GetISteamUser(hSteamUser, hSteamPipe, STEAMUSER_INTERFACE_VERSION);
    ogalibDataSteam.authSessionTicketAccountId = steamUser->GetSteamID().ConvertToUint64();

    ogalibDataSteam.authSessionTicketState = DataSteamAuthSessionTicketStateWaiting;
    ogalibDataSteam.authSessionTicketHandle = steamUser->GetAuthSessionTicket(ogalibDataSteam.authSessionTicket, sizeof(ogalibDataSteam.authSessionTicket), &ogalibDataSteam.authSessionTicketSize);
    if(ogalibDataSteam.authSessionTicketHandle == k_HAuthTicketInvalid) {
      ogalibDataSteam.authSessionTicketState = DataSteamAuthSessionTicketStateNone;
      ogalibData.loginInProgress = false;
      auto callback = ogalibDataSteam.authSessionTicketCallback;
      ogalibDataSteam.authSessionTicketCallback = nullptr;
      if(callback) {
        callback({
          {"error", "Could not request Steam authorization ticket."},
        });
      }
    }
  }
  else {
    ogalibData.loginInProgress = false;
    auto callback = ogalibDataSteam.authSessionTicketCallback;
    ogalibDataSteam.authSessionTicketCallback = nullptr;
    if(callback) {
      callback({
        {"error", "Steam is not available."},
      });
    }
  }
}
#endif
