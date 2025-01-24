#include "pch.h"
#include "DatabaseHelper.h"
#include "HelperFunctions.h"
#include <comdef.h>

_ConnectionPtr DatabaseHelper::m_pConnection = nullptr; // Initialize m_pConnection

/// @brief Establishes a connection to the database.
/// @return TRUE if the connection is successful, FALSE otherwise.
//BOOL DatabaseHelper::ConnectToDatabase()
BOOL DatabaseHelper::OpenDatabaseConnection()
{
    if (m_pConnection != nullptr && m_pConnection->State == adStateOpen)
    {
        return TRUE; // Connection is already open
    }

    try
    {
        // Initialize COM library
        HRESULT hr = CoInitialize(NULL);
        if (FAILED(hr))
        {
            return FALSE;
        }

        // Create ADO connection if not already created
        if (m_pConnection == nullptr)
        {
            m_pConnection.CreateInstance(__uuidof(Connection));
        }

        // Open the database connection
        hr = m_pConnection->Open("Provider=Microsoft.ACE.OLEDB.16.0;Data Source=ebank.accdb;", "", "", adConnectUnspecified);
        if (FAILED(hr))
        {
            return FALSE;
        }
    }
    catch (_com_error& e)
    {
		return FALSE;
    }

    return TRUE;
}

/// @brief Closes the database connection and uninitializes the COM library.
void DatabaseHelper::CloseDatabaseConnection()
{
    if (m_pConnection != nullptr && m_pConnection->State == adStateOpen)
    {
        m_pConnection->Close();
    }

    m_pConnection = nullptr;

    CoUninitialize();
}

/// @brief Saves or updates NalogData in the database. If the ID is empty, it inserts a new record; otherwise, it updates the existing record.
/// @param nalogData The data to be saved or updated.
void DatabaseHelper::SaveNalogData(const NalogData& nalogData)
{
    try
    {
        _CommandPtr pCommand;
        pCommand.CreateInstance(__uuidof(Command));
        pCommand->ActiveConnection = m_pConnection;

        CString concatenatedData = concatenateNalogData(nalogData);

        // Get current date/time for Datum_ustvarjen and Datum_urejen
        COleDateTime currentTime = COleDateTime::GetCurrentTime();
        CString strCurrentTime = currentTime.Format(_T("%Y-%m-%d %H:%M:%S"));

        CString query;
        if (nalogData.metadata.ID.IsEmpty())
        {
            // Prepare the SQL query when ID is not provided
            query = _T("INSERT INTO PlacilniNalogi (Status, Nalog, Datum_ustvarjen, Datum_urejen) VALUES (?, ?, ?, ?)");
        }
        else
        {
            // Prepare the SQL query when ID is provided
            query = _T("UPDATE PlacilniNalogi SET Status = ?, Nalog = ?, Datum_urejen = ? WHERE ID = ?");
        }
        pCommand->CommandText = (LPCTSTR)query;

        // Add parameters
        pCommand->Parameters->Append(pCommand->CreateParameter(_T("Status"), adBSTR, adParamInput, 255, _T("PREPARED")));
        pCommand->Parameters->Append(pCommand->CreateParameter(_T("Nalog"), adBSTR, adParamInput, 255, _bstr_t(concatenatedData)));
        if (nalogData.metadata.ID.IsEmpty())
        {
            pCommand->Parameters->Append(pCommand->CreateParameter(_T("Datum_ustvarjen"), adBSTR, adParamInput, 255, _bstr_t(strCurrentTime)));
        }
        pCommand->Parameters->Append(pCommand->CreateParameter(_T("Datum_urejen"), adBSTR, adParamInput, 255, _bstr_t(strCurrentTime)));
        if (!nalogData.metadata.ID.IsEmpty())
        {
            pCommand->Parameters->Append(pCommand->CreateParameter(_T("ID"), adInteger, adParamInput, 255, _ttoi(nalogData.metadata.ID)));
        }

        // Execute the query
        pCommand->Execute(NULL, NULL, adCmdText);
    }
    catch (_com_error& e)
    {
        AfxMessageBox("Neznana napaka:\n" + e.Description());
    }
}

