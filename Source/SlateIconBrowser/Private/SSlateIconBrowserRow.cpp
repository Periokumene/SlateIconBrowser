#include "SSlateIconBrowserRow.h"

#include "SCarouselNavigationBar.h"
#include "SCarouselNavigationButton.h"
#include "SlateIconBrowserHacker.h"
#include "SlateIconBrowserUserSettings.h"
#include "SlateOptMacros.h"
#include "SSearchableComboBox.h"
#include "SSlateIconBrowserTab.h"
#include "WidgetCarouselStyle.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"
#include "UI/Synth2DSliderStyle.h"
#include "UI/SynthKnobStyle.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Input/SVolumeControl.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"

#define LOCTEXT_NAMESPACE "SlateIconBrowserRow"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION


bool FSlateIconBrowserRowDesc::
HandleFilter(const FSlateIconBrowserFilterContext& Context) const
{
	bool bPass = true;
	const bool bValidFilterString = !Context.FilterString.Replace(TEXT(" "), TEXT("")).IsEmpty();
	bPass &= bValidFilterString ? PropertyName.ToString().Contains(Context.FilterString) : true;
	bPass &= CustomHandleFilter(Context);
	return bPass;
}


FString FSlateIconBrowserRowDesc::
GenerateCode(ECopyCodeFormat CodeFormat) const
{
	return PropertyName.ToString(); // UnImplement, just copy PropertyName
}

EVisibility FSlateIconBrowserRowDesc::
GetVisibility() const
{
	return EVisibility::Visible;
}

void FSlateIconBrowserRowDesc_Brush::
CacheFromStyle(const FSlateStyleSet* StyleOwner, TArray<TSharedPtr<FSlateIconBrowserRowDesc>>& RowListOut)
{
	TArray<FName> Keys;
	TMap<FName, FSlateBrush*>* BrushMap = Hacker::Steal_BrushResources(StyleOwner);
	BrushMap->GenerateKeyArray(Keys);
	for (const FName& Key : Keys)
	{
		const FSlateBrush* Brush = StyleOwner->GetOptionalBrush(Key);
		const bool bValidBrush   = Brush && Brush != FStyleDefaults::GetNoBrush();
		if (bValidBrush){
			RowListOut.Add(MakeShareable(new FSlateIconBrowserRowDesc_Brush(StyleOwner, Key)));
		}
	}
}


TSharedRef<SWidget> FSlateIconBrowserRowDesc_Brush::
GenerateVisualizer() const
{
	const FSlateBrush* Brush = FEditorStyle::GetBrush(TEXT("NoResourceWrongName"));
	if (const ISlateStyle* OwnerStyle = FSlateStyleRegistry::FindSlateStyle(OwnerStyleName)){
		Brush = OwnerStyle->GetOptionalBrush(PropertyName);
	}
	const FVector2D DesiredIconSize = Brush->GetImageType() == ESlateBrushImageType::NoImage ? FVector2D(20): Brush->GetImageSize();
	
	return SNew(SImage)
#if ENGINE_MAJOR_VERSION == 5
		.DesiredSizeOverride(DesiredIconSize)
#endif
		.Image(Brush);
}


FString FSlateIconBrowserRowDesc_Brush::
GenerateCode(ECopyCodeFormat CodeFormat) const
{
	// TODO Translator
	switch (CodeFormat) {
		case CF_FSlateIcon:               { return FString::Printf(TEXT("FSlateIcon(\"%s\", \"%s\")"), *OwnerStyleName.ToString(), *PropertyName.ToString()); }
		case CF_FSlateIconFinderFindIcon: { return FString::Printf(TEXT("FSlateIconFinder::FindIcon(\"%s\")"), *PropertyName.ToString()); }
		case CF_Custom:              { return USlateIconBrowserUserSettings::Get()->FormatCustomExpression.Replace(TEXT("$1"), *PropertyName.ToString()); }
		default:                          { ensureMsgf(false, TEXT("Unexpected new Format")); }
	}
	return FSlateIconBrowserRowDesc::GenerateCode(CodeFormat);
}


bool FSlateIconBrowserRowDesc_Brush::
CustomHandleFilter(const FSlateIconBrowserFilterContext& Context) const
{
	return Context.RowType == ESlateIconBrowserRowFilterType::Brush;
}


void FSlateIconBrowserRowDesc_Font::
CacheFromStyle(const FSlateStyleSet* StyleOwner, TArray<TSharedPtr<FSlateIconBrowserRowDesc>>& RowListOut)
{
	TArray<FName> Keys;
	TMap<FName, FSlateFontInfo>* FontMap = Hacker::Steal_FontInfoResources(StyleOwner);
	FontMap->GenerateKeyArray(Keys);
	for (const FName& Key : Keys){
		if (Key.ToString().Find(TEXT("FontAwesome")) == INDEX_NONE){
			RowListOut.Add(MakeShareable(new FSlateIconBrowserRowDesc_Font(StyleOwner, Key)));
		}
	}
}


TSharedRef<SWidget> FSlateIconBrowserRowDesc_Font::
GenerateVisualizer() const
{
	if (const ISlateStyle* OwnerStyle = FSlateStyleRegistry::FindSlateStyle(OwnerStyleName))
	{
		return SNew(SEditableText)
		.Font(OwnerStyle->GetFontStyle(PropertyName))
		.Text_Lambda([](){ return USlateIconBrowserUserSettings::Get()->FontPreviewText; })
		.OnTextChanged_Lambda([](const FText& InText)                     { USlateIconBrowserUserSettings::GetMutable()->FontPreviewText = InText; })
		.OnTextCommitted_Lambda([](const FText& InText, ETextCommit::Type){ USlateIconBrowserUserSettings::GetMutable()->FontPreviewText = InText; });
	}
	
	return SNew(STextBlock)
		.Text(FText::FromString(TEXT("ERROR")))
		.ColorAndOpacity(FLinearColor::Red);
}


bool FSlateIconBrowserRowDesc_Font::
CustomHandleFilter(const FSlateIconBrowserFilterContext& Context) const
{
	return Context.RowType == ESlateIconBrowserRowFilterType::Font;
}


void FSlateIconBrowserRowDesc_FontAwesome::
CacheFromStyle(const FSlateStyleSet* StyleOwner, TArray<TSharedPtr<FSlateIconBrowserRowDesc>>& RowListOut)
{
	TArray<FName> Keys;
	TMap<FName, FSlateFontInfo>* FontMap = Hacker::Steal_FontInfoResources(StyleOwner);
	FontMap->GenerateKeyArray(Keys);
	for (const FName& Key : Keys){
		if (Key.ToString().Find(TEXT("FontAwesome")) != INDEX_NONE){
			RowListOut.Add(MakeShareable(new FSlateIconBrowserRowDesc_FontAwesome(StyleOwner, Key)));
		}
	}
}


TSharedRef<SWidget> FSlateIconBrowserRowDesc_FontAwesome::
GenerateVisualizer() const
{
	if (const ISlateStyle* OwnerStyle = FSlateStyleRegistry::FindSlateStyle(OwnerStyleName))
	{
		return SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SEditableText)
				.Font(OwnerStyle->GetFontStyle(PropertyName))
				.OnTextCommitted_Lambda([](const FText& Text, ETextCommit::Type){
					USlateIconBrowserUserSettings::GetMutable()->FontAwesomePreviewText = Text;
				})
				.Text_Lambda([](){
					return USlateIconBrowserUserSettings::Get()->FontAwesomePreviewText;
				})
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(8, 0))
			[
				SNew(SSearchableComboBox)
				.OptionsSource(&USlateIconBrowserUserSettings::GetMutable()->GlyphNames)
				.OnSelectionChanged_Lambda([](TSharedPtr<FString> InGlyphName, ESelectInfo::Type)
				{
					const FText* GlyphValuePtr = USlateIconBrowserUserSettings::Get()->GlyphMap.Find(InGlyphName);
					GlyphValuePtr = GlyphValuePtr ? GlyphValuePtr : &FEditorFontGlyphs::Crosshairs;
					
					FText OldText = USlateIconBrowserUserSettings::Get()->FontAwesomePreviewText;
					FText AddText = *GlyphValuePtr;
					
					USlateIconBrowserUserSettings::GetMutable()->FontAwesomePreviewText =
						FText::Format(LOCTEXT("NewFontAwesomePreviewFomart", "{0}{1}"), OldText, AddText);
				})
				.OnGenerateWidget_Lambda([](TSharedPtr<FString> InGlyphName)
				{
					const FText* GlyphValuePtr = USlateIconBrowserUserSettings::Get()->GlyphMap.Find(InGlyphName);
					GlyphValuePtr = GlyphValuePtr ? GlyphValuePtr : &FEditorFontGlyphs::Crosshairs;
					
					return SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.HAlign(HAlign_Left)
						[
							SNew(STextBlock)
							.Text(FText::FromString(*InGlyphName))
						]
						+SHorizontalBox::Slot()
						.HAlign(HAlign_Right)
						.Padding(2)
						[
							SNew(STextBlock)
							.Font(FEditorStyle::GetFontStyle("FontAwesome.16"))
							.Text(*GlyphValuePtr)
						];
				})
				[
					SNew(STextBlock)
					.Text(LOCTEXT("GlyphPickerTitle", "Select"))
				]
			];
	}
	
	return SNew(STextBlock)
		.Text(FText::FromString(TEXT("ERROR")))
		.ColorAndOpacity(FLinearColor::Red);
}


