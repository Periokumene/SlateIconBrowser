#include "SSlateIconBrowserTab.h"

#include "SEnumCombobox.h"
#include "SlateIconBrowserStyle.h"
#include "SlateIconBrowserUserSettings.h"
#include "SlateIconBrowserUtils.h"
#include "SSlateIconBrowserRow.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Misc/ScopedSlowTask.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SSeparator.h"

#define LOCTEXT_NAMESPACE "SSlateIconBrowserTab"

FName SSlateIconBrowserTab::Key_StyleFilterNone = "Show All";

void SSlateIconBrowserTab::
Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		// Put your tab content here!
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			MakeHead()
		]
		
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			MakeFilter()
		]
		
		+SVerticalBox::Slot()
		.FillHeight(1.0)
		[
			MakeBody()
		]
		
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(0,4,0,0))
		[
			MakeBottom()
		]
	];
}


TSharedRef<SDockTab> SSlateIconBrowserTab::
OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	USlateIconBrowserUserSettings::GetMutable()->ValidateConfig();
	USlateIconBrowserUserSettings::GetMutable()->InitTranslator();
	
	TSharedRef<SSlateIconBrowserTab> NewTab = SNew(SSlateIconBrowserTab);
	NewTab->CacheAllStyleNames();
	NewTab->CacheAllLines();
	
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.OnTabClosed_Lambda([](TSharedRef<SDockTab>){
			GetMutableDefault<USlateIconBrowserUserSettings>()->FilterString = TEXT("");
		})
		[
			NewTab
		];
}



void SSlateIconBrowserTab::
CacheAllStyleNames()
{
	AllStyles.Empty(AllStyles.Num());
	
	FSlateStyleRegistry::IterateAllStyles(
		[&](const ISlateStyle& Style)
		{
			AllStyles.Add(MakeShareable(new FName(Style.GetStyleSetName())));
			return true;
		}
	);

	AllStyles.Sort([](const TSharedPtr<FName>& A, const TSharedPtr<FName>& B)
	{
		const FString AString = A->ToString();
		const FString BString = B->ToString();
		return AString.Compare(BString) < 0;
	});

	AllStyles.Insert(MakeShared<FName>(Key_StyleFilterNone), 0);
}

void SSlateIconBrowserTab::
CacheRowDescs()
{
	AllRows.Reset();

	TArray<const FSlateStyleSet*> StylesToScan;

	TArray<TSharedPtr<FName>> PendingScanStyleNames = { MakeShared<FName>(USlateIconBrowserUserSettings::Get()->SelectedStyle) };
	if (USlateIconBrowserUserSettings::Get()->SelectedStyle == Key_StyleFilterNone){
		PendingScanStyleNames = AllStyles;
	}

	FScopedSlowTask Task(PendingScanStyleNames.Num(), LOCTEXT("SlowTaskCacheRows", "Caching Style Rows"));
	Task.MakeDialog();
	for (const TSharedPtr<FName> StyleName : PendingScanStyleNames)
	{
		Task.EnterProgressFrame();
		if (*StyleName == Key_StyleFilterNone)
		{
			continue;
		}
		const ISlateStyle* Style = FSlateStyleRegistry::FindSlateStyle(*StyleName);
		const FSlateStyleSet* StyleSet = static_cast<const FSlateStyleSet*>(Style);
		if (!StyleSet){
			ensureMsgf(false, TEXT("Unexpected nullptr"));
			continue;
		}
		FSlateIconBrowserRowDesc_Brush::CacheFromStyle(StyleSet, AllRows);
		FSlateIconBrowserRowDesc_Font::CacheFromStyle(StyleSet, AllRows);
		FSlateIconBrowserRowDesc_Widget::CacheFromStyle(StyleSet, AllRows);
	}
}


void SSlateIconBrowserTab::
CacheAllLines()
{
	CacheRowDescs();
	FSlateIconBrowserFilterContext Context = FilterCollectContext();
	FilterRefresh(Context);
}

