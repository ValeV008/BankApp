#pragma once

#include <cctype>
#include <stdexcept>
#include <regex>
#include <msxml6.h>
#include "DatabaseHelper.h"

//load placnik data from conf.ini file
bool LoadConfiguration();

// functions for getting message string for paid or deleted items
CString getPaidItemsMessage(std::vector<CString> paidItems, std::vector<CString> invalidItems, float value);
CString getDeletedItemsMessage(const std::vector<CString>& deletedItems, const std::vector<CString>& unsuitableItems);

// functions for checking the validity of the user's input data
int charToNumber(char ch);
bool checkModulo97(const std::string& iban);
bool checkPrejemnikIBAN(CString prejemnikIBAN);
bool checkNazivPrejemnika(CString prejemnikNaziv);
bool checkReferencaPrejemnika(CString prejemnikReferenca);
bool checkPlacnikNamen(CString placnikNamen);
bool checkPrejemnikZnesek(CString prejemnikZnesek);

// functions for converting NalogData from/to CString
CString concatenateNalogData(const NalogData& nalogData);
NalogData unconcatenateNalogData(const CString& concatenatedData);