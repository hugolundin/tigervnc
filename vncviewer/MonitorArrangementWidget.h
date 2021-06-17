#include <vector>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Timer.H>
#include <FL/Fl_Group.H>
#include <FL/fl_draw.H>

#include "monitors.h"

class MonitorArrangementWidget: public Fl_Group {
public:
    MonitorArrangementWidget(int, int, int, int, std::vector<Monitor>&);
    ~MonitorArrangementWidget(){};
protected:
    virtual void draw();
    virtual int handle(int);
private:
    double arrangement_scale();
    int arrangement_real_width();
    int arrangement_real_height();

    std::vector<Monitor>& m_monitors;
};