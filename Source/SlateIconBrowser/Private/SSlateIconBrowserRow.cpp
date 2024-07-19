#include "SSlateIconBrowserRow.h"

#include "SlateIconBrowserUserSettings.h"
#include "SlateIconBrowserUtils.h"
#include "SlateOptMacros.h"
#include "Styling/SlateStyleRegistry.h"

#define LOCTEXT_NAMESPACE "SlateIconBrowserRow"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION


TSharedRef<SWidget> FSlateIconBrowserRowDesc::
GenerateVisualizer() const
{
	TSharedRef<SWidget> Visualizer = SNullWidget::NullWidget;
	switch (Type){
		case ESlateIconBrowserRowType::Brush: { Visualizer = GenerateVisualizer_Brush(*this); break; }
		case ESlateIconBrowserRowType::Font:  { Visualizer = GenerateVisualizer_Font(*this);  break; }
		case ESlateIconBrowserRowType::UnImplement: { ensureMsgf(false, TEXT("Unexpected default RowType, you might forgot to define it somewhere")); break;}
		default: { ensureMsgf(false, TEXT("Row Type forget to implement the generation function")); }
	}
	return Visualizer;
}


TSharedRef<SWidget> FSlateIconBrowserRowDesc::
GenerateVisualizer_Brush(const FSlateIconBrowserRowDesc& InDesc)
{
	const FSlateBrush* Brush = FEditorStyle::GetBrush(TEXT("NoResourceWrongName"));
	if (const ISlateStyle* OwnerStyle = FSlateStyleRegistry::FindSlateStyle(InDesc.OwnerStyleName)){
		Brush = OwnerStyle->GetOptionalBrush(InDesc.PropertyName);
	}
	const FVector2D DesiredIconSize = Brush->GetImageType() == ESlateBrushImageType::NoImage ? FVector2D(20): Brush->GetImageSize();
	
	return SNew(SImage)
#if ENGINE_MAJOR_VERSION == 5
		.DesiredSizeOverride(DesiredIconSize)
#endif
		.Image(Brush);
}


TSharedRef<SWidget> FSlateIconBrowserRowDesc::
GenerateVisualizer_Font(const FSlateIconBrowserRowDesc& InDesc)
{
	if (const ISlateStyle* OwnerStyle = FSlateStyleRegistry::FindSlateStyle(InDesc.OwnerStyleName)){
		if (InDesc.PropertyName.ToString().Find(TEXT("FontAwesome")) != INDEX_NONE) // TODO FontAwesome Special
		{
			return SNew(STextBlock)
			// .Font(OwnerStyle->GetFontStyle(InDesc.PropertyName))
			.Text(FText::FromString(TEXT("FontAwesome WIP")))
			.ColorAndOpacity(FColor::Orange);
		}
		else
		{
			return SNew(SEditableText)
			.Font(OwnerStyle->GetFontStyle(InDesc.PropertyName))
			.Text_Lambda([](){ return USlateIconBrowserUserSettings::Get()->FontPreviewText; })
			.OnTextChanged_Lambda([](const FText& InText)                     { USlateIconBrowserUserSettings::GetMutable()->FontPreviewText = InText; })
			.OnTextCommitted_Lambda([](const FText& InText, ETextCommit::Type){ USlateIconBrowserUserSettings::GetMutable()->FontPreviewText = InText; });
		}
	}
	
	return SNew(STextBlock)
		.Text(FText::FromString(TEXT("ERROR")))
		.ColorAndOpacity(FLinearColor::Red);
}


void SSlateIconBrowserRow::
Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InTableView, TSharedPtr<FSlateIconBrowserRowDesc> InRow)
{
	RowDesc = *InRow;
	SComboRow<TSharedPtr<FSlateIconBrowserRowDesc>>::Construct( SComboRow<TSharedPtr<FSlateIconBrowserRowDesc>>::FArguments(), InTableView);

	ChildSlot
	[
		SNew(SBorder)
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
	];
}


TSharedRef<ITableRow> SSlateIconBrowserRow::
GenerateRow(TSharedPtr<FSlateIconBrowserRowDesc> RowDesc, const TSharedRef<STableViewBase>& TableViewBase)
{
	return SNew(SSlateIconBrowserRow, TableViewBase, RowDesc);
}


FReply SSlateIconBrowserRow::
OnMouseDoubleClick(const FGeometry& Geometry, const FPointerEvent& PointerEvent)
{
	FSlateIconBrowserUtils::CopyIconCodeToClipboard(RowDesc.PropertyName, USlateIconBrowserUserSettings::Get()->CopyCodeStyle);
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
		CopyCode = FSlateIconBrowserUtils::GenerateCopyCode(RowDesc.PropertyName, CS_FSlateIcon);
		MenuBuilder.AddMenuEntry(
			FText::FromString(CopyCode),
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda(Clipboard, RowDesc.PropertyName, CS_FSlateIcon)));
		CopyCode = FSlateIconBrowserUtils::GenerateCopyCode(RowDesc.PropertyName, CS_FSlateIconFinderFindIcon);
		MenuBuilder.AddMenuEntry(
			FText::FromString(CopyCode),
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda(Clipboard, RowDesc.PropertyName, CS_FSlateIconFinderFindIcon)));
		
		if (!USlateIconBrowserUserSettings::Get()->CustomStyle.IsEmpty()) {
			CopyCode = FSlateIconBrowserUtils::GenerateCopyCode(RowDesc.PropertyName, CS_CustomStyle);
			MenuBuilder.AddMenuEntry(
				FText::FromString(CopyCode),
				FText::GetEmpty(),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda(Clipboard, RowDesc.PropertyName, CS_CustomStyle)));
		}
	}
	MenuBuilder.EndSection();

	TSharedPtr<SWidget> MenuWidget = MenuBuilder.MakeWidget();
	FWidgetPath WidgetPath = *PointerEvent.GetEventPath();
	const FVector2D& Location = PointerEvent.GetScreenSpacePosition();
	FSlateApplication::Get().PushMenu(WidgetPath.Widgets.Last().Widget, WidgetPath, MenuWidget.ToSharedRef(), Location, FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));
			
	return FReply::Handled();
}


END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE