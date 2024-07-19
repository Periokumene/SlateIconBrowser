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
		CopyText = USlateIconBrowserUserSettings::Get()->CustomStyle.Replace(TEXT("$1"), *Name.ToString());
		break;
	}
	return CopyText;
}

void FSlateIconBrowserUtils::
CacheRowDescs(TArray<FSlateIconBrowserRowDesc>& RowDescArrOut)
{
	const ISlateStyle* Style = FSlateStyleRegistry::FindSlateStyle(USlateIconBrowserUserSettings::Get()->SelectedStyle);
	const FSlateStyleSet* StyleSet = static_cast<const FSlateStyleSet*>(Style);
	if (!StyleSet){
		ensureMsgf(false, TEXT("Unexpected nullptr"));
	}
	
	const int32 KeyNum = HackerMisc::GetStyleKeys(StyleSet).Num();
	RowDescArrOut.Empty(KeyNum);
	
	// Brush
	// {
	// 	TArray<FName> Keys;
	// 	TMap<FName, FSlateBrush*>* BrushMap = Hacker::Steal_BrushResources(StyleSet);
	// 	BrushMap->GenerateKeyArray(Keys);
	// 	for (const FName& Key : Keys)
	// 	{
	// 		const FSlateBrush* Brush = Style->GetOptionalBrush(Key);
	// 		const bool bValidBrush   = Brush && Brush != FStyleDefaults::GetNoBrush();
	// 		if (bValidBrush){
	// 			RowDescArrOut.Emplace(USlateIconBrowserUserSettings::Get()->SelectedStyle, Key, ESlateIconBrowserRowType::Brush);
	// 		}
	// 	}
	// }
	
	// Font
	{
		TArray<FName> Keys;
		TMap<FName, FSlateFontInfo>* FontMap = Hacker::Steal_FontInfoResources(StyleSet);
		FontMap->GenerateKeyArray(Keys);
		for (const FName& Key : Keys){
			RowDescArrOut.Emplace(USlateIconBrowserUserSettings::Get()->SelectedStyle, Key, ESlateIconBrowserRowType::Font);
		}
	}
}


#undef LOCTEXT_NAMESPACE
