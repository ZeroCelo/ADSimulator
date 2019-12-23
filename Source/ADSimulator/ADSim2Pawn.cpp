// Fill out your copyright notice in the Description page of Project Settings.


#include "ADSim2Pawn.h"


#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Runtime/Core/Public/Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"

#include "EngineGlobals.h"

//Macro para geração do log no editor
DEFINE_LOG_CATEGORY(PawnLog2);

// Sets default values
AADSim2Pawn::AADSim2Pawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//NewObject<UMyObject>(this, TEXT("MyObj"));
	SimCurrentIndex.Set(-1);
	NumThreads = 1;
}

// Called when the game starts or when spawned
void AADSim2Pawn::BeginPlay()
{
	//MyMetric = NewObject< UADSim2MetricSingleSource >(this, TEXT("MetricObj"));
	if (MainWidgetClass != nullptr)
	{
		MainWidgetObj = CreateWidget<UADSim2MainWidget>(GetWorld(), MainWidgetClass);
		if (MainWidgetObj != nullptr)
		{
			MainWidgetObj->AddToViewport();
			ChartWidgetObj = MainWidgetObj->ChartWidgetObj;
		}
	}

	GetWorldTimerManager().SetTimer(ThreadCheckHandle, this, &AADSim2Pawn::CheckThread, 0.040f, true, 1.0f);

	FString DirFullPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() + TEXT("SimDumps/"));
	FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*DirFullPath);
	
	Super::BeginPlay();
}

void AADSim2Pawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ThreadPool.Num() > 0)
	{
		for (auto& Thread : ThreadPool) {
			Thread->EnsureCompletion();
			delete Thread;
			Thread = nullptr;
		}
		ThreadPool.Empty();
	}
	
	Super::EndPlay(EndPlayReason);
}

// Called every frame
void AADSim2Pawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AADSim2Pawn::CheckThread()
{
	//UE_LOG(PawnLog2, Log, TEXT("Pawn2[CheckThread] TNum=%d"), ThreadPool.Num());
	if (ThreadPool.Num())
	{
		int32 TDone = 0;
		for (int i = 0; i < ThreadPool.Num(); ++i)
		{
			switch (ThreadPool[i]->TState)
			{
			case ESimThreadState::Error:
				UE_LOG(PawnLog2, Log, TEXT("Pawn2[CheckThread] TNum=%d Error"), ThreadPool.Num());
				ThreadPool[i]->Stop();
				//StopSim();
				break;
			case ESimThreadState::JobDone:
				UE_LOG(PawnLog2, Log, TEXT("Pawn2[CheckThread] TNum=%d JobDone"), ThreadPool.Num());
				++TDone;
				//ThreadPool[i]->Stop();
				//StopSim();
				break;
			default:
				break;
			}
		}

		if (TDone >= ThreadPool.Num())
		{
			ThreadPool[0]->Stop();
			SaveSimDump();

			if (MainWidgetObj != nullptr && bPassMetricChart)
			{
				MainWidgetObj->SetChartRange();
			}

#if WITH_EDITOR
			//Test do Nq
			for (int32 i = 0; i < Simulations.Num(); ++i)
			{
				double Trans_ENq = Simulations[i].Metric1->TransNq1[i].GetAverage();
				double Trans_Begin = Simulations[i].Metric1->TransNq1[i].GetTimeBegin();
				double Trans_End = Simulations[i].Metric1->TransNq1[i].GetTimeEnd();
				UE_LOG(PawnLog2, Log, TEXT("Pawn2[CheckThread] Sim[%d] TransientNq={Begin=%.2f,End=%.2f, E[Nq]=%.2f}"), 
					i, Trans_Begin, Trans_End,	Trans_ENq
				);
				FSampleSum Test_ENq;
				for (int32 j = 0; j < Simulations[i].Metric1->RoundNq1[i].Num(); ++j)
				{
					double Round_ENq = Simulations[i].Metric1->RoundNq1[i][j].GetAverage();
					double Round_Begin = Simulations[i].Metric1->RoundNq1[i][j].GetTimeBegin();
					double Round_End = Simulations[i].Metric1->RoundNq1[i][j].GetTimeEnd();
					//if(j != (Simulations[i].Metric1->RoundNq1[i].Num() - 1))
						Test_ENq.AddValue(Round_ENq);
					UE_LOG(PawnLog2, Log, TEXT("Pawn2[CheckThread] Sim[%d] RoundNq[%d]={Begin=%.2f,End=%.2f, E[Nq]=%.2f}"),
						i, j, Round_Begin, Round_End, Round_ENq
					);
				}

				UE_LOG(PawnLog2, Log, TEXT("Pawn2[CheckThread] Sim[%d] SimNq={Test=%.3f, SimNq=%.3f}"),
					i, Test_ENq.GetAverage(), Simulations[i].Metric1->SimNq1[i].GetAverage()
				);
			}
#endif
		}
	}
}

