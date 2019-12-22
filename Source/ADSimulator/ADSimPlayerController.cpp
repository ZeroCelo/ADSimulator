// Fill out your copyright notice in the Description page of Project Settings.

/*
	Avaliação e Desempenho 2019-1
	Trabalho de Simulação

	Alunos:
		Marcelo Araujo Carvalho	DRE 109062115
*/

#include "ADSimPlayerController.h"

#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Runtime/Core/Public/Misc/Paths.h"

#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

//Macro para geração do log no editor
DEFINE_LOG_CATEGORY(ThreadLog);

//-----------------------Variáveis estáticas para comunicação entre threads
FThreadSharedData ADSimThread::ThreadData;
FCriticalSection ADSimThread::TMutexState;
FCriticalSection ADSimThread::TMutexMetric;
FCriticalSection ADSimThread::TMutexOutList;
FCriticalSection ADSimThread::TMutexEvents;
FThreadSafeBool ADSimThread::TPause = false;
FThreadSafeBool ADSimThread::TKill = false;
FEvent* ADSimThread::TSemaphore = nullptr;
//-----------------------Variáveis estáticas para comunicação entre threads

//-----------------------Construtor
ADSimThread::ADSimThread(int32 ThreadID, int32 ThreadNum, FThreadConfig &NewConfig)
{
	TKill = false;
	TPause = false;
	bShouldPause = false;

	//Initialize FEvent (as a cross platform (Confirmed Mac/Windows))
	if(TSemaphore == nullptr)
		TSemaphore = FGenericPlatformProcess::GetSynchEventFromPool(false);;

	TConfig.CopyConfig(NewConfig);
	TChunkCurrent = 0;
	TChunk = TConfig.EventChunk;
	TSleepTime = TConfig.SleepTime;
	TID = ThreadID;
	TNum = ThreadNum;

	FString TName(FString::Printf(TEXT("ADSimThread%02d"), TID));
	Thread = FRunnableThread::Create(this, *TName, 0, TPri_BelowNormal);
}

//-----------------------Destrutor
ADSimThread::~ADSimThread()
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

bool ADSimThread::Init()
{
	UE_LOG(ThreadLog, Log, TEXT("Thread[%d] init.."), TID);
	
	//Init the Data 
	return true;
}

