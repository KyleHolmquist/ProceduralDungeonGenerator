// Fill out your copyright notice in the Description page of Project Settings.


#include "GridSpace.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"

// Sets default values
AGridSpace::AGridSpace()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GridPlaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GridPlaneMeshComponent"));
	GridPlaneMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RootComponent = GridPlaneMesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneAsset(TEXT("/Engine/BasicShapes/Plane"));
    if (PlaneAsset.Succeeded())
    {
        GridPlaneMesh->SetStaticMesh(PlaneAsset.Object);
    }

	GridCenter = CreateDefaultSubobject<USceneComponent>(TEXT("GridCenter"));
	GridCenter->SetupAttachment((GetRootComponent()));

	FrontArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("FrontArrow"));
	FrontArrow->SetupAttachment((GetRootComponent()));

}

// Called when the game starts or when spawned
void AGridSpace::BeginPlay()
{
	Super::BeginPlay();

	FVector Origin = GetActorLocation();
	FrontNeighbor = Origin + FVector(100.f, 0.f, 0.f);
	RearNeighbor = Origin - FVector(100.f, 0.f, 0.f);
	RightNeighbor = Origin + FVector(0.f, 100.f, 0.f);
	LeftNeighbor = Origin - FVector(0.f, 100.f, 0.f);

	NeighborPositions.Add(FrontNeighbor);
	NeighborPositions.Add(RearNeighbor);
	NeighborPositions.Add(RightNeighbor);
	NeighborPositions.Add(LeftNeighbor);
	
}

// Called every frame
void AGridSpace::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

TArray<FVector> AGridSpace::GetEmptyNeighbors() const
{
	TArray<FVector> emptyNeighbors;
	for (const FVector& pos : NeighborPositions)
	{
		if (IsSpaceEmpty(pos, 10.f))
		{
			emptyNeighbors.Add(pos);
		}
	}

	return emptyNeighbors;
}

bool AGridSpace::IsSpaceEmpty(const FVector& Location, float Radius) const
{
	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(Radius);

	//Perform an overlap check
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	//Ignore self
	QueryParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->OverlapMultiByChannel(
		Overlaps,
		Location,
		FQuat::Identity,
		ECC_WorldStatic,
		CollisionShape,
		QueryParams
	);

	//Return true if no overlaps were found
	return !bHit;
}