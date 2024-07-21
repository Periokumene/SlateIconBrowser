#include "SlateIconBrowserUtils.h"

#include "SlateIconBrowserHacker.h"
#include "SSlateIconBrowserRow.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Styling/SlateStyle.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Windows/WindowsPlatformApplicationMisc.h"
#include "Styling/SlateStyleRegistry.h"

#define LOCTEXT_NAMESPACE "FSlateIconBrowserUtils"

void FSlateIconBrowserUtils::
CopyIconCodeToClipboard(FName Name, ECopyCodeStyle CodeStyle)
{
	FString CopyText = GenerateCopyCode(Name, CodeStyle);
	FPlatformApplicationMisc::ClipboardCopy(*CopyText);
	UE_LOG(LogTemp, Warning, TEXT("Copy code to clipboard: %s"), *CopyText);

	FNotificationInfo Info(LOCTEXT("CopiedNotification", "C++ code copied to clipboard"));
	Info.ExpireDuration = 3.f;
#if ENGINE_MAJOR_VERSION == 5
	Info.SubText = FText::FromString(CopyText);
#else
	Info.Text = FText::FromString(CopyText);
#endif
	FSlateNotificationManager::Get().AddNotification(Info);
}


FString FSlateIconBrowserUtils::
GenerateCopyCode(FName Name, ECopyCodeStyle CodeStyle)
{
	FString CopyText(TEXT(""));
	switch (CodeStyle) {
	case CS_FSlateIcon:
		// CopyText = FString::Printf(TEXT("FSlateIcon(%s, \"%s\")"), *TranslateDefaultStyleSets(USlateIconBrowserUserSettings::Get()->SelectedStyle), *Name.ToString());
		CopyText = FString::Printf(TEXT("FSlateIcon(%s, \"%s\")"), *USlateIconBrowserUserSettings::Get()->SelectedStyle.ToString(), *Name.ToString()); // TODO Translate is commented
		break;
	case CS_FSlateIconFinderFindIcon:
		CopyText = FString::Printf(TEXT("FSlateIconFinder::FindIcon(\"%s\")"), *Name.ToString());
		break;
	case CS_CustomStyle:
		CopyText = USlateIconBrowserUserSettings::Get()->CustomFormat.Replace(TEXT("$1"), *Name.ToString());
		break;
	}
	return CopyText;
}

#undef LOCTEXT_NAMESPACE
