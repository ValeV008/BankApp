#include "pch.h"
#include <tuple>
#include "framework.h"
#include "afxdialogex.h"
#include "BankApp.h"
#include "UrejanjeDlg.h"
#include "DatabaseHelper.h"
#include "PlacnikStaticData.h"
#include "HelperFunctions.h"

IMPLEMENT_DYNAMIC(CUrejanjeDlg, CDialogEx)

CUrejanjeDlg::CUrejanjeDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_UREJANJE_DIALOG, pParent), m_nalogData(NalogData())
{
	m_isPregled = FALSE;
}

CUrejanjeDlg::CUrejanjeDlg(NalogData& nalogData, CWnd* pParent /*=nullptr*/, bool isPregled /*=FALSE*/)
	: CDialogEx(IDD_UREJANJE_DIALOG, pParent), m_nalogData(nalogData), m_isPregled(isPregled)
{
}

CUrejanjeDlg::~CUrejanjeDlg()
{
}

void CUrejanjeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CUrejanjeDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CUrejanjeDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CUrejanjeDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

BOOL CUrejanjeDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

	initializeDefaultValues();
	populatePlacnikStaticData();
	disableEditingStaticData();

	if (m_nalogData.metadata.ID.IsEmpty())
	{
		return TRUE;
	}

	populatePlacnikStaticData();
	populatePlacnikData();
	populatePrejemnikData();
	populateMetadata();
	if (m_isPregled)
	{
		disableEditing();
	}

    return TRUE;
}

/// @brief Checks if any required fields are empty.
/// @return TRUE if all required fields are filled, otherwise FALSE.
bool CUrejanjeDlg::checkIfAnyEmpty()
{
	//check text fields
	const int controlIDs[] = {
		ECPlacnikReferenca,
		ECPlacnikKodaNamena,
		ECPlacnikNamen,
		ECPrejemnikZnesek,
		ECPrejemnikDatumPlacila,
		ECPrejemnikBic,
		ECPrejemnikReferenca,
		ECPrejemnikNaziv,
		ECPrejemnikNaslov,
		ECPrejemnikKraj,
		ECPrejemnikDrzava
	};
	for (int controlID : controlIDs)
	{
		CString text;
		GetDlgItemText(controlID, text);
		if (text.Trim().IsEmpty())
		{
			return false;
		}
	}

	//check comboboxes
	CString placnikReferencaKoda = getComboBoxKoda(ECPlacnikReferencaKoda);
	if (placnikReferencaKoda.Trim().IsEmpty())
	{
		return false;
	}
	CString placnikKodaNamena = getComboBoxKoda(ECPlacnikKodaNamena);
	if (placnikKodaNamena.Trim().IsEmpty())
	{
		return false;
	}
	CString prejemnikReferencaKoda = getComboBoxKoda(ECPrejemnikReferencaKoda);
	if (prejemnikReferencaKoda.Trim().IsEmpty())
	{
		return false;
	}

	return true;
}

/// @brief Validates the data entered in the dialog.
bool CUrejanjeDlg::checkData()
{
	if (!checkIfAnyEmpty())
	{
		AfxMessageBox(_T("Vsa polja so obvezna."));
		return false;
	}

	CString prejemnikIBAN;
	GetDlgItemText(ECPrejemnikIban, prejemnikIBAN);
	if (!checkPrejemnikIBAN(prejemnikIBAN))
	{
		AfxMessageBox(_T("Nepravilna oblika IBAN prejemnika. Pravilna oblika je ..."));
		return false;
	}

	CString prejemnikNaziv;
	GetDlgItemText(ECPrejemnikNaziv, prejemnikNaziv);
	if (!checkNazivPrejemnika(prejemnikNaziv))
	{
		AfxMessageBox(_T("Nepravilna oblika naziva prejemnika. Pravilna oblika je ..."));
		return false;
	}

	CString prejemnikReferenca;
	GetDlgItemText(ECPrejemnikReferenca, prejemnikReferenca);
	if (!checkReferencaPrejemnika(prejemnikReferenca))
	{
		AfxMessageBox(_T("Nepravilna oblika reference prejemnika plačila. Pravilna oblika je ..."));
		return false;
	}

	CString placnikNamen;
	GetDlgItemText(ECPlacnikNamen, placnikNamen);
	if (!checkPlacnikNamen(placnikNamen))
	{
		AfxMessageBox(_T("Nepravilna oblika namena plačila. Pravilna oblika je ..."));
		return false;
	}

	CString prejemnikZnesek;
	GetDlgItemText(ECPrejemnikZnesek, prejemnikZnesek);
	if (!checkPrejemnikZnesek(prejemnikZnesek))
	{
		AfxMessageBox(_T("Nepravilna oblika zneska. Pravilna oblika je ..."));
		return false;
	}

	return true;
}

