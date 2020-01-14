// Fill out your copyright notice in the Description page of Project Settings.


#include "ADSim2GameModeBase.h"

#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Runtime/Core/Public/Misc/Paths.h"

DEFINE_LOG_CATEGORY(SimulatorLog2);

void FSimQueue::AddClient(FSimClient& Cli)
{
	if (Cli.IsValid())
	{
		if (bOnlyQueue1) //Fila única
		{
			Queue1.Push(FSimClient(Cli));
		}
		else //fila com prioridades 1 > 2
		{
			if (Cli.Priority != 0)Queue2.Push(FSimClient(Cli));
			else Queue1.Push(FSimClient(Cli));
		}
	}
}

FSimClient FSimQueue::PopClient()
{
	FSimClient Ret;	//Variavel de retorno

	if (HasClient())	//Tem alguem na fila?
	{
		if (bOnlyQueue1)	//Fila única
		{
			if (bLCFS1)	//LCFS 
			{
				Ret.CopyClient(Queue1.Pop());
			}
			else //FCFS
			{
				Ret.CopyClient(Queue1[0]);
				Queue1.RemoveAt(0);
			}
		}
		else
		{
			//Fila com prioridades 1 > 2
			if (Queue1.Num())
			{
				if (bLCFS1)	//LCFS 				
				{
					Ret.CopyClient(Queue1.Pop());
				}
				else //FCFS
				{
					Ret.CopyClient(Queue1[0]);
					Queue1.RemoveAt(0);
				}
			}
			else
			{
				if (bLCFS2)	//LCFS 				
				{
					Ret.CopyClient(Queue2.Pop());
				}
				else //FCFS
				{
					Ret.CopyClient(Queue2[0]);
					Queue2.RemoveAt(0);
				}
			}
		}
	}

	return Ret;
}

FSimClient FSimQueue::GetClient()
{
	FSimClient Ret;	//Variavel de retorno

	if (HasClient())	//Tem alguem na fila?
	{
		if (bOnlyQueue1)	//Fila única
		{
			if (bLCFS1)	//LCFS 
			{
				Ret.CopyClient(Queue1.Last());
			}
			else //FCFS
			{
				Ret.CopyClient(Queue1[0]);
			}
		}
		else
		{
			//Fila com prioridades 1 > 2
			if (Queue1.Num())
			{
				if (bLCFS1)	//LCFS 				
				{
					Ret.CopyClient(Queue1.Last());
				}
				else //FCFS
				{
					Ret.CopyClient(Queue1[0]);
				}
			}
			else
			{
				if (bLCFS2)	//LCFS 				
				{
					Ret.CopyClient(Queue2.Last());
				}
				else //FCFS
				{
					Ret.CopyClient(Queue2[0]);
				}
			}
		}
	}

	return Ret;
}

void FSimulator::Reset()
{
	EventList.Empty();
	LastEvent.Reset();
	SysQueue.Reset();
	SysService.Reset();
	LastArrivalTime1 = 0.0;
	LastArrivalTime2 = 0.0;
	LastServiceOut = 0.0;
	SysQueue.bLCFS1 = Config.bLCFS1;
	SysQueue.bLCFS2 = Config.bLCFS2;
	SysQueue.bOnlyQueue1 = Config.bQueue1Only;
	CurrentIteration = 0;
	CurrentRound = (Config.TransientIterations > 0) ? 0 : 1;
	SimState = ESimulatorState::Unfinished;
	Metric1 = nullptr;
	Metric2 = nullptr;
	Metric3 = nullptr;
}

void FSimulator::GenerateAnalytic()
{
	if (Metric1 != nullptr)
	{
		if (Metric1->SimW1ChartData.Num() > 3)
		{
			Metric1->SimW1ChartData[3].Points[SimIndex].Coords.Y = GetAnalyticEW1();
			Metric1->SimNq1ChartData[3].Points[SimIndex].Coords.Y = GetAnalyticENq1();
		}
	}

	if (Metric2 != nullptr)
	{
		if (Metric2->SimW1ChartData.Num() > 3)
		{
			Metric2->SimW1ChartData[3].Points[SimIndex].Coords.Y = GetAnalyticEW2();
			Metric2->SimNq1ChartData[3].Points[SimIndex].Coords.Y = GetAnalyticENq2();
		}
	}

	if (Metric3 != nullptr)
	{
		if (Metric3->SimW1ChartData.Num() > 3)
		{
			Metric3->SimW1ChartData[3].Points[SimIndex].Coords.Y = GetAnalyticEW3();
			Metric3->SimNq1ChartData[3].Points[SimIndex].Coords.Y = GetAnalyticENq3();
		}
	}
}

float FSimulator::GetAnalyticEW1()
{
	float EX1 = 1.0f / Config.ServiceParam1.Value;
	float EX2 = 1.0f / Config.ServiceParam2.Value;
	float lambda = Config.ArrivalParam1.Value + Config.ArrivalParam2.Value;
	float p1 = Config.ArrivalParam1.Value / lambda;
	float p2 = Config.ArrivalParam2.Value / lambda;
	float EX = (p1 * EX1) + (p2 * EX2);
	float rho1 = Config.ArrivalParam1.Value*EX1;
	float rho2 = Config.ArrivalParam2.Value*EX2;
	float rho = rho1 + rho2;
	float EXr = ((rho1 / rho)*EX1) + ((rho2 / rho)*EX2);

	if (!SysQueue.bLCFS1 && SysQueue.bOnlyQueue1)
	{
		float EW1 = (rho*EXr) / (1.0f - rho);
		if (Config.ArrivalParam1.Value <= 0.0) EW1 = 0.0;
		return EW1;
	}
	else if (!SysQueue.bLCFS1 && !SysQueue.bOnlyQueue1)
	{
		float EW1 = (rho*EXr) / (1.0f - rho1);
		if (Config.ArrivalParam1.Value <= 0.0) EW1 = 0.0;
		return EW1;
	}

	return -1.0f;
}

float FSimulator::GetAnalyticEW2()
{
	float EX1 = 1.0f / Config.ServiceParam1.Value;
	float EX2 = 1.0f / Config.ServiceParam2.Value;
	float lambda = Config.ArrivalParam1.Value + Config.ArrivalParam2.Value;
	float p1 = Config.ArrivalParam1.Value / lambda;
	float p2 = Config.ArrivalParam2.Value / lambda;
	float EX = (p1 * EX1) + (p2 * EX2);
	float rho1 = Config.ArrivalParam1.Value*EX1;
	float rho2 = Config.ArrivalParam2.Value*EX2;
	float rho = rho1 + rho2;
	float EXr = ((rho1 / rho)*EX1) + ((rho2 / rho)*EX2);

	if (!SysQueue.bLCFS2 && SysQueue.bOnlyQueue1)
	{
		float EW2 = (rho*EXr) / (1.0f - rho);
		if (Config.ArrivalParam2.Value <= 0.0) EW2 = 0.0;
		return EW2;
	}
	else if (!SysQueue.bLCFS2 && !SysQueue.bOnlyQueue1)
	{	
		float EW2 = (rho*EXr) / ((1.0f - rho1)*(1.0f - rho));
		if (Config.ArrivalParam2.Value <= 0.0) EW2 = 0.0;
		return EW2;
	}

	return -1.0;
}

