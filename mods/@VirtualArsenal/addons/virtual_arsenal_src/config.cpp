class CfgPatches
{
    class virtual_arsenal
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 1.96;
        requiredAddons[] = {};
    };
};

// Scroll-menu action on every soldier (player only via condition).
class CfgVehicles
{
    class All {};
    class AllVehicles : All {};
    class Land : AllVehicles {};
    class Man : Land
    {
        class UserActions
        {
            class VA_OpenArsenal
            {
                displayName = "Virtual Arsenal";
                position = "aimpoint";
                radius = 3.5;
                onlyForPlayer = 1;
                condition = "alive this && local this && this == player";
                statement = "[] exec ""\virtual_arsenal\scripts\open.sqs""";
            };
        };
    };
};
