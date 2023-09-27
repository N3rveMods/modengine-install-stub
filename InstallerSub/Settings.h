#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/** 
	Modify URL_FOR_DOWNLOAD_WSTR to change the http/https URL of the zip file.
*/
#define URL_FOR_DOWNLOAD_WSTR (L"https://modsengine.com/modengine.zip")
//#define URL_FOR_DOWNLOAD_WSTR (L"https://www.dailymobilegadgets.info/TestDataAround40MB.zip")


/**
	If DOWNLOAD_TO_C_ROOT_DIR is set to true, the URL file will be downloaded at root directory of C. Else, it will be downloaded to the installation folder selected by user.
*/
#define DOWNLOAD_TO_C_ROOT_DIR				false //true

/**
	PROGRESSBAR_STEPS_FOR_DOWNLOADING -- the steps for downloading in progress bar.
	PROGRESSBAR_STEPS_FOR_UNZIPPING -- the steps for unzipping/installing in progress bar.
*/
#define PROGRESSBAR_STEPS_FOR_DOWNLOADING	50 // 5
#define PROGRESSBAR_STEPS_FOR_UNZIPPING		30 // 3

#ifdef __cplusplus
}
#endif
