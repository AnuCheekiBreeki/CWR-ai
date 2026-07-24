#include <Poseidon/Dev/Telemetry/MissionTelemetry.hpp>

#include <Poseidon/AI/AIGroup.hpp>
#include <Poseidon/AI/AIUnit.hpp>
#include <Poseidon/AI/EntityAI.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/Dev/Diag/PerfTrace.hpp>
#include <Poseidon/Foundation/Time/Time.hpp>
#include <Poseidon/World/World.hpp>

#include <atomic>
#include <cmath>
#include <cstdio>
#include <mutex>

namespace Poseidon::Dev::Telemetry
{

namespace
{

std::mutex g_mtx;
std::FILE* g_file = nullptr;
std::atomic<bool> g_enabled{false};
int g_sampleHz = 2;
Time g_lastSampleTime = Time(-1e9f);

float MissionElapsedSeconds()
{
    return (Glob.time - Time(0)).toFloat();
}

const char* SideLabel(TargetSide side)
{
    switch (side)
    {
        case TWest:
            return "WEST";
        case TEast:
            return "EAST";
        case TGuerrila:
            return "GUER";
        case TCivilian:
            return "CIV";
        default:
            return "UNKNOWN";
    }
}

float HeadingDegrees(Vector3Par dir)
{
    return std::atan2(dir.X(), dir.Z()) * (180.0f / 3.14159265f);
}

void AppendJsonString(char* buf, int bufSize, int& pos, const char* key, const char* value)
{
    if (!value)
    {
        value = "";
    }
    char escaped[512];
    escaped[0] = '\0';
    Poseidon::Dev::Perf::Trace::AppendEscapedString(escaped, sizeof(escaped), value);
    pos += std::snprintf(buf + pos, bufSize - pos, "%s\"%s\":%s", pos > 1 ? "," : "", key, escaped);
}

void AppendJsonInt(char* buf, int bufSize, int& pos, const char* key, int value)
{
    pos += std::snprintf(buf + pos, bufSize - pos, "%s\"%s\":%d", pos > 1 ? "," : "", key, value);
}

void AppendJsonFloat(char* buf, int bufSize, int& pos, const char* key, float value)
{
    pos += std::snprintf(buf + pos, bufSize - pos, "%s\"%s\":%.3f", pos > 1 ? "," : "", key, value);
}

void AppendJsonBool(char* buf, int bufSize, int& pos, const char* key, bool value)
{
    pos += std::snprintf(buf + pos, bufSize - pos, "%s\"%s\":%s", pos > 1 ? "," : "", key, value ? "true" : "false");
}

void EntityIdentityFields(char* buf, int bufSize, int& pos, EntityAI* entity)
{
    const NetworkId netId = entity->GetNetworkId();
    AppendJsonInt(buf, bufSize, pos, "netCreator", netId.creator);
    AppendJsonInt(buf, bufSize, pos, "netId", netId.id);
    AppendJsonString(buf, bufSize, pos, "name", entity->GetDebugName());
    if (const EntityAIType* type = entity->GetType())
    {
        AppendJsonString(buf, bufSize, pos, "class", type->GetName());
    }
    AppendJsonString(buf, bufSize, pos, "side", SideLabel(entity->GetTargetSide()));

    if (AIUnit* unit = entity->CommanderUnit())
    {
        if (AIGroup* group = unit->GetGroup())
        {
            AppendJsonString(buf, bufSize, pos, "group", group->GetDebugName());
        }
    }
}

std::FILE* OpenForWrite(const char* path)
{
#ifdef _MSC_VER
    std::FILE* f = nullptr;
    if (fopen_s(&f, path, "wb") != 0)
    {
        return nullptr;
    }
    return f;
#else
    return std::fopen(path, "wb");
#endif
}

} // namespace

bool MissionTelemetry::IsEnabled()
{
    return g_enabled.load(std::memory_order_acquire);
}

bool MissionTelemetry::Enable(const char* path, int sampleHz)
{
    if (!path || !*path)
    {
        return false;
    }
    std::lock_guard lock(g_mtx);
    if (g_file)
    {
        std::fclose(g_file);
        g_file = nullptr;
    }
    g_file = OpenForWrite(path);
    if (!g_file)
    {
        g_enabled.store(false, std::memory_order_release);
        return false;
    }
    g_sampleHz = sampleHz > 0 ? sampleHz : 2;
    g_lastSampleTime = Time(-1e9f);
    g_enabled.store(true, std::memory_order_release);
    return true;
}

void MissionTelemetry::Disable()
{
    std::lock_guard lock(g_mtx);
    g_enabled.store(false, std::memory_order_release);
    if (g_file)
    {
        std::fflush(g_file);
        std::fclose(g_file);
        g_file = nullptr;
    }
}

void MissionTelemetry::WriteLine(const char* jsonBody)
{
    if (!IsEnabled())
    {
        return;
    }
    std::lock_guard lock(g_mtx);
    if (!g_file)
    {
        return;
    }
    std::fputs(jsonBody, g_file);
    std::fputc('\n', g_file);
}

void MissionTelemetry::RecordMissionEvent(const char* marker)
{
    if (!IsEnabled() || !marker)
    {
        return;
    }
    char buf[512];
    std::snprintf(buf, sizeof(buf), "{\"t\":\"mission\",\"time\":%.3f,\"event\":\"%s\"}", MissionElapsedSeconds(),
                  marker);
    WriteLine(buf);
}

void MissionTelemetry::MaybeSampleWorld()
{
    if (!IsEnabled() || !GWorld)
    {
        return;
    }

    const float interval = 1.0f / static_cast<float>(g_sampleHz > 0 ? g_sampleHz : 2);
    if (g_lastSampleTime >= Time(0) && (Glob.time - g_lastSampleTime).toFloat() < interval)
    {
        return;
    }
    g_lastSampleTime = Glob.time;

    const float elapsed = MissionElapsedSeconds();

    for (int i = 0; i < GWorld->NVehicles(); ++i)
    {
        Entity* entity = GWorld->GetVehicle(i);
        EntityAI* ai = dyn_cast<EntityAI>(entity);
        if (!ai || ai->IsDammageDestroyed())
        {
            continue;
        }
        if (!ai->IsInLandscape())
        {
            continue;
        }

        const Vector3 pos = ai->Position();
        const Vector3 dir = ai->Direction();
        const float speed = ai->WorldSpeed().SquareSize();

        char buf[1024];
        int posOut = 0;
        buf[posOut++] = '{';
        posOut += std::snprintf(buf + posOut, sizeof(buf) - posOut, "\"t\":\"track\",\"time\":%.3f", elapsed);
        EntityIdentityFields(buf, sizeof(buf), posOut, ai);
        AppendJsonFloat(buf, sizeof(buf), posOut, "x", pos.X());
        AppendJsonFloat(buf, sizeof(buf), posOut, "y", pos.Y());
        AppendJsonFloat(buf, sizeof(buf), posOut, "z", pos.Z());
        AppendJsonFloat(buf, sizeof(buf), posOut, "hdg", HeadingDegrees(dir));
        AppendJsonFloat(buf, sizeof(buf), posOut, "spd", speed);
        AppendJsonBool(buf, sizeof(buf), posOut, "alive", true);
        posOut += std::snprintf(buf + posOut, sizeof(buf) - posOut, "}");
        WriteLine(buf);
    }
}

void MissionTelemetry::RecordDeath(EntityAI* killed, EntityAI* killer)
{
    if (!IsEnabled() || !killed)
    {
        return;
    }

    const Vector3 pos = killed->Position();
    char buf[1024];
    int posOut = 0;
    buf[posOut++] = '{';
    posOut += std::snprintf(buf + posOut, sizeof(buf) - posOut, "\"t\":\"death\",\"time\":%.3f", MissionElapsedSeconds());
    AppendJsonString(buf, sizeof(buf), posOut, "victimName", killed->GetDebugName());
    if (const EntityAIType* type = killed->GetType())
    {
        AppendJsonString(buf, sizeof(buf), posOut, "victimClass", type->GetName());
    }
    AppendJsonFloat(buf, sizeof(buf), posOut, "x", pos.X());
    AppendJsonFloat(buf, sizeof(buf), posOut, "y", pos.Y());
    AppendJsonFloat(buf, sizeof(buf), posOut, "z", pos.Z());
    if (killer)
    {
        AppendJsonString(buf, sizeof(buf), posOut, "killerName", killer->GetDebugName());
        if (const EntityAIType* killerType = killer->GetType())
        {
            AppendJsonString(buf, sizeof(buf), posOut, "killerClass", killerType->GetName());
        }
    }
    posOut += std::snprintf(buf + posOut, sizeof(buf) - posOut, "}");
    WriteLine(buf);
}

void MissionTelemetry::RecordDamage(EntityAI* injured, EntityAI* killer, float damage, const char* ammo)
{
    if (!IsEnabled() || !injured)
    {
        return;
    }

    const Vector3 pos = injured->Position();
    char buf[1024];
    int posOut = 0;
    buf[posOut++] = '{';
    posOut += std::snprintf(buf + posOut, sizeof(buf) - posOut, "\"t\":\"damage\",\"time\":%.3f", MissionElapsedSeconds());
    AppendJsonString(buf, sizeof(buf), posOut, "victimName", injured->GetDebugName());
    if (const EntityAIType* type = injured->GetType())
    {
        AppendJsonString(buf, sizeof(buf), posOut, "victimClass", type->GetName());
    }
    AppendJsonFloat(buf, sizeof(buf), posOut, "x", pos.X());
    AppendJsonFloat(buf, sizeof(buf), posOut, "y", pos.Y());
    AppendJsonFloat(buf, sizeof(buf), posOut, "z", pos.Z());
    AppendJsonFloat(buf, sizeof(buf), posOut, "damage", damage);
    if (killer)
    {
        AppendJsonString(buf, sizeof(buf), posOut, "killerName", killer->GetDebugName());
    }
    if (ammo && *ammo)
    {
        AppendJsonString(buf, sizeof(buf), posOut, "ammo", ammo);
    }
    posOut += std::snprintf(buf + posOut, sizeof(buf) - posOut, "}");
    WriteLine(buf);
}

} // namespace Poseidon::Dev::Telemetry
