// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ADSim2BlueprintFunctionLibrary.h"
#include "ADSim2MetricSingleSource.h"

#include "KantanCartesianDatasourceInterface.h"
#include "CartesianData.h"

#include "ADSim2ChartWidget.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(ChartWidgetLog2, Log, All);

/**
 * 
 */
UCLASS()
class ADSIMULATOR_API UADSim2ChartWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UADSim2ChartWidget(const FObjectInitializer &ObjectInitializer);

	void NativeConstruct() override;
	//void NativeDestruct() override;

	void SetupDataNameW1(FString NameStrW, FString NameStrNq, int32 MaxSim);
	void SetupDataNameW2(FString NameStrW, FString NameStrNq, int32 MaxSim);
	void SetupDataNameW3(FString NameStrW, FString NameStrNq, int32 MaxSim);
	void UpdateDataSource1();
	void UpdateDataSource2();
	void UpdateDataSource3();
	
	UFUNCTION(BlueprintCallable)
	void ConfigureDataSource1();

	UFUNCTION(BlueprintCallable)
	void ConfigureDataSource2();

	UFUNCTION(BlueprintCallable)
	void ConfigureDataSource3();
	
	UFUNCTION(BlueprintCallable)
	void AddEnablerButtons();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ResetDataSourceBP();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateDataSource1BP();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateDataSourceRange1BP();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ConfigureSeries1BP(FName NameID, bool bDrawPoints, bool bDrawLines);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void EnableSeries1BP(FName NameID, bool bEnable);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateDataSource2BP();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateDataSourceRange2BP();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ConfigureSeries2BP(FName NameID, bool bDrawPoints, bool bDrawLines);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void EnableSeries2BP(FName NameID, bool bEnable);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateDataSource3BP();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateDataSourceRange3BP();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ConfigureSeries3BP(FName NameID, bool bDrawPoints, bool bDrawLines);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void EnableSeries3BP(FName NameID, bool bEnable);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void AddEnablerButton1BP(FName NameID);

	void SetDataRef1(bool bNq, EChartType Type, TArray< FKantanSeriesData > *DataRef);
	void SetDataRef2(bool bNq, EChartType Type, TArray< FKantanSeriesData > *DataRef);
	void SetDataRef3(bool bNq, EChartType Type, TArray< FKantanSeriesData > *DataRef);

	//-------------------Update Ranges
	/* X(min,max) Y(min,max)*/
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRangeW1Trans(FVector2D X, FVector2D Y);

	/* X(min,max) Y(min,max)*/
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRangeW1Round(FVector2D X, FVector2D Y);

	/* X(min,max) Y(min,max)*/
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRangeW1IC(FVector2D X, FVector2D Y);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRangeW2Trans(FVector2D X, FVector2D Y);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRangeW2Round(FVector2D X, FVector2D Y);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRangeW2IC(FVector2D X, FVector2D Y);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRangeW3Trans(FVector2D X, FVector2D Y);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRangeW3Round(FVector2D X, FVector2D Y);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRangeW3IC(FVector2D X, FVector2D Y);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRangeNq1Trans(FVector2D X, FVector2D Y);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRangeNq1Round(FVector2D X, FVector2D Y);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRangeNq1IC(FVector2D X, FVector2D Y);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRangeNq2Trans(FVector2D X, FVector2D Y);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRangeNq2Round(FVector2D X, FVector2D Y);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRangeNq2IC(FVector2D X, FVector2D Y);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRangeNq3Trans(FVector2D X, FVector2D Y);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRangeNq3Round(FVector2D X, FVector2D Y);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRangeNq3IC(FVector2D X, FVector2D Y);

	//-------------------Update Ranges

	//-------------------Update Refs

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRefW1Trans(UObject* Ref);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRefW1Round(UObject* Ref);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRefW1IC(UObject* Ref);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRefW2Trans(UObject* Ref);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRefW2Round(UObject* Ref);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRefW2IC(UObject* Ref);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRefW3Trans(UObject* Ref);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRefW3Round(UObject* Ref);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRefW3IC(UObject* Ref);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRefNq1Trans(UObject* Ref);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRefNq1Round(UObject* Ref);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRefNq1IC(UObject* Ref);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRefNq2Trans(UObject* Ref);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRefNq2Round(UObject* Ref);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRefNq2IC(UObject* Ref);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRefNq3Trans(UObject* Ref);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRefNq3Round(UObject* Ref);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateRefNq3IC(UObject* Ref);
	//-------------------Update Refs
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDarkChart = true;

	UADSim2MetricSource* MetricsW1_Trans;
	UADSim2MetricSource* MetricsW1_Round;
	UADSim2MetricSource* MetricsW1_IC;
	UADSim2MetricSource* MetricsW2_Trans;
	UADSim2MetricSource* MetricsW2_Round;
	UADSim2MetricSource* MetricsW2_IC;
	UADSim2MetricSource* MetricsW3_Trans;
	UADSim2MetricSource* MetricsW3_Round;
	UADSim2MetricSource* MetricsW3_IC;
	
	UADSim2MetricSource* MetricsNq1_Trans;
	UADSim2MetricSource* MetricsNq1_Round;
	UADSim2MetricSource* MetricsNq1_IC;
	UADSim2MetricSource* MetricsNq2_Trans;
	UADSim2MetricSource* MetricsNq2_Round;
	UADSim2MetricSource* MetricsNq2_IC;
	UADSim2MetricSource* MetricsNq3_Trans;
	UADSim2MetricSource* MetricsNq3_Round;
	UADSim2MetricSource* MetricsNq3_IC;
};
