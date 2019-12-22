// Fill out your copyright notice in the Description page of Project Settings.
/**

	Avaliação e Desempenho 2019-2
	Trabalho de Simulação

	Alunos:
		Marcelo Araujo Carvalho	DRE 109062115
*/

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CartesianData.h"

//#include "ADSim2GameModeBase.h"

#include "ADSim2BlueprintFunctionLibrary.generated.h"

USTRUCT(BlueprintType)
struct FSimParameter
{
	GENERATED_BODY()

	//Se o parametro é exponencial, deterministico caso contrário
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bExponential;

	//Valor do parâmetro
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value;

	//Gerador de número aleatório
	float GetRandVal(FRandomStream& RandStream) {
		if (Value == 0.0) { return Value; }
		else if (!bExponential) { return 1.0f / Value; }
		else {
			float UniformVar = RandStream.GetFraction();
			return -(FGenericPlatformMath::Loge(1.0f - UniformVar) / Value);
		}
	}

	void CopyParam(const FSimParameter &NewParam)
	{
		bExponential = NewParam.bExponential;
		Value = NewParam.Value;
	}

	FSimParameter(const FSimParameter &NewParam) {
		CopyParam(NewParam);
	}

	FSimParameter() {
		Value = 0.5f;
		bExponential = true;
	}
};

USTRUCT(BlueprintType)
struct FSampleSum
{
	GENERATED_BODY()

	double ValueSum;
	double ValueSumSquare;
	double ValueMin;
	double ValueMax;
	int32 ValueSumTotal;

	double GetAverage() 
	{
		if (ValueSumTotal >= 1)	return ValueSum / ValueSumTotal;
		return 0.0;
	}
	
	double GetVariance() 
	{	//Formula -> Sum(X^2)/n-1 - Sum(X)*Sum(X)/n*n-1
		if (ValueSumTotal > 1) 
		{
			double TotalSum = (double)ValueSumTotal;
			double TotalSumMinus = TotalSum - 1.0;
			double Moment1Square = (ValueSum*ValueSum) / (TotalSum*TotalSumMinus);
			double Moment2 = ValueSumSquare / TotalSumMinus;
			return Moment2 - Moment1Square;
		}
		return 0.0;
	}

	double GetICLower();
	double GetICUpper();
	double GetICPrecision();
	double GetICPrecisionPercent() { return GetICPrecision()*100.0; }

	float GetICLowerf() { return (float)GetICLower(); }
	float GetICUpperf() { return (float)GetICUpper(); }
	float GetICPrecisionf() { return (float)GetICPrecision(); }
	float GetICPrecisionPercentf() { return (float)GetICPrecisionPercent(); }

	float GetAveragef() { return (float)GetAverage(); }
	float GetVariancef() { return (float)GetVariance(); }

	int32 GetNum() { return ValueSumTotal; }
	float GetMinf() { return (float)ValueMin; }
	float GetMaxf() { return (float)ValueMax; }

	void AddValue(double NewValue) 
	{
		ValueSum += NewValue;
		ValueSumSquare += NewValue * NewValue;
		++ValueSumTotal;
		if (NewValue < ValueMin)ValueMin = NewValue;
		else if (NewValue > ValueMax)ValueMax = NewValue;
	}

	void Reset() 
	{
		ValueSum = 0.0;
		ValueSumSquare = 0.0;
		ValueSumTotal = 0;
		ValueMin = 10000.0;
		ValueMax = -10000.0;
	}

	FSampleSum() 
	{
		Reset();
	}

	FSampleSum(const FSampleSum &Sample) 
	{
		ValueSum = Sample.ValueSum;
		ValueSumTotal = Sample.ValueSumTotal;
		ValueSumSquare = Sample.ValueSumSquare;
		ValueMin = Sample.ValueMin;
		ValueMax = Sample.ValueMax;
	}
};

USTRUCT(BlueprintType)
struct FSampleTimeSum 
{
	GENERATED_BODY()

	double ValueSum;
	double ValueSumSquare;
	double ValueMin;
	double ValueMax;
	double TimeBegin;
	double TimeEnd;	
	int32 NumDt;

	double GetAverage() 
	{
		double TimeDiff = TimeEnd - TimeBegin;
		if(TimeDiff > 0.0)
			return ValueSum / TimeDiff;
		return 0.0;
	}

	double GetVariance() 
	{	//Formula -> Sum(X^2)/n-1 - Sum(X)*Sum(X)/n*n-1
		double TimeDiff = TimeEnd - TimeBegin;
		double Moment1Square = GetAverage();
		Moment1Square *= Moment1Square;
		//Moment1Square *= TimeDiff;
		double Moment2 = ValueSumSquare / TimeDiff;
		if(TimeDiff != 0.0)
			return Moment2 - Moment1Square;
		return 0.0;
	}

	double GetTimeBegin() { return TimeBegin; }
	double GetTimeEnd() { return TimeEnd; }
	float GetAveragef() { return (float)GetAverage(); }
	float GetVariancef() { return (float)GetVariance(); }
	float GetMinf() { return (float)ValueMin; }
	float GetMaxf() { return (float)ValueMax; }
	float GetTimeBeginf() { return (float)GetTimeBegin(); }
	float GetTimeEndf() { return (float)GetTimeEnd(); }

