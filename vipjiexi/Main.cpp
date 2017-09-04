#include <Windows.h>
#include <windowsx.h>
#include <WinUser.h>
#include <commctrl.h>
#include <thread>
#include "resource.h"
#include "httpmags/tool.h"

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void ShowDownInfo(HWND hwnd);												// ��ȡ��ѡ��ͱ༭����ж�Ӧ�Ĳ���
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
		// ���öԻ����ͼ�� 
		//SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(hgInst, MAKEINTRESOURCE(IDI_ICON1)));
		return 0;
	}
	case WM_SYSCOMMAND:
	{
		if (wParam == SC_CLOSE)
		{
			PostQuitMessage(0);//�˳�     
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
				//��¼�ɹ�
				/*MessageBoxA(hDlg, returnInfo.data.c_str(),
					"��¼�ɹ�", MB_OK | MB_ICONINFORMATION);*/
			}
			else {
				//��¼ʧ��
				MessageBoxA(hDlg, returnInfo.strErrorInfo.c_str(), ("��ʾ"), MB_OK | MB_ICONINFORMATION);
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
	// ��ȡListView�ؼ��ľ��  
	HWND hListview = GetDlgItem(hwnd, IDC_RESLIST);
	HWND hProgress = GetDlgItem(hwnd, IDC_PROGRESS);
	// ����ListView����  
	LVCOLUMN vcl;
	vcl.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	// ��һ��  
	vcl.pszText = L"���";//�б���  
	vcl.cx = 50;//�п�  
	vcl.iSubItem = 0;//������������һ��������  
	ListView_InsertColumn(hListview, 0, &vcl);
	// �ڶ���  
	vcl.pszText = L"��Դ����";
	vcl.cx = 200;
	vcl.iSubItem = 1;//��������  
	ListView_InsertColumn(hListview, 1, &vcl);
	// ������  
	vcl.pszText = L"��ԴID";
	vcl.cx = 60;
	vcl.iSubItem = 2;
	ListView_InsertColumn(hListview, 2, &vcl);
	// ������  
	vcl.pszText = L"����״̬";
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
			//���������Ԫ�صĸ���  
			int arrCount = vtDownInfo.size();
			LVITEM vitem;
			vitem.mask = LVIF_TEXT;
			for (int i = 0; i < arrCount; i++)
			{
				/*
				���ԣ�
				���������������������
				*/
				id = std::to_wstring(totalCounts);
				vitem.pszText = (LPWSTR)id.c_str();
				vitem.iItem = totalCounts;
				vitem.iSubItem = 0;
				ListView_InsertItem(hListview, &vitem);
				// ��������  
				vitem.pszText = (LPWSTR)vtDownInfo[i].strResourceName.c_str();
				vitem.iSubItem = 1;
				ListView_SetItem(hListview, &vitem);
				vitem.iSubItem = 2;
				vitem.pszText = (LPWSTR)vtDownInfo[i].strResourceCurl.c_str();
				ListView_SetItem(hListview, &vitem);
				vitem.iSubItem = 3;
				vitem.pszText = L"";
				ListView_SetItem(hListview, &vitem);

				info = FormatString("��������Դ����:%d��", ++totalCounts);
				SetDlgItemTextA(hwnd, IDC_SHOWCOUNTS, info.c_str());
			}
		}
		if (pageNum == m_totalPage)
		{
			SetDlgItemTextA(hwnd, IDC_SHOWFINISH, "��ȡ���");
			break;
		}
		else
			SetDlgItemTextA(hwnd, IDC_SHOWFINISH, FormatString("��%dҳ",pageNum).c_str());
		// ��������������Ϣ�����뵱ǰ�ϴ���ֵ
		pageNum++;
		SendMessage(hProgress, PBM_SETPOS, pageNum*100/m_totalPage, 0L);
	}
	// ������ɣ������ʼ���۰�ť��
	EnableWindow(GetDlgItem(hwnd, IDC_STARTCOMMENT), TRUE);
}

void StartAddComment(HWND hwnd)
{
	// �����ʼ�����ڼ䣬����ʼ���۰�ť�����ڲ��ɵ��״̬
	EnableWindow(GetDlgItem(hwnd, IDC_STARTCOMMENT), FALSE);

	// ��ȡListView�ؼ��ľ��  
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
			ListView_SetItemText(hListview, i, 3, L"+���۳ɹ�!");
		}
		else
		{
			i--;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(60000));//60������һ��
	}

	// ������ɣ������ʼ���۰�ť��
	EnableWindow(GetDlgItem(hwnd, IDC_STARTCOMMENT), TRUE);
}