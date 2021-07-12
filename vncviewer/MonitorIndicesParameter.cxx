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

#include <algorithm>
#include <vector>
#include <string>
#include <limits>
#include <set>
#include <stdlib.h>
#include <stdexcept>

#include "i18n.h"
#include <FL/Fl.H>
#include <rfb/LogWriter.h>

#include "MonitorIndicesParameter.h"

using namespace rfb;
static LogWriter vlog("MonitorIndicesParameter");

MonitorIndicesParameter::MonitorIndicesParameter(const char* name_, const char* desc_, const char* v)
: StringParameter(name_, desc_, v) {}

std::set<int> MonitorIndicesParameter::getParam()
{
    bool valid = false;
    std::set<int> indices;
    std::set<int> config_indices;
    std::vector<MonitorIndicesParameter::Monitor> monitors = this->monitors();

    if (monitors.size() <= 0) {
        vlog.error(_("Failed to get monitors."));
        return indices;
    }

    valid = parse_indices(value, &config_indices);
    if (!valid) {
        return indices;
    }

    if (config_indices.size() <= 0) {
        return indices;
    }

    // Go through the monitors and see what indices are present in the config.
    for (int i = 0; i < ((int) monitors.size()); i++) {
        if (std::find(config_indices.begin(), config_indices.end(), i) != config_indices.end())
            indices.insert(monitors[i].fltk_index);
    }

    return indices;
}

bool MonitorIndicesParameter::setParam(const char* value)
{
    int index;
    std::set<int> indices;

    if (strlen(value) <= 0)
        return false;

    if (!parse_indices(value, &indices)) {
        vlog.error(_("Parsing failed for FullScreenSelectedMonitors."));
        return false;
    }

    for (std::set<int>::iterator it = indices.begin(); it != indices.end(); it++) {
        index = *it + 1;

        if (index <= 0 || index > Fl::screen_count())
            vlog.error(_("Monitor index %d does not exist."), index);
    }

    return StringParameter::setParam(value);
}

bool MonitorIndicesParameter::setParam(std::set<int> indices)
{
    static const int BUF_MAX_LEN = 1024;
    char buf[BUF_MAX_LEN] = {0};
    std::set<int> config_indices;
    std::vector<MonitorIndicesParameter::Monitor> monitors = this->monitors();

    if (monitors.size() <=  0) {
        vlog.error(_("Failed to get monitors."));
        // Don't return, store the configuration anyways.
    }

    for (int i = 0; i < ((int) monitors.size()); i++) {
        if (std::find(indices.begin(), indices.end(), monitors[i].fltk_index) != indices.end())
            config_indices.insert(i);
    }

    int bytes_written = 0;
    char const * separator = "";

    for (std::set<int>::iterator index = config_indices.begin();
         index != config_indices.end();
         index++)
    {
        bytes_written += snprintf(
            buf+bytes_written,
            BUF_MAX_LEN-bytes_written,
            "%s%u",
            separator,
            (*index)+1
        );

        separator = ",";
    }

    return setParam(buf);
}

static bool parse_number(std::string number, std::set<int> *indices)
{
    if (number.size() <= 0)
        return false;

    int v = strtol(number.c_str(), NULL, 0);

    if (v < 0 || v > INT_MAX) {
        vlog.error(_("The given monitor index (%s) is too large to be valid."), number.c_str());
        return false;
    }

    indices->insert(v-1);
    return true;
}

bool MonitorIndicesParameter::parse_indices(const char* value, std::set<int> *indices)
{
    char d;
    std::string current;

    for (size_t i = 0; i < strlen(value); i++) {
        d = value[i];

        if (d == ' ')
            continue;
        else if (d >= '0' && d <= '9')
            current.push_back(d);
        else if (d == ',') {
            if (!parse_number(current, indices))
                return false;

            current.clear();
        } else
            return false;
    }

    if (!parse_number(current, indices))
        return false;

    return true;
}

std::vector<MonitorIndicesParameter::Monitor> MonitorIndicesParameter::monitors()
{
    std::vector<Monitor> monitors;

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
        monitors.push_back(monitor);
    }

    // Sort the monitors according to the specification in the vncviewer manual. 
    qsort(&monitors[0], monitors.size(), sizeof(*(&monitors[0])), sort_cb);
    return monitors;
}

int MonitorIndicesParameter::sort_cb(const void *a, const void *b)
{
    MonitorIndicesParameter::Monitor * monitor1 = (MonitorIndicesParameter::Monitor *) a;
    MonitorIndicesParameter::Monitor * monitor2 = (MonitorIndicesParameter::Monitor *) b;

    if (monitor1->x < monitor2->x)
        return -1;

    if (monitor1->x == monitor2->x) {
        if (monitor1->y < monitor2->y)
            return -1;

        if (monitor1->y == monitor2->y)
            return 0;
    }

    return 1;  
}
