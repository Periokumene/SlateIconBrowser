#include "SlateIconBrowserUserSettings.h"

void USlateIconBrowserUserSettings::
ValidateConfig()
{
	if (SelectedStyle.IsNone())
	{
#if ENGINE_MAJOR_VERSION == 5
		SelectedStyle = FAppStyle::GetAppStyleSetName();
#else
		SelectedStyle = FEditorStyle::GetStyleSetName();
#endif
	}
}


void USlateIconBrowserUserSettings::
InitTranslator()
{
	if (bTranslatorInitialized)
	{
		return;
	}
	
	BaseTranslator.Add("CoreStyle", "FCoreStyle");
	BaseTranslator.Add("EditorStyle", "FEditorStyle");
	BaseTranslator.Add("ActorSequenceEditorStyle", "FActorSequenceEditorStyle");
	BaseTranslator.Add("ChaosSolverEditorStyle", "FChaosSolverEditorStyle");
	// TODO More Translators
}

FString USlateIconBrowserUserSettings::
TryTranslate(const FName& InStyleSetName)
{
	if (const FString* ResultPtr = AdditionalTranslator.Find(InStyleSetName))
	{
		return *ResultPtr;
	}

	if (const FString* ResultPtr = BaseTranslator.Find(InStyleSetName))
	{
		return *ResultPtr;
	}

	return InStyleSetName.ToString();
}
