// Fill out your copyright notice in the Description page of Project Settings.

/*
	Avaliação e Desempenho 2019-1
	Trabalho de Simulação

	Alunos:
		Marcelo Araujo Carvalho	DRE 109062115
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Runnable.h"
#include "RunnableThread.h"
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformMisc.h"
#include "Runtime/Core/Public/HAL/ThreadSafeBool.h"
#include "Runtime/Core/Public/HAL/ThreadSafeCounter.h"
#include "Runtime/Core/Public/Misc/DateTime.h"
#include "Runtime/Core/Public/Misc/Timespan.h"

#include "ADSimPawn.h"

#include "ADSimPlayerController.generated.h"

//Macro para geração do log no editor
DECLARE_LOG_CATEGORY_EXTERN(ThreadLog, Log, All);

//Define qual estado se encontra a thread de processamento
UENUM(BlueprintType)		
enum class ERunState : uint8
{
	Idle	UMETA(DisplayName = "Idle"),
	Running	UMETA(DisplayName = "Running"),
	Paused	UMETA(DisplayName = "Paused"),
	Done	UMETA(DisplayName = "Done"),
	RoundDone	UMETA(DisplayName = "RoundDone")
};

USTRUCT(BlueprintType)
struct FFlowParameter
{
	GENERATED_BODY()

	//Se o parametro é exponencial ou deterministico
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bExponential;

	//Valor do parâmetro
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value;

	//Gerador de número aleatório
	float GetRandVal(FRandomStream& RandStream) {
		if (!bExponential) { return 1.0f / Value; }
		else {
			float UniformVar = RandStream.GetFraction();
			return -(FGenericPlatformMath::Loge(1.0f - UniformVar) / Value);
		}
	}

	FFlowParameter(){
		Value = 0.5f;
		bExponential = true;
	}
};

USTRUCT(BlueprintType)
struct FRunConfig
{
	GENERATED_BODY()

	//Parametro de chegada lambda
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFlowParameter ArrivalParam;

	//Parametro de serviço mu
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FFlowParameter ServiceParam;

	//Iterações da Rodada, kmin
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Iterations;

	//Número de rodadas para processar
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Rounds;

	//Se é FCFS ou LCFS
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFirstServed;

	FRunConfig() {
		Iterations = 2500;
		Rounds = 3200;
		ServiceParam.Value = 1.0f;
		bFirstServed = true;
	}
};

USTRUCT(BlueprintType)
struct FThreadConfig
{
	GENERATED_BODY()

	//Número de threads para se iniciar - não funciona mais
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumThreads;

	//Quanto damos sleep para poupar recursos de CPU
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SleepTime;

	//Quantos eventos processar até o sleep
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EventChunk;

	//Qual iteração devemos pausar
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Iteration2Pause;

	//Qual rodada devemos pausar
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Round2Pause;

	//Se quisermos pausar a cada passo do simulador
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAlwaysPause;

	//Copia configuração
	void CopyConfig(FThreadConfig &NewConf) {
		NumThreads = NewConf.NumThreads;
		SleepTime = NewConf.SleepTime;
		EventChunk = NewConf.EventChunk;
		Iteration2Pause = NewConf.Iteration2Pause;
		Round2Pause = NewConf.Round2Pause;
		bAlwaysPause = NewConf.bAlwaysPause;
	}

	FThreadConfig() {
		NumThreads = 1;
		SleepTime = 0.05f;
		EventChunk = 40;
		Iteration2Pause = -2;
		Round2Pause = -2;
		bAlwaysPause = false;
	}
};

USTRUCT(BlueprintType)
struct FPMFSum{
	GENERATED_BODY()

	double ValueSum;
	double ValueSumSquare;
	double TimeBegin;
	double TimeEnd;

	float GetAveragef() {
		return (float)GetAverage();
	}

	double GetAverage() {
		double TimeDiff = TimeEnd - TimeBegin;
		return ValueSum / TimeDiff;
	}

	float GetVariancef() {
		return (float)GetVariance();
	}
	
	double GetVariance() {
		double AvgSquare = GetAverage();
		AvgSquare *= AvgSquare;
		double TimeDiff = TimeEnd - TimeBegin;
		double AvgMoment2 = ValueSumSquare / TimeDiff;
		return AvgMoment2 - AvgSquare;
	}

	void AddValue(double Value, double Delta) {
		ValueSum += Value*Delta;
		ValueSumSquare += Value*Value*Delta;
	}

	void AddTimeBegin(double Value) {
		TimeBegin = Value;
	}

	void AddTimeEnd(double Value) {
		TimeEnd = Value;
	}

	FPMFSum() {
		ValueSum = 0.0;
		ValueSumSquare = 0.0;
		TimeBegin = 0.0;
		TimeEnd = 0.0;
	}

	FPMFSum(const FPMFSum &NewPMF) {
		ValueSum = NewPMF.ValueSum;
		ValueSumSquare = NewPMF.ValueSumSquare;
		TimeBegin = NewPMF.TimeBegin;
		TimeEnd = NewPMF.TimeEnd;
	}
};

USTRUCT(BlueprintType)
struct FIterationSum
{
	GENERATED_BODY()

	double ValueSum;
	double ValueSumSquare;
	int32 ValueSumTotal;
	int32 CurrentRound;

	double GetAverage() {
		if(ValueSumTotal > 1)
			return ValueSum / ValueSumTotal;

		return ValueSum;
	}

	double GetVariance() {
		/*if (ValueSumTotal > 1) {
			double TotalSum = (double)ValueSumTotal;
			double TotalSumMinus = TotalSum - 1.0;
			double SumSquare = ValueSum*ValueSum;
			SumSquare /= TotalSum*TotalSumMinus;
			double Moment2 = ValueSumSquare/TotalSumMinus;
			return Moment2 - SumSquare;
		}*/
		//else if (ValueSumTotal == 1) {
		//	return ValueSumSquare - (ValueSum*ValueSum);
		//}
		
		return 0.0;
	}	

	double GetVariance2() {	//Ta Correto agora xD -> Sum(X^2)/n-1 - Sum(X)*Sum(X)/n*n-1
		if (ValueSumTotal > 1) {
			double TotalSum = (double)ValueSumTotal;
			double TotalSumMinus = TotalSum - 1.0;
			double SumSquare = (ValueSum*ValueSum)/(TotalSum*TotalSumMinus);
			double SumSquare2 = ValueSumSquare/TotalSumMinus;
			return SumSquare2 - SumSquare;
		}
		//else if (ValueSumTotal == 1) {
		//	return ValueSumSquare - (ValueSum*ValueSum);
		//}
		
		return 0.0;
	}

	float GetAveragef() {
		return (float)GetAverage();
	}

	float GetVariancef() {
		return (float)GetVariance();
	}

	void AddValue(double NewValue) {
		ValueSum += NewValue;
		ValueSumSquare += NewValue * NewValue;
		++ValueSumTotal;
	}

	void Reset() {
		ValueSum = 0.0;
		ValueSumSquare = 0.0;
		ValueSumTotal = 0;
		CurrentRound = 0;
	}

	FIterationSum() {
		Reset();
	}

	FIterationSum(const FIterationSum &Sum) {
		ValueSum = Sum.ValueSum;
		ValueSumTotal = Sum.ValueSumTotal;
		ValueSumSquare = Sum.ValueSumSquare;
		CurrentRound = Sum.CurrentRound;
	}
};

