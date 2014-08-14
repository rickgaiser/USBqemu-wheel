#include "../../USB.h"
#include "raw-config.h"
#include <setupapi.h>

_HidD_GetHidGuid HidD_GetHidGuid = NULL;
_HidD_GetAttributes HidD_GetAttributes = NULL;
_HidD_GetPreparsedData HidD_GetPreparsedData = NULL;
_HidP_GetCaps HidP_GetCaps = NULL;
_HidD_FreePreparsedData HidD_FreePreparsedData = NULL;
_HidD_GetFeature HidD_GetFeature = NULL;
_HidD_SetFeature HidD_SetFeature = NULL;
_HidP_GetSpecificButtonCaps HidP_GetSpecificButtonCaps = NULL;
_HidP_GetButtonCaps HidP_GetButtonCaps = NULL;
_HidP_GetUsages HidP_GetUsages = NULL;
_HidP_GetValueCaps HidP_GetValueCaps = NULL;
_HidP_GetUsageValue HidP_GetUsageValue = NULL;
_HidD_GetProductString HidD_GetProductString = NULL;

static HMODULE hModHid = 0;

int InitHid() {
    if (hModHid) {
        return 1;
    }
    hModHid = LoadLibraryA("hid.dll");
    if (hModHid) {
        if ((HidD_GetHidGuid = (_HidD_GetHidGuid)GetProcAddress(hModHid, "HidD_GetHidGuid")) &&
            (HidD_GetAttributes = (_HidD_GetAttributes)GetProcAddress(hModHid, "HidD_GetAttributes")) &&
            (HidD_GetPreparsedData = (_HidD_GetPreparsedData)GetProcAddress(hModHid, "HidD_GetPreparsedData")) &&
            (HidP_GetCaps = (_HidP_GetCaps)GetProcAddress(hModHid, "HidP_GetCaps")) &&
            (HidD_FreePreparsedData = (_HidD_FreePreparsedData)GetProcAddress(hModHid, "HidD_FreePreparsedData")) &&
            (HidP_GetSpecificButtonCaps = (_HidP_GetSpecificButtonCaps)GetProcAddress(hModHid, "HidP_GetSpecificButtonCaps")) &&
            (HidP_GetButtonCaps = (_HidP_GetButtonCaps)GetProcAddress(hModHid, "HidP_GetButtonCaps")) &&
            (HidP_GetUsages = (_HidP_GetUsages)GetProcAddress(hModHid, "HidP_GetUsages")) &&
            (HidP_GetValueCaps = (_HidP_GetValueCaps)GetProcAddress(hModHid, "HidP_GetValueCaps")) &&
            (HidP_GetUsageValue = (_HidP_GetUsageValue)GetProcAddress(hModHid, "HidP_GetUsageValue")) &&
            (HidD_GetProductString = (_HidD_GetProductString)GetProcAddress(hModHid, "HidD_GetProductString")) &&
            (HidD_GetFeature = (_HidD_GetFeature)GetProcAddress(hModHid, "HidD_GetFeature")) &&
            (HidD_SetFeature = (_HidD_SetFeature)GetProcAddress(hModHid, "HidD_SetFeature"))) {
            //pHidD_GetHidGuid(&GUID_DEVINTERFACE_HID);
            return 1;
        }
        UninitHid();
    }
    return 0;
}

void UninitHid() {
    if (hModHid) {
        FreeLibrary(hModHid);
        hModHid = 0;
    }
}
