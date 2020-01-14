// Fill out your copyright notice in the Description page of Project Settings.


#include "ADSim2MainWidget.h"
#include "ADSim2Pawn.h"

#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

#include "EngineGlobals.h"

UADSim2MainWidget::UADSim2MainWidget(const FObjectInitializer &ObjectInitializer) :Super(ObjectInitializer)
{
	ChartWidgetObj = nullptr;
}

void UADSim2MainWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UADSim2MainWidget::SetWidgetRef(UADSim2ChartWidget* NewWidgetRef)
{
	ChartWidgetObj = NewWidgetRef;

	AADSim2Pawn* Pawn = Cast<AADSim2Pawn>(UGameplayStatics::GetPlayerPawn(GetWorld(),0));

	bool bSetCharts = (Pawn != nullptr && Pawn->bPassMetricChart);

	if (ChartWidgetObj != nullptr && bSetCharts)
	{
		bool bNq = true;
		bool bW = false;
		ChartWidgetObj->SetDataRef1(bNq, EChartType::Transient, GetChartSeriesNq1Trans());
		ChartWidgetObj->SetDataRef1(bNq, EChartType::Round, GetChartSeriesNq1Round());
		ChartWidgetObj->SetDataRef1(bNq, EChartType::Simulation, GetChartSeriesNq1IC());
		ChartWidgetObj->SetDataRef1(bW, EChartType::Transient, GetChartSeriesW1Trans());
		ChartWidgetObj->SetDataRef1(bW, EChartType::Round, GetChartSeriesW1Round());
		ChartWidgetObj->SetDataRef1(bW, EChartType::Simulation, GetChartSeriesW1IC());

		ChartWidgetObj->SetDataRef2(bNq, EChartType::Transient, GetChartSeriesNq2Trans());
		ChartWidgetObj->SetDataRef2(bNq, EChartType::Round, GetChartSeriesNq2Round());
		ChartWidgetObj->SetDataRef2(bNq, EChartType::Simulation, GetChartSeriesNq2IC());
		ChartWidgetObj->SetDataRef2(bW, EChartType::Transient, GetChartSeriesW2Trans());
		ChartWidgetObj->SetDataRef2(bW, EChartType::Round, GetChartSeriesW2Round());
		ChartWidgetObj->SetDataRef2(bW, EChartType::Simulation, GetChartSeriesW2IC());

		ChartWidgetObj->SetDataRef3(bNq, EChartType::Transient, GetChartSeriesNq3Trans());
		ChartWidgetObj->SetDataRef3(bNq, EChartType::Round, GetChartSeriesNq3Round());
		ChartWidgetObj->SetDataRef3(bNq, EChartType::Simulation, GetChartSeriesNq3IC());
		ChartWidgetObj->SetDataRef3(bW, EChartType::Transient, GetChartSeriesW3Trans());
		ChartWidgetObj->SetDataRef3(bW, EChartType::Round, GetChartSeriesW3Round());
		ChartWidgetObj->SetDataRef3(bW, EChartType::Simulation, GetChartSeriesW3IC());
		
		ChartWidgetObj->UpdateRefW1Trans(ChartWidgetObj->MetricsW1_Trans);
		ChartWidgetObj->UpdateRefW1Round(ChartWidgetObj->MetricsW1_Round);
		ChartWidgetObj->UpdateRefW1IC(ChartWidgetObj->MetricsW1_IC);
		ChartWidgetObj->UpdateRefNq1Trans(ChartWidgetObj->MetricsNq1_Trans);
		ChartWidgetObj->UpdateRefNq1Round(ChartWidgetObj->MetricsNq1_Round);
		ChartWidgetObj->UpdateRefNq1IC(ChartWidgetObj->MetricsNq1_IC);

		ChartWidgetObj->UpdateRefW2Trans(ChartWidgetObj->MetricsW2_Trans);
		ChartWidgetObj->UpdateRefW2Round(ChartWidgetObj->MetricsW2_Round);
		ChartWidgetObj->UpdateRefW2IC(ChartWidgetObj->MetricsW2_IC);
		ChartWidgetObj->UpdateRefNq2Trans(ChartWidgetObj->MetricsNq2_Trans);
		ChartWidgetObj->UpdateRefNq2Round(ChartWidgetObj->MetricsNq2_Round);
		ChartWidgetObj->UpdateRefNq2IC(ChartWidgetObj->MetricsNq2_IC);

		ChartWidgetObj->UpdateRefW3Trans(ChartWidgetObj->MetricsW3_Trans);
		ChartWidgetObj->UpdateRefW3Round(ChartWidgetObj->MetricsW3_Round);
		ChartWidgetObj->UpdateRefW3IC(ChartWidgetObj->MetricsW3_IC);
		ChartWidgetObj->UpdateRefNq3Trans(ChartWidgetObj->MetricsNq3_Trans);
		ChartWidgetObj->UpdateRefNq3Round(ChartWidgetObj->MetricsNq3_Round);
		ChartWidgetObj->UpdateRefNq3IC(ChartWidgetObj->MetricsNq3_IC);
	}
}

