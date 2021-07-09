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

#ifndef __MONITOR_ARRANGEMENT_H__
#define __MONITOR_ARRANGEMENT_H__

#include <string>

#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>

class MonitorArrangement: public Fl_Group {
public:
  MonitorArrangement(int x, int y, int w, int h);
  ~MonitorArrangement();

  // Get selected indices.
  std::set<int> get();

  // Set selected indices. 
  void set(std::set<int> indices);

protected:
  virtual void draw();

private:
  const Fl_Color SELECTION_COLOR;
  const Fl_Color AVAILABLE_COLOR;
  std::vector<Fl_Button *> m_monitors;

  // Layout the monitor arrangement.
  void layout();

  // Return true if the given monitor is required to be part of the configuration
  // for it to be valid. A configuration is only valid if the framebuffer created
  // from is rectangular.
  bool is_required(int m);

  // Calculate the scale of the monitor arrangement. 
  double scale();

  // Return the size of the monitor arrangement. 
  std::pair<int, int> size();

  // Return the offset required for centering the monitor
  // arrangement in the given bounding area. 
  std::pair<int, int> offset();

  // Return the origin of the monitor arrangement (top left corner). 
  std::pair<int, int> origin();

  // Return true if the given coordinates are inside the given bounding area:
  //
  //   +------top_y-------+
  //   |                  |
  // left_x   (x,y)    right_x
  //   |                  |
  //   +-----bottom_y-----+
  //
  bool inside(int top_y, int bottom_y, int left_x, int right_x, int x, int y);

  // Get a textual description of the given monitor.
  std::string description(int m);
  
  #if defined(HAVE_XRANDR)
  const char* xrandr_get_display_name(int m);
  #endif

  static void monitor_pressed(Fl_Widget *widget, void *user_data);
  static void checkered_pattern_draw(
    int x, int y, int width, int height, Fl_Color color);
};

#endif
