
// BankAppDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "BankApp.h"
#include "BankAppDlg.h"
#include "DatabaseHelper.h"
#include "afxdialogex.h"
#pragma comment(lib, "msxml6.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

std::vector<CString> g_paidItems;
std::vector<CString> g_invalidItems;
float g_value = 0;


CBankAppDlg::CBankAppDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_BANKAPP_DIALOG, pParent)
{
}

void CBankAppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CBankAppDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CBankAppDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CBankAppDlg::OnBnClickedCancel)
	ON_BN_CLICKED(btnNovNalog, &CBankAppDlg::OnBnClickedbtnnovnalog)
	ON_BN_CLICKED(btnUrejanjeNalog, &CBankAppDlg::OnBnClickedbtnurejanjenalog)
	ON_BN_CLICKED(btnBrisanjeNalog, &CBankAppDlg::OnBnClickedbtnbrisanjenalog)
	ON_BN_CLICKED(btnPosiljanjeNalog, &CBankAppDlg::OnBnClickedbtnposiljanjenalog)
	ON_BN_CLICKED(btnPregledNalog, &CBankAppDlg::OnBnClickedbtnpreglednalog)
	ON_NOTIFY(NM_DBLCLK, lstSeznamNalog, &CBankAppDlg::OnNMDblclklstseznamnalog)
	ON_WM_TIMER()
	ON_MESSAGE(WM_USER_PROGRESS_DIALOG_CLOSED, &CBankAppDlg::OnProgressDialogClosed)
	ON_EN_CHANGE(MCFilterText, &CBankAppDlg::OnEnChangeMcfiltertext)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

void CBankAppDlg::OnClose()
{
	//AfxMessageBox(_T("Hvala za uporabo aplikacije!"));

	// Perform any necessary cleanup here
	DatabaseHelper::CloseDatabaseConnection();
	
	CDialogEx::OnClose();
}

BOOL CBankAppDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//open database connection
	if (!DatabaseHelper::OpenDatabaseConnection())
	{
		AfxMessageBox(_T("Gonilniki za podatkovno bazo niso bili najdeni. Program brez njih ne deluje pravilno."));
		return TRUE;
	}
	
	//load placnik static data from ebank.ini
	if (!LoadConfiguration())
	{
		AfxMessageBox(_T("Napaka pri branju konfiguracijske datoteke. Podatki plačnika bodo prazni."));
	}

	//set up filter array
	setupFilter();

	//set up list array
	SetupList();

	//populate list with data from database
	populateList();

	return TRUE;
}


// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CBankAppDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CBankAppDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/// @brief Sets up the filter options in the dialog.
void CBankAppDlg::setupFilter()
{
	CComboBox* filterType = (CComboBox*)GetDlgItem(MCFilterType);
	filterType->AddString(_T("Naziv prejemnika"));
	filterType->AddString(_T("Znesek"));
	filterType->AddString(_T("Rok plačila"));
	filterType->AddString(_T("Nujnost plačila"));
	filterType->AddString(_T("Status"));
	filterType->SetCurSel(0);
}

