#include "font.h"
#include "menu.h"
#include "controller.h"
#include "utils/cursor.hpp"
#include "utils/lines.hpp"
#include "libtpw_c/include/controller.h"

#define LINES 2

static Cursor cursor = {0,0};
bool init_once = false;
bool inventory_visible;

Line lines[LINES] = {
    {"item wheel", ITEM_WHEEL_INDEX, "Modify the item wheel", false},
    {"pause menu", PAUSE_MENU_INDEX, "Modify the pause menu collection", false}};

void InventoryMenu::render(Font& font) {

    if (button_is_pressed(Controller::B)) {
        init_once = false;
        inventory_visible = false;
        mm_visible = true;
        return;
    };

    if (!init_once) {Controller::reset_button(Controller::A);init_once = true;}
    
    if (button_is_down(Controller::A) && !button_is_held(Controller::A)) {
        switch (cursor.y) {
            case ITEM_WHEEL_INDEX: {
                inventory_visible = false;
                item_wheel_visible = true;
                return;
            }
            case PAUSE_MENU_INDEX: {
                inventory_visible = false;
                pause_visible = true;
                return;
            }
        }
    }

    Utilities::move_cursor(cursor, LINES);
    
    Utilities::render_lines(font, lines, cursor.y, LINES);
};