void ReportToSteamServers(uint64_t steam_id, const BanInfo& info) {
    ISteamGameServer* server = SteamGameServer();
    server->LogOnAnonymous();
    server->SendUserConnectAndAuthenticate(steam_id, info.ban_data);
}