/// @brief Loads all data from the PlacilniNalogi table.
/// @return A vector of tuples containing ID, Status, and NalogData.
std::vector<std::tuple<CString, CString, NalogData>> DatabaseHelper::LoadData()
{
    std::vector<std::tuple<CString, CString, NalogData>> dataList;

    try
    {
        _RecordsetPtr pRecordset;
        pRecordset.CreateInstance(__uuidof(Recordset));

        // Open a recordset
        pRecordset->Open("SELECT ID, Status, Nalog, Datum_ustvarjen, Datum_urejen FROM PlacilniNalogi", m_pConnection.GetInterfacePtr(), adOpenStatic, adLockOptimistic, adCmdText);

        // Iterate through the recordset and load data
        while (!pRecordset->EndOfFile)
        {
            _variant_t varID = pRecordset->Fields->Item["ID"]->Value;
            _variant_t varStatus = pRecordset->Fields->Item["Status"]->Value;
            _variant_t varNalog = pRecordset->Fields->Item["Nalog"]->Value;
            _variant_t varDatumUstvarjen = pRecordset->Fields->Item["Datum_ustvarjen"]->Value;
            _variant_t varDatumUrejen = pRecordset->Fields->Item["Datum_urejen"]->Value;

            CString fetchedID = (LPCTSTR)(_bstr_t)varID;
            CString strStatus = (LPCTSTR)(_bstr_t)varStatus;
            CString nalog = (LPCTSTR)(_bstr_t)varNalog;
            CString datumUstvarjen = (LPCTSTR)(_bstr_t)varDatumUstvarjen;
            CString datumUrejen = (LPCTSTR)(_bstr_t)varDatumUrejen;

            // Parse the Nalog field into a NalogData object
            NalogData nalogData = unconcatenateNalogData(nalog);
            nalogData.metadata.ID = fetchedID;
            nalogData.metadata.status = strStatus;
            nalogData.metadata.datum_ustvarjen = datumUstvarjen;
            nalogData.metadata.datum_urejen = datumUrejen;

            dataList.emplace_back(fetchedID, strStatus, nalogData);

            pRecordset->MoveNext();
        }

        pRecordset->Close();
    }
    catch (_com_error& e)
    {
        AfxMessageBox("Neznana napaka:\n" + e.Description());
    }

    return dataList;
}

/// @brief Loads data from the PlacilniNalogi table for a specific ID.
/// @param id The ID of the record to be loaded.
/// @return A tuple containing ID, Status, and NalogData.
std::tuple<CString, CString, NalogData> DatabaseHelper::LoadDataByID(const CString& id)
{
    std::tuple<CString, CString, NalogData> dataItem;

    try
    {
        _RecordsetPtr pRecordset;
        pRecordset.CreateInstance(__uuidof(Recordset));

        // Prepare the SQL query with the specified ID
        CString query;
        query.Format(_T("SELECT ID, Status, Nalog, Datum_ustvarjen, Datum_urejen FROM PlacilniNalogi WHERE ID = %s"), id);

        // Open a recordset
        pRecordset->Open((LPCTSTR)query, m_pConnection.GetInterfacePtr(), adOpenStatic, adLockOptimistic, adCmdText);

        // Check if the recordset is not empty
        if (!pRecordset->EndOfFile)
        {
            _variant_t varID = pRecordset->Fields->Item["ID"]->Value;
            _variant_t varStatus = pRecordset->Fields->Item["Status"]->Value;
            _variant_t varNalog = pRecordset->Fields->Item["Nalog"]->Value;
            _variant_t varDatumUstvarjen = pRecordset->Fields->Item["Datum_ustvarjen"]->Value;
            _variant_t varDatumUrejen = pRecordset->Fields->Item["Datum_urejen"]->Value;

            CString fetchedID = (LPCTSTR)(_bstr_t)varID;
            CString strStatus = (LPCTSTR)(_bstr_t)varStatus;
            CString nalog = (LPCTSTR)(_bstr_t)varNalog;
            CString datumUstvarjen = (LPCTSTR)(_bstr_t)varDatumUstvarjen;
            CString datumUrejen = (LPCTSTR)(_bstr_t)varDatumUrejen;

            // Parse the Nalog field into a NalogData object
            NalogData nalogData = unconcatenateNalogData(nalog);
            nalogData.metadata.ID = fetchedID;
            nalogData.metadata.status = strStatus;
            nalogData.metadata.datum_ustvarjen = datumUstvarjen;
            nalogData.metadata.datum_urejen = datumUrejen;

            dataItem = std::make_tuple(fetchedID, strStatus, nalogData);
        }

        pRecordset->Close();
    }
    catch (_com_error& e)
    {
        AfxMessageBox("Neznana napaka:\n" + e.Description());
    }

    return dataItem;
}

