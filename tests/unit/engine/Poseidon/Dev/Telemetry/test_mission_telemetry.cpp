// Unit tests for MissionTelemetry — JSONL mission event log used by
// headless simulate runs for Tacview-like offline analysis.

#include <catch2/catch_test_macros.hpp>
#include <Poseidon/Dev/Telemetry/MissionTelemetry.hpp>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace
{

std::string MakeTelemetryPath(const char* tag)
{
    static thread_local std::mt19937_64 rng{std::random_device{}()};
    std::ostringstream oss;
    oss << std::filesystem::temp_directory_path().generic_string() << "/mission-telemetry-" << tag << "-" << rng()
        << ".jsonl";
    return oss.str();
}

std::vector<std::string> ReadLines(const std::string& path)
{
    std::ifstream f(path);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(f, line))
    {
        if (!line.empty())
        {
            lines.push_back(line);
        }
    }
    return lines;
}

bool LooksLikeJsonObject(const std::string& s)
{
    if (s.size() < 2 || s.front() != '{' || s.back() != '}')
    {
        return false;
    }
    int depth = 0;
    bool inString = false;
    bool escaped = false;
    for (char c : s)
    {
        if (escaped)
        {
            escaped = false;
            continue;
        }
        if (c == '\\' && inString)
        {
            escaped = true;
            continue;
        }
        if (c == '"')
        {
            inString = !inString;
            continue;
        }
        if (inString)
        {
            continue;
        }
        if (c == '{')
        {
            ++depth;
        }
        else if (c == '}')
        {
            --depth;
        }
    }
    return depth == 0 && !inString;
}

} // namespace

TEST_CASE("MissionTelemetry::IsEnabled is true after Enable", "[mission_telemetry]")
{
    const std::string path = MakeTelemetryPath("enable");
    REQUIRE(Poseidon::Dev::Telemetry::MissionTelemetry::Enable(path.c_str(), 2));
    REQUIRE(Poseidon::Dev::Telemetry::MissionTelemetry::IsEnabled());
    Poseidon::Dev::Telemetry::MissionTelemetry::Disable();
    REQUIRE_FALSE(Poseidon::Dev::Telemetry::MissionTelemetry::IsEnabled());
    std::filesystem::remove(path);
}

TEST_CASE("MissionTelemetry::RecordMissionEvent writes mission marker", "[mission_telemetry][mission]")
{
    const std::string path = MakeTelemetryPath("mission");
    REQUIRE(Poseidon::Dev::Telemetry::MissionTelemetry::Enable(path.c_str(), 2));
    Poseidon::Dev::Telemetry::MissionTelemetry::RecordMissionEvent("loaded");
    Poseidon::Dev::Telemetry::MissionTelemetry::Disable();

    const auto lines = ReadLines(path);
    REQUIRE(lines.size() == 1);
    REQUIRE(LooksLikeJsonObject(lines[0]));
    REQUIRE(lines[0].find("\"t\":\"mission\"") != std::string::npos);
    REQUIRE(lines[0].find("\"event\":\"loaded\"") != std::string::npos);
    std::filesystem::remove(path);
}

TEST_CASE("MissionTelemetry::Disable is a no-op when disabled", "[mission_telemetry][edge]")
{
    Poseidon::Dev::Telemetry::MissionTelemetry::Disable();
    REQUIRE_FALSE(Poseidon::Dev::Telemetry::MissionTelemetry::IsEnabled());
}
