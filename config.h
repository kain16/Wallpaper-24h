#include <stddef.h>
#include <windows.h>
#ifndef CONFIG_H
#define CONFIG_H

void GenerateDefaultConfig(const char* path);
void GetConfigPath(char* outPath, size_t size);

typedef struct {
    int hour;          // 小时
    char imgPath[MAX_PATH]; // 图片路径
    char desc[100];    // 描述
    int duration;      // 显示时长（毫秒）
} WallpaperConfig;

int ReadConfigFile(const char* path, WallpaperConfig* outConfigs, int maxCount);

#endif#pragma once