/// @brief Sets up the list control in the dialog by enabling checkboxes, full row select, and inserting columns.
void CBankAppDlg::SetupList()
{
	CListCtrl* pListCtrl = (CListCtrl*)GetDlgItem(lstSeznamNalog);

	// Enable checkboxes and full row select
	pListCtrl->SetExtendedStyle(pListCtrl->GetExtendedStyle() | LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

	// Insert columns
	pListCtrl->InsertColumn(0, _T("ID"), LVCFMT_LEFT, 100);
	pListCtrl->InsertColumn(1, _T("Nujnost"), LVCFMT_LEFT, 70);
	pListCtrl->InsertColumn(2, _T("Št. računa plačnika"), LVCFMT_LEFT, 150);
	pListCtrl->InsertColumn(3, _T("Št. računa prejemnika"), LVCFMT_LEFT, 150);
	pListCtrl->InsertColumn(4, _T("Naziv prejemnika"), LVCFMT_LEFT, 100);
	pListCtrl->InsertColumn(5, _T("Namen plačila"), LVCFMT_LEFT, 100);
	pListCtrl->InsertColumn(6, _T("Rok plačila"), LVCFMT_LEFT, 100);
	pListCtrl->InsertColumn(7, _T("Status"), LVCFMT_LEFT, 80);
	pListCtrl->InsertColumn(8, _T("Znesek"), LVCFMT_LEFT, 80);
}


/// @brief Populates the list control with data from the database.
void CBankAppDlg::populateList()
{
	CListCtrl* pListCtrl = (CListCtrl*)GetDlgItem(lstSeznamNalog);
	pListCtrl->DeleteAllItems();

	m_allItems = DatabaseHelper::LoadData();
	int i = 0;

	for (const auto& data : m_allItems)
	{
		CString id;
		CString status;
		NalogData nalog;
		std::tie(id, status, nalog) = data;

		// Insert rows
		pListCtrl->InsertItem(i, id);
		pListCtrl->SetItemText(i, 1, nalog.placnik.nujno);
		pListCtrl->SetItemText(i, 2, g_placnikStaticData.iban);
		pListCtrl->SetItemText(i, 3, nalog.prejemnik.IBAN);
		pListCtrl->SetItemText(i, 4, nalog.prejemnik.naziv);
		pListCtrl->SetItemText(i, 5, nalog.placnik.namen);
		pListCtrl->SetItemText(i, 6, nalog.prejemnik.datumPlacila);
		pListCtrl->SetItemText(i, 7, status);
		pListCtrl->SetItemText(i, 8, nalog.prejemnik.Znesek);
		i++;
	}
}


/// @brief Retrieves the indexes of the checked items in the list control.
/// @return A vector containing the indexes of the checked items.
std::vector<int> CBankAppDlg::RetrieveCheckedItems()
{
	std::vector<int> checkedIndexes;

	// Get a pointer to the list control
	CListCtrl* pListCtrl = (CListCtrl*)GetDlgItem(lstSeznamNalog);

	// Check if the list control is valid
	if (pListCtrl == nullptr)
	{
		return checkedIndexes;
	}

	// Get the number of items in the list control
	int nItemCount = pListCtrl->GetItemCount();

	// Iterate through all the items
	for (int i = 0; i < nItemCount; ++i)
	{
		// Check if the item is checked
		if (pListCtrl->GetCheck(i))
		{
			// Item is checked, add its index to the list
			checkedIndexes.push_back(i);
		}
	}

	return checkedIndexes;
}


/// @brief Handles the OK button click event.
void CBankAppDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}

/// @brief Handles the Cancel button click event.
void CBankAppDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}

/// @brief Handles the creation of a new payment order.
void CBankAppDlg::OnBnClickedbtnnovnalog()
{
	// TODO: Add your control notification handler code here
	CUrejanjeDlg dlgUrejanje;
	INT_PTR nResponse = dlgUrejanje.DoModal();

	if (nResponse == IDOK)
	{
		// refresh whole list
		// TODO: could improve this to only add the new item (how to get ID of newly added item?)
		populateList();
	}
}

/// @brief Handles the editing of a payment order.
void CBankAppDlg::OnBnClickedbtnurejanjenalog()
{
	std::vector<int> checkedItems = RetrieveCheckedItems();

	if (checkedItems.size() != 1)
	{
		AfxMessageBox(_T("Prosimo označite ali dvokliknite zgolj en nalog, ki ga želite urejati."));
		return;
	}

	//get id of item at index from list
	CListCtrl* pListCtrl = (CListCtrl*)GetDlgItem(lstSeznamNalog);
	CString strItemID = pListCtrl->GetItemText(checkedItems.at(0), 0);

	//check status of item
	CString itemStatus = pListCtrl->GetItemText(checkedItems.at(0), 7);
	if (itemStatus != _T("PREPARED"))
	{
		AfxMessageBox(_T("Naloga ni možno urejati, ker nima statusa 'PREPARED'."));
		return;
	}

	//get data from database for selected item
	CString id;
	CString status;
	NalogData nalog;

	std::tuple<CString, CString, NalogData> data = DatabaseHelper::LoadDataByID(strItemID);
	std::tie(id, status, nalog) = data;

	if (id.IsEmpty())
	{
		AfxMessageBox(_T("Napaka pri pridobivanju plačilnega naloga iz baze."));
		return;
	}

	//send data to dialog and show it
	CUrejanjeDlg dlgUrejanje(nalog);
	INT_PTR nResponse = dlgUrejanje.DoModal();

	if (nResponse == IDOK)
	{
		//refresh list
		pListCtrl->SetItemText(checkedItems.at(0), 1, nalog.placnik.nujno);
		pListCtrl->SetItemText(checkedItems.at(0), 2, g_placnikStaticData.iban);
		pListCtrl->SetItemText(checkedItems.at(0), 3, nalog.prejemnik.IBAN);
		pListCtrl->SetItemText(checkedItems.at(0), 4, nalog.prejemnik.naziv);
		pListCtrl->SetItemText(checkedItems.at(0), 5, nalog.placnik.namen);
		pListCtrl->SetItemText(checkedItems.at(0), 6, nalog.prejemnik.datumPlacila);
		pListCtrl->SetItemText(checkedItems.at(0), 8, nalog.prejemnik.Znesek);
	}
}

