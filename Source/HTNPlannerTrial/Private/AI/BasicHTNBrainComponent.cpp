// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BasicHTNBrainComponent.h"

void UBasicHTNBrainComponent::PopulateMinerWorldState(const TArray<FHTNWorldStateElem>& InitialWorldState)
{
	// MinerWorldState���uEMinerWorldState::MAX�v�Ƀ��T�C�Y.
	MinerWorldState.Reinit(uint32(EMinerWorldState::MAX));

	// �S�Ă�WorldState��-1�ŏ�����.
	for (int32 WSIndex = 0; WSIndex < int32(EMinerWorldState::MAX); ++WSIndex)
	{
		MinerWorldState.SetValueUnsafe(WSIndex, -1);
	}

	//// �ǂ���̕��@�ł��l���Z�b�g�o����.
	//MinerWorldState.SetValueUnsafe(static_cast<uint8>(EMinerWorldState::HasOre), 1);
	//MinerWorldState.ApplyEffect(FHTNEffect(EMinerWorldState::HasOre, EHTNWorldStateOperation::Set).SetRHSAsValue(1));

	for (const auto& InitialState : InitialWorldState)
		MinerWorldState.SetValueUnsafe(InitialState.WorldState, InitialState.Value);
}

void UBasicHTNBrainComponent::PopulateMinerDomain()
{
	// ���[�g�ƂȂ�R���|�W�b�g�^�X�N���w�肷��.(��v���Ă��Ȃ���Όx�����o��)
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
			// �ǂ�Method�ɂ��Y�����Ȃ��ꍇ�̃^�X�N
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
		// Storage�ւ̈ړ��^�X�N.
		FHTNBuilder_PrimitiveTask& PrimitiveTaskBuilder = MinerDomainBuilder.AddPrimitiveTask(TEXT("NavigateToStorage"));
		// Storage�Ɉړ�.
		PrimitiveTaskBuilder.SetOperator(EMinerTaskOperator::NavigateTo, EMinerTaskOperatorParam::Storage);
		// CurrentLocation = Storage
		PrimitiveTaskBuilder.AddEffect(FHTNEffect(EMinerWorldState::CurrentLocation, EHTNWorldStateOperation::Set).SetRHSAsWSKey(EMinerTaskOperatorParam::Storage));
	}
	{
		// Mine�ւ̈ړ��^�X�N.
		FHTNBuilder_PrimitiveTask& PrimitiveTaskBuilder = MinerDomainBuilder.AddPrimitiveTask(TEXT("NavigateToMine"));
		// Mine�Ɉړ�.
		PrimitiveTaskBuilder.SetOperator(EMinerTaskOperator::NavigateTo, EMinerTaskOperatorParam::Mine);
		// CurrentLocation = Mine
		PrimitiveTaskBuilder.AddEffect(FHTNEffect(EMinerWorldState::CurrentLocation, EHTNWorldStateOperation::Set).SetRHSAsWSKey(EMinerTaskOperatorParam::Mine));
	}
	{
		// ToolStorage�ւ̈ړ��^�X�N.
		FHTNBuilder_PrimitiveTask& PrimitiveTaskBuilder = MinerDomainBuilder.AddPrimitiveTask(TEXT("NavigateToToolStorage"));
		// ToolStorage�Ɉړ�.
		PrimitiveTaskBuilder.SetOperator(EMinerTaskOperator::NavigateTo, EMinerTaskOperatorParam::ToolStorage);
		// CurrentLocation = ToolStorage
		PrimitiveTaskBuilder.AddEffect(FHTNEffect(EMinerWorldState::CurrentLocation, EHTNWorldStateOperation::Set).SetRHSAsWSKey(EMinerTaskOperatorParam::ToolStorage));
	}
	{
		// Ore(�z��)���擾����^�X�N.
		FHTNBuilder_PrimitiveTask& PrimitiveTaskBuilder = MinerDomainBuilder.AddPrimitiveTask(TEXT("PickupOre"));
		// Ore���擾.
		PrimitiveTaskBuilder.SetOperator(EMinerTaskOperator::PickUp, EMinerTaskOperatorParam::Ore);
		// HasOre = true.
		PrimitiveTaskBuilder.AddEffect(FHTNEffect(EMinerWorldState::HasOre, EHTNWorldStateOperation::Set).SetRHSAsValue(1));
	}
	{
		// Tool(�̌@����)���擾����^�X�N.
		FHTNBuilder_PrimitiveTask& PrimitiveTaskBuilder = MinerDomainBuilder.AddPrimitiveTask(TEXT("PickupTool"));
		// Tool���擾����.
		PrimitiveTaskBuilder.SetOperator(EMinerTaskOperator::PickUp, EMinerTaskOperatorParam::Tool);
		// HasTool = true.
		PrimitiveTaskBuilder.AddEffect(FHTNEffect(EMinerWorldState::HasTool, EHTNWorldStateOperation::Set).SetRHSAsValue(1));
	}
	{
		// Ore��u���^�X�N.
		FHTNBuilder_PrimitiveTask& PrimitiveTaskBuilder = MinerDomainBuilder.AddPrimitiveTask(TEXT("DropOre"));
		// Ore��u��.
		PrimitiveTaskBuilder.SetOperator(EMinerTaskOperator::Drop, EMinerTaskOperatorParam::Ore);
		// HasOre = false
		PrimitiveTaskBuilder.AddEffect(FHTNEffect(EMinerWorldState::HasOre, EHTNWorldStateOperation::Set).SetRHSAsValue(0));
		// CurrentNumOre += 1
		PrimitiveTaskBuilder.AddEffect(FHTNEffect(EMinerWorldState::CurrentNumOre, EHTNWorldStateOperation::Increase).SetRHSAsValue(1));
	}
	{
		// �������Ȃ��^�X�N.
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
	* �R���|�W�b�g�^�X�N���Ăяo���^�X�N���h���C���ɂȂ��ꍇ�̌x��
	* �uTask [���݂��Ȃ��^�X�N��] of [�R���|�W�b�g�^�X�N��] not found! Make sure it's defined in your domain. Domain compilation aborted.�v
	* 
	* SetRootName�Ŏw�肵�����O�ƁA�ŏ��̃R���|�W�b�g�^�X�N�̖��O����v���Ă��Ȃ��ꍇ�̌x��
	* Unable to find root task under the name [SetRootName�Ŏw�肵�����O]. Falling back to the first compound task, or first primitive task
	* 
	* ���[�v����悤�ȃh���C���͍\�z���Ȃ��ق����ǂ�����.(�J��Ԃ��n��HTN�̊O�Ŕ��f.�w��񐔖����ł���΍Ă�HTN�����s.�����łȂ���Ύ��s���Ȃ�.)
	* 
	* �I����\���^�X�N������Ƃ��������H
	* 
	* HTNCondition���S�Ė����Ȃ����A��̃^�X�N���ς܂�邱�Ƃ͂Ȃ��B(���[�v����h���C����g�ނƂ��͗v����.�������[�v�������h���C����T���������Ă��܂�)
	* 
	* FHTNResult��TaskIDs��ActionSequence�̒��g�̏��Ԃ͓���B���������e�͈قȂ�ATaskIDs�͕����ʂ���s���鏇�Ƀ^�X�N��ID(�h���C���R���p�C�����Ɋ��蓖�Ă�ꂽ����)���i�[����Ă���AActionSequence�͎��s�����Operator��ID�Ƃ��̃p�����[�^ID���i�[����Ă���.
	* 
	* ���[�U�[�����ׂ����ƁFHTN�h���C���\�z�A���܂����A�N�V�����V�[�P���X�����Ɏ��o�����s����d�g��
	* HTN Planner�v���O�C������邱�ƁFWorldState�Ɋ�Â��A�N�V�����V�[�P���X�����߂�
	*/
}
