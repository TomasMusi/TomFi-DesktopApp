#include <iostream>
#include "discord_rpc.h"
#include "discord_integration.h"
#include <cstring>
#include <ctime>

using namespace std;

void init_discord()
{

    cout << " I am running now!";
    DiscordEventHandlers handlers;
    std::memset(&handlers, 0, sizeof(handlers));

    Discord_Initialize("1383726668408946708", &handlers, 1, nullptr);

    DiscordRichPresence presence;
    std::memset(&presence, 0, sizeof(presence));

    presence.state = "Browsing";
    presence.details = "Using TomFi App";
    presence.startTimestamp = std::time(nullptr);
    presence.largeImageKey = "TomFi";
    presence.largeImageText = "TomFi";

    Discord_UpdatePresence(&presence);
}

void shutdown_discord()
{
    Discord_Shutdown();
}