	void AddValue(double Value, double Delta) 
	{
		ValueSum += Value * Delta;
		ValueSumSquare += Value * Value * Delta;
		TimeEnd += Delta;
		if (Value < ValueMin)ValueMin = Value;
		if (Value > ValueMax)ValueMax = Value;
		++NumDt;
	}

	void SetTimeBegin(double Value) { TimeBegin = Value; TimeEnd = TimeBegin; }
	void SetTimeEnd(double Value) { TimeEnd = Value; }

	void Reset() 
	{
		ValueSum = 0.0;
		ValueSumSquare = 0.0;
		TimeBegin = 0.0;
		TimeEnd = 0.0;
		ValueMin = 1000.0f;
		ValueMax = -1000.0f;
		NumDt = 0;
	}

	FSampleTimeSum() 
	{
		Reset();
	}

	FSampleTimeSum(const FSampleTimeSum &NewPMF) 
	{
		ValueSum = NewPMF.ValueSum;
		ValueSumSquare = NewPMF.ValueSumSquare;
		TimeBegin = NewPMF.TimeBegin;
		TimeEnd = NewPMF.TimeEnd;
		ValueMin = NewPMF.ValueMin;
		ValueMax = NewPMF.ValueMax;
		NumDt = NewPMF.NumDt;
	}
};

USTRUCT(BlueprintType)
struct FFileSetting
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SimDump;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RoundDump;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString IterationDump;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TransientDump;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSaveSimDump;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSaveRoundDump;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSaveIterationDump;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSaveTransientDump;

	FString DirStr;

	void CopySetting(const FFileSetting &NewFile)
	{
		bSaveSimDump = NewFile.bSaveSimDump;
		bSaveIterationDump = NewFile.bSaveIterationDump;
		bSaveRoundDump = NewFile.bSaveRoundDump;
		bSaveTransientDump = NewFile.bSaveTransientDump;
		SimDump = NewFile.SimDump;
		RoundDump = NewFile.RoundDump;
		IterationDump = NewFile.IterationDump;
		TransientDump = NewFile.TransientDump;
		DirStr = NewFile.DirStr;
	}

	FFileSetting()
	{
		bSaveSimDump = true;
		bSaveRoundDump = true;
		SimDump = TEXT("SimDump");
		RoundDump = TEXT("RoundDump");
		IterationDump = TEXT("IterationDump");
		TransientDump = TEXT("TransientDump");
		DirStr = TEXT("SomeDir");
	}
};

USTRUCT(BlueprintType)
struct FSimConfig
{
	GENERATED_BODY()

	//Parametro de chegada lambda1, prioridade
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSimParameter ArrivalParam1;

	//Parametro de chegada lambda2
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSimParameter ArrivalParam2;

	//Parametro de serviço mu1, prioridade
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSimParameter ServiceParam1;

	//Parametro de serviço mu2
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSimParameter ServiceParam2;

	//Gerador pseudo-aleatorio
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRandomStream RandStream;

	//Iterações da Rodada Transiente, kmin
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TransientIterations;

	//Iterações da Rodada, kmin
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Iterations;

	//Número de rodadas para processar
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Rounds;

	//Se é FCFS ou LCFS pra fila 1
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLCFS1;

	//Se é FCFS ou LCFS pra fila 2
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLCFS2;

	//Se deve ser fila única
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bQueue1Only;

	double GetServiceRand1() { return (double)ServiceParam1.GetRandVal(RandStream); }
	double GetServiceRand2() { return (double)ServiceParam2.GetRandVal(RandStream); }
	double GetArrivalRand1() { return (double)ArrivalParam1.GetRandVal(RandStream); }
	double GetArrivalRand2() { return (double)ArrivalParam2.GetRandVal(RandStream); }

	void CopyConfig(const FSimConfig &Cfg)
	{
		ArrivalParam1 = Cfg.ArrivalParam1;
		ArrivalParam2 = Cfg.ArrivalParam2;
		ServiceParam1 = Cfg.ServiceParam1;
		ServiceParam2 = Cfg.ServiceParam2;
		TransientIterations = Cfg.TransientIterations;
		Iterations = Cfg.Iterations;
		Rounds = Cfg.Rounds;
		bLCFS1 = Cfg.bLCFS1;
		bLCFS2 = Cfg.bLCFS2;
		bQueue1Only = Cfg.bQueue1Only;
		RandStream.Initialize(Cfg.RandStream.GetCurrentSeed());
	}

	FSimConfig(const FSimConfig &Cfg) {
		CopyConfig(Cfg);
	}

	FSimConfig() {
		Iterations = 2500;
		Rounds = 3200;
		ServiceParam1.Value = 1.0f;
		//ServiceParam2.Value = 1.0f;
		bLCFS1 = false;
		bLCFS2 = false;
		bQueue1Only = true;
		RandStream.Initialize(14000605);
	}
};

