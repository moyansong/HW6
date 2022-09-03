// Fill out your copyright notice in the Description page of Project Settings.


#include "TwinblastCharacter.h"
//#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"
#include "Components/InputComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Projectile.h"
#include "Grenade.h"
#include "Weapon.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "UObject/ConstructorHelpers.h"
#include "AttributeComponent.h"
#include "WeaponComponent.h"
#include "Blueprint/UserWidget.h"
#include "WorldUserWidget.h"
#include "DrawDebugHelpers.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"

// Sets default values
ATwinblastCharacter::ATwinblastCharacter()
{
	// Set size for collision capsule��ײ���Ҵ�С
	//������Ƚŵͻ����������ڿ��У���Ϊ��������ǰ����
	GetCapsuleComponent()->InitCapsuleSize(42.f, 86.0f);

	//�ý�ɫÿ֡������Tick
	PrimaryActorTick.bCanEverTick = true;

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
	MaxWalkSpeed = 600.0f;
	MaxRunSpeed = 1200.0f;
	CrosshairSpread = 0.f;
	WeaponClassArray.Init(nullptr, 2);

	//��������Controllerת
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 0.0f); 
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MaxFlySpeed = 2000.f;//��ɫϸ���ﹴѡCanbeFly
	GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 300.0f;//Movement���ϸ���ﹴѡCanbeCrouch
	//GetCharacterMovement()->CrouchedHalfHeight = 10.0f;//����ʱ��������ӽǸ߶�
	//GetCharacterMovement()->SetUpdatedComponent(CollisionComponent);

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent); //SetupAttachment�ǰѸ������Ϊ��һ��������������,���Ÿ����һ���˶�
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->SocketOffset = FVector(0.0f, 80.0f, 65.0f);
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = true; // Camera rotate relative to arm

	AttributeComponent = CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComponent"));
	//WeaponComponent = CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComponent"));

	AudioComponent = CreateDefaultSubobject<UAudioComponent>("AudioComponent");
	AudioComponent->SetupAttachment(RootComponent);	
}

// Called when the game starts or when spawned
void ATwinblastCharacter::BeginPlay()
{
	Super::BeginPlay();

	ApplyWeaponChange();

	PlayAnimMontage(StartMontage, 1.0f);
	if (AttributeWidgetClass != nullptr)
	{
		AttributeWidget = CreateWidget<UUserWidget>(GetWorld(), AttributeWidgetClass);
		if (AttributeWidget != nullptr)
		{
			AttributeWidget->AddToViewport();
		}
	}
	if (MoblieControlWidgetClass != nullptr)
	{
		MoblieControlWidget = CreateWidget<UUserWidget>(GetWorld(), MoblieControlWidgetClass);
		if (MoblieControlWidget != nullptr)
		{
			MoblieControlWidget->AddToViewport();
		}
	}
	/*if (HealthBarWidgetClass != nullptr)
	{
		HealthBarWidget = CreateWidget<UWorldUserWidget>(this, HealthBarWidgetClass);
		if (HealthBarWidget != nullptr)
		{
			HealthBarWidget->AddToViewport();
		}
	}*/
}

// Called every frame
void ATwinblastCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//float v = GetVelocity().Size();
	CrosshairSpread = UKismetMathLibrary::FMin(CrosshairSpread + GetVelocity().Size() / 15.0f, 40.0f);
	if (CrosshairSpread > 0.0f) CrosshairSpread -= 1.0f;
	
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::FromInt(AttributeComponent->Health));
}

void ATwinblastCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	AttributeComponent->OnHealthChanged.AddDynamic(this, &ATwinblastCharacter::OnHealthChange);
}