/*
	função onde ocorre todo o loop da thread até ela ser terminada, estruturada de forma que permita chamar a
	função de processamento e organizar as lógicas de sleep
*/
uint32 ADSimThread::Run()
{
	//Initial wait before starting
	FPlatformProcess::Sleep(0.03);
	UE_LOG(ThreadLog, Log, TEXT("Thread[%d] Begin Run.."), TID);

	bool bThreadAlone = (TNum <= 1);

	while (!TKill)
	{
		if (TPause)
		{
			UE_LOG(ThreadLog, Log, TEXT("Thread[%d] Pausing.."), TID);
			
			//will "sleep" the thread until it will get a signal "Trigger()"
			if (TSemaphore != nullptr)
				TSemaphore->Wait();
			else
				return -10;

			UE_LOG(ThreadLog, Log, TEXT("Thread[%d] Unpause.."), TID);
			bShouldPause = false;
			ThreadData.LastPauseIteration = -3;
			ThreadData.LastPauseRound = -3;
		}
		else
		{
			if (!bThreadAlone){
				//ProcessTogether(); //Descontinuado
			}
			else{
				//Chama a função principal de simulação
				ProcessAlone2();
			}
			++TChunkCurrent; //Incrementa o número de eventos processado

			if (bShouldPause){
				//UE_LOG(ThreadLog, Log, TEXT("Thread[%d] Should"), TID);
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

	UE_LOG(ThreadLog, Log, TEXT("Thread[%d] End run.."), TID);

	return 0;
}

void ADSimThread::PauseThread(){
	//UE_LOG(ThreadLog, Log, TEXT("Thread[%d] Someone Paused me"), TID);
	TPause = true;
}

void ADSimThread::ContinueThread()
{
	TPause = false;
	
	if (TSemaphore != nullptr)
	{
			//Here is a FEvent signal "Trigger()" -> it will wake up the thread.
			TSemaphore->Trigger();
	}
}

void ADSimThread::Stop()
{
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
void ADSimThread::EnsureCompletion()
{
	Stop();
	if (Thread != nullptr)
	{
		Thread->WaitForCompletion();
	}
}

/*
	Função que checa os estado da simulação e processa os devidos eventos
*/
void ADSimThread::ProcessAlone2()
{
	UE_LOG(ThreadLog, Log, TEXT("TID[%d] alone critical.."), TID);

	if (ThreadData.CurrentArrivalTimeStamp <= ThreadData.CurrentServiceTimeStamp ||
		ThreadData.EventsList.Num() < 1	) 
	{
		/*
			Quer dizer que não há chegadas o suficiente para o processamento dos serviços, então geramos 
			uma nova chegada até que tenha o suficiente. Aproveitamos para gerar os números aleatórios
			tanto para chegada quanto para o serviço relacionado a este cliente
		*/
		UE_LOG(ThreadLog, Log, TEXT("TID[%d] New Arrival.."), TID);
		FIterationEvent NewEvent;
		NewEvent.CurrentRound = ThreadData.CurrentRound;
		NewEvent.CurrentIteration = ThreadData.CurrentIteration++;
		NewEvent.ArrivalValue = ThreadData.GetArrivalRand();
		NewEvent.ServiceValue = ThreadData.GetServiceRand();

		NewEvent.ArrivalTime = ThreadData.CurrentArrivalTimeStamp + NewEvent.ArrivalValue;
		ThreadData.CurrentArrivalTimeStamp = NewEvent.ArrivalTime;

		// Se for o 1º evento de todos, a entrada em serviço será imediata
		if (ThreadData.CurrentServiceTimeStamp == 0.0) {
			ThreadData.CurrentServiceTimeStamp = ThreadData.CurrentArrivalTimeStamp;
		}

		// Adiciona chegada na lista de eventos, executa de forma thread-safe
		ADSimThread::TMutexEvents.Lock();
		ThreadData.EventsList.Add(NewEvent);
		ADSimThread::TMutexEvents.Unlock();
	}
	else 
	{
		/*
			Se a última chegada já foi inserida na lista de eventos, temos que processar os 
			estados relacionados ao serviço
		*/
		if (ThreadData.CurrentService.CurrentRound < 0) {
			/*
				Não há cliente no servidor, devemo adicioná-lo
			*/
			UE_LOG(ThreadLog, Log, TEXT("TID[%d] No Service"), TID);
			if (ThreadData.CurrentRunConfig.bFirstServed) {
				/*
					Caso seja FCFS, pegamos o 1º evento da fila de espera e o removemos, thread-safe
				*/
				ADSimThread::TMutexEvents.Lock();

				if (ThreadData.EventsList.Num()) {
					ThreadData.CurrentService.ReceiveIteration(ThreadData.EventsList[0]);
					ThreadData.EventsList.RemoveAt(0);
				}

				ADSimThread::TMutexEvents.Unlock();
			}
			else {
				/*
					Caso seja LCFS, pegamos o último evento da fila de espera e o removemos, 
					lembramos que o último evento do array é uma chegada no futuro, então temos que 
					pegar o penúltimo evento caso mais do que 1 evento na lista
					thread-safe
				*/
				ADSimThread::TMutexEvents.Lock();

				if (ThreadData.EventsList.Num() > 1) {
					int32 LCFSIndex = ThreadData.EventsList.Num() - 2;
					ThreadData.CurrentService.ReceiveIteration(ThreadData.EventsList[LCFSIndex]);
					ThreadData.EventsList.RemoveAt(LCFSIndex);
				}
				else if (ThreadData.EventsList.Num() > 0) {
					ThreadData.CurrentService.ReceiveIteration(ThreadData.EventsList.Pop());
				}

				ADSimThread::TMutexEvents.Unlock();
			}
		}
		else {
			/*
				Existe um cliente sendo servido
			*/
			UE_LOG(ThreadLog, Log, TEXT("TID[%d] Has Service"), TID);
			if (ThreadData.CurrentService.ServiceTime <= 0.0) {
				/*
					Não foi setado o seu tempo de entrada em serviço ainda
				*/
				UE_LOG(ThreadLog, Log, TEXT("TID[%d] ..Svc Enter"), TID);
				ThreadData.CurrentService.ServiceTime = ThreadData.CurrentServiceTimeStamp;
				ThreadData.CurrentServiceTimeStamp = ThreadData.CurrentService.GetServiceEndTime();
			}
			else {
				UE_LOG(ThreadLog, Log, TEXT("TID[%d] ..old"), TID);

				/*
					Testes adicionais para certificar que o tempo de entrada em serviço está correto
				*/
				if (ThreadData.CurrentService.ServiceTime < ThreadData.CurrentService.ArrivalTime) {
					UE_LOG(ThreadLog, Log, TEXT("TID[%d] ..Svc < Arv"), TID);
					ThreadData.CurrentService.ServiceTime = ThreadData.CurrentService.ArrivalTime;
					ThreadData.CurrentServiceTimeStamp = ThreadData.CurrentService.GetServiceEndTime();
				}
				else {
					/*
						Entrada em serviço está correto entao devemos processar esta saída
					*/
					UE_LOG(ThreadLog, Log, TEXT("TID[%d] ..Svc >= Arv Out"), TID);
					OutCurrentEvent2(ThreadData.CurrentService);
				}
			}
		}
	}

	/*
		Ao final de tudo, verificamos se devemos pausar/sleep
	*/
	TestShouldPause2();
}

void ADSimThread::TestShouldPause2() {
	//Caso a configuração seja para pausar a todo passo da simulação
	if(TConfig.bAlwaysPause)
		bShouldPause = true;

	if (ThreadData.CurrentRound == 0) {
		/*
			Estamos na fase Transiente
		*/
		if (ThreadData.CurrentServiceIteration > ThreadData.TransientIterations) {
			/* 
				Término da fase transiente, espero a game thread me acordar pra iniciar a 
				1ª rodada da simulação
				Setup das variáveis de estado
			*/
			ThreadData.CurrentServiceIteration = 1;
			ThreadData.CurrentIteration = 1;
			//++ThreadData.CurrentRound;
			++ThreadData.CurrentServiceRound;

			ThreadData.RoundStartTime = ThreadData.LastOutEvent.GetServiceEndTime();

			ADSimThread::TMutexState.Lock();
			if (ThreadData.CurrentState != ERunState::Done)
				ThreadData.CurrentState = ERunState::Done;
			ADSimThread::TMutexState.Unlock();

			bShouldPause = true;
		}
	}
	else {
		/*
			Estamos em alguma rodada da simulação
		*/
		if (ThreadData.CurrentServiceIteration > ThreadData.CurrentRunConfig.Iterations) {
			/*
				Caso a simulação tenha processado o ultimo cliente que queremos, nós devemos iniciar 
				uma nova rodada
				setup das variaveis de estado
			*/
			++ThreadData.CurrentRound;

			ThreadData.CurrentIteration = 1;
			ThreadData.CurrentServiceIteration = 1;

			/*
				Término da coleta de dados para rodada atual, setamos variaveis para a coleta 
				da proxima rodada
			*/
			ThreadData.RoundStartTime = ThreadData.LastOutEvent.GetServiceEndTime();

			ThreadData.RoundWaitSum.Add(FIterationSum());
			ThreadData.RoundNqSumPmf.Add(FPMFSum());
			ThreadData.RoundNqSumPmf.Last().AddTimeBegin(ThreadData.RoundStartTime);
			
			if (ThreadData.RoundWaitSum.Num() > 1) {
				int32 LastRound = ThreadData.RoundWaitSum.Num() - 2;
				ThreadData.RunEWaitSum.AddValue(ThreadData.RoundWaitSum[LastRound].GetAverage());
				ThreadData.RunVWaitSum.AddValue(ThreadData.RoundWaitSum[LastRound].GetVariance());
			}

			if (ThreadData.RoundNqSumPmf.Num() > 1) {
				int32 LastRound = ThreadData.RoundNqSumPmf.Num() - 2;
				ThreadData.RunENqSum.AddValue(ThreadData.RoundNqSumPmf[LastRound].GetAverage());
				ThreadData.RunVNqSum.AddValue(ThreadData.RoundNqSumPmf[LastRound].GetVariance());
			}
		}

		if (ThreadData.CurrentRound > ThreadData.CurrentRunConfig.Rounds) {
			/*
				Encerramos esta simulação caso a rodada atual seja maior do que a configuração
				aguardamos a game thread iniciar a próxima simulação ou terminar completamente
			*/
			ADSimThread::TMutexState.Lock();
			if (ThreadData.CurrentState != ERunState::Done)
				ThreadData.CurrentState = ERunState::Done;
			ADSimThread::TMutexState.Unlock();

			bShouldPause = true;

			if (ThreadData.RoundWaitSum.Num() > ThreadData.CurrentRunConfig.Rounds)
				ThreadData.RoundWaitSum.Pop();
			if (ThreadData.RoundNqSumPmf.Num() > ThreadData.CurrentRunConfig.Rounds)
				ThreadData.RoundNqSumPmf.Pop();
		}
	}
}

void ADSimThread::OutCurrentEvent2(FIterationEvent& CurrentEv) 
{
	/*
		Função que retira um cliente do sistema pois seu serviço terminou, também devemos determinar se o 
		cliente faz parte da rodada atual para podermos calcular as devidas métricas
	*/
	UE_LOG(ThreadLog, Log, TEXT("TID[%d] out event2.."), TID);
	
	if (CurrentEv.CurrentRound >= 0) {
		/*
			início da coleta de dados dos delta tempo anteriores que não foram processados
		*/
		double LastDeltaTime = 0.0;
		TArray<double> AddNq; 
		TArray<double> AddDelta;

		if (ThreadData.EventsList.Num()) {
			double LastEvent = ThreadData.LastOutEvent.GetServiceEndTime();

			if (CurrentEv.ArrivalTime > LastEvent)
				LastEvent = CurrentEv.ArrivalTime;
			if (ThreadData.EventsList.Num() > 1) {

				for (int32 i = 0; i < ThreadData.EventsList.Num() - 1; ++i) {
					double LastNqEvent = ThreadData.EventsList[i].ArrivalTime;
					if (LastNqEvent > LastEvent) {
						double dif = LastNqEvent - LastEvent;
						UE_LOG(ThreadLog, Log, TEXT("TID[%d] Out for Nq=%.1f LastNq=%.3f LastEv=%.3f Delta=%.3f"), TID, (double)(i+1), LastNqEvent, LastEvent, dif);
						AddNq.Add(i + 1);
						AddDelta.Add(dif);
						LastEvent = LastNqEvent;
					}
				}
			}

			LastDeltaTime = CurrentEv.GetServiceEndTime() - LastEvent;

			CurrentEv.Nq = (double)(ThreadData.EventsList.Num() - 1);
			UE_LOG(ThreadLog, Log, TEXT("TID[%d] Out Current Nq=%.1f SvcEnd=%.3f LastEv=%.3f Delta=%.3f"), TID, CurrentEv.Nq, CurrentEv.GetServiceEndTime(), LastEvent, LastDeltaTime);
			AddNq.Add(CurrentEv.Nq);
			AddDelta.Add(LastDeltaTime);
		}
		/*
			Término da coleta de dados dos delta tempo anteriores que não foram processados
		*/

		/*
			Testamos se o cliente é válido para calculo da metrica ou se ele faz parte de outra rodada
		*/
		if (CurrentEv.CurrentRound <= ThreadData.CurrentRunConfig.Rounds 
			&& CurrentEv.CurrentRound == ThreadData.CurrentRound) {

			if (CurrentEv.CurrentRound == 0) {
				/*
					Calcula metricas da fase transiente; 
					Como a fase transiente é uma série única de valores, para cada cliente k acumulamos apenas 
					os valores do 1º até o k-ésimo inclusivo, dessa forma vemos a tendência dos valores da 
					média/variância no gráfico
				*/

				//Metrica de Espera
				if (ThreadData.TransientWaitSum.Num()) {
					FIterationSum NewWaitSum(ThreadData.TransientWaitSum.Last());
					NewWaitSum.AddValue(CurrentEv.GetWaitTime());
					ThreadData.TransientWaitSum.Add(NewWaitSum);
				}
				else {
					FIterationSum NewWaitSum;
					NewWaitSum.AddValue(CurrentEv.GetWaitTime());
					ThreadData.TransientWaitSum.Add(NewWaitSum);
				}
				//Metrica de Nq
				for (int32 i = 0; i < AddNq.Num() && AddNq.Num() == AddDelta.Num(); ++i) {
					if (ThreadData.TransientNqPmf.Num()) {
						FPMFSum NewNqPmf(ThreadData.TransientNqPmf.Last());
						NewNqPmf.AddValue(AddNq[i], AddDelta[i]);
						NewNqPmf.AddTimeEnd(CurrentEv.GetServiceEndTime());
						ThreadData.TransientNqPmf.Add(NewNqPmf);
					}
					else {
						FPMFSum NewNqPmf;
						NewNqPmf.AddValue(AddNq[i], AddDelta[i]);
						NewNqPmf.AddTimeEnd(CurrentEv.GetServiceEndTime());
						ThreadData.TransientNqPmf.Add(NewNqPmf);
					}
				}
			}
			else {
				/*
					Calcula metricas da rodada atual
				*/
				
				//Metrica de Espera
				if (ThreadData.RoundWaitSum.Num()) 
					ThreadData.RoundWaitSum.Last().AddValue(CurrentEv.GetWaitTime());
				//Metrica de Nq
				for (int32 i = 0; i < AddNq.Num() && AddNq.Num() == AddDelta.Num(); ++i) {
					if (ThreadData.RoundNqSumPmf.Num()) {
						ThreadData.RoundNqSumPmf.Last().AddValue(AddNq[i], AddDelta[i]);
						ThreadData.RoundNqSumPmf.Last().AddTimeEnd(CurrentEv.GetServiceEndTime());
					}
				}
			}

			/*
				Prepara o evento para os arquivos de saída e incrementa a variável que conta o número
				de saídas coletadas
			*/
			ADSimThread::TMutexOutList.Lock();
			if(CurrentEv.CurrentRound == 0 && ThreadData.bSaveTransientOut)
				ThreadData.OutList.Add(CurrentEv);
			else if(CurrentEv.CurrentRound > 0 && ThreadData.bSaveIterationOut)
				ThreadData.OutList.Add(CurrentEv);

			ADSimThread::TMutexOutList.Unlock();
			ThreadData.CurrentServiceRound = CurrentEv.CurrentRound;
			++ThreadData.CurrentServiceIteration;
		}

		/*
			Remove o cliente do sistema
		*/
		ThreadData.LastOutEvent.ReceiveIteration(CurrentEv);
		ThreadData.CurrentService.ServiceTime = 0.0;
		ThreadData.CurrentService.CurrentRound = -1;	//Rodada negativa quer dizer servidor vazio
	}
}

AADSimPlayerController::AADSimPlayerController(const FObjectInitializer& ObjectInitializer) 
	:Super(ObjectInitializer) 
{
	RoundDumpFileName = TEXT("RoundDump");
	IterationDumpFileName = TEXT("IterationDump");
	TransientDumpFileName = TEXT("TransientDump");
	RunDumpFileName = TEXT("RunDump");
	StartTime = FDateTime::Now();
	EndTime = FDateTime::Now();
	ElapsedPause.FromMilliseconds(0.0);
	bSaveDumpRound = true;
	bSaveDumpIteration = false;
	bSaveDumpTransient = false;
	bSaveDumpRun = true;
}

void AADSimPlayerController::BeginPlay() 
{
	ThreadPool.Reserve(GetCPUThreads());
	ElapsedHistory.Reserve(6);
	/*
		Inicializa Dados compartilhados
	*/
	ADSimThread::ThreadData = FThreadSharedData();
	ADSimThread::ThreadData.ResetRun();

	ADSimThread::ThreadData.EventsList.Empty();	ADSimThread::ThreadData.EventsList.Shrink();
	ADSimThread::ThreadData.OutList.Empty();	ADSimThread::ThreadData.OutList.Shrink();
	ADSimThread::ThreadData.RoundNqSumPmf.Empty();	ADSimThread::ThreadData.RoundNqSumPmf.Shrink();
	ADSimThread::ThreadData.RoundWaitSum.Empty();	ADSimThread::ThreadData.RoundWaitSum.Shrink();
	ADSimThread::ThreadData.TransientWaitSum.Empty();	ADSimThread::ThreadData.TransientWaitSum.Shrink();
	ADSimThread::ThreadData.TransientNqPmf.Empty();	ADSimThread::ThreadData.TransientNqPmf.Shrink();

	Super::BeginPlay();
}

void AADSimPlayerController::BeginDestroy()
{
	if (ThreadPool.Num() > 0)
	{
		for (auto& Thread : ThreadPool) {
			Thread->EnsureCompletion();
			delete Thread;
			Thread = nullptr;
		}
		ThreadPool.Empty(GetCPUThreads());
	}
	Super::BeginDestroy();
}

void AADSimPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ThreadPool.Num() > 0)
	{
		for (auto& Thread : ThreadPool) {
			Thread->EnsureCompletion();
			delete Thread;
			Thread = nullptr;
		}
		ThreadPool.Empty(GetCPUThreads());
	}

	/*
		limpeza dos Dados compartilhados
	*/
	ADSimThread::ThreadData.EventsList.Empty();	ADSimThread::ThreadData.EventsList.Shrink();
	ADSimThread::ThreadData.OutList.Empty();	ADSimThread::ThreadData.OutList.Shrink();
	ADSimThread::ThreadData.RoundNqSumPmf.Empty();	ADSimThread::ThreadData.RoundNqSumPmf.Shrink();
	ADSimThread::ThreadData.RoundWaitSum.Empty();	ADSimThread::ThreadData.RoundWaitSum.Shrink();
	ADSimThread::ThreadData.TransientWaitSum.Empty();	ADSimThread::ThreadData.TransientWaitSum.Shrink();
	ADSimThread::ThreadData.TransientNqPmf.Empty();	ADSimThread::ThreadData.TransientNqPmf.Shrink();

	Super::EndPlay(EndPlayReason);
}

/*
	Função da engine que é executada a cada frame renderizado, utilizada para checar se tem dados a serem
	gravados em arquivos em simultâneo como processamento
*/
void AADSimPlayerController::Tick(float DeltaSeconds) 
{
	Super::Tick(DeltaSeconds);

	ADSimThread::TMutexState.Lock();
	CurrentState = ADSimThread::ThreadData.CurrentState;
	ADSimThread::TMutexState.Unlock();

	switch (CurrentState) {
	case(ERunState::Done):
		if (GEngine != nullptr)
			GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Yellow, FString::Printf(TEXT("Run Restart")));

		UE_LOG(ThreadLog, Log, TEXT("Controller: Tick Done"));

		EndTime = FDateTime::Now();
		ElapsedPause += EndTime - StartTime;
		StartTime = EndTime;
		ElapsedHistory.Add(ElapsedPause);
		if (ElapsedHistory.Num() >= 6) {
			ElapsedHistory.RemoveAt(0);
		}
		SaveDumpsIteTra();
		SaveDumpRound();
		SaveDumpRun();
		if (ADSimThread::ThreadData.CurrentRound == 0) {
			ADSimThread::ThreadData.CurrentRound = 1;
			SetCurrentState(ERunState::Running);
			if(ThreadPool.Num() && ThreadPool[0] != nullptr)ThreadPool[0]->ContinueThread();
		}
		else {
			StartRun();
		}
		//SaveDumps();
		break;
	case(ERunState::RoundDone):
		break;
	case(ERunState::Running):
		EndTime = FDateTime::Now();
		SaveDumpsIteTra();
		break;
	case(ERunState::Paused):
		//ElapsedPause += EndTime - StartTime;
		//StartTime = EndTime;
		SaveDumpsIteTra();
		break;
	}
}

void AADSimPlayerController::SetCurrentState(ERunState NewState) {
	CurrentState = NewState;
	ADSimThread::TMutexState.Lock();
	ADSimThread::ThreadData.CurrentState = CurrentState;
	ADSimThread::TMutexState.Unlock();
}

//Pega o numero de Nucleos da Máquina
int32 AADSimPlayerController::GetCPUCores()
{
	return FPlatformMisc::NumberOfCores();
}

//Pega o numero de threads da Máquina
int32 AADSimPlayerController::GetCPUThreads()
{
	return FPlatformMisc::NumberOfCoresIncludingHyperthreads();
}

FString AADSimPlayerController::GetCPUText()
{
	int32 cores = FPlatformMisc::NumberOfCores();
	int32 cores_logical = FPlatformMisc::NumberOfCoresIncludingHyperthreads();

	FString ret = FString::Printf(TEXT("(%dC/%dT)"), cores, cores_logical);
	return ret;
}

//Pega configurações da interface gráfica
void AADSimPlayerController::PassRunConfig(TArray<FFlowParameter> Arrivals, TArray<FFlowParameter> Services, TArray<float> Rounds, TArray<float> Iterations, bool bFCFS, bool bLCFS)
{
	int32 DisciplineNum = 1;
	if (bFCFS && bLCFS) DisciplineNum = 2;

	int32 TotalCombinations = Arrivals.Num()*Services.Num()*Rounds.Num()*Iterations.Num()*DisciplineNum;
		if (GEngine != nullptr)
			GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Yellow,
				FString::Printf(TEXT("Pass Config Total=%d{Srv=%d;Arv=%d;Rnd=%d;Ite=%d;FCFS=%d}"), 
					TotalCombinations, Services.Num(), Arrivals.Num(),Rounds.Num(),Iterations.Num(),
					DisciplineNum
				)
			);

	RunConfigs.Empty();
	RunConfigs.Reserve(TotalCombinations);

	bool bLCFStemp = bLCFS;
	for (int32 i = 0; i < DisciplineNum; ++i) {
		for (auto& service : Services) {
			for (auto& arrival : Arrivals) {
				for (auto& round : Rounds) {
					for (auto& iteration : Iterations)
					{
						FRunConfig NewConfig;
						NewConfig.ServiceParam = service;
						NewConfig.ArrivalParam = arrival;
						NewConfig.Iterations = (int32)iteration;
						NewConfig.Rounds = (int32)round;
												
						NewConfig.bFirstServed = !bLCFStemp;
						
						RunConfigs.Add(NewConfig);
						if (GEngine != nullptr)
							GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Yellow, 
								FString::Printf(TEXT("NewConfig{Arv(%s)=%.3f;Srv(%s)=%.3f;Rnd=%d;Ite=%d;FCFS=%s}"), 
									(NewConfig.ArrivalParam.bExponential ? TEXT("M") : TEXT("D")), NewConfig.ArrivalParam.Value, 
									(NewConfig.ServiceParam.bExponential ? TEXT("M") : TEXT("D")), NewConfig.ServiceParam.Value,
									NewConfig.Rounds, NewConfig.Iterations,	(NewConfig.bFirstServed ? TEXT("true") : TEXT("false"))
								)
							);
					}
				}
			}
		}

		if (bLCFStemp) bLCFStemp = false;
	}
}

