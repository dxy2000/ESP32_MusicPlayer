#pragma once

#include "ssd1306Display.h"

#include "driver/i2c_master.h"
#include <esp_log.h>
#include <esp_err.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lvgl_port.h>
#include <font/lv_font.h>

#include "config.h"


#define TAG "Ssd1306Display"

#define I2C_HOST  0

#define EXAMPLE_LCD_PIXEL_CLOCK_HZ    (400 * 1000)
#define EXAMPLE_PIN_NUM_RST           -1
#define EXAMPLE_I2C_HW_ADDR           0x3C

// The pixel number in horizontal and vertical
#define EXAMPLE_LCD_H_RES              128
#define EXAMPLE_LCD_V_RES              32     // 修改为实际高度

// Bit number used to represent command and parameter
#define EXAMPLE_LCD_CMD_BITS           8
#define EXAMPLE_LCD_PARAM_BITS         8

// 构造函数 初始化显示屏
Ssd1306Display::Ssd1306Display(){

    ESP_LOGI(TAG, "Initialize I2C bus");
    i2c_master_bus_handle_t i2c_bus = NULL;
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_HOST,
        .sda_io_num = DISPLAY_SDA_PIN,    // config.h 中自定义的
        .scl_io_num = DISPLAY_SCL_PIN,    // config.h 中自定义的
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags = {
            .enable_internal_pullup = true,
        }
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus));

    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = EXAMPLE_I2C_HW_ADDR, 
        .on_color_trans_done = nullptr,
        .user_ctx = nullptr,
        .control_phase_bytes = 1,               
        .dc_bit_offset = 6, 
        .lcd_cmd_bits = EXAMPLE_LCD_CMD_BITS,   
        .lcd_param_bits = EXAMPLE_LCD_PARAM_BITS, 
        .flags = {
            .dc_low_on_data = 0,
            .disable_control_phase = 0,
        },
        .scl_speed_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_bus, &io_config, &io_handle));

    ESP_LOGI(TAG, "Install SSD1306 panel driver");
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_RST,
        .bits_per_pixel = 1,
    };

    esp_lcd_panel_ssd1306_config_t ssd1306_config = {
        .height = EXAMPLE_LCD_V_RES,
    };
    panel_config.vendor_config = &ssd1306_config;    
    ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &panel_handle));
    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    esp_lcd_panel_disp_on_off(panel_handle, true);

    ESP_LOGI(TAG, "Initialize LVGL");
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    lvgl_port_init(&lvgl_cfg);

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .buffer_size = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES,
        .double_buffer = true,
        .hres = EXAMPLE_LCD_H_RES,
        .vres = EXAMPLE_LCD_V_RES,
        .monochrome = true,
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .color_format = LV_COLOR_FORMAT_RGB565,
        .flags = {
            .sw_rotate = false,
            .swap_bytes = false,
        }
    };
    display_ = lvgl_port_add_disp(&disp_cfg);
    if (display_ == nullptr) {
        ESP_LOGE(TAG, "Failed to add display");
        return;
    }

    // 设置 UI
    // Lock the mutex due to the LVGL APIs are not thread-safe
    if (lvgl_port_lock(0)) {  
        SetUpUI();
        // Release the mutex
        lvgl_port_unlock();
    }

    // 我的屏幕硬件转了180度，所以要转一下，若是没有，则不要这句
    lv_disp_set_rotation(display_, LV_DISPLAY_ROTATION_180);    
}

Ssd1306Display::~Ssd1306Display(){
    lvgl_port_deinit();
}

LV_FONT_DECLARE(myIcon);           // 声明字体
LV_FONT_DECLARE(hanyi_14);         // 声明字体

