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

#include <vector>

class MonitorArrangementDelegate {
public:
  virtual ~MonitorArrangementDelegate(){};
  virtual int count() const=0;
  virtual bool is_selected(unsigned int index) const=0;
  virtual bool is_required(unsigned int index) const=0;
  virtual void set(unsigned int, bool)=0;
  virtual void dimensions(int&, int&, int&, int&) const=0;
  virtual char const * description(unsigned int) const=0;
  virtual void dimensions(int&, int&, int&, int&, unsigned int) const=0;
  virtual int width() const=0;
  virtual int height() const=0;
};

class MonitorArrangement: public Fl_Group {
public:
  MonitorArrangement(
    int x, int y, int w, int h, MonitorArrangementDelegate *delegate);
  ~MonitorArrangement();
  
protected:
  virtual void draw();

private:  
  std::vector<Fl_Button *> m_monitors;
  MonitorArrangementDelegate *m_delegate;

  double scale();
  int offset_x();
  int offset_y();
  void style(int);
  void style(Fl_Button*, int);

  void notify(int);
  static void callback(Fl_Widget *, void*);

  static void checkered_pattern_draw(int, int, int, int, Fl_Color);
};

#endif
