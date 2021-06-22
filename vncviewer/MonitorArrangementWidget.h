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

#ifndef __MONITOR_ARRANGEMENT_WIDGET_H__
#define __MONITOR_ARRANGEMENT_WIDGET_H__

#include "Monitors.h"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Timer.H>
#include <FL/Fl_Group.H>
#include <FL/fl_draw.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Check_Browser.H>

class MonitorArrangementWidget: public Fl_Group {
public:
  MonitorArrangementWidget(int, int, int, int);
  ~MonitorArrangementWidget() {};
  void update();
  virtual void draw();
  virtual void show();
  
private:
  Fl_Button * frame_buffer;
  std::vector<Fl_Button *> m_monitor_widgets;

  double scale();
  int max_width();
  int max_height();

  void monitor_cb(Fl_Widget *, unsigned int);
  static void monitor_cb_adapter(Fl_Widget *, void *);
};

#endif
