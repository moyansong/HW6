// Fill out your copyright notice in the Description page of Project Settings.


#include "Grenade.h"
#include "Components/SphereComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Particles/ParticleSystem.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AttributeComponent.h"
#include "Containers/Array.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Sound/SoundCue.h"

// Sets default values
AGrenade::AGrenade()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    BaseDamage = 10.f;
    LifeSpan = 5.f;
    LiveTime = 0.f;

    if (!RootComponent)
    {
        RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
    }

    if (!CollisionComponent)
    {
        // ��������м򵥵���ײչʾ��
        CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
        // �����������ײ�뾶��
        CollisionComponent->InitSphereRadius(10.0f);
        //��ײ��������,����Щ����������ײ
        CollisionComponent->BodyInstance.SetCollisionProfileName(TEXT("Projectile"));
        //���ô�����Ļص�,Ӧ��Ǩ�Ƶ� virtual void PostInitializeComponents() override��
        CollisionComponent->OnComponentHit.AddDynamic(this, &AGrenade::OnHit);
        //�ص��¼��ص�
        CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AGrenade::OnOverlap);
        // �����������Ϊ��ײ���,�������λ�ü�Ϊ��ײ�����λ��
        RootComponent = CollisionComponent;
    }

    if (!MovementComponent)
    {
        // ʹ�ô����������������ƶ���
        MovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComponent"));
        //CollisionComponent�Ǹ����
        MovementComponent->SetUpdatedComponent(CollisionComponent);
        MovementComponent->InitialSpeed = 1000.f;
        MovementComponent->MaxSpeed = 0.0f;
        //ÿһ֡����ת
        MovementComponent->bRotationFollowsVelocity = true;
        MovementComponent->bShouldBounce = true;
        //����ϵ��
        MovementComponent->Bounciness = 0.3f;
        //������0����û��������1����ٷְ�����
        MovementComponent->ProjectileGravityScale = 1.0f;
    }

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);

    RadialForceComponent = CreateDefaultSubobject<URadialForceComponent>(TEXT("RaicalForceComponent"));
    RadialForceComponent->SetupAttachment(RootComponent);
    //�����������ķ�Χ
    RadialForceComponent->Radius = 500.f;
    //���Ĵ�С
    RadialForceComponent->ForceStrength = 1000.f;
    RadialForceComponent->ImpulseStrength = 100000.f;

}

// Called when the game starts or when spawned
void AGrenade::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGrenade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    LiveTime += DeltaTime;
    if (LiveTime >= LifeSpan)
    {
        Explode(BaseDamage);
    }
}

void AGrenade::FireInDirection(const FVector& ShootDirection)
{
    MovementComponent->Velocity = ShootDirection * MovementComponent->InitialSpeed;
}

void AGrenade::OnHit_Implementation(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
    FVector Velocity = MovementComponent->Velocity;
    //Ҫ�뷵��������ʣ�Ҫ��������ײѡ���ﹴѡReturn Material On Move
    //����ʹ������׷��ʱ����LineTraceSingleByChannel��һ������FCollisionQueryParams TraceParams(TEXT("TraceActor"), true, this);TraceParams.bReturnPhysicalMaterial = true;
    if (Hit.PhysMaterial.IsValid())
    {
        UPhysicalMaterial* HitPhysicalMaterial = Hit.PhysMaterial.Get();
        MovementComponent->Velocity = Velocity * HitPhysicalMaterial->Restitution * -1.f;
    }
}

void AGrenade::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor != GetOwner())
    {
        Explode(BaseDamage);
    }
}

void AGrenade::Explode(float Damage)
{
    UWorld* World = GetWorld();
    if (World)
    {
        UGameplayStatics::SpawnEmitterAtLocation(World, ExplodeParticle, this->GetTransform());
        //RadialForceComponent->FireImpulse();//����Χ�����������
        UGameplayStatics::ApplyRadialDamage(World, Damage, GetActorLocation(), RadialForceComponent->Radius, nullptr, TArray<AActor*>(), this);
        UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());
    }
    Destroy();
}
