// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/HTNBrainComponent.h"
#include "HTNBuilder.h"
#include "HTNPlanner.h"
#include "BasicHTNBrainComponent.generated.h"

UENUM(BlueprintType)
enum class EMinerWorldState : uint8
{
	HasOre,
	HasTool,
	CurrentLocation,
	CurrentNumOre,

	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EMinerTaskOperator : uint8
{
	Drop,
	PickUp,
	NavigateTo,
	Finish,

	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EMinerTaskOperatorParam : uint8
{
	Ore,
	Tool,
	Storage,
	ToolStorage,
	Mine,

	NONE,
	MAX UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FHTNResultAction
{
	GENERATED_USTRUCT_BODY()

	FHTNResultAction()
		: ActionID(255)
		, ActionParam(255)
	{}

	FHTNResultAction(uint8 InActionID, uint8 InActionParam)
		: ActionID(InActionID)
		, ActionParam(InActionParam)
	{}

	UPROPERTY(BlueprintReadOnly)
	uint8 ActionID;
	UPROPERTY(BlueprintReadOnly)
	uint8 ActionParam;
};


USTRUCT(BlueprintType)
struct FHTNWorldStateElem
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite)
		uint8 WorldState;
	UPROPERTY(BlueprintReadWrite)
		int32 Value;
};



UCLASS(BlueprintType, ClassGroup = AI, meta = (BlueprintSpawnableComponent))
class HTNPLANNERTRIAL_API UBasicHTNBrainComponent : public UHTNBrainComponent
{
	GENERATED_BODY()

private:
	FHTNBuilder_Domain MinerDomainBuilder;
	FHTNWorldState MinerWorldState;
	FHTNPlanner Planner;

public:
	void PopulateMinerDomain();

	UFUNCTION(BlueprintCallable, Category = "AI | HTN")
		void PopulateMinerWorldState(const TArray<FHTNWorldStateElem>& InitialWorldState);

	UFUNCTION(BlueprintCallable, Category = "AI | HTN")
		void MinerHTNPlanning(TArray<FHTNResultAction>& ResultActionSequence);

};
