#include "SSlateIconBrowserRow.h"

#include "SlateIconBrowserUserSettings.h"
#include "SlateIconBrowserUtils.h"
#include "SlateOptMacros.h"
#include "Styling/SlateStyleRegistry.h"

#define LOCTEXT_NAMESPACE "SlateIconBrowserRow"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION


void SSlateIconBrowserRow::
Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InTableView, TSharedPtr<FSlateIconBrowserRowDesc> InRow)
{
	RowDesc = *InRow;
	SComboRow<TSharedPtr<FSlateIconBrowserRowDesc>>::Construct( SComboRow<TSharedPtr<FSlateIconBrowserRowDesc>>::FArguments(), InTableView);

	
	const FSlateBrush* Brush = FEditorStyle::GetBrush(TEXT("NoResourceWrongName"));
	if (const ISlateStyle* OwnerStyle = FSlateStyleRegistry::FindSlateStyle(InRow->OwnerStyleName))
	{
		Brush = OwnerStyle->GetOptionalBrush(InRow->PropertyName);
	}
	FVector2D DesiredIconSize = Brush->GetImageType() == ESlateBrushImageType::NoImage ? FVector2D(20): Brush->GetImageSize();

	
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
				SNew(SImage)
#if ENGINE_MAJOR_VERSION == 5
				.DesiredSizeOverride(DesiredIconSize)
#endif
				.Image(Brush)
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