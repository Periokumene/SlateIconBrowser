// Copyright 2022 sirjofri. Licensed under MIT license. See License.txt for full license text.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SlateIconBrowserUserSettings.generated.h"


UENUM()
enum class ESlateIconBrowserRowFilterType
{
	Brush,
	Widget,
	Font,
	FontDefault,
	FontAwesome,
};


UENUM()
enum ECopyCodeStyle
{
	CS_FSlateIcon,
	CS_FSlateIconFinderFindIcon,
	CS_CustomStyle,
};

UCLASS(Config=EditorPerProjectUserSettings, ConfigDoNotCheckDefaults)
class USlateIconBrowserUserSettings : public UObject
{
	GENERATED_BODY()
	
public:
	// I think there is no need to make filter string serialized by config
	// This makes it cumbersome if I open IconBrowser next time but I just want to search another Brush
	UPROPERTY()
	FString FilterString;

	UPROPERTY(Config)
	FString CustomStyle;

	UPROPERTY(Config)
	FName SelectedStyle;

	UPROPERTY(Config)
	TEnumAsByte<ECopyCodeStyle> CopyCodeStyle;

	UPROPERTY()
	bool bWidgetInsertText;
	
	UPROPERTY()
	FText FontPreviewText = FText::FromString(TEXT("Why did we play Haruhikage?!")); // Just for fun!

	UPROPERTY()
	ESlateIconBrowserRowFilterType RowType;

	static const USlateIconBrowserUserSettings* Get()  { return GetDefault<USlateIconBrowserUserSettings>(); }
	static USlateIconBrowserUserSettings* GetMutable() { return GetMutableDefault<USlateIconBrowserUserSettings>(); }
};
