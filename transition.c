#include <windows.h>
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "msimg32.lib")  // AlphaBlend 依赖
#include "wallpaper.h"
#include "transition.h"
#include <string.h>
#include <stdio.h>

// 全局变量声明（与现有代码对齐）
extern HWND g_RealWorkerW;
extern char g_CurrentWallpaperPath[260];
extern HBITMAP LoadImageToHBITMAP(const char* imagePath);
extern BOOL SetWallpaperToDesktop(HWND hWnd, HBITMAP hBmp);

/*// 工具函数：加载图片为HBITMAP（复用现有逻辑）
static HBITMAP LoadImageToHBITMAP_Wrapper(const char* imagePath) {
    if (!imagePath || !*imagePath) return NULL;

    WCHAR wPath[MAX_PATH] = { 0 };
    MultiByteToWideChar(CP_ACP, 0, imagePath, -1, wPath, MAX_PATH);

    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) return NULL;

    HBITMAP hBitmap = NULL;
    IStream* pStream = NULL;

    if (SHCreateStreamOnFileW(wPath, STGM_READ, &pStream) == S_OK) {
        IPicture* pPic = NULL;
        if (OleLoadPicture(pStream, 0, FALSE, &IID_IPicture, (LPVOID*)&pPic) == S_OK) {
            HDC hdc = GetDC(NULL);
            long w, h;
            pPic->lpVtbl->get_Width(pPic, &w);
            pPic->lpVtbl->get_Height(pPic, &h);

            // 适配屏幕分辨率
            int cx = MulDiv(w, GetDeviceCaps(hdc, HORZRES), 2540);
            int cy = MulDiv(h, GetDeviceCaps(hdc, VERTRES), 2540);

            HDC memDC = CreateCompatibleDC(hdc);
            hBitmap = CreateCompatibleBitmap(hdc, cx, cy);
            HBITMAP old = (HBITMAP)SelectObject(memDC, hBitmap);

            // 渲染图片到HBITMAP
            pPic->lpVtbl->Render(pPic, memDC, 0, 0, cx, cy, 0, h, w, -h, NULL);

            SelectObject(memDC, old);
            DeleteDC(memDC);
            ReleaseDC(NULL, hdc);

            pPic->lpVtbl->Release(pPic);
        }
        pStream->lpVtbl->Release(pStream);
    }
    CoUninitialize();
    return hBitmap;
}*/

// 核心：Alpha混合两张图片（实现渐变过渡）
static void AlphaBlendImages(HDC hdcDest, int x, int y, int w, int h,
    HDC hdcSrc1, HDC hdcSrc2, BYTE alpha) {
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, alpha, AC_SRC_ALPHA };
    // 先绘制旧图
    BitBlt(hdcDest, x, y, w, h, hdcSrc1, 0, 0, SRCCOPY);
    // 以alpha透明度绘制新图（叠加）
    AlphaBlend(hdcDest, x, y, w, h, hdcSrc2, 0, 0, w, h, blend);
}

