#pragma once

#include "FortPlayerController.h"
#include "FortPlayerStateAthena.h"
#include "FortPlayerPawn.h"
#include "SoftObjectPtr.h"
#include "FortKismetLibrary.h"
#include "AthenaMarkerComponent.h"
#include "FortVolume.h"

static void ApplyHID(AFortPlayerPawn* Pawn, UObject* HeroDefinition)
{
	using UFortHeroSpecialization = UObject;

	static auto SpecializationsOffset = HeroDefinition->GetOffset("Specializations");
	auto& Specializations = HeroDefinition->Get<TArray<TSoftObjectPtr<UFortHeroSpecialization>>>(SpecializationsOffset);

	auto PlayerState = Pawn->GetPlayerState();

	for (int i = 0; i < Specializations.Num(); i++)
	{
		auto& SpecializationSoft = Specializations.at(i);

		static auto FortHeroSpecializationClass = FindObject<UClass>("/Script/FortniteGame.FortHeroSpecialization");
		auto Specialization = SpecializationSoft.Get(FortHeroSpecializationClass, true);

		if (Specialization)
		{
			static auto Specialization_CharacterPartsOffset = Specialization->GetOffset("CharacterParts");
			auto& CharacterParts = Specialization->Get<TArray<TSoftObjectPtr<UObject>>>(Specialization_CharacterPartsOffset);

			static auto CustomCharacterPartClass = FindObject<UClass>("/Script/FortniteGame.CustomCharacterPart");

			/* if (bUseServerChoosePart)
			{
				for (int z = 0; z < CharacterParts.Num(); z++)
				{
					Pawn->ServerChoosePart((EFortCustomPartType)z, CharacterParts.at(z).Get(CustomCharacterPartClass, true));
				}

				continue; // hm?
			} */

			bool aa;

			TArray<UObject*> CharacterPartsaa;

			for (int z = 0; z < CharacterParts.Num(); z++)
			{
				auto& CharacterPartSoft = CharacterParts.at(z, GetSoftObjectSize());
				auto CharacterPart = CharacterPartSoft.Get(CustomCharacterPartClass, true);

				CharacterPartsaa.Add(CharacterPart);

				continue;
			}

			UFortKismetLibrary::ApplyCharacterCosmetics(GetWorld(), CharacterPartsaa, PlayerState, &aa);
			CharacterPartsaa.Free();
		}
	}
}

static bool ApplyCID(AFortPlayerPawn* Pawn, UObject* CID, bool bUseServerChoosePart = false)
{
	if (!CID)
		return false;

	auto PlayerController = Cast<AFortPlayerController>(Pawn->GetController());

	if (!PlayerController)
		return false;

	/* auto PCCosmeticLoadout = PlayerController->GetCosmeticLoadout();

	if (!PCCosmeticLoadout)
	{
		LOG_INFO(LogCosmetics, "PCCosmeticLoadout is not set! Will not be able to apply skin.");
		return false;
	}

	auto PawnCosmeticLoadout = PlayerController->GetCosmeticLoadout();

	if (!PawnCosmeticLoadout)
	{
		LOG_INFO(LogCosmetics, "PawnCosmeticLoadout is not set! Will not be able to apply skin.");
		return false;
	}

	PCCosmeticLoadout->GetCharacter() = CID;
	PawnCosmeticLoadout->GetCharacter() = CID;
	PlayerController->ApplyCosmeticLoadout(); // would cause recursive

	return true; */

	if (Fortnite_Version == 1.72)
		return false;

	static auto HeroDefinitionOffset = CID->GetOffset("HeroDefinition");
	auto HeroDefinition = CID->Get(HeroDefinitionOffset);

	ApplyHID(Pawn, HeroDefinition);

	// static auto HeroTypeOffset = PlayerState->GetOffset("HeroType");
	// PlayerState->Get(HeroTypeOffset) = HeroDefinition;

	return true;
}

struct FGhostModeRepData
{
	bool& IsInGhostMode()
	{
		static auto bInGhostModeOffset = FindOffsetStruct("/Script/FortniteGame.GhostModeRepData", "bInGhostMode");
		return *(bool*)(__int64(this) + bInGhostModeOffset);
	}

