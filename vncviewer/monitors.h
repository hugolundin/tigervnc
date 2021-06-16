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

#ifndef __MONITORS_H__
#define __MONITORS_H__

typedef struct {
    int x;
    int y;
    int w;
    int h;
    int index;
    bool selected;
    int fltk_index;
} Monitor;

int get_monitors(Monitor *, int);
int get_selected_monitors(Monitor *, int);
int get_selected_monitors_count();
void get_primary_screen_dimensions(int&, int&, int&, int&);
bool full_screen_monitors_selected();
bool full_screen_all_monitors_selected();
void get_full_screen_dimensions(int&, int&, int&, int&);

#endif
