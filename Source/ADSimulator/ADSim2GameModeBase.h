// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Runtime/Core/Public/Misc/DateTime.h"
#include "Runtime/Core/Public/Misc/Timespan.h"

//#include "ADSim2Pawn.h"
#include "ADSim2BlueprintFunctionLibrary.h"

#include "ADSim2GameModeBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(SimulatorLog2, Log, All);

USTRUCT(BlueprintType)
struct FSimClient
{
	GENERATED_BODY()

	//Define a coloração do cliente
	int32 ClientRound;
	int32 ClientIteration;

	//Informações a respeito deste cliente
	double ArrivalTime;		//Tempo de chegada ao sistema
	double ArrivalValue;
	double ServiceTime;		//Tempo de entrada em serviço
	double ServiceValue;
	int32 Priority;				//Se é do tipo 1 ou 2

	//Retorna o tempo de término de serviço
	double GetServiceEndTime() {
		return ServiceTime + ServiceValue;
	}

	//Retorna o tempo de espera na fila de espera
	double GetWaitTime() {
		if (ServiceTime >= ArrivalTime)
			return ServiceTime - ArrivalTime;
		else
			return -1.0;
	}

	//Copia Cliente
	void CopyClient(const FSimClient &Ev) {
		ClientRound = Ev.ClientRound;
		ClientIteration = Ev.ClientIteration;
		ArrivalTime = Ev.ArrivalTime;
		ArrivalValue = Ev.ArrivalValue;
		ServiceTime = Ev.ServiceTime;
		ServiceValue = Ev.ServiceValue;
		Priority = Ev.Priority;
	}

	bool IsValid() { return ClientRound >= 0; }
	bool IsServiceValid() { return ServiceTime >= ArrivalTime; }

	void Reset() {
		ClientRound = -1;	//Client inválido, serve pra dizer que ninguem está sendo servido
		ClientIteration = 1;
		ServiceTime = 0.0; ArrivalTime = 0.0;
		ServiceValue = 0.0; ArrivalValue = 0.0;
		Priority = 0;
	}

	FSimClient() {
		Reset();
	}
	FSimClient(const FSimClient &Ev) {
		CopyClient(Ev);
	}
};

USTRUCT(BlueprintType)
struct FSimQueue
{
	GENERATED_BODY()

	TArray<FSimClient> Queue1;
	TArray<FSimClient> Queue2;

	bool bLCFS1;
	bool bLCFS2;
	bool bOnlyQueue1;
	
	FSimClient GetClient(); //Pega próximo cliente da fila de espera e nao remove
	FSimClient PopClient(); //Pega próximo cliente da fila de espera e remove da fila
	void AddClient(FSimClient& Cli);

	bool HasClient() { return Queue1.Num() + Queue2.Num(); }
	int32 NumClient() { return Queue1.Num() + Queue2.Num(); }
	int32 NumQueue1() { return Queue1.Num(); }
	int32 NumQueue2() { return Queue2.Num(); }

	void Reset()
	{
		Queue1.Empty();
		Queue2.Empty();
		bOnlyQueue1 = true;
		bLCFS1 = false;
		bLCFS2 = false;
	}

	FSimQueue() { Reset(); }
};

UENUM(BlueprintType)
enum class ESimEventType : uint8
{
	Arrival		UMETA(DisplayName = "Arrival"),
	ServiceIn	UMETA(DisplayName = "ServiceIn"),
	ServiceOut	UMETA(DisplayName = "ServiceOut"),
	None		UMETA(DisplayName = "None")
};

USTRUCT(BlueprintType)
struct FSimEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESimEventType Type;
	
	double Time;
	double Value;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Priority;

	void Reset()
	{
		Type = ESimEventType::None;
		Time = 0.0;
		Value = 0.0;
		Priority = 0;
	}

	float GetTimef() const { return (float)Time; }
	float GetValuef() const { return (float)Value; }

	bool operator ==(const FSimEvent& OtherEvent) const
	{
		return Time == OtherEvent.Time;
	}

	bool operator <(const FSimEvent& OtherEvent) const
	{
		if (Time == OtherEvent.Time)
		{
			if (Type == ESimEventType::ServiceOut && OtherEvent.Type != Type)
			{
				return true;
			}
		}
		return Time < OtherEvent.Time;
		//return OtherEvent.Time < Time;
	}

	void CopyEvent(const FSimEvent& Ev)
	{
		Type = Ev.Type;
		Time = Ev.Time;
		Value = Ev.Value;
		Priority = Ev.Priority;
	}

	FSimEvent(ESimEventType NewType, double NewTime,double NewValue, int32 NewPriority = 0)
	{
		Type = NewType;
		Time = NewTime;
		Value = NewValue;
		Priority = NewPriority;
	}

	FSimEvent() { Reset(); }
	FSimEvent(const FSimEvent &NewEv) { CopyEvent(NewEv); }
};

