// ProgressDlg.cpp : implementation file
//

#include "pch.h"
#include "BankApp.h"
#include "afxdialogex.h"
#include "ProgressDlg.h"


// ProgressDlg dialog

IMPLEMENT_DYNAMIC(ProgressDlg, CDialogEx)

ProgressDlg::ProgressDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PROGRESS_DIALOG, pParent)
{

}

ProgressDlg::~ProgressDlg()
{
}

void ProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, PBProgress, m_ProgressCtrl);
}

BOOL ProgressDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_ProgressCtrl.SetRange(0, 100);  // Set range from 0 to 100%

	return TRUE;  // return TRUE unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(ProgressDlg, CDialogEx)
	ON_MESSAGE(WM_USER_PROGRESS_UPDATE, &ProgressDlg::OnProgressUpdate)
	ON_MESSAGE(WM_USER_CLOSE_PROGRESS, &ProgressDlg::OnCloseProgress)
END_MESSAGE_MAP()


// ProgressDlg message handlers

/// @brief Updates the progress bar with the given value.
/// @param value The progress value to set.
void ProgressDlg::updateProgress(int value)
{
	CProgressCtrl* pProgressCtrl = (CProgressCtrl*)GetDlgItem(PBProgress);
	pProgressCtrl->SetPos(value);
}

/// @brief Handles the custom message to update the progress bar.
/// @param wParam The progress value to set.
/// @param lParam Additional message-specific information.
/// @return The result of the message processing.
LRESULT ProgressDlg::OnProgressUpdate(WPARAM wParam, LPARAM lParam)
{
	int progress = (int)wParam;
	m_ProgressCtrl.SetPos(progress);

	return 0;
}

/// @brief Handles the custom message to close the progress dialog.
/// @param wParam Additional message-specific information.
/// @param lParam Additional message-specific information.
/// @return The result of the message processing.
LRESULT ProgressDlg::OnCloseProgress(WPARAM wParam, LPARAM lParam)
{
	// Close and destroy the dialog
	DestroyWindow();

	// Post a custom message to the parent (CBankApp) that the progress dialog is closed
	AfxGetMainWnd()->PostMessage(WM_USER_PROGRESS_DIALOG_CLOSED, 0, 0);

	return 0;
}