//Pega configuração da interface gráfica
void AADSimPlayerController::PassThreadConfig(FThreadConfig PassConfig)
{
	ThreadConfig = PassConfig;
	if (GEngine != nullptr)
		GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Yellow,
			FString::Printf(TEXT("Thread Config{Num=%d;Chk=%d;Slp=%.3f}"),
				ThreadConfig.NumThreads, ThreadConfig.EventChunk, ThreadConfig.SleepTime
			)
		);
}

//Gera texto para o log das saidas do sistema
FString AADSimPlayerController::GetRandomStr(bool bPrint)
{
	FString ret("No Output");
	FString ret2;
	if (bPrint) {
		ret = FString::Printf(TEXT("Rnd/Ite=[%04d/%05d]\nOuts={Num=%d,ArvTime=%.3f,SvcTime=%.3f}\nCurSvc={Rnd=%d,Ite=%d,ArvTime=%.3f,SvcTime=%.3f,SvcVal=%.3f,Nq=%.3f,EvNum=%d}\n"),
			ADSimThread::ThreadData.CurrentRound, ADSimThread::ThreadData.CurrentIteration,
			ADSimThread::ThreadData.OutList.Num(), ADSimThread::ThreadData.CurrentArrivalTimeStamp,
			ADSimThread::ThreadData.CurrentServiceTimeStamp,
			ADSimThread::ThreadData.CurrentService.CurrentRound, ADSimThread::ThreadData.CurrentService.CurrentIteration, ADSimThread::ThreadData.CurrentService.ArrivalTime,
			ADSimThread::ThreadData.CurrentService.ServiceTime,	ADSimThread::ThreadData.CurrentService.ServiceValue, 
			ADSimThread::ThreadData.CurrentService.Nq,	ADSimThread::ThreadData.EventsList.Num()
		);

		ret += FString::Printf(TEXT("LastSvc={ArvTime=%.3f,SvcTime=%.3f,SvcVal=%.3f,Nq=%.3f}\n"),
			ADSimThread::ThreadData.LastOutEvent.ArrivalTime,
			ADSimThread::ThreadData.LastOutEvent.ServiceTime,
			ADSimThread::ThreadData.LastOutEvent.ServiceValue,
			ADSimThread::ThreadData.LastOutEvent.Nq
		);
	
	
		if (ADSimThread::TMutexOutList.TryLock())
		{

			int32 startlimit = ADSimThread::ThreadData.OutList.Num() - 17;
			int32 forlimit = ADSimThread::ThreadData.OutList.Num();

			if (startlimit < 0)startlimit = 0;

			for (int32 i = startlimit; i < forlimit; ++i) {
				ret2 += FString::Printf(TEXT("Out[%d]={Rnd=%d,Ite=%d,ArvTime=%.3f,ArvVal=%.3f,SvcTime=%.3f,SvcVal=%.3f,Nq=%.3f}\n"),
					i, ADSimThread::ThreadData.OutList[i].CurrentRound, ADSimThread::ThreadData.OutList[i].CurrentIteration,
					ADSimThread::ThreadData.OutList[i].ArrivalTime, ADSimThread::ThreadData.OutList[i].ArrivalValue,
					ADSimThread::ThreadData.OutList[i].ServiceTime, ADSimThread::ThreadData.OutList[i].ServiceValue,
					ADSimThread::ThreadData.OutList[i].Nq
				);
			}
			ADSimThread::TMutexOutList.Unlock();
		}
	}

	ret += TEXT("\n");
	ret += ret2;
	
	return ret;
}