/// @brief Sets the status of a record to 'DELETED' for a specific ID.
/// @param id The ID of the record to be deleted.
/// @return TRUE if the record does not exist (was deleted), FALSE otherwise.
BOOL DatabaseHelper::DeleteDataByID(const CString& id)
{
    try
    {
       //set nalog status to 'DELETED'
		_CommandPtr pCommand;
		pCommand.CreateInstance(__uuidof(Command));
		pCommand->ActiveConnection = m_pConnection;

		CString query;
		query.Format(_T("UPDATE PlacilniNalogi SET Status = 'DELETED' WHERE ID = %s"), id);
		pCommand->CommandText = (LPCTSTR)query;

		// Execute the query
		pCommand->Execute(NULL, NULL, adCmdText);
    }
    catch (_com_error& e)
    {
        AfxMessageBox("Neznana napaka:\n" + e.Description());
    }

    return !checkIfNalogByIDExists(id);
}

/// @brief Checks if a record with a specific ID exists and is not marked as 'DELETED'.
/// @param id The ID of the record to be checked.
/// @return TRUE if the record exists and is not 'DELETED', FALSE otherwise.
BOOL DatabaseHelper::checkIfNalogByIDExists(const CString& id)
{
    BOOL rez = TRUE;

    try
    {
        _RecordsetPtr pRecordset;
        pRecordset.CreateInstance(__uuidof(Recordset));

        // Prepare the SQL query with the specified ID
        CString query;
        query.Format(_T("SELECT Status FROM PlacilniNalogi WHERE ID = %s"), id);

        // Open a recordset
        pRecordset->Open((LPCTSTR)query, m_pConnection.GetInterfacePtr(), adOpenStatic, adLockOptimistic, adCmdText);

        // Check if the recordset is not empty
        if (!pRecordset->EndOfFile)
        {
            _variant_t varStatus = pRecordset->Fields->Item["Status"]->Value;
            CString strStatus = (LPCTSTR)(_bstr_t)varStatus;
            if (strStatus == _T("DELETED"))
            {
                rez = FALSE;
            }
        }

        pRecordset->Close();
    }
    catch (_com_error& e)
    {
        AfxMessageBox("Neznana napaka:\n" + e.Description());
    }

    return rez;
}

/// @brief Sets the status of a record for a specific ID.
/// @param id The ID of the record.
/// @param status The new status to be set.
/// @return TRUE if the status is successfully set to 'PAID', FALSE otherwise.
BOOL DatabaseHelper::SetStatusByID(const CString& id, const CString& status)
{
	try
	{
		_CommandPtr pCommand;
		pCommand.CreateInstance(__uuidof(Command));
		pCommand->ActiveConnection = m_pConnection;

		CString query;
		query.Format(_T("UPDATE PlacilniNalogi SET Status = '%s' WHERE ID = %s"), status, id);
		pCommand->CommandText = (LPCTSTR)query;

		// Execute the query
		pCommand->Execute(NULL, NULL, adCmdText);
	}
	catch (_com_error& e)
	{
        AfxMessageBox("Neznana napaka:\n" + e.Description());
	}

	return checkIfNalogByIDIsPaid(id);
}

/// @brief Checks if a record with a specific ID has the status 'PAID'.
/// @param id The ID of the record to be checked.
/// @return TRUE if the status is 'PAID', FALSE otherwise.
BOOL DatabaseHelper::checkIfNalogByIDIsPaid(const CString& id)
{
    BOOL rez = FALSE;

    try
    {
        _RecordsetPtr pRecordset;
        pRecordset.CreateInstance(__uuidof(Recordset));

        // Prepare the SQL query with the specified ID
        CString query;
        query.Format(_T("SELECT Status FROM PlacilniNalogi WHERE ID = %s"), id);

        // Open a recordset
        pRecordset->Open((LPCTSTR)query, m_pConnection.GetInterfacePtr(), adOpenStatic, adLockOptimistic, adCmdText);

        // Check if the recordset is not empty
        if (!pRecordset->EndOfFile)
        {
            _variant_t varStatus = pRecordset->Fields->Item["Status"]->Value;
            CString strStatus = (LPCTSTR)(_bstr_t)varStatus;
            if (strStatus == _T("PAID"))
            {
                rez = TRUE;
            }
        }

        pRecordset->Close();
    }
    catch (_com_error& e)
    {
        AfxMessageBox("Neznana napaka:\n" + e.Description());
    }

    return rez;
}

