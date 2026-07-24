// Mission telemetry — JSONL battlefield event log for headless simulate runs.
//
// Writes time-stamped entity tracks and combat events suitable for offline
// analysis (Tacview-like timeline review without graphics).
//
// Enable after AppConfig is parsed:
//
//   if (const auto& p = AppConfig::Instance().GetTelemetryPath(); !p.empty())
//       Poseidon::Dev::Telemetry::MissionTelemetry::Enable(p.c_str(),
//           AppConfig::Instance().GetTelemetryHz());
//
// Sample tracks from the simulate server tick; record combat from AI stats hooks.

#pragma once

namespace Poseidon
{
class EntityAI;
}

namespace Poseidon::Dev::Telemetry
{

class MissionTelemetry
{
public:
    static bool IsEnabled();
    static bool Enable(const char* path, int sampleHz = 2);
    static void Disable();

    /// Periodic entity position/orientation sample (mission game time).
    static void MaybeSampleWorld();

    /// Mission lifecycle markers: loaded, complete, endGame, timeout.
    static void RecordMissionEvent(const char* marker);

    static void RecordDeath(EntityAI* killed, EntityAI* killer);
    static void RecordDamage(EntityAI* injured, EntityAI* killer, float damage, const char* ammo);

private:
    static void WriteLine(const char* jsonBody);
};

} // namespace Poseidon::Dev::Telemetry
