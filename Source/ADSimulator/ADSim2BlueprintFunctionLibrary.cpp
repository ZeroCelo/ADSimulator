// Fill out your copyright notice in the Description page of Project Settings.

/*
	Avaliação e Desempenho 2019-2
	Trabalho de Simulação

	Alunos:
		Marcelo Araujo Carvalho	DRE 109062115
*/

#include "ADSim2BlueprintFunctionLibrary.h"

#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Runtime/Core/Public/Misc/Paths.h"

DEFINE_LOG_CATEGORY(Metric2);

bool UADSim2BlueprintFunctionLibrary::IsInEditor()
{
#if WITH_EDITOR
	return true;
#else
	return false;
#endif
}

int32 UADSim2BlueprintFunctionLibrary::GetCPUCores() { return FPlatformMisc::NumberOfCores(); }
int32 UADSim2BlueprintFunctionLibrary::GetCPUThreads() { return FPlatformMisc::NumberOfCoresIncludingHyperthreads(); }

void UADSim2BlueprintFunctionLibrary::OpenDumpDir()
{
	FString FolderPathUrl = TEXT("file://") + FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()) + TEXT("SimDumps/");
	//FString FolderPathUrl = TEXT("file://") + FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir());
	FPlatformProcess::LaunchURL(*FolderPathUrl, nullptr, nullptr);
}

FString UADSim2BlueprintFunctionLibrary::GetCPUText()
{
	int32 cores = FPlatformMisc::NumberOfCores();
	int32 cores_logical = FPlatformMisc::NumberOfCoresIncludingHyperthreads();

	FString ret = FString::Printf(TEXT("(%dC/%dT)"), cores, cores_logical);
	return ret;
}

double FSampleSum::GetICLower()
{
	double Var = GetVariance();
	double Avg = GetAverage();
	double N = GetNum();
	double IC = 0.0;
	if (N > 0.0) IC = 1.96*(FMath::Sqrt(Var) / FMath::Sqrt(N));
	return Avg - IC;
}

double FSampleSum::GetICUpper()
{
	double Var = GetVariance();
	double Avg = GetAverage();
	double N = GetNum();
	double IC = 0.0;
	if (N > 0.0) IC = 1.96*(FMath::Sqrt(Var) / FMath::Sqrt(N));
	return Avg + IC;
}

double FSampleSum::GetICPrecision()
{
	double Var = GetVariance();
	double Avg = GetAverage();
	double N = GetNum();
	double IC = 0.0;
	if (N > 0.0) IC = 1.96*(FMath::Sqrt(Var) / FMath::Sqrt(N));
	return (IC / Avg);
}

void FSimMetricData::LastRoundW1(int32 SimIndex)
{
	if (RoundW1.IsValidIndex(SimIndex))
	{
		RoundW1[SimIndex].Pop();
		if (RoundW1[SimIndex].Num())
			SimW1[SimIndex].AddValue(RoundW1[SimIndex].Last().GetAverage());

		SimW1ChartData[0].Points[SimIndex].Coords.Y = SimW1[SimIndex].GetAveragef();
		SimW1ChartData[1].Points[SimIndex].Coords.Y = SimW1[SimIndex].GetICLowerf();
		SimW1ChartData[2].Points[SimIndex].Coords.Y = SimW1[SimIndex].GetICUpperf();

		if (SimW1[SimIndex].GetICUpperf() > SimW1_Max[SimIndex].Y)SimW1_Max[SimIndex].Y = SimW1[SimIndex].GetICUpperf();
		if (SimW1[SimIndex].GetICLowerf() < SimW1_Min[SimIndex].Y)SimW1_Min[SimIndex].Y = SimW1[SimIndex].GetICLowerf();
	}
}