USTRUCT(BlueprintType)
struct FSimMetricData
{
	GENERATED_BODY()
	//Metricas para N simulaçao
	
	//Metricas de Espera, W1,W2
	TArray< TArray< FSampleSum > > RoundW1;	//Amostra de Cada rodada, [SimIndex]
	TArray< FSampleSum > SimW1;				//Amostra das Medias por rodada, [SimIndex]
	TArray< FSampleSum > TransW1;		//Transient Cumulativo, [SimIndex]
	TArray< FKantanSeriesData > TransW1ChartData;	//[SimIndex]
	TArray< FKantanSeriesData > RoundW1ChartData;	//[SimIndex]
	TArray< FKantanSeriesData > SimW1ChartData;	//[SimIndex] é um ponto daqui,0=Avg,1=LowerIC,2=UpperIC

	TArray < FVector2D > SimW1_Min;
	TArray < FVector2D > SimW1_Max;
	TArray < FVector2D > RoundW1_Min;
	TArray < FVector2D > RoundW1_Max;
	TArray < FVector2D > TransW1_Min;	//Min de cada SimIndex
	TArray < FVector2D > TransW1_Max;	//Max de cada SimIndex

	//Metricas da Fila de Espera, Nq1,Nq2
	TArray< TArray< FSampleTimeSum > > RoundNq1;	//Amostra de Cada rodada, [SimIndex]
	TArray< FSampleSum > SimNq1;				//Amostra das Medias por rodada, [SimIndex]
	TArray< FSampleTimeSum > TransNq1;		//Transient Cumulativo, [SimIndex]
	TArray< FKantanSeriesData > TransNq1ChartData;	//[SimIndex]
	TArray< FKantanSeriesData > RoundNq1ChartData;	//[SimIndex]
	TArray< FKantanSeriesData > SimNq1ChartData;	//[SimIndex] é um ponto daqui,0=Avg,1=LowerIC,2=UpperIC

	TArray < FVector2D > SimNq1_Min;
	TArray < FVector2D > SimNq1_Max;
	TArray < FVector2D > RoundNq1_Min;
	TArray < FVector2D > RoundNq1_Max;
	TArray < FVector2D > TransNq1_Min;	//Min de cada SimIndex
	TArray < FVector2D > TransNq1_Max;	//Max de cada SimIndex
	
	FVector2D GetSimW1Min();
	FVector2D GetSimNq1Min();
	FVector2D GetSimW1Max();
	FVector2D GetSimNq1Max();
	FVector2D GetRoundW1Min();
	FVector2D GetRoundNq1Min();
	FVector2D GetRoundW1Max();
	FVector2D GetRoundNq1Max();
	FVector2D GetTransW1Min();
	FVector2D GetTransNq1Min();
	FVector2D GetTransW1Max();
	FVector2D GetTransNq1Max();

	TArray< FKantanSeriesData >& GetTransW1Chart() { return TransNq1ChartData; }
	TArray< FKantanSeriesData >& GetRoundW1Chart() { return RoundNq1ChartData; }
	TArray< FKantanSeriesData >& GetSimW1Chart() { return SimNq1ChartData; }
	TArray< FKantanSeriesData >& GetTransNq1Chart() { return TransNq1ChartData; }
	TArray< FKantanSeriesData >& GetRoundNq1Chart() { return RoundNq1ChartData; }
	TArray< FKantanSeriesData >& GetSimNq1Chart() { return SimNq1ChartData; }

	FString GetSampleNameString(FString NameStr, int32 SampleIndex);
	FString GetSampleNameICString(FString NameStr);
	FString GetSampleNameICUpperString(FString NameStr);
	FString GetSampleNameICLowerString(FString NameStr);
	FString GetSampleNameAnalyticString(FString NameStr);

	void SetupDataW1(FString NameStr, int32 MaxSim);
	void SetupDataNq1(FString NameStr, int32 MaxSim);
	void AddRoundW1(int32 SimIndex);
	void AddRoundNq1(int32 SimIndex);
	void AddValueW1(double Value, int32 SimIndex);
	void AddValueNq1(double Value, double dt, int32 SimIndex);
	void AddValueW1Transient(double Value, int32 SimIndex);
	void AddValueNq1Transient(double Value, double dt, int32 SimIndex);
	void LastRoundW1(int32 SimIndex);
	void LastRoundNq1(int32 SimIndex);

	FSimMetricData(){}
};

UCLASS()
class ADSIMULATOR_API UADSim2BlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintPure, Category = "ADSimFunction")
	static bool IsInEditor();

	UFUNCTION(BlueprintPure, Category = "ADSimFunction")
	static FString GetCPUText();

	UFUNCTION(BlueprintPure, Category = "ADSimFunction")
	static int32 GetCPUCores();

	UFUNCTION(BlueprintPure, Category = "ADSimFunction")
	static int32 GetCPUThreads();

	UFUNCTION(BlueprintCallable, Category = "ADSimFunction")
	static void OpenDumpDir();
};
