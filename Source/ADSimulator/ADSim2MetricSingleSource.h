// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "KantanCartesianDatasourceInterface.h"
#include "CartesianData.h"
#include "ADSim2BlueprintFunctionLibrary.h"

#include "ADSim2MetricSingleSource.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(SingleSourceLog2, Log, All);

UENUM(BlueprintType)
enum class EChartType : uint8
{
	Transient	UMETA(DisplayName = "Transient"),
	Round		UMETA(DisplayName = "Round"),
	Simulation	UMETA(DisplayName = "Simulation")
};

/**
 * 
 */
UCLASS()
class ADSIMULATOR_API UADSim2MetricSource : public UObject, public IKantanCartesianDatasourceInterface
{
	GENERATED_BODY()
	
public:

	UADSim2MetricSource();

	FSimMetricData Data;
	EChartType CurrentDataType = EChartType::Transient;
	bool bShowNq1 = false;	//Show W1 otherwise

	TArray< FKantanSeriesData > DumbData;
	TArray< FKantanSeriesData > *RefData;

	void SetRefData(TArray< FKantanSeriesData > *NewRef) { RefData = NewRef; }

	void NextDataType();
	void PreviousDataType();
	void SwitchType();
	void SetDataType(EChartType NewType) { CurrentDataType = NewType; }
	void SetDataName(FString NameStrW, FString NameStrNq, int32 MaxSim);
	FVector2D GetDataMin();
	FVector2D GetDataMax();

	bool Has2Series();

	//Kantan Charts Interface
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Cartesian Datasource")
	int32 GetNumSeries() const;
	virtual int32 GetNumSeries_Implementation() const override;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Cartesian Datasource")
	FName GetSeriesId(int32 CatIdx) const;
	virtual FName GetSeriesId_Implementation(int32 CatIdx) const override;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Cartesian Datasource")
	FText GetSeriesName(int32 SeriesIdx) const;
	virtual FText GetSeriesName_Implementation(int32 SeriesIdx) const override;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Cartesian Datasource")
	TArray< FKantanCartesianDatapoint > GetSeriesDatapoints(int32 SeriesIdx) const;
	virtual TArray< FKantanCartesianDatapoint > GetSeriesDatapoints_Implementation(int32 SeriesIdx) const override;
};