/// @brief Handles the deletion of payment orders.
void CBankAppDlg::OnBnClickedbtnbrisanjenalog()
{
	std::vector<CString> deletedItems;
	std::vector<CString> unsuitableItems;
	std::vector<int> checkedItems = RetrieveCheckedItems();

	if (checkedItems.size() == 0)
	{
		AfxMessageBox(_T("Prosimo označite nalog/e, ki jih želite izbrisati."));
		return;
	}
	
	for (int i = 0; i < checkedItems.size(); i++)
	{
		//get id of item at index from list
		CListCtrl* pListCtrl = (CListCtrl*)GetDlgItem(lstSeznamNalog);
		CString strItemID = pListCtrl->GetItemText(checkedItems.at(i), 0);

		//check status of item
		CString itemStatus = pListCtrl->GetItemText(checkedItems.at(i), 7);
		if (itemStatus != _T("PREPARED"))
		{
			unsuitableItems.push_back(strItemID);
			continue;
		}

		//delete item from database
		if (!DatabaseHelper::DeleteDataByID(strItemID))
		{
			AfxMessageBox(_T("Napaka pri brisanju plačilnega naloga iz baze."));
		}
		else
		{
			deletedItems.push_back(strItemID);
		}
	}

	//show message
	CString message = getDeletedItemsMessage(deletedItems, unsuitableItems);
	if (!(message.IsEmpty()))
	{
		AfxMessageBox(message);
	}

	// refresh whole list
	// TODO: could improve this to only delete the new item/s
	//pseudocode: for each ID in deletedItems, find item in the list, call DatabaseHelper::LoadDataByID() and update item's data in the list
	if (!deletedItems.empty())
	{
		populateList();
	}
}

/// @brief Handles the timer event.
void CBankAppDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		m_elapsedTime += 1;

		// Update progress bar based on elapsed time
		if (m_elapsedTime <= 10)
		{
			int progress = m_elapsedTime * 10;  // Calculate progress percentage (10% per second)
			pDlg->updateProgress(progress);
		}

		if (m_elapsedTime >= 10)
		{
			if (pDlg != nullptr)
			{
				pDlg->DestroyWindow();
				delete pDlg;
				pDlg = nullptr;
			}

			KillTimer(1);
		}
	}

	CDialogEx::OnTimer(nIDEvent);  // Call the base class to handle other timers if needed
}


/// @brief Updates the progress bar in the progress dialog.
UINT UpdateProgressThread(LPVOID pParam)
{
    ProgressDlg* pDlg = (ProgressDlg*)pParam;

    if (pDlg == nullptr)
        return 1; 

    for (int i = 1; i <= 10; ++i)  // 10 seconds (1 update per second)
    {
        // Simulate work with a sleep of 1 second
        Sleep(1000);

        // Post a message to the dialog to update the progress bar on the main UI thread
        pDlg->PostMessage(WM_USER_PROGRESS_UPDATE, i * 10, 0);  // Progress percentage: 10%, 20%, ..., 100%
    }

    // After 10 seconds, post a message to close the dialog
    pDlg->PostMessage(WM_USER_CLOSE_PROGRESS, 0, 0);

    return 0;
}

