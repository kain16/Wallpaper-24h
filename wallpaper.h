#define _CRT_SECURE_NO_WARNINGS
#ifndef WALLPAPER_H
#define WALLPAPER_H
#include <windows.h>
#include <time.h>
#pragma once

#define MAX_PATH_LEN 512
#define HOURS_IN_DAY 24
#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 0x8001
#define ID_TRAY_SETTINGS 0x8002
#define ID_TRAY_ABOUT 0x8003
#define ID_TRAY_REFRESH 0x8004

// 每小时的壁纸配置项
typedef struct {
    int hour;                  // 小时(0-23)
    char imagePath[MAX_PATH_LEN]; // 壁纸图片路径
    char description[64];      // 描述信息
    DWORD transitionMs;        // 过渡时间(毫秒)
} ScheduleItem;

// 壁纸引擎核心结构体（启用完整字段）
typedef struct {
    CRITICAL_SECTION cs;//线程安全临界区
    BOOL isRunning;//运行状态
    HANDLE hSchedulerThread;//调度线程句柄
    int currentHour;//当前匹配的小时
    HBITMAP currentBmp;//当前壁纸位图
    HBITMAP nextBmp;//下一张壁纸位图
    HWND workerW;//workerW窗口
    HBITMAP workerBMP;
    int slotCount;
    struct {
        int hour;
        char imagePath[260];
        DWORD transitionMs;
    } schedule[24];
} WallpaperEngine;

// 全局RealWorkerW（壁纸绘制窗口句柄）
extern HWND g_RealWorkerW;
extern char g_CurrentWallpaperPath[260]; // 记录当前壁纸路径

// 函数声明
int GetCurrentHour();
int FindMatchedSlot(WallpaperEngine* engine, int hour);
DWORD WINAPI SchedulerThread(LPVOID param);
void WallpaperEngine_StartScheduler(WallpaperEngine* engine);
void WallpaperEngine_StopScheduler(WallpaperEngine* engine);
BOOL WallpaperEngine_Init(WallpaperEngine* engine);
BOOL WallpaperEngine_LoadSchedule(WallpaperEngine* engine, const char* configPath);
BOOL WallpaperEngine_SetSystemWallpaper(const char* imagePath);
BOOL InitSystemTray(HWND hwnd);
void ShowContextMenu(HWND hwnd);
void CleanupTray(void);
void WallpaperEngine_Cleanup(WallpaperEngine* engine);
BOOL SetWallpaperToDesktop(HWND hWnd, HBITMAP hBmp);
void RefreshWallpaperConfig(HWND hwnd);
// 补全带过渡动画的函数声明
BOOL WallpaperEngine_SetWallpaper(const char* newImagePath, unsigned long transitionMs);
// 新增：获取系统壁纸绘制窗口WorkerW（关键，解决绘制失效）
HWND GetWorkerWWindow();
HBITMAP LoadImageToHBITMAP(const char* imagePath);

#endif