#pragma once
#include "SlateIconBrowserUserSettings.h"


struct FSlateIconBrowserRowDesc;

struct FSlateIconBrowserUtils
{
	static void CopyIconCodeToClipboard(FName Name, ECopyCodeStyle CodeStyle);
	static FString GenerateCopyCode(FName Name, ECopyCodeStyle CodeStyle);
	static void CacheRowDescs(TArray<FSlateIconBrowserRowDesc>& RowDescArrOut);
};

