// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BasicHTNBrainComponent.h"

void UBasicHTNBrainComponent::PopulateMinerWorldState(const TArray<FHTNWorldStateElem>& InitialWorldState)
{
	// MinerWorldStateを「EMinerWorldState::MAX」にリサイズ.
	MinerWorldState.Reinit(uint32(EMinerWorldState::MAX));

	// 全てのWorldStateを-1で初期化.
	for (int32 WSIndex = 0; WSIndex < int32(EMinerWorldState::MAX); ++WSIndex)
	{
		MinerWorldState.SetValueUnsafe(WSIndex, -1);
	}

	//// どちらの方法でも値をセット出来る.
	//MinerWorldState.SetValueUnsafe(static_cast<uint8>(EMinerWorldState::HasOre), 1);
	//MinerWorldState.ApplyEffect(FHTNEffect(EMinerWorldState::HasOre, EHTNWorldStateOperation::Set).SetRHSAsValue(1));

	for (const auto& InitialState : InitialWorldState)
		MinerWorldState.SetValueUnsafe(InitialState.WorldState, InitialState.Value);
}

void UBasicHTNBrainComponent::PopulateMinerDomain()
{
	// ルートとなるコンポジットタスクを指定する.(一致していなければ警告が出る)
	MinerDomainBuilder.SetRootName(TEXT("CarryOre"));
	{
		FHTNBuilder_CompositeTask& CompositeTaskBuilder = MinerDomainBuilder.AddCompositeTask(TEXT("CarryOre"));
		{
			FHTNBuilder_Method& MethodsBuilder = CompositeTaskBuilder.AddMethod(
				TArray<FHTNCondition>({
					// HasOre == TRUE
					FHTNCondition(EMinerWorldState::HasOre, EHTNWorldStateCheck::IsTrue),
					// CurrentNumOre < 3
					FHTNCondition(EMinerWorldState::CurrentNumOre, EHTNWorldStateCheck::Less).SetRHSAsValue(3)
				})
			);
			MethodsBuilder.AddTask(TEXT("NavigateToStorage"));
			MethodsBuilder.AddTask(TEXT("DropOre"));
			MethodsBuilder.AddTask(TEXT("CarryOre"));
		}
		{
			FHTNBuilder_Method& MethodsBuilder = CompositeTaskBuilder.AddMethod(
				TArray<FHTNCondition>({
					// CurrentNumOre < 3
					FHTNCondition(EMinerWorldState::CurrentNumOre, EHTNWorldStateCheck::Less).SetRHSAsValue(3)
				})
			);
			MethodsBuilder.AddTask(TEXT("FindOre"));
		}
		{
			// どのMethodにも該当しない場合のタスク
			FHTNBuilder_Method& MethodsBuilder = CompositeTaskBuilder.AddMethod();
			MethodsBuilder.AddTask(TEXT("DoNothingTask"));
		}
	}

	{
		FHTNBuilder_CompositeTask& CompositeTaskBuilder = MinerDomainBuilder.AddCompositeTask(TEXT("FindOre"));
		{
			FHTNBuilder_Method& MethodsBuilder = CompositeTaskBuilder.AddMethod(
				TArray<FHTNCondition>({
					// HasTool == TRUE
					FHTNCondition(EMinerWorldState::HasTool, EHTNWorldStateCheck::IsTrue)
				}));
			MethodsBuilder.AddTask(TEXT("NavigateToMine"));
			MethodsBuilder.AddTask(TEXT("PickupOre"));
			MethodsBuilder.AddTask(TEXT("CarryOre"));
		}
		{
			FHTNBuilder_Method& MethodsBuilder = CompositeTaskBuilder.AddMethod();
			MethodsBuilder.AddTask(TEXT("NavigateToToolStorage"));
			MethodsBuilder.AddTask(TEXT("PickupTool"));
			MethodsBuilder.AddTask(TEXT("FindOre"));
		}
	}

	{
		// Storageへの移動タスク.
		FHTNBuilder_PrimitiveTask& PrimitiveTaskBuilder = MinerDomainBuilder.AddPrimitiveTask(TEXT("NavigateToStorage"));
		// Storageに移動.
		PrimitiveTaskBuilder.SetOperator(EMinerTaskOperator::NavigateTo, EMinerTaskOperatorParam::Storage);
		// CurrentLocation = Storage
		PrimitiveTaskBuilder.AddEffect(FHTNEffect(EMinerWorldState::CurrentLocation, EHTNWorldStateOperation::Set).SetRHSAsWSKey(EMinerTaskOperatorParam::Storage));
	}
	{
		// Mineへの移動タスク.
		FHTNBuilder_PrimitiveTask& PrimitiveTaskBuilder = MinerDomainBuilder.AddPrimitiveTask(TEXT("NavigateToMine"));
		// Mineに移動.
		PrimitiveTaskBuilder.SetOperator(EMinerTaskOperator::NavigateTo, EMinerTaskOperatorParam::Mine);
		// CurrentLocation = Mine
		PrimitiveTaskBuilder.AddEffect(FHTNEffect(EMinerWorldState::CurrentLocation, EHTNWorldStateOperation::Set).SetRHSAsWSKey(EMinerTaskOperatorParam::Mine));
	}
	{
		// ToolStorageへの移動タスク.
		FHTNBuilder_PrimitiveTask& PrimitiveTaskBuilder = MinerDomainBuilder.AddPrimitiveTask(TEXT("NavigateToToolStorage"));
		// ToolStorageに移動.
		PrimitiveTaskBuilder.SetOperator(EMinerTaskOperator::NavigateTo, EMinerTaskOperatorParam::ToolStorage);
		// CurrentLocation = ToolStorage
		PrimitiveTaskBuilder.AddEffect(FHTNEffect(EMinerWorldState::CurrentLocation, EHTNWorldStateOperation::Set).SetRHSAsWSKey(EMinerTaskOperatorParam::ToolStorage));
	}
	{
		// Ore(鉱石)を取得するタスク.
		FHTNBuilder_PrimitiveTask& PrimitiveTaskBuilder = MinerDomainBuilder.AddPrimitiveTask(TEXT("PickupOre"));
		// Oreを取得.
		PrimitiveTaskBuilder.SetOperator(EMinerTaskOperator::PickUp, EMinerTaskOperatorParam::Ore);
		// HasOre = true.
		PrimitiveTaskBuilder.AddEffect(FHTNEffect(EMinerWorldState::HasOre, EHTNWorldStateOperation::Set).SetRHSAsValue(1));
	}
	{
		// Tool(採掘道具)を取得するタスク.
		FHTNBuilder_PrimitiveTask& PrimitiveTaskBuilder = MinerDomainBuilder.AddPrimitiveTask(TEXT("PickupTool"));
		// Toolを取得する.
		PrimitiveTaskBuilder.SetOperator(EMinerTaskOperator::PickUp, EMinerTaskOperatorParam::Tool);
		// HasTool = true.
		PrimitiveTaskBuilder.AddEffect(FHTNEffect(EMinerWorldState::HasTool, EHTNWorldStateOperation::Set).SetRHSAsValue(1));
	}
	{
		// Oreを置くタスク.
		FHTNBuilder_PrimitiveTask& PrimitiveTaskBuilder = MinerDomainBuilder.AddPrimitiveTask(TEXT("DropOre"));
		// Oreを置く.
		PrimitiveTaskBuilder.SetOperator(EMinerTaskOperator::Drop, EMinerTaskOperatorParam::Ore);
		// HasOre = false
		PrimitiveTaskBuilder.AddEffect(FHTNEffect(EMinerWorldState::HasOre, EHTNWorldStateOperation::Set).SetRHSAsValue(0));
		// CurrentNumOre += 1
		PrimitiveTaskBuilder.AddEffect(FHTNEffect(EMinerWorldState::CurrentNumOre, EHTNWorldStateOperation::Increase).SetRHSAsValue(1));
	}
	{
		// 何もしないタスク.
		FHTNBuilder_PrimitiveTask& PrimitiveTaskBuilder = MinerDomainBuilder.AddPrimitiveTask(TEXT("DoNothingTask"));
		PrimitiveTaskBuilder.SetOperator(EMinerTaskOperator::Finish, EMinerTaskOperatorParam::NONE);
	}


	MinerDomainBuilder.Compile();

}

