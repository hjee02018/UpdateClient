
// ViVAUpdateDlg.cpp : 구현 파일
//


#include "stdafx.h"
#include "ViVAUpdate.h"
#include "ViVAUpdateDlg.h"
#include "afxdialogex.h"
#include "targetver.h"

#include <Shlwapi.h>
#pragma comment( lib, "shlwapi.lib" )
#pragma warning(disable:4996)



//added
#include <iostream>
#include <string>
#include <WS2tcpip.h>


#pragma comment(lib, "ws2_32.lib")


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MAXBUF 1004096
#define MAXBUF2 102400

// CViVAUpdateDlg.cpp : 구현 파일
//
using namespace std;


CViVAUpdateDlg::CViVAUpdateDlg(CWnd* pParent)
	: CDialogEx(IDD_VIVAUPDATE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CViVAUpdateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, selectedlist);
	DDX_Control(pDX, IDC_LIST4, filelist);
}

BEGIN_MESSAGE_MAP(CViVAUpdateDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SYSCOMMAND()
	//ON_BN_CLICKED(IDC_BUTTON2, &CViVAUpdateDlg::OnBnClickedButton2) // disconnetc btn
	//ON_BN_CLICKED(IDC_BUTTON1, &CViVAUpdateDlg::OnBnClickedButton1)
	ON_WM_SIZING()
END_MESSAGE_MAP()

BOOL CViVAUpdateDlg::OnInitDialog()
{

	if (::IsWindow(this->GetSafeHwnd()) != NULL)
	{
		this->EnableWindow(TRUE);
	}
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);
	SetWindowText(_T("ViVAUpdate_Client"));
	ShowWindow(SW_SHOW);


	CRect rect;
	readiniFile(".\\UpdateVersion.ini");

	filelist.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES); //LVS_EX_CHECKBOXES 
	filelist.GetClientRect(&rect);
	selectedlist.GetClientRect(&rect);
	filelist.InsertColumn(0, "connection time", LVCFMT_CENTER, 150);
	filelist.InsertColumn(1, "server ip", LVCFMT_CENTER, 150); //main serverIP
	filelist.InsertColumn(2, _T("Alive"), LVCFMT_CENTER, 70);
	filelist.InsertColumn(3, _T("-"), LVCFMT_CENTER, 70);
	filelist.InsertColumn(3, _T("-"), LVCFMT_CENTER, 70);

	while (1)
	{
		serverConnect2();
		Wait(30000);
		socket2.Close();
	}
	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CViVAUpdateDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}


HCURSOR CViVAUpdateDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CViVAUpdateDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CViVAUpdateDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else if (nID == SC_CLOSE)
	{
		if (MessageBox("서버와 연결을 종료하겠습니까?", "closing socket", MB_YESNO) == IDYES)
		{
			socket2.Close();
			CDialogEx::OnSysCommand(nID, lParam);
		}
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}