//Gera texto para o log da configuração da threads e lista de eventos
FString AADSimPlayerController::GetThreadStr(bool bPrint)
{
	FString RetOut("No Threads\n");
	
	
		if (ThreadPool.Num() > 0) {
			RetOut.Empty();
			for (auto& Thread : ThreadPool) {
				if (Thread != nullptr) {
					RetOut.Append(FString::Printf(TEXT("Thread[%d/%d]={ Pause=%s }"), Thread->GetTID(), Thread->GetTNum(),
						(Thread->IsThreadPaused() ? TEXT("true") : TEXT("false"))
						)
					);
					RetOut.Append(TEXT(" \n"));
				}
			}

			if (ThreadPool[0] != nullptr && ThreadPool[0]->IsThreadPaused())
				SetCurrentState(ERunState::Paused);
		}
		RetOut.Append(TEXT(" \n"));
	
	if (bPrint) {
		if (ADSimThread::TMutexEvents.TryLock())
		{
			int32 startlimit = ADSimThread::ThreadData.EventsList.Num() - 10;

			if (startlimit < 0)startlimit = 0;
			//int32 i = forlimit - 1;
			//if (forlimit) {
			for (int32 i = startlimit; i < ADSimThread::ThreadData.EventsList.Num(); ++i) {
				if (i < ADSimThread::ThreadData.EventsList.Num()) {
					RetOut += FString::Printf(TEXT("Event[%d]={Rnd=%d,Ite=%d,ArvTime=%.3f,ArvVal=%.3f,SvcTime=%.3f,SvcVal=%.3f,Nq=%.3f}\n"),
						i, ADSimThread::ThreadData.EventsList[i].CurrentRound, ADSimThread::ThreadData.EventsList[i].CurrentIteration,
						ADSimThread::ThreadData.EventsList[i].ArrivalTime, ADSimThread::ThreadData.EventsList[i].ArrivalValue,
						ADSimThread::ThreadData.EventsList[i].ServiceTime, ADSimThread::ThreadData.EventsList[i].ServiceValue,
						ADSimThread::ThreadData.EventsList[i].Nq
					);
				}
			}
			ADSimThread::TMutexEvents.Unlock();
		}
	}

	return RetOut;
}

