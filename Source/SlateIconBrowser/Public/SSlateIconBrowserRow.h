#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"


enum class ESlateIconBrowserRowType
{
	UnImplement,
	Brush,
	Widget,
	Font
};


struct FSlateIconBrowserRowDesc
{
	FSlateIconBrowserRowDesc(){}
	FSlateIconBrowserRowDesc(const FName& InStyleOwnerName, const FName& InPropertyName, ESlateIconBrowserRowType InType)
		: OwnerStyleName(InStyleOwnerName)
		, PropertyName(InPropertyName)
		, Type(InType)
	{}
	
	FName OwnerStyleName;
	FName PropertyName;
	ESlateIconBrowserRowType Type = ESlateIconBrowserRowType::UnImplement;

	TSharedRef<SWidget> GenerateVisualizer() const;
	static TSharedRef<SWidget> GenerateVisualizer_Brush(const FSlateIconBrowserRowDesc&);
	static TSharedRef<SWidget> GenerateVisualizer_Font(const FSlateIconBrowserRowDesc&);

};


class SSlateIconBrowserRow : public SComboRow<TSharedPtr<FSlateIconBrowserRowDesc>>
{
public:
	SLATE_BEGIN_ARGS(SSlateIconBrowserRow){}
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InTableView, TSharedPtr<FSlateIconBrowserRowDesc> InDesc);
	static TSharedRef<ITableRow> GenerateRow(TSharedPtr<FSlateIconBrowserRowDesc> RowDesc, const TSharedRef<STableViewBase>& TableViewBase);

	FReply OnMouseDoubleClick(const FGeometry& Geometry, const FPointerEvent& PointerEvent);
	FReply EntryContextMenu(const FGeometry& Geometry, const FPointerEvent& PointerEvent);

	
private:
	FSlateIconBrowserRowDesc RowDesc;
};