BOOL CViVAUpdateDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_SPACE || pMsg->wParam == VK_RETURN)
		{
			return TRUE;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CViVAUpdateDlg::createScrollBar()
{
	CString str;
	CSize sz;
	int dx = 0;
	CDC* pDC = selectedlist.GetDC();
	for (int i = 0; i < selectedlist.GetCount(); i++)
	{
		selectedlist.GetText(i, str);
		sz = pDC->GetTextExtent(str);

		if (sz.cx > dx)
			dx = sz.cx;
	}
	selectedlist.ReleaseDC(pDC);
	selectedlist.SetHorizontalExtent(dx);
}

// 서버와 통신하기 위한 기초 정보를 .ini파일에서 읽어옴
void CViVAUpdateDlg::readiniFile(char *iniFile)
{
	char server_ip[32] = { NULL, };
	char update_folder[256] = { NULL, };
	GetPrivateProfileString("UpdateVersion", "LastUpdate_Date", NULL, c_date, sizeof(c_date), iniFile);
	GetPrivateProfileString("UpdateVersion", "LastUpdate_Version", NULL, c_ver, sizeof(c_ver), iniFile);
	GetPrivateProfileString("UpdateVersion", "UpdateFolder", NULL, update_folder, sizeof(update_folder), iniFile);
	GetPrivateProfileString("UpdateServer", "IP_ADDRESS", NULL, server_ip, sizeof(server_ip), iniFile);
	server_port = GetPrivateProfileInt("UpdateServer", "PORT", 9999, iniFile);
	mode = GetPrivateProfileInt("UpdateVersion", "MODE", 9999, iniFile);
	SetDlgItemText(IDC_INFO2, c_date);
	SetDlgItemText(IDC_INFO4, update_folder);
	SetDlgItemText(IDC_INFO, c_ver);
	s_path = update_folder;
	this->server_ip = server_ip;
}

// 지속적인 업데이트를 위해 최신 업데이트 정보를 기록
void CViVAUpdateDlg::writeiniFile(char*iniFile)
{
	WritePrivateProfileString("UpdateVersion", "LastUpdate_Version", c_ver, iniFile);
	SetDlgItemText(IDC_INFO, c_ver);
	WritePrivateProfileString("UpdateVersion", "LastUpdate_Date", c_date, iniFile);
	SetDlgItemText(IDC_INFO2, c_date);
}


void CViVAUpdateDlg::Wait(DWORD dwMillisecond)
{
	MSG msg;
	DWORD dwStart;
	dwStart = GetTickCount();
	while (GetTickCount() - dwStart < dwMillisecond)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}


void CViVAUpdateDlg::file_make(CString dir,CString str)
{
	CString preFix(_T("")), token(_T(""));
	int start = 0, end;
	while ((end = dir.Find('\\', start)) >= 0)
	{
		CString token = dir.Mid(start, end - start);
		CreateDirectory(preFix + token, NULL);
		preFix += token;
		preFix += _T("/");
		start = end + 1;
	}
	token = dir.Mid(start);
	CreateDirectory(preFix + token, NULL);
	// dir : C:\Users\hrkim\Documents\Visual Studio 2015\Projects\ViVA_Update\ExecUpdate\TEST\SURV
	// str : C:\Users\hrkim\Documents\Visual Studio 2015\Projects\ViVA_Update\ExecUpdate\TEST\SURV\frozen_graph.pb

	CFile file;
	file.Open(str, CFile::modeCreate, NULL);

	//if (fopen(str, "w") == NULL)
	//	logResult("err : file(0)directory invalid");

	//FILE* fp;
	//logResult(str);
	//fopen_s(&fp, str, "w+");
	//if (fp == NULL)
	//	selectedlist.InsertString(0, _T("err : file(0) directory invalid"));

}


int CViVAUpdateDlg::file_save(char* filename, char* data, int size)
{
	FILE* fp;
	fopen_s(&fp, filename, "ab");
	if (fp == NULL)
		selectedlist.InsertString(0, _T("err : file directory invalid"));
	fwrite(data, size, 1, fp);
	fclose(fp);
	return 0;
}

char* CViVAUpdateDlg::split_index(char* str, int index, char delim, int* next_index)
{
	char* sp;
	char* dp;
	int n = 0;
	int len = 0;

	dp = sp = str;
	*next_index = 0;
	while (*sp)
	{
		if (*sp == delim)
		{
			if (n == index)
			{
				(*next_index)++;
				char* res = (char*)malloc(len + 1);
				memcpy(res, dp, len);
				res[len] = 0;
				return res;
			}
			dp = sp + 1;
			len = -1;
			n++;
		}
		len++;
		sp++;
		(*next_index)++;
	}
	return 0;
}

void CViVAUpdateDlg::logResult(const char far*p, ...)
{
	va_list args;
	va_start(args, p);

	int n;
	char szBuffer[512] = { 0 };

	SYSTEMTIME cur;
	GetLocalTime(&cur);
	char buffer[512];
	if ((n = _vsnprintf(szBuffer, 512 - 1, p, args)) > 0)
	{
		try
		{
			if (selectedlist.GetCount() > 30000)
				selectedlist.ResetContent();

			sprintf(buffer, "[%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d] %s",
				cur.wYear, cur.wMonth, cur.wDay, cur.wHour, cur.wMinute, cur.wSecond,
				szBuffer);
			selectedlist.InsertString(0, buffer);
		}
		catch (...)
		{
		}
	}
	va_end(args);
}

// 서버와 소켓 통신 및 파일 전송을 담당하는 메인 함수
void CViVAUpdateDlg::serverConnect2()
{
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);
	AfxSocketInit(NULL);
	socket2.Create();
	DWORD recvTimeout = 10000;
	setsockopt(socket2, SOL_SOCKET, SO_RCVTIMEO, (char*)&recvTimeout, sizeof(recvTimeout));
	if (!socket2.Connect(server_ip, server_port))
	{
		logResult("socket connection err...\n");
		filelist.SetItemText(0, 2, _T("x"));
		socket2.Close();
		return;
	}
	char buffer[512];
	sprintf(buffer, _T("success connect Server %s : %d\n"), server_ip, server_port);
	logResult(buffer);

	CTime ctime = CTime::GetCurrentTime();
	CString txt;
	txt.Format("%d-%02d-%02d %02d:%02d:%02d", ctime.GetYear(), ctime.GetMonth(), ctime.GetDay(), ctime.GetHour(), ctime.GetMinute(), ctime.GetSecond());
	filelist.InsertItem(0, txt);
	filelist.SetItemText(0, 1, server_ip);
	filelist.SetItemText(0, 2, _T("v"));

	CString strbuf = _T("");
	int step = 0;
	int next_index = 0;
	int	cbRcvd = 0;

	char filename[256];
	int file_index = 0;
	int file_size = 0;
	int save_size = 0;
	saved_size = 0;
	char buffer2[MAXBUF];

	CString	strBuffer = _T("");
	::memset(buffer2, 0, sizeof(buffer2));
	logResult("Request Update Info File...\n");

  // 서버가 연결을 종료 할 때까지
	while (1)
	{
		switch (step)
		{
		case 0:
		{
			socket2.Send("GET_VERSION", strlen("GET_VERSION"), 0);
			step++;
		}
		break;
		case 2:
		{
			socket2.Send("GET_UPDATE_COUNT", strlen("GET_UPDATE_COUNT"), 0);
			step++;
		}
		break;
		case 4:
		{
			char tmp5[20];
			sprintf(tmp5, "GET_UPDATE_FILE:%d", file_index);
			socket2.Send(tmp5, strlen(tmp5), 0);
			step++;
		}
		break;
		case 99: // file receive end | version no updated
		{
			step = 0;
			// Wait(30000);
		}
		break;
		}

		if (step == 0)
		{
			Wait(60000);
			logResult("recheck...\n");
			continue;
			break;
		}

		memset(buffer2, 0x00, sizeof(buffer2));
		if ((cbRcvd = socket2.Receive(buffer2, MAXBUF)) <= 0)
		{
			logResult("socket recv err...\n");
			filelist.SetItemText(0, 2, _T("x"));
			break;
		}
		else
		{
			if (step == 6)
			{
				file_save(filename, buffer2, cbRcvd);
				saved_size += cbRcvd;
				save_size += cbRcvd;
				if (file_size <= save_size)
				{
					if (file_count <= (file_index + 1))
					{
						CTime ctime = CTime::GetCurrentTime();
						CString txt;
						txt.Format("%d-%02d-%02d %02d:%02d:%02d", ctime.GetYear(), ctime.GetMonth(), ctime.GetDay(), ctime.GetHour(), ctime.GetMinute(), ctime.GetSecond());
						strcpy(c_date, txt);
						SetDlgItemText(IDC_INFO2, txt);
						SetDlgItemText(IDC_INFO, c_ver);
						SetDlgItemInt(IDC_INFO6, file_count);
						SetDlgItemInt(IDC_INFO5, saved_size);
						writeiniFile(".\\UpdateVersion.ini");
						logResult("file receive end...\n");
						step = 99;
					}
					else
					{
						file_index++;
						step = 4;
					}
				}
				continue;
			}

			char* funs = split_index(buffer2, 0, ':', &next_index);
			if (funs == 0)
				continue;

			char func[512];
			sprintf(func, "function=[%s]\n", funs);
			logResult(func);
			next_index = 0;
			switch (step)
			{
			case 1:
			{
				char*vers = split_index(buffer2, 1, ':', &next_index);
				char buf2[20];
				sprintf(buf2, "Version = %s", vers);

				if (strcmp(c_ver, vers) == 0)
				{
					free(vers);
					step = 99;
					break;
				}
				else
					logResult(buf2);

				strcpy(c_ver, vers);
				free(vers);
				step++;
			}
			break;
			case 3:
			{
				char* cs = split_index(buffer2, 1, ':', &next_index);
				file_count = atoi(cs);
				file_index = 0;

				char buf3[20];
				sprintf(buf3, "File Count = %d", file_count);
				logResult(buf3);
				free(cs);
				step++;
			}
			break;
			case 5:
			{
				char* cs = split_index(buffer2, 1, ':', &next_index);
				char* fs = split_index(buffer2, 2, ':', &next_index);
				char* ss = split_index(buffer2, 3, ':', &next_index);
				file_index = atoi(cs);
				file_size = atoi(ss);
				strcpy(filename, s_path);
				strcat(filename, "\\");

				char txt[512];
				char* test2 = NULL;
				char*test = strtok_s(fs, "/", &test2);

				for (int i = 0; i < strlen(test2); i++)
					if (test2[i] == '/')
						test2[i] = '\\';

				sprintf_s(txt, "Update file : %s", test2);
				logResult(txt);
				strcat(filename, test2);

				CString str = (LPCSTR)filename;
				CString dir = str.Left(str.ReverseFind('\\'));
				file_make(dir,str);

				sprintf_s(txt, "File Index = %d:File Name = %s:File Size = %dbyte:NextIndex = %d:recvLen = %d..", file_index, filename, file_size, next_index, cbRcvd);
				logResult(txt);
				createScrollBar();
				save_size = 0;
				cbRcvd = cbRcvd - next_index;
				if (cbRcvd > 0)
				{
					file_save(filename, buffer2 + next_index, cbRcvd);
					save_size += cbRcvd;
				} // 
				free(cs);
				free(fs);
				free(ss);
				step++;
			}
			break;
			}
			free(funs);
			memset(buffer2, 0x00, sizeof(buffer2));
		}
		Wait(1000);
	}
	socket2.Close();
	WSACleanup();
	return;
}

void CViVAUpdateDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialogEx::OnSizing(fwSide, pRect);
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
}