USTRUCT(BlueprintType)
struct FIterationEvent
{
	GENERATED_BODY()

	//Define a coloração do cliente
	int32 CurrentRound;	
	int32 CurrentIteration;

	//Informações a respeito deste cliente
	double ArrivalTime;	
	float ArrivalValue;
	double ServiceTime;
	float ServiceValue;
	double Nq;

	//Retorna o tempo de término de serviço
	double GetServiceEndTime() {
		return ServiceTime + (double)ServiceValue;
	}

	//Retorna o tempo de espera na fila de espera
	double GetWaitTime() {
		if (ServiceTime >= ArrivalTime)
			return ServiceTime - ArrivalTime;
		else
			return -1.0;
	}

	//Copia Evento
	void ReceiveIteration(const FIterationEvent &Ev) {
		CurrentRound = Ev.CurrentRound;
		CurrentIteration = Ev.CurrentIteration;
		ArrivalTime = Ev.ArrivalTime;
		ArrivalValue = Ev.ArrivalValue;
		ServiceTime = Ev.ServiceTime;
		ServiceValue = Ev.ServiceValue;
		Nq = Ev.Nq;
	}

	void Reset() {
		CurrentRound = -1;
		CurrentIteration = 1;
		Nq = 0.0;
		ServiceTime = 0.0; ArrivalTime = 0.0;
		ServiceValue = 0.0f; ArrivalValue = 0.0f;
	}

