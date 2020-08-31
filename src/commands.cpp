#include "commands.h"
#include "menu.h"
#include "utils.h"
#include "timer.h"
#include "gorge.h"
#include "fs.h"
#include "libtpw_c/include/controller.h"
#include "libtpw_c/include/tp.h"
#include "libtpw_c/include/system.h"

bool reload_area_flag = false;
bool timer_started = false;
bool reset_timer = false;

namespace Commands {
    static float saved_x = 0.0f;
    static float saved_y = 0.0f;
    static float saved_z = 0.0f;
    static uint16_t saved_angle = 0;
    static float saved_c0 = 0.0f;
    static float saved_c1 = 0.0f;
    static float saved_c2 = 0.0f;
    static float saved_c3 = 0.0f;
    static float saved_c4 = 0.0f;
    static float saved_c5 = 0.0f;
    static float saved_c6 = 0.0f;
    static float saved_c7 = 0.0f;
    static int button_last_frame;
    static int button_this_frame;

    void store_position() {
        saved_x = tp_zelAudio.link_debug_ptr->position.x;
        saved_y = tp_zelAudio.link_debug_ptr->position.y;
        saved_z = tp_zelAudio.link_debug_ptr->position.z;
        saved_angle = tp_zelAudio.link_debug_ptr->facing;
        saved_c0 = tp_matrixInfo.matrix_info->camera0;
        saved_c1 = tp_matrixInfo.matrix_info->camera1;
        saved_c2 = tp_matrixInfo.matrix_info->camera2;
        saved_c3 = tp_matrixInfo.matrix_info->camera3;
        saved_c4 = tp_matrixInfo.matrix_info->camera4;
        saved_c5 = tp_matrixInfo.matrix_info->camera5;
        saved_c6 = tp_matrixInfo.matrix_info->camera6;
        saved_c7 = tp_matrixInfo.matrix_info->camera7;
    }

    void load_position() {
        tp_zelAudio.link_debug_ptr->position.x = saved_x;
        tp_zelAudio.link_debug_ptr->position.y = saved_y;
        tp_zelAudio.link_debug_ptr->position.z = saved_z;
        tp_zelAudio.link_debug_ptr->facing = saved_angle;
        tp_matrixInfo.matrix_info->camera0 = saved_c0;
        tp_matrixInfo.matrix_info->camera1 = saved_c1;
        tp_matrixInfo.matrix_info->camera2 = saved_c2;
        tp_matrixInfo.matrix_info->camera3 = saved_c3;
        tp_matrixInfo.matrix_info->camera4 = saved_c4;
        tp_matrixInfo.matrix_info->camera5 = saved_c5;
        tp_matrixInfo.matrix_info->camera6 = saved_c6;
        tp_matrixInfo.matrix_info->camera7 = saved_c7;
    }

    void moon_jump() {
        if (tp_gameInfo.momentum_ptr) {
            tp_gameInfo.momentum_ptr->link_momentum.y = 56.0f;
        };
    };

    void toggle_timer() {
        if (button_this_frame == 0x6C00 && button_last_frame != 0x6C00) {
            timer_started = !timer_started;
        };
    }

    void hit_reset() {
        reset_timer = true;
    }

    void reload_area() {
        inject_save_flag = true;
        if (g_area_reload_behavior == LOAD_AREA) {
            tp_gameInfo.spawn_speed = 0.0f;
            tp_gameInfo.loading_animation = 13;  // instant load
            tp_gameInfo.respawn_animation = 0;
            tp_gameInfo.warp.entrance.void_flag = 0;
            practice_file.inject_options_before_load = nullptr;
            practice_file.inject_options_during_load = nullptr;
            practice_file.inject_options_after_load = nullptr;
        }
    }  // namespace Commands

    void gorge_void() {
        if (button_this_frame == 0x6A00 && button_last_frame != 0x6A00) {
            loadFile("tpgzw/save_files/any/gorge_void.bin");
            practice_file.inject_options_before_load = SaveInjector::inject_default_before;
            practice_file.inject_options_during_load = GorgeVoidIndicator::warp_to_gorge;
            practice_file.inject_options_after_load = GorgeVoidIndicator::prep_rupee_roll;
            inject_save_flag = true;
        }
    }

    struct Command {
        bool active;
        uint16_t buttons;
        void (*command)();
    };

    static Command Commands[7] = {
        {false, (Controller::Mote::A | Controller::Mote::Z | Controller::Mote::MINUS), store_position},
        {false, (Controller::Mote::A | Controller::Mote::Z | Controller::Mote::TWO), load_position},
        {false, (Controller::Mote::A | Controller::Mote::Z | Controller::Mote::C), moon_jump},
        {false, 0x6410, reload_area},
        {false, 0x6C00, toggle_timer},
        {false, 0x6410, hit_reset},
        {false, 0x6A00, gorge_void}};

    void process_inputs() {
        button_this_frame = tp_mPad.buttons;
        for (auto c : Commands) {
            if (c.active == true && tp_mPad.buttons == c.buttons) {
                c.command();
                Controller::set_buttons_down(0x0);
                Controller::set_buttons_pressed(0x0);
            };
        };
        button_last_frame = button_this_frame;
    };

    void enable_command(int idx) {
        Commands[idx].active = true;
    }

    void disable_command(int idx) {
        Commands[idx].active = false;
    }

}  // namespace Commands