float FSimulator::GetAnalyticEW3()
{
	float EX1 = 1.0f / Config.ServiceParam1.Value;
	float EX2 = 1.0f / Config.ServiceParam2.Value;
	float lambda = Config.ArrivalParam1.Value + Config.ArrivalParam2.Value;
	float p1 = Config.ArrivalParam1.Value / lambda;
	float p2 = Config.ArrivalParam2.Value / lambda;
	float EX = (p1 * EX1) + (p2 * EX2);
	float rho1 = Config.ArrivalParam1.Value*EX1;
	float rho2 = Config.ArrivalParam2.Value*EX2;
	float rho = rho1 + rho2;
	float EXr = ((rho1 / rho)*EX1) + ((rho2 / rho)*EX2);

	if (!SysQueue.bLCFS1 && !SysQueue.bLCFS2 && SysQueue.bOnlyQueue1)
	{
		float EW = (rho*EXr) / (1.0f - rho);
		return EW;
	}
	else if (!SysQueue.bLCFS1 && !SysQueue.bLCFS2 && !SysQueue.bOnlyQueue1)
	{
		float EW1 = (rho*EXr) / (1.0f - rho1);
		float EW2 = (rho*EXr) / ((1.0f - rho1)*(1.0f - rho));
		return (EW1 * p1) + (EW2 * p2);
	}

	return -1.0f;
}

float FSimulator::GetAnalyticENq1()
{
	float lambda1 = Config.ArrivalParam1.Value;
	float EW1 = GetAnalyticEW1();
	if (!SysQueue.bLCFS1 && SysQueue.bOnlyQueue1)
	{
		/*float EX1 = 1.0f / Config.ServiceParam1.Value;
		float EX2 = 1.0f / Config.ServiceParam2.Value;
		float rho1 = Config.ArrivalParam1.Value*EX1;
		float rho2 = Config.ArrivalParam2.Value*EX2;
		float rho = rho1 + rho2;*/

		float ENq1 = lambda1 * EW1;
		//ENq1 += rho;
		return ENq1;
	}
	else if (!SysQueue.bLCFS1 && !SysQueue.bOnlyQueue1)
	{
		float ENq1 = lambda1 * EW1;
		return ENq1;
	}

	return -1.0f;
}

float FSimulator::GetAnalyticENq2()
{
	float lambda2 = Config.ArrivalParam2.Value;
	float EW2 = GetAnalyticEW2();

	if (!SysQueue.bLCFS2 && SysQueue.bOnlyQueue1)
	{
		float ENq2 = lambda2 * EW2;
		return ENq2;
	}
	else if (!SysQueue.bLCFS2 && !SysQueue.bOnlyQueue1)
	{
		float ENq2 = lambda2 * EW2;
		return ENq2;
	}

	return -1.0f;
}

float FSimulator::GetAnalyticENq3()
{
	float ENq1 = GetAnalyticENq1();
	float ENq2 = GetAnalyticENq2();

	if (!SysQueue.bLCFS1 && !SysQueue.bLCFS2 && SysQueue.bOnlyQueue1)
	{
		return ENq1 + ENq2;
	}
	else if (!SysQueue.bLCFS1 && !SysQueue.bLCFS2 && !SysQueue.bOnlyQueue1)
	{
		return ENq1 + ENq2;
	}

	return -1.0f;
}

FString FSimulator::GetSimStateStr()
{
	FString Ret;
	switch (SimState)
	{
	case ESimulatorState::Finished: Ret = TEXT("Fin"); break;
	case ESimulatorState::Unfinished: Ret = TEXT("Unfin"); break;
	default: Ret = TEXT("Error"); break;
	}
	return Ret;
}

void FSimulator::AddEvent(ESimEventType NewType, double NewTime, double NewValue, int32 NewPriority)
{
	EventList.Add(FSimEvent(NewType, NewTime, NewValue, NewPriority));
	EventList.Sort();
}

void FSimulator::PrintEvents(FString Str)
{
	int32 MaxIteration = 3;
	if (EventList.Num() <= MaxIteration)MaxIteration = EventList.Num();
	for (int32 i = 0; i < MaxIteration; ++i)
	{
		FString EventType;
		switch (EventList[i].Type)
		{
		case ESimEventType::Arrival: EventType = TEXT("Arvl"); break;
		case ESimEventType::ServiceIn: EventType = TEXT("SvcIn"); break;
		case ESimEventType::ServiceOut: EventType = TEXT("SvcOut"); break;
		default: EventType = TEXT("None"); break;
		}
		UE_LOG(SimulatorLog2, Log, TEXT("FSim[%s] Event[%d]{Time=%.2f, Type=%s,Pri=%d}"), *Str, i, EventList[i].Time, *EventType, EventList[i].Priority);
	}
}

