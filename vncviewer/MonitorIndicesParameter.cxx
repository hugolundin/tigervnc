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
#include <set>

#include <FL/Fl.H>
#include <rfb/LogWriter.h>

#include "MonitorIndicesParameter.h"

using namespace rfb;
static LogWriter vlog("MonitorIndicesParameter");

MonitorIndicesParameter::MonitorIndicesParameter(const char* name_, const char* desc_, const char* v)
: StringParameter(name_, desc_, v) {}

std::set<int> MonitorIndicesParameter::getParam()
{
    int value = 0;
    int count = 0;
    std::set<int> indices;
    std::set<int> config_indices;
    std::vector<MonitorIndicesParameter::Monitor> monitors = this->monitors();

    if (monitors.size() <= 0) {
        vlog.error("No monitors found.");
        return indices;
    }

    // sscanf will modify the parsed string. Therefore
    // we make a copy of it before parsing.
    const char* config = getValueStr();

    while (*config) {
        if (1 == sscanf(config, "%d%n", &value, &count)) {

            // Config indices start from 1, but we use them to access
            // elements in the monitors array which is zero-indexed. 
            config_indices.insert(value-1);
        }

        config += 1;

        // Scan until we find a new number.
        for (; *config; config++) {
            if (*config >= '0' && *config <= '9') {
                break;
            }
        }
    }

    if (config_indices.size() <= 0) {
        vlog.debug("No indices parsed.");
        return indices;
    }

    // Go through the monitors and see what indices are present in the config.
    for (int i = 0; i < ((int) monitors.size()); i++) {
        if (std::find(config_indices.begin(), config_indices.end(), i) != config_indices.end())
            indices.insert(monitors[i].fltk_index);
    }

    return indices;
}


bool MonitorIndicesParameter::setParam(std::set<int> indices)
{
    static const int BUF_MAX_LEN = 1024;
    char buf[BUF_MAX_LEN] = {0};
    std::set<int> config_indices;
    std::vector<MonitorIndicesParameter::Monitor> monitors = this->monitors();

    if (monitors.size() <=  0) {
        vlog.error("No monitors found.");
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

    return StringParameter::setParam(buf);
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
