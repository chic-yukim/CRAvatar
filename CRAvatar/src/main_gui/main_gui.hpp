#pragma once

#include <render_pipeline/rppanda/showbase/direct_object.hpp>

class MainApp;

class MainGUI : public rppanda::DirectObject
{
public:
    MainGUI(MainApp& app);

    virtual ~MainGUI();

private:
    void on_imgui_new_frame();

    void setup_foot_mouse();
    void ui_foot_mouse();

    MainApp& app_;
};
