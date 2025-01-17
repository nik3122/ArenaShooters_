// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/ASPlayerState.h"
#include "Net/UnrealNetwork.h"

AASPlayerState::AASPlayerState()
{
}

void AASPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AASPlayerState, KillCount);
	DOREPLIFETIME(AASPlayerState, DeathCount);
}

void AASPlayerState::SetPlayerName(const FString& S)
{
	Super::SetPlayerName(S);

	if (GetNetMode() == NM_DedicatedServer)
	{
		OnChangedPlayerName.Broadcast(GetPlayerName());
	}
}

void AASPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();

	OnChangedPlayerName.Broadcast(GetPlayerName());
}

void AASPlayerState::OnRep_PlayerId()
{
	Super::OnRep_PlayerId();

	OnChangedPlayerId.Broadcast(GetPlayerId());
}

int32 AASPlayerState::GetKillCount() const
{
	return KillCount;
}

void AASPlayerState::SetKillCount(int32 Count)
{
	KillCount = Count;

	OnChangedKillCount.Broadcast(KillCount);
}

void AASPlayerState::ModifyKillCount(int32 Count)
{
	SetKillCount(KillCount + Count);
}

int32 AASPlayerState::GetDeathCount() const
{
	return DeathCount;
}

void AASPlayerState::SetDeathCount(int32 Count)
{
	DeathCount = Count;

	OnChangedDeathCount.Broadcast(DeathCount);
}

void AASPlayerState::ModifyDeathCount(int32 Count)
{
	SetDeathCount(DeathCount + Count);
}

void AASPlayerState::OnKill()
{
	ModifyKillCount(1);
}

void AASPlayerState::OnDie()
{
	ModifyDeathCount(1);
}

void AASPlayerState::OnRep_KillCount()
{
	OnChangedKillCount.Broadcast(KillCount);
}

void AASPlayerState::OnRep_DeathCount()
{
	OnChangedDeathCount.Broadcast(DeathCount);
}
