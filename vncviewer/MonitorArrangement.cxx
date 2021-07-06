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

#include <set>
#include <vector>
#include <string>
#include <utility>
#include <sstream>
#include <assert.h>
#include <algorithm>

#ifdef HAVE_XRANDR
#include <X11/extensions/Xrandr.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

#include <rfb/LogWriter.h>
#include <FL/Fl.H>
#include <FL/fl_draw.H>

#include "MonitorArrangement.h"

static rfb::LogWriter vlog("MonitorArrangement");
static const Fl_Boxtype FL_CHECKERED_BOX = FL_FREE_BOXTYPE;

MonitorArrangement::MonitorArrangement(
   int x, int y, int w, int h)
:  Fl_Group(x, y, w, h),
   SELECTION_COLOR(fl_rgb_color(53, 132, 228)),
   AVAILABLE_COLOR(fl_lighter(fl_lighter(fl_lighter(FL_BACKGROUND_COLOR)))),
   m_monitors()
{
  // Used for required monitors. 
  Fl::set_boxtype(FL_CHECKERED_BOX, checkered_pattern_draw, 0, 0, 0, 0);

  box(FL_DOWN_BOX);
  color(fl_lighter(FL_BACKGROUND_COLOR));
  layout();
  end();
}

MonitorArrangement::~MonitorArrangement()
{

}

std::set<int> MonitorArrangement::get()
{
  std::set<int> indices;

  for (int i = 0; i < (int) m_monitors.size(); i++) {
    if (m_monitors[i]->value() == 1)
      indices.insert(i);
  }

  return indices;
}

void MonitorArrangement::set(std::set<int> indices)
{
  for (int i = 0; i < (int) m_monitors.size(); i++) {
    bool selected = std::find(indices.begin(), indices.end(), i) != indices.end();
    m_monitors[i]->value(selected ? 1 : 0);
  }
}

void MonitorArrangement::draw()
{
  for (int i = 0; i < (int) m_monitors.size(); i++) {
    Fl_Button * monitor = m_monitors[i];

    if (is_required(i)) {
      monitor->box(FL_CHECKERED_BOX);
      monitor->color(SELECTION_COLOR);
    } else {
      monitor->box(FL_BORDER_BOX);
      monitor->color(AVAILABLE_COLOR);
      monitor->selection_color(SELECTION_COLOR);
    }
  }

  Fl_Group::draw();
}

void MonitorArrangement::layout()
{
  int x, y, w, h;
  double scale = this->scale();
  const double MARGIN_SCALE_FACTOR = 0.99;
  std::pair<int, int> offset = this->offset();

  for (int i = 0; i < Fl::screen_count(); i++) {
    Fl::screen_xywh(x, y, w, h, i);

    Fl_Button *monitor = new Fl_Button(
      /* x = */ this->x() + offset.first + x*scale + (1 - MARGIN_SCALE_FACTOR)*x*scale,
      /* y = */ this->y() + offset.second +  y*scale + (1 - MARGIN_SCALE_FACTOR)*y*scale,
      /* w = */ w*scale*MARGIN_SCALE_FACTOR,
      /* h = */ h*scale*MARGIN_SCALE_FACTOR
    );

    monitor->clear_visible_focus();
    monitor->callback(monitor_pressed, this);
    monitor->type(FL_TOGGLE_BUTTON);
    monitor->when(FL_WHEN_CHANGED);
    m_monitors.push_back(monitor);
  }

  for (int i = 0; i < (int) m_monitors.size(); i++)
    m_monitors[i]->copy_tooltip(description(i).c_str());
}

bool MonitorArrangement::is_required(int m)
{
  // A selected monitor is never required. 
  if (m_monitors[m]->value() == 1)
    return false;

  // If no monitors are selected, none are required.
  std::set<int> selected = get();
  if (selected.size() <= 0)
    return false;


  // Go through all selected monitors and find the monitor 
  // indices that bounds the fullscreen frame buffer. If
  // the given monitor's coordinates are inside the bounds,
  // while not being selected, it is instead required.

  int x, y, w, h;
  int top_y, bottom_y, left_x, right_x;
  std::set<int>::iterator it = selected.begin();

  // Base the rest of the calculations on the dimensions
  // obtained for the first monitor.
  Fl::screen_xywh(x, y, w, h, *it);
  top_y = y;
  bottom_y = y + h;
  left_x = x;
  right_x = x + w;

  // Go through the rest of the monitors,
  // exhausting the rest of the iterator.
  for (; it != selected.end(); it++) {
    Fl::screen_xywh(x, y, w, h, *it);

    if (y < top_y) {
      top_y = y;
    }

    if ((y + h) > bottom_y) {
      bottom_y = y + h;
    }

    if (x < left_x) {
      left_x = x;
    }

    if ((x + w) > right_x) {
      right_x = x + w;
    }
  }

  // Finally, get the dimensions of the monitor that we 
  // are looking whether it is required. 
  Fl::screen_xywh(x, y, w, h, m);

  return inside(top_y, bottom_y, left_x, right_x, x, y) 
      || inside(top_y, bottom_y, left_x, right_x, x+w, y)
      || inside(top_y, bottom_y, left_x, right_x, x, y+h)
      || inside(top_y, bottom_y, left_x, right_x, x+w, y+h);
}

