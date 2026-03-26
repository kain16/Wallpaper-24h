#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include "wallpaper.h"
#include "config.h"
#include <stdio.h>
#include <time.h>

#define WINDOW_CLASS_NAME "WallpaperEngineClass"
#define WINDOW_TITLE "24小时动态壁纸引擎"

static WallpaperEngine g_engine;

void ResumeFromSuspendRefreshWallpaper(void);
int GetCurrentHour(void);//
int FindMatchedSlot(WallpaperEngine* engine, int hour);//
//void InitSystemTray(HWND hwnd);//
void ShowContextMenu(HWND hwnd);//
void CleanupTray(void);//

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        InitSystemTray(hwnd);
        if (!WallpaperEngine_Init(&g_engine)) {
            MessageBoxA(hwnd, "引擎初始化失败", "错误", MB_ICONERROR);
            PostQuitMessage(1);
            return -1;
        }
        char configPath[MAX_PATH];
        GetConfigPath(configPath, MAX_PATH);
        if (!WallpaperEngine_LoadSchedule(&g_engine, configPath)) {
            GenerateDefaultConfig(configPath);
            if (!WallpaperEngine_LoadSchedule(&g_engine, configPath)) {
                MessageBoxA(hwnd, "生成默认配置失败，程序将退出", "错误", MB_ICONERROR);
                PostQuitMessage(1);
                return -1;
            }
            MessageBoxA(hwnd, "已加载默认配置，请编辑配置文件", "提示", MB_OK);
        }
        WallpaperEngine_StartScheduler(&g_engine);
        break;

        time_t now = time(NULL);
        struct tm* local = localtime(&now);
        char debugMsg[256];
        snprintf(debugMsg, sizeof(debugMsg),
            "调度器已启动\n当前时间: %02d:%02d\n当前小时: %d",
            local->tm_hour, local->tm_min, local->tm_hour);
        MessageBoxA(NULL, debugMsg, "调试", MB_OK);
        break;

    case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP)
            ShowContextMenu(hwnd);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_TRAY_EXIT:
            PostQuitMessage(0);
            break;
        case ID_TRAY_SETTINGS:
            MessageBoxA(hwnd, "请直接编辑 config.txt", "设置", MB_OK);
            break;
        case ID_TRAY_REFRESH: {
            WallpaperEngine_StopScheduler(&g_engine);
            char configPath[MAX_PATH];
            GetConfigPath(configPath, MAX_PATH);
            if (WallpaperEngine_LoadSchedule(&g_engine, configPath)) {
                WallpaperEngine_StartScheduler(&g_engine);
                MessageBoxA(hwnd, "配置刷新成功", "提示", MB_OK | MB_ICONINFORMATION);
            }
            else {
                WallpaperEngine_StartScheduler(&g_engine);
                MessageBoxA(hwnd, "配置刷新失败，使用原有配置", "错误", MB_OK | MB_ICONERROR);
            }
            break;
        }
        case ID_TRAY_ABOUT:
            MessageBoxA(hwnd,
                "24小时动态壁纸引擎 v1.0\n按时间自动切换壁纸\n支持 PNG / JPG / BMP",
                "关于", MB_OK | MB_ICONINFORMATION);
            break;
        }
        break;

    case WM_POWERBROADCAST:
        if (wParam == PBT_APMRESUMESUSPEND || wParam == PBT_APMRESUMEAUTOMATIC) {
            ResumeFromSuspendRefreshWallpaper();
        }
        return TRUE;

    case WM_CLOSE:
        DestroyWindow(hwnd);
        PostQuitMessage(0);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void ResumeFromSuspendRefreshWallpaper(void)
{
    FILE* log = fopen("wallpaper_log.txt", "a");
    if (log) {
        time_t now = time(NULL);
        struct tm* local = localtime(&now);
        fprintf(log, "[恢复] 系统唤醒时间：%02d:%02d\n", local->tm_hour, local->tm_min);
        fclose(log);
    }

    WallpaperEngine_StopScheduler(&g_engine);

    int currentHour = GetCurrentHour();
    int slot = FindMatchedSlot(&g_engine, currentHour);

    if (slot >= 0 && slot < g_engine.slotCount) {
        // 设置当前时段壁纸
        if (WallpaperEngine_SetSystemWallpaper(g_engine.schedule[slot].imagePath)) {
            if (log) {
                FILE* log = fopen("wallpaper_log.txt", "a");
                if (log) {
                    fprintf(log, "[恢复] 成功设置壁纸：%s\n", g_engine.schedule[slot].imagePath);
                    fclose(log);
                }
            }
        }
        else {
            if (log) {
                FILE* log = fopen("wallpaper_log.txt", "a");
                if (log) {
                    fprintf(log, "[恢复] 设置壁纸失败：%s\n", g_engine.schedule[slot].imagePath);
                    fclose(log);
                }
            }
        }
    }
    else {
        if (log) {
            FILE* log = fopen("wallpaper_log.txt", "a");
            if (log) {
                fprintf(log, "[恢复] 未找到当前时段(%d点)的壁纸配置\n", currentHour);
                fclose(log);
            }
        }
    }

    // 重启调度器
    WallpaperEngine_StartScheduler(&g_engine);
}

#pragma warning(disable:28251)//临时禁用-C28251
_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HANDLE hMutex = CreateMutexA(NULL, TRUE, "WallpaperEngineMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBoxA(NULL, "壁纸引擎已在运行", "提示", MB_OK);
        if (hMutex != NULL) {
            CloseHandle(hMutex);
            hMutex = NULL;
        }
        return 0;
    }

    WNDCLASSEXA wc = { sizeof(WNDCLASSEXA) };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = WINDOW_CLASS_NAME;
    RegisterClassExA(&wc);

    HWND hwnd = CreateWindowExA(
        0, WINDOW_CLASS_NAME, WINDOW_TITLE,
        0, 0, 0, 0, 0,
        HWND_MESSAGE, NULL, hInstance, NULL
    );

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    WallpaperEngine_StopScheduler(&g_engine);
    WallpaperEngine_Cleanup(&g_engine);
    CleanupTray();
    if(hMutex != NULL) {
        CloseHandle(hMutex);
        hMutex = NULL;
    }
    return 0;
}