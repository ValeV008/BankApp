// DatabaseHelper.h
#pragma once

#include <afxdb.h>
#ifdef _WIN64
#import "C:\\Program Files\\Common Files\\System\\ado\\msado15.dll" no_namespace rename("EOF", "EndOfFile")
#else
#import "C:\\Program Files (x86)\\Common Files\\System\\ado\\msado15.dll" no_namespace rename("EOF", "EndOfFile")
#endif
#include <vector>
#include <tuple>

struct NalogData
{
	struct placnik_t
	{
		CString referencaKoda;
		CString referenca;
		CString kodaNamena;
		CString namen;
		CString nujno;
	} placnik;

	struct prejemnik_t
	{
		CString Znesek;
		CString datumPlacila;
		CString BIC;
		CString IBAN;
		CString referencaKoda;
		CString referenca;
		CString naziv;
		CString naslov;
		CString kraj;
		CString drzava;
	} prejemnik;

	struct metadataData
	{
		CString ID;
		CString status;
		CString datum_ustvarjen;
		CString datum_urejen;
	} metadata;
};

class DatabaseHelper
{
public:
	static void SaveNalogData(const NalogData& nalogData);
    static std::vector<std::tuple<CString, CString, NalogData>> LoadData();
	static std::tuple<CString, CString, NalogData> LoadDataByID(const CString& id);
	static BOOL DeleteDataByID(const CString& id);
	static BOOL checkIfNalogByIDExists(const CString& id);
	static BOOL DatabaseHelper::SetStatusByID(const CString& id, const CString& status);
	static BOOL OpenDatabaseConnection();
	static void DatabaseHelper::CloseDatabaseConnection();

private:
    static _ConnectionPtr m_pConnection;
	static BOOL DatabaseHelper::checkIfNalogByIDIsPaid(const CString& id);
};