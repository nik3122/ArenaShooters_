// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ASMatchGameStateBase.h"
#include "GameMode/ASItemFactoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "ASGameInstance.h"
#include "Controller/ASPlayerState.h"

AASMatchGameStateBase::AASMatchGameStateBase()
{
	ItemFactory = CreateDefaultSubobject<UASItemFactoryComponent>(TEXT("ItemFactory"));
}

void AASMatchGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AASMatchGameStateBase, ItemFactory);
	DOREPLIFETIME(AASMatchGameStateBase, MaxNumPlayers);
	DOREPLIFETIME(AASMatchGameStateBase, StartTimeForProcess);
	DOREPLIFETIME(AASMatchGameStateBase, GoalNumOfKills);
	DOREPLIFETIME(AASMatchGameStateBase, InnerMatchState);
	DOREPLIFETIME(AASMatchGameStateBase, MatchFinishTime);
}

void AASMatchGameStateBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	auto GameInst = GetGameInstance<UASGameInstance>();
	if (IsValid(GameInst))
	{
		SetInnerMatchState(GameInst->GetInnerMatchState());
	}
	else
	{
		AS_LOG_S(Error);
	}
}

void AASMatchGameStateBase::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	OnChangedNumPlayers.Broadcast(PlayerArray.Num());
}

void AASMatchGameStateBase::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	OnChangedNumPlayers.Broadcast(PlayerArray.Num());
}

UASItemFactoryComponent* AASMatchGameStateBase::GetItemFactory()
{
	return ItemFactory;
}

int32 AASMatchGameStateBase::GetMaxNumPlayer() const
{
	return MaxNumPlayers;
}

void AASMatchGameStateBase::SetMaxNumPlayers(int32 Num)
{
	MaxNumPlayers = Num;
}

int32 AASMatchGameStateBase::GetGoalNumOfKills() const
{
	return GoalNumOfKills;
}

void AASMatchGameStateBase::SetGoalNumOfKills(int32 Num)
{
	GoalNumOfKills = Num;
}

void AASMatchGameStateBase::SetStartTimeForProcess(float StartTime)
{
	StartTimeForProcess = StartTime;

	OnStartTimeForProcess.Broadcast(StartTimeForProcess);
}

EInnerMatchState AASMatchGameStateBase::GetInnerMatchState() const
{
	return InnerMatchState;
}

void AASMatchGameStateBase::SetInnerMatchState(EInnerMatchState State)
{
	InnerMatchState = State;

	OnChangedInnerMatchState.Broadcast(InnerMatchState);
}

bool AASMatchGameStateBase::IsMatchProcess() const
{
	return InnerMatchState == EInnerMatchState::Process;
}

void AASMatchGameStateBase::MulticastOnKill_Implementation(AASPlayerState* KillerPlayerState, AASPlayerState* DeadPlayerState)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		// server

	}
	else
	{
		// client

		// todo: Kill log
	}
}

void AASMatchGameStateBase::OnFinishMatch()
{
	auto GameInst = GetGameInstance<UASGameInstance>();
	if (IsValid(GameInst))
	{
		GameInst->SetInnerMatchState(EInnerMatchState::Finish);
	}
	else
	{
		AS_LOG_S(Error);
	}

	SetInnerMatchState(EInnerMatchState::Finish);
}

FDateTime AASMatchGameStateBase::GetMatchFinishTime() const
{
	return MatchFinishTime;
}

void AASMatchGameStateBase::SetMatchFinishTime(float FinishTime)
{
	MatchFinishTime = FinishTime;

	OnSetMatchFinishTime.Broadcast(MatchFinishTime);
}

void AASMatchGameStateBase::OnRep_StartTimeForProcess()
{
	OnStartTimeForProcess.Broadcast(StartTimeForProcess);
}

void AASMatchGameStateBase::OnRep_InnerMatchState()
{
	OnChangedInnerMatchState.Broadcast(InnerMatchState);
}

void AASMatchGameStateBase::OnRep_MatchFinishTime()
{
	OnSetMatchFinishTime.Broadcast(MatchFinishTime);
}

AASPlayerState* AASMatchGameStateBase::GetPlayerStateOfTopKillCount() const
{
	AASPlayerState* TopPlayer = nullptr;
	int32 TopNumOfKills = 0;
	for (auto& PlayerState : PlayerArray)
	{
		auto ASPlayerState = Cast<AASPlayerState>(PlayerState);
		if (!IsValid(ASPlayerState))
			continue;

		int32 KillCount = ASPlayerState->GetKillCount();
		if (KillCount > TopNumOfKills)
		{
			TopPlayer = ASPlayerState;
			TopNumOfKills = KillCount;
		}
	}

	return TopPlayer;
}
