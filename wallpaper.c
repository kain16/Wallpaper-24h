#define _CRT_SECURE_NO_WARNINGS
#include "wallpaper.h"
#include "transition.h"
#include "config.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shlobj.h>
#include <olectl.h>
#include <ole2.h>

#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"ole32.lib")
#pragma comment(lib,"oleaut32.lib")
#pragma comment(lib,"shell32.lib")
#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib,"msimg32.lib")

// Global variables
HWND g_RealWorkerW = NULL;
char g_CurrentWallpaperPath[260] = { 0 };

// IID_IPicture definition
const IID IID_IPicture = { 0x7BF80980, 0xBF32, 0x101A, {0x8BB, 0x00, 0xAA, 0x00, 0x30, 0x0C, 0xAB, 0xAB} };

#ifndef SHCreateStreamOnFileW
__declspec(dllimport) HRESULT WINAPI SHCreateStreamOnFileW(
    LPCWSTR pszFile,
    DWORD grfMode,
    IStream** ppstm
);
#endif

// Load image to HBITMAP (exported function)
HBITMAP LoadImageToHBITMAP(const char* imagePath)
{
    if (!imagePath || !*imagePath)
        return NULL;

    WCHAR wPath[MAX_PATH] = { 0 };
    MultiByteToWideChar(CP_ACP, 0, imagePath, -1, wPath, MAX_PATH);

    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
        return NULL;

    HBITMAP hBitmap = NULL;
    IStream* pStream = NULL;

    if (SHCreateStreamOnFileW(wPath, STGM_READ, &pStream) == S_OK)
    {
        IPicture* pPic = NULL;
        if (OleLoadPicture(pStream, 0, FALSE, &IID_IPicture, (LPVOID*)&pPic) == S_OK)
        {
            HDC hdc = GetDC(NULL);
            long w, h;
            pPic->lpVtbl->get_Width(pPic, &w);
            pPic->lpVtbl->get_Height(pPic, &h);

            // Local variables: cx/cy (declared before use)
            int cx = MulDiv(w, GetDeviceCaps(hdc, HORZRES), 254);
            int cy = MulDiv(h, GetDeviceCaps(hdc, VERTRES), 254);

            HDC memDC = CreateCompatibleDC(hdc);
            hBitmap = CreateCompatibleBitmap(hdc, cx, cy);
            HBITMAP old = (HBITMAP)SelectObject(memDC, hBitmap);

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
}

// Save HBITMAP to BMP file
BOOL SaveHBITMAPToBMP(HBITMAP hBmp, const char* szFile)
{
    if (!hBmp || !szFile)
        return FALSE;

    BITMAP bmp;
    GetObject(hBmp, sizeof(BITMAP), &bmp);

    BITMAPINFOHEADER bi = { 0 };
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmp.bmWidth;
    bi.biHeight = bmp.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;

    DWORD size = ((bmp.bmWidth * 24 + 31) / 32) * 4 * bmp.bmHeight;
    BYTE* pData = (BYTE*)malloc(size);
    if (!pData) return FALSE;

    GetDIBits(GetDC(NULL), hBmp, 0, bmp.bmHeight, pData, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    HANDLE hFile = CreateFileA(szFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        free(pData);
        return FALSE;
    }

    BITMAPFILEHEADER bf = { 0 };
    bf.bfType = 0x4D42;
    bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bf.bfSize = bf.bfOffBits + size;

    DWORD wr;
    WriteFile(hFile, &bf, sizeof(BITMAPFILEHEADER), &wr, NULL);
    WriteFile(hFile, &bi, sizeof(BITMAPINFOHEADER), &wr, NULL);
    WriteFile(hFile, pData, size, &wr, NULL);

    CloseHandle(hFile);
    free(pData);
    return TRUE;
}

// Initialize wallpaper engine
BOOL WallpaperEngine_Init(WallpaperEngine* engine)
{
    if (!engine) return FALSE;
    memset(engine, 0, sizeof(WallpaperEngine));
    InitializeCriticalSection(&engine->cs);
    engine->isRunning = TRUE;
    engine->currentHour = -1;
    engine->workerW = GetWorkerWWindow();
    g_RealWorkerW = engine->workerW;
    return TRUE;
}

// Cleanup wallpaper engine
void WallpaperEngine_Cleanup(WallpaperEngine* engine)
{
    if (!engine) return;
    engine->isRunning = FALSE;
    if (engine->hSchedulerThread)
    {
        WaitForSingleObject(engine->hSchedulerThread, 3000);
        CloseHandle(engine->hSchedulerThread);
    }
    if (engine->currentBmp) DeleteObject(engine->currentBmp);
    if (engine->nextBmp) DeleteObject(engine->nextBmp);
    DeleteCriticalSection(&engine->cs);
}

// Load schedule from config file
BOOL WallpaperEngine_LoadSchedule(WallpaperEngine* engine, const char* configPath)
{
    if (!engine || !configPath) return FALSE;
    FILE* fp = fopen(configPath, "r");
    if (!fp) return FALSE;

    char line[1024];
    int idx = 0;

    while (fgets(line, sizeof(line), fp) && idx < 24)
    {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r' || strlen(line) < 5)
            continue;

        int h = 0;
        char p[260] = { 0 };
        char d[64] = { 0 };
        DWORD ms = 0;
        int parseCount = sscanf(line, "%d|%259[^|]|%63[^|]|%lu", &h, p, d, &ms);

        if (parseCount == 4)
        {
            engine->schedule[idx].hour = h;
            strncpy_s(engine->schedule[idx].imagePath, _countof(engine->schedule[idx].imagePath), p, _TRUNCATE);
            engine->schedule[idx].transitionMs = ms;
            idx++;

            // Log: parse success
            FILE* log = fopen("wallpaper_log.txt", "a");
            if (log) {
                fprintf(log, "[Parse OK] %d|%s|%lu\n", h, p, ms);
                fclose(log);
            }
        }
        else
        {
            // Log: parse failed
            FILE* log = fopen("wallpaper_log.txt", "a");
            if (log) {
                fprintf(log, "[Parse FAIL] Line: %s, Count: %d\n", line, parseCount);
                fclose(log);
            }
        }
    }

    fclose(fp);
    engine->slotCount = idx;
    return idx > 0;
}

// Set system wallpaper (base function)
BOOL WallpaperEngine_SetSystemWallpaper(const char* imagePath)
{
    if (!imagePath || !*imagePath) return FALSE;

    char ext[20];
    _splitpath_s(imagePath, NULL, 0, NULL, 0, NULL, 0, ext, 20);
    _strlwr_s(ext, 20);

    char tempPath[MAX_PATH];
    BOOL temp = FALSE;

    if (_strcmpi(ext, ".png") == 0 || _strcmpi(ext, ".jpg") == 0 || _strcmpi(ext, ".jpeg") == 0)
    {
        GetTempPathA(MAX_PATH, tempPath);
        strcat_s(tempPath, _countof(tempPath), "~wallpaper.bmp");

        HBITMAP hbmp = LoadImageToHBITMAP(imagePath);
        if (hbmp) {
            SaveHBITMAPToBMP(hbmp, tempPath);
            DeleteObject(hbmp);
            temp = TRUE;
        }
        else {
            FILE* log = fopen("wallpaper_log.txt", "a");
            if (log) {
                fprintf(log, "[Load FAIL] Image: %s\n", imagePath);
                fclose(log);
            }
            return FALSE;
        }
    }
    else
    {
        strcpy_s(tempPath, _countof(tempPath), imagePath);
    }

    BOOL ok = SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, (LPVOID)tempPath, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
    if (temp) DeleteFileA(tempPath);

    // Log: set result
    FILE* log = fopen("wallpaper_log.txt", "a");
    if (log) {
        fprintf(log, "[Set Wallpaper] %s , %s\n", imagePath, ok ? "OK" : "FAIL");
        fclose(log);
    }

    return ok;
}

// Draw bitmap to WorkerW window
BOOL SetWallpaperToDesktop(HWND hwnd, HBITMAP hbmp)
{
    if (!hwnd || !hbmp) return FALSE;

    HDC hdc = GetDC(hwnd);
    HDC mem = CreateCompatibleDC(hdc);
    HBITMAP old = (HBITMAP)SelectObject(mem, hbmp);

    RECT rc;
    GetClientRect(hwnd, &rc);
    StretchBlt(hdc, 0, 0, rc.right, rc.bottom, mem, 0, 0, GetDeviceCaps(hdc, HORZRES), GetDeviceCaps(hdc, VERTRES), SRCCOPY);

    SelectObject(mem, old);
    DeleteDC(mem);
    ReleaseDC(hwnd, hdc);
    return TRUE;
}

// Get WorkerW window (exported function)
HWND GetWorkerWWindow()
{
    HWND hProgman = FindWindowA("Progman", NULL);
    SendMessageA(hProgman, 0x052C, 0, 0);

    HWND hWorkerW = NULL;
    while ((hWorkerW = FindWindowExA(NULL, hWorkerW, "WorkerW", NULL)) != NULL)
    {
        HWND hSHELLDLL_DefView = FindWindowExA(hWorkerW, NULL, "SHELLDLL_DefView", NULL);
        if (hSHELLDLL_DefView != NULL)
        {
            hWorkerW = FindWindowExA(NULL, hWorkerW, "WorkerW", NULL);
            break;
        }
    }
    return hWorkerW ? hWorkerW : GetDesktopWindow();
}

// Base wallpaper set wrapper
static BOOL SetDesktopWallpaper_Base(const char* imagePath) {
    return WallpaperEngine_SetSystemWallpaper(imagePath);
}

// Set wallpaper with transition (exported function)
BOOL WallpaperEngine_SetWallpaper(const char* newImagePath, unsigned long transitionMs) {
    if (newImagePath == NULL || strlen(newImagePath) == 0) {
        return FALSE;
    }

    HWND hWorkerW = GetWorkerWWindow();
    if (hWorkerW == NULL) {
        return FALSE;
    }

    BOOL transitionOk = FALSE;
    if (strlen(g_CurrentWallpaperPath) > 0 && strcmp(g_CurrentWallpaperPath, newImagePath) != 0) {
        transitionOk = WallpaperTransition_C(
            hWorkerW,
            g_CurrentWallpaperPath,
            newImagePath,
            transitionMs
        );
    }

    // Local variable: setOk (declared before use)
    BOOL setOk = SetDesktopWallpaper_Base(newImagePath);
    if (setOk) {
        strncpy_s(g_CurrentWallpaperPath, _countof(g_CurrentWallpaperPath),
            newImagePath, _TRUNCATE);

        FILE* log = fopen("wallpaper_log.txt", "a");
        if (log) {
            fprintf(log, "[Update Path] %s\n", g_CurrentWallpaperPath);
            fclose(log);
        }
    }

    return setOk;
}