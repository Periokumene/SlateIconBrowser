#include "SSlateIconBrowserRow.h"

#include "SlateIconBrowserHacker.h"
#include "SlateIconBrowserUserSettings.h"
#include "SlateIconBrowserUtils.h"
#include "SlateOptMacros.h"
#include "SSlateIconBrowserTab.h"
#include "Styling/SlateStyleRegistry.h"

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
		RowListOut.Add(MakeShareable(new FSlateIconBrowserRowDesc_Font(StyleOwner, Key)));
	}
}


TSharedRef<SWidget> FSlateIconBrowserRowDesc_Font::
GenerateVisualizer() const
{
	if (const ISlateStyle* OwnerStyle = FSlateStyleRegistry::FindSlateStyle(OwnerStyleName)){
		if (PropertyName.ToString().Find(TEXT("FontAwesome")) != INDEX_NONE) // TODO FontAwesome Special
		{
			return SNew(STextBlock)
			// .Font(OwnerStyle->GetFontStyle(InDesc.PropertyName))
			.Text(FText::FromString(TEXT("FontAwesome WIP")))
			.ColorAndOpacity(FColor::Orange);
		}
		else
		{
			return SNew(SEditableText)
			.Font(OwnerStyle->GetFontStyle(PropertyName))
			.Text_Lambda([](){ return USlateIconBrowserUserSettings::Get()->FontPreviewText; })
			.OnTextChanged_Lambda([](const FText& InText)                     { USlateIconBrowserUserSettings::GetMutable()->FontPreviewText = InText; })
			.OnTextCommitted_Lambda([](const FText& InText, ETextCommit::Type){ USlateIconBrowserUserSettings::GetMutable()->FontPreviewText = InText; });
		}
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


void FSlateIconBrowserRowDesc_Widget::
CacheFromStyle(const FSlateStyleSet* StyleOwner, TArray<TSharedPtr<FSlateIconBrowserRowDesc>>& RowListOut)
{
	TMap<FName, TSharedRef<FSlateWidgetStyle>>* WidgetStyleMap = Hacker::Steal_WidgetStyleValues(StyleOwner);
	for (const auto& Pair : *WidgetStyleMap){
		RowListOut.Add(MakeShareable(new FSlateIconBrowserRowDesc_Widget(StyleOwner, Pair.Key, Pair.Value)));
	}
}

TSharedRef<SWidget> FSlateIconBrowserRowDesc_Widget::
GenerateVisualizer() const
{
	if (WidgetStyleTypeName == FButtonStyle::TypeName) { return GenerateVisualizer_Button(); }
	if (WidgetStyleTypeName == FCheckBoxStyle::TypeName)  { return GenerateVisualizer_CheckBox();  }
	if (WidgetStyleTypeName == FTextBlockStyle::TypeName) { return GenerateVisualizer_TextBlock(); }
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
    	return SNew(SHorizontalBox)
    	+SHorizontalBox::Slot()
    	[
    		SNew(STextBlock)
    		.Visibility_Static(&FSlateIconBrowserRowDesc_Widget::GetVisibility_Base)
    		.TextStyle(WS)
    	]
    	+SHorizontalBox::Slot()
    	[
    		SNew(SButton)
    		.Text_Static(&FSlateIconBrowserRowDesc_Widget::GetInsertText)
    		.Visibility_Static(&FSlateIconBrowserRowDesc_Widget::GetVisibility_Insert)
    		.TextStyle(WS)
    	];
    }
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
	FSlateIconBrowserUtils::CopyIconCodeToClipboard(RowDesc->PropertyName, USlateIconBrowserUserSettings::Get()->CopyCodeStyle);
	return FReply::Handled();
}


FReply SSlateIconBrowserRow::
EntryContextMenu(const FGeometry& Geometry, const FPointerEvent& PointerEvent)
{
	if (PointerEvent.GetEffectingButton() != EKeys::RightMouseButton)
		return FReply::Unhandled();

	if (PointerEvent.GetEventPath() == nullptr)
		return FReply::Unhandled();

	FString CopyCode;
	auto Clipboard = [&](FName N, ECopyCodeStyle CodeStyle)
	{
		FSlateIconBrowserUtils::CopyIconCodeToClipboard(N, CodeStyle);
	};

	
	FMenuBuilder MenuBuilder(true, nullptr);
	MenuBuilder.BeginSection("CopyOptions", LOCTEXT("CopyCodeOptions", "Copy Code"));
	{
		CopyCode = FSlateIconBrowserUtils::GenerateCopyCode(RowDesc->PropertyName, CS_FSlateIcon);
		MenuBuilder.AddMenuEntry(
			FText::FromString(CopyCode),
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda(Clipboard, RowDesc->PropertyName, CS_FSlateIcon)));
		CopyCode = FSlateIconBrowserUtils::GenerateCopyCode(RowDesc->PropertyName, CS_FSlateIconFinderFindIcon);
		MenuBuilder.AddMenuEntry(
			FText::FromString(CopyCode),
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda(Clipboard, RowDesc->PropertyName, CS_FSlateIconFinderFindIcon)));
		
		if (!USlateIconBrowserUserSettings::Get()->CustomFormat.IsEmpty()) {
			CopyCode = FSlateIconBrowserUtils::GenerateCopyCode(RowDesc->PropertyName, CS_CustomStyle);
			MenuBuilder.AddMenuEntry(
				FText::FromString(CopyCode),
				FText::GetEmpty(),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda(Clipboard, RowDesc->PropertyName, CS_CustomStyle)));
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