	FIterationEvent() {
		Reset();
	}
	FIterationEvent(const FIterationEvent &Ev) {
		ReceiveIteration(Ev);
	}
};

USTRUCT(BlueprintType)
struct FThreadSharedData
{
	GENERATED_BODY()

	//Gerador de números pseudo-aleatórios
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRandomStream RandStream;

	//Configuração atual
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRunConfig CurrentRunConfig;

	//Estado Atual da thread
	ERunState CurrentState;
	
	//----------Variáveis do estado do simulador
	int32 TransientIterations;
	int32 CurrentRound;
	int32 CurrentIteration;
	int32 CurrentServiceRound;
	int32 CurrentServiceIteration;
	double CurrentArrivalTimeStamp;
	double CurrentServiceTimeStamp;
	double RoundStartTime;
	
	TArray<FIterationEvent> EventsList;
	FIterationEvent CurrentService;
	FIterationEvent LastOutEvent;
	//----------Variáveis do estado do simulador
	
	//----------Variáveis da coleta de dados
	TArray<FIterationSum> RoundWaitSum;
	TArray<FPMFSum> RoundNqSumPmf;

	FIterationSum RunEWaitSum;
	FIterationSum RunVWaitSum;
	FIterationSum RunVNqSum;
	FIterationSum RunENqSum;
	
	TArray<FIterationSum> TransientWaitSum;
	TArray<FPMFSum> TransientNqPmf;
	FIterationSum TransientArrivalSum;	
	FIterationSum TransientServiceSum;	
	//----------Variáveis da coleta de dados

	//----------Variável para saída de arquivo
	TArray<FIterationEvent> OutList;
	//----------Variável para saída de arquivo

	int32 LastPauseIteration;
	int32 LastPauseRound;

	bool bShouldUseTransient;

	bool bSaveTransientOut;
	bool bSaveIterationOut;

	//Inicia a semente
	void InitSeed(int32 NewSeed){RandStream.Initialize(NewSeed);}
	
	//Passa a configuração atual da simulação
	void InitRunConfig(FRunConfig NewConfig) { 
		CurrentRunConfig = NewConfig; 
		ResetRun();
	}

	//Reinicia todas as variáveis para começar uma nova simulação
	void ResetRun() {
		RandStream.Reset();
		if (bShouldUseTransient) CurrentRound = 0;
		else CurrentRound = 1;
		CurrentIteration = 1;
		CurrentServiceIteration = 1;
		CurrentServiceRound = CurrentRound;
		CurrentArrivalTimeStamp = 0.0; CurrentServiceTimeStamp = 0.0;
		
		EventsList.Empty();	//EventsList.Reserve(CurrentRunConfig.Iterations);
		OutList.Empty(); //OutList.Reserve(CurrentRunConfig.Iterations*CurrentRunConfig.Rounds);
		
		RoundWaitSum.Empty(); //RoundWaitSum.Reserve(CurrentRunConfig.Rounds);
		//RoundNqSum.Empty(); RoundNqSum.Reserve(CurrentRunConfig.Rounds);
		RoundNqSumPmf.Empty(); //RoundNqSumPmf.Reserve(CurrentRunConfig.Rounds);
		RoundWaitSum.Add(FIterationSum());
		//RoundNqSum.Add(FIterationSum());
		RoundNqSumPmf.Add(FPMFSum());
		
		TransientWaitSum.Empty();
		TransientArrivalSum.Reset(); TransientServiceSum.Reset();
		//TransientNqSum.Empty(); TransientNqPmf.Empty();

		RunEWaitSum.Reset(); RunVWaitSum.Reset();
		RunENqSum.Reset(); RunVNqSum.Reset();
		
		CurrentService.Reset();
		CurrentState = ERunState::Idle;
		//LastOutEvent.CurrentRound = -1;
		LastOutEvent.Reset();
		LastPauseRound = -3;
		LastPauseIteration = -3;
		RoundStartTime = 0.0;

		UE_LOG(ThreadLog, Log, TEXT("ThreadData.Reset CurRnd=%d ShouldTrans=%d "), CurrentRound, bShouldUseTransient);
	}