bool AADSim2Pawn::IsSimDone()
{
	int32 Index = SimCurrentIndex.GetValue();
	if (Index > Simulations.Num() || Simulations.Num() <= 0)
		return true;
	return false;
}

bool AADSim2Pawn::HasSimStart()
{
	int32 Index = SimCurrentIndex.GetValue();
	if (Index < 0 || Index > Simulations.Num())
		return false;
	return true;
}

bool AADSim2Pawn::HasSimPaused()
{
	return ADSim2Thread::TPause;
}

ESimulatorState AADSim2Pawn::GetSimState(int32 SimIndex)
{
	if (Simulations.IsValidIndex(SimIndex))
		return Simulations[SimIndex].SimState;
	return ESimulatorState::Error;
}

FString AADSim2Pawn::GetSimTimeStr(int32 SimIndex)
{
	if (Simulations.IsValidIndex(SimIndex))
		return Simulations[SimIndex].SimTime.GetTimeStr();
	return TEXT("Epa");
}

FTimespan AADSim2Pawn::GetSimTime(int32 SimIndex)
{
	if (Simulations.IsValidIndex(SimIndex))
		return Simulations[SimIndex].SimTime.ElapsedTime;
	return FTimespan::FromSeconds(0.0);
}

FTimespan AADSim2Pawn::GetSimTimeTotal()
{
	FTimespan sum = FTimespan::FromSeconds(0.0);
	
	if (Simulations.Num())
	{
		for (int i = 0; i < Simulations.Num(); ++i)
			sum += Simulations[i].SimTime.ElapsedTime;
	}

	return sum;
}

FString AADSim2Pawn::GetSimTimeTotalStr()
{
	FTimespan Total = GetSimTimeTotal();
	return FString::Printf(TEXT("%02d:%02d:%02d.%03d"),
		Total.GetHours(), Total.GetMinutes(),
		Total.GetSeconds(), Total.GetFractionMilli()
	);
}

void AADSim2Pawn::OpenDumpDir()
{
	FString FolderPathUrl = TEXT("file://") + FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()) + TEXT("SimDumps/");
	//FString FolderPathUrl = TEXT("file://") + FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir());
	FPlatformProcess::LaunchURL(*FolderPathUrl, nullptr, nullptr);
}

void AADSim2Pawn::SetFileDump(const FString &NewSimDump, const FString &NewRoundDump, const FString &NewIterationDump, const FString &NewTransientDump)
{
	FileCfg.SimDump = NewSimDump;
	FileCfg.RoundDump = NewRoundDump;
	FileCfg.IterationDump = NewIterationDump;
	FileCfg.TransientDump = NewTransientDump;

	FDateTime DateNow = FDateTime::Now();
	FString TimeStr = FString::Printf(TEXT("SimDumps/Dump_%04d%02d%02d_%02d%02d%02d/"),
		DateNow.GetYear(), DateNow.GetMonth(), DateNow.GetDay(),
		DateNow.GetHour(), DateNow.GetMinute(), DateNow.GetSecond()
	);

	FileCfg.DirStr = TimeStr;
}

void AADSim2Pawn::SetFileDumpBool(bool bNewSimDump, bool bNewRoundDump, bool bNewIterationDump, bool bNewTransientDump)
{
	FileCfg.bSaveSimDump = bNewSimDump;
	FileCfg.bSaveRoundDump = bNewRoundDump;
	FileCfg.bSaveIterationDump = bNewIterationDump;
	FileCfg.bSaveTransientDump = bNewTransientDump;
}

void AADSim2Pawn::SetThreadSetting(int32 NThreads, int32 EventChunk, float SleepTime, bool bPauseAlways)
{
	TSetting.SleepTime = SleepTime;
	TSetting.EventChunk = EventChunk;
	TSetting.bAlwaysPause = bPauseAlways;
	NumThreads = NThreads;
}