void UADSim2MainWidget::SetChartRange()
{
	if (ChartWidgetObj != nullptr)
	{
		ChartWidgetObj->UpdateRangeW1Trans(NewRangeX(SimData1.GetTransW1Min(), SimData1.GetTransW1Max()),NewRangeY(SimData1.GetTransW1Min(), SimData1.GetTransW1Max()));
		ChartWidgetObj->UpdateRangeW1Round(NewRangeX(SimData1.GetRoundW1Min(), SimData1.GetRoundW1Max()), NewRangeY(SimData1.GetRoundW1Min(), SimData1.GetRoundW1Max()));
		ChartWidgetObj->UpdateRangeW1IC(NewRangeX(SimData1.GetSimW1Min(), SimData1.GetSimW1Max()), NewRangeY(SimData1.GetSimW1Min(), SimData1.GetSimW1Max()));
		ChartWidgetObj->UpdateRangeNq1Trans(NewRangeX(SimData1.GetTransNq1Min(), SimData1.GetTransNq1Max()), NewRangeY(SimData1.GetTransNq1Min(), SimData1.GetTransNq1Max()));
		ChartWidgetObj->UpdateRangeNq1Round(NewRangeX(SimData1.GetRoundNq1Min(), SimData1.GetRoundNq1Max()), NewRangeY(SimData1.GetRoundNq1Min(), SimData1.GetRoundNq1Max()));
		ChartWidgetObj->UpdateRangeNq1IC(NewRangeX(SimData1.GetSimNq1Min(), SimData1.GetSimNq1Max()), NewRangeY(SimData1.GetSimNq1Min(), SimData1.GetSimNq1Max()));
		
		ChartWidgetObj->UpdateRangeW2Trans(NewRangeX(SimData2.GetTransW1Min(), SimData2.GetTransW1Max()), NewRangeY(SimData2.GetTransW1Min(), SimData2.GetTransW1Max()));
		ChartWidgetObj->UpdateRangeW2Round(NewRangeX(SimData2.GetRoundW1Min(), SimData2.GetRoundW1Max()), NewRangeY(SimData2.GetRoundW1Min(), SimData2.GetRoundW1Max()));
		ChartWidgetObj->UpdateRangeW2IC(NewRangeX(SimData2.GetSimW1Min(), SimData2.GetSimW1Max()), NewRangeY(SimData2.GetSimW1Min(), SimData2.GetSimW1Max()));
		ChartWidgetObj->UpdateRangeNq2Trans(NewRangeX(SimData2.GetTransNq1Min(), SimData2.GetTransNq1Max()), NewRangeY(SimData2.GetTransNq1Min(), SimData2.GetTransNq1Max()));
		ChartWidgetObj->UpdateRangeNq2Round(NewRangeX(SimData2.GetRoundNq1Min(), SimData2.GetRoundNq1Max()), NewRangeY(SimData2.GetRoundNq1Min(), SimData2.GetRoundNq1Max()));
		ChartWidgetObj->UpdateRangeNq2IC(NewRangeX(SimData2.GetSimNq1Min(), SimData2.GetSimNq1Max()), NewRangeY(SimData2.GetSimNq1Min(), SimData2.GetSimNq1Max()));
		
		ChartWidgetObj->UpdateRangeW3Trans(NewRangeX(SimData3.GetTransW1Min(), SimData3.GetTransW1Max()), NewRangeY(SimData3.GetTransW1Min(), SimData3.GetTransW1Max()));
		ChartWidgetObj->UpdateRangeW3Round(NewRangeX(SimData3.GetRoundW1Min(), SimData3.GetRoundW1Max()), NewRangeY(SimData3.GetRoundW1Min(), SimData3.GetRoundW1Max()));
		ChartWidgetObj->UpdateRangeW3IC(NewRangeX(SimData3.GetSimW1Min(), SimData3.GetSimW1Max()), NewRangeY(SimData3.GetSimW1Min(), SimData3.GetSimW1Max()));
		ChartWidgetObj->UpdateRangeNq3Trans(NewRangeX(SimData3.GetTransNq1Min(), SimData3.GetTransNq1Max()), NewRangeY(SimData3.GetTransNq1Min(), SimData3.GetTransNq1Max()));
		ChartWidgetObj->UpdateRangeNq3Round(NewRangeX(SimData3.GetRoundNq1Min(), SimData3.GetRoundNq1Max()), NewRangeY(SimData3.GetRoundNq1Min(), SimData3.GetRoundNq1Max()));
		ChartWidgetObj->UpdateRangeNq3IC(NewRangeX(SimData3.GetSimNq1Min(), SimData3.GetSimNq1Max()), NewRangeY(SimData3.GetSimNq1Min(), SimData3.GetSimNq1Max()));
		
	}
}