FSlateIconBrowserFilterContext SSlateIconBrowserTab::
FilterCollectContext() const
{
	FSlateIconBrowserFilterContext FilterContext;
	FilterContext.FilterString = USlateIconBrowserUserSettings::Get()->FilterString;
	FilterContext.RowType      = USlateIconBrowserUserSettings::Get()->RowType;
	return FilterContext;
}


void SSlateIconBrowserTab::
FilterRefresh(const FSlateIconBrowserFilterContext& Context)
{
	Rows.Empty(AllRows.Num());
	WidgetStyleList.Reset();
	
	for (const auto& LineDesc : AllRows){
		if (bool bPass = LineDesc->HandleFilter(Context))
		{
			LineDesc->CustomGenDetailFilter(SharedThis(this));
			Rows.Add(LineDesc);
		}
	}

	Rows.Sort([](const TSharedPtr<FSlateIconBrowserRowDesc>& A, const TSharedPtr<FSlateIconBrowserRowDesc>& B){
		return A->PropertyName.Compare(B->PropertyName) < 0;
	});
	
	if (ListView.IsValid()){
		ListView.Get()->RequestListRefresh();
		ListView.Get()->ScrollToTop();
	}
	if (WidgetStyleComboBox.IsValid()){
		WidgetStyleList.Sort([](const TSharedPtr<FName>& A, const TSharedPtr<FName>& B) { return A->Compare(*B) < 0; });
		WidgetStyleComboBox->RefreshOptions();
	}
}

TSharedRef<ITableRow> SSlateIconBrowserTab::
OnGenerateRow(TSharedPtr<FSlateIconBrowserRowDesc> RowDesc, const TSharedRef<STableViewBase>& TableViewBase) const
{
	return SNew(SSlateIconBrowserRow, TableViewBase, RowDesc);
}


void SSlateIconBrowserTab::
OnFilterStringChange(const FText& Text)
{
	USlateIconBrowserUserSettings::GetMutable()->FilterString = Text.ToString();
	USlateIconBrowserUserSettings::GetMutable()->SaveConfig();

	FSlateIconBrowserFilterContext Context = FilterCollectContext();
	FilterRefresh(Context);
}


TSharedRef<SWidget> SSlateIconBrowserTab::
MakeBottom()
{
	return SNew(SBorder)
#if ENGINE_MAJOR_VERSION == 5
	.BorderImage(EDITOR_STYLE_SAFE()::Get().GetBrush("Brushes.Panel"))
#else
	.BorderImage(EDITOR_STYLE_SAFE()::Get().GetBrush("ToolPanel.GroupBorder"))
#endif
	.Padding(FMargin(10, 5))
	[
		// Left Hints
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		[
			SAssignNew(CopyNoteTextBlock, STextBlock)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
			.Text_Lambda([this]{ return GetCodeStyleText(USlateIconBrowserUserSettings::Get()->CopyCodeStyle); })
		]

		// Right Controller, including RowType Filter and RowType-based Detailed Control
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			[
				MakeDetailControl_WidgetStyle()
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(2, 0))
			[
				SNew(SSeparator)
				.Orientation(Orient_Vertical)
			]

			// RowType Switcher
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SEnumComboBox, StaticEnum<ESlateIconBrowserRowFilterType>())
				.CurrentValue_Lambda([](){ return static_cast<int>(USlateIconBrowserUserSettings::Get()->RowType); })
				.OnEnumSelectionChanged_Lambda([this](int32 Value, ESelectInfo::Type)
				{
					const ESlateIconBrowserRowFilterType Type = static_cast<ESlateIconBrowserRowFilterType>(Value);
					USlateIconBrowserUserSettings::GetMutable()->RowType = Type;
					CacheAllLines();
				})
			]
		]
	];
}