/// @brief Handles the sending of payment orders.
void CBankAppDlg::OnBnClickedbtnposiljanjenalog()
{
	// TODO: Add your control notification handler code here
	std::vector<int> checkedItems = RetrieveCheckedItems();

	if (checkedItems.size() == 0)
	{
		AfxMessageBox(_T("Prosimo označite nalog/e, ki jih želite plačati."));
		return;
	}

	for (int i = 0; i < checkedItems.size(); i++)
	{
		//get id of item at index from list
		CListCtrl* pListCtrl = (CListCtrl*)GetDlgItem(lstSeznamNalog);
		CString strItemID = pListCtrl->GetItemText(checkedItems.at(i), 0);

		//check status of item
		CString itemStatus = pListCtrl->GetItemText(checkedItems.at(i), 7);
		if (itemStatus != _T("PREPARED"))
		{
			g_invalidItems.push_back(strItemID);
			continue;
		}

		//set item status to PAID
		if (DatabaseHelper::SetStatusByID(strItemID, _T("PAID")))
		{
			//update g_paidItems and g_value
			g_paidItems.push_back(strItemID);
			CString valStr = pListCtrl->GetItemText(checkedItems.at(i), 8);
			valStr.Replace(_T(','), _T('.'));
			g_value += _ttof(valStr);
		}
		else
		{
			AfxMessageBox(_T("Napaka pri brisanju plačilnega naloga iz baze."));
		}
	}

	//if we have paid items, show progress dialog
	if (g_value != 0.0)
	{
		// show progress dialog, either by SetTimer functionality, or by creating new thread
		// this is only to show two different ways of doing it

		/*pDlg = new ProgressDlg();
		pDlg->Create(IDD_PROGRESS_DIALOG);
		pDlg->ShowWindow(SW_SHOW);
		SetTimer(1, 1000, nullptr);
		m_elapsedTime = 0;*/

		m_pProgressDlg = new ProgressDlg();
		m_pProgressDlg->Create(IDD_PROGRESS_DIALOG, this);
		m_pProgressDlg->ShowWindow(SW_SHOW);
		m_pThread = AfxBeginThread(UpdateProgressThread, m_pProgressDlg);

		//disable buttons while waiting for progress dialog to finish
		GetDlgItem(btnNovNalog)->EnableWindow(FALSE);
		GetDlgItem(btnUrejanjeNalog)->EnableWindow(FALSE);
		GetDlgItem(btnBrisanjeNalog)->EnableWindow(FALSE);
		GetDlgItem(btnPosiljanjeNalog)->EnableWindow(FALSE);
		GetDlgItem(btnPregledNalog)->EnableWindow(FALSE);
		GetDlgItem(lstSeznamNalog)->EnableWindow(FALSE);
		GetDlgItem(MCFilterText)->EnableWindow(FALSE);
		GetDlgItem(MCFilterType)->EnableWindow(FALSE);
	}
	else
	{
		CString message = getPaidItemsMessage(g_paidItems, g_invalidItems, g_value);
		if (!(message.IsEmpty()))
		{
			AfxMessageBox(message);
		}

		g_paidItems.clear();
		g_invalidItems.clear();
		g_value = 0;
	}
}

/// @brief Handles the closing of the progress dialog.
LRESULT CBankAppDlg::OnProgressDialogClosed(WPARAM wParam, LPARAM lParam)
{
	//enable buttons after progress dialog has finished
	GetDlgItem(btnNovNalog)->EnableWindow(TRUE);
	GetDlgItem(btnUrejanjeNalog)->EnableWindow(TRUE);
	GetDlgItem(btnBrisanjeNalog)->EnableWindow(TRUE);
	GetDlgItem(btnPosiljanjeNalog)->EnableWindow(TRUE);
	GetDlgItem(btnPregledNalog)->EnableWindow(TRUE);
	GetDlgItem(lstSeznamNalog)->EnableWindow(TRUE);
	GetDlgItem(MCFilterText)->EnableWindow(TRUE);
	GetDlgItem(MCFilterType)->EnableWindow(TRUE);

	// Show message after progress dialog is closed
	CString message = getPaidItemsMessage(g_paidItems, g_invalidItems, g_value);
	if (!(message.IsEmpty()))
	{
		AfxMessageBox(message);
	}

	g_paidItems.clear();
	g_invalidItems.clear();
	g_value = 0;

	populateList();

	return 0;
}

/// @brief Displays a dialog for viewing a payment order.
void CBankAppDlg::viewDialog(int selectedItem)
{
	//get data from database for selected item
	CString id;
	CString status;
	NalogData nalog;

	//get id of item at index from list
	CListCtrl* pListCtrl = (CListCtrl*)GetDlgItem(lstSeznamNalog);
	CString strItemID = pListCtrl->GetItemText(selectedItem, 0);

	std::tuple<CString, CString, NalogData> data = DatabaseHelper::LoadDataByID(strItemID);
	std::tie(id, status, nalog) = data;

	if (id.IsEmpty())
	{
		AfxMessageBox(_T("Napaka pri pridobivanju plačilnega naloga iz baze."));
		return;
	}

	//send data to dialog and show it
	CUrejanjeDlg dlgUrejanje(nalog, nullptr, TRUE);
	INT_PTR nResponse = dlgUrejanje.DoModal();
}

