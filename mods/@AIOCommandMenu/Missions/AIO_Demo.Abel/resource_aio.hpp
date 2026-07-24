// AIO Command Menu — Poseidon visual clone of the vanilla commanding / radio menu.
// Geometry & colors match CfgInGameUI >> Menu (left=0.78 top=0.02 w=0.20 h=0.28).
// Arma 3 AIO uses showCommandingMenu; Poseidon has no scripted commanding-menu API,
// so this is a createDialog lookalike (CT_ACTIVETEXT rows + caption).
// Include after resource_arsenal.hpp. Install: install_resource_extra.sh

#include "resource_arsenal.hpp"

///////////////////////////////////////////////////////////////////////////
/// Shared ActiveText style (vanilla command-menu look)
///////////////////////////////////////////////////////////////////////////
class AIO_RscActiveMenu
{
  type = 11; // CT_ACTIVETEXT
  idc = -1;
  style = 0; // ST_LEFT
  x = 0.785;
  w = 0.190;
  h = 0.022;
  font = "tahomaB24";
  sizeEx = 0.02;
  color[] = {0.8, 0.8, 0.8, 1};
  colorActive[] = {1, 1, 1, 1};
  soundEnter[] = {"", 0.1, 1};
  soundPush[] = {"", 0.1, 1};
  soundClick[] = {"ui\ui_ok", 0.2, 1};
  soundEscape[] = {"ui\ui_cc", 0.2, 1};
  text = "";
  action = "";
};

///////////////////////////////////////////////////////////////////////////
/// AIO Command Menu (radio / commanding-menu layout)
///////////////////////////////////////////////////////////////////////////
class RscDisplayAIOCommandMenu
{
  idd = 77100;
  movingEnable = 0;
  enableSimulation = 1;
  controlsBackground[] = {"AIO_Frame"};
  objects[] = {};
  controls[] =
  {
    "AIO_Item1",
    "AIO_Item2",
    "AIO_Item3",
    "AIO_Item4",
    "AIO_Item5",
    "AIO_Item6",
    "AIO_Item7",
    "AIO_Item8",
    "AIO_Back",
    "AIO_Caption"
  };

  // Semi-transparent panel — same role as InGameUI::DrawMenu frame
  class AIO_Frame : VA_RscText
  {
    idc = -1;
    x = 0.78;
    y = 0.02;
    w = 0.20;
    h = 0.28;
    colorBackground[] = {0, 0, 0, 0.8};
    colorText[] = {0, 0, 0, 0};
    text = "";
  };

  class AIO_Item1 : AIO_RscActiveMenu
  {
    idc = 77101;
    y = 0.025;
    text = "1  Virtual Arsenal";
    action = "closeDialog 0; [] exec ""\aio\scripts\arsenal\open_arsenal.sqs""";
  };
  class AIO_Item2 : AIO_RscActiveMenu
  {
    idc = 77102;
    y = 0.047;
    text = "2  Heal up!";
    action = "closeDialog 0; [] exec ""\aio\scripts\cheats\heal.sqs""";
  };
  class AIO_Item3 : AIO_RscActiveMenu
  {
    idc = 77103;
    y = 0.069;
    text = "3  God mode";
    action = "closeDialog 0; [] exec ""\aio\scripts\cheats\godmode.sqs""";
  };
  class AIO_Item4 : AIO_RscActiveMenu
  {
    idc = 77104;
    y = 0.091;
    text = "4  Add ammo";
    action = "closeDialog 0; [] exec ""\aio\scripts\cheats\ammo.sqs""";
  };
  class AIO_Item5 : AIO_RscActiveMenu
  {
    idc = 77105;
    y = 0.113;
    text = "5  Teleport";
    action = "closeDialog 0; [] exec ""\aio\scripts\cheats\teleport.sqs""";
  };
  class AIO_Item6 : AIO_RscActiveMenu
  {
    idc = 77106;
    y = 0.135;
    text = "6  Skip time +1h";
    action = "closeDialog 0; [] exec ""\aio\scripts\cheats\skiptime.sqs""";
  };
  class AIO_Item7 : AIO_RscActiveMenu
  {
    idc = 77107;
    y = 0.157;
    text = "7  AccTime cycle";
    action = "closeDialog 0; [] exec ""\aio\scripts\cheats\acctime.sqs""";
  };
  class AIO_Item8 : AIO_RscActiveMenu
  {
    idc = 77108;
    y = 0.179;
    text = "8  Captive toggle";
    action = "closeDialog 0; [] exec ""\aio\scripts\cheats\captive.sqs""";
  };

  // Bottom row — mirrors Backspace item in DrawMenu
  class AIO_Back : AIO_RscActiveMenu
  {
    idc = 2;
    y = 0.273;
    text = "Backspace  Back";
    action = "closeDialog 0";
  };

  // Caption under the frame (vanilla DrawMenu title strip)
  class AIO_Caption : VA_RscText
  {
    idc = 77110;
    style = 1;
    x = 0.78;
    y = 0.30;
    w = 0.20;
    h = 0.028;
    font = "tahomaB24";
    sizeEx = 0.02;
    colorBackground[] = {0, 0, 0, 0.55};
    colorText[] = {0.8, 0.8, 0.8, 1};
    text = "Cheats";
  };
};
