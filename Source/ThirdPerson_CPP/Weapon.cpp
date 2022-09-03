// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/ArrowComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Particles/ParticleSystem.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AttributeComponent.h"
#include "Containers/Array.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Sound/SoundCue.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "Projectile.h"
#include "TwinblastCharacter.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    Range = 10000000.f;
    RecoilForce = 1.f;
    Damage = 50.f;
    FireMontagePlayRate = 1.f;

    if (!RootComponent)
    {
        RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
    }

    //MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
    //RootComponent = MeshComponent;

    SKMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
    SKMeshComponent->SetupAttachment(RootComponent);
    //RootComponent = SKMeshComponent;
  

    if (!CollisionComponent)
    {
        CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
        CollisionComponent->InitSphereRadius(30.0f);
        CollisionComponent->BodyInstance.SetCollisionProfileName(TEXT("Projectile"));
        CollisionComponent->SetupAttachment(RootComponent);
        CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnOverlap);
        CollisionComponent->OnComponentHit.AddDynamic(this, &AWeapon::OnHit);
        //RootComponent = CollisionComponent;
    }

    if (!MovementComponent)
    {
        MovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComponent"));
        MovementComponent->SetUpdatedComponent(RootComponent);
        MovementComponent->InitialSpeed = 0.f;
        MovementComponent->MaxSpeed = 0.0f;
        MovementComponent->bRotationFollowsVelocity = false;
        MovementComponent->bShouldBounce = false;
        //MovementComponent->Bounciness = 0.f;
        MovementComponent->ProjectileGravityScale = 0.f;
    }

    MuzzleComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("MuzzleComponent"));
    MuzzleComponent->SetupAttachment(RootComponent);
    ShellCaseComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ShellCaseComponent"));
    ShellCaseComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::Fire_Implementation(FVector HitLocation)
{
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = Cast<APawn>(GetOwner());

    UWorld* World = GetWorld();

    FVector ProjectileLocation = MuzzleComponent->GetComponentLocation();
    FRotator ProjectileRotation = (HitLocation - ProjectileLocation).Rotation();

    if (ProjectileClass)
    {
        AProjectile* Projectile = World->SpawnActor<AProjectile>(ProjectileClass, ProjectileLocation, ProjectileRotation, SpawnParams);
    }
    if (GunSpark)
    {
        UGameplayStatics::SpawnEmitterAtLocation(World, GunSpark, ProjectileLocation, ProjectileRotation, 1.0f);
    }
    if (ShellCase)
    {
        UGameplayStatics::SpawnEmitterAtLocation(World, ShellCase, ShellCaseComponent->GetComponentLocation(), ProjectileRotation, 1.0f);
    }
    if (FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, FireSound, ProjectileLocation);
    }
}

void AWeapon::Test()
{
    GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, FString::Printf(TEXT("fuck")));
}

void AWeapon::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
    //¿ÛÑª
    if (OtherActor != this && OtherActor != GetOwner())
    {
        UAttributeComponent* AttributeComponent = Cast<UAttributeComponent>(OtherActor->GetComponentByClass(UAttributeComponent::StaticClass()));
        if (AttributeComponent)
        {
            AttributeComponent->ApplyHealthChange(Damage);
        }
    }
    //Ê©¼Ó³åÁ¿
    if (OtherActor != this && OtherActor != GetOwner() && OtherComponent->IsSimulatingPhysics())
    {
        //OtherComponent->AddImpulseAtLocation(FVector(100.f, 100.f, 100.f), Hit.ImpactPoint);
    }
}

void AWeapon::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ATwinblastCharacter* TargetCharacter = Cast<ATwinblastCharacter>(OtherActor);
    if (TargetCharacter)
    {
        TargetCharacter->AddWeapon(GetClass());
    }
    Destroy();
}