#pragma once

#include "i2sOutput.hpp"
#include "audioCtl.hpp"

#include "iot_button.h"
#include "config.h"
#include "esp_log.h"
#include "driver/gpio.h"

static const char *ButtonTAG = "Button";


class ButtonCtl{

public:

    static ButtonCtl& GetInstance(){
        return instance;
    }

    void InitButtons(){
        InitPlayButton();
        InitDownButton();
        InitUpButton();
        InitNextButton();
        InitPreButton();
    }

    // play button 的 single click 事件的回调函数
    static void PlayButtonOnClick(void *arg, void *data){
        ESP_LOGI(ButtonTAG, "Play Button Onclick!");
        AudioCtl::GetInstance().ChangeStatus();
    }
    // 初始化 play button
    void InitPlayButton(){
        button_config_t btn_cfg = {
            .type = BUTTON_TYPE_GPIO,
            .gpio_button_config = {
                .gpio_num = AUDIO_BUTTON_GPIO_PLAY,   // config.h 中定义好的
                .active_level = 0,
            },
        };
        // 创造 play button
        button_handle_t btn = iot_button_create(&btn_cfg);
        assert(btn);
        // 为 play button 的 BUTTON_SINGLE_CLICK 事件注册回调函数 PlayButtonOnClick()
        esp_err_t err = iot_button_register_cb(btn, BUTTON_SINGLE_CLICK, PlayButtonOnClick, NULL);
        ESP_ERROR_CHECK(err);
    }

    // Down Button
    static void DownButtonOnClick(void *arg, void *data){
        ESP_LOGI(ButtonTAG, "Down Button Onclick!");
        AudioCtl::GetInstance().VolDown();
    }
    void InitDownButton(){
        button_config_t btn_cfg = {
            .type = BUTTON_TYPE_GPIO,
            .gpio_button_config = {
                .gpio_num = AUDIO_BUTTON_GPIO_VOL_DOWN,
                .active_level = 0,
            },
        };
        button_handle_t btn = iot_button_create(&btn_cfg);
        assert(btn);
        esp_err_t err = iot_button_register_cb(btn, BUTTON_SINGLE_CLICK, DownButtonOnClick, NULL);
        ESP_ERROR_CHECK(err);
    }

    // Up Button
    static void UpButtonOnClick(void *arg, void *data){
        ESP_LOGI(ButtonTAG, "Up Button Onclick!");
        AudioCtl::GetInstance().VolUp();
    }
    void InitUpButton(){
        button_config_t btn_cfg = {
            .type = BUTTON_TYPE_GPIO,
            .gpio_button_config = {
                .gpio_num = AUDIO_BUTTON_GPIO_VOL_UP,
                .active_level = 0,
            },
        };
        button_handle_t btn = iot_button_create(&btn_cfg);
        assert(btn);
        esp_err_t err = iot_button_register_cb(btn, BUTTON_SINGLE_CLICK, UpButtonOnClick, NULL);
        ESP_ERROR_CHECK(err);
    }

    // Music Pre
    static void PreButtonSingleClick(void *arg, void *data){
        ESP_LOGI(ButtonTAG, "Pre Button Onclick!");
        AudioCtl::GetInstance().PreviousMusic();
    }
    void InitPreButton(){
        button_config_t btn_cfg = {
            .type = BUTTON_TYPE_GPIO,
            .gpio_button_config = {
                .gpio_num = AUDIO_BUTTON_GPIO_MUSIC_PRE,
                .active_level = 0,
            },
        };
        button_handle_t btn = iot_button_create(&btn_cfg);
        assert(btn);
        esp_err_t err = iot_button_register_cb(btn, BUTTON_SINGLE_CLICK, PreButtonSingleClick, NULL);
        ESP_ERROR_CHECK(err);
    }

    // Music Next
    static void NextButtonSingleClick(void *arg, void *data){
        ESP_LOGI(ButtonTAG, "Next Button Onclick!");
        AudioCtl::GetInstance().NextMusic();
    }
    void InitNextButton(){
        button_config_t btn_cfg = {
            .type = BUTTON_TYPE_GPIO,
            .gpio_button_config = {
                .gpio_num = AUDIO_BUTTON_GPIO_MUSIC_NEXT,
                .active_level = 0,
            },
        };
        button_handle_t btn = iot_button_create(&btn_cfg);
        assert(btn);
        esp_err_t err = iot_button_register_cb(btn, BUTTON_SINGLE_CLICK, NextButtonSingleClick, NULL);
        ESP_ERROR_CHECK(err);
    }


private:
    static ButtonCtl instance;

};

ButtonCtl ButtonCtl::instance;