void FSimMetricData::LastRoundNq1(int32 SimIndex)
{
	if (RoundNq1.IsValidIndex(SimIndex))
	{
		RoundNq1[SimIndex].Pop();
		//if (RoundNq1[SimIndex].Num())
			//SimNq1[SimIndex].AddValue(RoundNq1[SimIndex].Last().GetAverage());

		SimNq1ChartData[0].Points[SimIndex].Coords.Y = SimNq1[SimIndex].GetAveragef();
		SimNq1ChartData[1].Points[SimIndex].Coords.Y = SimNq1[SimIndex].GetICLowerf();
		SimNq1ChartData[2].Points[SimIndex].Coords.Y = SimNq1[SimIndex].GetICUpperf();

		if (SimNq1[SimIndex].GetICUpperf() > SimNq1_Max[SimIndex].Y)SimNq1_Max[SimIndex].Y = SimNq1[SimIndex].GetICUpperf();
		if (SimNq1[SimIndex].GetICLowerf() < SimNq1_Min[SimIndex].Y)SimNq1_Min[SimIndex].Y = SimNq1[SimIndex].GetICLowerf();
	}
}

void FSimMetricData::AddRoundW1(int32 SimIndex)
{
	if (RoundW1.IsValidIndex(SimIndex))
	{
		if (RoundW1[SimIndex].Num())
			SimW1[SimIndex].AddValue(RoundW1[SimIndex].Last().GetAverage());
		RoundW1[SimIndex].Add(FSampleSum());

		SimW1ChartData[0].Points[SimIndex].Coords.Y = SimW1[SimIndex].GetAveragef();
		SimW1ChartData[1].Points[SimIndex].Coords.Y = SimW1[SimIndex].GetICLowerf();
		SimW1ChartData[2].Points[SimIndex].Coords.Y = SimW1[SimIndex].GetICUpperf();
		
		RoundW1ChartData[SimIndex].Points.Add(FKantanCartesianDatapoint());
		RoundW1ChartData[SimIndex].Points.Last().Coords.Set(RoundW1[SimIndex].Num() - 1, 0.0f);

		if (SimW1[SimIndex].GetICUpperf() > SimW1_Max[SimIndex].Y)SimW1_Max[SimIndex].Y = SimW1[SimIndex].GetICUpperf();
		if (SimW1[SimIndex].GetICLowerf() < SimW1_Min[SimIndex].Y)SimW1_Min[SimIndex].Y = SimW1[SimIndex].GetICLowerf();
		//if ((RoundW1[SimIndex].Num() - 1) > RoundW1_Max[SimIndex].X)RoundW1_Max[SimIndex].X = (RoundW1[SimIndex].Num() - 1);
		RoundW1_Max[SimIndex].X = (RoundW1[SimIndex].Num() - 1);
	}
}

void FSimMetricData::AddRoundNq1(int32 SimIndex)
{
	if (RoundNq1.IsValidIndex(SimIndex))
	{
		double LastRoundTime = 0.0;
		if (RoundNq1[SimIndex].Num()) 
		{
			SimNq1[SimIndex].AddValue(RoundNq1[SimIndex].Last().GetAverage());
			LastRoundTime = RoundNq1[SimIndex].Last().GetTimeEnd();
			UE_LOG(Metric2, Log, TEXT("FSimMetric[AddRoundNq1] SimNq1[%d] Avg=%.3f, End=%.3f"),
				SimIndex, RoundNq1[SimIndex].Last().GetAverage(), RoundNq1[SimIndex].Last().GetTimeEnd()
			);
		}
		else
		{
			LastRoundTime = TransNq1[SimIndex].GetTimeEnd();
		}
		
		RoundNq1[SimIndex].Add(FSampleTimeSum());
		RoundNq1[SimIndex].Last().SetTimeBegin(LastRoundTime);

		SimNq1ChartData[0].Points[SimIndex].Coords.Y = SimNq1[SimIndex].GetAveragef();
		SimNq1ChartData[1].Points[SimIndex].Coords.Y = SimNq1[SimIndex].GetICLowerf();
		SimNq1ChartData[2].Points[SimIndex].Coords.Y = SimNq1[SimIndex].GetICUpperf();
		RoundNq1ChartData[SimIndex].Points.Add(FKantanCartesianDatapoint());
		RoundNq1ChartData[SimIndex].Points.Last().Coords.Set(RoundNq1[SimIndex].Num() - 1, 0.0f);

		if (SimNq1[SimIndex].GetICUpperf() > SimNq1_Max[SimIndex].Y)SimNq1_Max[SimIndex].Y = SimNq1[SimIndex].GetICUpperf();
		if (SimNq1[SimIndex].GetICLowerf() < SimNq1_Min[SimIndex].Y)SimNq1_Min[SimIndex].Y = SimNq1[SimIndex].GetICLowerf();
		//if ((RoundNq1[SimIndex].Num() - 1) > RoundNq1_Max[SimIndex].X)RoundNq1_Max[SimIndex].X = (RoundNq1[SimIndex].Num() - 1);
		RoundNq1_Max[SimIndex].X = (RoundNq1[SimIndex].Num() - 1);
	}
}

void FSimMetricData::AddValueW1(double Value, int32 SimIndex)
{
	if (RoundW1.IsValidIndex(SimIndex))
	{
		RoundW1[SimIndex].Last().AddValue(Value);
		if (RoundW1ChartData[SimIndex].Points.Num())
			RoundW1ChartData[SimIndex].Points.Last().Coords.Y = RoundW1[SimIndex].Last().GetAveragef();
		
		if (RoundW1[SimIndex].Last().GetAveragef() < RoundW1_Min[SimIndex].Y)RoundW1_Min[SimIndex].Y = RoundW1[SimIndex].Last().GetAveragef();
		if (RoundW1[SimIndex].Last().GetAveragef() > RoundW1_Max[SimIndex].Y)RoundW1_Max[SimIndex].Y = RoundW1[SimIndex].Last().GetAveragef();
	}
}

void FSimMetricData::AddValueNq1(double Value, double dt, int32 SimIndex)
{
	if (RoundNq1.IsValidIndex(SimIndex))
	{
		RoundNq1[SimIndex].Last().AddValue(Value, dt);
		if (RoundNq1ChartData[SimIndex].Points.Num())
			RoundNq1ChartData[SimIndex].Points.Last().Coords.Y = RoundNq1[SimIndex].Last().GetAveragef();

		if (RoundNq1[SimIndex].Last().GetAveragef() < RoundNq1_Min[SimIndex].Y)RoundNq1_Min[SimIndex].Y = RoundNq1[SimIndex].Last().GetAveragef();
		if (RoundNq1[SimIndex].Last().GetAveragef() > RoundNq1_Max[SimIndex].Y)RoundNq1_Max[SimIndex].Y = RoundNq1[SimIndex].Last().GetAveragef();
	}
}

void FSimMetricData::AddValueW1Transient(double Value, int32 SimIndex)
{
	if (TransW1.IsValidIndex(SimIndex))
	{
		TransW1[SimIndex].AddValue(Value);
		TransW1ChartData[SimIndex].Points.Add(FKantanCartesianDatapoint());
		TransW1ChartData[SimIndex].Points.Last().Coords.Set(TransW1[SimIndex].GetNum() - 1, TransW1[SimIndex].GetAveragef());
		
		if (TransW1[SimIndex].GetAveragef() < TransW1_Min[SimIndex].Y)TransW1_Min[SimIndex].Y = TransW1[SimIndex].GetAveragef();
		if (TransW1[SimIndex].GetAveragef() > TransW1_Max[SimIndex].Y)TransW1_Max[SimIndex].Y = TransW1[SimIndex].GetAveragef();
		//if ((TransW1[SimIndex].GetNum() - 1) > TransW1_Max[SimIndex].X)TransW1_Max[SimIndex].X = (TransW1[SimIndex].GetNum() - 1);
		TransW1_Max[SimIndex].X = (TransW1[SimIndex].GetNum() - 1);
	}
}