/// @brief Updates the NalogData object with the data from the dialog and saves it to the database.
/// @return TRUE if the data is successfully updated and saved, otherwise FALSE.
bool CUrejanjeDlg::UpdateNalogData()
{
	if (!checkData())
	{
		return false;
	}

	//update placnik and prejemnik m_nalogData
	updatePlacnikData();
	updatePrejemnikData();
	
	DatabaseHelper::SaveNalogData(m_nalogData);

	return true;
}

/// @brief Populates the dialog with static data for the payer (placnik).
void CUrejanjeDlg::populatePlacnikStaticData()
{
	SetDlgItemText(ECPlacnikIban, g_placnikStaticData.iban);
	SetDlgItemText(ECPlacnikNaziv, g_placnikStaticData.ime);
	SetDlgItemText(ECPlacnikNaslov, g_placnikStaticData.naslov);
	SetDlgItemText(ECPlacnikKraj, g_placnikStaticData.kraj);
	SetDlgItemText(ECPlacnikDrzava, g_placnikStaticData.drzava);
}

/// @brief Populates the dialog with data for the payer (placnik) from the NalogData object.
void CUrejanjeDlg::populatePlacnikData()
{
	SetDlgItemText(ECPlacnikReferenca, m_nalogData.placnik.referenca);
	SetDlgItemText(ECPlacnikNamen, m_nalogData.placnik.namen);
	SetDlgItemText(ECPlacnikNamen, m_nalogData.placnik.namen);
	CheckDlgButton(ECPlacnikNujno, m_nalogData.placnik.nujno == _T("DA") ? BST_CHECKED : BST_UNCHECKED);

	CComboBox* placnikReferencaKoda = (CComboBox*)GetDlgItem(ECPlacnikReferencaKoda);
	if (m_nalogData.placnik.referencaKoda == _T("SI99"))
	{
		placnikReferencaKoda->SetCurSel(0);
	}
	else if (m_nalogData.placnik.referencaKoda == _T("NRC"))
	{
		placnikReferencaKoda->SetCurSel(1);
	}

	CComboBox* placnikKodaNamena = (CComboBox*)GetDlgItem(ECPlacnikKodaNamena);
	if (m_nalogData.placnik.kodaNamena == _T("TAXS"))
	{
		placnikKodaNamena->SetCurSel(0);
	}
	else if (m_nalogData.placnik.kodaNamena == _T("OTHR"))
	{
		placnikKodaNamena->SetCurSel(1);
	}
}

/// @brief Updates the NalogData object with the data for the payer (placnik) from the dialog.
void CUrejanjeDlg::updatePlacnikData()
{
	m_nalogData.placnik.referencaKoda = getComboBoxKoda(ECPlacnikReferencaKoda);
	GetDlgItemText(ECPlacnikReferenca, m_nalogData.placnik.referenca);
	m_nalogData.placnik.kodaNamena = getComboBoxKoda(ECPlacnikKodaNamena);
	GetDlgItemText(ECPlacnikNamen, m_nalogData.placnik.namen);
	m_nalogData.placnik.nujno = IsDlgButtonChecked(ECPlacnikNujno) == BST_CHECKED ? _T("DA") : _T("NE");
}

