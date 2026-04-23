#include "server/monitor/Monitor.hpp"

#include "server/ServerInstance.hpp"

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace server
{

MonitorDirector::MonitorDirector(ServerInstance& serverInstance)
  : _serverInstance(serverInstance)
{
}

void MonitorDirector::Initialize()
{
  _server.RegisterProvider(
    "ranch",
    [this]()
    {
      const auto snap = _serverInstance.GetRanchDirector().GetSnapshot();
      nlohmann::json ranches = nlohmann::json::array();
      for (const auto& r : snap.ranches)
      {
        nlohmann::json clients = nlohmann::json::array();
        for (const auto& c : r.clients)
        {
          clients.push_back({
            {"username",       c.userName},
            {"character_uid",  c.characterUid},
            {"character_name", c.characterName}
          });
        }
        ranches.push_back({
          {"rancher_uid",  r.rancherUid},
          {"ranch_name",   r.ranchName},
          {"client_count", r.clients.size()},
          {"clients",      std::move(clients)}
        });
      }
      return nlohmann::json{
        {"total_clients", snap.totalClients},
        {"ranches",       std::move(ranches)}
      };
    });

  _server.RegisterProvider(
    "race",
    [this]()
    {
      const auto snap = _serverInstance.GetRaceDirector().GetSnapshot();
      nlohmann::json roomsJson = nlohmann::json::array();
      for (const auto& r : snap.rooms)
      {
        nlohmann::json clients = nlohmann::json::array();
        for (const auto& c : r.clients)
        {
          clients.push_back({
            {"username",       c.userName},
            {"character_uid",  c.characterUid},
            {"character_name", c.characterName}
          });
        }
        roomsJson.push_back({
          {"room_uid",           r.roomUid},
          {"room_name",          r.roomName},
          {"game_mode",          r.gameMode},
          {"team_mode",          r.teamMode},
          {"current_players",    r.clients.size()},
          {"max_players",        r.maxPlayers},
          {"stage",              r.stage},
          {"stage_start_ms",     r.stageStartMs},
          {"selected_course_id", r.selectedCourseId},
          {"map_block_id",       r.mapBlockId},
          {"master_name",        r.masterName},
          {"clients",            std::move(clients)}
        });
      }
      return nlohmann::json{
        {"total_clients", snap.totalClients},
        {"rooms",         std::move(roomsJson)}
      };
    });

  const auto& cfg = _serverInstance.GetSettings().monitor;
  _server.Listen(cfg.listen.address, cfg.listen.port);
}

void MonitorDirector::Run()
{
  _server.Run();
}

void MonitorDirector::Terminate()
{
  _server.Stop();
}

} // namespace server