// Called to bind functionality to input������
void ATwinblastCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	//"��һ������������Ŀ���õ�input�����õ�����(������ص��������ֲ�һ��)���ڶ������������õļ�λ�������func�ǻص�����"
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ATwinblastCharacter::JumpStart);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ATwinblastCharacter::JumpStop);

	PlayerInputComponent->BindAxis("MoveForward", this, &ATwinblastCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATwinblastCharacter::MoveRight);
	//PlayerInputComponent->BindAction("Run", IE_Pressed, this, &ATwinblastCharacter::ChangeWalkSpeed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ATwinblastCharacter::Crouching);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnAtRate", this, &ATwinblastCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpAtRate", this, &ATwinblastCharacter::LookUpAtRate);

	//�ֻ��Ĵ�����
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ATwinblastCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ATwinblastCharacter::TouchStopped);
	PlayerInputComponent->BindTouch(IE_Repeat, this, &ATwinblastCharacter::TouchUpdate);
	
	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ATwinblastCharacter::OnResetVR);

	//Fire
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ATwinblastCharacter::Fire);
	PlayerInputComponent->BindAction("Throw", IE_Pressed, this, &ATwinblastCharacter::ThrowStart);
	//PlayerInputComponent->BindAction("Throw", IE_Released, this, &ATwinblastCharacter::ThrowStop);

	PlayerInputComponent->BindAction("ChangeWeapon", IE_Pressed, this, &ATwinblastCharacter::ApplyWeaponChange);
}

void ATwinblastCharacter::OnResetVR()
{
	//UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

//FingerIndex���µ���ָ��
void ATwinblastCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void ATwinblastCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false; 
}

void ATwinblastCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
	{
		if (TouchItem.bIsPressed)
		{
			if (GetWorld() != nullptr)
			{
				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
				if (ViewportClient != nullptr)
				{
					FVector MoveDelta = Location - TouchItem.Location;
					FVector2D ScreenSize;
					ViewportClient->GetViewportSize(ScreenSize);
					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
					{
						TouchItem.bMoved = true;
						float Value = ScaledDelta.X * BaseTurnRate;
						AddControllerYawInput(Value);
					}
					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
					{
						TouchItem.bMoved = true;
						float Value = ScaledDelta.Y * BaseTurnRate;
						AddControllerPitchInput(Value);
					}
					TouchItem.Location = Location;
				}
				TouchItem.Location = Location;
			}
		}
	}
}

void ATwinblastCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	//����z��ת��������/��ת
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ATwinblastCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ATwinblastCharacter::MoveForward(float Value)
{
	//Value����input�����õ�scale
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward�ҵ�ǰ�������ǰ��
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);//��ת����ǰ�������ת����

		// get forward vector
		//FRotationMatrix(YawRotation)������z��ת����ǰ������ȡx����
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ATwinblastCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		//AddMovementInput(GetActorRightVector(), Value);//��ȡ�����ұߵ����������һֱ���žͻ�תȦ
		AddMovementInput(Direction, Value);//�����ValueҲֻ�ܽ���-1��1�ģ��ٴ�û��
	}
}

void ATwinblastCharacter::JumpStart()
{
	CrouchStop();
	Jump(); 
}

void ATwinblastCharacter::JumpStop()
{
	StopJumping();
}

void ATwinblastCharacter::ChangeWalkSpeed(float Speed)
{
	GetCharacterMovement()->MaxWalkSpeed = Speed;
}


void ATwinblastCharacter::Crouching()
{
	if (GetCharacterMovement()->IsCrouching())
	{
		CrouchStop();
	}
	else
	{
		CrouchStart();
	}
}

void ATwinblastCharacter::CrouchStart()
{
	if (GetCharacterMovement()->IsCrouching())
	{
		return;
	}
	Crouch();
	bIsCrouched = true;
}

void ATwinblastCharacter::CrouchStop()
{
	if (!GetCharacterMovement()->IsCrouching())
	{
		return;
	}
	UnCrouch();
	bIsCrouched = false;
}