// 过渡动画线程函数
static DWORD WINAPI TransitionThread(LPVOID param) {
    typedef struct {
        HWND hWnd;//Worker窗口
        const char* oldImagePath;//旧壁纸路径
        const char* newImagePath;//新壁纸路径
        unsigned long durationMs;//过度时长
    } TransitionParam;

    TransitionParam* pParam = (TransitionParam*)param;
    //入参校验
    if (!pParam ||!pParam->hWnd|| !pParam->oldImagePath || !pParam->newImagePath || pParam->durationMs < 100) {
        if(pParam)free(pParam);
        return 1;
    }

    // 1. 加载新旧壁纸为HBITMAP
    HBITMAP hBmpOld = LoadImageToHBITMAP(pParam->oldImagePath);//LoadImageToHBITMAP_Wrapper
    HBITMAP hBmpNew = LoadImageToHBITMAP(pParam->newImagePath); //LoadImageToHBITMAP_Wrapper
    if (!hBmpOld || !hBmpNew) {
        if (hBmpOld) DeleteObject(hBmpOld);
        if (hBmpNew) DeleteObject(hBmpNew);
        free(pParam);
        return 1;
    }

    // 2. 获取屏幕DC和分辨率
    HDC hdcScreen = GetDC(NULL);
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    HWND hWorker = pParam->hWnd;//

    // 3. 创建兼容DC用于绘制
    HDC hdcOld = CreateCompatibleDC(hdcScreen);
    HDC hdcNew = CreateCompatibleDC(hdcScreen);
    HDC hdcBlend = CreateCompatibleDC(hdcScreen);
    HBITMAP hBmpBlend = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);

    // 4. 关联HBITMAP到DC
    HGDIOBJ hOldObj = SelectObject(hdcOld, hBmpOld);
    HGDIOBJ hNewObj = SelectObject(hdcNew, hBmpNew);
    HGDIOBJ hBlendObj = SelectObject(hdcBlend, hBmpBlend);

    // 5. 计算过渡步长（总时长拆分为30帧，保证流畅度）
    int steps = 60;
    DWORD stepTime = pParam->durationMs / steps;
    if (stepTime < 1) stepTime = 1; // 最小步长1ms

    // 6. 逐帧绘制过渡效果
    for (int i = 0; i <= steps; i++) {
        // 计算当前透明度：旧图从255→0，新图从0→255
        BYTE alphaNew = (BYTE)(255 * i / steps);
        BYTE alphaOld = 255 - alphaNew;

        // 清空混合DC
        BitBlt(hdcBlend, 0, 0, screenWidth, screenHeight, NULL, 0, 0, WHITENESS);

        // 绘制旧图（渐变淡出）
        if(alphaOld>0)
        AlphaBlendImages(hdcBlend, 0, 0, screenWidth, screenHeight,
            hdcOld, hdcOld, alphaOld);
        // 绘制新图（渐变淡入）
        if(alphaNew>0)
        AlphaBlendImages(hdcBlend, 0, 0, screenWidth, screenHeight,
            hdcNew, hdcNew, alphaNew);

        // 将混合结果绘制到桌面窗口
        //BitBlt(GetDC(pParam->hWnd), 0, 0, screenWidth, screenHeight,
        //    hdcBlend, 0, 0, SRCCOPY);
        SetWallpaperToDesktop(hWorker, hBmpBlend);
        // 步长延迟（保证过渡时长）
        Sleep(stepTime);
    }

    // 7. 资源释放
    SelectObject(hdcOld, hOldObj);
    SelectObject(hdcNew, hNewObj);
    SelectObject(hdcBlend, hBlendObj);

    DeleteDC(hdcOld);
    DeleteDC(hdcNew);
    DeleteDC(hdcBlend);
    DeleteObject(hBmpBlend);
    DeleteObject(hBmpOld);
    DeleteObject(hBmpNew);
    ReleaseDC(NULL, hdcScreen);
    //ReleaseDC(pParam->hWnd, GetDC(pParam->hWnd));

    //8.释放线程参数
    free((void*)pParam->oldImagePath);
    free((void*)pParam->newImagePath);
    free(pParam);
    return 0;
}

// 对外暴露的过渡动画接口（符合transition.h声明）
BOOL WallpaperTransition_C(
    HWND hWnd,
    const char* oldImagePath,
    const char* newImagePath,
    unsigned long durationMs
) {
    // 入参校验
    if (!hWnd || !oldImagePath || !*oldImagePath || !newImagePath || !*newImagePath) {
        return FALSE;
    }

    // 避免重复过渡（路径相同则直接返回）
    if (strcmp(oldImagePath, newImagePath) == 0) {
        return TRUE;
    }

    // 封装线程参数
    typedef struct {
        HWND hWnd;
        const char* oldImagePath;
        const char* newImagePath;
        unsigned long durationMs;
    } TransitionParam;

    TransitionParam* pParam = (TransitionParam*)malloc(sizeof(TransitionParam));
    if (!pParam) return FALSE;
    
    // 拷贝字符串避免原指针失效
    pParam->hWnd = hWnd;
    pParam->oldImagePath = _strdup(oldImagePath);  
    pParam->newImagePath = _strdup(newImagePath);
    pParam->durationMs = durationMs;

    // 创建过渡线程（后台执行，不阻塞主线程）
    HANDLE hThread = CreateThread(
        NULL, 0, TransitionThread, pParam, 0, NULL
    );

    if (!hThread) {
        free((void*)pParam->oldImagePath);
        free((void*)pParam->newImagePath);
        free(pParam);
        return FALSE;
    }

    // 释放线程句柄（线程仍在后台运行）
    ResumeThread(hThread);
    CloseHandle(hThread);
    return TRUE;
}