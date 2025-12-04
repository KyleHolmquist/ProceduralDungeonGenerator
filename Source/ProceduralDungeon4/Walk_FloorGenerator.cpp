// Fill out your copyright notice in the Description page of Project Settings.


#include "Walk_FloorGenerator.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AWalk_FloorGenerator::AWalk_FloorGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AWalk_FloorGenerator::BeginPlay()
{
	Super::BeginPlay();

	GenerateMap();
	SpawnGeometry();
	
}

// Called every frame
void AWalk_FloorGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWalk_FloorGenerator::GenerateMap()
{
	const int32 NumCells = MapWidth * MapHeight;
	if (NumCells <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Walk_FloorGenerator: invalid map size."));
		return;
	}

	InitializeMap();
	RunRandomWalk();
}

void AWalk_FloorGenerator::InitializeMap()
{
	const int32 NumCells = MapWidth * MapHeight;
	Map.SetNum(NumCells);

	//Start with all walls
	for (int32 i = 0; i < NumCells; ++i)
	{
		//Wall
		Map[i] = true;
	}
}

void AWalk_FloorGenerator::RunRandomWalk()
{
	if (MapWidth <= 2 || MapHeight <= 2) return;

	int32 X, Y;

	if (bStartInCenter)
	{
		X = MapWidth / 2;
		Y = MapHeight / 2;
	}
	else
	{
		X = FMath::RandRange(1, MapWidth - 2);
		Y = FMath::RandRange(1, MapHeight - 2);
	}

	//Floor
	Map[Index(X, Y)] = false;

	for (int32 Step = 0; Step < NumSteps; ++Step)
	{
		int32 Dir = FMath::RandRange(0, 3);

		switch(Dir)
		{
			case 0:
				X++;
				break;
			case 1:
				X--;
				break;
			case 2:
				Y++;
				break;
			case 3:
				Y--;
				break;
			default:
				break;
		}

		//Keep within bound with 1-cell border of walls
		X = FMath::Clamp(X, 1, MapWidth - 2);
		Y = FMath::Clamp(Y, 1, MapHeight - 2);

		//Carve floor
		Map[Index(X, Y)] = false;
	}
}

void AWalk_FloorGenerator::SpawnGeometry()
{
	UWorld* World = GetWorld();
	if(!World) return;

	if (!FloorMesh && !WallMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("Walk_FloorGenerator: no meshes assigned"));
		return;
	}

	const float BasePlaneSize = 100.f;

	for (int32 y = 0; y < MapHeight; ++y)
	{
		for (int32 x = 0; x < MapWidth; ++x)
		{
			const bool bIsWall = Map[Index(x, y)];

			const FVector CellWorld = GetActorLocation() + FVector(x * TileSize, y * TileSize, 0.f);

			//Floor
			if (!bIsWall && FloorMesh)
			{
				const FVector Pos = CellWorld + FVector(0.f, 0.f, FloorZ);

				AStaticMeshActor* FloorActor = World->SpawnActor<AStaticMeshActor>(Pos, FRotator::ZeroRotator);
				if (!FloorActor) continue;

				UStaticMeshComponent* MeshComp = FloorActor->GetStaticMeshComponent();
				if(!MeshComp)
				{
					FloorActor->Destroy();
					continue;
				}

				MeshComp->SetStaticMesh(FloorMesh);

				const float Scale = TileSize / BasePlaneSize;
				FloorActor->SetActorScale3D(FVector(Scale, Scale, 1.f));
				FloorActor->SetMobility(EComponentMobility::Static);
			}
			//Walls
			else if (bIsWall && WallMesh)
			{
				const FVector Pos = CellWorld + FVector(0.f, 0.f, FloorZ + WallHeight * 0.f);

				AStaticMeshActor* WallActor = World->SpawnActor<AStaticMeshActor>(Pos, FRotator::ZeroRotator);
				if (!WallActor) continue;

				UStaticMeshComponent* MeshComp = WallActor->GetStaticMeshComponent();
				if (!MeshComp)
				{
					WallActor->Destroy();
					continue;
				}

				MeshComp->SetStaticMesh(WallMesh);

				const float XYScale = TileSize / BasePlaneSize;
				const float ZScale = WallHeight / BasePlaneSize;
				WallActor->SetActorScale3D(FVector(XYScale, XYScale, ZScale));
				WallActor->SetMobility(EComponentMobility::Static);
			}
		}
	}

}