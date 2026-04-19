#pragma once
#ifndef TRANSITION_H
#define TRANSITION_H

#include <windows.h>

/**
 * 壁纸过渡动画核心函数
 * @param hWnd 窗口句柄（桌面窗口句柄）
 * @param oldImagePath 旧壁纸路径
 * @param newImagePath 新壁纸路径
 * @param durationMs 过渡时长（毫秒）
 * @param transitionType 过渡类型（0=淡入淡出，1=滑动，2=渐变等）
 * @return BOOL 成功返回TRUE，失败返回FALSE
 */
BOOL WallpaperTransition_C(
    HWND hWnd,
    const char* oldImagePath,
    const char* newImagePath,
    unsigned long durationMs
);

#endif // TRANSITION_H