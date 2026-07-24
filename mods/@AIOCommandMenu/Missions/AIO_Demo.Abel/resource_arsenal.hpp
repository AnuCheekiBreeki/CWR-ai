// Virtual Arsenal dialog + control classes (shared by mission description.ext
// and optional BIN/resource-extra merge).

#define CT_STATIC 0
#define CT_BUTTON 1
#define CT_LISTBOX 5
#define ST_LEFT 0
#define ST_CENTER 2
#define ST_FRAME 64

#define FontM "tahomaB24"

class VA_RscText
{
    type = CT_STATIC;
    idc = -1;
    style = ST_LEFT;
    colorBackground[] = {0, 0, 0, 0};
    colorText[] = {1, 1, 1, 1};
    font = FontM;
    sizeEx = 0.024;
    text = "";
};

class VA_RscButton
{
    type = CT_BUTTON;
    idc = -1;
    style = ST_CENTER;
    colorText[] = {1, 1, 1, 1};
    colorDisabled[] = {0.4, 0.4, 0.4, 1};
    colorBackground[] = {0.12, 0.18, 0.12, 0.9};
    colorBackgroundDisabled[] = {0.1, 0.1, 0.1, 0.6};
    colorBackgroundActive[] = {0.2, 0.35, 0.2, 1};
    colorFocused[] = {0.2, 0.35, 0.2, 1};
    colorShadow[] = {0, 0, 0, 0.4};
    colorBorder[] = {0, 0, 0, 1};
    soundEnter[] = {"", 0.1, 1};
    soundPush[] = {"", 0.1, 1};
    soundClick[] = {"ui\ui_ok", 0.2, 1};
    soundEscape[] = {"ui\ui_cc", 0.2, 1};
    font = FontM;
    sizeEx = 0.022;
    offsetX = 0.002;
    offsetY = 0.002;
    offsetPressedX = 0.001;
    offsetPressedY = 0.001;
    borderSize = 0;
    text = "";
};

class VA_RscListBox
{
    type = CT_LISTBOX;
    idc = -1;
    style = ST_LEFT;
    colorText[] = {0.85, 1, 0.85, 1};
    colorSelect[] = {0, 0, 0, 1};
    colorSelectBackground[] = {0.4, 0.8, 0.4, 1};
    colorBackground[] = {0, 0, 0, 0.55};
    font = FontM;
    sizeEx = 0.022;
    rowHeight = 0.028;
};

class RscDisplayVirtualArsenal
{
    idd = 77001;
    movingEnable = 1;
    controlsBackground[] = {"VA_Bg", "VA_Frame"};
    objects[] = {};
    controls[] =
    {
        "VA_Title",
        "VA_CatPrimary",
        "VA_CatSecondary",
        "VA_CatHandgun",
        "VA_CatBinocular",
        "VA_CatMagazine",
        "VA_CatItem",
        "VA_List",
        "VA_Equipped",
        "VA_Hint",
        "VA_BtnApply",
        "VA_BtnClear",
        "VA_BtnRotateL",
        "VA_BtnRotateR",
        "VA_BtnClose"
    };

    class VA_Bg : VA_RscText
    {
        idc = -1;
        x = 0.02; y = 0.04; w = 0.96; h = 0.92;
        colorBackground[] = {0.02, 0.05, 0.02, 0.82};
        text = "";
    };
    class VA_Frame : VA_RscText
    {
        idc = -1;
        style = ST_FRAME;
        x = 0.03; y = 0.05; w = 0.94; h = 0.90;
        colorText[] = {0.4, 0.8, 0.4, 1};
        text = "";
    };
    class VA_Title : VA_RscText
    {
        idc = 77010;
        x = 0.05; y = 0.07; w = 0.60; h = 0.04;
        sizeEx = 0.032;
        text = "VIRTUAL ARSENAL";
    };
    class VA_CatPrimary : VA_RscButton { idc = 77021; x = 0.05; y = 0.13; w = 0.14; h = 0.035; text = "Primary"; };
    class VA_CatSecondary : VA_RscButton { idc = 77022; x = 0.20; y = 0.13; w = 0.14; h = 0.035; text = "Launcher"; };
    class VA_CatHandgun : VA_RscButton { idc = 77023; x = 0.35; y = 0.13; w = 0.14; h = 0.035; text = "Handgun"; };
    class VA_CatBinocular : VA_RscButton { idc = 77024; x = 0.50; y = 0.13; w = 0.14; h = 0.035; text = "Optics"; };
    class VA_CatMagazine : VA_RscButton { idc = 77025; x = 0.65; y = 0.13; w = 0.14; h = 0.035; text = "Mags"; };
    class VA_CatItem : VA_RscButton { idc = 77026; x = 0.80; y = 0.13; w = 0.14; h = 0.035; text = "Items"; };
    class VA_List : VA_RscListBox { idc = 77030; x = 0.05; y = 0.18; w = 0.42; h = 0.58; };
    class VA_Equipped : VA_RscListBox { idc = 77031; x = 0.50; y = 0.18; w = 0.44; h = 0.40; };
    class VA_Hint : VA_RscText
    {
        idc = 77011;
        x = 0.50; y = 0.60; w = 0.44; h = 0.12;
        text = "Select a class, Apply equips player. Rotate orbits the camera around you.";
    };
    class VA_BtnApply : VA_RscButton { idc = 77041; x = 0.05; y = 0.80; w = 0.16; h = 0.04; text = "Apply"; };
    class VA_BtnClear : VA_RscButton { idc = 77042; x = 0.23; y = 0.80; w = 0.16; h = 0.04; text = "Clear gear"; };
    class VA_BtnRotateL : VA_RscButton { idc = 77043; x = 0.50; y = 0.80; w = 0.14; h = 0.04; text = "<< Rotate"; };
    class VA_BtnRotateR : VA_RscButton { idc = 77044; x = 0.66; y = 0.80; w = 0.14; h = 0.04; text = "Rotate >>"; };
    class VA_BtnClose : VA_RscButton { idc = 77045; x = 0.82; y = 0.80; w = 0.12; h = 0.04; text = "Close"; };
};