void FSimMetricData::AddValueNq1Transient(double Value, double dt, int32 SimIndex)
{
	if (TransNq1.IsValidIndex(SimIndex))
	{
		TransNq1[SimIndex].AddValue(Value,dt);
		TransNq1ChartData[SimIndex].Points.Add(FKantanCartesianDatapoint());
		TransNq1ChartData[SimIndex].Points.Last().Coords.Set(TransNq1[SimIndex].GetTimeEndf(), TransNq1[SimIndex].GetAveragef());

		if (TransNq1[SimIndex].GetAveragef() < TransNq1_Min[SimIndex].Y)TransNq1_Min[SimIndex].Y = TransNq1[SimIndex].GetAveragef();
		if (TransNq1[SimIndex].GetAveragef() > TransNq1_Max[SimIndex].Y)TransNq1_Max[SimIndex].Y = TransNq1[SimIndex].GetAveragef();
		TransNq1_Max[SimIndex].X = TransNq1[SimIndex].GetTimeEndf();
	}
}

void FSimMetricData::SetupDataW1(FString NameStr, int32 MaxSim)
{
	TransW1_Max.Empty(MaxSim);
	TransW1_Min.Empty(MaxSim);
	RoundW1_Max.Empty(MaxSim);
	RoundW1_Min.Empty(MaxSim);
	SimW1_Max.Empty(MaxSim);
	SimW1_Min.Empty(MaxSim);
	
	RoundW1.Empty(MaxSim);
	SimW1.Empty(MaxSim);
	TransW1.Empty(MaxSim);
	TransW1ChartData.Empty(MaxSim);
	RoundW1ChartData.Empty(MaxSim);
	
	SimW1ChartData.Empty(4);
	//Avg
	SimW1ChartData.Add(FKantanSeriesData());
	SimW1ChartData.Last().Id = FName(*GetSampleNameICString(NameStr));
	SimW1ChartData.Last().Name = FText::FromString(*GetSampleNameICString(NameStr));
	//Lower
	SimW1ChartData.Add(FKantanSeriesData());
	SimW1ChartData.Last().Id = FName(*GetSampleNameICLowerString(NameStr));
	SimW1ChartData.Last().Name = FText::FromString(*GetSampleNameICLowerString(NameStr));
	//Upper
	SimW1ChartData.Add(FKantanSeriesData());
	SimW1ChartData.Last().Id = FName(*GetSampleNameICUpperString(NameStr));
	SimW1ChartData.Last().Name = FText::FromString(*GetSampleNameICUpperString(NameStr));
	//Analitico
	SimW1ChartData.Add(FKantanSeriesData());
	SimW1ChartData.Last().Id = FName(*GetSampleNameAnalyticString(NameStr));
	SimW1ChartData.Last().Name = FText::FromString(*GetSampleNameAnalyticString(NameStr));

	for (int i = 0; i < MaxSim; ++i)
	{
		RoundW1.Add(TArray< FSampleSum >());
		SimW1.Add(FSampleSum());
		TransW1.Add(FSampleSum());
		
		TransW1ChartData.Add(FKantanSeriesData());
		TransW1ChartData.Last().Id = FName(*GetSampleNameString(NameStr, i));
		TransW1ChartData.Last().Name = FText::FromString(*GetSampleNameString(NameStr, i));
		RoundW1ChartData.Add(FKantanSeriesData());
		RoundW1ChartData.Last().Id = FName(*GetSampleNameString(NameStr, i));
		RoundW1ChartData.Last().Name = FText::FromString(*GetSampleNameString(NameStr, i));
		
		SimW1ChartData[0].Points.Add(FKantanCartesianDatapoint());
		SimW1ChartData[0].Points.Last().Coords.Set(i,0.0f);
		SimW1ChartData[1].Points.Add(FKantanCartesianDatapoint());
		SimW1ChartData[1].Points.Last().Coords.Set(i, 0.0f);
		SimW1ChartData[2].Points.Add(FKantanCartesianDatapoint());
		SimW1ChartData[2].Points.Last().Coords.Set(i, 0.0f);
		SimW1ChartData[3].Points.Add(FKantanCartesianDatapoint());
		SimW1ChartData[3].Points.Last().Coords.Set(i, 0.0f);	//Analitico

		TransW1_Max.Add(FVector2D(-1000.0f,-1000.0f));
		TransW1_Min.Add(FVector2D(00.0f, 1000.0f));
		RoundW1_Max.Add(FVector2D(-1000.0f, -1000.0f));
		RoundW1_Min.Add(FVector2D(00.0f, 1000.0f));
		SimW1_Max.Add(FVector2D(MaxSim - 1, -1000.0f));
		SimW1_Min.Add(FVector2D(0.0f, 1000.0f));
	}
}

