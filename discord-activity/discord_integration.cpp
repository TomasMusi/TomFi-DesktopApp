#include <iostream>
#include "discord_rpc.h"
#include "discord_integration.h"
#include <cstring>
#include <ctime>
#include "../env.hpp"

using namespace std;

void init_discord()
{
    string discord_id = env_vars["DISCORD_ID"];
    cout << " I am running now!";
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));

    Discord_Initialize(discord_id.c_str(), &handlers, 1, nullptr);

    DiscordRichPresence presence;
    memset(&presence, 0, sizeof(presence));

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
