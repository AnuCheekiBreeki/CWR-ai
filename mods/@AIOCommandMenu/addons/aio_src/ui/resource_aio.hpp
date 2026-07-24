// AIO Command Menu dialog (Poseidon MVP).
// Include after resource_arsenal.hpp (shares VA_Rsc* control classes).
// Install: mods/@AIOCommandMenu/install_resource_extra.sh

#include "resource_arsenal.hpp"

///////////////////////////////////////////////////////////////////////////
/// AIO Command Menu (cheat + arsenal hub)
///////////////////////////////////////////////////////////////////////////
class RscDisplayAIOCommandMenu
{
  idd = 77100;
  movingEnable = 1;
  controlsBackground[] = {"AIO_Bg", "AIO_Title"};
  objects[] = {};
  controls[] =
  {
    "AIO_List",
    "AIO_Hint",
    "AIO_BtnRun",
    "AIO_BtnClose"
  };

  class AIO_Bg : VA_RscText
  {
    idc = -1;
    x = 0.02; y = 0.08; w = 0.40; h = 0.84;
    colorBackground[] = {0.05, 0.05, 0.08, 0.92};
    text = "";
  };
  class AIO_Title : VA_RscText
  {
    idc = -1;
    x = 0.02; y = 0.08; w = 0.40; h = 0.05;
    colorBackground[] = {0.12, 0.18, 0.28, 1};
    colorText[] = {1, 1, 1, 1};
    sizeEx = 0.032;
    text = "AIO COMMAND MENU";
  };
  class AIO_List : VA_RscListBox
  {
    idc = 77130;
    x = 0.04; y = 0.15; w = 0.36; h = 0.58;
    sizeEx = 0.024;
    rowHeight = 0.032;
  };
  class AIO_Hint : VA_RscText
  {
    idc = 77111;
    x = 0.04; y = 0.74; w = 0.36; h = 0.06;
    sizeEx = 0.020;
    colorText[] = {0.85, 0.85, 0.9, 1};
    text = "Select an action, then Execute.";
  };
  class AIO_BtnRun : VA_RscButton
  {
    idc = 77141;
    x = 0.04; y = 0.82; w = 0.16; h = 0.045;
    text = "Execute";
  };
  class AIO_BtnClose : VA_RscButton
  {
    idc = 77142;
    x = 0.22; y = 0.82; w = 0.16; h = 0.045;
    text = "Close";
  };
};