//função para auxiliar gravação de arquivo durante desenvolvimento
void AADSimPlayerController::SaveToFile()
{
	if (ADSimThread::ThreadData.CurrentRound > 0) {
		FString FilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()) + TEXT("Dumps/") + *IterationDumpFileName + TEXT(".txt");
		FString FileContent = TEXT("Index,Round,Iteration,ArvTime,ArvVal,SvcTime,SvcVal,Nq\n");
		for (int32 i = 0; i < ADSimThread::ThreadData.OutList.Num(); ++i) {
			FileContent += FString::Printf(TEXT("%d,%d,%d,%f,%f,%f,%f,%f\n"),
				i, ADSimThread::ThreadData.OutList[i].CurrentRound, ADSimThread::ThreadData.OutList[i].CurrentIteration,
				ADSimThread::ThreadData.OutList[i].ArrivalTime, ADSimThread::ThreadData.OutList[i].ArrivalValue,
				ADSimThread::ThreadData.OutList[i].ServiceTime, ADSimThread::ThreadData.OutList[i].ServiceValue,
				ADSimThread::ThreadData.OutList[i].Nq
			);
		}

		FFileHelper::SaveStringToFile(FileContent, *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_None);
	}
	else {
		FString FilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()) + TEXT("Dumps/") + *TransientDumpFileName + TEXT(".txt");
		FString FileContent = TEXT("Index,Round,Iteration,ArvTime,ArvVal,SvcTime,SvcVal,Nq\n");
		for (int32 i = 0; i < ADSimThread::ThreadData.OutList.Num(); ++i) {
			FileContent += FString::Printf(TEXT("%d,%d,%d,%f,%f,%f,%f,%f\n"),
				i, ADSimThread::ThreadData.OutList[i].CurrentRound, ADSimThread::ThreadData.OutList[i].CurrentIteration,
				ADSimThread::ThreadData.OutList[i].ArrivalTime, ADSimThread::ThreadData.OutList[i].ArrivalValue,
				ADSimThread::ThreadData.OutList[i].ServiceTime, ADSimThread::ThreadData.OutList[i].ServiceValue,
				ADSimThread::ThreadData.OutList[i].Nq
			);
		}

		FFileHelper::SaveStringToFile(FileContent, *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_None);
	}
}