void FSimMetricData::SetupDataNq1(FString NameStr, int32 MaxSim)
{
	TransNq1_Max.Empty(MaxSim);
	TransNq1_Min.Empty(MaxSim);
	RoundNq1_Max.Empty(MaxSim);
	RoundNq1_Min.Empty(MaxSim);
	SimNq1_Max.Empty(MaxSim);
	SimNq1_Min.Empty(MaxSim);

	RoundNq1.Empty(MaxSim);
	SimNq1.Empty(MaxSim);
	TransNq1.Empty(MaxSim);
	TransNq1ChartData.Empty(MaxSim);
	RoundNq1ChartData.Empty(MaxSim);

	SimNq1ChartData.Empty();
	//Avg
	SimNq1ChartData.Add(FKantanSeriesData());
	SimNq1ChartData.Last().Id = FName(*GetSampleNameICString(NameStr));
	SimNq1ChartData.Last().Name = FText::FromString(*GetSampleNameICString(NameStr));
	SimNq1ChartData.Last().Points.Reserve(MaxSim);
	//Lower
	SimNq1ChartData.Add(FKantanSeriesData());
	SimNq1ChartData.Last().Id = FName(*GetSampleNameICLowerString(NameStr));
	SimNq1ChartData.Last().Name = FText::FromString(*GetSampleNameICLowerString(NameStr));
	SimNq1ChartData.Last().Points.Reserve(MaxSim);
	//Upper
	SimNq1ChartData.Add(FKantanSeriesData());
	SimNq1ChartData.Last().Id = FName(*GetSampleNameICUpperString(NameStr));
	SimNq1ChartData.Last().Name = FText::FromString(*GetSampleNameICUpperString(NameStr));
	SimNq1ChartData.Last().Points.Reserve(MaxSim);
	//Analitic
	SimNq1ChartData.Add(FKantanSeriesData());
	SimNq1ChartData.Last().Id = FName(*GetSampleNameAnalyticString(NameStr));
	SimNq1ChartData.Last().Name = FText::FromString(*GetSampleNameAnalyticString(NameStr));
	SimNq1ChartData.Last().Points.Reserve(MaxSim);

	for (int i = 0; i < MaxSim; ++i)
	{
		RoundNq1.Add(TArray< FSampleTimeSum >());
		SimNq1.Add(FSampleSum());
		TransNq1.Add(FSampleTimeSum());

		TransNq1ChartData.Add(FKantanSeriesData());
		TransNq1ChartData.Last().Id = FName(*GetSampleNameString(NameStr, i));
		TransNq1ChartData.Last().Name = FText::FromString(*GetSampleNameString(NameStr, i));
		RoundNq1ChartData.Add(FKantanSeriesData());
		RoundNq1ChartData.Last().Id = FName(*GetSampleNameString(NameStr, i));
		RoundNq1ChartData.Last().Name = FText::FromString(*GetSampleNameString(NameStr, i));

		SimNq1ChartData[0].Points.Add(FKantanCartesianDatapoint());
		SimNq1ChartData[0].Points.Last().Coords.Set(i, 0.0f);
		SimNq1ChartData[1].Points.Add(FKantanCartesianDatapoint());
		SimNq1ChartData[1].Points.Last().Coords.Set(i, 0.0f);
		SimNq1ChartData[2].Points.Add(FKantanCartesianDatapoint());
		SimNq1ChartData[2].Points.Last().Coords.Set(i, 0.0f);
		SimNq1ChartData[3].Points.Add(FKantanCartesianDatapoint());
		SimNq1ChartData[3].Points.Last().Coords.Set(i, 0.0f);	//Analitico

		TransNq1_Max.Add(FVector2D(-1000.0f, -1000.0f));
		TransNq1_Min.Add(FVector2D(0.0f, 1000.0f));
		RoundNq1_Max.Add(FVector2D(-1000.0f, -1000.0f));
		RoundNq1_Min.Add(FVector2D(0.0f, 1000.0f));
		SimNq1_Max.Add(FVector2D(MaxSim - 1, -1000.0f));
		SimNq1_Min.Add(FVector2D(0.0f, 1000.0f));
	}
}