double MonitorArrangement::scale()
{
  const int MARGIN = 20;
  std::pair<int, int> size = this->size();

  double s_w = static_cast<double>(this->w()-MARGIN) / static_cast<double>(size.first);
  double s_h = static_cast<double>(this->h()-MARGIN) / static_cast<double>(size.second);

  // Choose the one that scales the least, in order to
  // maximize our use of the given bounding area.
  if (s_w > s_h)
    return s_h;
  else
    return s_w;
}

std::pair<int, int> MonitorArrangement::size()
{
  int x, y, w, h;
  int top, bottom, left, right;
  int x_min, x_max, y_min, y_max;
  x_min = x_max = y_min = y_max = 0;

  for (int i = 0; i < Fl::screen_count(); i++) {
    Fl::screen_xywh(x, y, w, h, i);
    
    top = y;
    bottom = y + h;
    left = x;
    right = x + w;
    
    if (top < y_min)
      y_min = top;

    if (bottom > y_max)
      y_max = bottom;

    if (left < x_min)
      x_min = left;

    if (right > x_max)
      x_max = right;
  }
  
  return std::make_pair(x_max - x_min, y_max - y_min);
}

std::pair<int, int> MonitorArrangement::offset()
{
  double scale = this->scale();
  std::pair<int, int> size = this->size();
  std::pair<int, int> origin = this->origin();

  int offset_x = (this->w()/2) - (size.first/2 * scale);
  int offset_y = (this->h()/2) - (size.second/2 * scale);

  return std::make_pair(offset_x + abs(origin.first), offset_y + abs(origin.second));
}

std::pair<int, int> MonitorArrangement::origin()
{
  int x, y, w, h, ox, oy;
  ox = oy = 0;

  for (int i = 0; i < Fl::screen_count(); i++) {
    Fl::screen_xywh(x, y, w, h, i);

    if (x < ox)
      ox = x;

    if (y < oy)
      oy = y;
  }

  return std::make_pair(ox, oy);
}

bool MonitorArrangement::inside(
  int top_y, int bottom_y, int left_x, int right_x, int x, int y)
{
  return (x > left_x) && (x < right_x) && (y > top_y) && (y < bottom_y);
}

std::string MonitorArrangement::description(int m)
{
  std::stringstream ss;
  assert(m < (int) m_monitors.size());

  // Get the platform specific name of the monitor. 
  ss << name(m);

  if (ss.str().empty()) {
    int x, y, w, h;
    Fl::screen_xywh(x, y, w, h, m);

    // Fallback to showing resolution and position of monitor.
    ss << w << "x" << h << "+" << x << "+" << y;
  }

  return ss.str();
}

std::string MonitorArrangement::name(int m)
{
  assert(m < (int) m_monitors.size());

  int x, y, w, h;
  std::stringstream ss;
  Fl::screen_xywh(x, y, w, h, m);

#if !defined(WIN32) && !defined(__APPLE__)
#ifdef HAVE_XRANDR

  int ev, err, xi_major;
  fl_open_display();
  assert(fl_display != NULL);

  if (!XQueryExtension(fl_display, "RANDR", &xi_major, &ev, &err)) {
    vlog.info("Unable to find X11 RANDR extension.");
    return std::string();
  }

  XRRScreenResources *res = XRRGetScreenResources(fl_display, DefaultRootWindow(fl_display));
  if (!res) {
    vlog.error("Unable to get XRRScreenResources for fl_display.");
    return std::string();
  }

  for (int i = 0; i < res->ncrtc; i++) {
    XRRCrtcInfo *crtc = XRRGetCrtcInfo(fl_display, res, res->crtcs[i]);

    if (!crtc) {
      vlog.error("Unable to get XRRCrtcInfo for crtc %d.", i);
      continue;
    }

    for (int j = 0; j < crtc->noutput; j++) {
      bool monitor_found = (crtc->x == x) &&
          (crtc->y == y) &&
          (crtc->width == ((unsigned int) w)) &&
          (crtc->height == ((unsigned int) h));

      if (monitor_found) {
        XRROutputInfo *output = XRRGetOutputInfo(fl_display, res, crtc->outputs[j]);
        if (!output) {
          vlog.error("Unable to get XRROutputInfo for crtc %d, output %d.", i, j);
          continue;
        }

        ss << output->name;
      }
    }
  }

  // Show resolution and position in parenthesis.
  ss << " (" << w << "x" << h << "+" << x << "+" << y << ")";
#endif
#elif defined(WIN32)
  // FIXME: Add support for showing the monitor name on Windows. 
#elif defined(__APPLE__)
  // FIXME: Add support for showing the monitor name on macOS. 
#endif

  return ss.str();
}

void MonitorArrangement::monitor_pressed(Fl_Widget *widget, void *user_data)
{
  MonitorArrangement *self = (MonitorArrangement *) user_data;

  // When a monitor is selected, FLTK changes the state of it for us. 
  // However, selecting a monitor might implicitly change the state of
  // others (if they become required). FLTK only redraws the selected
  // monitor. Therefore, we must trigger a redraw of the whole widget
  // manually. 
  self->redraw();
}

void MonitorArrangement::checkered_pattern_draw(
  int x, int y, int width, int height, Fl_Color color)
{
  bool draw_checker = false;
  const int CHECKER_SIZE = 8;

  fl_color(fl_lighter(fl_lighter(fl_lighter(color))));
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
