// Fill out your copyright notice in the Description page of Project Settings.


#include "BSP_FloorGenerator.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/KismetMathLibrary.h"



// Sets default values
ABSP_FloorGenerator::ABSP_FloorGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ABSP_FloorGenerator::BeginPlay()
{
	Super::BeginPlay();

	GenerateBSP();

	SpawnFloorPlanes();
	
}

void ABSP_FloorGenerator::GenerateBSP()
{
	LeafRegions.Empty();

	FBSPLeaf Root(FIntPoint(0, 0), MapSize);
	SplitSpace(Root, 0);

}

void ABSP_FloorGenerator::SplitSpace(const FBSPLeaf& Region, int32 Depth)
{
	const int32 Width = Region.Width();
	const int32 Height = Region.Height();

	//Stop if too small or depth reached
	if (Depth >= MaxDepth || Width <= MinLeafSize * 2 && Height <= MinLeafSize * 2)
	{
		LeafRegions.Add(Region);
		return;
	}

	//Decide whether to split vertically or horizontally
	bool bSplitVertically;

	//If one dimension is much larger, favor splitting that axis
	if (Width > Height)
	{
		bSplitVertically = true;
	}
	else if (Height > Width)
	{
		bSplitVertically = false;
	}
	else
	{
		//50/50 when equal
		bSplitVertically = FMath::RandBool();
	}

	//If the chosen axis is too small to split, try another axis
	if (bSplitVertically && Width < MinLeafSize * 2)
	{
		bSplitVertically = false;
	}
	else if (!bSplitVertically && Height < MinLeafSize * 2)
	{
		bSplitVertically = true;
	}

	//If it still can't be split, this is a leaf
	if (bSplitVertically && Width < MinLeafSize * 2 ||
	!bSplitVertically && Height < MinLeafSize * 2)
	{
		LeafRegions.Add(Region);
		return;
	}

	if (bSplitVertically)
	{
		//Vertical split: X Axis
		const int32 SplitMin = Region.Min.X + MinLeafSize;
		const int32 SplitMax = Region.Max.X - MinLeafSize;

		if (SplitMin >= SplitMax)
		{
			LeafRegions.Add(Region);
			return;
		}

		const int32 SplitX = FMath::RandRange(SplitMin, SplitMax);

		FBSPLeaf Left(FIntPoint(Region.Min.X, Region.Min.Y), FIntPoint(SplitX, Region.Max.Y));
		FBSPLeaf Right(FIntPoint(SplitX, Region.Min.Y), FIntPoint(Region.Max.X, Region.Max.Y));

		SplitSpace(Left, Depth + 1);
		SplitSpace(Right, Depth + 1);
	}
	else
	{
		//Horizontal Split: Y Axis
		const int32 SplitMin = Region.Min.Y + MinLeafSize;
		const int32 SplitMax = Region.Max.Y - MinLeafSize;

		if (SplitMin >= SplitMax)
		{
			LeafRegions.Add(Region);
			return;
		}

		const int32 SplitY = FMath::RandRange(SplitMin, SplitMax);

		FBSPLeaf Bottom(FIntPoint(Region.Min.X, Region.Min.Y), FIntPoint(Region.Max.X, SplitY));
		FBSPLeaf Top(FIntPoint(Region.Min.X, SplitY), FIntPoint(Region.Max.X, Region.Max.Y));

		SplitSpace(Bottom, Depth + 1);
		SplitSpace(Top, Depth + 1);
	}
}

