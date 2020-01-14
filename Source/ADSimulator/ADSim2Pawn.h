// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

//#include "ADSim2GameModeBase.h"
//#include "ADSim2BlueprintFunctionLibrary.h"
#include "ADSim2PlayerController.h"
#include "ADSim2MetricSingleSource.h"
#include "ADSim2MainWidget.h"

#include "ADSim2Pawn.generated.h"

//Macro para geração do log no editor
DECLARE_LOG_CATEGORY_EXTERN(PawnLog2, Log, All);

UCLASS()
class ADSIMULATOR_API AADSim2Pawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AADSim2Pawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	TArray< ADSim2Thread* > ThreadPool;
	TArray< FSimulator > Simulations;
	FThreadSafeCounter SimCurrentIndex;

	FFileSetting FileCfg;
	FThreadSetting TSetting;
	int32 NumThreads;

	FTimerHandle ThreadCheckHandle;

	void CheckThread();
	void SaveSimDump();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ADSim2Test")
	bool bPassMetricChart = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ADSim2Test")
	int32 SimMax = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ADSim2Test")
	int32 RoundMax = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ADSim2Test")
	int32 ValueMax = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ADSim2UMG")
	TSubclassOf< class UADSim2MainWidget > MainWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ADSim2UMG")
	class UADSim2MainWidget* MainWidgetObj;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ADSim2UMG")
	class UADSim2ChartWidget* ChartWidgetObj;

	UFUNCTION(BlueprintCallable)
	void OpenDumpDir();

	UFUNCTION(BlueprintCallable)
	void SetFileDump(const FString &NewSimDump,const FString &NewRoundDump, const FString &NewIterationDump, const FString &NewTransientDump);

	UFUNCTION(BlueprintCallable)
	void SetFileDumpBool(bool bNewSimDump, bool bNewRoundDump, bool bNewIterationDump, bool bNewTransientDump);

	UFUNCTION(BlueprintCallable)
	void SetThreadSetting(int32 NThreads, int32 EventChunk, float SleepTime, bool bPauseAlways);

	UFUNCTION(Blueprintpure, BlueprintCallable)
	int32 GetSimNum() { return Simulations.Num(); }

	UFUNCTION(Blueprintpure, BlueprintCallable)
	int32 GetSimEventNum(int32 SimIndex) { return Simulations[SimIndex].EventList.Num(); }

	UFUNCTION(Blueprintpure, BlueprintCallable)
	int32 GetSimEventCurrentRound(int32 SimIndex) { return Simulations[SimIndex].CurrentRound; }

	UFUNCTION(Blueprintpure, BlueprintCallable)
	int32 GetSimEventCurrentIteration(int32 SimIndex) { return Simulations[SimIndex].CurrentIteration; }

	UFUNCTION(Blueprintpure, BlueprintCallable)
	int32 GetSimQ1(int32 SimIndex) { return Simulations[SimIndex].SysQueue.NumQueue1(); }

	UFUNCTION(Blueprintpure, BlueprintCallable)
	int32 GetSimQ2(int32 SimIndex) { return Simulations[SimIndex].SysQueue.NumQueue2(); }

	UFUNCTION(Blueprintpure, BlueprintCallable)
	float GetSimEventCurrentLast1(int32 SimIndex) { return Simulations[SimIndex].LastArrivalTime1; }

	UFUNCTION(Blueprintpure, BlueprintCallable)
	float GetSimEventCurrentLast2(int32 SimIndex) { return Simulations[SimIndex].LastArrivalTime2; }

	UFUNCTION(Blueprintpure, BlueprintCallable)
	float GetSimEventCurrentLastSvc(int32 SimIndex) { return Simulations[SimIndex].LastServiceOut; }

	UFUNCTION(Blueprintpure, BlueprintCallable)
	FSimEvent GetSimEventCurrentLastEvent(int32 SimIndex) { return Simulations[SimIndex].LastEvent; }

	UFUNCTION(Blueprintpure, BlueprintCallable)
	FSimEvent GetSimEventCurrentFirstEvent(int32 SimIndex) { 
		if(Simulations[SimIndex].EventList.Num())
			return Simulations[SimIndex].EventList[0]; 
		return FSimEvent();
	}

	UFUNCTION(Blueprintpure, BlueprintCallable)
	FString GetSimSysSvcStr(int32 SimIndex) { 
		return FString::Printf(TEXT("[%d; %d; %d; %d]"), Simulations[SimIndex].SysService.IsValid(), Simulations[SimIndex].SysService.Priority+1, Simulations[SimIndex].SysService.ClientRound, Simulations[SimIndex].SysService.ClientIteration);
	}

	UFUNCTION(Blueprintpure, BlueprintCallable)
	ESimEventType GetSimEventType(int32 SimIndex, int32 EventIndex) {
		return Simulations[SimIndex].EventList[EventIndex].Type;
	}

	UFUNCTION(Blueprintpure, BlueprintCallable)
	float GetSimEventTime(int32 SimIndex, int32 EventIndex) {
		return Simulations[SimIndex].EventList[EventIndex].Time;
	}

	UFUNCTION(Blueprintpure, BlueprintCallable)
	float GetSimEventValue(int32 SimIndex, int32 EventIndex) {
		return Simulations[SimIndex].EventList[EventIndex].Value;
	}

	UFUNCTION(Blueprintpure, BlueprintCallable)
	int32 GetSimEventPriority(int32 SimIndex, int32 EventIndex) {
		return Simulations[SimIndex].EventList[EventIndex].Priority;
	}

	UFUNCTION(Blueprintpure, BlueprintCallable)
	bool GetSimEventIndexValid(int32 SimIndex, int32 EventIndex) {
		return Simulations[SimIndex].EventList.IsValidIndex(EventIndex);
	}

	UFUNCTION(Blueprintpure, BlueprintCallable)
	bool GetSimIndexValid(int32 SimIndex) {
		return Simulations.IsValidIndex(SimIndex);
	}

	UFUNCTION(Blueprintpure, BlueprintCallable)
	float GetEventTimef(const FSimEvent &Event) { return Event.GetTimef(); }

	UFUNCTION(Blueprintpure, BlueprintCallable)
	float GetEventValuef(const FSimEvent &Event) { return Event.GetValuef(); }

	//Thread Safe
	UFUNCTION(BlueprintCallable)
	bool IsSimDone();

	//Thread Safe
	UFUNCTION(BlueprintCallable)
	bool HasSimStart();

	UFUNCTION(BlueprintCallable)
	bool HasSimPaused();

	UFUNCTION(Blueprintpure, BlueprintCallable)
	ESimulatorState GetSimState(int32 SimIndex);

	UFUNCTION(Blueprintpure, BlueprintCallable)
	FString GetSimTimeStr(int32 SimIndex);

	UFUNCTION(Blueprintpure, BlueprintCallable)
	FTimespan GetSimTime(int32 SimIndex);

	UFUNCTION(Blueprintpure, BlueprintCallable)
	FTimespan GetSimTimeTotal();

	UFUNCTION(Blueprintpure, BlueprintCallable)
	FString GetSimTimeTotalStr();

	UFUNCTION(BlueprintCallable)
	void TestSomething();

	UFUNCTION(BlueprintCallable)
	void TestSomething2();

	UFUNCTION(BlueprintCallable)
	void StartSim();

	UFUNCTION(BlueprintCallable)
	void StopSim();

	UFUNCTION(BlueprintCallable)
	void PauseSim();

	UFUNCTION(BlueprintCallable)
	void ResumeSim();

	UFUNCTION(BlueprintCallable)
	void NextSim();

	UFUNCTION(BlueprintCallable)
	void PassSimConfig(const TArray< FSimParameter > &Arrivals1, const TArray< FSimParameter > &Services1,
		const TArray< FSimParameter > &Arrivals2, const TArray< FSimParameter > &Services2, 
		const TArray<int32> &Iterations, const TArray<int32> &Rounds, 
		const TArray<int32> &Seeds, const int32 TransientIterations,
		const bool bQueue1Only, const bool bQueue1LCFS, const bool bQueue2LCFS
	);
};