FVector2D FSimMetricData::GetTransW1Min()
{
	float MinX = 1000.0f, MinY = 1000.0f;
	for (int i = 0; i < TransW1_Min.Num(); ++i)
	{
		if (TransW1_Min[i].Y < MinY) MinY = TransW1_Min[i].Y;
		if (TransW1_Min[i].X < MinX) MinX = TransW1_Min[i].X;
	}

	/*if (TransW1_Min.Num() < 2)
	{
		TransW1_Min.Last().Y -= FMath::Max(TransW1_Min.Last().Y,1.0f)*0.1f;
		TransW1_Min.Last().X -= FMath::Max(TransW1_Min.Last().X,1.0f)*0.1f;
	}*/

	return FVector2D(MinX, MinY);
}

FVector2D FSimMetricData::GetTransNq1Min()
{
	float MinX = 1000.0f, MinY = 1000.0f;
	for (int i = 0; i < TransNq1_Min.Num(); ++i)
	{
		if (TransNq1_Min[i].Y < MinY) MinY = TransNq1_Min[i].Y;
		if (TransNq1_Min[i].X < MinX) MinX = TransNq1_Min[i].X;
	}

	/*if (TransNq1_Min.Num() < 2)
	{
		TransNq1_Min.Last().Y -= FMath::Max(FMath::Abs(TransNq1_Min.Last().Y),1.0f)*0.1f;
		TransNq1_Min.Last().X -= FMath::Max(FMath::Abs(TransNq1_Min.Last().X),1.0f)*0.1f;
	}*/

	return FVector2D(MinX, MinY);
}

FVector2D FSimMetricData::GetTransW1Max()
{
	float MaxX = -1000.0f, MaxY = -1000.0f;
	for (int i = 0; i < TransW1_Max.Num(); ++i)
	{
		if (TransW1_Max[i].Y > MaxY) MaxY = TransW1_Max[i].Y;
		if (TransW1_Max[i].X > MaxX) MaxX = TransW1_Max[i].X;
	}

	/*if (TransW1_Max.Num() < 2)
	{
		TransW1_Max.Last().Y += FMath::Max(TransW1_Max.Last().Y,1.0f)*0.1f;
		TransW1_Max.Last().X += FMath::Max(TransW1_Max.Last().X,1.0f)*0.1f;
	}*/

	return FVector2D(MaxX, MaxY);
}

FVector2D FSimMetricData::GetTransNq1Max()
{
	float MaxX = -1000.0f, MaxY = -1000.0f;
	for (int i = 0; i < TransNq1_Max.Num(); ++i)
	{
		if (TransNq1_Max[i].Y > MaxY) MaxY = TransNq1_Max[i].Y;
		if (TransNq1_Max[i].X > MaxX) MaxX = TransNq1_Max[i].X;
	}

	/*if (TransNq1_Max.Num() < 2)
	{
		TransNq1_Max.Last().Y += FMath::Max(TransNq1_Max.Last().Y,1.0f)*0.1f;
		TransNq1_Max.Last().X += FMath::Max(TransNq1_Max.Last().X,1.0f)*0.1f;
	}*/

	return FVector2D(MaxX, MaxY);
}

