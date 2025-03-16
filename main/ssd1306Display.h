#pragma once

#include <lvgl.h>
#include <esp_timer.h>
#include <esp_log.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <iostream>
#include <memory>

#define TAG "Ssd1306Display"

class Ssd1306Display{
public:
    Ssd1306Display();
    ~Ssd1306Display();

    void SetUpUI();
    void SetIcon(const std::string& status);
    void SetName(const std::string& name);
    void SetLrc(const std::string& lrc);

private:
    lv_display_t *display_ = nullptr;

    lv_obj_t *icon_label_;
    lv_obj_t *name_label_;
    lv_obj_t *lrc_label_;

}; 

