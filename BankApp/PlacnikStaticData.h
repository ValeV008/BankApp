#pragma once

#include <afxstr.h>

struct PlacnikStaticData {
    CString iban;
    CString ime;
    CString naslov;
    CString kraj;
    CString drzava;
};

extern PlacnikStaticData g_placnikStaticData; // Declaration of the global variable