bool FSlateIconBrowserRowDesc_FontAwesome::
CustomHandleFilter(const FSlateIconBrowserFilterContext& Context) const
{
	return Context.RowType == ESlateIconBrowserRowFilterType::FontAwesome;
}

void FSlateIconBrowserRowDesc_Widget::
CacheFromStyle(const FSlateStyleSet* StyleOwner, TArray<TSharedPtr<FSlateIconBrowserRowDesc>>& RowListOut)
{
	// Seldom Need Customization maybe ?
	const TSet<FName> MeaningLessStyleType = {
		FScrollBorderStyle::TypeName,
		FSynth2DSliderStyle::TypeName,
		FSynthKnobStyle::TypeName,
		FDockTabStyle::TypeName,
		FTableColumnHeaderStyle::TypeName,
		FHeaderRowStyle::TypeName,
		FWindowStyle::TypeName
	};
	
	TMap<FName, TSharedRef<FSlateWidgetStyle>>* WidgetStyleMap = Hacker::Steal_WidgetStyleValues(StyleOwner);
	
	for (const auto& Pair : *WidgetStyleMap){
		if (!MeaningLessStyleType.Find(Pair.Value->GetTypeName()))
		{
			RowListOut.Add(MakeShareable(new FSlateIconBrowserRowDesc_Widget(StyleOwner, Pair.Key, Pair.Value)));
		}
	}
}

