#include "pch.h"
#include "HelperFunctions.h"
#include "PlacnikStaticData.h"
#include <sstream>
#include <iostream>

PlacnikStaticData g_placnikStaticData;

/// @brief Loads the configuration from an XML file and populates the global PlacnikStaticData object.
/// @return TRUE if the configuration is successfully loaded, otherwise FALSE.
bool LoadConfiguration()
{
	CString strFilePath = _T("ebank.ini");

	// Create an instance of the XML DOM Document
	CComPtr<IXMLDOMDocument> spXMLDoc;
	HRESULT hr = spXMLDoc.CoCreateInstance(__uuidof(DOMDocument60), NULL, CLSCTX_INPROC_SERVER);
	if (FAILED(hr))
	{
		return FALSE;
	}

	// Load the XML file
	VARIANT_BOOL vbSuccess;
	hr = spXMLDoc->load(CComVariant(strFilePath), &vbSuccess);
	if (FAILED(hr) || vbSuccess == VARIANT_FALSE)
	{
		return FALSE;
	}

	// Get the root element
	CComPtr<IXMLDOMElement> spRoot;
	hr = spXMLDoc->get_documentElement(&spRoot);
	if (FAILED(hr) || spRoot == NULL)
	{
		return FALSE;
	}

	// Function to safely select node and get text
	auto loadValFromNode = [&](const WCHAR* nodeName) -> CString {
		CComBSTR bstrNodeName(nodeName);
		CComPtr<IXMLDOMNode> spNode;
		HRESULT hr = spRoot->selectSingleNode(bstrNodeName, &spNode);
		CString strValue(_T(""));

		if (SUCCEEDED(hr) && spNode != NULL)
		{
			CComBSTR bstrText;
			hr = spNode->get_text(&bstrText);
			if (SUCCEEDED(hr))
			{
				strValue = CString(bstrText); // Save the value to strValue
			}
			else
			{
				// Failed to get text for nodeName
				return FALSE;
			}
		}
		else
		{
			// Failed to select node nodeName
			return FALSE;
		}
		
		return strValue; // Return the retrieved value
	};

	// Retrieve values using the helper function
	g_placnikStaticData.iban = loadValFromNode(L"iban");
	g_placnikStaticData.ime = loadValFromNode(L"ime");
	g_placnikStaticData.naslov = loadValFromNode(L"naslov");
	g_placnikStaticData.kraj = loadValFromNode(L"kraj");
	g_placnikStaticData.drzava = loadValFromNode(L"drzava");

	// Release COM objects so that CoUninitialize doesn't fail
	spRoot = nullptr; // Release spRoot
	spXMLDoc = nullptr; // Release spXMLDoc before CoUninitialize

	return TRUE;
}

/// @brief Generates a message string for paid and invalid items.
/// @param paidItems Vector of paid items.
/// @param invalidItems Vector of invalid items.
/// @param value Total value of paid items.
/// @return A CString containing the message.
CString getPaidItemsMessage(std::vector<CString> paidItems, std::vector<CString> invalidItems, float value)
{
	std::ostringstream oss;
	CString valStr;

	// add paid items to message
	if (!(paidItems.empty()))
	{
		oss << "Uspešno plačani nalogi: ";
		for (size_t i = 0; i < paidItems.size(); ++i)
		{
			oss << CW2A(paidItems[i], CP_UTF8);
			if (i != paidItems.size() - 1)
			{
				oss << ", ";
			}
		}
		oss << ".";
	}

	//add invalid items to message
	if (!(invalidItems.empty()))
	{
		oss << "\nNaslednji nalogi niso bili plačani, ker nimajo status PREPARED: ";
		for (size_t i = 0; i < invalidItems.size(); ++i)
		{
			oss << CW2A(invalidItems[i], CP_UTF8);
			if (i != invalidItems.size() - 1)
			{
				oss << ", ";
			}
		}
		oss << ".";
	}

	//add total value to message
	if (!(paidItems.empty()))
	{
		//magic convertion from float to ASCII string
		valStr.Format(_T("%.2f"), value);
		valStr.Replace(_T('.'), _T(','));
		oss << "\nSkupni znesek: " << CW2A(valStr, CP_UTF8);
		//CT2A asciiValStr(valStr);
		//oss << "\nSkupni znesek: " << asciiValStr;
	}

	return CString(CA2T(oss.str().c_str(), CP_UTF8));
}


