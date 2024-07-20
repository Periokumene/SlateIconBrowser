#pragma once

#include "CoreMinimal.h"
#include "SlateIconBrowser.h"
#include "SlateIconBrowserUserSettings.h"


class FSlateStyleSet;

struct FSlateIconBrowserRowDesc
{
	FSlateIconBrowserRowDesc(){}
	FSlateIconBrowserRowDesc(const FName& InStyleOwnerName, const FName& InPropertyName)
		: OwnerStyleName(InStyleOwnerName)
		, PropertyName(InPropertyName)
	{}
	virtual ~FSlateIconBrowserRowDesc() {}
	
	FName OwnerStyleName;
	FName PropertyName;
	virtual TSharedRef<SWidget> GenerateVisualizer() const = 0;
	bool HandleFilter(const FSlateIconBrowserFilterContext& Context) const;
	
protected:
	virtual bool CustomHandleFilter(const FSlateIconBrowserFilterContext& Context) const = 0;
};



struct FSlateIconBrowserRowDesc_Brush : FSlateIconBrowserRowDesc
{
	FSlateIconBrowserRowDesc_Brush(const FName& InStyleOwnerName, const FName& InPropertyName) : FSlateIconBrowserRowDesc(InStyleOwnerName, InPropertyName) {}
	static void CacheFromStyle(const FSlateStyleSet* StyleOwner, TArray<TSharedPtr<FSlateIconBrowserRowDesc>>& RowListOut);
	virtual TSharedRef<SWidget> GenerateVisualizer() const override;
	virtual bool CustomHandleFilter(const FSlateIconBrowserFilterContext& Context) const override;
};


struct FSlateIconBrowserRowDesc_Font : FSlateIconBrowserRowDesc
{
	FSlateIconBrowserRowDesc_Font(const FName& InStyleOwnerName, const FName& InPropertyName) : FSlateIconBrowserRowDesc(InStyleOwnerName, InPropertyName) {}
	static void CacheFromStyle(const FSlateStyleSet* StyleOwner, TArray<TSharedPtr<FSlateIconBrowserRowDesc>>& RowListOut);
	virtual TSharedRef<SWidget> GenerateVisualizer() const override;
	virtual bool CustomHandleFilter(const FSlateIconBrowserFilterContext& Context) const override;
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
	TSharedPtr<FSlateIconBrowserRowDesc> RowDesc;
};
