// Fill out your copyright notice in the Description page of Project Settings.


#include "ADSim2MetricSingleSource.h"


DEFINE_LOG_CATEGORY(SingleSourceLog2);

UADSim2MetricSource::UADSim2MetricSource()
{
	RefData = nullptr;
	DumbData.Add(FKantanSeriesData());
	DumbData.Last().Id = FName(TEXT("DumbID"));
	DumbData.Last().Name = FText::FromString(TEXT("DumbName"));
	DumbData.Last().Points.Add(FKantanCartesianDatapoint());
	DumbData.Last().Points.Last().Coords.Set(0.5f, 0.25f);
	DumbData.Last().Points.Add(FKantanCartesianDatapoint());
	DumbData.Last().Points.Last().Coords.Set(0.5f, 0.75f);
	DumbData.Last().Points.Add(FKantanCartesianDatapoint());
	DumbData.Last().Points.Last().Coords.Set(0.25f, 0.5f);
	DumbData.Last().Points.Add(FKantanCartesianDatapoint());
	DumbData.Last().Points.Last().Coords.Set(0.75f, 0.5f);
	
	DumbData.Add(FKantanSeriesData());
	DumbData.Last().Id = FName(TEXT("OpsID"));
	DumbData.Last().Name = FText::FromString(TEXT("OpsName"));
	DumbData.Last().Points.Add(FKantanCartesianDatapoint());
	DumbData.Last().Points.Last().Coords.Set(0.25f, 0.25f);
	DumbData.Last().Points.Add(FKantanCartesianDatapoint());
	DumbData.Last().Points.Last().Coords.Set(0.75f, 0.75f);
	DumbData.Last().Points.Add(FKantanCartesianDatapoint());
	DumbData.Last().Points.Last().Coords.Set(0.25f, 0.75f);
	DumbData.Last().Points.Add(FKantanCartesianDatapoint());
	DumbData.Last().Points.Last().Coords.Set(0.75f, 0.25f);
}

int32 UADSim2MetricSource::GetNumSeries_Implementation() const
{
	if (RefData != nullptr)
		return RefData->Num();
	return DumbData.Num();
}

FName UADSim2MetricSource::GetSeriesId_Implementation(int32 CatIdx) const
{
	if (RefData != nullptr)
		return (*RefData)[CatIdx].Id;
	return DumbData[CatIdx].Id;
}

FText UADSim2MetricSource::GetSeriesName_Implementation(int32 SeriesIdx) const
{
	if (RefData != nullptr)
		return (*RefData)[SeriesIdx].Name;
	return DumbData[SeriesIdx].Name;
}

TArray< FKantanCartesianDatapoint > UADSim2MetricSource::GetSeriesDatapoints_Implementation(int32 SeriesIdx) const
{
	if (RefData != nullptr)
		return (*RefData)[SeriesIdx].Points;
	return DumbData[SeriesIdx].Points;
}

void UADSim2MetricSource::SetDataName(FString NameStrW, FString NameStrNq,int32 MaxSim)
{
	Data.SetupDataW1(NameStrW, MaxSim);
	Data.SetupDataNq1(NameStrNq,MaxSim);
}

void UADSim2MetricSource::NextDataType()
{
	switch (CurrentDataType)
	{
	case EChartType::Transient:
		CurrentDataType = EChartType::Round;
		break;
	case EChartType::Round:
		CurrentDataType = EChartType::Simulation;
		break;
	case EChartType::Simulation:
		CurrentDataType = EChartType::Transient;
		break;
	}
}

void UADSim2MetricSource::PreviousDataType()
{
	switch (CurrentDataType)
	{
	case EChartType::Transient:
		CurrentDataType = EChartType::Simulation;
		break;
	case EChartType::Round:
		CurrentDataType = EChartType::Transient;
		break;
	case EChartType::Simulation:
		CurrentDataType = EChartType::Round;
		break;
	}
}

FVector2D UADSim2MetricSource::GetDataMin()
{
	switch (CurrentDataType)
	{
	case EChartType::Transient:
		if (!bShowNq1) return Data.GetTransW1Min();
		else return Data.GetTransNq1Min();
		break;
	case EChartType::Round:
		if (!bShowNq1) return Data.GetRoundW1Min();
		else return Data.GetRoundNq1Min();
		break;
	case EChartType::Simulation:
		if (!bShowNq1) return Data.GetSimW1Min();
		else return Data.GetSimNq1Min();
		break;
	}

	return Data.GetTransW1Min();
}

FVector2D UADSim2MetricSource::GetDataMax()
{
	switch (CurrentDataType)
	{
	case EChartType::Transient:
		if (!bShowNq1) return Data.GetTransW1Max();
		else return Data.GetTransNq1Max();
		break;
	case EChartType::Round:
		if (!bShowNq1) return Data.GetRoundW1Max();
		else return Data.GetRoundNq1Max();
		break;
	case EChartType::Simulation:
		if (!bShowNq1) return Data.GetSimW1Max();
		else return Data.GetSimNq1Max();
		break;
	}

	return Data.GetTransW1Max();
}

bool UADSim2MetricSource::Has2Series()
{
	int32 SeriesCount = 0;
	switch (CurrentDataType)
	{
	case EChartType::Transient:
		if (!bShowNq1)
		{
			if (Data.TransW1.Num())
			{
				if (Data.TransW1.Last().GetNum() > SeriesCount)SeriesCount = Data.TransW1.Last().GetNum();
			}
		}
		else
		{
			if (Data.TransNq1.Num())
			{
				if (Data.TransNq1.Last().NumDt > SeriesCount)SeriesCount = Data.TransNq1.Last().NumDt;
			}
		}
		break;
	case EChartType::Round:
		if (!bShowNq1)
		{
			if (Data.RoundW1.Num())
			{
				for (int i = 0; i < Data.RoundW1.Num(); ++i)
				{
					if (Data.RoundW1[i].Num() > SeriesCount)SeriesCount = Data.RoundW1[i].Num();
				}
			}
		}
		else
		{
			if (Data.RoundNq1.Num())
			{
				for (int i = 0; i < Data.RoundNq1.Num(); ++i)
				{
					if (Data.RoundNq1[i].Num() > SeriesCount)SeriesCount = Data.RoundNq1[i].Num();
				}
			}
		}
		break;
	case EChartType::Simulation:
		if (!bShowNq1)
		{
			SeriesCount = Data.SimW1.Num();
		}
		else
		{
			SeriesCount = Data.SimNq1.Num();
		}
		break;
	}

	return SeriesCount > 1;
}

void UADSim2MetricSource::SwitchType()
{
	switch (CurrentDataType)
	{
	case EChartType::Transient:
		if (!bShowNq1);
		else ;
		break;
	case EChartType::Round:
		if (!bShowNq1);
		else;
		break;
	case EChartType::Simulation:
		if (!bShowNq1);
		else;
		break;
	default: break;
	}
}
