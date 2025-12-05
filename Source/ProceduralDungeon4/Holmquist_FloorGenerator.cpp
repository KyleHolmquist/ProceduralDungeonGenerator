// Fill out your copyright notice in the Description page of Project Settings.


#include "Holmquist_FloorGenerator.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AHolmquist_FloorGenerator::AHolmquist_FloorGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AHolmquist_FloorGenerator::BeginPlay()
{
	Super::BeginPlay();

	//Allocate Grid
	GenerateRoomLayout();
	SpawnFloorTiles();
	
}

// Called every frame
void AHolmquist_FloorGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHolmquist_FloorGenerator::GenerateRoomLayout()
{
	//Clear state
	const int32 NumCells = GridWidth * GridHeight;
	if (NumCells <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Holmquist_FloorGenerator: Invalid grid size"));
		return;
	}

	Grid.Init(false, NumCells);
	Frontier.Reset();

	//RNG Setup
	FRandomStream Rng;
	if (Seed >= 0)
	{
		Rng.Initialize(Seed);
	}
	else
	{
		//Non-deterministic
		Rng.GenerateNewSeed();
	}

	//Choose starting cell at center of grid
	const int32 StartX = GridWidth / 2;
	const int32 StartY = GridHeight / 2;

	Grid[Index(StartX, StartY)] = true;
	Frontier.Add(FIntPoint(StartX, StartY));

	int32 TilesPlaced = 1;
	const int32 TargetTiles = FMath::Clamp(NumTiles, 1, NumCells);

	//Grow the room
	while (TilesPlaced < TargetTiles && Frontier.Num() > 0)
	{
		//Choose a random floor cell to grow from
		const int32 FrontierIndex = Rng.RandRange(0, Frontier.Num() - 1);
		const FIntPoint Cell = Frontier[FrontierIndex];
		
		const int32 X = Cell.X;
		const int32 Y = Cell.Y;

		//Gather empty neighbors around the cell
		TArray<FIntPoint> EmptyNeighbors;

		const int32 DX[4] = {1, -1, 0, 0};
		const int32 DY[4] = {0, 0, 1, -1};

		for (int32 i = 0; i < 4; ++i)
		{
			const int32 NX = X + DX[i];
			const int32 NY = Y + DY[i];

			//Stay inside the grid
			if (NX < 0 || NX >= GridWidth || NY < 0 || NY >= GridHeight) continue;

			if (!Grid[Index(NX, NY)])
			{
				EmptyNeighbors.Add(FIntPoint(NX, NY));
			}
		}

		if (EmptyNeighbors.Num() == 0)
		{
			//This floor cell has no empty neighbors. Remove from Frontier array
			Frontier.RemoveAtSwap(FrontierIndex);
			continue;
		}

		//Choose a random empty neighbor
		const int32 RandEmptyIndex = Rng.RandRange(0, EmptyNeighbors.Num() - 1);
		const FIntPoint NewCell = EmptyNeighbors[RandEmptyIndex];

		//Carve floor
		Grid[Index(NewCell.X, NewCell.Y)] = true;
		Frontier.Add(NewCell);

		++TilesPlaced;
	}

	UE_LOG(LogTemp, Log, TEXT("Holmquist_FloorGenerator: Placed %d floor tiles (target %d)."),
			TilesPlaced, TargetTiles);
}