//abre diretório dos arquivos gerados
void AADSimPlayerController::OpenDumpFolder()
{
	FString FolderPathUrl = TEXT("file://") + FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()) + TEXT("Dumps/");
	if (GEngine != nullptr) {
		GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Yellow, FString::Printf(TEXT("Opening Folder: %s"), *FolderPathUrl));
	}
	FPlatformProcess::LaunchURL(*FolderPathUrl, nullptr, nullptr);
}

//função para salvar os arquivos das iterações e transiente
void AADSimPlayerController::SaveDumpsIteTra()
{
	ADSimThread::TMutexOutList.Lock();
	if (ADSimThread::ThreadData.OutList.Num()) {
		if (ADSimThread::ThreadData.OutList[0].CurrentRound == 0) {
			ADSimThread::TMutexOutList.Unlock();
			SaveDumpTransient();
		}
		else {
			ADSimThread::TMutexOutList.Unlock();
			SaveDumpIteration();
		}
	}
	else {
		ADSimThread::TMutexOutList.Unlock();
	}
		
}

//função para salvar o arquivo transiente
void AADSimPlayerController::SaveDumpTransient(){
	int32 MaxRemove = 50;

	if (bSaveDumpTransient) {
		FString TransientFullName = GetSomeDumpName(TransientDumpFileName, true);
		FString TransientFilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()) + TEXT("Dumps/") + *TransientFullName;

		FString FileContent;// = TEXT("Index,Round,Iteration,ArvTime,ArvVal,SvcTime,SvcVal,Nq\n");

		if (!TransientDumpCount)
			FileContent += TEXT("Round,Iteration,ArvTime,ArvVal,SvcTime,SvcVal,Nq\n");
		++TransientDumpCount;

		ADSimThread::TMutexOutList.Lock();
		int32 OutNum = ADSimThread::ThreadData.OutList.Num();
		//if (CurrentState != ERunState::Done)OutNum -= 3;
		//if (OutNum < 0)OutNum = 0;

		int32 OutRemoved = 0;

		for (int32 i = 0; i < OutNum; ++i) {
			FIterationEvent OutRef(ADSimThread::ThreadData.OutList[0]);
			ADSimThread::ThreadData.OutList.RemoveAt(0);

			FileContent += FString::Printf(TEXT("%d,%d,%f,%f,%f,%f,%f\n"),
				OutRef.CurrentRound, OutRef.CurrentIteration,
				OutRef.ArrivalTime, OutRef.ArrivalValue,
				OutRef.ServiceTime, OutRef.ServiceValue,
				OutRef.Nq
			);

			++OutRemoved;
			if (OutRemoved > MaxRemove)	break;
		}
		ADSimThread::TMutexOutList.Unlock();

		FFileHelper::SaveStringToFile(FileContent, *TransientFilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
	}
	else {
		ADSimThread::TMutexOutList.Lock();
		//ADSimThread::ThreadData.OutList.Empty();
		if (ADSimThread::ThreadData.OutList.Num()) {
			for (int32 i = 0; i < MaxRemove; ++i) {
				if (ADSimThread::ThreadData.OutList.Num())
					ADSimThread::ThreadData.OutList.RemoveAt(0);
				else
					break;
			}
		}
		ADSimThread::TMutexOutList.Unlock();
	}
}

//função para salvar o arquivo de iterações
void AADSimPlayerController::SaveDumpIteration() {
	int32 MaxRemove = 50;
	if (bSaveDumpIteration) {
		FString IterationFullName = GetSomeDumpName(IterationDumpFileName, true);
		FString IterationFilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()) + TEXT("Dumps/") + *IterationFullName;

		FString FileContent;// = TEXT("Index,Round,Iteration,ArvTime,ArvVal,SvcTime,SvcVal,Nq\n");

		if (!IterationDumpCount)
			FileContent += TEXT("Round,Iteration,ArvTime,ArvVal,SvcTime,SvcVal,Nq\n");
		++IterationDumpCount;

		ADSimThread::TMutexOutList.Lock();
		int32 OutNum = ADSimThread::ThreadData.OutList.Num();

		if (CurrentState == ERunState::Done)MaxRemove = OutNum;
		//if (OutNum < 0)OutNum = 0;

		int32 OutRemoved = 0;

		for (int32 i = 0; i < OutNum; ++i) {
			FIterationEvent OutRef(ADSimThread::ThreadData.OutList[0]);
			ADSimThread::ThreadData.OutList.RemoveAt(0);

			FileContent += FString::Printf(TEXT("%d,%d,%f,%f,%f,%f,%f\n"),
				OutRef.CurrentRound, OutRef.CurrentIteration,
				OutRef.ArrivalTime, OutRef.ArrivalValue,
				OutRef.ServiceTime, OutRef.ServiceValue,
				OutRef.Nq
			);

			++OutRemoved;
			if (OutRemoved > MaxRemove)	break;
		}
		ADSimThread::TMutexOutList.Unlock();

		FFileHelper::SaveStringToFile(FileContent, *IterationFilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
	}
	else {
		ADSimThread::TMutexOutList.Lock();
		//ADSimThread::ThreadData.OutList.Empty();
		if (ADSimThread::ThreadData.OutList.Num()) {
			for (int32 i = 0; i < MaxRemove; ++i) {
				if (ADSimThread::ThreadData.OutList.Num())
					ADSimThread::ThreadData.OutList.RemoveAt(0);
				else
					break;
			}
		}
		ADSimThread::TMutexOutList.Unlock();
	}
}

//função para salvar o arquivo das rodadas
void AADSimPlayerController::SaveDumpRound() {
	if (bSaveDumpRound) {
		if (ADSimThread::ThreadData.CurrentRound > 0) {
			UE_LOG(ThreadLog, Log, TEXT("Controller: Saving Round"));
			FString RoundFullName = GetSomeDumpName(RoundDumpFileName, true);
			FString RoundFilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()) + TEXT("Dumps/") + *RoundFullName;

			FString FileContent("Index,Round,E[W],V(W),E[Nq],V(Nq)\n");

			//ADSimThread::TMutexOutList.Lock();
			int32 OutNum = ADSimThread::ThreadData.RoundWaitSum.Num();

			for (int32 i = 0; i < OutNum; ++i) {
				FIterationSum WaitRef(ADSimThread::ThreadData.RoundWaitSum[i]);
				//ADSimThread::ThreadData.RoundWaitSum.RemoveAt(0);
				//FPMFSum NqRef(ADSimThread::ThreadData.RoundNqSumPmf[i]);
				FPMFSum NqRef(ADSimThread::ThreadData.RoundNqSumPmf[i]);
				//ADSimThread::ThreadData.RoundNqSumPmf.RemoveAt(0);

				FileContent += FString::Printf(TEXT("%d,%d,%f,%f,%f,%f\n"),
					i, WaitRef.CurrentRound,
					WaitRef.GetAverage(), WaitRef.GetVariance(),
					NqRef.GetAverage(), NqRef.GetVariance()
				);
			}
			//ADSimThread::TMutexOutList.Unlock();
			//ADSimThread::ThreadData.RoundNqSum.Empty();
			FFileHelper::SaveStringToFile(FileContent, *RoundFilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
		}
	}
	else {
		UE_LOG(ThreadLog, Log, TEXT("Controller: not Saving Round"));
		//ADSimThread::ThreadData.RoundWaitSum.Empty();
		//ADSimThread::ThreadData.RoundNqSum.Empty();
		//ADSimThread::ThreadData.RoundNqSumPmf.Empty();
	}
}

