// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

//#include "ADSim2BlueprintFunctionLibrary.h"
#include "ADSim2GameModeBase.h"

#include "Runnable.h"
#include "RunnableThread.h"
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformMisc.h"
#include "Runtime/Core/Public/HAL/ThreadSafeBool.h"
#include "Runtime/Core/Public/HAL/ThreadSafeCounter.h"
#include "Runtime/Core/Public/Misc/DateTime.h"
#include "Runtime/Core/Public/Misc/Timespan.h"

#include "ADSim2PlayerController.generated.h"

/**
 * 
 */

//Macro para geração do log no editor
DECLARE_LOG_CATEGORY_EXTERN(ThreadLog2, Log, All);

USTRUCT(BlueprintType)
struct FThreadSetting
{
	GENERATED_BODY()

	//Quanto damos sleep para poupar recursos de CPU
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SleepTime;

	//Quantos eventos processar até o sleep
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EventChunk;

	//Se quisermos pausar a cada passo do simulador
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAlwaysPause;

	//Copia configuração
	void CopyConfig(FThreadSetting &NewConf) 
	{
		SleepTime = NewConf.SleepTime;
		EventChunk = NewConf.EventChunk;
		bAlwaysPause = NewConf.bAlwaysPause;
	}

	FThreadSetting() 
	{
		SleepTime = 0.05f;
		EventChunk = 40;
		bAlwaysPause = false;
	}
};

UENUM(BlueprintType)
enum class ESimThreadState : uint8
{
	Init	UMETA(DisplayName = "Init"),
	Process	UMETA(DisplayName = "Process"),
	Paused	UMETA(DisplayName = "Paused"),
	JobDone	UMETA(DisplayName = "JobDone"),
	Error	UMETA(DisplayName = "Error")
};

class ADSIMULATOR_API ADSim2Thread : public FRunnable
{
public:
	//Constructor
	ADSim2Thread(int32 ThreadID, int32 ThreadNum, FThreadSetting &NewConfig,AActor* OwnerPawn);
	//Destructor
	~ADSim2Thread();

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

	int32 GetTID() { return TID; }
	int32 GetTNum() { return TNum; }

	void ProcessSimulation();
	//void OutCurrentEvent2(FIterationEvent& Current);
	void TestPause();

	AActor* PlayerPawn;

	FThreadSetting TConfig;
	//FSimData* SimData;
	int32 SimIndex;
	ESimThreadState TState;
	
	static FEvent* TSemaphore;
	static FThreadSafeBool TKill;
	static FThreadSafeBool TPause;

private:
	//Thread to run the worker FRunnable on
	FRunnableThread* Thread;

	bool bShouldPause;

	int32 TID; //Thread ID [0..Num-1]
	int32 TNum;//Total threads

	int32 TChunkCurrent;
	int32 TChunk;
	float TSleepTime;
};

UCLASS()
class ADSIMULATOR_API AADSim2PlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	AADSim2PlayerController(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	//virtual void BeginDestroy() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
