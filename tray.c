#define _CRT_SECURE_NO_WARNINGS
#include "wallpaper.h"
#include <shellapi.h>
#include <windows.h>

#define TRAY_ICON_ID 1
#define WM_SHOW_SETTINGS (WM_USER + 1)
//全局托盘数据和菜单句柄
static NOTIFYICONDATA nid;
static HMENU hMenu;

BOOL InitSystemTray(HWND hwnd) {
    memset(&nid, 0, sizeof(NOTIFYICONDATA));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = TRAY_ICON_ID;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    strcpy(nid.szTip, "24小时动态壁纸引擎");

    //添加托盘图标
    if (!Shell_NotifyIcon(NIM_ADD, &nid)) {
        return FALSE;
    }

    // 创建右键菜单
    hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, ID_TRAY_SETTINGS, "设置(&S)");
    AppendMenu(hMenu, MF_STRING, ID_TRAY_REFRESH, "刷新配置(&R)");
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL); //分隔线
    AppendMenu(hMenu, MF_STRING, ID_TRAY_ABOUT, "关于(&A)");
    AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, "退出(&X)");
   
    return TRUE;
}
//显示右键菜单
void ShowContextMenu(HWND hwnd) {
    POINT pt;
    GetCursorPos(&pt);
    //显示菜单
    SetForegroundWindow(hwnd);
    TrackPopupMenu(hMenu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON,
        pt.x, pt.y, 0, hwnd, NULL);
    PostMessage(hwnd, WM_NULL, 0, 0);//激活菜单
}
//清楚系统托盘
void CleanupTray(void) {
    Shell_NotifyIcon(NIM_DELETE, &nid);
    DestroyMenu(hMenu);
}