TSharedRef<SWidget> SSlateIconBrowserTab::
MakeDetailControl_WidgetStyle()
{
	return SNew(SHorizontalBox)
		.Visibility_Lambda([]()
		{
			return USlateIconBrowserUserSettings::Get()->RowType == ESlateIconBrowserRowFilterType::Widget ?
				EVisibility::Visible:
				EVisibility::Collapsed;
		})
		
		// Widget Filter
		+SHorizontalBox::Slot()
		.AutoWidth()
		[
			SAssignNew(WidgetStyleComboBox, SComboBox<TSharedPtr<FName>>)
			.OnSelectionChanged_Lambda([this](TSharedPtr<FName> InItem, ESelectInfo::Type){
				if (InItem.IsValid()){
					USlateIconBrowserUserSettings::GetMutable()->HighlightWidgetStyleName = *InItem;
					ListView.Get()->ScrollToTop();
				}
			})
			.OptionsSource(&WidgetStyleList)
			.OnGenerateWidget(this, &SSlateIconBrowserTab::OnGenerateStyleSelection)
			[
				SNew(STextBlock)
				.ColorAndOpacity_Lambda([this]()
				{
					FLinearColor Color = FLinearColor::Black;
					Color = USlateIconBrowserUserSettings::Get()->HighlightWidgetStyleName == "None" ? FColor::Orange : Color;
					Color = Rows.Num() == 0 ? FLinearColor::Red : Color;
					return FSlateColor(Color);
				})
				.Text_Lambda([this]{
					FText Display = FText::FromName(USlateIconBrowserUserSettings::Get()->HighlightWidgetStyleName);
					if (USlateIconBrowserUserSettings::Get()->HighlightWidgetStyleName == "None"){
						Display = LOCTEXT("HighligtWS Hint", "Select");
					}
					if (Rows.Num() == 0){
						Display = LOCTEXT("NoRow", "Empty");
					}
					return Display;
				})
			]
		]
		
		//Optional Insert Text to widget
		+SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(FMargin(2,0,0,0))
		[
			SNew(SCheckBox)
			.IsChecked_Lambda([]()
			{
				return USlateIconBrowserUserSettings::Get()->bWidgetInsertText?
					ECheckBoxState::Checked:
					ECheckBoxState::Unchecked;
			})
			.OnCheckStateChanged_Lambda([](ECheckBoxState NewState)
			{
				USlateIconBrowserUserSettings::GetMutable()->bWidgetInsertText = NewState == ECheckBoxState::Checked;
			})
			[
				SNew(STextBlock).Text(LOCTEXT("InsertText", "Insert Text"))
			]
		];
}



TArray<TSharedPtr<FName>>& SSlateIconBrowserTab::
GetWidgetStyleList()
{
	return WidgetStyleList;
}

FText SSlateIconBrowserTab::
GetCodeStyleText(ECopyCodeStyle CopyStyle)
{
	switch (CopyStyle) {
	case CS_FSlateIcon:
		return FText::FromString(TEXT("FSlateIcon(...)"));
		break;
	case CS_FSlateIconFinderFindIcon:
		return FText::FromString(TEXT("FSlateIconFinder::FindIcon(...)"));
		break;
	case CS_CustomStyle:
		return FText::FromString(USlateIconBrowserUserSettings::Get()->CustomFormat.IsEmpty() ?
			TEXT("(Custom)") :
			USlateIconBrowserUserSettings::Get()->CustomFormat);
		break;
	}
	return FText::GetEmpty();
}

TSharedRef<SWidget> SSlateIconBrowserTab::
MakeHead()
{
	FMenuBarBuilder MenuBuilder(nullptr);
	{
		MenuBuilder.AddPullDownMenu(
			LOCTEXT("SettingsMenu", "Settings"),
			LOCTEXT("SettingsMenuTooltip", "Opens the settings menu."),
			FNewMenuDelegate::CreateStatic(&SSlateIconBrowserTab::FillSettingsMenu));
		MenuBuilder.AddPullDownMenu(
			LOCTEXT("HelpMenu", "Help"),
			LOCTEXT("HelpMenuTooltip", "Opens the help menu."),
			FNewMenuDelegate::CreateStatic(&SSlateIconBrowserTab::FillHelpMenu));
	}
	TSharedRef<SWidget> Widget = MenuBuilder.MakeWidget();
	Widget->SetVisibility(EVisibility::Visible);
	return Widget;
}


