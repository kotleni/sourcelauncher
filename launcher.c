#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <stdio.h>
#include "config.h"
#include "resource.h"

static HWND hComboExe, hComboGame;
static HWND hChkTemp, hChkDev, hChkConsole, hChkNoBorder, hChkWindow, hChkInternal, hChkSteam;

static int exe_priority(const char *name) {
    const char *prio[] = { "runme.exe", "hl2.exe", "portal2.exe", "left4dead.exe" };
    for (int i = 0; i < 4; i++)
        if (lstrcmpiA(name, prio[i]) == 0)
            return 1;
    return 0;
}

static void detect_exes(void) {
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA("*.exe", &fd);
    int found = 0;
    int defaultIdx = 0;

    const char *names[] = { "runme.exe", "hl2.exe", "portal2.exe", "left4dead.exe" };
    for (int i = 0; i < 4; i++) {
        DWORD attr = GetFileAttributesA(names[i]);
        if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
            if (defaultIdx == 0)
                defaultIdx = found;
            SendMessageA(hComboExe, CB_ADDSTRING, 0, (LPARAM)names[i]);
            found++;
        }
    }

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                !exe_priority(fd.cFileName)) {
                SendMessageA(hComboExe, CB_ADDSTRING, 0, (LPARAM)fd.cFileName);
                found++;
            }
        } while (FindNextFileA(hFind, &fd));
        FindClose(hFind);
    }

    if (!found) {
        SendMessageA(hComboExe, CB_ADDSTRING, 0, (LPARAM)"hl2.exe");
    }
    SendMessageA(hComboExe, CB_SETCURSEL, defaultIdx, 0);
}

static void detect_game_folders(void) {
    const char *folders[] = { "portal2", "left4dead", "portal", "hl2" };
    int found = 0;
    for (int i = 0; i < 4; i++) {
        DWORD attr = GetFileAttributesA(folders[i]);
        if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
            SendMessageA(hComboGame, CB_ADDSTRING, 0, (LPARAM)folders[i]);
            found++;
        }
    }
    if (!found)
        SendMessageA(hComboGame, CB_ADDSTRING, 0, (LPARAM)"portal2");
    SendMessageA(hComboGame, CB_SETCURSEL, 0, 0);
}

static void do_run(HWND hwnd) {
    char cmdline[2048];
    char exe[256];
    char game[256];
    int pos = 0;

    SendMessageA(hComboExe, CB_GETLBTEXT, SendMessageA(hComboExe, CB_GETCURSEL, 0, 0), (LPARAM)exe);
    SendMessageA(hComboGame, CB_GETLBTEXT, SendMessageA(hComboGame, CB_GETCURSEL, 0, 0), (LPARAM)game);

    pos += snprintf(cmdline + pos, sizeof(cmdline) - pos, "\"%s\"", exe);

    if (SendMessageA(hChkTemp, BM_GETCHECK, 0, 0) == BST_CHECKED)
        pos += snprintf(cmdline + pos, sizeof(cmdline) - pos, " -tempcontent");
    if (SendMessageA(hChkDev, BM_GETCHECK, 0, 0) == BST_CHECKED)
        pos += snprintf(cmdline + pos, sizeof(cmdline) - pos, " -dev");
    if (game[0])
        pos += snprintf(cmdline + pos, sizeof(cmdline) - pos, " -game %s", game);
    if (SendMessageA(hChkConsole, BM_GETCHECK, 0, 0) == BST_CHECKED)
        pos += snprintf(cmdline + pos, sizeof(cmdline) - pos, " -console");
    if (SendMessageA(hChkNoBorder, BM_GETCHECK, 0, 0) == BST_CHECKED)
        pos += snprintf(cmdline + pos, sizeof(cmdline) - pos, " -noborder");
    if (SendMessageA(hChkWindow, BM_GETCHECK, 0, 0) == BST_CHECKED)
        pos += snprintf(cmdline + pos, sizeof(cmdline) - pos, " -window");
    if (SendMessageA(hChkInternal, BM_GETCHECK, 0, 0) == BST_CHECKED)
        pos += snprintf(cmdline + pos, sizeof(cmdline) - pos, " -internalbuild");
    if (SendMessageA(hChkSteam, BM_GETCHECK, 0, 0) == BST_CHECKED)
        pos += snprintf(cmdline + pos, sizeof(cmdline) - pos, " -steam");

    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi = {0};

    if (!CreateProcessA(NULL, cmdline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        MessageBoxA(hwnd, "Failed to launch game!", "Error", MB_OK | MB_ICONERROR);
    } else {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

static BOOL CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_INITDIALOG: {
        char title[128];
        snprintf(title, sizeof(title), "%s %s", APP_NAME, VERSION);
        SetWindowTextA(hwnd, title);
        hComboExe = GetDlgItem(hwnd, IDC_COMBO_EXE);
        hComboGame = GetDlgItem(hwnd, IDC_COMBO_GAME);
        hChkTemp = GetDlgItem(hwnd, IDC_CHK_TEMP);
        hChkDev = GetDlgItem(hwnd, IDC_CHK_DEV);
        hChkConsole = GetDlgItem(hwnd, IDC_CHK_CONSOLE);
        hChkNoBorder = GetDlgItem(hwnd, IDC_CHK_NOBORDER);
        hChkWindow = GetDlgItem(hwnd, IDC_CHK_WINDOW);
        hChkInternal = GetDlgItem(hwnd, IDC_CHK_INTERNAL);
        hChkSteam = GetDlgItem(hwnd, IDC_CHK_STEAM);

        detect_exes();
        detect_game_folders();
        SendMessageA(hChkTemp, BM_SETCHECK, BST_CHECKED, 0);
        SendMessageA(hChkInternal, BM_SETCHECK, BST_CHECKED, 0);
        return TRUE;
    }

    case WM_NOTIFY: {
        NMHDR *nm = (NMHDR *)lp;
        if (nm->code == NM_CLICK && nm->idFrom == IDC_SYSLINK_GITHUB) {
            ShellExecuteA(NULL, "open", "https://github.com/kotleni/sourcelauncher", NULL, NULL, SW_SHOW);
            return TRUE;
        }
        break;
    }

    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case IDC_BTN_RUN:
            do_run(hwnd);
            return TRUE;
        case IDCANCEL:
            EndDialog(hwnd, 0);
            return TRUE;
        }
        break;

    case WM_CLOSE:
        EndDialog(hwnd, 0);
        return TRUE;
    }
    return FALSE;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR cmdLine, int nShow) {
    (void)hPrev; (void)cmdLine; (void)nShow;

    InitCommonControlsEx(&(INITCOMMONCONTROLSEX){ sizeof(INITCOMMONCONTROLSEX), ICC_STANDARD_CLASSES | ICC_LINK_CLASS });
    DialogBoxA(hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, DlgProc);
    return 0;
}
