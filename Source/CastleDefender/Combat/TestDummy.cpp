#include "Combat/TestDummy.h"

#include "Combat/CombatStateComponent.h"
#include "Combat/HealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/GameDebug.h"
#include "Core/GameLog.h"
#include "Core/GameTags.h"
#include "DrawDebugHelpers.h"
#include "UObject/ConstructorHelpers.h"

ATestDummy::ATestDummy()
	: TeamId(Team_Enemy)
{
	// Tick: debug draw only (game.debug.Combat); this actor never ships in content.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	Mesh->SetStaticMesh(Cylinder.Object);
	Mesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 1.8f));
	RootComponent = Mesh;

	Health = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	CombatState = CreateDefaultSubobject<UCombatStateComponent>(TEXT("CombatState"));
	CombatStateConfig.MaxPoise = 50.f;
}

void ATestDummy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

#if ENABLE_DRAW_DEBUG
	if (GameDebug::CVarCombat.GetValueOnGameThread() > 0)
	{
		const FVector Top = GetActorLocation() + FVector(0.f, 0.f, 130.f);
		DrawDebugSphere(GetWorld(), Top, 25.f, 12, Health->IsDead() ? FColor::Red : FColor::Green);
		DrawDebugString(GetWorld(), Top + FVector(0.f, 0.f, 40.f),
			FString::Printf(TEXT("%.0f / %.0f"), Health->GetCurrentHealth(), Health->GetMaxHealth()), nullptr, FColor::White, 0.f);
	}
#endif
}

void ATestDummy::ApplyDebugHit(float Damage)
{
	FCombatHit Hit;
	Hit.Damage = Damage;
	Hit.DamageType = GameTags::Damage_Physical;
	Hit.SourceLayer = ECombatLayer::Environment;
	Hit.HitLocation = GetActorLocation();
	Health->ApplyHit(Hit);
}

void ATestDummy::BeginPlay()
{
	Super::BeginPlay();
	Health->OnDeath.AddDynamic(this, &ATestDummy::HandleDeath);
	CombatState->Init(CombatStateConfig);
}

void ATestDummy::HandleDeath(const FCombatHit& KillingHit)
{
	UE_LOG(LogGameCombat, Log, TEXT("%s died"), *GetName());
}
