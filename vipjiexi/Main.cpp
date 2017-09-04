#include <Windows.h>
#include <windowsx.h>
#include <WinUser.h>
#include <commctrl.h>
#include <thread>
#include "resource.h"
#include "httpmags/tool.h"

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void ShowDownInfo(HWND hwnd);												// 获取单选框和编辑框进行对应的操作
void StartAddComment(HWND hwnd);
HINSTANCE hthisapp;
HWND hdlg;

int WINAPI WinMain(HINSTANCE hThisApp, HINSTANCE hPrevApp, LPSTR lpCmd, int nShow)
{
	hthisapp = hThisApp;
	hdlg = CreateDialog(hthisapp, MAKEINTRESOURCE(IDD_CSDNLOGIN), NULL, (DLGPROC)DlgProc);

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
				EndDialog(hDlg, IDCANCEL);
				hdlg = CreateDialog(hthisapp, MAKEINTRESOURCE(IDD_CSDN), NULL, (DLGPROC)DlgProc);
				ShowWindow(hdlg, SW_SHOW);
				std::thread action(ShowDownInfo, hdlg);
				action.detach();
				//登录成功
				/*MessageBoxA(hDlg, returnInfo.data.c_str(),
					"登录成功", MB_OK | MB_ICONINFORMATION);*/
			}
			else {
				//登录失败
				MessageBoxA(hDlg, returnInfo.strErrorInfo.c_str(), ("提示"), MB_OK | MB_ICONINFORMATION);
			}
			/*std::thread action(performActions, hDlg);
			action.detach();*/
			break;
		}
		case IDC_STARTCOMMENT:
		{
			std::thread action(StartAddComment, hdlg);
			action.detach();
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

void ShowDownInfo(HWND hwnd)
{
	// 获取ListView控件的句柄  
	HWND hListview = GetDlgItem(hwnd, IDC_RESLIST);
	HWND hProgress = GetDlgItem(hwnd, IDC_PROGRESS);
	// 设置ListView的列  
	LVCOLUMN vcl;
	vcl.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	// 第一列  
	vcl.pszText = L"序号";//列标题  
	vcl.cx = 50;//列宽  
	vcl.iSubItem = 0;//子项索引，第一列无子项  
	ListView_InsertColumn(hListview, 0, &vcl);
	// 第二列  
	vcl.pszText = L"资源名称";
	vcl.cx = 200;
	vcl.iSubItem = 1;//子项索引  
	ListView_InsertColumn(hListview, 1, &vcl);
	// 第三列  
	vcl.pszText = L"资源ID";
	vcl.cx = 60;
	vcl.iSubItem = 2;
	ListView_InsertColumn(hListview, 2, &vcl);
	// 第三列  
	vcl.pszText = L"评价状态";
	vcl.cx = 90;
	vcl.iSubItem = 3;
	ListView_InsertColumn(hListview, 3, &vcl);

	int m_totalPage = GetTotalPageNum();
	int totalCounts = 0;
	int pageNum = 1;

	while (TRUE)
	{
		std::vector<DownResourceInfo> vtDownInfo;
		std::wstring id;
		std::string info;
		
		vtDownInfo = GetToCommentList(pageNum);

		if (vtDownInfo.size() > 0)
		{
			//求出数组中元素的个数  
			int arrCount = vtDownInfo.size();
			LVITEM vitem;
			vitem.mask = LVIF_TEXT;
			for (int i = 0; i < arrCount; i++)
			{
				/*
				策略：
				先添加项再设置子项内容
				*/
				id = std::to_wstring(totalCounts);
				vitem.pszText = (LPWSTR)id.c_str();
				vitem.iItem = totalCounts;
				vitem.iSubItem = 0;
				ListView_InsertItem(hListview, &vitem);
				// 设置子项  
				vitem.pszText = (LPWSTR)vtDownInfo[i].strResourceName.c_str();
				vitem.iSubItem = 1;
				ListView_SetItem(hListview, &vitem);
				vitem.iSubItem = 2;
				vitem.pszText = (LPWSTR)vtDownInfo[i].strResourceCurl.c_str();
				ListView_SetItem(hListview, &vitem);
				vitem.iSubItem = 3;
				vitem.pszText = L"";
				ListView_SetItem(hListview, &vitem);

				info = FormatString("待评论资源数量:%d个", ++totalCounts);
				SetDlgItemTextA(hwnd, IDC_SHOWCOUNTS, info.c_str());
			}
		}
		if (pageNum == m_totalPage)
		{
			SetDlgItemTextA(hwnd, IDC_SHOWFINISH, "获取完毕");
			break;
		}
		else
			SetDlgItemTextA(hwnd, IDC_SHOWFINISH, FormatString("第%d页",pageNum).c_str());
		// 给进度条发送消息，传入当前上传的值
		pageNum++;
		SendMessage(hProgress, PBM_SETPOS, pageNum*100/m_totalPage, 0L);
	}
	// 加载完成，激活“开始评论按钮”
	EnableWindow(GetDlgItem(hwnd, IDC_STARTCOMMENT), TRUE);
}

void StartAddComment(HWND hwnd)
{
	// 点击开始评论期间，“开始评论按钮”处于不可点击状态
	EnableWindow(GetDlgItem(hwnd, IDC_STARTCOMMENT), FALSE);

	// 获取ListView控件的句柄  
	HWND hListview = GetDlgItem(hwnd, IDC_RESLIST);
	int listCounts = ListView_GetItemCount(hListview);

	for (int i = 0; i < listCounts; i++)
	{
		wchar_t rescurl[40];
		ListView_GetItemText(hListview, i, 2, rescurl, 40);
		
		std::string sourceId;
		Wchar_tToString(sourceId, rescurl);
		if (AddComment(sourceId))
		{
			ListView_SetItemText(hListview, i, 3, L"+评价成功!");
		}
		else
		{
			i--;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(60000));//60秒评论一次
	}

	// 加载完成，激活“开始评论按钮”
	EnableWindow(GetDlgItem(hwnd, IDC_STARTCOMMENT), TRUE);
}