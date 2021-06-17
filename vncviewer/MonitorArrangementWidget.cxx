#include <vector>
#include <string>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Timer.H>
#include <FL/Fl_Group.H>
#include <FL/fl_draw.H>

#include "monitors.h"
#include "MonitorArrangementWidget.h"

MonitorArrangementWidget::MonitorArrangementWidget(int x, int y, int w, int h, std::vector<Monitor>& monitors):
    Fl_Group(x, y, w, h), m_monitors(monitors)
{
    // Scale the arrangement such that it fits in the given area.
    double scale = arrangement_scale();

    for (std::vector<Monitor>::iterator monitor = m_monitors.begin();
         monitor != m_monitors.end();
         monitor++)
    {
        Fl_Box * box = new Fl_Box(
            /* x = */ x + monitor->x * scale,
            /* y = */ y + monitor->y * scale,
            /* w = */ monitor->w * scale,
            /* h = */ monitor->h * scale
        );

        box->box(FL_BORDER_BOX);
        box->labelfont(FL_BOLD);
        box->labelsize(20);
        box->copy_label(std::to_string(monitor->index).c_str());
    }

    // Add dynamically created widgets here.
    end();
}

void MonitorArrangementWidget::draw()
{
    Fl_Group::draw();
}

int MonitorArrangementWidget::handle(int event)
{
    return Fl_Group::handle(event);
}

double MonitorArrangementWidget::arrangement_scale()
{
    double s_width = ((double) w()) / ((double) arrangement_real_width());
    double s_height = ((double) h()) / ((double) arrangement_real_height());

    // Use the scaling that fills up the available area most.
    if (s_width > s_height) {
        return s_height;
    } else {
        return s_width;
    }
}

int MonitorArrangementWidget::arrangement_real_width()
{
    int width = 0;

    for (std::vector<Monitor>::iterator monitor = m_monitors.begin();
        monitor != m_monitors.end();
        monitor++)
    {
        
        if ((monitor->x + monitor->w) > width) {
            width = monitor->x + monitor->w;
        }
    }

    return width;
}

int MonitorArrangementWidget::arrangement_real_height()
{
    int height = 0;

    for (std::vector<Monitor>::iterator monitor = m_monitors.begin();
        monitor != m_monitors.end();
        monitor++)
    {
        
        if ((monitor->y + monitor->h) > height) {
            height = monitor->y + monitor->h;
        }
    }

    return height;
}