void FSimulator::GenerateArrivals()
{
	//UE_LOG(SimulatorLog2, Log, TEXT("Simulator[GenArrivals] "));
	double Arrival1 = 0.0;
	double Arrival2 = 0.0;

	if (LastArrivalTime1 <= LastServiceOut)Arrival1 = Config.GetArrivalRand1();
	if (LastArrivalTime2 <= LastServiceOut)Arrival2 = Config.GetArrivalRand2();
	
	if (Arrival1 > 0.0 && Arrival1 != INFINITY)
	{
		LastArrivalTime1 += Arrival1;
		AddEvent(ESimEventType::Arrival, LastArrivalTime1, Arrival1);
	}
	if (Arrival2 > 0.0 && Arrival2 != INFINITY)
	{
		LastArrivalTime2 += Arrival2;
		AddEvent(ESimEventType::Arrival, LastArrivalTime2, Arrival2, 1);
	}

	UE_LOG(SimulatorLog2, Log, TEXT("FSim[GenArrivals] Arv1=%.2f Last1=%.2f"), Arrival1, LastArrivalTime1);
	UE_LOG(SimulatorLog2, Log, TEXT("FSim[GenArrivals] Arv2=%.2f Last2=%.2f"),Arrival2,LastArrivalTime2);

	//Error Check?
	if (EventList.Num() <= 0)
	{
		UE_LOG(SimulatorLog2, Log, TEXT("FSim[GenArrivals] Error! "));
		SimState = ESimulatorState::Error;
	}
}

bool FSimulator::IsServiceOutLast()
{
	if (LastServiceOut > LastArrivalTime1 && LastArrivalTime1 > 0.0)
	{
		return true;
	}

	if (LastServiceOut > LastArrivalTime2 && LastArrivalTime2 > 0.0)
	{
		return true;
	}

	return false;
}

void FSimulator::Process()
{
	UE_LOG(SimulatorLog2, Log, TEXT("FSim[Process] Rnd=%d Ite=%d State=%s EvNum=%d bLastOut=%d "),CurrentRound,CurrentIteration,*GetSimStateStr(),EventList.Num(),IsServiceOutLast());
	if (EventList.Num() && !IsServiceOutLast())
	{
		//PrintEvents(TEXT("ProcessPrint"));
		FSimEvent CurrentEvent(EventList[0]);
		EventList.RemoveAt(0);
		
		switch (CurrentEvent.Type)
		{
		case ESimEventType::Arrival:
			ProcessArrival(CurrentEvent);
			break;
		case ESimEventType::ServiceIn:
			ProcessServiceIn(CurrentEvent);
			break;
		case ESimEventType::ServiceOut:
			ProcessServiceOut();
			break;
		default:	//error check?
			break;
		}
		
		LastEvent.CopyEvent(CurrentEvent);

		//Metricas de Nq
		if (LastEvent.Type != ESimEventType::None)
		{
			double dt = 0.0;

			if(EventList.Num())
				dt = EventList[0].Time - LastEvent.Time;

			//if(LastEvent.Type != ESimEventType::Arrival)
				//dt = CurrentEvent.Time - LastEvent.Time;
			
			double Nq1Value = SysQueue.NumQueue1();
			double Nq2Value = SysQueue.NumQueue2();
			double NqValue = Nq1Value + Nq2Value;

			/*UE_LOG(SimulatorLog2, Log, TEXT("Simulator[Process] Metric1=%s, Metric2=%s, Metric3=%s"),
				(Metric1 != nullptr) ? TEXT("Valid") : TEXT("Null"),
				(Metric2 != nullptr) ? TEXT("Valid") : TEXT("Null"),
				(Metric3 != nullptr) ? TEXT("Valid") : TEXT("Null")
			);*/

			UE_LOG(SimulatorLog2, Log, TEXT("Simulator[Process] Dt=%.2f Nq1=%.0f, Nq2=%.0f"),
				dt, Nq1Value, Nq2Value
			);
			
			if (CurrentRound == 0)
			{
				if (Metric1 != nullptr)Metric1->AddValueNq1Transient(Nq1Value, dt, SimIndex);
				if (Metric2 != nullptr)Metric2->AddValueNq1Transient(Nq2Value, dt, SimIndex);
				if (Metric3 != nullptr)Metric3->AddValueNq1Transient(NqValue, dt, SimIndex);
			}
			else
			{
				if (Metric1 != nullptr)Metric1->AddValueNq1(Nq1Value, dt, SimIndex);
				if (Metric2 != nullptr)Metric2->AddValueNq1(Nq2Value, dt, SimIndex);
				if (Metric3 != nullptr)Metric3->AddValueNq1(NqValue, dt, SimIndex);
			}
		}
		//LastEvent.CopyEvent(CurrentEvent);
		//PrintEvents(TEXT("ProcessPrint"));
	}
	else
	{
		GenerateArrivals();
	}

	TestEnding();
}

