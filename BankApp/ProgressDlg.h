#pragma once
#include "afxdialogex.h"

#define WM_USER_PROGRESS_UPDATE (WM_USER + 1)
#define WM_USER_CLOSE_PROGRESS (WM_USER + 2)
#define WM_USER_PROGRESS_DIALOG_CLOSED (WM_USER + 3)

// ProgressDlg dialog

class ProgressDlg : public CDialogEx
{
	DECLARE_DYNAMIC(ProgressDlg)

public:
	ProgressDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~ProgressDlg();
	void updateProgress(int value);
	afx_msg LRESULT OnProgressUpdate(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCloseProgress(WPARAM wParam, LPARAM lParam);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROGRESS_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

private:
	CProgressCtrl m_ProgressCtrl;
};
