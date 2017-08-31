#include <Windows.h>
#include <windowsx.h>
#include <WinUser.h>
#include <commctrl.h>
#include <thread>
#include "resource.h"
#include "httpmags/tool.h"

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void performActions(HWND hwnd);												// 获取单选框和编辑框进行对应的操作

int WINAPI WinMain(HINSTANCE hThisApp, HINSTANCE hPrevApp, LPSTR lpCmd, int nShow)
{
	HWND hdlg = CreateDialog(hThisApp, MAKEINTRESOURCE(IDD_CSDNLOGIN), NULL, (DLGPROC)DlgProc);

	if (!hdlg)
	{
		return 0;
	}
	ShowWindow(hdlg, SW_SHOW);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		// 设置对话框的图标 
		//SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(hgInst, MAKEINTRESOURCE(IDI_ICON1)));
		return 0;
	}
	case WM_SYSCOMMAND:
	{
		if (wParam == SC_CLOSE)
		{
			PostQuitMessage(0);//退出     
		}
		return 0;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDOK:
		{
			char m_user[MAX_PATH], m_pwd[MAX_PATH];
			GetDlgItemTextA(hDlg, IDC_USER, m_user, MAX_PATH);
			GetDlgItemTextA(hDlg, IDC_PWD, m_pwd, MAX_PATH);
			ReturnInfo returnInfo = LoginServer(m_user, m_pwd);
			if (returnInfo.bReturn == TRUE)
			{
				//登录成功
				MessageBoxA(hDlg, returnInfo.data.c_str(),
					"登录成功", MB_OK | MB_ICONINFORMATION);
			}
			else {
				//登录失败
				MessageBoxA(hDlg, returnInfo.strErrorInfo.c_str(), ("提示"), MB_OK | MB_ICONINFORMATION);
			}
			/*std::thread action(performActions, hDlg);
			action.detach();*/
			break;
		}
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			break;
		default:
			break;
		}
		break;
	}
	default:
		break;
	}
	return (INT_PTR)FALSE;
}

void performActions(HWND hwnd)
{

}