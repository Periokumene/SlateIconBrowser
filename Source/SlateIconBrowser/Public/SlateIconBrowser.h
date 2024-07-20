// Copyright 2022 sirjofri. Licensed under MIT license. See License.txt for full license text.

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "SlateIconBrowserUserSettings.h"
#include "Modules/ModuleManager.h"

struct FSlateIconBrowserRowDesc;
class FToolBarBuilder;
class FMenuBuilder;
class ITableRow;
class STableViewBase;
class STextBlock;
template<typename T> class SListView;
template<typename T> class SComboBox;


#if ENGINE_MAJOR_VERSION == 4
#define EDITOR_STYLE_SAFE() FEditorStyle
#else
#define EDITOR_STYLE_SAFE() FAppStyle
#endif


struct FSlateIconBrowserFilterContext
{
	FString FilterString;
	ESlateIconBrowserRowFilterType RowType;
};


class FSlateIconBrowserModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
