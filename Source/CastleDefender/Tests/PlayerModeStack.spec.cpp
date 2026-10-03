#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Player/PlayerMode.h"

BEGIN_DEFINE_SPEC(FPlayerModeStackSpec, "CastleDefender.Player.ModeStack", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	FPlayerModeStack Stack;
END_DEFINE_SPEC(FPlayerModeStackSpec)

void FPlayerModeStackSpec::Define()
{
	BeforeEach([this]()
	{
		Stack = FPlayerModeStack();
	});

	It("starts in Combat", [this]()
	{
		TestTrue("Top", Stack.Top() == EPlayerMode::Combat);
	});

	It("returns to the previous mode, not Combat, after popping the top", [this]()
	{
		Stack.Push(EPlayerMode::Build, "Build");
		Stack.Push(EPlayerMode::Modal, "Menu");
		TestTrue("Top after pushes", Stack.Top() == EPlayerMode::Modal);

		TestTrue("Pop Menu", Stack.Pop("Menu"));
		TestTrue("Top after popping Modal", Stack.Top() == EPlayerMode::Build);

		TestTrue("Pop Build", Stack.Pop("Build"));
		TestTrue("Top after popping Build", Stack.Top() == EPlayerMode::Combat);
	});

	It("removes only the matching entry on an out-of-order pop", [this]()
	{
		Stack.Push(EPlayerMode::Build, "Build");
		Stack.Push(EPlayerMode::Modal, "Menu");

		TestTrue("Pop Build under Modal", Stack.Pop("Build"));
		TestTrue("Top unchanged", Stack.Top() == EPlayerMode::Modal);
		TestEqual("Depth", Stack.Num(), 1);

		TestTrue("Pop Menu", Stack.Pop("Menu"));
		TestTrue("Back to Combat", Stack.Top() == EPlayerMode::Combat);
	});

	It("ignores a pop with an unknown reason", [this]()
	{
		Stack.Push(EPlayerMode::Build, "Build");
		TestFalse("Pop unknown", Stack.Pop("Unknown"));
		TestTrue("Top unchanged", Stack.Top() == EPlayerMode::Build);
	});
}

#endif
