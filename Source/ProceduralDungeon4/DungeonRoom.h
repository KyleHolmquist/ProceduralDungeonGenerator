// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonRoom.generated.h"

class AGridSpace;

UCLASS()
class PROCEDURALDUNGEON4_API ADungeonRoom : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADungeonRoom();

	//How many tiles this room will be made with
	UPROPERTY(BlueprintReadOnly, Category="Room")
	int32 NumTiles = 0;

	//References to the spawned tiles
	UPROPERTY(VisibleAnywhere, Category="Room")
	TArray<TObjectPtr<AGridSpace>> Tiles;

	UPROPERTY(EditAnywhere, Category = "Room")
	UMaterialInterface* EdgeMaterial;

	//Call right after spawn to set basic info
	void Initialize(int32 InNumTiles);

	//Add a tile to this room
	void AddTile(AGridSpace* Tile);

	//Average center of room of tiles
	FVector GetCenter() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY()
	TObjectPtr<USceneComponent> Root;

	void FindEdges();

private:

	bool FoundEdges = false;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
