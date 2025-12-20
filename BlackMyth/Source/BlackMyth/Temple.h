// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractInterface.h"
#include "Temple.generated.h"

class UStaticMeshComponent;
class USphereComponent;

UCLASS()
class BLACKMYTH_API AInteractableActor : public AActor, public IInteractInterface
{
    GENERATED_BODY()

public:
    AInteractableActor();

protected:
    virtual void BeginPlay() override;

public:
    // Mesh
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* Mesh;

    // 互动触发
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

    // 玩家靠近时显示Widget
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<class UUserWidget> TempleWidgetClass;

    UPROPERTY()
    UUserWidget* InteractWidgetInstance;

    // 交互菜单Widget
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<class UUserWidget> InteractMenuWidgetClass;

    UPROPERTY()
    UUserWidget* InteractMenuInstance;

    // 当玩家进入/离开触发器
    UFUNCTION()
    void OnPlayerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // 交互逻辑
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void DoInteract();

    // 接口实现
    UFUNCTION()
    void OnInteract_Implementation(AActor* Interactor);
};