/// @brief Generates a message string for deleted and unsuitable items.
/// @param deletedItems Vector of deleted items.
/// @param unsuitableItems Vector of unsuitable items.
/// @return A CString containing the message.
CString getDeletedItemsMessage(const std::vector<CString>& deletedItems, const std::vector<CString>& unsuitableItems)
{
	std::ostringstream oss;

	if (!(deletedItems.empty()))
	{
		oss << "Uspešno izbrisani nalogi: ";
		for (size_t i = 0; i < deletedItems.size(); ++i)
		{
			oss << CW2A(deletedItems[i], CP_UTF8);
			if (i != deletedItems.size() - 1)
			{
				oss << ", ";
			}
		}
		oss << ".";
	}


	if (!(unsuitableItems.empty()))
	{
		oss << "\nNaslednji nalogi niso bili izbrisani, ker nimajo statusa 'PREPARED': ";
		for (size_t i = 0; i < unsuitableItems.size(); ++i)
		{
			oss << CW2A(unsuitableItems[i], CP_UTF8);
			if (i != unsuitableItems.size() - 1)
			{
				oss << ", ";
			}
		}
		oss << ".";
	}

	return CString(CA2T(oss.str().c_str(), CP_UTF8));
}


/// @brief Converts a character to its numeric equivalent.
/// @param ch The character to convert.
/// @return The numeric equivalent of the character.
/// @throws std::invalid_argument if the character is not a digit or letter.
int charToNumber(char ch) {
	if (std::isdigit(ch)) {
		return ch - '0';
	}
	else if (std::isalpha(ch)) {
		return std::toupper(ch) - 'A' + 10;
	}
	else {
		throw std::invalid_argument("Invalid character in IBAN");
	}
}

/// @brief Validates an IBAN using the modulo 97 algorithm.
/// @param iban The IBAN string to validate.
/// @return TRUE if the IBAN is valid, otherwise FALSE.
bool checkModulo97(const std::string& iban) {
	std::string rearrangedIban;

	// Step 1: Rearrange the IBAN
	try
	{
		rearrangedIban = iban.substr(4) + iban.substr(0, 4);
	}
	catch (const std::out_of_range& e)
	{
		return false;
	}

	// Step 2: Convert letters to numbers
	std::string numericIban;
	try {
		for (char ch : rearrangedIban) {
			numericIban += std::to_string(charToNumber(ch));
		}
	}
	catch (const std::invalid_argument& e) {
		return false;
	}

	// Step 3: Perform modulo 97 operation
	std::string::size_type pos = 0;
	long long remainder = 0;
	try
	{
		while (pos < numericIban.size()) {
			// Concatenate the remainder with the next 9 digits (or fewer if remaining digits < 9)
			std::string part = std::to_string(remainder) + numericIban.substr(pos, 9);
			remainder = std::stoll(part) % 97;  // Use stoll instead of stoi to handle larger numbers
			pos += 9;
		}
	}
	catch (const std::out_of_range& e)
	{
		return false;
	}

	// Step 4: Check the result
	return remainder == 1;
}


/// @brief Validates the recipient's IBAN.
/// @param prejemnikIBAN The recipient's IBAN as a CString.
/// @return TRUE if the IBAN is valid, otherwise FALSE.
bool checkPrejemnikIBAN(CString prejemnikIBAN)
{
	// Convert CString to std::string
	CT2CA pszConvertedAnsiString(prejemnikIBAN);
	std::string strPrejemnikIBAN(pszConvertedAnsiString);

	// Define the regex pattern
	std::regex pattern("[0-9A-Za-z\\- ]{0,42}");

	// Check if the prejemnikIBAN matches the pattern
	if (!std::regex_match(strPrejemnikIBAN, pattern))
	{
		return false;
	}

	// Check if the IBAN is valid using modulo 97
	return checkModulo97(strPrejemnikIBAN);
}

/// @brief Validates the recipient's name.
/// @param prejemnikNaziv The recipient's name as a CString.
/// @return TRUE if the name is valid, otherwise FALSE.
bool checkNazivPrejemnika(CString prejemnikNaziv)
{
	// Convert CString to std::string
	CT2CA pszConvertedAnsiString(prejemnikNaziv);
	std::string strPrejemnikNaziv(pszConvertedAnsiString);

	// Define the regex pattern
	std::regex pattern(".{0,70}");

	// Check if the prejemnikNaziv matches the pattern
	return std::regex_match(strPrejemnikNaziv, pattern);
}