void SSlateIconBrowserTab::
FillHelpMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection(TEXT("HelpMenu"));
	{
		MenuBuilder.AddMenuEntry(
		LOCTEXT("HelpDocumentation", "Documentation"),
		LOCTEXT("HelpDocumentationTooltip", "Opens the documentation"),
#if ENGINE_MAJOR_VERSION == 5
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Documentation"),
#else
		FSlateIcon(FEditorStyle::GetStyleSetName(), "Icons.Documentation"),
#endif
		
		FUIAction(
			FExecuteAction::CreateLambda([=]
			{
				FPlatformProcess::LaunchURL(TEXT("https://github.com/sirjofri/SlateIconBrowser"), nullptr, nullptr);
			})
		));
	}
	MenuBuilder.EndSection();
}


void SSlateIconBrowserTab::
FillSettingsMenu(FMenuBuilder& MenuBuilder)
{
	TArray<ECopyCodeStyle> Formats = {
		CS_FSlateIcon,
		CS_FSlateIconFinderFindIcon,
		CS_CustomStyle
	};
	auto ExecuteLambda = [](ECopyCodeStyle InFormat)
	{
		USlateIconBrowserUserSettings::GetMutable()->CopyCodeStyle = InFormat;
		USlateIconBrowserUserSettings::GetMutable()->SaveConfig();
	};
	auto IsCheckLambda = [](ECopyCodeStyle InFormat)
	{
		return USlateIconBrowserUserSettings::Get()->CopyCodeStyle == InFormat ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	};

	
	MenuBuilder.BeginSection(TEXT("CopySettings"), LOCTEXT("CopySettings", "Code to copy"));
	{
		for (const auto& Format : Formats)
		{
			FUIAction Action;
			Action.ExecuteAction.BindLambda(ExecuteLambda, Format);
			Action.GetActionCheckState.BindLambda(IsCheckLambda, Format);
			
			MenuBuilder.AddMenuEntry(GetCodeStyleText(Format), GetCodeStyleTooltip(Format), FSlateIcon(),
				Action, NAME_None, EUserInterfaceActionType::RadioButton);
		}
	}
	MenuBuilder.EndSection();
}


TSharedRef<SWidget> SSlateIconBrowserTab::
MakeFilter()
{
	return SNew(SVerticalBox)
	+SVerticalBox::Slot()
	.AutoHeight()
	[
		SNew(SExpandableArea)
		.InitiallyCollapsed(true)
		.HeaderContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("CustomStyleExpandHeader", "Custom Style"))
		]
		.BodyContent()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("CustomStyleTextFieldLabel", "Custom Code Style:"))
				.ToolTipText(NSLOCTEXT("FSlateIconBrowserModule", "CustomStyleTooltipText", "Enter custom style. $1 will be replaced by the icon name."))
				.Margin(FMargin(10, 5))
			]
			+SHorizontalBox::Slot()
			.FillWidth(1.f)
			[
				SNew(SEditableTextBox)
				.HintText(LOCTEXT("CustomStyleTextFieldHint", "$1 will be replaced by the icon name"))
				.ToolTipText(NSLOCTEXT("FSlateIconBrowserModule", "CustomStyleTooltipText", "Enter custom style. $1 will be replaced by the icon name."))
				.Text(this, &SSlateIconBrowserTab::GetCustomFormat)
				.OnTextCommitted(this, &SSlateIconBrowserTab::OnCustomFormatChange)
			]
		]
	]
	+SVerticalBox::Slot()
	.AutoHeight()
	[
		SNew(SSeparator)
		.Orientation(EOrientation::Orient_Horizontal)
	]
	+SVerticalBox::Slot()
	.AutoHeight()
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.AutoWidth()
		[
			SAssignNew(StyleSelectionComboBox, SComboBox<TSharedPtr<FName>>)
			.OnSelectionChanged(this, &SSlateIconBrowserTab::OnStyleSelectionChange)
			.OptionsSource(&AllStyles)
			.OnGenerateWidget(this, &SSlateIconBrowserTab::OnGenerateStyleSelection)
			[
				SNew(STextBlock)
				.Text_Lambda([this]{ return FText::FromName(USlateIconBrowserUserSettings::Get()->SelectedStyle); })
			]
		]
		+SHorizontalBox::Slot()
		.FillWidth(1.0)
		[
			SNew(SEditableTextBox)
			.HintText(LOCTEXT("SearchHintText", "Search"))
			.OnTextChanged(FOnTextChanged::CreateRaw(this, &SSlateIconBrowserTab::OnFilterStringChange))
			.Text_Lambda([&]{ return FText::FromString(USlateIconBrowserUserSettings::Get()->FilterString); })
		]
	];
}