void AADSim2Pawn::StartSim()
{
	UE_LOG(PawnLog2, Log, TEXT("Pawn2[StartSim] !HasSimStart=%d"), !HasSimStart());
	if (!HasSimStart()) 
	{
		//if (ChartWidgetObj != nullptr)ChartWidgetObj->ResetDataSourceBP();

		if (ThreadPool.Num())
		{
			for (auto& Thread : ThreadPool) {
				Thread->EnsureCompletion();
				delete Thread;
				Thread = nullptr;
			}
			ThreadPool.Empty(NumThreads);
		}
		
		for(int i = 0;i < NumThreads;++i)
			ThreadPool.Add(new ADSim2Thread(i, NumThreads, TSetting,this));
	}
}

void AADSim2Pawn::StopSim()
{
	UE_LOG(PawnLog2, Log, TEXT("Pawn2[StopSim] HasSimStart=%d"), HasSimStart());
	//if (HasSimStart()) 
	//{
		if (ThreadPool.Num())
		{
			for (auto& Thread : ThreadPool) {
				Thread->EnsureCompletion();
				delete Thread;
				Thread = nullptr;
			}
			ThreadPool.Empty(NumThreads);
		}
		SimCurrentIndex.Set(Simulations.Num() + 1);
	//}
}

void AADSim2Pawn::PauseSim()
{
	UE_LOG(PawnLog2, Log, TEXT("Pawn2[PauseSim]"));
	if (ThreadPool.Num())
	{
		ThreadPool[0]->PauseThread();
	}
}

void AADSim2Pawn::ResumeSim()
{
	UE_LOG(PawnLog2, Log, TEXT("Pawn2[ResumeSim]"));
	if (ThreadPool.Num())
	{
		for (auto& Thread : ThreadPool) {
			if (Thread != nullptr) {
				Thread->TConfig.bAlwaysPause = TSetting.bAlwaysPause;
				Thread->ContinueThread();
			}
		}
	}
}

void AADSim2Pawn::NextSim()
{
	UE_LOG(PawnLog2, Log, TEXT("Pawn2[NextSim]"));
	if (ThreadPool.Num())
	{
		for (auto& Thread : ThreadPool) {
			if (Thread != nullptr) {
				Thread->TConfig.bAlwaysPause = true;
				Thread->ContinueThread();
			}
		}
	}
}

