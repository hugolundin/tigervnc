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

#ifdef HAVE_XRANDR
#include <X11/extensions/Xrandr.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <set>

#include "monitors.h"
#include "parameters.h"

#include <FL/Fl.H>
#include <FL/x.H>

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
int coordinates_sort_cb(const void * a, const void * b)
{
    Monitor * monitor1 = (Monitor *) a;
    Monitor * monitor2 = (Monitor *) b;

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

void load_selected_indices(std::set<int>& indices)
{   
    // Because sscanf modifies the string it parses, we want to 
    // make a copy before using it. 
    const char * config = fullScreenSelectedMonitors.getValueStr();

    int value = 0;
    int count = 0;

    while (*config) {
        if (1 == sscanf(config, "%d%n", &value, &count)) {
            indices.insert(value);
        }

        config += count;

        // Scan until we find a new number.
        for (; *config; config++) {
            if (*config >= '0' && *config <= '9') {
                break;
            }
        }
    }
}



void load_monitors(std::vector<Monitor>& monitors)
{
    std::set<int> indices;
    load_selected_indices(indices);

    for (int i = 0; i < Fl::screen_count(); i++) {
        Monitor monitor = {0};

        // Get the properties of the monitor at the current index;
        Fl::screen_xywh(
            monitor.x,
            monitor.y,
            monitor.w,
            monitor.h,
            i
        );

        monitor.fltk_index = i;
        monitor.selected = fullScreenAllMonitors ? true : false;
        monitors.push_back(monitor);
    }

    // Sort the monitors such that indices from the config file corresponds
    // to the layout of the monitors.
    qsort(&monitors[0], monitors.size(), sizeof(*(&monitors[0])), coordinates_sort_cb);

    for (std::vector<Monitor>::size_type i = 0; i < monitors.size(); i++) {
        
        // Configuration indices start at 1. 
        monitors[i].index = i + 1;

        if (indices.find(monitors[i].index) != indices.end()) {
            monitors[i].selected = true;
        }

        #if !defined(WIN32) && !defined(__APPLE__)
            #ifdef HAVE_XRANDR
                int ev, err, xi_major;
                fl_open_display();
                assert(fl_display != NULL);

                if (!XQueryExtension(fl_display, "RANDR", &xi_major, &ev, &err)) {
                    // TODO: Log a message.
                    return;
                }
               
                XRRScreenResources *res = XRRGetScreenResources(fl_display, DefaultRootWindow(fl_display));
                if (!res) {
                    // TODO: Log a message.
                    return;
                }

                for (int j = 0; j < res->ncrtc; j++) {
                    XRRCrtcInfo *crtc = XRRGetCrtcInfo(fl_display, res, res->crtcs[j]);
                    if (!crtc) {
                        // TODO: Log a message.
                        continue;
                    }

                    for (int k = 0; k < crtc->noutput; k++) {
                        bool monitor_found = (crtc->x == monitors[i].x) &&
                            (crtc->y == monitors[i].y) &&
                            (crtc->width == ((unsigned int) monitors[i].w)) &&
                            (crtc->height == ((unsigned int) monitors[i].h));

                        if (monitor_found) {
                            XRROutputInfo *output = XRRGetOutputInfo(fl_display, res, crtc->outputs[k]);
                            if (!output) {
                                // TODO: Log a message. 
                                continue;
                            }

                            strncpy(monitors[i].name, output->name, MONITOR_NAME_MAX_LEN);
                            XRRFreeOutputInfo(output);
                        }
                    }

                    XRRFreeCrtcInfo(crtc);
                }
            #endif
        #elif defined(WIN32)
            // TODO: Get name from WIN32 APIs. 
        #elif defined(__APPLE__)
            // TODO: Get name from Apple APIs. 
        #endif
    }
}

void load_selected_monitors(std::vector<Monitor>& monitors)
{
    std::vector<Monitor> all_monitors;
    load_monitors(all_monitors);

    for (
        std::vector<Monitor>::iterator monitor = all_monitors.begin();
        monitor != all_monitors.end();
        monitor++
    ) {
        if (monitor->selected) {
            monitors.push_back(*monitor);
        }
    }
}

void get_full_screen_dimensions(int& top, int& bottom, int& left, int& right)
{
    std::vector<Monitor> monitors;
    load_selected_monitors(monitors);

    if (monitors.size() <= 0) {
        top = bottom = left = right = -1;
        return;
    }

    int top_y, bottom_y, left_x, right_x;
    std::vector<Monitor>::iterator monitor = monitors.begin();

    top = bottom = left = right = monitor->fltk_index;
    top_y = monitor->y;
    bottom_y = monitor->y + monitor->h;
    left_x = monitor->x;
    right_x = monitor->x + monitor->w;

    for (; monitor != monitors.end(); monitor++) {
        if (monitor->y < top_y) {
            top = monitor->fltk_index;
            top_y = monitor->y;
        }

        if ((monitor->y + monitor->h) > bottom_y) {
            bottom = monitor->fltk_index;
            bottom_y = monitor->y + monitor->h;
        }

        if (monitor->x < left_x) {
            left = monitor->fltk_index;
            left_x = monitor->x;
        }

        if ((monitor->x + monitor->w) > right_x) {
            right = monitor->fltk_index;
            right_x = monitor->x + monitor->w;
        }
    }
}
