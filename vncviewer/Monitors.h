/* Copyright 2021 Hugo Lundin <huglu@cendio.se> for Cendio AB
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

#include <vector>
#include <set>
#include <list>
#include <map>

#define DESCRIPTION_MAX_LEN 256

class Monitors {
public:
  Monitors();
  ~Monitors();
  static Monitors& shared();

  void refresh();
  int count();
  bool is_selected(unsigned int);
  bool is_required(unsigned int);
  void dimensions(int&, int&, int&, int&, unsigned int);
  char const * description(unsigned int);
  bool has_required();

  void toggle(unsigned int);
  void set(unsigned int, bool);

  int top();
  int left();
  int right();
  int bottom();

  static void add_callback(void (*)(void*), void *);
  static void remove_callback(void (*)(void *));
  void frame_buffer_dimensions(int&, int&, int&, int&);
  void debug();
  bool fullscreen_multiple_monitors_enabled();

protected:
  static std::map<void (*)(void*), void*> callbacks;

private:
  typedef struct {
    int x, y, w, h;
    int fltk_index;
    char description[DESCRIPTION_MAX_LEN];
  } Monitor;

  int m_top;
  int m_bottom;
  int m_left;
  int m_right;

  int m_top_y;
  int m_bottom_y;
  int m_left_x;
  int m_right_x;

  std::vector<Monitor> m_monitors;
  std::set<unsigned int> m_indices;

  void load_indices();
  void load_monitors();
  void load_dimensions();

  bool inside(int x, int y);
  static int sort_cb(const void*, const void*);
};

#endif
