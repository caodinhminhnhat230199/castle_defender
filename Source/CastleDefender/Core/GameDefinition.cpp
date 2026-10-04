#include "Core/GameDefinition.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "GameDefinition"

FPrimaryAssetId UGameDefinition::GetPrimaryAssetId() const
{
	if (HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
	{
		return FPrimaryAssetId();
	}

	// Blueprint subclasses share the type of their native definition class.
	const UClass* NativeClass = GetClass();
	while (NativeClass && !NativeClass->HasAnyClassFlags(CLASS_Native))
	{
		NativeClass = NativeClass->GetSuperClass();
	}
	return FPrimaryAssetId(FPrimaryAssetType(NativeClass->GetFName()), GetFName());
}

#if WITH_EDITOR
EDataValidationResult UGameDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (DisplayName.IsEmpty())
	{
		Context.AddError(LOCTEXT("MissingDisplayName", "DisplayName is required."));
		Result = EDataValidationResult::Invalid;
	}

	return CombineDataValidationResults(Result, EDataValidationResult::Valid);
}
#endif

#undef LOCTEXT_NAMESPACE