FVector2D FSimMetricData::GetRoundW1Max()
{
	float MaxX = -1000.0f, MaxY = -1000.0f;
	for (int i = 0; i < RoundW1_Max.Num(); ++i)
	{
		if (RoundW1_Max[i].Y > MaxY) MaxY = RoundW1_Max[i].Y;
		if (RoundW1_Max[i].X > MaxX) MaxX = RoundW1_Max[i].X;
	}

	/*if (RoundW1_Max.Num() < 2)
	{
		RoundW1_Max.Last().Y += FMath::Max(RoundW1_Max.Last().Y,1.0f)*0.1f;
		RoundW1_Max.Last().X += FMath::Max(RoundW1_Max.Last().X,1.0f)*0.1f;
	}*/

	return FVector2D(MaxX, MaxY);
}

FVector2D FSimMetricData::GetRoundNq1Max()
{
	float MaxX = -1000.0f, MaxY = -1000.0f;
	for (int i = 0; i < RoundNq1_Max.Num(); ++i)
	{
		if (RoundNq1_Max[i].Y > MaxY) MaxY = RoundNq1_Max[i].Y;
		if (RoundNq1_Max[i].X > MaxX) MaxX = RoundNq1_Max[i].X;
	}

	/*if (RoundNq1_Max.Num() < 2)
	{
		RoundNq1_Max.Last().Y += FMath::Max(RoundNq1_Max.Last().Y,1.0f)*0.1f;
		RoundNq1_Max.Last().X += FMath::Max(RoundNq1_Max.Last().X,1.0f)*0.1f;
	}*/

	return FVector2D(MaxX, MaxY);
}

FVector2D FSimMetricData::GetRoundW1Min()
{
	float MinX = 1000.0f, MinY = 1000.0f;
	for (int i = 0; i < RoundW1_Min.Num(); ++i)
	{
		if (RoundW1_Min[i].Y < MinY) MinY = RoundW1_Min[i].Y;
		if (RoundW1_Min[i].X < MinX) MinX = RoundW1_Min[i].X;
	}

	/*if (RoundW1_Min.Num() < 2)
	{
		RoundW1_Min.Last().Y -= FMath::Max(FMath::Abs(RoundW1_Min.Last().Y),1.0f)*0.1f;
		RoundW1_Min.Last().X -= FMath::Max(FMath::Abs(RoundW1_Min.Last().X),1.0f)*0.1f;
	}*/

	return FVector2D(MinX,MinY);
}

FVector2D FSimMetricData::GetRoundNq1Min()
{
	float MinX = 1000.0f, MinY = 1000.0f;
	for (int i = 0; i < RoundNq1_Min.Num(); ++i)
	{
		if (RoundNq1_Min[i].Y < MinY) MinY = RoundNq1_Min[i].Y;
		if (RoundNq1_Min[i].X < MinX) MinX = RoundNq1_Min[i].X;
	}

	/*if (RoundNq1_Min.Num() < 2)
	{
		RoundNq1_Min.Last().Y -= FMath::Max(FMath::Abs(RoundNq1_Min.Last().Y),1.0f)*0.1f;
		RoundNq1_Min.Last().X -= FMath::Max(FMath::Abs(RoundNq1_Min.Last().X),1.0f)*0.1f;
	}*/

	return FVector2D(MinX, MinY);
}