void UADSim2MainWidget::SetupDataNameW1(FString NameStrW, FString NameStrNq, int32 MaxSim)
{
	SimData1.SetupDataW1(NameStrW, MaxSim);
	SimData1.SetupDataNq1(NameStrNq, MaxSim);
}

void UADSim2MainWidget::SetupDataNameW2(FString NameStrW, FString NameStrNq, int32 MaxSim)
{
	SimData2.SetupDataW1(NameStrW, MaxSim);
	SimData2.SetupDataNq1(NameStrNq, MaxSim);
}

void UADSim2MainWidget::SetupDataNameW3(FString NameStrW, FString NameStrNq, int32 MaxSim)
{
	SimData3.SetupDataW1(NameStrW, MaxSim);
	SimData3.SetupDataNq1(NameStrNq, MaxSim);
}

TArray< FKantanSeriesData >* UADSim2MainWidget::GetChartSeriesW1Trans()
{
	return &SimData1.TransW1ChartData;
}

TArray< FKantanSeriesData >* UADSim2MainWidget::GetChartSeriesW1Round()
{
	return &SimData1.RoundW1ChartData;
}

TArray< FKantanSeriesData >* UADSim2MainWidget::GetChartSeriesW1IC()
{
	return &SimData1.SimW1ChartData;
}

TArray< FKantanSeriesData >* UADSim2MainWidget::GetChartSeriesNq1Trans()
{
	return &SimData1.TransNq1ChartData;
}

TArray< FKantanSeriesData >* UADSim2MainWidget::GetChartSeriesNq1Round()
{
	return &SimData1.RoundNq1ChartData;
}

TArray< FKantanSeriesData >* UADSim2MainWidget::GetChartSeriesNq1IC()
{
	return &SimData1.SimNq1ChartData;
}

TArray< FKantanSeriesData >* UADSim2MainWidget::GetChartSeriesW2Trans()
{
	return &SimData2.TransW1ChartData;
}

TArray< FKantanSeriesData >* UADSim2MainWidget::GetChartSeriesW2Round()
{
	return &SimData2.RoundW1ChartData;
}

TArray< FKantanSeriesData >* UADSim2MainWidget::GetChartSeriesW2IC()
{
	return &SimData2.SimW1ChartData;
}

TArray< FKantanSeriesData >* UADSim2MainWidget::GetChartSeriesNq2Trans()
{
	return &SimData2.TransNq1ChartData;
}

TArray< FKantanSeriesData >* UADSim2MainWidget::GetChartSeriesNq2Round()
{
	return &SimData2.RoundNq1ChartData;
}

TArray< FKantanSeriesData >* UADSim2MainWidget::GetChartSeriesNq2IC()
{
	return &SimData2.SimNq1ChartData;
}

TArray< FKantanSeriesData >* UADSim2MainWidget::GetChartSeriesW3Trans()
{
	return &SimData3.TransW1ChartData;
}

TArray< FKantanSeriesData >* UADSim2MainWidget::GetChartSeriesW3Round()
{
	return &SimData3.RoundW1ChartData;
}

TArray< FKantanSeriesData >* UADSim2MainWidget::GetChartSeriesW3IC()
{
	return &SimData3.SimW1ChartData;
}

TArray< FKantanSeriesData >* UADSim2MainWidget::GetChartSeriesNq3Trans()
{
	return &SimData3.TransNq1ChartData;
}

TArray< FKantanSeriesData >* UADSim2MainWidget::GetChartSeriesNq3Round()
{
	return &SimData3.RoundNq1ChartData;
}

TArray< FKantanSeriesData >* UADSim2MainWidget::GetChartSeriesNq3IC()
{
	return &SimData3.SimNq1ChartData;
}

FVector2D UADSim2MainWidget::NewRangeX(FVector2D Min, FVector2D Max)
{
	FVector2D NewX(Min.X, Max.X);
	if (NewX.X == NewX.Y)
	{
		NewX.X -= Min.X*0.1f;
		NewX.Y += Min.X*0.1f;
	}
	return NewX;
}

FVector2D UADSim2MainWidget::NewRangeY(FVector2D Min, FVector2D Max)
{
	FVector2D NewY(Min.Y, Max.Y);
	if (NewY.X == NewY.Y)
	{
		NewY.X -= Min.Y*0.1f;
		NewY.Y += Min.Y*0.1f;
	}
	return NewY;
}