//função para salvar o arquivo de simulação
void AADSimPlayerController::SaveDumpRun() {
	if (CurrentState == ERunState::Done) {
		if (bSaveDumpRun && ADSimThread::ThreadData.CurrentRound > 0) {
			FString RunFullName = RunDumpFileName + CurrentRunDumpSuffix + TEXT(".txt");
			UE_LOG(ThreadLog, Log, TEXT("Controller Run Should %s"), *RunFullName);

			FString RunFilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()) + TEXT("Dumps/") + *RunFullName;

			FString FileContent;

			if (!RunDumpCount)
				FileContent += TEXT("FCFS, Rho, Rounds, Kmin, E[W], E[W] IC, V(W), V(W) IC, E[Nq], E[Nq] IC, V(Nq), V(Nq) IC, V(W) IC2,V(Nq) IC2\n");
			++RunDumpCount;

			double NRounds = ADSimThread::ThreadData.CurrentRunConfig.Rounds;
			double EW_Avg = ADSimThread::ThreadData.RunEWaitSum.GetAverage();
			double EW_Var = ADSimThread::ThreadData.RunEWaitSum.GetVariance();
			double IC_EW = 1.96*(FMath::Sqrt(EW_Var) / FMath::Sqrt(NRounds));
			double IC_EW_P = (EW_Avg - IC_EW) / (EW_Avg + IC_EW);
			IC_EW_P *= 100.0;

			double VW_Avg = ADSimThread::ThreadData.RunVWaitSum.GetAverage();
			double VW_Var = ADSimThread::ThreadData.RunVWaitSum.GetVariance();
			double IC_VW = 1.96*(FMath::Sqrt(VW_Var) / FMath::Sqrt(NRounds));
			double IC_VW_P = (VW_Avg - IC_VW) / (VW_Avg + IC_VW);
			IC_VW_P *= 100.0;

			double ENq_Avg = ADSimThread::ThreadData.RunENqSum.GetAverage();
			double ENq_Var = ADSimThread::ThreadData.RunENqSum.GetVariance();
			double IC_ENq = 1.96*(FMath::Sqrt(ENq_Var) / FMath::Sqrt(NRounds));
			double IC_ENq_P = (ENq_Avg - IC_ENq) / (ENq_Avg + IC_ENq);
			IC_ENq_P *= 100.0;

			double VNq_Avg = ADSimThread::ThreadData.RunVNqSum.GetAverage();
			double VNq_Var = ADSimThread::ThreadData.RunVNqSum.GetVariance();
			double IC_VNq = 1.96*(FMath::Sqrt(VNq_Var) / FMath::Sqrt(NRounds));
			double IC_VNq_P = (VNq_Avg - IC_VNq) / (VNq_Avg + IC_VNq);
			IC_VNq_P *= 100.0;

			double IC2_VW = ((NRounds - 1)*VW_Var) / 1.96;
			double IC2_VW_P = (VW_Var - IC2_VW) / (VW_Var + IC2_VW);
			double IC2_VNq = ((NRounds - 1)*VNq_Var) / 1.96;
			double IC2_VNq_P = (VNq_Var - IC2_VNq) / (VNq_Var + IC2_VNq);

			FileContent += FString::Printf(TEXT("%s,%.1f,%d,%d,%.4f,%.4f(%05.2f%%),%.4f,%.4f(%05.2f%%),%.4f,%.4f(%05.2f%%),%.4f,%.4f(%05.2f%%),%.4f(%05.2f%%),%.4f(%05.2f%%)\n"),
				(ADSimThread::ThreadData.CurrentRunConfig.bFirstServed ? TEXT("Sim") : TEXT("Nao")),
				ADSimThread::ThreadData.CurrentRunConfig.ArrivalParam.Value,
				ADSimThread::ThreadData.CurrentRunConfig.Rounds,
				ADSimThread::ThreadData.CurrentRunConfig.Iterations,
				EW_Avg,2.0*IC_EW,IC_EW_P,
				VW_Avg, 2.0*IC_VW,IC_VW_P,
				ENq_Avg, 2.0*IC_ENq,IC_ENq_P,
				VNq_Avg, 2.0*IC_VNq,IC_VNq_P,
				IC2_VW, IC2_VW_P,
				IC2_VNq, IC2_VNq_P
			);

			FFileHelper::SaveStringToFile(FileContent, *RunFilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
		}
	}
}

//função helper para o nome dos arquivos
FString AADSimPlayerController::GetSomeDumpName(const FString &FileName, bool bUseCurrent) {
	FString IterationFullName(TEXT("Ops.txt"));
	if (!bUseCurrent) {
		FDateTime DateNow = FDateTime::Now();
		FString TimeStr = FString::Printf(TEXT("%d_%04d%02d%02d-%02d%02d%02d"),
			RunConfigs.Num(),
			DateNow.GetYear(), DateNow.GetMonth(), DateNow.GetDay(),
			DateNow.GetHour(), DateNow.GetMinute(), DateNow.GetSecond()
		);

		IterationFullName = FileName + TimeStr + TEXT(".txt");
	}
	else {
		IterationFullName = FileName + CurrentDumpSuffix + TEXT(".txt");
	}
	return IterationFullName;
}

//função helper para pegar o tempo decorrido da simulação
FString AADSimPlayerController::GetElapsedRunTime() {
	FString Ret("HH:MM:SS.mmm");

	if (CurrentState != ERunState::Idle) {
		FTimespan Elapsed = (EndTime - StartTime) + ElapsedPause;
		Ret = FString::Printf(TEXT("%02d:%02d:%02d.%03d"),
			Elapsed.GetHours(),Elapsed.GetMinutes(),Elapsed.GetSeconds(), Elapsed.GetFractionMilli()
		);
	}
	return Ret;
}

