// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class USphereComponent;
class URadialForceComponent;
class UProjectileMovementComponent;
class UParticleSystem;
class UPhysicalMaterial;
class USoundCue;
class UAnimSequence;
class UBlendSpace;
class UBlendSpace1D;
class AProjectile;
class UArrowComponent;

UCLASS()
class THIRDPERSON_CPP_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

//------------------------------------Component----------------------------------------------------------
	// ������ײ���
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Projectile)
		USphereComponent* CollisionComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Movement)
		UProjectileMovementComponent* MovementComponent;

	//�궨�ӵ����ɵ�λ�úͷ���,����ƫ��ȥ��ͼ���
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Movement)
		UArrowComponent* MuzzleComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Movement)
		UArrowComponent* ShellCaseComponent;

	//�������
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Material)
		UMaterialInstanceDynamic* MaterialInstance;

	//�������
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Mesh)
		USkeletalMeshComponent* SKMeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Mesh)
		UStaticMeshComponent* MeshComponent;

//--------------------------------------Animation--------------------------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationSequence")
		UAnimSequence* AS_Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationSequence")
		UAnimSequence* AS_CrouchIdle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationSequence")
		UAnimSequence* AS_JumpStart;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationSequence")
		UAnimSequence* AS_JumpFall;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationSequence")
		UAnimSequence* AS_JumpLand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationSequence")
		UAnimSequence* AS_JumpRecovery;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BlendSpace")
		UBlendSpace* BS_Move;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BlendSpace")
		UBlendSpace1D* BS_Stop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BlendSpace")
		UBlendSpace1D* BS_CrouchWalk;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
		UAnimMontage* FireMontage;

//--------------------------------------Sound--------------------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
		USoundCue* FireSound;

//--------------------------------------Projectile--------------------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Projectile)
		TSubclassOf<class AProjectile> ProjectileClass;

//-----------------------------------------ParticleSystem-----------------------------------------------------
	//��ǹ��
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Emitter")
		UParticleSystem* GunSpark;
	//����
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Emitter")
		UParticleSystem* ShellCase;

//--------------------------------------Function--------------------------------------------------------------------
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
		void Fire(FVector HitLocation);//HitLoction��ʾ�ӵ�Ҫ������
		void Fire_Implementation(FVector HitLocation);

	UFUNCTION(BlueprintCallable)
		void Test();

//--------------------------------------CallBack--------------------------------------------------------------------
	UFUNCTION()
		void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	//��ս������
	UFUNCTION()
		void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);
//--------------------------------------Parameter--------------------------------------------------------------------
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Parameter")
		float Range;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Parameter")
		float RecoilForce;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Parameter")
		float Damage;//��ս�����˺�

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Parameter")
		float FireMontagePlayRate;
};