TSharedRef<SWidget> FSlateIconBrowserRowDesc_Widget::
GenerateVisualizer() const
{
	if (WidgetStyleTypeName == FButtonStyle::TypeName)    { return GenerateVisualizer_Button();    }
	if (WidgetStyleTypeName == FCheckBoxStyle::TypeName)  { return GenerateVisualizer_CheckBox();  }
	if (WidgetStyleTypeName == FTextBlockStyle::TypeName) { return GenerateVisualizer_TextBlock(); }
	if (WidgetStyleTypeName == FComboBoxStyle::TypeName)  { return GenerateVisualizer_ComboBox();  }
	
	if (WidgetStyleTypeName == FComboButtonStyle::TypeName)       { return GenerateVisualizer_ComboButton();       }
	if (WidgetStyleTypeName == FDockTabStyle::TypeName)           { return GenerateVisualizer_DockTab();           }
	if (WidgetStyleTypeName == FEditableTextBoxStyle::TypeName)   { return GenerateVisualizer_EditableTextBox();   }
	if (WidgetStyleTypeName == FEditableTextStyle::TypeName)      { return GenerateVisualizer_EditableText();      }
	if (WidgetStyleTypeName == FExpandableAreaStyle::TypeName)    { return GenerateVisualizer_ExpandableArea();    }
	if (WidgetStyleTypeName == FHeaderRowStyle::TypeName)         { return GenerateVisualizer_HeaderRow();         }
	if (WidgetStyleTypeName == FHyperlinkStyle::TypeName)         { return GenerateVisualizer_Hyperlink();         }
	if (WidgetStyleTypeName == FProgressBarStyle::TypeName)       { return GenerateVisualizer_ProgressBar();       }
	if (WidgetStyleTypeName == FScrollBarStyle::TypeName)         { return GenerateVisualizer_ScrollBar();         }
	if (WidgetStyleTypeName == FScrollBorderStyle::TypeName)      { return GenerateVisualizer_ScrollBorder();      }
	if (WidgetStyleTypeName == FScrollBoxStyle::TypeName)         { return GenerateVisualizer_ScrollBox();         }
	if (WidgetStyleTypeName == FSearchBoxStyle::TypeName)         { return GenerateVisualizer_SearchBox();         }
	if (WidgetStyleTypeName == FSliderStyle::TypeName)            { return GenerateVisualizer_Slider();            }
	if (WidgetStyleTypeName == FSpinBoxStyle::TypeName)           { return GenerateVisualizer_SpinBox();           }
	if (WidgetStyleTypeName == FSplitterStyle::TypeName)          { return GenerateVisualizer_Splitter();          }
	if (WidgetStyleTypeName == FTableColumnHeaderStyle::TypeName) { return GenerateVisualizer_TableColumnHeader(); }
	if (WidgetStyleTypeName == FTableRowStyle::TypeName)          { return GenerateVisualizer_TableRow();          }
	if (WidgetStyleTypeName == FVolumeControlStyle::TypeName)     { return GenerateVisualizer_VolumeControl();     }
	if (WidgetStyleTypeName == FSynth2DSliderStyle::TypeName)     { return GenerateVisualizer_Synth2DSlider();     }
	if (WidgetStyleTypeName == FSynthKnobStyle::TypeName)         { return GenerateVisualizer_SynthKnob();         }

	if (WidgetStyleTypeName == FInlineEditableTextBlockStyle::TypeName)        { return GenerateVisualizer_InlineEditableTextBlock(); }
	if (WidgetStyleTypeName == FWidgetCarouselNavigationBarStyle::TypeName)    { return GenerateVisualizer_WidgetCarouseNavigationBar(); }
	if (WidgetStyleTypeName == FWidgetCarouselNavigationButtonStyle::TypeName) { return GenerateVisualizer_WidgetCarouseNavigationButton(); }
	if (WidgetStyleTypeName == FWindowStyle::TypeName)                         { return GenerateVisualizer_Window() ; }
	
	return GenerateVisualizer_Failure();
}


TSharedRef<SWidget> FSlateIconBrowserRowDesc_Widget::
GenerateVisualizer_Button() const
{
	if (const FButtonStyle* WS = GetLegalWidgetStyle<FButtonStyle>())
	{
		return SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		[
			SNew(SButton)
			.Visibility_Static(&FSlateIconBrowserRowDesc_Widget::GetVisibility_Base)
			.ButtonStyle(WS)
		]
		+SHorizontalBox::Slot()
		[
			SNew(SButton)
			.Text_Static(&FSlateIconBrowserRowDesc_Widget::GetInsertText)
			.Visibility_Static(&FSlateIconBrowserRowDesc_Widget::GetVisibility_Insert)
			.ButtonStyle(WS)
		];
	}
	return GenerateVisualizer_Failure();
}


TSharedRef<SWidget> FSlateIconBrowserRowDesc_Widget::
GenerateVisualizer_CheckBox() const
{
	if (const FCheckBoxStyle* WS = GetLegalWidgetStyle<FCheckBoxStyle>())
	{
		return SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		[
			SNew(SCheckBox)
			.Visibility_Static(&FSlateIconBrowserRowDesc_Widget::GetVisibility_Base)
			.Style(WS)
		]
		+SHorizontalBox::Slot()
		[
			SNew(SCheckBox)
			.Visibility_Static(&FSlateIconBrowserRowDesc_Widget::GetVisibility_Insert)
			.Style(WS)
			[
				SNew(STextBlock)
				.Text_Static(&FSlateIconBrowserRowDesc_Widget::GetInsertText)
			]
		];
	}
	return GenerateVisualizer_Failure();
}


TSharedRef<SWidget> FSlateIconBrowserRowDesc_Widget::
GenerateVisualizer_TextBlock() const
{
	if (const FTextBlockStyle* WS = GetLegalWidgetStyle<FTextBlockStyle>())
    {
    	return SNew(STextBlock)
			.Text_Static(&FSlateIconBrowserRowDesc_Widget::GetInsertText)
			.TextStyle(WS);
    }
    return GenerateVisualizer_Failure();
}