void FSimulator::TestEnding()
{
	if (CurrentRound > Config.Rounds)
	{
		UE_LOG(SimulatorLog2, Log, TEXT("FSim[TestEnding] Finished"));
		SimState = ESimulatorState::Finished;
		if (Metric1 != nullptr)
		{
			Metric1->LastRoundW1(SimIndex);
			Metric1->LastRoundNq1(SimIndex);
		}
		if (Metric2 != nullptr)
		{
			Metric2->LastRoundW1(SimIndex);
			Metric2->LastRoundNq1(SimIndex);
		}
		if (Metric3 != nullptr)
		{
			Metric3->LastRoundW1(SimIndex);
			Metric3->LastRoundNq1(SimIndex);
		}
	}
}

void FSimulator::ProcessArrival(const FSimEvent &CurrentEvent) 
{ 
	//UE_LOG(SimulatorLog2, Log, TEXT("FSim[ProcArrivals] "));
	FSimClient NewClient;
	NewClient.ArrivalTime = CurrentEvent.Time;
	NewClient.ArrivalValue = CurrentEvent.Value;
	NewClient.Priority = CurrentEvent.Priority;
	NewClient.ClientIteration = CurrentIteration++;
	NewClient.ClientRound = CurrentRound;
	
	if(NewClient.Priority != 0) NewClient.ServiceValue = Config.GetServiceRand2();
	else NewClient.ServiceValue = Config.GetServiceRand1();

	UE_LOG(SimulatorLog2, Log, TEXT("FSim[ProcArrivals] {Arv=%.2f,Svc=%.2f,Pri=%d}"),NewClient.ArrivalTime,NewClient.ServiceValue,NewClient.Priority);
	UE_LOG(SimulatorLog2, Log, TEXT("FSim[ProcArrivals] HasClient=%d, QNum=%d {%d + %d}"), SysQueue.HasClient(), SysQueue.NumClient(), SysQueue.NumQueue1(), SysQueue.NumQueue2());

	if (!SysQueue.HasClient() && !SysService.IsValid())
	{
		//NewClient.ServiceTime = NewClient.ArrivalTime;
		//EventList.Add(FSimEvent(ESimEventType::ServiceIn, NewClient.ArrivalTime, NewClient.ServiceValue));
		//EventList.Sort();
		AddEvent(ESimEventType::ServiceIn, NewClient.ArrivalTime, NewClient.ServiceValue, NewClient.Priority);
	}

	SysQueue.AddClient(NewClient);

	//PrintEvents(TEXT("ArrivalsPrint"));
}

