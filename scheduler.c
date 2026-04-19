#define _CRT_SECURE_NO_WARNINGS
#include "wallpaper.h"
#include "transition.h"
#include <stdio.h>
#include<string.h>

// 获取当前系统小时（0-23）
int GetCurrentHour(void) {
    time_t now = time(NULL);
    struct tm* local = localtime(&now);
    return local?local->tm_hour:0;
}

// 匹配当前小时对应的壁纸（兼容整点/非整点）
int FindMatchedSlot(WallpaperEngine* engine, int hour) {
    if (!engine || engine->slotCount == 0)return -1;
    int bestIdx = -1;
    int bestHour = -1;

    // 遍历配置，找到最接近当前小时的配置
    for (int i = 0; i < engine->slotCount; i++) {
        if (engine->schedule[i].hour <= hour && engine->schedule[i].hour > bestHour) {
            bestHour = engine->schedule[i].hour;
            bestIdx = i;
        }
    }

    // 日志记录匹配结果
    FILE* log = fopen("wallpaper_log.txt", "a");
    if (log) {
        if (bestIdx >= 0) {
            fprintf(log, "[匹配] 当前小时%d → 配置小时%d → 壁纸：%s\n",
                hour, engine->schedule[bestIdx].hour, engine->schedule[bestIdx].imagePath);
        }
        else {
            fprintf(log, "[匹配] 当前小时%d 无对应配置\n", hour);
        }
        fclose(log);
    }

    return bestIdx;
}

// 调度线程：每分钟检查一次小时，变化则切换壁纸
DWORD WINAPI SchedulerThread(LPVOID param) {
    WallpaperEngine* engine = (WallpaperEngine*)param;
    if (!engine) return 1;
    while (engine->isRunning) {
        int currentHour = GetCurrentHour();
        // 小时变化时切换壁纸
        if (currentHour != engine->currentHour) {
            int slotIdx = FindMatchedSlot(engine, currentHour);
            if (slotIdx >= 0) {
                //日志记录切换行为
                FILE* log = fopen("wallpaper_log.txt", "a");
                if (log) {
                    fprintf(log, "[切换] 小时%d → 小时%d，壁纸：%s\n",
                        engine->currentHour, currentHour, engine->schedule[slotIdx].imagePath);
                    fclose(log);
                }
                engine->currentHour = currentHour;
                // 调用系统API设置壁纸
                WallpaperEngine_SetWallpaper(engine->schedule[slotIdx].imagePath,engine->schedule[slotIdx].transitionMs);
            }
        }

        Sleep(20000); // 每20s检查一次（降低CPU占用）
    }
    return 0;
}

// 启动调度器（初始化时立即切换一次壁纸）
void WallpaperEngine_StartScheduler(WallpaperEngine* engine) {
    // 立即切换到当前小时的壁纸
    int currentHour = GetCurrentHour();
    int slotIdx = FindMatchedSlot(engine, currentHour);
    if (slotIdx >= 0) {
        engine->currentHour = currentHour;
        WallpaperEngine_SetWallpaper(engine->schedule[slotIdx].imagePath,engine->schedule[slotIdx].transitionMs);
    }

    // 创建调度线程
    engine->hSchedulerThread = CreateThread(
        NULL, 0, SchedulerThread, engine, 0, NULL
    );

    if (!engine->hSchedulerThread) {
        MessageBoxA(NULL, "创建调度线程失败！", "错误", MB_ICONERROR);
    }
}

// 停止调度器
void WallpaperEngine_StopScheduler(WallpaperEngine* engine) {
    if (!engine) return;
    engine->isRunning = FALSE;
    if (engine->hSchedulerThread) {//
        WaitForSingleObject(engine->hSchedulerThread, 5000);
        CloseHandle(engine->hSchedulerThread);
        engine->hSchedulerThread = NULL;//
    }
}
void Scheduler_CheckAndSwitch(WallpaperEngine* engine, int currentHour) {
    if (!engine)return;
    int bestIdx = FindMatchedSlot(engine, currentHour);
    if (bestIdx < 0) {
        return;
    }
    // 防止重复
    if (strcmp(engine->schedule[bestIdx].imagePath, g_CurrentWallpaperPath) == 0) {
        return; // 壁纸未变化，无需切换
    }
    // 3. 调用带过渡动画的切换函数（传递配置中的过渡时长）
    WallpaperEngine_SetWallpaper(
        engine->schedule[bestIdx].imagePath,    // 新壁纸路径
        engine->schedule[bestIdx].transitionMs  // 过渡时长（从config.txt读取，比如2000毫秒）
    );
}