/// @brief Updates the NalogData object with the data for the recipient (prejemnik) from the dialog.
void CUrejanjeDlg::updatePrejemnikData()
{
	GetDlgItemText(ECPrejemnikZnesek, m_nalogData.prejemnik.Znesek);
	COleDateTime dateTime;
	CDateTimeCtrl* pDateTimeCtrl = (CDateTimeCtrl*)GetDlgItem(ECPrejemnikDatumPlacila);
	pDateTimeCtrl->GetTime(dateTime);
	m_nalogData.prejemnik.datumPlacila = dateTime.Format(_T("%d.%m.%Y"));
	GetDlgItemText(ECPrejemnikBic, m_nalogData.prejemnik.BIC);
	GetDlgItemText(ECPrejemnikIban, m_nalogData.prejemnik.IBAN);
	m_nalogData.prejemnik.referencaKoda = getComboBoxKoda(ECPrejemnikReferencaKoda);
	GetDlgItemText(ECPrejemnikReferenca, m_nalogData.prejemnik.referenca);
	GetDlgItemText(ECPrejemnikNaziv, m_nalogData.prejemnik.naziv);
	GetDlgItemText(ECPrejemnikNaslov, m_nalogData.prejemnik.naslov);
	GetDlgItemText(ECPrejemnikKraj, m_nalogData.prejemnik.kraj);
	GetDlgItemText(ECPrejemnikDrzava, m_nalogData.prejemnik.drzava);
}

/// @brief Populates the dialog with data for the recipient (prejemnik) from the NalogData object.
void CUrejanjeDlg::populatePrejemnikData()
{
	SetDlgItemText(ECPrejemnikZnesek, m_nalogData.prejemnik.Znesek);
	SetDlgItemText(ECPrejemnikBic, m_nalogData.prejemnik.BIC);
	SetDlgItemText(ECPrejemnikIban, m_nalogData.prejemnik.IBAN);
	SetDlgItemText(ECPrejemnikReferenca, m_nalogData.prejemnik.referenca);
	SetDlgItemText(ECPrejemnikNaziv, m_nalogData.prejemnik.naziv);
	SetDlgItemText(ECPrejemnikNaslov, m_nalogData.prejemnik.naslov);
	SetDlgItemText(ECPrejemnikKraj, m_nalogData.prejemnik.kraj);
	SetDlgItemText(ECPrejemnikDrzava, m_nalogData.prejemnik.drzava);

	COleDateTime dateTime;
	int day, month, year;
	CDateTimeCtrl* pDateTimeCtrl = (CDateTimeCtrl*)GetDlgItem(ECPrejemnikDatumPlacila);
	if (_stscanf_s(m_nalogData.prejemnik.datumPlacila, _T("%d.%d.%d"), &day, &month, &year) == 3) {
		dateTime.SetDate(year, month, day);
		pDateTimeCtrl->SetTime(dateTime);
	}
	else {
		// Handle the error if the date string is invalid
		AfxMessageBox(_T("Napaka pri branju roka plačila."));
	}

	CComboBox* prejemnikReferencaKoda = (CComboBox*)GetDlgItem(ECPrejemnikReferencaKoda);
	if (m_nalogData.prejemnik.referencaKoda == _T("SI99"))
	{
		prejemnikReferencaKoda->SetCurSel(0);
	}
	else if (m_nalogData.prejemnik.referencaKoda == _T("NRC"))
	{
		prejemnikReferencaKoda->SetCurSel(1);
	}
}

/// @brief Disables editing of static data fields for the payer (placnik).
void CUrejanjeDlg::disableEditingStaticData()
{
	//disable placnik static data editing
	((CEdit*)GetDlgItem(ECPlacnikIban))->SetReadOnly(TRUE);
	((CEdit*)GetDlgItem(ECPlacnikNaziv))->SetReadOnly(TRUE);
	((CEdit*)GetDlgItem(ECPlacnikNaslov))->SetReadOnly(TRUE);
	((CEdit*)GetDlgItem(ECPlacnikKraj))->SetReadOnly(TRUE);
	((CEdit*)GetDlgItem(ECPlacnikDrzava))->SetReadOnly(TRUE);
}

