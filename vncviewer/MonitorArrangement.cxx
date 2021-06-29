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

#include <vector>
#include <algorithm>

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

#include "MonitorArrangement.h"

#define MONITOR_ARRANGEMENT_MARGIN 20
#define MONITOR_MARGIN_SCALE_FACTOR 0.99
#define MONITOR_AVAILABLE_COLOR fl_lighter(fl_lighter(fl_lighter(FL_BACKGROUND_COLOR)))
#define MONITOR_SELECTED_COLOR fl_rgb_color(53, 132, 228)
#define MONITOR_REQUIRED_COLOR fl_lighter(fl_lighter(fl_rgb_color(53, 132, 228)))

#define FL_CHECKERED_BOX (Fl_Boxtype) FL_FREE_BOXTYPE

typedef struct {
    unsigned int index;
    MonitorArrangement *destination;
} CallbackData;

MonitorArrangement::MonitorArrangement(
    int x, int y, int w, int h, MonitorArrangementDelegate *delegate
): Fl_Group(x, y, w, h), m_delegate(delegate)
{
  box(FL_DOWN_BOX);
  color(fl_lighter(FL_BACKGROUND_COLOR));

  // Register a custom boxtype for the required monitor appearance.
  Fl::set_boxtype(FL_CHECKERED_BOX, checkered_pattern_draw, 0, 0, 0, 0);

  double s = scale();
  int x_m, y_m, w_m, h_m;

  for (int i = 0; i < m_delegate->count(); i++) {
    m_delegate->dimensions(x_m, y_m, w_m, h_m, i);

    Fl_Button *monitor = new Fl_Button(
      /* x = */ x + offset_x() + x_m*s + (1 - MONITOR_MARGIN_SCALE_FACTOR)*x_m*s,
      /* y = */ y + offset_y() + y_m*s + (1 - MONITOR_MARGIN_SCALE_FACTOR)*y_m*s,
      /* w = */ w_m*s*MONITOR_MARGIN_SCALE_FACTOR,
      /* h = */ h_m*s*MONITOR_MARGIN_SCALE_FACTOR
    );

    CallbackData *data = new CallbackData();
    data->destination = this;
    data->index = (unsigned int) i;

    monitor->clear_visible_focus();
    monitor->copy_tooltip(m_delegate->description(i));
    monitor->callback(callback, data);
    monitor->type(FL_TOGGLE_BUTTON);
    monitor->when(FL_WHEN_CHANGED);
    monitor->value(m_delegate->is_selected(i) ? 1 : 0);
    style(monitor, i);

    m_monitors.push_back(monitor);
  }

  end();
}

MonitorArrangement::~MonitorArrangement()
{

}

void MonitorArrangement::draw()
{
  for (int i = 0; i != m_delegate->count(); i++) {
    style(i);
  }

  Fl_Group::draw();
}

void MonitorArrangement::show()
{
    Fl_Group::show();
}

double MonitorArrangement::scale()
{
  double s_w = static_cast<double>(this->w()-MONITOR_ARRANGEMENT_MARGIN) / static_cast<double>(m_delegate->width());
  double s_h = static_cast<double>(this->h()-MONITOR_ARRANGEMENT_MARGIN) / static_cast<double>(m_delegate->height());

  // Choose the one that scales the least, in order to
  // maximize our use of the given bounding area.
  if (s_w > s_h) {
    return s_h;
  } else {
    return s_w;
  }
}

int MonitorArrangement::offset_x()
{
  return (this->w()/2) - (m_delegate->width()/2 * scale());
}

int MonitorArrangement::offset_y()
{
  return (this->h()/2) - (m_delegate->height()/2 * scale());
}

void MonitorArrangement::style(int i)
{
  style(m_monitors[i], i);
}

void MonitorArrangement::style(Fl_Button *monitor, int i)
{
  if (m_delegate->is_selected(i)) {
    monitor->box(FL_BORDER_BOX);
    monitor->color(MONITOR_SELECTED_COLOR);
    monitor->selection_color(MONITOR_SELECTED_COLOR);
  } else if (m_delegate->is_required(i)) {
    monitor->box(FL_CHECKERED_BOX);
    monitor->color(MONITOR_REQUIRED_COLOR);
    monitor->selection_color(MONITOR_REQUIRED_COLOR);
  } else {
    monitor->box(FL_BORDER_BOX);
    monitor->color(MONITOR_AVAILABLE_COLOR);
    monitor->selection_color(MONITOR_AVAILABLE_COLOR);
  }
}

void MonitorArrangement::notify(int i)
{
  m_delegate->set(i, m_monitors[i]->value() == 1);
  redraw();
}

void MonitorArrangement::callback(Fl_Widget *, void *data)
{
  CallbackData *cbd = (CallbackData *)data;
  cbd->destination->notify(cbd->index);
}

void MonitorArrangement::checkered_pattern_draw(
  int x, int y, int width, int height, Fl_Color color)
{
  bool draw_checker = false;
  const int CHECKER_SIZE = 8;
  
  fl_color(MONITOR_AVAILABLE_COLOR);
  fl_rectf(x, y, width, height);
  
  fl_color(Fl::draw_box_active() ? color : fl_inactive(color));

  // Round up the square count. Later on, we remove square area that are
  // outside the given bounding area. 
  const int count = (width + CHECKER_SIZE - 1) / CHECKER_SIZE;
  
  for (int i = 0; i < count; i++) {
    for (int j = 0; j < count; j++) {
      
      draw_checker = (i + j) % 2 == 0;

      if (draw_checker) {
        fl_rectf(
          /* x = */ x + i * CHECKER_SIZE,
          /* y = */ y + j * CHECKER_SIZE,
          /* w = */ CHECKER_SIZE - std::max(0, ((i + 1) * CHECKER_SIZE) - width),
          /* h = */ CHECKER_SIZE - std::max(0, ((j + 1) * CHECKER_SIZE) - height)
        );
      }
    }
  }

  fl_color(Fl::draw_box_active() ? FL_BLACK : fl_inactive(FL_BLACK));
  fl_rect(x, y, width, height);
}