void AHolmquist_FloorGenerator::SpawnFloorTiles()
{
	if (!FloorMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("Holmquist_FloorGenerator: FloorMesh not assigned."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	const float BaseSize = 100.f;
	const float HalfTile = TileSize * 0.5f;

	//---- Floors and Edge Walls ----

	for (int32 y = 0; y < GridHeight; ++y)
	{
		for (int32 x = 0; x < GridWidth; ++x)
		{
			const bool bIsFloor = Grid[Index(x, y)];
			const FVector TileCenter = (GetActorLocation() + FVector(x * TileSize, y * TileSize, 0.f));

			//---- Floor ----
			if (bIsFloor && FloorMesh)
			{
				const FVector FloorPos = TileCenter + FVector(0.f, 0.f, FloorZ);
				AStaticMeshActor* FloorActor = World->SpawnActor<AStaticMeshActor>(FloorPos, FRotator::ZeroRotator);
				if (!FloorActor) continue;

				UStaticMeshComponent* MeshComp = FloorActor->GetStaticMeshComponent();
				if (!MeshComp)
				{
					FloorActor->Destroy();
					continue;
				}
				
				MeshComp->SetStaticMesh(FloorMesh);

				const float FloorScale = TileSize / BaseSize;
				FloorActor->SetActorScale3D(FVector(FloorScale, FloorScale, 1.f));
				FloorActor->SetMobility(EComponentMobility::Static);
			}

			//---- Walls around Floor ----
			if (!bIsFloor || !WallMesh)
			{
				continue;
			}

			// Helper lambda to ask "is there floor at (NX, NY)?"
			auto HasFloorAt = [&](int32 NX, int32 NY) -> bool
			{
				if (NX < 0 || NX >= GridWidth || NY < 0 || NY >= GridHeight)
				{
					return false;
				}
				return Grid[Index(NX, NY)];
			};

			// EAST (+X) edge  → vertical wall (length along Y)
			{
				const bool bHasEastFloor = HasFloorAt(x + 1, y);
				if (!bHasEastFloor)
				{
					const FVector WallPos = TileCenter + FVector(HalfTile, 0.f, FloorZ + WallHeight * 0.f);
					const FRotator Rot(0.f, 90.f, 0.f);

					AStaticMeshActor* WallActor = World->SpawnActor<AStaticMeshActor>(WallPos, Rot);
					if (WallActor)
					{
						UStaticMeshComponent* WComp = WallActor->GetStaticMeshComponent();
						if (!WComp)
						{
							WallActor->Destroy();
						}
						else
						{
							WComp->SetStaticMesh(WallMesh);

							const float ScaleX = TileSize      / BaseSize; // length
							const float ScaleY = WallThickness / BaseSize; // thickness
							const float ScaleZ = WallHeight    / BaseSize; // height

							WallActor->SetActorScale3D(FVector(ScaleX, ScaleY, ScaleZ));
							WallActor->SetMobility(EComponentMobility::Static);
						}
					}
				}

				// WEST (-X) edge → vertical wall
				{
					const bool bHasWestFloor = HasFloorAt(x - 1, y);
					if (!bHasWestFloor)
					{
						const FVector WallPos = TileCenter + FVector(-HalfTile, 0.f, FloorZ + WallHeight * 0.f);
						const FRotator Rot(0.f, 90.f, 0.f);

						AStaticMeshActor* WallActor = World->SpawnActor<AStaticMeshActor>(WallPos, Rot);
						if (WallActor)
						{
							UStaticMeshComponent* WComp = WallActor->GetStaticMeshComponent();
							if (!WComp)
							{
								WallActor->Destroy();
							}
							else
							{
								WComp->SetStaticMesh(WallMesh);

								const float ScaleX = TileSize      / BaseSize;
								const float ScaleY = WallThickness / BaseSize;
								const float ScaleZ = WallHeight    / BaseSize;

								WallActor->SetActorScale3D(FVector(ScaleX, ScaleY, ScaleZ));
								WallActor->SetMobility(EComponentMobility::Static);
							}
						}
					}
				}

				// NORTH (+Y) edge → horizontal wall (length along X)
				{
					const bool bHasNorthFloor = HasFloorAt(x, y + 1);
					if (!bHasNorthFloor)
					{
						const FVector WallPos = TileCenter + FVector(0.f, HalfTile, FloorZ + WallHeight * 0.f);
						const FRotator Rot(0.f, 0.f, 0.f);

						AStaticMeshActor* WallActor = World->SpawnActor<AStaticMeshActor>(WallPos, Rot);
						if (WallActor)
						{
							UStaticMeshComponent* WComp = WallActor->GetStaticMeshComponent();
							if (!WComp)
							{
								WallActor->Destroy();
							}
							else
							{
								WComp->SetStaticMesh(WallMesh);

								const float ScaleX = TileSize      / BaseSize;
								const float ScaleY = WallThickness / BaseSize;
								const float ScaleZ = WallHeight    / BaseSize;

								WallActor->SetActorScale3D(FVector(ScaleX, ScaleY, ScaleZ));
								WallActor->SetMobility(EComponentMobility::Static);
							}
						}
					}
				}

				// SOUTH (-Y) edge → horizontal wall
				{
					const bool bHasSouthFloor = HasFloorAt(x, y - 1);
					if (!bHasSouthFloor)
					{
						const FVector WallPos = TileCenter + FVector(0.f, -HalfTile, FloorZ + WallHeight * 0.f);
						const FRotator Rot(0.f, 0.f, 0.f);

						AStaticMeshActor* WallActor = World->SpawnActor<AStaticMeshActor>(WallPos, Rot);
						if (WallActor)
						{
							UStaticMeshComponent* WComp = WallActor->GetStaticMeshComponent();
							if (!WComp)
							{
								WallActor->Destroy();
							}
							else
							{
								WComp->SetStaticMesh(WallMesh);

								const float ScaleX = TileSize      / BaseSize;
								const float ScaleY = WallThickness / BaseSize;
								const float ScaleZ = WallHeight    / BaseSize;

								WallActor->SetActorScale3D(FVector(ScaleX, ScaleY, ScaleZ));
								WallActor->SetMobility(EComponentMobility::Static);
							}
						}
					}
				}
			}
		}
	}

	//---- Pillars in Interior Gaps ----

	if (bSpawnPillarsInGaps && WallMesh)
	{
		for (int32 y = 0; y < GridHeight; ++y)
		{
			for (int32 x = 0; x < GridWidth; ++x)
			{
				if (Grid[Index(x, y)]) continue;

				//Count floor neighbors
				int32 FloorNeighbors = 0;
				const int32 DX[4] = {1, -1, 0, 0};
				const int32 DY[4] = {0, 0, -1, 1};
				
				for (int32 i = 0; i < 4; ++i)
				{
					const int32 NX = x + DX[i];
					const int32 NY = y + DY[i];

					if (NX < 0 || NX >= GridWidth || NY < 0 || NY >= GridHeight) continue;

					if (Grid[Index(NX, NY)]) ++FloorNeighbors;
				}

				//Only spawn pillars when it's a hole surrounded by floor
				if (FloorNeighbors < 3) continue;

				const FVector PillarPos = GetActorLocation() + FVector(x * TileSize, y * TileSize, FloorZ);

				AStaticMeshActor* PillarActor = World->SpawnActor<AStaticMeshActor>(PillarPos, FRotator::ZeroRotator);
				if (!PillarActor) continue;

				UStaticMeshComponent* PComp = PillarActor->GetStaticMeshComponent();
				if (!PComp)
				{
					PillarActor->Destroy();
					continue;
				}

				PComp->SetStaticMesh(WallMesh);

				const float XYScale = PillarSize / BaseSize;
				const float ZScale = WallHeight / BaseSize;

				PillarActor->SetActorScale3D(FVector(XYScale, XYScale, ZScale));
				PillarActor->SetMobility(EComponentMobility::Static);
			}
		}
	}
}