UENUM(BlueprintType)
enum class ESimulatorState : uint8
{
	Unfinished	UMETA(DisplayName = "Unfinished"),
	Finished	UMETA(DisplayName = "Finished"),
	Error		UMETA(DisplayName = "Error")
};

USTRUCT(BlueprintType)
struct FTimeCounter
{
	GENERATED_BODY()

	FDateTime StartTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPause;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTimespan ElapsedTime;

	void Start() 
	{ 
		StartTime = FDateTime::UtcNow(); 
		ElapsedTime = FTimespan::FromMilliseconds(0.0);	
		bPause = false;
	}
	void Resume() 
	{ 
		StartTime = FDateTime::UtcNow(); 
		bPause = false;
	}
	void Pause() 
	{ 
		ElapsedTime += FDateTime::UtcNow() - StartTime; 
		bPause = true;
	}
	void Stop() 
	{ 
		ElapsedTime += FDateTime::UtcNow() - StartTime; 
		bPause = true;
	}

	FString GetTimeStr()
	{ 
		if (bPause) {
			return FString::Printf(TEXT("%02d:%02d:%02d.%03d"),
				ElapsedTime.GetHours(), ElapsedTime.GetMinutes(),
				ElapsedTime.GetSeconds(), ElapsedTime.GetFractionMilli()
			);
		}
		else
		{
			FTimespan temporary = ElapsedTime;
			temporary += FDateTime::UtcNow() - StartTime;
			return FString::Printf(TEXT("%02d:%02d:%02d.%03d"),
				temporary.GetHours(), temporary.GetMinutes(),
				temporary.GetSeconds(), temporary.GetFractionMilli()
			);
		}
	}

	FTimeCounter()
	{
		ElapsedTime = FTimespan::FromMilliseconds(0.0);
		StartTime = FDateTime::UtcNow();
		bPause = true;
	}
};

USTRUCT(BlueprintType)
struct FSimulator
{
	GENERATED_BODY()

	TArray<FSimEvent> EventList;//Lista de eventos
	FSimEvent LastEvent;		//Ultimo evento processado
	FSimQueue SysQueue;			//Fila de espera
	FSimClient SysService;		//Cliente em serviço, Round -1 caso esteja vazio
	
	FFileSetting FileConfig;	//Config para salvar metricas em arquivo
	FSimConfig Config;	//Config para essa simulação
	FTimeCounter SimTime;	//Tempo de simulação, precisa ser ativado por fora

	double LastArrivalTime1;	//Ultima chegada tipo 1
	double LastArrivalTime2;	//Ultima chegada tipo 2
	double LastServiceOut;		//Ultimo tempo de serviço processado
	int32 CurrentIteration;	//Iteraçao atual (Cliente)
	int32 CurrentRound;		//Rodada Atual, 0 => rodada transiente
	ESimulatorState SimState;	//Estado da simulação, Detalhes acima

	//Dados para metricas
	int32 SimIndex;	//Indice dessa Simulação
	FSimMetricData* Metric1;	//W1 Nq1
	FSimMetricData* Metric2;	//W2 Nq2
	FSimMetricData* Metric3;	//W Nq de todo sistema

	void GenerateAnalytic();
	float GetAnalyticENq1();
	float GetAnalyticENq2();
	float GetAnalyticENq3();
	float GetAnalyticEW1();
	float GetAnalyticEW2();
	float GetAnalyticEW3();

	void SaveRoundDump();
	void Process();
	void GenerateArrivals();
	void Reset();
	void TestEnding();

	void PrintEvents(FString Str);
	FString GetSimStateStr();

	void AddEvent(ESimEventType NewType, double NewTime, double NewValue, int32 NewPriority = 0);

	void ProcessArrival(const FSimEvent &CurrentEvent);		//Processa uma chegada no sistema
	void ProcessServiceIn(const FSimEvent &CurrentEvent);	//Cliente entrando em serviço
	void ProcessServiceOut();	//Cliente sai do sistema

	bool ProcessEnding() { return SimState != ESimulatorState::Unfinished; }
	bool IsServiceOutLast();

	FSimulator() { Reset(); }
	FSimulator(FSimConfig Cfg) { Config.CopyConfig(Cfg); Reset();}
};

UCLASS()
class ADSIMULATOR_API AADSim2GameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
};