void UBasicHTNBrainComponent::MinerHTNPlanning(TArray<FHTNResultAction>& ResultActionSequence)
{
	PopulateMinerDomain();

	FHTNResult Result;
	Planner.GeneratePlan(*(MinerDomainBuilder.DomainInstance), MinerWorldState, Result);

	ResultActionSequence.Reserve(Result.ActionsSequence.Num());

	UE_LOG(LogTemp, Error, TEXT("====================== Action Sequence ======================"));
	FString ActionName, ParamName;
	for (int32 SequenceIndex = 0; SequenceIndex < Result.ActionsSequence.Num(); ++SequenceIndex)
	{
		uint8 ActionID = Result.ActionsSequence[SequenceIndex].ActionID;
		uint8 ActionParam = Result.ActionsSequence[SequenceIndex].Parameter;
		ResultActionSequence.Add(FHTNResultAction(ActionID, ActionParam));

		ActionName = UEnum::GetValueAsString(static_cast<EMinerTaskOperator>(ActionID));
		ParamName = UEnum::GetValueAsString(static_cast<EMinerTaskOperatorParam>(ActionParam));
		UE_LOG(LogTemp, Error, TEXT("ActionID : %s,    ActionParam : %s"), *ActionName, *ParamName);
	}
	UE_LOG(LogTemp, Error, TEXT("=============================================================="));




	/*
	* コンポジットタスクが呼び出すタスクがドメインにない場合の警告
	* 「Task [存在しないタスク名] of [コンポジットタスク名] not found! Make sure it's defined in your domain. Domain compilation aborted.」
	* 
	* SetRootNameで指定した名前と、最初のコンポジットタスクの名前が一致していない場合の警告
	* Unable to find root task under the name [SetRootNameで指定した名前]. Falling back to the first compound task, or first primitive task
	* 
	* ループするようなドメインは構築しないほうが良いかも.(繰り返す系はHTNの外で判断.指定回数未満であれば再びHTNを実行.そうでなければ実行しない.)
	* 
	* 終わりを表すタスクがあるといいかも？
	* 
	* HTNConditionが全て満たない時、空のタスクが積まれることはない。(ループするドメインを組むときは要注意.無限ループが生じドメインを探索し続けてしまう)
	* 
	* FHTNResultのTaskIDsとActionSequenceの中身の順番は同一。ただし内容は異なり、TaskIDsは文字通り実行する順にタスクのID(ドメインコンパイル時に割り当てられたもの)が格納されており、ActionSequenceは実行されるOperatorのIDとそのパラメータIDが格納されている.
	* 
	* ユーザーがやるべきこと：HTNドメイン構築、求まったアクションシーケンスを順に取り出し実行する仕組み
	* HTN Plannerプラグインがやること：WorldStateに基づきアクションシーケンスを求める
	*/
}
