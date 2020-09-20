#include <stdio.h>
#include "gorge.h"
#include <string.h>
#include "libtpw_c/include/tp.h"
#include "libtpw_c/include/system.h"
#include "libtpw_c/include/controller.h"
#include "fifo_queue.h"
#include "controller.h"
#include "log.h"
#include "fs.h"
#define WARP_CS_FRAMES 132

bool inject_gorge_flag = false;

bool is_loading_gorge = false;
uint32_t loading_timer = 0;

// FIXME Get all the value from the Wii GV.
namespace GorgeVoidIndicator {
    using namespace Controller;

    static bool start_timer = false;
    uint32_t previous_counter = 0;
    uint32_t current_counter = 0;
    uint32_t counter_difference = 0;
    static int after_cs_val = 0;
    static bool got_it = false;
    static char buf[20];

    void prep_rupee_roll() {
        Log log;
        //log.PrintLog("Saving temp flag #14 to 0x20", DEBUG);
        //tp_gameInfo.temp_flags.temp_flag_bit_field_14 = 0x20;
        //log.PrintLog("Setting cs_val to 0x900", DEBUG);
        //tp_gameInfo.cs_val = 0x900;
        is_loading_gorge = true;
        inject_gorge_flag = false;
    }

    void warp_to_gorge() { //HACKY CHANGE FOR BitE ROUTE GORGE VOID
        Log log;
        
        // set gorge map info
        tp_gameInfo.warps.kak_gorge_warp = 0;
        tp_gameInfo.temp_flags.temp_flag_bit_field_13 = 0;
        tp_gameInfo.temp_flags.temp_flag_bit_field_14 = 0;
        tp_gameInfo.warps.kak_gorge_unk = 0;

        // change form to human
        tp_gameInfo.link.is_wolf = false;

        // set loading info
        tp_gameInfo.warp.entrance.void_flag = 0;
        tp_gameInfo.event_to_play = 0;
        tp_gameInfo.respawn_animation = 0;
        tp_gameInfo.warp.entrance.spawn = 0;
        tp_gameInfo.warp.entrance.room = 6;
        tp_gameInfo.warp.entrance.state = 0xFF;
        strcpy((char *)tp_gameInfo.warp.entrance.stage, "F_SP121");

        // reset health, item
        tp_gameInfo.respawn_item_id = 40;
        tp_gameInfo.link.heart_quarters = 12;  // 3 hearts

        // trigger loading, convert some of these to const later
        log.PrintLog("Setting respawn id to 2", DEBUG);
        tp_gameInfo.special_spawn_id = 2;
        log.PrintLog("Setting respawn position: {-11856.857f, -5700.0f, 56661.5}", DEBUG);
        tp_gameInfo.respawn_position = {-11856.857f, -5700.0f, 56661.5};
        log.PrintLog("Setting respawn angle: 24169", DEBUG);
        tp_gameInfo.respawn_angle = 24169;
        log.PrintLog("Clearing rupee flags", DEBUG);
        Inventory::clear_rupee_flags();

        
    }
    void run() {
        Log log;
        
        // reset counters on load
        if (tp_fopScnRq.isLoading == 1) {
            counter_difference = 0;
            after_cs_val = 0;
            got_it = false;
            start_timer = false;
        }

        current_counter = TP::get_frame_count();
        
        // situation specific frame counters
        if (start_timer == false && tp_gameInfo.freeze_game == 1 && tp_gameInfo.cs_val == 0x128 && strcmp((const char *)tp_gameInfo.current_stage, "F_SP121") == 0) {
            start_timer = true;
            previous_counter = current_counter;
            counter_difference = 0;
        }

        if (is_loading_gorge){ //Loads the rooms required to get glitched portal graphics
            loading_timer = loading_timer + 1;
            if (loading_timer==30){
                tp_zelAudio.link_debug_ptr->position = {-46261.4805f, -7473.1777f, 98200.4141f};
            }
            else if (loading_timer==60){
                tp_zelAudio.link_debug_ptr->position = {-24044.8496f, -7608.4185f, 77374.0391f};
            }
            else if (loading_timer==90){
                tp_zelAudio.link_debug_ptr->position = {-17588.9492f, -6491.45557f, 68698.1016f};
            }
            else if (loading_timer==120){
                tp_zelAudio.link_debug_ptr->position = {-10030.374f, -7200.0f, 58834.3828f};
                tp_gameInfo.temp_flags.temp_flag_bit_field_14 = 0x20;
                tp_gameInfo.cs_val = 0x900;
                is_loading_gorge = false;
                loading_timer = 0;
            }
        }

        if (start_timer == true) {
            counter_difference += current_counter - previous_counter;
            previous_counter = current_counter;

            if (counter_difference > WARP_CS_FRAMES) {
                after_cs_val = counter_difference - WARP_CS_FRAMES;
            }

            sprintf(buf, "counter: %d", counter_difference);
            log.PrintLog(buf, DEBUG);
            sprintf(buf, "inputs: %d", tp_mPadStatus.sval);
            log.PrintLog(buf, DEBUG);

            // only care about 10f before and after
            if (counter_difference > 123 && after_cs_val < 10) {
                // went early
                if (!got_it && !(button_is_held(Z) && button_is_held(A)) && (counter_difference < WARP_CS_FRAMES) &&
                    (button_is_down(A) && button_is_down(Z))) {
                    int final_val = WARP_CS_FRAMES - counter_difference;
                    sprintf(buf, "%df early", final_val);
                    FIFOQueue::push(buf, Queue);
                }

                // got it
                else if (!got_it && !(button_is_held(Z) && button_is_held(A)) && (counter_difference == WARP_CS_FRAMES) &&
                         (button_is_down(A) && button_is_down(Z))) {
                    FIFOQueue::push("got it", Queue);
                    got_it = true;
                }

                // went late
                else if (!got_it && !(button_is_held(Z) && button_is_held(A)) && after_cs_val > 0 &&
                         (button_is_down(A) && button_is_down(Z))) {
                    sprintf(buf, "%df late", after_cs_val);
                    FIFOQueue::push(buf, Queue);
                }
            }
        }
    }
}  // namespace GorgeVoidIndicator