//retorna texto com o historico de tempos de simulação
FString AADSimPlayerController::GetHistoryRunTime() {
	FString Ret;

	//if (ElapsedHistory.Num()) {
		for (int i = 1; i <= 5; ++i) {
			int32 tempi = ElapsedHistory.Num() - i;

			if (tempi >= 0 && tempi < ElapsedHistory.Num()) {
				FTimespan Elapsed = ElapsedHistory[tempi];
				Ret += FString::Printf(TEXT("[%02d]%02d:%02d:%02d.%03d\n"),
					RunConfigs.Num(),
					Elapsed.GetHours(), Elapsed.GetMinutes(), Elapsed.GetSeconds(), Elapsed.GetFractionMilli()
				);
			}
			else {
				Ret += TEXT("[XX]--:--:--.---\n");
			}
		}
	//}

	return Ret;
}

void AADSimPlayerController::GetTransientMetric(TArray<FVector2D> &Wait, TArray<FVector2D> &Nqueue)
{
	for (int32 i = 0; i < ADSimThread::ThreadData.TransientWaitSum.Num(); ++i) 
	{
		Wait.Add(FVector2D(
			ADSimThread::ThreadData.TransientWaitSum[i].GetAveragef(),
			ADSimThread::ThreadData.TransientWaitSum[i].GetVariancef()
			)
		);

		if (i < ADSimThread::ThreadData.TransientNqPmf.Num()) {
			Nqueue.Add(FVector2D(
				ADSimThread::ThreadData.TransientNqPmf[i].GetAveragef(),
				ADSimThread::ThreadData.TransientNqPmf[i].GetVariancef()
				)
			);
		}
	}
}

void AADSimPlayerController::GetRoundMetric(TArray<FVector2D> &Wait, TArray<FVector2D> &Nqueue) 
{
	for (int32 i = 0; i < ADSimThread::ThreadData.RoundWaitSum.Num(); ++i)
	{
		Wait.Add(FVector2D(
			ADSimThread::ThreadData.RoundWaitSum[i].GetAveragef(),
			ADSimThread::ThreadData.RoundWaitSum[i].GetVariancef()
			)
		);

		if (i < ADSimThread::ThreadData.RoundNqSumPmf.Num()) {
			Nqueue.Add(FVector2D(
				ADSimThread::ThreadData.RoundNqSumPmf[i].GetAveragef(),
				ADSimThread::ThreadData.RoundNqSumPmf[i].GetVariancef()
			)
			);
		}
	}
}

void AADSimPlayerController::GetRoundMetricEvolution(TArray<FVector2D> &Wait, TArray<FVector2D> &Nqueue)
{
	FIterationSum WaitTemp;
	FIterationSum NqTemp;
	for (int32 i = 0; i < ADSimThread::ThreadData.RoundWaitSum.Num() && ADSimThread::ThreadData.RoundWaitSum.Num() == ADSimThread::ThreadData.RoundNqSumPmf.Num(); ++i) 
	{
		
		WaitTemp.AddValue(ADSimThread::ThreadData.RoundWaitSum[i].GetAverage());
		NqTemp.AddValue(ADSimThread::ThreadData.RoundNqSumPmf[i].GetAverage());

		Wait.Add(FVector2D(
			WaitTemp.GetAveragef(),
			WaitTemp.GetVariancef()
		)
		);

		Nqueue.Add(FVector2D(
			NqTemp.GetAveragef(),
			NqTemp.GetVariancef()
		)
		);
	}
}

void AADSimPlayerController::StartRun()
{
	if (GEngine != nullptr)
		GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Red, FString::Printf(TEXT("Start Run")));

	if (RunConfigs.Num()) {
		ADSimThread::ThreadData.InitRunConfig(RunConfigs.Pop());
				
		FDateTime DateNow = FDateTime::Now();
		CurrentDumpSuffix = FString::Printf(TEXT("%d_%04d%02d%02d-%02d%02d%02d"),
			RunConfigs.Num(),
			DateNow.GetYear(), DateNow.GetMonth(), DateNow.GetDay(),
			DateNow.GetHour(), DateNow.GetMinute(), DateNow.GetSecond()
		);
		
		if (CurrentState == ERunState::Idle) {
			CurrentRunDumpSuffix = CurrentDumpSuffix;
			UE_LOG(ThreadLog, Log, TEXT("Controller Setting Run %s"), *CurrentRunDumpSuffix);
			RunDumpCount = 0;
			IterationDumpCount = 0;
			TransientDumpCount = 0;
		}

		StartTime = DateNow;
		ElapsedPause = FTimespan::FromMilliseconds(0.0);
		SetCurrentState(ERunState::Running);

		ADSimThread::ThreadData.bSaveIterationOut = bSaveDumpIteration;
		ADSimThread::ThreadData.bSaveTransientOut = bSaveDumpTransient;

		if (ThreadPool.Num() <= 0)
		{
			//Infelizmente não deu pra implementar mais que 1 thread de simulação
			/*for (int32 i = 0; i < ThreadConfig.NumThreads; ++i) {
				ThreadPool.Add(new ADSimThread(i, ThreadConfig.NumThreads, ThreadConfig.EventChunk,ThreadConfig.SleepTime));
			}*/
			ThreadPool.Add(new ADSimThread(0, 1, ThreadConfig));
		}
		else {
			for (int32 i = 0; i < ThreadPool.Num(); ++i) {
				if (ThreadPool[i] != nullptr) {
					ThreadPool[i]->ContinueThread();
				}
			}
		}
	}
	else {
		if (GEngine != nullptr)
			GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Red, FString::Printf(TEXT("Idle")));
		StopRun();
		//SetCurrentState(ERunState::Idle);
	}
}

void AADSimPlayerController::StopRun() 
{
	if (GEngine != nullptr)
		GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Red, FString::Printf(TEXT("Stop Run")));
	
	if (ThreadPool.Num() > 0)
	{
		for (auto& Thread : ThreadPool) {
			Thread->EnsureCompletion();
			delete Thread;
			Thread = nullptr;
		}
		ThreadPool.Empty(GetCPUThreads());
	}
	EndTime = FDateTime::Now();
	ElapsedPause += EndTime - StartTime;
	StartTime = EndTime;
	SetCurrentState(ERunState::Idle);
}

void AADSimPlayerController::PauseRun()
{
	if (GEngine != nullptr)
		GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Red, FString::Printf(TEXT("Pause Run")));
	
	if (ThreadPool.Num() > 0)
	{
		ThreadPool[0]->PauseThread();
	}

	EndTime = FDateTime::Now();
	ElapsedPause += EndTime - StartTime;
	StartTime = EndTime;
	SetCurrentState(ERunState::Paused);
}
void AADSimPlayerController::ResumeRun()
{
	if (GEngine != nullptr)
		GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Red, FString::Printf(TEXT("Resume Run")));
	
	if (ThreadPool.Num() > 0)
	{
		for (auto& Thread : ThreadPool) {
			if (Thread != nullptr) {
				Thread->ContinueThread();
			}
		}
	}
	StartTime = FDateTime::Now();
	SetCurrentState(ERunState::Running);
}

UADSimFunctionLibrary::UADSimFunctionLibrary(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{

}

bool UADSimFunctionLibrary::IsWithEditor() 
{
#if WITH_EDITOR
	return true;
#else
	return false;
#endif
}