void AADSim2Pawn::PassSimConfig(const TArray< FSimParameter > &Arrivals1, const TArray< FSimParameter > &Services1,
	const TArray< FSimParameter > &Arrivals2, const TArray< FSimParameter > &Services2,
	const TArray<int32> &Iterations, const TArray<int32> &Rounds,
	const TArray<int32> &Seeds, const int32 TransientIterations,
	const bool bQueue1Only, const bool bQueue1LCFS, const bool bQueue2LCFS
)
{
	UE_LOG(PawnLog2, Log, TEXT("Pawn2[PassSimConfig] {Arv1=%d;Svc1=%d;Arv2=%d;Svc2=%d;}"),
		Arrivals1.Num(), Services1.Num(), Arrivals2.Num(), Services2.Num()
	);
	UE_LOG(PawnLog2, Log, TEXT("Pawn2[PassSimConfig] {Ite=%d;Rnd=%d;Seed=%d;TIte=%d}"),
		Iterations.Num(), Rounds.Num(), Seeds.Num(), TransientIterations
	);
	UE_LOG(PawnLog2, Log, TEXT("Pawn2[PassSimConfig] {Q1Only=%d;Q1LCFS=%d;Q2LCFS=%d}"),
		bQueue1Only, bQueue1LCFS, bQueue2LCFS
	);
	int32 ExpectedConfigs = Arrivals1.Num()*Services1.Num()*Iterations.Num()*Rounds.Num()*Seeds.Num();
	SimCurrentIndex.Set(-1);
	int32 SimIndexCount = 0;

	Simulations.Empty();

	for (int32 i = 0; i < Seeds.Num(); ++i)
	{
		int32 Seed = Seeds[i];
		for (int32 j = 0; j < Rounds.Num(); ++j)
		{
			int32 Round = Rounds[j];
			for (int32 z = 0; z < Iterations.Num(); ++z)
			{
				int32 Iteration = Iterations[z];
				for (int32 a = 0; a < Arrivals1.Num(); ++a)
				{
					FSimParameter Arrival1(Arrivals1[a]);
					FSimParameter Arrival2(Arrivals2[a]);

					for (int32 s = 0; s < Services1.Num(); ++s)
					{
						FSimParameter Service1(Services1[s]);
						FSimParameter Service2(Services2[s]);

						FSimConfig SimCfg;
						SimCfg.ArrivalParam1.CopyParam(Arrival1);
						SimCfg.ArrivalParam2.CopyParam(Arrival2);
						SimCfg.ServiceParam1.CopyParam(Service1);
						SimCfg.ServiceParam2.CopyParam(Service2);
						SimCfg.Iterations = Iteration;
						SimCfg.Rounds = Round;
						SimCfg.RandStream.Initialize(Seed);
						SimCfg.TransientIterations = TransientIterations;
						SimCfg.bLCFS1 = bQueue1LCFS;
						SimCfg.bLCFS2 = bQueue2LCFS;
						SimCfg.bQueue1Only = bQueue1Only;

						Simulations.Add(FSimulator(SimCfg));
						Simulations.Last().FileConfig.CopySetting(FileCfg);
						Simulations.Last().SimIndex = SimIndexCount++;
						
						if (MainWidgetObj != nullptr)
						{
							Simulations.Last().Metric1 = &MainWidgetObj->SimData1;
							Simulations.Last().Metric2 = &MainWidgetObj->SimData2;
							Simulations.Last().Metric3 = &MainWidgetObj->SimData3;
						}

						UE_LOG(PawnLog2, Log, TEXT("Pawn2[PassSimConfig] Sim[%d]{A1=%.1f;A2=%.1f,S1=%.1f,S2=%.1f,Ite=%d,Rnd=%d,TIte=%d,Q1Only=%d,LCFS1=%d,LCFS2=%d,Seed=%d}"), Simulations.Num() - 1,
							Simulations.Last().Config.ArrivalParam1.Value, Simulations.Last().Config.ArrivalParam2.Value,
							Simulations.Last().Config.ServiceParam1.Value, Simulations.Last().Config.ServiceParam2.Value,
							Simulations.Last().Config.Iterations, Simulations.Last().Config.Rounds,
							Simulations.Last().Config.TransientIterations, Simulations.Last().Config.bQueue1Only,
							Simulations.Last().Config.bLCFS1, Simulations.Last().Config.bLCFS2,
							Simulations.Last().Config.RandStream.GetCurrentSeed()
						);
					}
				}
			}
		}
	}

	if (MainWidgetObj != nullptr)
	{
		MainWidgetObj->SetupDataNameW1(TEXT("W"), TEXT("Nq"), Simulations.Num());
		MainWidgetObj->SetupDataNameW2(TEXT("W"), TEXT("Nq"), Simulations.Num());
		MainWidgetObj->SetupDataNameW3(TEXT("W"), TEXT("Nq"), Simulations.Num());
	}

	if (bPassMetricChart)
	{
		for (int i = 0; i < Simulations.Num(); ++i)
		{
			Simulations[i].GenerateAnalytic();
		}
	}

	UE_LOG(PawnLog2, Log, TEXT("Pawn2[PassSimConfig] E[Config]=%d Configs.Num=%d"), ExpectedConfigs, Simulations.Num());
}

void AADSim2Pawn::TestSomething2()
{
	if (GEngine != nullptr)
		GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Yellow, FString::Printf(TEXT("Sim2Pawn: Test Smth2")));

}

void AADSim2Pawn::TestSomething()
{
	if (GEngine != nullptr)
		GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Yellow, FString::Printf(TEXT("Sim2Pawn: Test Smth")));

	if (ChartWidgetObj != nullptr)
	{
		if (GEngine != nullptr)
			GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Red, FString::Printf(TEXT("Sim2Pawn: Chart !null")));

	}
}

