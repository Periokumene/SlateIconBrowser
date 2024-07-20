// Copyright 2022 sirjofri. Licensed under MIT license. See License.txt for full license text.

#include "SlateIconBrowser.h"

#include "SlateIconBrowserStyle.h"
#include "HAL/PlatformApplicationMisc.h"
#include "LevelEditor.h"

#include "SSlateIconBrowserTab.h"


#define LOCTEXT_NAMESPACE "FSlateIconBrowserModule"

void FSlateIconBrowserModule::StartupModule()
{
	FSlateIconBrowserStyle::Initialize();
	FSlateIconBrowserStyle::ReloadTextures();
	SSlateIconBrowserTab::Register();
}

void FSlateIconBrowserModule::ShutdownModule()
{
	FSlateIconBrowserStyle::Shutdown();
	// FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(SlateIconBrowserTabName);
}

#undef LOCTEXT_NAMESPACE



#if ENGINE_MAJOR_VERSION == 5
IMPLEMENT_MODULE(FSlateIconBrowserModule, EditorIconBrowser)
#else
IMPLEMENT_MODULE(FSlateIconBrowserModule, SlateIconBrowser)
#endif
