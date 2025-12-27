
#include "BlackMythGameInstance.h"

void UBlackMythGameInstance::SetRespawnPoint(FVector Location, FRotator Rotation, FName TempleID)
{
    bHasRespawnPoint = true;
    RespawnLocation = Location;
    RespawnRotation = Rotation;
    RespawnTempleID = TempleID;
    
    UE_LOG(LogTemp, Log, TEXT("[GameInstance] Respawn point saved at Temple: %s, Location: %s"), 
        *TempleID.ToString(), *Location.ToString());
}
