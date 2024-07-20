#pragma once

#include "CoreMinimal.h"
#include "SlateIconBrowser.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"


class FSlateStyleSet;

struct FSlateIconBrowserRowDesc
{
	FSlateIconBrowserRowDesc(){}
	FSlateIconBrowserRowDesc(const ISlateStyle* InStyleOwner, const FName& InPropertyName)
		: OwnerStyleName(InStyleOwner->GetStyleSetName())
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


//======================================================================================================================
struct FSlateIconBrowserRowDesc_Brush : FSlateIconBrowserRowDesc
{
	FSlateIconBrowserRowDesc_Brush(const ISlateStyle* InStyleOwner, const FName& InPropertyName) : FSlateIconBrowserRowDesc(InStyleOwner, InPropertyName) {}
	static void CacheFromStyle(const FSlateStyleSet* StyleOwner, TArray<TSharedPtr<FSlateIconBrowserRowDesc>>& RowListOut);
	virtual TSharedRef<SWidget> GenerateVisualizer() const override;
	virtual bool CustomHandleFilter(const FSlateIconBrowserFilterContext& Context) const override;
};


//======================================================================================================================
struct FSlateIconBrowserRowDesc_Font : FSlateIconBrowserRowDesc
{
	FSlateIconBrowserRowDesc_Font(const ISlateStyle* InStyleOwner, const FName& InPropertyName) : FSlateIconBrowserRowDesc(InStyleOwner, InPropertyName) {}
	static void CacheFromStyle(const FSlateStyleSet* StyleOwner, TArray<TSharedPtr<FSlateIconBrowserRowDesc>>& RowListOut);
	virtual TSharedRef<SWidget> GenerateVisualizer() const override;
	virtual bool CustomHandleFilter(const FSlateIconBrowserFilterContext& Context) const override;
};


//======================================================================================================================
struct FSlateIconBrowserRowDesc_Widget : FSlateIconBrowserRowDesc
{
	FSlateIconBrowserRowDesc_Widget(const ISlateStyle* InStyleOwner, const FName& InPropertyName, const TSharedRef<FSlateWidgetStyle> WidgetStyle)
		: FSlateIconBrowserRowDesc(InStyleOwner, InPropertyName), WidgetStyleTypeName(WidgetStyle->GetTypeName()) {}
	static void CacheFromStyle(const FSlateStyleSet* StyleOwner, TArray<TSharedPtr<FSlateIconBrowserRowDesc>>& RowListOut);
	
private:
	virtual bool CustomHandleFilter(const FSlateIconBrowserFilterContext& Context) const override;

	// Yes template can be avoided here, but it will make redundant code everywhere......
	template<typename WidgetStyleType>
	const WidgetStyleType* GetLegalWidgetStyle() const; 
	
	virtual TSharedRef<SWidget> GenerateVisualizer() const override;
	TSharedRef<SWidget> GenerateVisualizer_Button() const;
	TSharedRef<SWidget> GenerateVisualizer_CheckBox() const;
	TSharedRef<SWidget> GenerateVisualizer_TextBlock() const;
	TSharedRef<SWidget> GenerateVisualizer_Failure(const FText& InComment = FText::GetEmpty()) const;

	static EVisibility GetVisibility_Base();
	static EVisibility GetVisibility_Insert();
	static FText GetInsertText();
	
	FName WidgetStyleTypeName;
};


template <typename WidgetStyleType>
const WidgetStyleType* FSlateIconBrowserRowDesc_Widget::
GetLegalWidgetStyle() const
{
	if (const ISlateStyle* OwnerStyle = FSlateStyleRegistry::FindSlateStyle(OwnerStyleName))
	{
		const WidgetStyleType& WS = OwnerStyle->GetWidgetStyle<WidgetStyleType>(PropertyName);
		if (&WS != &WidgetStyleType::GetDefault()){
			return &WS;
		}
	}
	return nullptr;
}


//======================================================================================================================
// Container to hold the RowDesc, All Detail should be handled by RowDesc virtual function.
// This widget is only used to route interaction event to RowDesc.
//======================================================================================================================
class SSlateIconBrowserRow : public SComboRow<TSharedPtr<FSlateIconBrowserRowDesc>>
{
public:
	SLATE_BEGIN_ARGS(SSlateIconBrowserRow){}
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InTableView, TSharedPtr<FSlateIconBrowserRowDesc> InDesc);

	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	FSlateColor GetHoverColor() const;
	FReply OnMouseDoubleClick(const FGeometry& Geometry, const FPointerEvent& PointerEvent);
	FReply EntryContextMenu(const FGeometry& Geometry, const FPointerEvent& PointerEvent);

private:
	TSharedPtr<FSlateIconBrowserRowDesc> RowDesc;
};
