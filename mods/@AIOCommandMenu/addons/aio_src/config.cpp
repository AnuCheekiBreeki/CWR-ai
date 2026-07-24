class CfgPatches
{
    class aio_command_menu
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 1.96;
        requiredAddons[] = {};
    };
};

class CfgVehicles
{
    class All {};
    class AllVehicles : All {};
    class Land : AllVehicles {};
    class Man : Land
    {
        class UserActions
        {
            class AIO_OpenMenu
            {
                displayName = "AIO Command Menu";
                position = "aimpoint";
                radius = 3.5;
                onlyForPlayer = 1;
                condition = "alive this && local this && this == player";
                statement = "[] exec ""\aio\scripts\open_menu.sqs""";
            };
            class AIO_OpenArsenal
            {
                displayName = "Virtual Arsenal";
                position = "aimpoint";
                radius = 3.5;
                onlyForPlayer = 1;
                condition = "alive this && local this && this == player";
                statement = "[] exec ""\aio\scripts\arsenal\open_arsenal.sqs""";
            };
        };
    };
};
