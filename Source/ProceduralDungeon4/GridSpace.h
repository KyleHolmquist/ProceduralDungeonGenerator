// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridSpace.generated.h"

class UPlaneComponent;
class USceneComponent;
class UArrowComponent;

UCLASS()
class PROCEDURALDUNGEON4_API AGridSpace : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGridSpace();

	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UStaticMeshComponent* GridPlaneMesh;

	TArray<FVector> GetEmptyNeighbors() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	USceneComponent* GridCenter;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UArrowComponent* FrontArrow;

	bool IsSpaceEmpty(const FVector& Location, float Radius) const;

	FVector FrontNeighbor;
	FVector RearNeighbor;
	FVector RightNeighbor;
	FVector LeftNeighbor;

	UPROPERTY()
	TArray<FVector> NeighborPositions;

};