/// @brief Validates the recipient's reference.
/// @param prejemnikReferenca The recipient's reference as a CString.
/// @return TRUE if the reference is valid, otherwise FALSE.
bool checkReferencaPrejemnika(CString prejemnikReferenca)
{
	// Convert CString to std::string
	CT2CA pszConvertedAnsiString(prejemnikReferenca);
	std::string strPrejemnikReferenca(pszConvertedAnsiString);

	// Define the regex pattern
	std::regex pattern("[0-9A-Za-z\\- ]{0,25}");

	// Check if the prejemnikNaziv matches the pattern
	return std::regex_match(strPrejemnikReferenca, pattern);
}

/// @brief Validates the payment purpose.
/// @param namen The payment purpose as a CString.
/// @return TRUE if the purpose is valid, otherwise FALSE.
bool checkPlacnikNamen(CString namen)
{
	// Convert CString to std::string
	CT2CA pszConvertedAnsiString(namen);
	std::string strNamen(pszConvertedAnsiString);

	// Define the regex pattern
	std::regex pattern("[0-9A-Za-z\\- ]{0,42}");

	// Check if the namen matches the pattern
	return std::regex_match(strNamen, pattern);
}

/// @brief Validates the recipient's amount.
/// @param prejemnikZnesek The recipient's amount as a CString.
/// @return TRUE if the amount is valid, otherwise FALSE.
bool checkPrejemnikZnesek(CString prejemnikZnesek)
{
	// Convert CString to std::string
	CT2CA pszConvertedAnsiString(prejemnikZnesek);
	std::string strPrejemnikZnesek(pszConvertedAnsiString);

	// Define the regex pattern
	std::regex pattern(R"(^\d{1,11}(,\d{1,2})?$)");

	// Check if the prejemnikZnesek matches the pattern
	return std::regex_match(strPrejemnikZnesek, pattern);
}

/// @brief Parses a concatenated CString into a NalogData object.
/// @param concatenatedData The concatenated string to be parsed.
/// @return A NalogData object.
NalogData unconcatenateNalogData(const CString& concatenatedData)
{
	NalogData nalogData;
	int start = 0;
	int end = 0;
	std::vector<CString*> fields = {
		&nalogData.placnik.referencaKoda,
		&nalogData.placnik.referenca,
		&nalogData.placnik.kodaNamena,
		&nalogData.placnik.namen,
		&nalogData.placnik.nujno,
		&nalogData.prejemnik.Znesek,
		&nalogData.prejemnik.datumPlacila,
		&nalogData.prejemnik.BIC,
		&nalogData.prejemnik.IBAN,
		&nalogData.prejemnik.referencaKoda,
		&nalogData.prejemnik.referenca,
		&nalogData.prejemnik.naziv,
		&nalogData.prejemnik.naslov,
		&nalogData.prejemnik.kraj,
		&nalogData.prejemnik.drzava
	};

	for (auto& field : fields)
	{
		end = concatenatedData.Find(_T('|'), start);
		if (end == -1) {
			*field = concatenatedData.Mid(start);
			break;
		}
		*field = concatenatedData.Mid(start, end - start);
		start = end + 1;
	}

	return nalogData;
}

/// @brief Concatenates the fields of NalogData into a single CString separated by |.
/// @param nalogData The data to be concatenated.
/// @return A concatenated CString.
CString concatenateNalogData(const NalogData& nalogData)
{
	CString concatenatedData;

	concatenatedData.Format(_T("%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s|%s"),
		nalogData.placnik.referencaKoda,
		nalogData.placnik.referenca,
		nalogData.placnik.kodaNamena,
		nalogData.placnik.namen,
		nalogData.placnik.nujno,
		nalogData.prejemnik.Znesek,
		nalogData.prejemnik.datumPlacila,
		nalogData.prejemnik.BIC,
		nalogData.prejemnik.IBAN,
		nalogData.prejemnik.referencaKoda,
		nalogData.prejemnik.referenca,
		nalogData.prejemnik.naziv,
		nalogData.prejemnik.naslov,
		nalogData.prejemnik.kraj,
		nalogData.prejemnik.drzava
	);

	return concatenatedData;
}