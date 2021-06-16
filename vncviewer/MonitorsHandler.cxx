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

#include "MonitorsHandler.h"
#include "parameters.h"

#include <FL/Fl.H>

// TODO: Find existing define to use.
#define FLTK_MAX_MONITORS 16
#define SELECTED_MONITORS_MAX_LEN 31
#define SELECTED_MONITORS_ALL_STR "all"

// Callback function for `qsort` to sort the monitors. 
// They are sorted after their coordinates: smallest x-coordinate first
// and if there is a conflict, smallest y-coordinate. 
//  
//  Example 1 (left to right):
// 
//  +-------+  +-------+ +-------+
//  |   1   |  |   2   | |   3   |
//  +-------+  +-------+ +-------+  
// 
//  Example 2 (left to right, conflict in the middle where the smaller 
//  y-coordinate takes precedence):
// 
//  +-------+  +-------+ +-------+
//  |   1   |  |   2   | |   3   |
//  +-------+  +-------+ +-------+ 
//             +-------+
//             |   4   |
//             +-------+
//
//  Example 3 (left to right, conflict in the middle where the smaller 
//  y-coordinate takes precedence):
// 
//             +-------+
//             |   2   |
//             +-------+
//  +-------+  +-------+ +-------+
//  |   1   |  |   3   | |   4   |
//  +-------+  +-------+ +-------+
//
int compare_monitors(const void * a, const void * b)
{
    struct MonitorsHandler::Monitor * monitor1 = (struct MonitorsHandler::Monitor *) a;
    struct MonitorsHandler::Monitor * monitor2 = (struct MonitorsHandler::Monitor *) b;

    if (monitor1->x < monitor2->x) {
        return -1;
    }

    if (monitor1->x == monitor2->x) {
        if (monitor1->y < monitor2->y) {
            return -1;
        }

        if (monitor1->y == monitor2->y) {
            return 0;
        }
    }

    return 1;
}

int MonitorsHandler::get_monitors(struct Monitor * monitors, int monitors_len)
{
    int indices[FLTK_MAX_MONITORS] = {0};
    int indices_len = parse_selected_indices(indices, FLTK_MAX_MONITORS);
    bool all_selected = full_screen_selected_monitors_all();

    // TODO: Make sure that monitors_len is big enough. 
    for (int i = 0; i < Fl::screen_count(); i++) {

        // Get the properties of the monitor at the current index;
        Fl::screen_xywh(
            monitors[i].x,
            monitors[i].y,
            monitors[i].width,
            monitors[i].height,
            i
        );

        monitors[i].fltk_index = i;
    }

    // Sort the monitors such that indices from the config file corresponds
    // to the layout of the monitors.
    qsort(monitors, Fl::screen_count(), sizeof(*monitors), compare_monitors);

    for (int i = 0; i < Fl::screen_count(); i++) {
        
        // Configuration indices start at 1. 
        monitors[i].index = i + 1;

        if (all_selected) {
            monitors[i].selected = true;
        } else {
            // Check if the monitor is selected.
            for (int j = 0; j < indices_len; j++) {
                if (i + 1 == indices[j]) {
                    monitors[i].selected = true;
                }
            }
        }
    }

    return Fl::screen_count();
}

int MonitorsHandler::get_selected_monitors(struct Monitor * monitors, int monitors_len)
{
    struct Monitor all_monitors[FLTK_MAX_MONITORS] = {0};
    int all_monitors_len = get_monitors(all_monitors, FLTK_MAX_MONITORS);
    
    int count = 0;
    for (int i = 0; i < all_monitors_len; i++) {
        if (all_monitors[i].selected) {
            monitors[count++] = all_monitors[i];
        }
    }

    return count;
}

bool MonitorsHandler::full_screen_selected_monitors()
{
    return strnlen(fullScreenSelectedMonitors, SELECTED_MONITORS_MAX_LEN) > 0;
}

bool MonitorsHandler::full_screen_selected_monitors_all()
{
    return !strncmp(
        fullScreenSelectedMonitors.getValueStr(),
        SELECTED_MONITORS_ALL_STR,
        sizeof(SELECTED_MONITORS_ALL_STR)
    );
}

void MonitorsHandler::full_screen_dimensions(int& top, int& bottom, int& left, int& right)
{
    assert(full_screen_selected_monitors());

    int monitors_len = 0;
    struct Monitor monitors[FLTK_MAX_MONITORS] = {0};

    monitors_len = get_selected_monitors(monitors, FLTK_MAX_MONITORS);
    if (monitors_len <= 0) {
        top = bottom = left = right = -1;
        return;
    }

    int sx, sy, sw, sh;
    int top_y, bottom_y, left_x, right_x;

    top = bottom = left = right = monitors[0].fltk_index;
    Fl::screen_xywh(sx, sy, sw, sh, monitors[0].fltk_index);
    top_y = sy;
    bottom_y = sy + sh;
    left_x = sx;
    right_x = sx + sw;

    for (int i = 1; i < monitors_len; i++) {
        Fl::screen_xywh(sx, sy, sw, sh, monitors[i].fltk_index);

        if (sy < top_y) {
            top = monitors[i].fltk_index;
            top_y = sy;
        }

        if ((sy + sh) > bottom_y) {
            bottom = monitors[i].fltk_index;
            bottom_y = sy + sh;
        }

        if (sx < left_x) {
            left = monitors[i].fltk_index;
            left_x = sx;
        }

        if ((sx + sw) > right_x) {
            right = monitors[i].fltk_index;
            right_x = sx + sw;
        }
    }
}

int MonitorsHandler::get_selected_monitors_count()
{
    int indices[FLTK_MAX_MONITORS] = {0};
    return parse_selected_indices(indices, FLTK_MAX_MONITORS);
}

int MonitorsHandler::parse_selected_indices(int * indices, int indices_len)
{   
    if (full_screen_selected_monitors_all()) {
        return 0;
    }

    // Because sscanf modifies the string it parses, we want to 
    // make a copy before using it. 
    const char * config = fullScreenSelectedMonitors.getValueStr() + '\0';

    int value = 0;
    int count = 0;
    int parsed_indices_len = 0;

    while (*config) {
        if (1 == sscanf(config, "%d%n", &value, &count)) {
            indices[parsed_indices_len++] = value;
        }

        config += count;

        // Scan until we find a new number.
        for (; *config; config++) {
            if (*config >= '0' && *config <= '9') {
                break;
            }
        }
    }

    return parsed_indices_len;
}