TSharedRef<SWidget> FSlateIconBrowserRowDesc_Widget::
GenerateVisualizer_EditableTextBox() const
{
	if (const FEditableTextBoxStyle* WS = GetLegalWidgetStyle<FEditableTextBoxStyle>())
	{
		return SNew(SEditableTextBox)
			.Style(WS)
			.Text_Static(&FSlateIconBrowserRowDesc_Widget::GetInsertText);
	}
	return GenerateVisualizer_Failure();
}


TSharedRef<SWidget> FSlateIconBrowserRowDesc_Widget::
GenerateVisualizer_EditableText() const
{
	if (const FEditableTextStyle* WS = GetLegalWidgetStyle<FEditableTextStyle>())
	{
		return SNew(SEditableText)
			.Style(WS)
			.Text_Static(&FSlateIconBrowserRowDesc_Widget::GetInsertText);
	}
	return GenerateVisualizer_Failure();
}


TSharedRef<SWidget> FSlateIconBrowserRowDesc_Widget::
GenerateVisualizer_Hyperlink() const
{
	if (const FHyperlinkStyle* WS = GetLegalWidgetStyle<FHyperlinkStyle>())
	{
		return SNew(SHyperlink)
			.Style(WS)
			.Text_Static(&FSlateIconBrowserRowDesc_Widget::GetInsertText)
			.OnNavigate_Lambda([](){ FMessageDialog::Debugf(FText::FromString(TEXT("This is a HyperLink"))); });
	}
	return GenerateVisualizer_Failure();
}


TSharedRef<SWidget> FSlateIconBrowserRowDesc_Widget::
GenerateVisualizer_InlineEditableTextBlock() const
{
	if (const FInlineEditableTextBlockStyle* WS = GetLegalWidgetStyle<FInlineEditableTextBlockStyle>())
	{
		return SNew(SInlineEditableTextBlock)
			.Style(WS)
			.Text_Static(&FSlateIconBrowserRowDesc_Widget::GetInsertText);
	}
	return GenerateVisualizer_Failure();
}


TSharedRef<SWidget> FSlateIconBrowserRowDesc_Widget::
GenerateVisualizer_ComboBox() const
{
	if (const FComboBoxStyle* WS = GetLegalWidgetStyle<FComboBoxStyle>())
	{
		return SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		[
			SNew(SComboBox<TSharedPtr<FName>>)
			.ComboBoxStyle(WS)
			.Visibility_Static(&FSlateIconBrowserRowDesc_Widget::GetVisibility_Base)
		]
		+SHorizontalBox::Slot()
		[
			SNew(SComboBox<TSharedPtr<FName>>)
			.ComboBoxStyle(WS)
			.Visibility_Static(&FSlateIconBrowserRowDesc_Widget::GetVisibility_Insert)
			[
				SNew(STextBlock)
				.Text_Static(&FSlateIconBrowserRowDesc_Widget::GetInsertText)
			]
		];
	}
	return GenerateVisualizer_Failure();
}


TSharedRef<SWidget> FSlateIconBrowserRowDesc_Widget::
GenerateVisualizer_ComboButton() const
{
	if (const FComboButtonStyle* WS = GetLegalWidgetStyle<FComboButtonStyle>())
	{
		return SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		[
			SNew(SComboButton)
			.ComboButtonStyle(WS)
			.Visibility_Static(&FSlateIconBrowserRowDesc_Widget::GetVisibility_Base)
		]
		+SHorizontalBox::Slot()
		[
			SNew(SComboButton)
			.ComboButtonStyle(WS)
			.Visibility_Static(&FSlateIconBrowserRowDesc_Widget::GetVisibility_Insert)
			.ButtonContent()
			[
				SNew(STextBlock)
				.Text_Static(&FSlateIconBrowserRowDesc_Widget::GetInsertText)
			]
		];
	}
	return GenerateVisualizer_Failure();
}

