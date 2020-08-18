#include "libtpw_c/include/controller.h"
#include "cheats.h"
#include "controller.h"
#include "menu.h"
#include "commands.h"
#include "libtpw_c/include/tp.h"
#include "libtpw_c/include/system.h"

#define BUTTON_STATES 13
#define REPEAT_TIME 6
#define REPEAT_DELAY 10

static uint16_t sButtons_down_last_frame = 0;
static uint16_t sButtons_down = 0;
static uint16_t sButtons_pressed = 0;
bool a_held = true;
bool a_held_last_frame = true;
uint16_t current_input = 0x0000;

bool can_move_cursor = false;
static uint16_t sNum_frames_cursor_buffer = 0;

struct ButtonState {
    uint16_t button;
    uint32_t pressed_frame;
    bool is_down;
};

static ButtonState buttonStates[BUTTON_STATES] = {
    {Controller::Mote::DPAD_LEFT, 0xFFFFFFFF, false},
    {Controller::Mote::DPAD_RIGHT, 0xFFFFFFFF, false},
    {Controller::Mote::DPAD_DOWN, 0xFFFFFFFF, false},
    {Controller::Mote::DPAD_UP, 0xFFFFFFFF, false},
    {Controller::Mote::PLUS, 0xFFFFFFFF, false},
    {Controller::Mote::TWO, 0xFFFFFFFF, false},
    {Controller::Mote::ONE, 0xFFFFFFFF, false},
    {Controller::Mote::B, 0xFFFFFFFF, false},
    {Controller::Mote::A, 0xFFFFFFFF, false},
    {Controller::Mote::MINUS, 0xFFFFFFFF, false},
    {Controller::Mote::Z, 0xFFFFFFFF, false},
    {Controller::Mote::C, 0xFFFFFFFF, false},
    {Controller::Mote::HOME, 0xFFFFFFFF, false}};

extern "C" uint32_t read_controller() {
    sButtons_down_last_frame = sButtons_down;
    sButtons_down = tp_mPad.buttons;
    sButtons_pressed = sButtons_down & (0xFFFF ^ sButtons_down_last_frame);

    uint8_t idx = 0;
    for (; idx < BUTTON_STATES; idx++) {
        buttonStates[idx].is_down = (buttonStates[idx].button & sButtons_down) != 0;
        if ((buttonStates[idx].button & sButtons_pressed) != 0) {
            buttonStates[idx].pressed_frame = TP::get_frame_count() + 1;
        }
    }

    Cheats::apply_cheats();
    if (mm_visible || prac_visible || settings_visible || cheats_visible ||
        tools_visible || inventory_visible || item_wheel_visible || pause_visible ||
        memory_visible || warping_visible || flags_menu_visible ||
        scene_menu_visible || any_saves_visible || hundo_saves_visible) {
        current_input = Controller::get_current_inputs();
        a_held = a_held_last_frame && current_input == Controller::Mote::A;
        a_held_last_frame = current_input == Controller::Mote::A;

        // prevent accidentally moving cursor down when opening menu
        if (!can_move_cursor) {
            if (current_input & Controller::Mote::MINUS) {
                can_move_cursor = true;
            } else if (current_input & (Controller::Mote::Z | Controller::Mote::C)) {
                sNum_frames_cursor_buffer = 0;
            } else if (sNum_frames_cursor_buffer < 1) {
                sNum_frames_cursor_buffer = 1;
            }

            if (sNum_frames_cursor_buffer >= 4) {
                can_move_cursor = true;
            } else if (sNum_frames_cursor_buffer > 0) {
                sNum_frames_cursor_buffer++;
            }
        }

        Controller::set_buttons_down(0x0);
        Controller::set_buttons_pressed(0x0);
    } else {
        can_move_cursor = false;
        sNum_frames_cursor_buffer = 0;
        Commands::process_inputs();
    }
    return 0x80000000;
}

namespace Controller {

    bool button_is_down(int idx) {
        return buttonStates[idx].is_down;
    }

    bool button_is_pressed(int idx) {
        auto delta = TP::get_frame_count() - buttonStates[idx].pressed_frame;
        auto just_clicked = delta == 0;
        auto held_down_long_enough = delta > REPEAT_DELAY;
        auto is_repeat_frame = held_down_long_enough && delta % REPEAT_TIME == 0;
        auto down = button_is_down(idx);
        return down && (just_clicked || is_repeat_frame);
    }

    uint16_t get_current_inputs() {
        return tp_mPad.buttons;
    }

    bool button_is_held(int idx) {
        auto delta = TP::get_frame_count() - buttonStates[idx].pressed_frame + 1;
        if (delta != 0) {
            return true;
        } else {
            return false;
        }
    }
}  // namespace Controller