	UFortWorldItemDefinition*& GetGhostModeItemDef()
	{
		static auto GhostModeItemDefOffset = FindOffsetStruct("/Script/FortniteGame.GhostModeRepData", "GhostModeItemDef");
		return *(UFortWorldItemDefinition**)(__int64(this) + GhostModeItemDefOffset);
	}
};

class AFortPlayerControllerAthena : public AFortPlayerController
{
public:
	static inline void (*GetPlayerViewPointOriginal)(AFortPlayerControllerAthena* PlayerController, FVector& Location, FRotator& Rotation);
	static inline void (*ServerReadyToStartMatchOriginal)(AFortPlayerControllerAthena* PlayerController);
	static inline void (*ServerRequestSeatChangeOriginal)(AFortPlayerControllerAthena* PlayerController, int TargetSeatIndex);
	static inline void (*EnterAircraftOriginal)(UObject* PC, AActor* Aircraft);
	static inline void (*StartGhostModeOriginal)(UObject* Context, FFrame* Stack, void* Ret);
	static inline void (*EndGhostModeOriginal)(AFortPlayerControllerAthena* PlayerController);

	class UAthenaResurrectionComponent*& GetResurrectionComponent()
	{
		static auto ResurrectionComponentOffset = GetOffset("ResurrectionComponent");
		return Get<class UAthenaResurrectionComponent*>(ResurrectionComponentOffset);
	}

	AFortPlayerStateAthena* GetPlayerStateAthena()
	{
		return (AFortPlayerStateAthena*)GetPlayerState();
	}

	FGhostModeRepData* GetGhostModeRepData()
	{
		static auto GhostModeRepDataOffset = GetOffset("GhostModeRepData", false);

		if (GhostModeRepDataOffset == -1)
			return nullptr;

		return GetPtr<FGhostModeRepData>(GhostModeRepDataOffset);
	}

	bool IsInGhostMode()
	{
		auto GhostModeRepData = GetGhostModeRepData();

		if (!GhostModeRepData)
			return false;

		return GhostModeRepData->IsInGhostMode();
	}

	UAthenaMarkerComponent* GetMarkerComponent()
	{
		static auto MarkerComponentOffset = GetOffset("MarkerComponent");
		return Get<UAthenaMarkerComponent*>(MarkerComponentOffset);
	}

	AFortVolume*& GetCreativePlotLinkedVolume()
	{
		static auto CreativePlotLinkedVolumeOffset = GetOffset("CreativePlotLinkedVolume");
		return Get<AFortVolume*>(CreativePlotLinkedVolumeOffset);
	}

	static void StartGhostModeHook(UObject* Context, FFrame* Stack, void* Ret); // we could native hook this but eh
	static void EndGhostModeHook(AFortPlayerControllerAthena* PlayerController);
	static void EnterAircraftHook(UObject* PC, AActor* Aircraft);
	static void ServerRequestSeatChangeHook(AFortPlayerControllerAthena* PlayerController, int TargetSeatIndex); // actually in zone
	static void ServerRestartPlayerHook(AFortPlayerControllerAthena* Controller);
	static void ServerGiveCreativeItemHook(AFortPlayerControllerAthena* Controller, FFortItemEntry CreativeItem);
	static void ServerTeleportToPlaygroundLobbyIslandHook(AFortPlayerControllerAthena* Controller);
	static void ServerAcknowledgePossessionHook(APlayerController* Controller, APawn* Pawn);
	static void ServerPlaySquadQuickChatMessageHook(AFortPlayerControllerAthena* PlayerController, __int64 ChatEntry, __int64 SenderID);
	static void GetPlayerViewPointHook(AFortPlayerControllerAthena* PlayerController, FVector& Location, FRotator& Rotation);
	static void ServerReadyToStartMatchHook(AFortPlayerControllerAthena* PlayerController);
	static void UpdateTrackedAttributesHook(AFortPlayerControllerAthena* PlayerController);

	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>(L"/Script/FortniteGame.FortPlayerControllerAthena");
		return Class;
	}
};