FVector2D FSimMetricData::GetSimW1Max()
{
	float MaxX = -1000.0f, MaxY = -1000.0f;
	for (int i = 0; i < SimW1_Max.Num(); ++i)
	{
		if (SimW1_Max[i].Y > MaxY) MaxY = SimW1_Max[i].Y;
		if (SimW1_Max[i].X > MaxX) MaxX = SimW1_Max[i].X;
	}

	/*if (SimW1_Max.Num() < 2)
	{
		SimW1_Max.Last().Y += FMath::Max(SimW1_Max.Last().Y,1.0f)*0.1f;
		SimW1_Max.Last().X += FMath::Max(SimW1_Max.Last().X,1.0f)*0.1f;
	}*/

	return FVector2D(MaxX,MaxY);
}

FVector2D FSimMetricData::GetSimNq1Max()
{
	float MaxX = -1000.0f, MaxY = -1000.0f;
	for (int i = 0; i < SimNq1_Max.Num(); ++i)
	{
		if (SimNq1_Max[i].Y > MaxY) MaxY = SimNq1_Max[i].Y;
		if (SimNq1_Max[i].X > MaxX) MaxX = SimNq1_Max[i].X;
	}

	/*if (SimNq1_Max.Num() < 2)
	{
		SimNq1_Max.Last().Y += FMath::Max(SimNq1_Max.Last().Y,1.0f)*0.1f;
		SimNq1_Max.Last().X += FMath::Max(SimNq1_Max.Last().X,1.0f)*0.1f;
	}*/

	return FVector2D(MaxX, MaxY);
}

FVector2D FSimMetricData::GetSimW1Min()
{
	float MinX = 1000.0f, MinY = 1000.0f;
	for (int i = 0; i < SimW1_Min.Num(); ++i)
	{
		if (SimW1_Min[i].Y < MinY)MinY = SimW1_Min[i].Y;
		if (SimW1_Min[i].X < MinX)MinX = SimW1_Min[i].X;
	}

	/*if (SimW1_Min.Num() < 2)
	{
		SimW1_Min.Last().Y -= FMath::Max(FMath::Abs(SimW1_Min.Last().Y),1.0f)*0.1f;
		SimW1_Min.Last().X -= FMath::Max(FMath::Abs(SimW1_Min.Last().X),1.0f)*0.1f;
	}*/

	return FVector2D(MinX,MinY);
}

FVector2D FSimMetricData::GetSimNq1Min()
{
	float MinX = 1000.0f, MinY = 1000.0f;
	for (int i = 0; i < SimNq1_Min.Num(); ++i)
	{
		if (SimNq1_Min[i].Y < MinY)MinY = SimNq1_Min[i].Y;
		if (SimNq1_Min[i].X < MinX)MinX = SimNq1_Min[i].X;
	}

	/*if (SimNq1_Min.Num() < 2)
	{
		SimNq1_Min.Last().Y -= FMath::Max(FMath::Abs(SimNq1_Min.Last().Y),1.0f)*0.1f;
		SimNq1_Min.Last().X -= FMath::Max(FMath::Abs(SimNq1_Min.Last().X),1.0f)*0.1f;
	}*/

	return FVector2D(MinX, MinY);
}

FString FSimMetricData::GetSampleNameString(FString NameStr, int32 SampleIndex) 
{ 
	return FString::Printf(TEXT(" %s_%02d"), *NameStr, SampleIndex + 1); 
}

FString FSimMetricData::GetSampleNameICString(FString NameStr) 
{ 
	return FString::Printf(TEXT(" E[%s]"), *NameStr); 
}

FString FSimMetricData::GetSampleNameICUpperString(FString NameStr) 
{ 
	return FString::Printf(TEXT(" %s IC_Up"), *NameStr); 
}

FString FSimMetricData::GetSampleNameICLowerString(FString NameStr) 
{ 
	return FString::Printf(TEXT(" %s IC_Low"), *NameStr); 
}

FString FSimMetricData::GetSampleNameAnalyticString(FString NameStr)
{
	return FString::Printf(TEXT(" %s IC_Analytic"), *NameStr);
}