void ABSP_FloorGenerator::SpawnFloorPlanes()
{
	if (!FloorMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("BSP_FloorGenerator: FloorMesh is null"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	//Assume the plane and cube meshes are 100x100 units. Adjust if need be
	const float BaseMeshSize = 100.f;

	for (const FBSPLeaf& Leaf : LeafRegions)
	{
		const int32 LeafW = Leaf.Width();
		const int32 LeafH = Leaf.Height();

		if (LeafW <= 0 || LeafH <= 0)
		{
			continue;
		}

		//Random padding inside the leaf so rooms don't fill entire region
		const int32 PadMin = RoomPaddingMin;
		const int32 PadMax = RoomPaddingMax;

		int32 PadLeft = FMath::RandRange(PadMin, PadMax);
		int32 PadRight = FMath::RandRange(PadMin, PadMax);
		int32 PadBottom = FMath::RandRange(PadMin, PadMax);
		int32 PadTop = FMath::RandRange(PadMin, PadMax);

		//Clamp padding so room doesn't invert
		PadLeft = FMath::Clamp(PadLeft, 0, LeafW - 1);
		PadRight = FMath::Clamp(PadRight, 0, LeafW - 1 - PadLeft);
		PadBottom = FMath::Clamp(PadBottom, 0, LeafH - 1);
		PadTop = FMath::Clamp(PadTop, 0, LeafH - 1 - PadBottom);

		const int32 RoomMinX = Leaf.Min.X + PadLeft;
		const int32 RoomMaxX = Leaf.Max.X - PadRight;
		const int32 RoomMinY = Leaf.Min.Y + PadBottom;
		const int32 RoomMaxY = Leaf.Max.Y - PadTop;

		const int32 RoomW = RoomMaxX - RoomMinX;
		const int32 RoomH = RoomMaxY - RoomMinY;

		if (RoomW <= 0 || RoomH <= 0)
		{
			continue;
		}

		const float WorldWidth = RoomW * TileSize;
		const float WorldHeight = RoomH * TileSize;

		//Center of this room in world space
		const FVector RoomCenter(
			(RoomMinX + RoomW * 0.5f) * TileSize,
			(RoomMinY + RoomH * 0.5f) * TileSize,
			FloorZ
		);

		const FVector WorldLocation = GetActorLocation() + RoomCenter;

		//Scale so that a 100x100 plane becomes WorldWidth x WorldHeight
		const FVector FloorScale(
			WorldWidth / BaseMeshSize,
			WorldHeight / BaseMeshSize,
			1.f
		);

		//---- Floor ---- 

		AStaticMeshActor* FloorActor = World->SpawnActor<AStaticMeshActor>(WorldLocation, FRotator::ZeroRotator);
		if (!FloorActor) continue;

		if (UStaticMeshComponent* MeshComp = FloorActor->GetStaticMeshComponent())
		{
			MeshComp->SetStaticMesh(FloorMesh);
			FloorActor->SetActorScale3D(FloorScale);
			FloorActor->SetMobility(EComponentMobility::Static);
		}
		else
		{
			FloorActor->Destroy();
			continue;
		}

		//---- Walls ----

		if (WallMesh)
		{
			const float RoomWorldWidth = WorldWidth;
			const float RoomWorldHeight = WorldHeight;

			//Bottom wall
			{
				const FVector WallCenter(
					(RoomMinX + RoomW * 0.5f) * TileSize,
					RoomMinY * TileSize,
					FloorZ + WallHeight * 0.5f
				);

				AStaticMeshActor* Wall = World->SpawnActor<AStaticMeshActor>(GetActorLocation() + WallCenter, FRotator::ZeroRotator);
				if (Wall)
				{
					if (UStaticMeshComponent* WComp = Wall->GetStaticMeshComponent())
					{
						WComp->SetStaticMesh(WallMesh);
						const float ScaleX = WallThickness / BaseMeshSize;
						const float ScaleY = RoomWorldHeight / BaseMeshSize;
						const float ScaleZ = WallHeight / BaseMeshSize;
						Wall->SetActorScale3D(FVector(ScaleX, ScaleY, ScaleZ));
						Wall->SetMobility(EComponentMobility::Static);
					}
					else
					{
						Wall->Destroy();
					}
				}
			}

			//Right Wall
			{
				const FVector WallCenter(
					RoomMaxX * TileSize,
					(RoomMinY + RoomH * 0.5f) * TileSize,
					FloorZ + WallHeight * 0.5f
				);

				AStaticMeshActor* Wall = World->SpawnActor<AStaticMeshActor>(GetActorLocation() + WallCenter, FRotator::ZeroRotator);
				if (Wall)
				{
					if (UStaticMeshComponent* WComp = Wall->GetStaticMeshComponent())
					{
						WComp->SetStaticMesh(WallMesh);
						const float ScaleX = WallThickness / BaseMeshSize;
						const float ScaleY = RoomWorldHeight / BaseMeshSize;
						const float ScaleZ = WallHeight / BaseMeshSize;
						Wall->SetActorScale3D(FVector(ScaleX, ScaleY, ScaleZ));
						Wall->SetMobility(EComponentMobility::Static);
					}
					else
					{
						Wall->Destroy();
					}
				}
			}
		}

	}
}

// Called every frame
void ABSP_FloorGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