	float GetServiceRand() {
		return CurrentRunConfig.ServiceParam.GetRandVal(RandStream);
	}

	float GetArrivalRand() {
		return CurrentRunConfig.ArrivalParam.GetRandVal(RandStream);
	}

	FThreadSharedData():RandStream(14000605) {
		LastPauseRound = -3;
		LastPauseIteration = -3;
		CurrentState = ERunState::Idle;
		TransientIterations = 50;
		bShouldUseTransient = true;
	}
};

class ADSIMULATOR_API ADSimThread : public FRunnable
{
public:
	//Constructor
	ADSimThread(int32 ThreadID, int32 ThreadNum, FThreadConfig &NewConfig);
	//Destructor
	~ADSimThread();

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

	bool IsThreadPaused() { return (bool)TPause; }

	int32 GetTID() { return TID; }
	int32 GetTNum() { return TNum; }

	void ProcessAlone2();

	void OutCurrentEvent2(FIterationEvent& Current);

	void TestShouldPause2();

	FThreadConfig TConfig;

	UPROPERTY()
	static FThreadSharedData ThreadData;

	static FCriticalSection TMutexState;
	static FCriticalSection TMutexMetric;
	static FCriticalSection TMutexOutList;
	static FCriticalSection TMutexEvents;
	static FEvent* TSemaphore;

private:
	//Thread to run the worker FRunnable on
	FRunnableThread* Thread;

	bool bShouldPause;

	int32 TID; //Thread ID [0..Num-1]
	int32 TNum;//Total threads

	int32 TChunkCurrent;
	int32 TChunk;
	float TSleepTime;

	//As the name states those members are Thread safe
	static FThreadSafeBool TKill;
	static FThreadSafeBool TPause;
};

