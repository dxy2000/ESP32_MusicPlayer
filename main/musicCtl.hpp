#pragma once

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/dirent.h>
#include <stdio.h>
#include <vector>
#include <cJSON.h>

static const char *MusicCtlTag = "musicCtl";

class MusicCtl{
public:

    static MusicCtl& GetInstance(){
        return instance;
    }

    MusicCtl(){
        curIndex_ = 0;
    }

    // 从文件中获取 json 字符串
    char* GetJsonStr(const std::string& path){
        FILE* fp = fopen(path.c_str(), "r");
        if (fp == NULL) {
            ESP_LOGE(MusicCtlTag, "Fail to open file:%s", path.c_str());
            return NULL;
        }
        // 获取文件大小
        fseek(fp, 0, SEEK_END);
        long fileSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        // 分配内存, 读取文件内容到 str 中
        char* str = (char *)malloc(fileSize + 1);
        fread(str, 1, fileSize, fp);
        str[fileSize] = '\0';
        // 关闭文件
        fclose(fp);
        return str;
    }

    void LoadFileNames(){
        // 读取 json 文件
        std::string path = "/sdcard/list.txt";
        char* jsonStr = GetJsonStr(path);
        if(jsonStr == NULL){
            return;
        }
        ESP_LOGI(MusicCtlTag, "Succeeded to open json file:%s", path.c_str());
        
        // 解析 JSON 字符串
        cJSON *jsonData = cJSON_Parse(jsonStr);
        if (jsonData == NULL) {
            ESP_LOGE(MusicCtlTag, "Fail to parse json file!");
            free(jsonStr);
            return;
        }
        if(cJSON_IsArray(jsonData)){
            int array_size = cJSON_GetArraySize(jsonData);
            for (int i = 0; i < array_size; i++) {
                cJSON *element = cJSON_GetArrayItem(jsonData, i);
                cJSON *fileName = cJSON_GetObjectItemCaseSensitive(element, "name");
                if (cJSON_IsString(fileName)) {
                    std::string temp_name = cJSON_GetStringValue(fileName);
                    names_.push_back(temp_name);
                }else{
                    ESP_LOGE(MusicCtlTag, "element name is not string!");
                }
            }
        }
        ESP_LOGI(MusicCtlTag, "Parse Json successfully!");

        // 释放资源
        cJSON_Delete(jsonData);
        free(jsonStr);
    }

    // 根据 序号 获得 歌曲名
    std::string GetName(int index){
        return names_[index];
    }

    int GetCurIndex(){
        return curIndex_;
    }

    // 转到下一个歌曲
    void GotoNext(){
        curIndex_++;
        if(curIndex_ == names_.size()){
            curIndex_ = 0;
        }
        ESP_LOGI(MusicCtlTag, "Next Music: %s", names_[curIndex_].c_str());
    }

    // 转到上一首歌曲
    void GotoPrevious(){
        curIndex_--;
        if(curIndex_ == -1){
            curIndex_ = names_.size() - 1;
        }
        ESP_LOGI(MusicCtlTag, "Previous Music: %s", names_[curIndex_].c_str());
    }   

    // For Test
    void PrintMusicList(){
        for(int i=0; i<names_.size(); i++){
            std::cout << names_[i] << std::endl;
        }
    }


private:
    std::vector<std::string> names_;
    int curIndex_;

    static MusicCtl instance;
};

MusicCtl MusicCtl::instance;