void AADSim2Pawn::SaveSimDump()
{
	if (FileCfg.bSaveSimDump && Simulations.Num())
	{
		FString SimWFullName = FileCfg.SimDump + TEXT("_W.txt");
		FString SimWFilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() + FileCfg.DirStr + SimWFullName);
		FString SimNqFullName = FileCfg.SimDump + TEXT("_Nq.txt");
		FString SimNqFilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() + FileCfg.DirStr + SimNqFullName);

		FString FileContentW("Round,Iterations,Lam1,Lam2,mu1,mu2,E[W1],+W1_IC,-W1_IC,W1 Analitico,E[W2],+W2_IC,-W2_IC,W2 Analitico,E[W],+W_IC,-W_IC,W Analitico\n");
		FString FileContentNq("Round,Iterations,Lam1,Lam2,mu1,mu2,E[Nq1],+Nq1_IC,-Nq1_IC,Nq1 Analitico,E[Nq2],+Nq2_IC,-Nq2_IC,Nq2 Analitico,E[Nq],+Nq_IC,-Nq_IC,Nq Analitico\n");

		FFileHelper::SaveStringToFile(FileContentW, *SimWFilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
		FFileHelper::SaveStringToFile(FileContentNq, *SimNqFilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
		
		for (int32 i = 0; i < Simulations.Num(); ++i)
		{
			if (Simulations[i].Metric1 != nullptr && Simulations[i].Metric2 != nullptr && Simulations[i].Metric3 != nullptr)
			{
				//UE_LOG(PawnLog2, Log, TEXT("Pawn2[SaveSimDump] Sim[%d]{SimW1.Num=%d, SimNq1.Num=%d}"), i, Simulations[i].Metric1->SimW1.Num(), Simulations[i].Metric1->SimNq1.Num());
				//UE_LOG(PawnLog2, Log, TEXT("Pawn2[SaveSimDump] Sim[%d]{RoundW1.Num=%d, RoundNq1.Num=%d}"), i, Simulations[i].Metric1->RoundW1.Num(), Simulations[i].Metric1->RoundNq1.Num());
				//UE_LOG(PawnLog2, Log, TEXT("Pawn2[SaveSimDump] Sim[%d]{TransW1.Num=%d, TransNq1.Num=%d}"), i, Simulations[i].Metric1->TransW1.Num(), Simulations[i].Metric1->TransNq1.Num());
				int32 SimRounds = Simulations[i].Config.Rounds;
				int32 SimIterations = Simulations[i].Config.Iterations;
				float SimLambda1 = Simulations[i].Config.ArrivalParam1.Value;
				float SimLambda2 = Simulations[i].Config.ArrivalParam2.Value;
				float SimMu1 = Simulations[i].Config.ServiceParam1.Value;
				float SimMu2 = Simulations[i].Config.ServiceParam2.Value;

				double EW1 = Simulations[i].Metric1->SimW1[i].GetAverage();
				double EW1_Anal = Simulations[i].GetAnalyticEW1();
				double EW2 = Simulations[i].Metric2->SimW1[i].GetAverage();
				double EW2_Anal = Simulations[i].GetAnalyticEW2();
				double EW = Simulations[i].Metric3->SimW1[i].GetAverage();
				double EW_Anal = Simulations[i].GetAnalyticEW3();
				
				double EW1_Up = Simulations[i].Metric1->SimW1[i].GetICUpper();
				double EW2_Up = Simulations[i].Metric2->SimW1[i].GetICUpper();
				double EW_Up = Simulations[i].Metric3->SimW1[i].GetICUpper();

				double EW_Low = Simulations[i].Metric3->SimW1[i].GetICLower();
				double EW2_Low = Simulations[i].Metric2->SimW1[i].GetICLower();
				double EW1_Low = Simulations[i].Metric1->SimW1[i].GetICLower();
				
				double ENq1 = Simulations[i].Metric1->SimNq1[i].GetAverage();
				double ENq1_Anal = Simulations[i].GetAnalyticENq1();
				double ENq2 = Simulations[i].Metric2->SimNq1[i].GetAverage();
				double ENq2_Anal = Simulations[i].GetAnalyticENq2();
				double ENq = Simulations[i].Metric3->SimNq1[i].GetAverage();
				double ENq_Anal = Simulations[i].GetAnalyticENq3();

				double ENq1_Up = Simulations[i].Metric1->SimNq1[i].GetICUpper();
				double ENq2_Up = Simulations[i].Metric2->SimNq1[i].GetICUpper();
				double ENq_Up = Simulations[i].Metric3->SimNq1[i].GetICUpper();
				
				double ENq1_Low = Simulations[i].Metric1->SimNq1[i].GetICLower();
				double ENq2_Low = Simulations[i].Metric2->SimNq1[i].GetICLower();
				double ENq_Low = Simulations[i].Metric3->SimNq1[i].GetICLower();

				FileContentW = FString::Printf(TEXT("%d,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n"),
					SimRounds, SimIterations, SimLambda1, SimLambda2, SimMu1, SimMu2,
					EW1, EW1_Up, EW1_Low, EW1_Anal,
					EW2, EW2_Up, EW2_Low, EW2_Anal,
					EW, EW_Up, EW_Low, EW_Anal
				);

				FileContentNq = FString::Printf(TEXT("%d,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n"),
					SimRounds, SimIterations, SimLambda1, SimLambda2, SimMu1, SimMu2,
					ENq1, ENq1_Up, ENq1_Low, ENq1_Anal,
					ENq2, ENq2_Up, ENq2_Low, ENq2_Anal,
					ENq, ENq_Up, ENq_Low, ENq_Anal
				);

				FFileHelper::SaveStringToFile(FileContentW, *SimWFilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
				FFileHelper::SaveStringToFile(FileContentNq, *SimNqFilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
			}
		}
	}
}
