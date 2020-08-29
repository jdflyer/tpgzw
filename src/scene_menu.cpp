#include "menu.h"
#include "controller.h"
#include "libtpw_c/include/controller.h"
#include "utils.h"
#include <stdio.h>
#include "libtpw_c/include/tp.h"
#define LINES SCENE_AMNT

static Cursor cursor = {0, 0};
bool init_once = false;
bool scene_menu_visible;
using namespace Scene;

SceneItem SceneItems[SCENE_AMNT] = {
    {FREEZE_ACTOR_INDEX, false},
    {HIDE_ACTOR_INDEX, false},
    {DISABLE_SFX_INDEX, false},
    {FREEZE_CAMERA_INDEX, false},
    {HIDE_HUD_INDEX, false}};

Line lines[LINES] = {
    {"disable sfx", DISABLE_SFX_INDEX, "Disables sound effects (item, weather, etc.)", true, &SceneItems[DISABLE_SFX_INDEX].active},
    {"freeze actors", FREEZE_ACTOR_INDEX, "Freezes actors", true, &SceneItems[FREEZE_ACTOR_INDEX].active},
    {"freeze camera", FREEZE_CAMERA_INDEX, "Locks the camera in place", true, &SceneItems[FREEZE_CAMERA_INDEX].active},
    {"hide actors", HIDE_ACTOR_INDEX, "Hides actors", true, &SceneItems[HIDE_ACTOR_INDEX].active},
    {"hide hud", HIDE_HUD_INDEX, "Hides the heads-up display", true, &SceneItems[HIDE_HUD_INDEX].active},
    {"", TIME_HOURS_INDEX, "The current in-game hour", false},
    {"", TIME_MINUTES_INDEX, "The current in-game minutes", false}};

void SceneMenu::render(Font& font) {

    if (button_is_pressed(Controller::B)) {
        scene_menu_visible = false;
        mm_visible = true;
        init_once = false;
        return;
    };

    if (!init_once) {
        current_input = 0;
        init_once = true;
    }

    int current_hour = (int)tp_gameInfo.raw_game_time / 15;
    if (current_hour > 23) {
        current_hour = 0;
    }
    int current_minute = (int)((4.0f * tp_gameInfo.raw_game_time) - current_hour * 60);

    sprintf(lines[TIME_HOURS_INDEX].line, "time (hrs):      %d", current_hour);
    sprintf(lines[TIME_MINUTES_INDEX].line, "time (mins):     %d", current_minute);

    Utilities::move_cursor(cursor, LINES);
    Utilities::render_lines(font, lines, cursor.x, LINES, 160.0f);

    if (current_input == Controller::Mote::A && a_held == false) {
        SceneItems[cursor.x].active = !SceneItems[cursor.x].active;
        if (SceneItems[cursor.x].active) {
            switch (cursor.x) {
                case TIME_HOURS_INDEX: {
                    tp_gameInfo.raw_game_time += 14.75f;
                }
                case TIME_MINUTES_INDEX: {
                    tp_gameInfo.raw_game_time += 0.25f;
                }
            }
        }
    }
}
