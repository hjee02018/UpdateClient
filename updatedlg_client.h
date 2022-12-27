
// ViVAUpdateDlg.h : 헤더 파일
//

#pragma once

#include "afxcmn.h"
#include <afx.h>
#include <afxsock.h>
#include <stdio.h>

#include <string.h>
#include <direct.h>
#include <Windows.h>

// CViVAUpdateDlg 대화 상자
class CViVAUpdateDlg : public CDialogEx
{
// 생성입니다.
public:
	CViVAUpdateDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.
	virtual BOOL PreTranslateMessage(MSG* pMsg);

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VIVAUPDATE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.

// 구현입니다.
protected:
	HICON m_hIcon;
	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:

	CSocket socket2;

	// ini
	void readiniFile(char*iniFile);
	void writeiniFile(char*iniFile);
	
	char c_date[20];
	char c_ver[20];
	CString	s_path;
	CString server_ip;
	UINT server_port;

	int mode = 1;// 0 : delete&rewrite && 1 : append

	int file_count = 0;
	int saved_size = 0; 

	CListCtrl filelist; // server connection info
	CListBox selectedlist; // client log

	CStatic m_static_status;
	afx_msg void serverConnect2();
	int file_save(char* filename, char*data, int size);
	void file_make(CString dir, CString str);
	void Wait(DWORD dwMillisecond);
	char* split_index(char* str, int index, char delim, int* next_index);
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnStnClickedInfo4();
	void createScrollBar();
	void logResult(const char far*p, ...);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
};
