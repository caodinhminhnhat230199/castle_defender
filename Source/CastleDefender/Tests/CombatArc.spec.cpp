#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/CombatLibrary.h"
#include "Combat/TestDummy.h"

BEGIN_DEFINE_SPEC(FCombatArcSpec, "CastleDefender.Combat.Arc", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	ATestDummy* Defender = nullptr;
END_DEFINE_SPEC(FCombatArcSpec)

void FCombatArcSpec::Define()
{
	BeforeEach([this]()
	{
		Defender = NewObject<ATestDummy>();
		// Face +X forward, centered at origin
		Defender->SetActorLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
	});

	Describe("180 Degree Half-Plane Front Arc", [this]()
	{
		It("evaluates 0, 89, 90, 91, and 180 degrees correctly", [this]()
		{
			// 0 deg: directly in front (+X)
			TestTrue("0 deg in front", UCombatLibrary::IsInFrontArc(Defender, FVector(200.f, 0.f, 0.f), 180.f));

			// +89 deg: inside
			const float Rad89 = FMath::DegreesToRadians(89.f);
			TestTrue("89 deg in front", UCombatLibrary::IsInFrontArc(Defender, FVector(200.f * FMath::Cos(Rad89), 200.f * FMath::Sin(Rad89), 0.f), 180.f));

			// +90 deg: right at boundary
			TestTrue("90 deg boundary in front", UCombatLibrary::IsInFrontArc(Defender, FVector(0.f, 200.f, 0.f), 180.f));

			// +91 deg: outside front arc
			const float Rad91 = FMath::DegreesToRadians(91.f);
			TestFalse("91 deg outside arc", UCombatLibrary::IsInFrontArc(Defender, FVector(200.f * FMath::Cos(Rad91), 200.f * FMath::Sin(Rad91), 0.f), 180.f));

			// 180 deg: directly behind (-X)
			TestFalse("180 deg directly behind", UCombatLibrary::IsInFrontArc(Defender, FVector(-200.f, 0.f, 0.f), 180.f));
		});
	});

	Describe("140 Degree Warlord Block Arc", [this]()
	{
		It("evaluates 0, +-69, +-70, +-71, and 180 degrees correctly", [this]()
		{
			// 0 deg: directly ahead
			TestTrue("0 deg inside 140 arc", UCombatLibrary::IsInFrontArc(Defender, FVector(300.f, 0.f, 0.f), 140.f));

			// +69 and -69 deg: inside 70 deg half-angle
			const float Rad69 = FMath::DegreesToRadians(69.f);
			TestTrue("+69 deg inside 140 arc", UCombatLibrary::IsInFrontArc(Defender, FVector(300.f * FMath::Cos(Rad69), 300.f * FMath::Sin(Rad69), 0.f), 140.f));
			TestTrue("-69 deg inside 140 arc", UCombatLibrary::IsInFrontArc(Defender, FVector(300.f * FMath::Cos(Rad69), -300.f * FMath::Sin(Rad69), 0.f), 140.f));

			// +70 deg: boundary
			const float Rad70 = FMath::DegreesToRadians(70.f);
			TestTrue("+70 deg boundary inside 140 arc", UCombatLibrary::IsInFrontArc(Defender, FVector(300.f * FMath::Cos(Rad70), 300.f * FMath::Sin(Rad70), 0.f), 140.f));

			// +71 and -71 deg: outside 70 deg half-angle
			const float Rad71 = FMath::DegreesToRadians(71.f);
			TestFalse("+71 deg outside 140 arc", UCombatLibrary::IsInFrontArc(Defender, FVector(300.f * FMath::Cos(Rad71), 300.f * FMath::Sin(Rad71), 0.f), 140.f));
			TestFalse("-71 deg outside 140 arc", UCombatLibrary::IsInFrontArc(Defender, FVector(300.f * FMath::Cos(Rad71), -300.f * FMath::Sin(Rad71), 0.f), 140.f));

			// Behind
			TestFalse("Behind outside 140 arc", UCombatLibrary::IsInFrontArc(Defender, FVector(-300.f, 0.f, 0.f), 140.f));
		});
	});

	Describe("Edge Cases", [this]()
	{
		It("handles null defender, zero arc, full 360 arc, and overlapping locations", [this]()
		{
			TestFalse("Null defender returns false", UCombatLibrary::IsInFrontArc(nullptr, FVector(100.f, 0.f, 0.f), 180.f));
			TestFalse("0 deg arc returns false", UCombatLibrary::IsInFrontArc(Defender, FVector(100.f, 0.f, 0.f), 0.f));
			TestTrue("360 deg arc includes behind", UCombatLibrary::IsInFrontArc(Defender, FVector(-100.f, 0.f, 0.f), 360.f));
			TestTrue("Overlapping location returns true", UCombatLibrary::IsInFrontArc(Defender, FVector::ZeroVector, 90.f));
		});
	});

	AfterEach([this]()
	{
		Defender = nullptr;
	});
}

#endif
