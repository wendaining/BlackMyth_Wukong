// Fill out your copyright notice in the Description page of Project Settings.

#include "Temple.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Blueprint/UserWidget.h"
#include "WukongCharacter.h"
#include "Kismet/GameplayStatics.h"

AInteractableActor::AInteractableActor()
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;

    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComponent);
    InteractionSphere->SetSphereRadius(200.f);
    InteractionSphere->SetCollisionProfileName(TEXT("Trigger"));
}

void AInteractableActor::BeginPlay()
{
    Super::BeginPlay();

    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AInteractableActor::OnPlayerEnter);
    InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AInteractableActor::OnPlayerExit);
}

void AInteractableActor::OnPlayerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA(AWukongCharacter::StaticClass()))
    {
        if (TempleWidgetClass)
        {
            InteractWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), TempleWidgetClass);
            if (InteractWidgetInstance)
            {
                InteractWidgetInstance->AddToViewport();
            }
        }

        AWukongCharacter* Player = Cast<AWukongCharacter>(OtherActor);
        if (Player)
        {
            Player->CurrentInteractable = this;
        }
    }
}

void AInteractableActor::OnPlayerExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor->IsA(AWukongCharacter::StaticClass()))
    {
        if (InteractWidgetInstance)
        {
            InteractWidgetInstance->RemoveFromParent();
            InteractWidgetInstance = nullptr;
        }

        AWukongCharacter* Player = Cast<AWukongCharacter>(OtherActor);
        if (Player && Player->CurrentInteractable == this)
        {
            Player->CurrentInteractable = nullptr;
        }
    }
}

void AInteractableActor::DoInteract()
{
    UE_LOG(LogTemp, Warning, TEXT("Interacted with %s"), *GetName());
}

void AInteractableActor::OnInteract_Implementation(AActor* Interactor)
{
    UE_LOG(LogTemp, Log, TEXT("Temple Interacted"));
}