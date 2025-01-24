
// BankAppDlg.h : header file
//

#pragma once

#include <vector>
#include "ProgressDlg.h"
#include "UrejanjeDlg.h"
#include "PlacnikStaticData.h"
#include "HelperFunctions.h"

#define WM_UPDATE_UI (WM_USER + 1)

enum FilterType
{
	NAZIV_PREJEMNIKA = 0,
	ZNESEK,
	ROK_PLACILA,
	NUJNOST_PLACILA,
	STATUS
};

enum FilterColumn
{
	COL_NUJNOST_PLACILA = 1,
	COL_NAZIV_PREJEMNIKA = 4,
	COL_ROK_PLACILA = 6,
	COL_STATUS = 7,
	COL_ZNESEK = 8
};

struct placnikStaticData
{
	CString IBAN;
	CString ime;
	CString naslov;
	CString kraj;
	CString drzava;
};

// CBankAppDlg dialog
class CBankAppDlg : public CDialogEx
{
// Construction
public:
	CBankAppDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BANKAPP_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedbtnurejanjenalog();
	afx_msg void OnBnClickedbtnnovnalog();
	afx_msg void OnBnClickedbtnbrisanjenalog();
	afx_msg void OnBnClickedbtnposiljanjenalog();
	afx_msg void OnBnClickedbtnpreglednalog();
	afx_msg void OnNMDblclklstseznamnalog(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnChangeMcfiltertext();

private:
	std::vector<int> RetrieveCheckedItems();
	void viewDialog(int selectedItem);
	void SetupList();
	void setupFilter();
	void populateList();
	int m_elapsedTime;
	ProgressDlg* pDlg;
	void OnTimer(UINT_PTR nIDEvent);
	void filterList(const CString& filterText, FilterColumn col);

	std::vector<std::tuple<CString, CString, NalogData>> m_allItems;
	ProgressDlg* m_pProgressDlg;  // Pointer to the modeless progress dialog
	CWinThread* m_pThread;        // Pointer to the worker thread
	afx_msg LRESULT OnProgressDialogClosed(WPARAM wParam, LPARAM lParam);  // Handler for progress dialog closed
};