/// @brief Handles the viewing of a payment order.
void CBankAppDlg::OnBnClickedbtnpreglednalog()
{
	// TODO: Add your control notification handler code here
	std::vector<int> checkedItems = RetrieveCheckedItems();

	if (checkedItems.size() != 1)
	{
		AfxMessageBox(_T("Prosimo označite ali dvokliknite zgolj en nalog, ki ga želite pregledati."));
		return;
	}

	viewDialog(checkedItems.at(0));
}

/// @brief Handles the double-click event on the list control.
void CBankAppDlg::OnNMDblclklstseznamnalog(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = pNMItemActivate->iItem;
	CListCtrl* pListCtrl = (CListCtrl*)GetDlgItem(lstSeznamNalog);

	if (nItem != -1) // Ensure an item was double-clicked
	{
		viewDialog(nItem);
	}

	*pResult = 0;
}

/// @brief Handles the change event on the filter text control.
void CBankAppDlg::OnEnChangeMcfiltertext()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here

	//get filter text
	CString filterText;
	GetDlgItemText(MCFilterText, filterText);

	//get filter type's index
	CComboBox* filterType = (CComboBox*)GetDlgItem(MCFilterType);
	int filterTypeIndex = filterType->GetCurSel();

	if (filterText.IsEmpty())
	{
		populateList();
		return;
	}

	switch (FilterType(filterTypeIndex))
	{
	case NAZIV_PREJEMNIKA:
		filterList(filterText, FilterColumn::COL_NAZIV_PREJEMNIKA);
		break;
	case ZNESEK:
		filterList(filterText, FilterColumn::COL_ZNESEK);
		break;
	case ROK_PLACILA:
		filterList(filterText, FilterColumn::COL_ROK_PLACILA);
		break;
	case NUJNOST_PLACILA:
		filterList(filterText, FilterColumn::COL_NUJNOST_PLACILA);
		break;
	case STATUS:
		filterList(filterText, FilterColumn::COL_STATUS);
		break;
	default:
		filterList(filterText, FilterColumn::COL_NAZIV_PREJEMNIKA);
		break;
	}
}

/// @brief Filters the list control based on the filter text and column.
void CBankAppDlg::filterList(const CString& filterText, FilterColumn col)
{
	CListCtrl* pListCtrl = (CListCtrl*)GetDlgItem(lstSeznamNalog);
	std::vector<std::tuple<CString, CString, NalogData>> matchingItems;

	for (const auto& data : m_allItems)
	{
		CString id;
		CString status;
		NalogData nalog;
		std::tie(id, status, nalog) = data;
	
		CString itemText;
		switch (col)
		{
		case FilterColumn::COL_NUJNOST_PLACILA:
			itemText = nalog.placnik.nujno;
			break;
		case FilterColumn::COL_NAZIV_PREJEMNIKA:
			itemText = nalog.prejemnik.naziv;
			break;
		case FilterColumn::COL_ROK_PLACILA:
			itemText = nalog.prejemnik.datumPlacila;
			break;
		case FilterColumn::COL_STATUS:
			itemText = status;
			break;
		case FilterColumn::COL_ZNESEK:
			itemText = nalog.prejemnik.Znesek;
			break;
		default:
			itemText = nalog.prejemnik.naziv;
			break;
		}

		// Check if the item text contains the filter text
		if (itemText.Find(filterText) != -1)
		{
			matchingItems.push_back(data);
		}
	}

	// Clear the list control
	pListCtrl->DeleteAllItems();

	// Add the matching items back to the list control
	int i = 0;
	for (std::tuple<CString, CString, NalogData> data : matchingItems)
	{
		CString id;
		CString status;
		NalogData nalog;
		std::tie(id, status, nalog) = data;

		// Insert rows
		pListCtrl->InsertItem(i, id);
		pListCtrl->SetItemText(i, 1, nalog.placnik.nujno);
		pListCtrl->SetItemText(i, 2, g_placnikStaticData.iban);
		pListCtrl->SetItemText(i, 3, nalog.prejemnik.IBAN);
		pListCtrl->SetItemText(i, 4, nalog.prejemnik.naziv);
		pListCtrl->SetItemText(i, 5, nalog.placnik.namen);
		pListCtrl->SetItemText(i, 6, nalog.prejemnik.datumPlacila);
		pListCtrl->SetItemText(i, 7, status);
		pListCtrl->SetItemText(i, 8, nalog.prejemnik.Znesek);
		i++;
	}
}