UCLASS()
class ADSIMULATOR_API AADSimPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	AADSimPlayerController(const FObjectInitializer& ObjectInitializer);

	TArray<ADSimThread*> ThreadPool;

	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	//função que inicia a simulação
	UFUNCTION(BlueprintCallable)
	void StartRun();

	//função que interrompe a simulação
	UFUNCTION(BlueprintCallable)
	void StopRun();

	//função que resume a simulação
	UFUNCTION(BlueprintCallable)
	void ResumeRun();

	//função que pausa a simulação
	UFUNCTION(BlueprintCallable)
	void PauseRun();

	UFUNCTION(BlueprintCallable)
	void PassTransientConfig(int32 NewTransient,bool bNewShouldTransient) {
		ADSimThread::ThreadData.TransientIterations = NewTransient;
		ADSimThread::ThreadData.bShouldUseTransient = bNewShouldTransient;
	}

	UFUNCTION(BlueprintCallable)
	void PassThreadConfig(FThreadConfig PassConfig);

	UFUNCTION(BlueprintCallable)
	void PassRunConfig(TArray<FFlowParameter> Arrivals, TArray<FFlowParameter> Services, TArray<float> Rounds, TArray<float> Iterations, bool bFCFS, bool bLCFS);

	UFUNCTION(BlueprintCallable)
	void PassSeed(int32 NewSeed) { ADSimThread::ThreadData.InitSeed(NewSeed); }

	UFUNCTION(BlueprintPure)
	ERunState GetCurrentState() { return CurrentState; }

	UFUNCTION(BlueprintCallable)
	void SetCurrentState(ERunState NewState); 

	UFUNCTION(BlueprintCallable)
	void SaveToFile();

	UFUNCTION(BlueprintCallable)
	void SetDumpFile(FString NewRoundFile, FString NewIterationFile, FString NewTransientFile, FString NewRunFile) {
		RoundDumpFileName = NewRoundFile; IterationDumpFileName = NewIterationFile;
		TransientDumpFileName = NewTransientFile; RunDumpFileName = NewRunFile;
	}

	UFUNCTION(BlueprintCallable)
	void SetDumpSave(bool bShouldSaveRound, bool bShouldSaveIteration, bool bShouldSaveTransient, bool bShouldSaveRun){
		bSaveDumpRound = bShouldSaveRound; bSaveDumpRun = bShouldSaveRun;
		bSaveDumpIteration = bShouldSaveIteration; 
		bSaveDumpTransient = bShouldSaveTransient; 
	}

	UFUNCTION(BlueprintPure)
	void GetTransientMetric(TArray<FVector2D> &Wait, TArray<FVector2D> &Nqueue);

	UFUNCTION(BlueprintPure)
	void GetRoundMetric(TArray<FVector2D> &Wait, TArray<FVector2D> &Nqueue);

	UFUNCTION(BlueprintPure)
	void GetRoundMetricEvolution(TArray<FVector2D> &Wait, TArray<FVector2D> &Nqueue);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSaveDumpRound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSaveDumpIteration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSaveDumpTransient;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSaveDumpRun;

	UFUNCTION(BlueprintCallable)
	void OpenDumpFolder();

	UFUNCTION(BlueprintCallable)
	void SaveDumpsIteTra();

	void SaveDumpTransient();
	void SaveDumpIteration();
	void SaveDumpRound();
	void SaveDumpRun();

	UFUNCTION(BlueprintPure)
	FString GetSomeDumpName(const FString &FileName, bool bUseCurrent = false);

	UFUNCTION(BlueprintPure)
	FThreadConfig GetCurrentThreadConfig() { return ThreadConfig; }

	UFUNCTION(BlueprintPure)
	FString GetThreadStr(bool bPrint = false);

	UFUNCTION(BlueprintPure)
	int32 GetThreadPoolNum() { return ThreadPool.Num(); }

	UFUNCTION(BlueprintPure)
	FString GetCPUText();

	UFUNCTION(BlueprintPure)
	int32 GetCPUCores();

	UFUNCTION(BlueprintPure)
	int32 GetCPUThreads();

	UFUNCTION(BlueprintPure)
	FString GetRandomStr(bool bPrint);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString IterationDumpFileName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RoundDumpFileName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TransientDumpFileName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RunDumpFileName;

	UFUNCTION(BlueprintCallable)
	void SetPlotTransient(bool bNewPlot) { bPlotTransient = bNewPlot; }

	UFUNCTION(BlueprintPure)
	bool GetPlotTransient() { return bPlotTransient; }

	bool bPlotTransient;

	FString CurrentDumpSuffix;
	FString CurrentRunDumpSuffix;
	int32 RunDumpCount;
	int32 RoundDumpCount;
	int32 IterationDumpCount;
	int32 TransientDumpCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ERunState CurrentState;

	//Information for each run
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FRunConfig> RunConfigs;

	//Current Thread Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FThreadConfig ThreadConfig;

	UFUNCTION(BlueprintPure)
	FString GetElapsedRunTime();

	UFUNCTION(BlueprintPure)
	FString GetHistoryRunTime();

	FDateTime StartTime;
	FDateTime EndTime;
	FTimespan ElapsedPause;
	TArray<FTimespan> ElapsedHistory;
};

UCLASS()
class UADSimFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "VictoryBPLibrary")
	static bool IsWithEditor();

};
