// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ADSim2BlueprintFunctionLibrary.h"
#include "ADSim2ChartWidget.h"

#include "KantanCartesianDatasourceInterface.h"
#include "CartesianData.h"

#include "ADSim2MainWidget.generated.h"

/**
 * 
 */
UCLASS()
class ADSIMULATOR_API UADSim2MainWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UADSim2MainWidget(const FObjectInitializer &ObjectInitializer);

	void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void SetWidgetRef(UADSim2ChartWidget* NewWidgetRef);

	FSimMetricData SimData1;
	FSimMetricData SimData2;
	FSimMetricData SimData3;

	UADSim2ChartWidget* ChartWidgetObj;
	
	void SetChartRange();

	void SetupDataNameW1(FString NameStrW, FString NameStrNq, int32 MaxSim);
	void SetupDataNameW2(FString NameStrW, FString NameStrNq, int32 MaxSim);
	void SetupDataNameW3(FString NameStrW, FString NameStrNq, int32 MaxSim);
	
	FVector2D NewRangeX(FVector2D Min, FVector2D Max);
	FVector2D NewRangeY(FVector2D Min, FVector2D Max);

	TArray< FKantanSeriesData >* GetChartSeriesW1Trans();
	TArray< FKantanSeriesData >* GetChartSeriesW1Round();
	TArray< FKantanSeriesData >* GetChartSeriesW1IC();
	TArray< FKantanSeriesData >* GetChartSeriesW2Trans();
	TArray< FKantanSeriesData >* GetChartSeriesW2Round();
	TArray< FKantanSeriesData >* GetChartSeriesW2IC();
	TArray< FKantanSeriesData >* GetChartSeriesW3Trans();
	TArray< FKantanSeriesData >* GetChartSeriesW3Round();
	TArray< FKantanSeriesData >* GetChartSeriesW3IC();

	TArray< FKantanSeriesData >* GetChartSeriesNq1Trans();
	TArray< FKantanSeriesData >* GetChartSeriesNq1Round();
	TArray< FKantanSeriesData >* GetChartSeriesNq1IC();
	TArray< FKantanSeriesData >* GetChartSeriesNq2Trans();
	TArray< FKantanSeriesData >* GetChartSeriesNq2Round();
	TArray< FKantanSeriesData >* GetChartSeriesNq2IC();
	TArray< FKantanSeriesData >* GetChartSeriesNq3Trans();
	TArray< FKantanSeriesData >* GetChartSeriesNq3Round();
	TArray< FKantanSeriesData >* GetChartSeriesNq3IC();
};
