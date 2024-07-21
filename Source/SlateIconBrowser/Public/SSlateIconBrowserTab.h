#pragma once
#include "SlateIconBrowser.h"
#include "SlateIconBrowserUserSettings.h"


struct FSlateIconBrowserRowDesc;

class SSlateIconBrowserTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SSlateIconBrowserTab){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	
	static void Register();
	static TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

	
	// Top Bar
	TSharedRef<SWidget> MakeHead();
	static void FillHelpMenu(FMenuBuilder& MenuBuilder);
	static void FillSettingsMenu(FMenuBuilder& MenuBuilder);
	

	// Search Helper
	TSharedRef<SWidget> MakeFilter();
	void OnStyleSelectionChange(TSharedPtr<FName> InItem, ESelectInfo::Type);
	void OnFilterStringChange(const FText& Text);
	void OnCustomFormatChange(const FText& Text, ETextCommit::Type);
	
	TSharedRef<SWidget> OnGenerateStyleSelection(TSharedPtr<FName> InItem);
	FText GetCustomFormat() const;
	
	
	// Body
	TSharedRef<SWidget> MakeBody();
	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FSlateIconBrowserRowDesc> RowDesc, const TSharedRef<STableViewBase>& TableViewBase) const;
	void CacheAllStyleNames();
	void CacheRowDescs();
	void CacheAllLines();

	// Filter!!
	FSlateIconBrowserFilterContext FilterCollectContext() const;
	void FilterRefresh(const FSlateIconBrowserFilterContext& Context);

	
	// Bottom
	TSharedRef<SWidget> MakeBottom();
	
	TSharedRef<SWidget> MakeDetailControl_WidgetStyle();
	TArray<TSharedPtr<FName>>& GetWidgetStyleList(); //Expose For Widget
	TSharedPtr<SComboBox<TSharedPtr<FName>>> WidgetStyleComboBox;
	TArray<TSharedPtr<FName>> WidgetStyleList;

	
private:
	TArray<TSharedPtr<FSlateIconBrowserRowDesc>> Rows;
	TArray<TSharedPtr<FSlateIconBrowserRowDesc>> AllRows;
	TArray<TSharedPtr<FName>> AllStyles;
	static FName Key_StyleFilterNone;
	
	TSharedPtr<STextBlock> CopyNoteTextBlock;
	TSharedPtr<SListView<TSharedPtr<FSlateIconBrowserRowDesc>>> ListView;
	TSharedPtr<SComboBox<TSharedPtr<FName>>> StyleSelectionComboBox;
};
