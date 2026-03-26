#define _CRT_SECURE_NO_WARNINGS
#include "config.h"
#include <stdio.h>
#include <shlobj.h>
#include <string.h>
#include <windows.h>

// 生成默认配置文件
void GenerateDefaultConfig(const char* path) {
    FILE* fp = fopen(path, "w");
    if (!fp) return;

    fprintf(fp, "# 24小时壁纸配置文件\n");
    fprintf(fp, "# 格式: 小时|图片路径|描述|过渡时间(毫秒)\n\n");

    // 生成示例24小时配置
    struct {
        int hour;
        const char* desc;
        const char* filename;
    } defaults[] = {
        {0, "深夜", "midnight.jpg"},
        {5, "黎明前", "predawn.jpg"},
        {6, "日出", "sunrise.jpg"},
        {8, "早晨", "morning.jpg"},
        {12, "正午", "noon.jpg"},
        {16, "下午", "afternoon.jpg"},
        {18, "日落", "sunset.jpg"},
        {19, "黄昏", "dusk.jpg"},
        {21, "夜晚", "night.jpg"},
        {-1, NULL, NULL}
    };

    char exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);
    char* lastSlash = strrchr(exePath, '\\');
    if (lastSlash) *(lastSlash + 1) = '\0';

    for (int i = 0; defaults[i].hour >= 0; i++) {
        char fullPath[MAX_PATH];
        snprintf(fullPath, MAX_PATH, "%s\\wallpapers\\%s",exePath, defaults[i].filename);
        fprintf(fp, "%d|%s|%s|2000\n",
            defaults[i].hour, fullPath, defaults[i].desc);
    }

    fclose(fp);
    // 日志记录配置生成
    FILE* log = fopen("wallpaper_log.txt", "a");
    if (log) {
        fprintf(log, "[配置] 生成默认配置文件：%s\n", path);
        fclose(log);
    }
}

// 获取配置文件路径
void GetConfigPath(char* outPath, size_t size) {
    // 尝试多个位置
    char exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);

    // 去掉 exe 文件名，得到目录
    char* lastSlash = strrchr(exePath, '\\');
    if (lastSlash) {
        *(lastSlash + 1) = '\0';
    }
    snprintf(outPath, size, "%sconfig.txt", exePath);
}
int ReadConfigFile(const char* path, WallpaperConfig* outConfigs, int maxCount) {
    FILE* fp = fopen(path, "r");
    if (!fp) {
        // 配置文件不存在则生成默认配置
        GenerateDefaultConfig(path);
        fp = fopen(path, "r");
        if (!fp) return 0; // 仍打不开则返回0
    }

    char line[512];
    int configCount = 0;
    while (fgets(line, sizeof(line), fp) != NULL && configCount < maxCount) {
        // 跳过注释行和空行
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;

        // 解析行格式：小时|图片路径|描述|显示时长
        WallpaperConfig cfg;
        memset(&cfg, 0, sizeof(WallpaperConfig));
        if (sscanf(line, "%d|%[^|]|%[^|]|%d",
            &cfg.hour, cfg.imgPath, cfg.desc, &cfg.duration) == 4) {
            outConfigs[configCount++] = cfg;
        }
    }

    fclose(fp);
    return configCount;
}