/// @brief Disables editing of all fields in the dialog.
void CUrejanjeDlg::disableEditing()
{
	((CComboBox*)GetDlgItem(ECPlacnikReferencaKoda))->EnableWindow(FALSE);
	((CEdit*)GetDlgItem(ECPlacnikReferenca))->SetReadOnly(TRUE);
	((CComboBox*)GetDlgItem(ECPlacnikKodaNamena))->EnableWindow(FALSE);
	((CEdit*)GetDlgItem(ECPlacnikNamen))->SetReadOnly(TRUE);
	((CButton*)GetDlgItem(ECPlacnikNujno))->EnableWindow(FALSE);

	((CEdit*)GetDlgItem(ECPrejemnikZnesek))->SetReadOnly(TRUE);
	((CDateTimeCtrl*)GetDlgItem(ECPrejemnikDatumPlacila))->EnableWindow(FALSE);
	((CEdit*)GetDlgItem(ECPrejemnikBic))->SetReadOnly(TRUE);
	((CEdit*)GetDlgItem(ECPrejemnikIban))->SetReadOnly(TRUE);
	((CComboBox*)GetDlgItem(ECPrejemnikReferencaKoda))->EnableWindow(FALSE);
	((CEdit*)GetDlgItem(ECPrejemnikReferenca))->SetReadOnly(TRUE);
	((CEdit*)GetDlgItem(ECPrejemnikNaziv))->SetReadOnly(TRUE);
	((CEdit*)GetDlgItem(ECPrejemnikNaslov))->SetReadOnly(TRUE);
	((CEdit*)GetDlgItem(ECPrejemnikKraj))->SetReadOnly(TRUE);
	((CEdit*)GetDlgItem(ECPrejemnikDrzava))->SetReadOnly(TRUE);
}

/// @brief Initializes default values for combo boxes and static text fields in the dialog.
void CUrejanjeDlg::initializeDefaultValues()
{
	CComboBox* placnikReferencaKoda = (CComboBox*)GetDlgItem(ECPlacnikReferencaKoda);
	placnikReferencaKoda->AddString(_T("SI99"));
	placnikReferencaKoda->AddString(_T("NRC"));

	CComboBox* placnikKodaNamena = (CComboBox*)GetDlgItem(ECPlacnikKodaNamena);
	placnikKodaNamena->AddString(_T("TAXS"));
	placnikKodaNamena->AddString(_T("OTHR"));

	CComboBox* prejemnikReferencaKoda = (CComboBox*)GetDlgItem(ECPrejemnikReferencaKoda);
	prejemnikReferencaKoda->AddString(_T("SI99"));
	prejemnikReferencaKoda->AddString(_T("NRC"));

	SetDlgItemText(STStatus, _T(""));
	SetDlgItemText(STID, _T(""));
	SetDlgItemText(STPripravljen, _T(""));
	SetDlgItemText(STPrejet, _T(""));
}

/// @brief Populates the dialog with metadata from the NalogData object.
void CUrejanjeDlg::populateMetadata()
{
	SetDlgItemText(STStatus, m_nalogData.metadata.status);
	SetDlgItemText(STID, m_nalogData.metadata.ID);
	SetDlgItemText(STPripravljen, m_nalogData.metadata.datum_ustvarjen);
	SetDlgItemText(STPrejet, m_nalogData.metadata.datum_urejen);

	SetDlgItemText(STKomentar, _T("Komentar: /"));
	SetDlgItemText(STPripravil, _T("Pripravil: ") + g_placnikStaticData.ime);
	SetDlgItemText(STPodpisal, _T("Podpisal: /"));
}


/// @brief Handles the OK button click event. Updates the NalogData object and closes the dialog if not in pregled mode.
void CUrejanjeDlg::OnBnClickedOk()
{
	if (m_isPregled)
	{
		CDialogEx::OnOK();
	}
	else if (UpdateNalogData())
	{
		CDialogEx::OnOK();
	}
}

/// @brief Retrieves the selected code from a combo box.
/// @param controlID The ID of the combo box control.
/// @return The selected code as a CString.
CString CUrejanjeDlg::getComboBoxKoda(int controlID)
{
	CString rez = _T("");
	CComboBox* comboBox = (CComboBox*)GetDlgItem(controlID);

	// Check if comboBox is valid before accessing it
	if (comboBox != nullptr)
	{
		int index = comboBox->GetCurSel();
		if (index != CB_ERR)
		{
			CString koda;
			comboBox->GetLBText(index, koda);
			rez = koda;
		}
	}

	return rez;
}

/// @brief Handles the Cancel button click event. Closes the dialog.
void CUrejanjeDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}
