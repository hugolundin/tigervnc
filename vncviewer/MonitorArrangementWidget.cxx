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

#include "Monitors.h"
#include "MonitorArrangementWidget.h"

typedef struct {
    MonitorArrangementWidget *destination;
    unsigned int index;
} CallbackData;

MonitorArrangementWidget::MonitorArrangementWidget(
    int x, int y, int w, int h
): Fl_Group(x, y, w, h)
{
  double s = scale();
  int monitor_x, monitor_y, monitor_w, monitor_h;

  int fbx, fby, fbw, fbh;
  Monitors::shared().frame_buffer_dimensions(fbx, fby, fbw, fbh);

  frame_buffer = new Fl_Button(
    x + fbx*s,
    y + fby*s,
    fbw*s,
    fbh*s
  );

  frame_buffer->box(FL_FLAT_BOX);
  frame_buffer->clear_visible_focus();
  frame_buffer->color(fl_rgb_color(189, 207, 222));

  for (int i = 0; i < Monitors::shared().count(); i++) {
    Monitors::shared().dimensions(monitor_x, monitor_y, monitor_w, monitor_h, i);

    Fl_Button * box = new Fl_Button(
        /* x = */ x + monitor_x * s,
        /* y = */ y + monitor_y * s,
        /* w = */ monitor_w * s,
        /* h = */ monitor_h * s
    );

    CallbackData *data = new CallbackData();
    data->destination = this;
    data->index = (unsigned int) i;
    
    box->callback(monitor_cb_adapter, data);
    box->when(FL_WHEN_RELEASE);
    box->clear_visible_focus();
    m_monitor_widgets.push_back(box);
  }

  end();
}


void MonitorArrangementWidget::draw()
{
  // Scale the arrangement such that it fits in the given area.
  double s = scale();
  // int offset_x = ((w() - max_width()*s) / 2);
  // int offset_y = ((h() - max_height()*s) / 2);

  int x, y, w, h;
  for (int i = 0; i < Monitors::shared().count(); i++) {
    Fl_Button * box = m_monitor_widgets[i];
    Monitors::shared().dimensions(x, y, w, h, i);
  
    box->box(FL_BORDER_BOX);
    // box->labelfont(FL_BOLD);
    //box->labelsize(20);
    //box->copy_label(std::to_string(monitor.index).c_str());
    box->resize(this->x() + x * s, this->y() + y * s, w * s, h * s);
    box->copy_tooltip(Monitors::shared().description(i));

    if (Monitors::shared().is_selected(i)) {
      box->color(fl_rgb_color(53, 132, 228));
      box->selection_color(fl_rgb_color(53, 132, 228));
    } else if (Monitors::shared().is_required(i)) {
      box->color(fl_lighter(fl_lighter(fl_rgb_color(53, 132, 228))));
      box->selection_color(fl_lighter(fl_lighter(fl_rgb_color(53, 132, 228))));
    } else {
      box->color(fl_lighter(FL_BACKGROUND_COLOR));
      box->selection_color(fl_lighter(FL_BACKGROUND_COLOR));
    }
  }
    
  if (!Monitors::shared().has_required()) {
    frame_buffer->show();
  } else {
    frame_buffer->hide();
  }

  Fl_Group::draw();
}

void MonitorArrangementWidget::update()
{
  double s = scale();

  int fbx, fby, fbw, fbh;
  Monitors::shared().frame_buffer_dimensions(fbx, fby, fbw, fbh);
  frame_buffer->resize(this->x() + fbx*s, this->y() + fby*s, fbw*s, fbh*s);
  frame_buffer->redraw();
  //redraw();
  parent()->redraw();
  Fl::redraw();
}

void MonitorArrangementWidget::show()
{
    Fl_Group::show();
}

double MonitorArrangementWidget::scale()
{
  double s_width = ((double) w()) / ((double) max_width());
  double s_height = ((double) h()) / ((double) max_height());

  // Use the scaling that fills up the available area most.
  if (s_width > s_height) {
    return s_height;
  } else {
    return s_width;
  }
}

int MonitorArrangementWidget::max_width()
{
  int max_width = 0;

  int x, y, w, h;
  for (int i = 0; i < Monitors::shared().count(); i++) {
    Monitors::shared().dimensions(x, y, w, h, i);

    if (x + w > max_width) {
        max_width = x + w;
    }
  }

  return max_width;
}

int MonitorArrangementWidget::max_height()
{
  int max_height = 0;

  int x, y, w, h;
  for (int i = 0; i < Monitors::shared().count(); i++) {
    Monitors::shared().dimensions(x, y, w, h, i);

    if (y + h > max_height) {
        max_height = y + h;
    }
  }

  return max_height;
}

void MonitorArrangementWidget::monitor_cb(Fl_Widget * widget, unsigned int index)
{
  Monitors::shared().toggle(index);
}

void MonitorArrangementWidget::monitor_cb_adapter(Fl_Widget * widget, void * user_data)
{
    CallbackData *data = (CallbackData *) user_data;
    ((MonitorArrangementWidget *) data->destination)->monitor_cb(widget, data->index);
}
