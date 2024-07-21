// Copyright 2022 sirjofri. Licensed under MIT license. See License.txt for full license text.

#pragma once

#include "CoreMinimal.h"
#include "EditorFontGlyphs.h"
#include "Engine/DeveloperSettings.h"
#include "UObject/Object.h"
#include "SlateIconBrowserUserSettings.generated.h"


UENUM()
enum class ESlateIconBrowserRowFilterType
{
	Brush,
	Widget,
	Font,
	FontAwesome,

	// TODO
	// FontDefault,
};


// Name this enum `Style` make it confused with WidgetStyle,
// but actually they mean totally different two things
UENUM()
enum ECopyCodeFormat
{
	CF_FSlateIcon,
	CF_FSlateIconFinderFindIcon,
	CF_Custom,
};



UCLASS(Config=EditorPerProjectUserSettings, ConfigDoNotCheckDefaults)
class USlateIconBrowserUserSettings : public UObject
{
	GENERATED_BODY()
	
public:
	static const USlateIconBrowserUserSettings* Get()  { return GetDefault<USlateIconBrowserUserSettings>(); }
	static USlateIconBrowserUserSettings* GetMutable() { return GetMutableDefault<USlateIconBrowserUserSettings>(); }
	static FText GetCopyCodeFormatDescription(ECopyCodeFormat Format);

	// Filter Related
	void ValidateConfig();
	
	UPROPERTY()
	FString FilterString;
	
	UPROPERTY(Config)
	FString FormatCustomExpression;

	UPROPERTY(Config)
	FName SelectedStyle;

	UPROPERTY(Config)
	TEnumAsByte<ECopyCodeFormat> CopyCodeFormat;

	
	// Widget Style Visualizer
	UPROPERTY()
	FName HighlightWidgetStyleName;
	
	UPROPERTY()
	bool bWidgetInsertText;
	
	UPROPERTY()
	FText FontPreviewText = FText::FromString(TEXT("Why did we play Haruhikage?!")); // Just for fun!

	UPROPERTY()
	FText FontAwesomePreviewText;

	// Row Type Filter
	UPROPERTY()
	ESlateIconBrowserRowFilterType RowType;


private:
	UPROPERTY(VisibleAnywhere, Transient)
	TMap<FName, FString> BaseTranslator;
	
	UPROPERTY(EditAnywhere, Config)
	TMap<FName, FString> AdditionalTranslator;
	
	bool bTranslatorInitialized = false;
	
public:
	void InitTranslator();
	FString TryTranslate(const FName& InStyleSetName);

public:
	// Diable Row List Selection, hover color will be lost when right click menu pop up
	// So use this to optimize hover color change
	TWeakPtr<SWidget> LastHoveredRow;


public:
	void InitGlyphList();
	TArray<TSharedPtr<FString>> GlyphNames;
	TMap<TSharedPtr<FString>, FText> GlyphMap;

private:
	bool bGlyphInit;
};