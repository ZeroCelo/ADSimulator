// Fill out your copyright notice in the Description page of Project Settings.


#include "ADSim2PlayerController.h"

#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Runtime/Core/Public/Misc/Paths.h"
//#include "KantanCartesianDatasourceInterface.h"
#include "ADSim2Pawn.h"

#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

//Macro para geração do log no editor
DEFINE_LOG_CATEGORY(ThreadLog2);

//-----------------------Variáveis estáticas para comunicação entre threads
FThreadSafeBool ADSim2Thread::TPause = false;
FThreadSafeBool ADSim2Thread::TKill = false;
FEvent* ADSim2Thread::TSemaphore = nullptr;
//-----------------------Variáveis estáticas para comunicação entre threads

//-------------------ADSim2Thread2-------------Begin
ADSim2Thread::ADSim2Thread(int32 ThreadID, int32 ThreadNum, FThreadSetting &NewConfig, AActor* OwnerPawn)
{
	TKill = false;
	TPause = false;
	bShouldPause = false;
	SimIndex = -1;
	//SimData = NewData;
	TState = ESimThreadState::Init;
	PlayerPawn = OwnerPawn;
	
	//Initialize FEvent (as a cross platform (Confirmed Mac/Windows))
	if (TSemaphore == nullptr)
		TSemaphore = FGenericPlatformProcess::GetSynchEventFromPool(false);;

	TConfig.CopyConfig(NewConfig);
	TChunkCurrent = 0;
	TChunk = TConfig.EventChunk;
	TSleepTime = TConfig.SleepTime;
	TID = ThreadID;
	TNum = ThreadNum;

	FString TName(FString::Printf(TEXT("ADSim2Thread%02d"), TID));
	Thread = FRunnableThread::Create(this, *TName, 0, TPri_BelowNormal);
}

ADSim2Thread::~ADSim2Thread()
{
	if (TSemaphore != nullptr)
	{
		//Cleanup the FEvent
		FGenericPlatformProcess::ReturnSynchEventToPool(TSemaphore);
		TSemaphore = nullptr;
	}

	if (Thread != nullptr)
	{
		//Cleanup the worker thread
		delete Thread;
		Thread = nullptr;
	}
}

bool ADSim2Thread::IsThreadPaused()
{
	return TPause;
}

bool ADSim2Thread::Init()
{
	UE_LOG(ThreadLog2, Log, TEXT("Thread2[%d] init.."), TID);

	AADSim2Pawn* Pawn = Cast<AADSim2Pawn>(PlayerPawn);

	if (Pawn != nullptr)
	{
		SimIndex = Pawn->SimCurrentIndex.Increment();
	}

	//Init the Data 
	return true;
}

/*
	função onde ocorre todo o loop da thread até ela ser terminada, estruturada de forma que permita chamar a
	função de processamento e organizar as lógicas de sleep
*/
uint32 ADSim2Thread::Run()
{
	//Initial wait before starting
	FPlatformProcess::Sleep(0.03);
	UE_LOG(ThreadLog2, Log, TEXT("Thread2[%d] Begin Run.."), TID);

	TState = ESimThreadState::Process;

	AADSim2Pawn* Pawn = Cast<AADSim2Pawn>(PlayerPawn);

	if (Pawn != nullptr && Pawn->Simulations.IsValidIndex(SimIndex))
	{
		//Pawn->Simulations[SimIndex].GenerateAnalytic();
		Pawn->Simulations[SimIndex].SimTime.Start();
	}

	while (!TKill)
	{
		if (TPause)
		{
			UE_LOG(ThreadLog2, Log, TEXT("Thread2[%d] Pausing.."), TID);
			if(TState == ESimThreadState::Process)
				TState = ESimThreadState::Paused;

			if (Pawn != nullptr && Pawn->Simulations.IsValidIndex(SimIndex)) 
			{
				Pawn->Simulations[SimIndex].SimTime.Pause();
			}

			//will "sleep" the thread until it will get a signal "Trigger()"
			if (TSemaphore != nullptr)
				TSemaphore->Wait();
			else
				return -10;

			UE_LOG(ThreadLog2, Log, TEXT("Thread2[%d] Unpause.."), TID);
			TState = ESimThreadState::Process;
			bShouldPause = false;

			if (Pawn != nullptr && Pawn->Simulations.IsValidIndex(SimIndex))
			{
				Pawn->Simulations[SimIndex].SimTime.Resume();
			}
		}
		else
		{
			//Chama a função principal de simulação
			ProcessSimulation();
			++TChunkCurrent; //Incrementa o número de eventos processado

			if (bShouldPause) {
				TPause = true;
				TChunkCurrent = 0;
			}

			//A little sleep between the chunks (So CPU will rest a bit -- (may be omitted))
			if (TChunkCurrent > TChunk && TChunk > 0) {
				TChunkCurrent = 0;
				FPlatformProcess::Sleep(TSleepTime);
			}
		}
	}

	UE_LOG(ThreadLog2, Log, TEXT("Thread2[%d] End run.."), TID);

	if (Pawn != nullptr && Pawn->Simulations.IsValidIndex(SimIndex))
	{
		Pawn->Simulations[SimIndex].SimTime.Stop();
	}

	return 0;
}