void FSimulator::ProcessServiceIn(const FSimEvent &CurrentEvent)
{ 
	//UE_LOG(SimulatorLog2, Log, TEXT("FSim[ProcSvcIn] "));
	FSimClient QueueClient(SysQueue.PopClient());
	QueueClient.ServiceTime = CurrentEvent.Time;
	
	SysService.CopyClient(QueueClient);

	UE_LOG(SimulatorLog2, Log, TEXT("FSim[ProcSvcIn] QueuClient{Arv=%.2f,Svc=%.2f,SvcEnd=%.2f}"),SysService.ArrivalTime,SysService.ServiceTime,SysService.GetServiceEndTime());

	//EventList.Add(FSimEvent(ESimEventType::ServiceOut, QueueClient.GetServiceEndTime(), QueueClient.ServiceValue));
	//EventList.Sort();
	LastServiceOut = QueueClient.GetServiceEndTime();
	AddEvent(ESimEventType::ServiceOut, QueueClient.GetServiceEndTime(), QueueClient.ServiceValue,QueueClient.Priority);
}

void FSimulator::ProcessServiceOut()
{
	//UE_LOG(SimulatorLog2, Log, TEXT("FSim[ProcSvcOut] "));
	UE_LOG(SimulatorLog2, Log, TEXT("FSim[ProcSvcOut] Sys{ArvT=%.2f,SvcT=%.2f,SvcEnd=%.2f,Pri=%d}"),SysService.ArrivalTime,SysService.ServiceTime,SysService.GetServiceEndTime(),SysService.Priority);
	FSimClient OutClient(SysService);
	SysService.Reset();

	LastServiceOut = OutClient.GetServiceEndTime();
	
	//UE_LOG(SimulatorLog2, Log, TEXT("FSim[ProcSvcOut] HasClient=%d, QNum=%d {%d + %d}"), SysQueue.HasClient(), SysQueue.NumClient(), SysQueue.NumQueue1(), SysQueue.NumQueue2());

	if (SysQueue.HasClient())
	{
		FSimClient QueueClient(SysQueue.GetClient());
		//EventList.Add(FSimEvent(ESimEventType::ServiceIn, LastServiceOut, QueueClient.ServiceValue));
		//EventList.Sort();
		AddEvent(ESimEventType::ServiceIn, LastServiceOut, QueueClient.ServiceValue, QueueClient.Priority);
	}
	else 
	{
		GenerateArrivals();
	}

	double W = OutClient.GetWaitTime();

	if (OutClient.ClientRound == 0 && CurrentRound == 0)
	{
		//Metricas Transient
		if (OutClient.Priority == 0)
		{
			if (Metric1 != nullptr && Metric3 != nullptr)
			{
				Metric1->AddValueW1Transient(W, SimIndex);
				Metric3->AddValueW1Transient(W, SimIndex);
			}
			else {
				UE_LOG(SimulatorLog2, Log, TEXT("FSim[ProcSvcOut] Ai meu poha Metric1"));
			} //ErrorCheck?
		}
		else
		{
			if (Metric2 != nullptr && Metric3 != nullptr)
			{
				//UE_LOG(SimulatorLog2, Log, TEXT("FSim[ProcSvcOut] W2Trans[%d]=%.2f"),SimIndex,W);
				Metric2->AddValueW1Transient(W, SimIndex);
				Metric3->AddValueW1Transient(W, SimIndex);
			}
			else {
				UE_LOG(SimulatorLog2, Log, TEXT("FSim[ProcSvcOut] Ai meu poha Metric2"));
			} //ErrorCheck?
		}

		if (OutClient.ClientIteration > Config.TransientIterations)
		{
			++CurrentRound;
			CurrentIteration = 0;
			if (Metric1 != nullptr)
			{
				Metric1->AddRoundW1(SimIndex);
				Metric1->AddRoundNq1(SimIndex);
			}
			if (Metric2 != nullptr)
			{
				Metric2->AddRoundW1(SimIndex);
				Metric2->AddRoundNq1(SimIndex);
			}
			if (Metric3 != nullptr)
			{
				Metric3->AddRoundW1(SimIndex);
				Metric3->AddRoundNq1(SimIndex);
			}
		}
	} 
	else 
	{
		UE_LOG(SimulatorLog2, Log, TEXT("FSim[ProcSvcOut] OutClient{Rnd=%d,Ite=%d,Pri=%d"), OutClient.ClientRound, OutClient.ClientIteration, OutClient.Priority);
		
		if (OutClient.ClientRound == CurrentRound)
		{
			//Metricas
			if (OutClient.Priority == 0)
			{
				if (Metric1 != nullptr && Metric3 != nullptr)
				{
					Metric1->AddValueW1(W, SimIndex);
					Metric3->AddValueW1(W, SimIndex);
				}
				else {
					UE_LOG(SimulatorLog2, Log, TEXT("FSim[ProcSvcOut] Ai meu poha Metric1"));
				} //ErrorCheck?
			}
			else
			{
				if (Metric2 != nullptr && Metric3 != nullptr)
				{
					//UE_LOG(SimulatorLog2, Log, TEXT("FSim[ProcSvcOut] W2Round[%d]=%.2f"), SimIndex, W);
					Metric2->AddValueW1(W, SimIndex);
					Metric3->AddValueW1(W, SimIndex);
				}
				else {
					UE_LOG(SimulatorLog2, Log, TEXT("FSim[ProcSvcOut] Ai meu poha Metric2"));
				} //ErrorCheck?
			}

			if (OutClient.ClientIteration > Config.Iterations)
			{
				SaveRoundDump();
				++CurrentRound;
				CurrentIteration = 0;
				if (Metric1 != nullptr)
				{
					Metric1->AddRoundW1(SimIndex);
					Metric1->AddRoundNq1(SimIndex);
				}
				if (Metric2 != nullptr)
				{
					Metric2->AddRoundW1(SimIndex);
					Metric2->AddRoundNq1(SimIndex);
				}
				if (Metric3 != nullptr)
				{
					Metric3->AddRoundW1(SimIndex);
					Metric3->AddRoundNq1(SimIndex);
				}
			}
		}
	}	
}

