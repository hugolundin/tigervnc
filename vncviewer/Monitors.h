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

#include "MonitorArrangement.h"

#include <vector>
#include <set>
#include <list>
#include <map>

#define DESCRIPTION_MAX_LEN 256

enum ConfigurationMode {
  CURRENT,
  ALL,
  SELECTED
};

class Monitors: public MonitorArrangementDelegate {
public:
  Monitors();
  ~Monitors();

  /// Get the current monitor count. 
  int count() const;

  /// Set the indices configuration. 
  void set_indices(char const *);

  /// Set the mode configuration.
  void set_mode(char const *);

  /// Store the current configuration to disk. 
  void save(char *, int);

  /// Returns true if the configuration has monitors
  /// that are required to be part of it to create a 
  /// rectangular frame buffer. 
  bool has_required() const;

  /// Return the dimensions of the selection frame buffer.
  void dimensions(int&, int&, int&, int&) const;

  /// Toggle the state for the given monitor. 
  void toggle(unsigned int);

  /// Set the state for the given monitor. 
  void set(unsigned int, bool);

  /// Return true if the given monitor is selected. 
  bool is_selected(unsigned int) const;

  /// Return true if the given monitor is required. 
  bool is_required(unsigned int) const;

  /// Return the description for the given monitor. 
  char const * description(unsigned int) const;

  /// Return the dimensions for the given monitor.
  void dimensions(int&, int&, int&, int&, unsigned int) const;

  /// Return the FLTK index of the primary monitor in the current configuration.
  int primary() const;

  /// Return the FLTK index of the monitor limiting the top of the frame buffer. 
  int top() const;

  /// Return the FLTK index of the monitor limiting the left of the frame buffer. 
  int left() const;

  /// Return the FLTK index of the monitor limiting the right of the frame buffer. 
  int right() const;

  /// Return the FLTK index of the monitor limiting the bottom of the frame buffer. 
  int bottom() const;

  /// Return the width of the monitor configuration.
  int width() const;

  /// Return the height of the monitor configuration.
  int height() const;

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

  ConfigurationMode m_mode;
  std::vector<Monitor> m_monitors;
  std::set<unsigned int> m_indices;
  
  /// Load the monitors available on the system. 
  void load_monitors();

  /// Calculate the dimensions for fullscreen mode.
  void calculate_dimensions();
  
  /// Return true if the given coordinates are inside 
  /// the frame buffer for the selected monitors.
  bool inside(int x, int y) const;

  /// Return true if a mode with multiple monitors in 
  /// fullscreen is currently being used.
  bool multiple_monitors() const;

  /// Callback for sorting monitors.
  static int sort_cb(const void*, const void*);
};

#endif