TSharedRef<SWidget> FSlateIconBrowserRowDesc_Widget::
GenerateVisualizer_ExpandableArea() const
{
	if (const FExpandableAreaStyle* WS = GetLegalWidgetStyle<FExpandableAreaStyle>())
	{
		return SNew(SExpandableArea)
			.Style(WS)
			.HeaderContent() [ SNew(STextBlock).Text_Static(&FSlateIconBrowserRowDesc_Widget::GetInsertText) ]
			.BodyContent()   [ SNew(STextBlock).Text_Static(&FSlateIconBrowserRowDesc_Widget::GetInsertText) ];
	}
	return GenerateVisualizer_Failure();
}


TSharedRef<SWidget> FSlateIconBrowserRowDesc_Widget::
GenerateVisualizer_ProgressBar() const
{
	if (const FProgressBarStyle* WS = GetLegalWidgetStyle<FProgressBarStyle>())
	{
		return SNew(SBox)
			.WidthOverride(256)
			[
				SNew(SProgressBar)
				.Percent(0.393939)
				.Style(WS)
			];
	}
	return GenerateVisualizer_Failure();
}


TSharedRef<SWidget> FSlateIconBrowserRowDesc_Widget::
GenerateVisualizer_Slider() const
{
	if (const FSliderStyle* WS = GetLegalWidgetStyle<FSliderStyle>())
	{
		return SNew(SBox)
			.WidthOverride(256)
			[
				SNew(SSlider).Style(WS)
			];
	}
	return GenerateVisualizer_Failure();
}

TSharedRef<SWidget> FSlateIconBrowserRowDesc_Widget::
GenerateVisualizer_SpinBox() const
{
	if (const FSpinBoxStyle* WS = GetLegalWidgetStyle<FSpinBoxStyle>())
	{
		return SNew(SSpinBox<int>)
			.MinDesiredWidth(50)
			.MaxFractionalDigits(1)
			.MinValue(0)
			.MaxValue(1024)
			.Style(WS);
	}
	return GenerateVisualizer_Failure();
}


TSharedRef<SWidget> FSlateIconBrowserRowDesc_Widget::
GenerateVisualizer_VolumeControl() const
{
	if (const FVolumeControlStyle* WS = GetLegalWidgetStyle<FVolumeControlStyle>())
	{
		return SNew(SVolumeControl).Style(WS);
	}
	return GenerateVisualizer_Failure();
}


TSharedRef<SWidget> FSlateIconBrowserRowDesc_Widget::
GenerateVisualizer_SearchBox() const
{
	if (const FSearchBoxStyle* WS = GetLegalWidgetStyle<FSearchBoxStyle>())
	{
		return SNew(SSearchBox)
			.MinDesiredWidth(256)
			.HintText_Static(&FSlateIconBrowserRowDesc_Widget::GetInsertText)
			.Style(WS);
	}
	return GenerateVisualizer_Failure();
}


TSharedRef<SWidget> FSlateIconBrowserRowDesc_Widget::
GenerateVisualizer_ScrollBar() const
{
	if (const FScrollBarStyle* WS = GetLegalWidgetStyle<FScrollBarStyle>())
	{
		return SNew(SScrollBar).Style(WS);
	}
	return GenerateVisualizer_Failure();
}


TSharedRef<SWidget> FSlateIconBrowserRowDesc_Widget::
GenerateVisualizer_ScrollBox() const
{
	if (const FScrollBoxStyle* WS = GetLegalWidgetStyle<FScrollBoxStyle>())
	{
		return SNew(SScrollBox).Style(WS);
	}
	return GenerateVisualizer_Failure();
}

TSharedRef<SWidget> FSlateIconBrowserRowDesc_Widget::
GenerateVisualizer_Splitter() const
{
	if (const FSplitterStyle* WS = GetLegalWidgetStyle<FSplitterStyle>())
	{
		return SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			[ SNew(SBorder).Padding(FMargin(32, 2)) ]
			+SHorizontalBox::Slot()
			[ SNew(SSplitter).Orientation(Orient_Vertical).Style(WS) ]
			+SHorizontalBox::Slot()
			[ SNew(SBorder).Padding(FMargin(32, 2)) ];
	}
	return GenerateVisualizer_Failure();
}



TSharedRef<SWidget> FSlateIconBrowserRowDesc_Widget::
GenerateVisualizer_TableRow() const
{
	// TODO Table Row Visualizer is a little complex
	return GenerateVisualizer_Failure();
}


