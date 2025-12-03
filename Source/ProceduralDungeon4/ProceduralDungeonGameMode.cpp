// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralDungeonGameMode.h"
#include "ProceduralDungeon4Character.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

AProceduralDungeonGameMode::AProceduralDungeonGameMode()
{
    DefaultPawnClass = nullptr;
    PlayerControllerClass = APlayerController::StaticClass();
    bStartPlayersAsSpectators = true;
    
}

void AProceduralDungeonGameMode::SpawnPlayerAtTransform(const FTransform& SpawnTransform)
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("No Player Controller found!"));
        return;
    } 
    UE_LOG(LogTemp, Warning, TEXT("Player Controller found!"));
    

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // Spawn your Character (C++ class or the BP via its native class)
    AProceduralDungeon4Character* NewPawn =
        World->SpawnActor<AProceduralDungeon4Character>(
            AProceduralDungeon4Character::StaticClass(),
            SpawnTransform,
            Params);

    if (NewPawn)
    {
        if (APawn* Old = PC->GetPawn())
        {
            PC->UnPossess();
            Old->Destroy(); // optional if replacing
        }
        PC->Possess(NewPawn);
    }
}

