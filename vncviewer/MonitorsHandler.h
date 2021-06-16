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

#ifndef __MONITORS_HANDLER_H__
#define __MONITORS_HANDLER_H__

class MonitorsHandler
{
public:
    struct Monitor {
        int x;
        int y;
        int w;
        int h;
        int index;
        bool selected;
        int fltk_index;
    };

    static int get_monitors(struct Monitor *, int);
    static int get_selected_monitors(struct Monitor *, int);
    static int get_selected_monitors_count();
    static void primary_screen_dimensions(int&, int&, int&, int&);
    static bool full_screen_selected_monitors();
    static bool full_screen_selected_monitors_all();
    static void full_screen_dimensions(int&, int&, int&, int&);
private:
    static int parse_selected_indices(int *, int);
    MonitorsHandler();
};

#endif
