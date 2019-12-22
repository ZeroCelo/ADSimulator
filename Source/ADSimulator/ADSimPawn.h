// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "ADSimPawn.generated.h"

//if the array is not yet ready we will generate the vector on the caller thread.
FVector GenerateRandomVecInRange(int min, int max);

class ADSIMULATOR_API RNGThread : public FRunnable
{
public:
	//Constructor
	RNGThread(int Count = 50000, int minNumber = 0, int maxNumber = 1000, int chunkCount = 100);
	//Destructor
	~RNGThread();

	//Use this method to kill the thread!!
	void EnsureCompletion();

	//Pause the thread 
	void PauseThread();

	//Continue/UnPause the thread
	void ContinueThread();

	//FRunnable interface.
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();

	bool IsThreadPaused();

	void CreateThread();

	FVector GetRandomVector();
	int32 GetRandomVectorNum();

private:
	//Thread to run the worker FRunnable on
	FRunnableThread* Thread;

	FCriticalSection m_mutex;
	FEvent * m_semaphore;

	int m_chunkCount;
	int m_amount;
	int m_MinInt;
	int m_MaxInt;

	//As the name states those members are Thread safe
	FThreadSafeBool m_Kill;
	FThreadSafeBool m_Pause;

	//The array is a private member because we don't want to allow anyone from outside the class to access it since we want to ensure a thread safety.
	TArray<FVector> m_RandomVecs;
};

UCLASS()
class ADSIMULATOR_API AADSimPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AADSimPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
