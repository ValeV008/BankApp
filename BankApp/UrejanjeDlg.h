#pragma once

#include "afxdialogex.h"
#include "DatabaseHelper.h" // Include the new header file
#include <afxdb.h>

// CUrejanjeDlg dialog

class CUrejanjeDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CUrejanjeDlg)

public:
    CUrejanjeDlg(CWnd* pParent = nullptr);   // standard constructor
	CUrejanjeDlg(NalogData& nalogData, CWnd* pParent = nullptr, bool isPregled = false);
    virtual ~CUrejanjeDlg();
    bool UpdateNalogData();
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_UREJANJE_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()

private:
    NalogData& m_nalogData;
    bool m_isPregled;
    CString getComboBoxKoda(int controlID);
    bool checkData();
    bool checkIfAnyEmpty();
	void populatePlacnikData();
    void populatePrejemnikData();
    void initializeDefaultValues();
    void populateMetadata();
    void disableEditing();
	void disableEditingStaticData();
    void updatePlacnikData();
	void populatePlacnikStaticData();
    void updatePrejemnikData();
};