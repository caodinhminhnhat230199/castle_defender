#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Army/Squad.h"
#include "Player/CommandComponent.h"
#include "Player/HeroPlayerController.h"
#include "Tests/EnemyTestFixture.h"
#include "Tests/SquadRegistryTestListener.h"
#include "Core/GameTuningSettings.h"
#include "GameFramework/Pawn.h"

BEGIN_DEFINE_SPEC(FSquadRegistrySpec, "CastleDefender.Army.Registry", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FSquadRegistrySpec)

void FSquadRegistrySpec::Define()
{
	It("accepts three distinct squads, rejects a fourth, and makes duplicate registration idempotent", [this]()
	{
		TGuardValue<int32> Cap(GetMutableDefault<UGameTuningSettings>()->MaxActiveSquads, 3);
		FEnemyTestWorld F;
		UCommandComponent* Registry = NewObject<UCommandComponent>(F.World->SpawnActor<AHeroPlayerController>());
		ASquad* First = F.World->SpawnActor<ASquad>();
		ASquad* Second = F.World->SpawnActor<ASquad>();
		ASquad* Third = F.World->SpawnActor<ASquad>();
		ASquad* Fourth = F.World->SpawnActor<ASquad>();
		TestTrue("First accepted", Registry->RegisterSquad(First));
		TestTrue("Second accepted", Registry->RegisterSquad(Second));
		TestTrue("Third accepted", Registry->RegisterSquad(Third));
		TestTrue("Same squad is idempotent at cap", Registry->RegisterSquad(First));
		TestFalse("Fourth rejected", Registry->RegisterSquad(Fourth));
		TestEqual("Cap preserved", Registry->GetSquads().Num(), 3);
		Registry->UnregisterSquad(Second);
		TestTrue("Released slot reusable", Registry->RegisterSquad(Fourth));
		TestFalse("Removed squad absent", Registry->GetSquads().Contains(Second));
		TestEqual("Still three", Registry->GetSquads().Num(), 3);
	});

	It("rejects null, foreign-world and destroyed squads without consuming slots", [this]()
	{
		FEnemyTestWorld F;
		FEnemyTestWorld Foreign;
		UCommandComponent* Registry = NewObject<UCommandComponent>(F.World->SpawnActor<AHeroPlayerController>());
		ASquad* ForeignSquad = Foreign.World->SpawnActor<ASquad>();
		ASquad* Destroyed = F.World->SpawnActor<ASquad>();
		Destroyed->Destroy();
		TestFalse("Null rejected", Registry->RegisterSquad(nullptr));
		TestFalse("Foreign world rejected", Registry->RegisterSquad(ForeignSquad));
		TestFalse("Destroyed rejected", Registry->RegisterSquad(Destroyed));
		TestEqual("No slots consumed", Registry->GetSquads().Num(), 0);
		TestFalse("No gameplay tick", Registry->PrimaryComponentTick.bCanEverTick);
	});

	It("uses the data cap and publishes only real changes after updating the registry", [this]()
	{
		TGuardValue<int32> Cap(GetMutableDefault<UGameTuningSettings>()->MaxActiveSquads, 1);
		FEnemyTestWorld F;
		UCommandComponent* Registry = NewObject<UCommandComponent>(F.World->SpawnActor<AHeroPlayerController>());
		USquadRegistryTestListener* Listener = NewObject<USquadRegistryTestListener>(Registry);
		Listener->Registry = Registry;
		Registry->OnSquadsChanged.AddDynamic(Listener, &USquadRegistryTestListener::HandleChanged);
		ASquad* First = F.World->SpawnActor<ASquad>();
		ASquad* Second = F.World->SpawnActor<ASquad>();
		TestTrue("One allowed", Registry->RegisterSquad(First));
		TestTrue("Duplicate allowed", Registry->RegisterSquad(First));
		TestFalse("Configured cap obeyed", Registry->RegisterSquad(Second));
		Registry->UnregisterSquad(Second);
		TestEqual("Only one real change", Listener->Changes, 1);
		Registry->UnregisterSquad(First);
		Registry->UnregisterSquad(First);
		TestTrue("Slot reused", Registry->RegisterSquad(Second));
		TestEqual("Three mutations", Listener->Changes, 3);
		TestTrue("Observers see committed counts", Listener->Counts == TArray<int32>({1, 0, 1}));
	});

	It("keeps command authority on the same controller across pawn replacement", [this]()
	{
		FEnemyTestWorld F;
		AHeroPlayerController* Controller = F.World->SpawnActor<AHeroPlayerController>();
		UCommandComponent* Registry = Controller->GetCommandComponent();
		TestNotNull("Default component", Registry);
		if (!Registry) { return; }
		ASquad* Squad = F.World->SpawnActor<ASquad>();
		TestTrue("Registered", Registry->RegisterSquad(Squad));
		APawn* First = F.World->SpawnActor<APawn>();
		APawn* Replacement = F.World->SpawnActor<APawn>();
		Controller->Possess(First);
		Controller->UnPossess();
		First->Destroy();
		Controller->Possess(Replacement);
		TestTrue("Same controller registry", Controller->GetCommandComponent() == Registry);
		TestTrue("Squad still commandable after body replacement", Registry->GetSquads().Contains(Squad));
	});
}
#endif
