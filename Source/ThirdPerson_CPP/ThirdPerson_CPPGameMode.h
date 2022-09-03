// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameModeBase.h"
#include "ThirdPerson_CPPGameMode.generated.h"

UCLASS(minimalapi)
class AThirdPerson_CPPGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AThirdPerson_CPPGameMode();

private:
	virtual void StartPlay() override;

public:
    /** �Ƴ���ǰ�˵��ؼ�������ָ���ࣨ���У����½��ؼ���*/
    UFUNCTION(BlueprintCallable, Category = "UMG Game")
        void ChangeMenuWidget(TSubclassOf<UUserWidget> NewWidgetClass, int32 ZOrder = 0);

protected:
    /** ��Ϸ��ʼʱ���á�*/
    //virtual void BeginPlay() override;

    /** ��Ϸ��ʼʱ�������˵��Ŀؼ��ࡣ��һ��ģ����Ϊ������Ϸ����ʱ����CurrWidget*/
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UMG Game")
        TSubclassOf<UUserWidget> StartingWidgetClass;

    /** �����˵��Ŀؼ�ʵ����*/
    UPROPERTY()
        UUserWidget* CurrentWidget;
};