void SSlateIconBrowserTab::
OnStyleSelectionChange(TSharedPtr<FName> InItem, ESelectInfo::Type)
{
	USlateIconBrowserUserSettings::GetMutable()->SelectedStyle = *InItem;
	USlateIconBrowserUserSettings::GetMutable()->SaveConfig();
	CacheAllLines();
}

TSharedRef<SWidget> SSlateIconBrowserTab::
OnGenerateStyleSelection(TSharedPtr<FName> InItem)
{
	return SNew(STextBlock).Text(FText::FromName(*InItem.Get()));
}


FText SSlateIconBrowserTab::
GetCustomFormat() const
{
	return FText::FromString(USlateIconBrowserUserSettings::Get()->CustomFormat);
}

void SSlateIconBrowserTab::
OnCustomFormatChange(const FText& Text, ETextCommit::Type)
{
	USlateIconBrowserUserSettings::GetMutable()->CustomFormat = Text.ToString();
	USlateIconBrowserUserSettings::GetMutable()->SaveConfig();
	CopyNoteTextBlock->Invalidate(EInvalidateWidgetReason::Paint); // What's Invalidate
}


TSharedRef<SWidget> SSlateIconBrowserTab::
MakeBody()
{
	return SNew(SVerticalBox)
	+SVerticalBox::Slot()
	.FillHeight(1.0)
	[
		SAssignNew(ListView, SListView<TSharedPtr<FSlateIconBrowserRowDesc>>)
		.OnGenerateRow(this, &SSlateIconBrowserTab::OnGenerateRow)
		.ListItemsSource(&Rows)
		.SelectionMode(ESelectionMode::None)
		.Visibility_Lambda([this]{ return Rows.Num() == 0 ? EVisibility::Collapsed : EVisibility::Visible; })
	]
	+SVerticalBox::Slot()
	.VAlign(EVerticalAlignment::VAlign_Center)
	[
		SNew(SBox)
		.HAlign(EHorizontalAlignment::HAlign_Center)
		.Visibility_Lambda([this]{ return Rows.Num() == 0 ? EVisibility::Visible : EVisibility::Collapsed; })
		[
			SNew(STextBlock)
			.Text(LOCTEXT("NoResults", "No results found."))
		]
	];
}


FText SSlateIconBrowserTab::
GetCodeStyleTooltip(ECopyCodeStyle CopyStyle)
{
	return FText::Format(LOCTEXT("CopyStyleTooltip", "Copy code in {0} style"), GetCodeStyleText(CopyStyle));
}


void SSlateIconBrowserTab::
Register()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner("SlateIconBrowser", FOnSpawnTab::CreateStatic(&SSlateIconBrowserTab::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FSlateIconBrowserTabTitle", "Slate Icon Browser"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory())
		.SetIcon(FSlateIcon(FSlateIconBrowserStyle::GetStyleSetName(), "SlateIconBrowser.Icon"))
		.SetMenuType(ETabSpawnerMenuType::Enabled);
}


#undef LOCTEXT_NAMESPACE