void ADSim2Thread::TestPause()
{
	if (TConfig.bAlwaysPause)
		bShouldPause = true;
}

void ADSim2Thread::ProcessSimulation()
{
	//if (SimData != nullptr)
	AADSim2Pawn* Pawn = Cast<AADSim2Pawn>(PlayerPawn);
	if(Pawn != nullptr)
	{
		if (Pawn->Simulations.IsValidIndex(SimIndex))
		{
			UE_LOG(ThreadLog2, Log, TEXT("Thread2[%d] Sim[%d].State=%s"), TID,SimIndex, *Pawn->Simulations[SimIndex].GetSimStateStr());
			if (!Pawn->Simulations[SimIndex].ProcessEnding())
			{
				Pawn->Simulations[SimIndex].Process();
			}
			else
			{
				UE_LOG(ThreadLog2, Log, TEXT("Thread2[%d] SimIndex Increment"), TID);
				Pawn->Simulations[SimIndex].SimTime.Stop();
				SimIndex = Pawn->SimCurrentIndex.Increment();
				if(Pawn->Simulations.IsValidIndex(SimIndex))
					Pawn->Simulations[SimIndex].SimTime.Start();
			}
		}
		else
		{
			UE_LOG(ThreadLog2, Log, TEXT("Thread2[%d] SimIndex !Valid, JobDone"), TID);
			TState = ESimThreadState::JobDone;
			Pawn->SimCurrentIndex.Increment();
			bShouldPause = true;
		}

		TestPause();
	}
	else
	{
		UE_LOG(ThreadLog2, Log, TEXT("Thread2[%d] Error Pawn Null"), TID);
		/*if (Pawn->Simulations.IsValidIndex(SimIndex))
		{
			Pawn->Simulations[SimIndex].SimTime.Stop();
		}*/
		TState = ESimThreadState::Error;
		TKill = true;
	}
}

void ADSim2Thread::PauseThread() 
{
	//UE_LOG(ThreadLog2, Log, TEXT("Thread2[%d] PauseThread call"), TID);
	TPause = true;
}

void ADSim2Thread::ContinueThread()
{
	//UE_LOG(ThreadLog2, Log, TEXT("Thread2[%d] ContinueThread call"), TID);
	TPause = false;

	if (TSemaphore != nullptr)
	{
		//Here is a FEvent signal "Trigger()" -> it will wake up the thread.
		TSemaphore->Trigger();
	}
}

void ADSim2Thread::Stop()
{
	//UE_LOG(ThreadLog2, Log, TEXT("Thread2[%d] Stop call"), TID);
	TKill = true; //Thread kill condition "while (!m_Kill){...}"
	TPause = false;

	for (int i = 0; i < TNum; ++i) {
		if (TSemaphore != nullptr)
		{
			//We shall signal "Trigger" the FEvent (in case the Thread is sleeping it shall wake up!!)
			TSemaphore->Trigger();
		}
	}
}

//Use this method to kill the thread!!
void ADSim2Thread::EnsureCompletion()
{
	//UE_LOG(ThreadLog2, Log, TEXT("Thread2[%d] EnsureCompletion call"), TID);
	Stop();
	if (Thread != nullptr)
	{
		Thread->WaitForCompletion();
	}
}

//-------------------ADSim2Thread2-------------End

AADSim2PlayerController::AADSim2PlayerController(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{

}

void AADSim2PlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AADSim2PlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AADSim2PlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}