void ATwinblastCharacter::Fire()
{
	if (Weapon == nullptr) return;

	CrosshairSpread = 40.0f;

	//����ʱ���ܿ�ǹ
	GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeed;

	//��ȡǹ��λ��
	FVector LeftGunLocation = GetMesh()->GetSocketLocation(FName(TEXT("weapon_forestock_bott_l")));
	FVector RefLocation = GetMesh()->GetBoneLocation(FName(TEXT("pelvis")));

	UWorld* World = GetWorld();
	if (World)
	{
		//���߼���ӵ��ô�����
		FVector Start = FollowCamera->GetComponentLocation();
		FVector End = FollowCamera->GetForwardVector() * Weapon->Range + Start;
		FVector HitLocation = End;
		FHitResult Hit; 
		bool bHit = World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility);//����׼�ǵ�����
		float Distance = UKismetMathLibrary::Vector_Distance(End, LeftGunLocation);
		
		if (bHit)
		{
			Distance = UKismetMathLibrary::Vector_Distance(Hit.Location, LeftGunLocation);
			HitLocation = Hit.Location;
		}
	
		//�����ٶȺ�ǹ��Zֵ��ƫ������ǹе�ĺ�����ϵ��ʩ�Ӻ�����
		HitLocation.Z += GetVelocity().Size() / 100.f * Weapon->RecoilForce;
		if (!bIsCrouched) HitLocation.Z += Distance / 10000.f * (LeftGunLocation.Z - RefLocation.Z);

		Weapon->Fire(HitLocation);
		PlayAnimMontage(Weapon->FireMontage, Weapon->FireMontagePlayRate);
	}
}

void ATwinblastCharacter::ThrowStart()
{
	float Duration = 0.f;
	if (GetVelocity().Size() < 1.f)
	{
		Duration = PlayAnimMontage(ThrowMontage_FullBody, 1.0f);
	}
	else
	{
		Duration = PlayAnimMontage(ThrowMontage_UpperBody, 1.0f);
	}

	//Ϊ�����������ɵ�ʱ������̫���׳���ʱ����ͬ���ڶ�����ͼ������
	/*FVector RightHandLocation = GetMesh()->GetSocketLocation(FName(TEXT("hand_r")));
	FVector CameraLocation;
	FRotator CameraRotation;
	GetActorEyesViewPoint(CameraLocation, CameraRotation);
	CameraRotation.Pitch += 50.f;
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	AGrenade* Grenade = GetWorld()->SpawnActor<AGrenade>(GrenadeClass, RightHandLocation, CameraRotation, SpawnParams);*/
}

void ATwinblastCharacter::ThrowStop()
{
	if (GetVelocity().Size() < 1.f) PlayAnimMontage(ThrowMontage_FullBody, 1.0f, FName("Prepare"));
	else PlayAnimMontage(ThrowMontage_UpperBody, 1.0f);
}

void ATwinblastCharacter::OnHealthChange(AActor* InstigatocActor, UAttributeComponent* OwningCopmonent, float NewHealth, float Damage)
{
	if (NewHealth <= 0.0f)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		//GetMesh()->SetAllBodiesSimulatePhysics(true);
		//GetMesh()->SetCollisionProfileName("Ragdoll");
		DisableInput(PC);//�������κ�����
	}
}

float ATwinblastCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (AttributeComponent)
	{
		AttributeComponent->ApplyHealthChange(Damage);
	}
	return Damage;
}

bool ATwinblastCharacter::AddWeapon(TSubclassOf<class AWeapon> NewWeaponClass)
{
	int32 PrevNum = WeaponClassArray.Num();
	WeaponClassArray.AddUnique(NewWeaponClass);
	return PrevNum == WeaponClassArray.Num();
}

void ATwinblastCharacter::ApplyWeaponChange()
{
	//WeaponComponent->ApplyWeaponChange();
	++CurrWeaponIndex;
	if (CurrWeaponIndex == WeaponClassArray.Num())
	{
		CurrWeaponIndex = 0;
	}

	if (Weapon)
	{
		Weapon->Destroy();
		Weapon = nullptr;
	}

	if (WeaponClassArray[CurrWeaponIndex] != nullptr)
	{
		Weapon = GetWorld()->SpawnActor<AWeapon>(WeaponClassArray[CurrWeaponIndex], FVector::ZeroVector, FRotator::ZeroRotator);
		Weapon->SetOwner(this);
		Weapon->CollisionComponent->BodyInstance.SetCollisionProfileName(TEXT("NoCollision"));
		//Mesh�����Ǹ�������ܰ�Socket��
		Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true), FName(TEXT("WeaponSocket")));
	}

	OnWeaponChange.Broadcast(this);
}
