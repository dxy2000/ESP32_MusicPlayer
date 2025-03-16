#pragma once

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include <vector>

#define TAG "LrcFileCtl"

// 歌词结构体
struct Lyric {
    int time;          // 时间戳
    std::string res;   // 歌词
};

class LrcFileCtl{
public:

    static LrcFileCtl& GetInstance(){
        return instance;
    }


    void Init(const int& index){

        lrcs.clear();
        ESP_LOGI(TAG, "Read lrc file index: %d",index);   

        // 打开 lrc 文件
        std::string path = "/sdcard/lrc/";
        path += std::to_string(index);
        path +=  ".txt";
        FILE* fp = fopen(path.c_str(), "r");
        if(fp == NULL){
            ESP_LOGE(TAG, "Fail to open lrc file: %s", path.c_str());   
            return;
        }

        // 获取文件大小
        fseek(fp, 0, SEEK_END);
        long fileSize = ftell(fp);
        rewind(fp);
        // 申请堆内存，并读取文件到内存中
        char* lrcString = (char *)malloc(fileSize + 1);
        fread(lrcString, 1, fileSize, fp);
        // 关闭文件
        fclose(fp);

        // 将读取到的数据按行切割
        char* line;
        line = strtok(lrcString, "\r\n");
        while(line != NULL){
            char* buffer = strdup(line);
            ParseLrcLine(buffer);
            line = strtok(NULL, "\r\n"); 
            free(buffer);  
        }
        free(line);
        free(lrcString);

        // 初始化当前行号
        lineIndex_ = 0;
    }

    // 解析一行
    void ParseLrcLine(char* line){
        if(line[0] == '['){
            // 获取时间戳，ms 为单位
            int m = 0, s = 0, h = 0;
            sscanf(line,"[%d:%d.%d]",&m,&s,&h);
            int time = m*60*1000 + s*1000 + h;  

            // 获取歌词内容
            std::string content(line);
            size_t pos = content.find(']'); 
            std::string res;
            if (pos != std::string::npos) { 
                res = content.substr(pos + 1); 
            } 
            Lyric lrc{time, res};
            lrcs.push_back(lrc);
        }
    }

    // 测试用
    void PrintLrcs(){
        for(auto item : lrcs){
            std::cout << item.time << " " << item.res << std::endl;
        }
    }

    // 获取下一行歌词的时间戳
    int GetNextLrcTime(){
        if(lineIndex_ + 1 < lrcs.size()){
            return lrcs[lineIndex_ + 1].time;
        }
        return 0;
    }

    void MoveToNextLine(){
        if(lineIndex_ + 1 < lrcs.size()){
            lineIndex_++;
        }
    }

    // 获取当前行的歌词
    std::string GetCurLrc(){
        return lrcs[lineIndex_].res;
    }

    bool CheckFirst(){
        if(lineIndex_ == 0){
            return true;
        }
        return false;
    }

    bool CheckLast(){
        if(lineIndex_ + 1 == lrcs.size()){
            return true;
        }
        return false;
    }

private:
    int lineIndex_;                 // 当前行号
    std::vector<Lyric> lrcs;        // 存储歌词

    static LrcFileCtl instance;
};

LrcFileCtl LrcFileCtl::instance;