TSharedRef<SWidget> FSlateIconBrowserRowDesc_Widget::
GenerateVisualizer_Failure(const FText& InComment) const
{
	const FText TypeName   = FText::FromName(WidgetStyleTypeName);
	return SNew(STextBlock)
	.Text(FText::Format(LOCTEXT("WSTypeHint", "{0} | {1} "), TypeName, InComment))
	.ColorAndOpacity(FColor::Orange);
}



TSharedRef<SWidget> FSlateIconBrowserRowDesc_Widget::
GenerateVisualizer_WidgetCarouseNavigationBar() const
{
	if (const FWidgetCarouselNavigationBarStyle* WS = GetLegalWidgetStyle<FWidgetCarouselNavigationBarStyle>())
	{
		return SNew(SCarouselNavigationBar)
			.ItemCount(10)
			.Style(WS);
	}
	return GenerateVisualizer_Failure();
}

TSharedRef<SWidget> FSlateIconBrowserRowDesc_Widget::
GenerateVisualizer_WidgetCarouseNavigationButton() const
{
	if (const FWidgetCarouselNavigationButtonStyle* WS = GetLegalWidgetStyle<FWidgetCarouselNavigationButtonStyle>())
	{
		return SNew(SCarouselNavigationButton).Style(WS);
	}
	return GenerateVisualizer_Failure();
}

EVisibility FSlateIconBrowserRowDesc_Widget::
GetVisibility_Base()
{
	return USlateIconBrowserUserSettings::Get()->bWidgetInsertText ? EVisibility::Collapsed : EVisibility::Visible;
}

EVisibility FSlateIconBrowserRowDesc_Widget::
GetVisibility_Insert()
{
	return USlateIconBrowserUserSettings::Get()->bWidgetInsertText ? EVisibility::Visible : EVisibility::Collapsed;
}

FText FSlateIconBrowserRowDesc_Widget::
GetInsertText()
{
	return USlateIconBrowserUserSettings::Get()->FontPreviewText;
}


bool FSlateIconBrowserRowDesc_Widget::
CustomHandleFilter(const FSlateIconBrowserFilterContext& Context) const
{
	return Context.RowType == ESlateIconBrowserRowFilterType::Widget;
}

EVisibility FSlateIconBrowserRowDesc_Widget::
GetVisibility() const
{
	// TODO IsNone? Better a unique specifier
	FName HighlightName = USlateIconBrowserUserSettings::Get()->HighlightWidgetStyleName;
	const bool bVisible = HighlightName.IsNone() || HighlightName == WidgetStyleTypeName;
	return bVisible ? EVisibility::Visible : EVisibility::Collapsed;
}

void FSlateIconBrowserRowDesc_Widget::
CustomGenDetailFilter(TSharedPtr<SSlateIconBrowserTab> TabOwner)
{
	bool bExist = false;
	for (const auto& WSNamePtr : TabOwner->WidgetStyleList){
		if (WSNamePtr->IsEqual(WidgetStyleTypeName)){
			bExist = true;
			break;
		}
	}

	if (!bExist)
	{
		TabOwner->WidgetStyleList.Add(MakeShared<FName>(WidgetStyleTypeName));
	}
}


void SSlateIconBrowserRow::
Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InTableView, TSharedPtr<FSlateIconBrowserRowDesc> InRow)
{
	RowDesc = InRow;
	SComboRow<TSharedPtr<FSlateIconBrowserRowDesc>>::Construct( SComboRow<TSharedPtr<FSlateIconBrowserRowDesc>>::FArguments(), InTableView);
	
	ChildSlot
	[
		SNew(SBox)
		.Visibility_Lambda([this](){ return RowDesc->GetVisibility(); })
		.Padding(1.f)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.LightGroupBorder"))
			.BorderBackgroundColor(this, &SSlateIconBrowserRow::GetHoverColor)
			.OnMouseDoubleClick(this, &SSlateIconBrowserRow::OnMouseDoubleClick)
			.OnMouseButtonUp_Raw(this, &SSlateIconBrowserRow::EntryContextMenu)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.VAlign(EVerticalAlignment::VAlign_Center)
				.Padding(FMargin(10, 5))
				[
					SNew(STextBlock)
					.Text(FText::FromName(InRow->PropertyName))
				]
				+SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(EVerticalAlignment::VAlign_Center)
				.Padding(FMargin(10, 5))
				[
					InRow->GenerateVisualizer()
				]
			]
		]
	];
}