void FSimulator::SaveRoundDump()
{
	if (FileConfig.bSaveRoundDump) {
		if (Metric1 != nullptr && Metric2 != nullptr && Metric3 != nullptr)
		{
			FString RoundFullName = FileConfig.RoundDump + FString::Printf(TEXT("%d.txt"), SimIndex);
			FString RoundFilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() + FileConfig.DirStr + RoundFullName);
			UE_LOG(SimulatorLog2, Log, TEXT("FSim[SaveDump] Saving file %s in %s"), *RoundFullName,*RoundFilePath);
			UE_LOG(SimulatorLog2, Log, TEXT("FSim[SaveDump] Round=%d"), CurrentRound);

			FString FileContent;
			if(CurrentRound <= 1)
				FileContent = TEXT("Round,E[W1],E[W2],E[W],E[Nq1],E[Nq2],E[Nq]\n");

			double EW1 = -1.0;
			double EW2 = -1.0;
			double EW = -1.0;
			double ENq1 = -1.0;
			double ENq2 = -1.0;
			double ENq = -1.0;

			if (Metric1->RoundW1.IsValidIndex(SimIndex))
			{
				EW1 = Metric1->RoundW1[SimIndex].Last().GetAverage();
				EW2 = Metric2->RoundW1[SimIndex].Last().GetAverage();
				EW = Metric3->RoundW1[SimIndex].Last().GetAverage();
			}

			if (Metric1->RoundNq1.IsValidIndex(SimIndex))
			{
				ENq1 = Metric1->RoundNq1[SimIndex].Last().GetAverage();
				ENq2 = Metric2->RoundNq1[SimIndex].Last().GetAverage();
				ENq = Metric3->RoundNq1[SimIndex].Last().GetAverage();
			}

			FileContent += FString::Printf(TEXT("%d,%f,%f,%f,%f,%f,%f\n"),
				CurrentRound, 
				EW1, EW2, EW,
				ENq1, ENq2, ENq
			);

			FFileHelper::SaveStringToFile(FileContent, *RoundFilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
		}
	}
}
