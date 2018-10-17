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

    MainApp& app_;
};