// 设置 UI
void Ssd1306Display::SetUpUI()
{
    lv_obj_t *scr = lv_disp_get_scr_act(display_);

    /* 总容器 */
    lv_obj_t *container = lv_obj_create(scr);
    lv_obj_set_size(container, LV_HOR_RES, LV_VER_RES);         // 宽度为屏幕宽度，高度为屏幕高度
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);       // 弹性布局，子元素排成一列
    lv_obj_set_style_pad_all(container, 0, 0);              
    lv_obj_set_style_border_width(container, 0, 0);      
    lv_obj_set_style_pad_row(container, 0, 0);
    
    /* 上边的容器 */
    lv_obj_t *status_container = lv_obj_create(container); 
    lv_obj_set_size(status_container, LV_HOR_RES, 18);               // 宽度为固定的 28 像素，高度为屏幕高度
    lv_obj_set_flex_flow(status_container, LV_FLEX_FLOW_ROW);       // 弹性布局，子元素排成一行
    lv_obj_set_style_pad_all(status_container, 0, 0);  
    lv_obj_set_style_border_width(status_container, 0, 0);

    /* 左边图标的容器 */
    lv_obj_t *icon_container = lv_obj_create(status_container); 
    lv_obj_set_size(icon_container, 28, LV_SIZE_CONTENT);                // 宽度为固定的 28 像素，高度为自动
    lv_obj_set_style_pad_all(icon_container, 0, 0);  
    lv_obj_set_style_border_width(icon_container, 0, 0);
    
    /* 右边歌名的容器 */
    lv_obj_t *name_container = lv_obj_create(status_container);
    lv_obj_set_size(name_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);   // 宽度为 AUTO，高度为自动
    lv_obj_set_style_pad_all(name_container, 0, 0);
    lv_obj_set_style_border_width(name_container, 0, 0);
    lv_obj_set_flex_grow(name_container, 1);     // 在主轴上可以伸长以填满空余空间

    /* 下边的歌词的容器 */
    lv_obj_t *lrc_container = lv_obj_create(container);
    lv_obj_set_size(lrc_container, LV_HOR_RES, LV_SIZE_CONTENT);   // 宽度为屏幕宽度，高度为 AUTO
    lv_obj_set_style_pad_all(lrc_container, 0, 0);
    lv_obj_set_style_border_width(lrc_container, 0, 0);
    lv_obj_set_flex_grow(lrc_container, 1);     // 在主轴上可以伸长以填满空余空间

    // 图标 label
    icon_label_ = lv_label_create(icon_container);
    lv_obj_align(icon_label_, LV_ALIGN_CENTER, 0, 0);       // 对齐方式：居中
    lv_obj_set_style_text_font(icon_label_, &myIcon, 0);    // 设置字体 
    lv_label_set_text(icon_label_, "");                     // 初始文字为空
    lv_obj_set_style_pad_all(icon_label_, 0, 0);
    lv_obj_set_style_border_width(icon_label_, 0, 0);

    // 歌名 label
    name_label_ = lv_label_create(name_container); 
    lv_obj_set_style_text_font(name_label_, &hanyi_14, 0);       // 设置字体
    lv_label_set_text(name_label_, "");                          // 初始文字为空
    lv_label_set_long_mode(name_label_, LV_LABEL_LONG_SCROLL);   // 文本过长，滚动显示
    lv_obj_set_width(name_label_, lv_pct(100));
    lv_obj_set_style_text_align(name_label_, LV_TEXT_ALIGN_LEFT, 0);  // 文本对齐方式

    // 歌词 label
    lrc_label_ = lv_label_create(lrc_container);
    lv_obj_set_style_text_font(lrc_label_, &hanyi_14, 0);       // 设置字体
    lv_label_set_text(lrc_label_, "");                          // 初始文字为空
    lv_label_set_long_mode(lrc_label_, LV_LABEL_LONG_SCROLL);   // 文本过长，滚动显示
    lv_obj_set_width(lrc_label_, lv_pct(100));
    lv_obj_set_style_text_align(lrc_label_, LV_TEXT_ALIGN_LEFT, 0);  // 文本对齐方式
}

#define PLAY_ICON "\xEE\x98\xA4"	// 播放图标的 unicode 码 0xe624 对应的 UTF8 编码
#define PAUSE_ICON "\xEE\x98\xA9"	// 暂停图标的 unicode 码 0xe629 对应的 UTF8 编码

// 设置图标
void Ssd1306Display::SetIcon(const std::string& status){
     // Lock the mutex due to the LVGL APIs are not thread-safe
     if (lvgl_port_lock(0)) {  
        if(status == "play"){
            lv_label_set_text(icon_label_, PLAY_ICON);  
        }else{
            lv_label_set_text(icon_label_, PAUSE_ICON);  
        }
        // Release the mutex
        lvgl_port_unlock();
    }
}

// 设置歌名
void Ssd1306Display::SetName(const std::string& name){
    // Lock the mutex due to the LVGL APIs are not thread-safe
    if (lvgl_port_lock(0)) {  
        lv_label_set_text(name_label_, name.c_str());  
       // Release the mutex
       lvgl_port_unlock();
   }
}

// 设置歌词
void Ssd1306Display::SetLrc(const std::string& lrc){
    // Lock the mutex due to the LVGL APIs are not thread-safe
    if (lvgl_port_lock(0)) {  
        lv_label_set_text(lrc_label_, lrc.c_str());  
       // Release the mutex
       lvgl_port_unlock();
   }
}

