/* Copyright 2021 Hugo Lundin <huglu@cendio.se> for Cendio AB.
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

#ifdef HAVE_XRANDR
#include <X11/extensions/Xrandr.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <set>
#include <map>

#include <rfb/LogWriter.h>
#include <rfb/CMsgWriter.h>

#include "Monitors.h"

#include <FL/Fl.H>
#include <FL/x.H>

using namespace rfb;

static rfb::LogWriter vlog("Monitors");

Monitors::Monitors():
    m_top(-1), m_bottom(-1), m_left(-1), m_right(-1),
    m_top_y(-1), m_bottom_y(-1), m_left_x(-1), m_right_x(-1),
    m_mode(CURRENT), m_monitors(), m_indices()
{
    load_monitors();
    calculate_dimensions();
}

Monitors::~Monitors()
{

}

int Monitors::count() const
{
    return m_monitors.size();
}

void Monitors::set_indices(char const *config)
{
    m_indices.clear();
    int value = 0;
    int count = 0;

    while (*config) {
        if (1 == sscanf(config, "%d%n", &value, &count)) {
            m_indices.insert(value-1);
        }

        config += count;

        // Scan until we find a new number.
        for (; *config; config++) {
            if (*config >= '0' && *config <= '9') {
                break;
            }
        }
    }

    calculate_dimensions();
}

void Monitors::set_mode(char const *config)
{
    if (!strcmp(config, "All")) {
        m_mode = ALL;
    } else if (!strcmp(config, "Selected")) {
        m_mode = SELECTED;
    } else {
        m_mode = CURRENT;
    }

    calculate_dimensions();
}

void Monitors::save(char * buf, int buf_len)
{
    char const * separator = "";
    int bytes_written = 0;

    for (std::set<unsigned int>::iterator index = m_indices.begin();
         index != m_indices.end();
         index++)
    {
        bytes_written += snprintf(
            buf+bytes_written,
            buf_len-bytes_written,
            "%s%u",
            separator,
            (*index)+1
        );

        separator = ",";
    }
}

bool Monitors::has_required() const
{
    for (int i = 0; i < count(); i++) {
        if (is_required(i)) {
            return true;
        }
    }

    return false;
}

void Monitors::dimensions(int& x, int& y, int& w, int& h) const
{
    if (multiple_monitors()) {
        x = m_left_x;
        y = m_top_y;
        w = m_right_x - m_left_x;
        h = m_bottom_y - m_top_y;
    } else {
        x = y = w = h = 0;
    }
}

void Monitors::toggle(unsigned int monitor)
{
    set(monitor, m_indices.find(monitor) == m_indices.end());
}

void Monitors::set(unsigned int monitor, bool select)
{
    if (select) {
        m_indices.insert(monitor);
    } else {
        m_indices.erase(monitor);
    }

    calculate_dimensions();
}

bool Monitors::is_selected(unsigned int monitor) const
{
    assert(monitor >= 0);
    assert(monitor < m_monitors.size());

    if (m_mode == ALL) {
        return true;
    }

    for (std::set<unsigned int>::iterator index = m_indices.begin();
         index != m_indices.end();
         index++)
    {
        if (*index == monitor) {
            return true;
        }
    }

    return false;
}

bool Monitors::is_required(unsigned monitor) const
{
    int x, y, w, h;
    assert(monitor >= 0);
    assert(monitor < m_monitors.size());

    // Selected monitors are never required. 
    if (is_selected(monitor)) {
        return false;
    }

    dimensions(x, y, w, h, monitor);
    return inside(x, y) || inside(x+w, y) || inside(x, y+h) || inside(x+w, y+h);
}

char const * Monitors::description(unsigned int monitor) const
{
    return m_monitors[monitor].description;
}

void Monitors::dimensions(int& x, int& y, int& w, int& h, unsigned int monitor) const
{
    assert(monitor >= 0);
    assert(monitor < m_monitors.size());

    x = m_monitors[monitor].x;
    y = m_monitors[monitor].y;
    w = m_monitors[monitor].w;
    h = m_monitors[monitor].h;
}

int Monitors::primary() const
{
    // On macOS, Windows and GNOME the monitor with FLTK index 0
    // is the primary monitor (containing a toolbar). If that one
    // is part of the current configuration, we return it.
    for (int monitor = 0; monitor < count(); monitor++) {
        if (is_selected(monitor) || is_required(monitor)) {
            if (m_monitors[monitor].fltk_index == 0) {
                vlog.debug("%d (%d) is primary monitor", monitor+1, 0);
                return 0;
            }
        }
    }

    // If the primary isn't part of the configuration, we take the lowest
    // index of our own mapping (the leftmost monitor).
    for (int monitor = 0; monitor < count(); monitor++) {
        if (is_selected(monitor) || is_required(monitor)) {
            vlog.debug("%d (%d) is primary monitor", monitor+1, m_monitors[monitor].fltk_index);
            return m_monitors[monitor].fltk_index;
        }
    }

    vlog.debug("No primary monitor found ");
    return -1;
}

int Monitors::top() const
{
    return multiple_monitors() ? m_top : -1;
}

int Monitors::left() const
{
    return multiple_monitors() ? m_left : -1;
}

int Monitors::right() const
{
    return multiple_monitors() ? m_right : -1;
}

int Monitors::bottom() const
{
    return multiple_monitors() ? m_bottom : -1;
}

int Monitors::width() const
{
    int w = 0;
    int result = 0;

    for (int i = 0; i < count(); i++) {
        w = m_monitors[i].w + m_monitors[i].x;
        if (w > result) {
            result = w;
        }
    }

    return result;
}

int Monitors::height() const
{
    int h = 0;
    int result = 0;

    for (int i = 0; i < count(); i++) {
        h = m_monitors[i].h + m_monitors[i].y;
        if (h > result) {
            result = h;
        }
    }

    return result;
}

void Monitors::load_monitors()
{
    // Start by creating a struct for every monitor.
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
        m_monitors.push_back(monitor);
    }

    // No monitors found. 
    if (m_monitors.size() <= 0) {
        return;
    }

    // Sort the monitors according to the specification in the vncviewer manual. 
    qsort(&m_monitors[0], m_monitors.size(), sizeof(*(&m_monitors[0])), sort_cb);

    #if !defined(WIN32) && !defined(__APPLE__)
    #ifdef HAVE_XRANDR

    int ev, err, xi_major;
    fl_open_display();
    assert(fl_display != NULL);

    if (!XQueryExtension(fl_display, "RANDR", &xi_major, &ev, &err)) {
        vlog.info("X11 RANDR extension not available.");
        return;
    }

    XRRScreenResources *res = XRRGetScreenResources(fl_display, DefaultRootWindow(fl_display));
    if (!res) {
        vlog.error("Unable to get XRRScreenResources for fl_display.");
        return;
    }

    for (int i = 0; i < count(); i++) {
        for (int j = 0; j < res->ncrtc; j++) {
            XRRCrtcInfo *crtc = XRRGetCrtcInfo(fl_display, res, res->crtcs[j]);
            if (!crtc) {
                vlog.error("Unable to get XRRCrtcInfo for crtc %d.", j);
                continue;
            }

            for (int k = 0; k < crtc->noutput; k++) {
                bool monitor_found = (crtc->x == m_monitors[i].x) &&
                    (crtc->y == m_monitors[i].y) &&
                    (crtc->width == ((unsigned int) m_monitors[i].w)) &&
                    (crtc->height == ((unsigned int) m_monitors[i].h));

                if (monitor_found) {
                    XRROutputInfo *output = XRRGetOutputInfo(fl_display, res, crtc->outputs[k]);
                    if (!output) {
                        vlog.error("Unable to get XRROutputInfo for crtc %d, output %d", j, k);
                        continue;
                    }

                    snprintf(
                        m_monitors[i].description,
                        DESCRIPTION_MAX_LEN,
                        "%s (%dx%d+%d+%d)",
                        output->name,
                        m_monitors[i].w,
                        m_monitors[i].h,
                        m_monitors[i].x,
                        m_monitors[i].y
                    );
                    XRRFreeOutputInfo(output);
                }
            }

            XRRFreeCrtcInfo(crtc);
        }
    }
    #endif
    #elif defined(WIN32)
    // TODO: Get name from WIN32 APIs. 
    #elif defined(__APPLE__)
    // TODO: Get name from Apple APIs. 
    #endif
}

void Monitors::calculate_dimensions()
{
    std::vector<Monitor> selected;

    // Filter out the monitors which has been explicitly selected.
    for (std::vector<Monitor>::size_type monitor = 0;
         monitor < m_monitors.size();
         monitor++)
    {   
        if (m_mode == ALL || is_selected(monitor)) {
            selected.push_back(m_monitors[monitor]);
        }
    }

    // No monitors have been selected. 
    if (selected.size() == 0) {
        return;
    }

    // Calculate the dimensions and which fltk indices that
    // limits the frame buffer.
    std::vector<Monitor>::iterator monitor = selected.begin();
    
    // Initially we limit the area to the first selected monitor.
    m_top = m_bottom = m_left = m_right = monitor->fltk_index;
    m_top_y = monitor->y;
    m_bottom_y = monitor->y + monitor->h;
    m_left_x = monitor->x;
    m_right_x = monitor->x + monitor->w;

    // Exhaust the iterator.
    for (; monitor != selected.end(); monitor++) {
        if (monitor->y < m_top_y) {
            m_top = monitor->fltk_index;
            m_top_y = monitor->y;
        }

        if ((monitor->y + monitor->h) > m_bottom_y) {
            m_bottom = monitor->fltk_index;
            m_bottom_y = monitor->y + monitor->h;
        }

        if (monitor->x < m_left_x) {
            m_left = monitor->fltk_index;
            m_left_x = monitor->x;
        }

        if ((monitor->x + monitor->w) > m_right_x) {
            m_right = monitor->fltk_index;
            m_right_x = monitor->x + monitor->w;
        }
    }
}

bool Monitors::inside(int x, int y) const
{
    return (x > m_left_x) && (x < m_right_x) && (y > m_top_y) && (y < m_bottom_y);
}

bool Monitors::multiple_monitors() const
{
    return m_mode != CURRENT;
}

int Monitors::sort_cb(const void *a, const void *b)
{
    Monitors::Monitor * monitor1 = (Monitors::Monitor *) a;
    Monitors::Monitor * monitor2 = (Monitors::Monitor *) b;

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