FReply SSlateIconBrowserRow::
OnMouseDoubleClick(const FGeometry& Geometry, const FPointerEvent& PointerEvent)
{
	FString Code = RowDesc->GenerateCode(USlateIconBrowserUserSettings::Get()->CopyCodeFormat);
	ExecuteCopyCode(Code);
	return FReply::Handled();
}


FReply SSlateIconBrowserRow::
EntryContextMenu(const FGeometry& Geometry, const FPointerEvent& PointerEvent)
{
	if (PointerEvent.GetEffectingButton() != EKeys::RightMouseButton){
		return FReply::Unhandled();
	}
	if (PointerEvent.GetEventPath() == nullptr){
		return FReply::Unhandled();
	}
	

	FString CopyCode;
	FMenuBuilder MenuBuilder(true, nullptr);
	MenuBuilder.BeginSection("CopyOptions", LOCTEXT("CopyCodeOptions", "Copy Code"));
	{
		CopyCode = RowDesc->GenerateCode(CF_FSlateIcon);
		MenuBuilder.AddMenuEntry(FText::FromString(CopyCode), FText::GetEmpty(), FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&SSlateIconBrowserRow::ExecuteCopyCode, CopyCode)));
		
		CopyCode = RowDesc->GenerateCode(CF_FSlateIconFinderFindIcon);
		MenuBuilder.AddMenuEntry(FText::FromString(CopyCode), FText::GetEmpty(), FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&SSlateIconBrowserRow::ExecuteCopyCode, CopyCode)));
		
		if (!USlateIconBrowserUserSettings::Get()->FormatCustomExpression.IsEmpty())
		{
			CopyCode = RowDesc->GenerateCode(CF_Custom);
			MenuBuilder.AddMenuEntry(FText::FromString(CopyCode), FText::GetEmpty(), FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&SSlateIconBrowserRow::ExecuteCopyCode, CopyCode)));
		}
	}
	MenuBuilder.EndSection();

	
	TSharedPtr<SWidget> MenuWidget = MenuBuilder.MakeWidget();
	FWidgetPath WidgetPath = *PointerEvent.GetEventPath();
	const FVector2D& Location = PointerEvent.GetScreenSpacePosition();
	FSlateApplication::Get().PushMenu(WidgetPath.Widgets.Last().Widget, WidgetPath, MenuWidget.ToSharedRef(), Location, FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));

	
	return FReply::Handled();
}

void SSlateIconBrowserRow::
ExecuteCopyCode(FString InCode)
{
	FPlatformApplicationMisc::ClipboardCopy(*InCode);
	UE_LOG(LogTemp, Warning, TEXT("Copy code to clipboard: %s"), *InCode);

	FNotificationInfo Info(LOCTEXT("CopiedNotification", "C++ code copied to clipboard"));
	Info.ExpireDuration = 3.f;
#if ENGINE_MAJOR_VERSION == 5
	Info.SubText = FText::FromString(CopyText);
#else
	Info.Text = FText::FromString(InCode);
#endif
	FSlateNotificationManager::Get().AddNotification(Info);
}


void SSlateIconBrowserRow::
OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	SComboRow<TSharedPtr<FSlateIconBrowserRowDesc>>::OnMouseEnter(MyGeometry, MouseEvent);
	USlateIconBrowserUserSettings::GetMutable()->LastHoveredRow = SharedThis(this);
}

FSlateColor SSlateIconBrowserRow::
GetHoverColor() const
{
	constexpr  FLinearColor HoverColor(0.1,0.1,0.1);
	constexpr  FLinearColor NormalColor(0.02,0.02,0.02);
	
	TWeakPtr<SWidget> LastHoverRow = USlateIconBrowserUserSettings::Get()->LastHoveredRow;
	if (LastHoverRow.IsValid()){
		return LastHoverRow.Pin().Get() == this ? HoverColor : NormalColor; 
	}
	return NormalColor;
}


END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE