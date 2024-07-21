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
	
	virtual bool CustomHandleFilter(const FSlateIconBrowserFilterContext& Context) const = 0;
	virtual void CustomGenDetailFilter(TSharedPtr<class SSlateIconBrowserTab> TabOwner) {}
	virtual EVisibility GetVisibility() const;
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
	virtual EVisibility GetVisibility() const override;
	virtual void CustomGenDetailFilter(TSharedPtr<SSlateIconBrowserTab> TabOwner) override;

	// Yes template can be avoided here, but it will make redundant code everywhere......
	template<typename WidgetStyleType>
	const WidgetStyleType* GetLegalWidgetStyle() const; 


	// Widget Style Visualizer
	virtual TSharedRef<SWidget> GenerateVisualizer() const override;
	TSharedRef<SWidget> GenerateVisualizer_Failure(const FText& InComment = FText::GetEmpty()) const; // Unimplement or meaningless
	
	TSharedRef<SWidget> GenerateVisualizer_Button() const;
	TSharedRef<SWidget> GenerateVisualizer_CheckBox() const;

	TSharedRef<SWidget> GenerateVisualizer_TextBlock() const;
	TSharedRef<SWidget> GenerateVisualizer_EditableTextBox() const;
	TSharedRef<SWidget> GenerateVisualizer_EditableText() const;
	TSharedRef<SWidget> GenerateVisualizer_Hyperlink() const;
	TSharedRef<SWidget> GenerateVisualizer_InlineEditableTextBlock() const;
	
	TSharedRef<SWidget> GenerateVisualizer_ComboBox() const;
	TSharedRef<SWidget> GenerateVisualizer_ComboButton() const;

	TSharedRef<SWidget> GenerateVisualizer_ExpandableArea() const;
	
	TSharedRef<SWidget> GenerateVisualizer_ProgressBar() const;
	TSharedRef<SWidget> GenerateVisualizer_Slider() const;
	TSharedRef<SWidget> GenerateVisualizer_SpinBox() const;
	TSharedRef<SWidget> GenerateVisualizer_VolumeControl() const;

	TSharedRef<SWidget> GenerateVisualizer_SearchBox() const;
	TSharedRef<SWidget> GenerateVisualizer_ScrollBar() const;
	TSharedRef<SWidget> GenerateVisualizer_ScrollBox() const;
	TSharedRef<SWidget> GenerateVisualizer_Splitter() const;
	
	TSharedRef<SWidget> GenerateVisualizer_TableRow() const;
	
	TSharedRef<SWidget> GenerateVisualizer_WidgetCarouseNavigationBar() const;
	TSharedRef<SWidget> GenerateVisualizer_WidgetCarouseNavigationButton() const;

	TSharedRef<SWidget> GenerateVisualizer_ScrollBorder() const      { return GenerateVisualizer_Failure(); } // TODO ?
	TSharedRef<SWidget> GenerateVisualizer_Synth2DSlider() const     { return GenerateVisualizer_Failure(); } // Meaningless
	TSharedRef<SWidget> GenerateVisualizer_SynthKnob() const         { return GenerateVisualizer_Failure(); } // Meaningless
	TSharedRef<SWidget> GenerateVisualizer_DockTab() const           { return GenerateVisualizer_Failure(); } // Meaningless
	TSharedRef<SWidget> GenerateVisualizer_TableColumnHeader() const { return GenerateVisualizer_Failure(); } // Meaningless
	TSharedRef<SWidget> GenerateVisualizer_HeaderRow() const         { return GenerateVisualizer_Failure(); } // Meaningless
	TSharedRef<SWidget> GenerateVisualizer_Window() const            { return GenerateVisualizer_Failure(); } // Meaningless
	

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
