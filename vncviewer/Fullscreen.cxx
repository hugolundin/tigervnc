/* Copyright 2021 Hugo Lundin for Cendio AB
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "Fullscreen.h"
#include "parameters.h"

#include <FL/Fl.H>

#define SELECTED_MONITORS_MAX_LEN 31
#define SELECTED_MONITORS_ALL_STR "all"

bool Fullscreen::selectedMonitorsEnabled()
{
    return strnlen(fullScreenSelectedMonitors, SELECTED_MONITORS_MAX_LEN) > 0;
}

bool Fullscreen::selectedMonitorsAllEnabled()
{
    return !strncmp(
        fullScreenSelectedMonitors.getValueStr(),
        SELECTED_MONITORS_ALL_STR,
        sizeof(SELECTED_MONITORS_ALL_STR)
    );
}

static int cmp(const void * a, const void * b)
{
    int * s1 = (int *) a;
    int * s2 = (int *) b;

    if (s1[0] < s2[0]) {
        return -1;
    }

    if (s1[0] == s2[0]) {
        if (s1[1] < s2[1]) {
            return -1;
        }

        if (s1[1] == s2[1]) {
            return 0;
        }
    }

    return 1;
}

void Fullscreen::get_dimensions(int& top, int& bottom, int& left, int& right)
{
    int fl_monitor_ids_len = 0;
    int fl_monitor_ids[16] = {0};

    if (Fullscreen::selectedMonitorsEnabled()) {

        if (selectedMonitorsAllEnabled()) {
        fl_monitor_ids_len = Fl::screen_count();

        for (int i = 0; i < fl_monitor_ids_len; i++) {
            fl_monitor_ids[i] = i;
        }
        } else {
            // -- Get identifiers for all displays that we want to use.
            // TODO: Use FLTK's max display constant here. 
            int monitor_ids_len = 0;
            int monitor_ids[16] = {0};

            // Because sscanf will modifies the string it is parsing, we start by making
            // a copy of the fullScreenSelectedMonitors configuration.
            const char * selected_monitors = fullScreenSelectedMonitors.getValueStr();

            // Parse what monitors to use in full screen.
            for (monitor_ids_len = 0; monitor_ids_len < 16; monitor_ids_len++) {
            int n;
            int count = sscanf(selected_monitors, "%d,%n", &monitor_ids[monitor_ids_len], &n);
            if (count != 1) {
                break;
            }

            selected_monitors += n;
            }

            for (int i = 0; i < monitor_ids_len; i++) {
            printf("Monitor %d\n", monitor_ids[i]);
            }

            // -- Get FLTK displays and create an array with them in the order we expect.
            int fl_monitors_len = 0;
            int fl_monitors[16][5] = {{0}};

            for (fl_monitors_len = 0; fl_monitors_len < Fl::screen_count(); fl_monitors_len++) {
            Fl::screen_xywh(
                fl_monitors[fl_monitors_len][0],
                fl_monitors[fl_monitors_len][1],
                fl_monitors[fl_monitors_len][2],
                fl_monitors[fl_monitors_len][3],
                fl_monitors_len
            );

            fl_monitors[fl_monitors_len][4] = fl_monitors_len;
            }

            qsort(fl_monitors, fl_monitors_len, sizeof(*fl_monitors), cmp);
        
            fl_monitor_ids_len = monitor_ids_len;
            
            for (int i = 0; i < fl_monitor_ids_len; i++) {
            fl_monitor_ids[i] = fl_monitors[monitor_ids[i] - 1][4];
            }
        }

        int top_y, bottom_y, left_x, right_x;

        int sx, sy, sw, sh;

        top = bottom = left = right = fl_monitor_ids[0];

        Fl::screen_xywh(sx, sy, sw, sh, fl_monitor_ids[0]);
        top_y = sy;
        bottom_y = sy + sh;
        left_x = sx;
        right_x = sx + sw;

        for (int i = 1; i <= fl_monitor_ids_len; i++) {
            Fl::screen_xywh(sx, sy, sw, sh, fl_monitor_ids[i]);

            if (sy < top_y) {
                top = fl_monitor_ids[i];
                top_y = sy;
            }
            if ((sy + sh) > bottom_y) {
                bottom = fl_monitor_ids[i];
                bottom_y = sy + sh;
            }
            if (sx < left_x) {
                left = fl_monitor_ids[i];
                left_x = sx;
            }
            if ((sx + sw) > right_x) {
                right = fl_monitor_ids[i];
                right_x = sx + sw;
            }
        }
    } else {
        top = bottom = left = right = -1;
  }
}
