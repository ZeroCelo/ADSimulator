// Fill out your copyright notice in the Description page of Project Settings.


#include "ADSim2ChartWidget.h"
#include "ADSim2Pawn.h"

#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(ChartWidgetLog2);

UADSim2ChartWidget::UADSim2ChartWidget(const FObjectInitializer &ObjectInitializer) :Super(ObjectInitializer)
{
	//MetricsCurrentRef1 = nullptr;
	//MetricsCurrentRef2 = nullptr;
	//MetricsCurrentRef3 = nullptr;
	
	MetricsW1_Trans = nullptr;
	MetricsW1_Round = nullptr;
	MetricsW1_IC = nullptr;
	MetricsW2_Trans = nullptr;
	MetricsW2_Round = nullptr;
	MetricsW2_IC = nullptr;
	MetricsW3_Trans = nullptr;
	MetricsW3_Round = nullptr;
	MetricsW3_IC = nullptr;

	MetricsNq1_Trans = nullptr;
	MetricsNq1_Round = nullptr;
	MetricsNq1_IC = nullptr;
	MetricsNq2_Trans = nullptr;
	MetricsNq2_Round = nullptr;
	MetricsNq2_IC = nullptr;
	MetricsNq3_Trans = nullptr;
	MetricsNq3_Round = nullptr;
	MetricsNq3_IC = nullptr;
}

void UADSim2ChartWidget::NativeConstruct()
{
	//MetricsCurrentRef1 = NewObject< UADSim2MetricSource >(this, TEXT("MetricRef1Obj"));
	//MetricsCurrentRef2 = NewObject< UADSim2MetricSource >(this, TEXT("MetricRef2Obj"));
	//MetricsCurrentRef3 = NewObject< UADSim2MetricSource >(this, TEXT("MetricRef3Obj"));

	MetricsW1_Trans = NewObject< UADSim2MetricSource >(this, TEXT("MetricW1TransObj"));
	MetricsW1_Round = NewObject< UADSim2MetricSource >(this, TEXT("MetricW1RoundObj"));
	MetricsW1_IC = NewObject< UADSim2MetricSource >(this, TEXT("MetricW1ICObj"));
	MetricsNq1_Trans = NewObject< UADSim2MetricSource >(this, TEXT("MetricNq1TransObj"));
	MetricsNq1_Round = NewObject< UADSim2MetricSource >(this, TEXT("MetricNq1RoundObj"));
	MetricsNq1_IC = NewObject< UADSim2MetricSource >(this, TEXT("MetricNq1ICObj"));

	MetricsW2_Trans = NewObject< UADSim2MetricSource >(this, TEXT("MetricW2TransObj"));
	MetricsW2_Round = NewObject< UADSim2MetricSource >(this, TEXT("MetricW2RoundObj"));
	MetricsW2_IC = NewObject< UADSim2MetricSource >(this, TEXT("MetricW2ICObj"));
	MetricsNq2_Trans = NewObject< UADSim2MetricSource >(this, TEXT("MetricNq2TransObj"));
	MetricsNq2_Round = NewObject< UADSim2MetricSource >(this, TEXT("MetricNq2RoundObj"));
	MetricsNq2_IC = NewObject< UADSim2MetricSource >(this, TEXT("MetricNq2ICObj"));

	MetricsW3_Trans = NewObject< UADSim2MetricSource >(this, TEXT("MetricW3TransObj"));
	MetricsW3_Round = NewObject< UADSim2MetricSource >(this, TEXT("MetricW3RoundObj"));
	MetricsW3_IC = NewObject< UADSim2MetricSource >(this, TEXT("MetricW3ICObj"));
	MetricsNq3_Trans = NewObject< UADSim2MetricSource >(this, TEXT("MetricNq3TransObj"));
	MetricsNq3_Round = NewObject< UADSim2MetricSource >(this, TEXT("MetricNq3RoundObj"));
	MetricsNq3_IC = NewObject< UADSim2MetricSource >(this, TEXT("MetricNq3ICObj"));

	Super::NativeConstruct();
}

void UADSim2ChartWidget::UpdateDataSource1()
{
	/*UpdateDataSource1BP();
	ConfigureDataSource1();
	UpdateDataSourceRange1BP();*/
}

void UADSim2ChartWidget::UpdateDataSource2()
{
	/*UpdateDataSource2BP();
	ConfigureDataSource2();
	UpdateDataSourceRange2BP();*/
}

void UADSim2ChartWidget::UpdateDataSource3()
{
	/*UpdateDataSource3BP();
	ConfigureDataSource3();
	UpdateDataSourceRange3BP();*/
}

void UADSim2ChartWidget::ConfigureDataSource1()
{
	/*if (MetricsCurrentRef1 != nullptr)
	{
		int32 max = IKantanCartesianDatasourceInterface::Execute_GetNumSeries(MetricsCurrentRef1); //<---- Crash de vez em quando
		if (max > 0)
		{
			for (int i = 0; i < max; ++i)
			{
				ConfigureSeries1BP(IKantanCartesianDatasourceInterface::Execute_GetSeriesId(MetricsCurrentRef1, i), true, true);
				EnableSeries1BP(IKantanCartesianDatasourceInterface::Execute_GetSeriesId(MetricsCurrentRef1, i), true);
				//AddEnablerButton1BP(IKantanCartesianDatasourceInterface::Execute_GetSeriesId(MetricsCurrentRef1, i));
			}
		}
	}*/
}

void UADSim2ChartWidget::ConfigureDataSource2()
{
	/*if (MetricsCurrentRef2 != nullptr)
	{
		int32 max = IKantanCartesianDatasourceInterface::Execute_GetNumSeries(MetricsCurrentRef2);
		if (max > 0)
		{
			for (int i = 0; i < max; ++i)
			{
				ConfigureSeries2BP(IKantanCartesianDatasourceInterface::Execute_GetSeriesId(MetricsCurrentRef2, i), true, true);
				EnableSeries2BP(IKantanCartesianDatasourceInterface::Execute_GetSeriesId(MetricsCurrentRef2, i), true);
				//AddEnablerButton1BP(IKantanCartesianDatasourceInterface::Execute_GetSeriesId(MetricsCurrentRef2, i));
			}
		}
	}*/
}

void UADSim2ChartWidget::ConfigureDataSource3()
{
	/*if (MetricsCurrentRef3 != nullptr)
	{
		int32 max = IKantanCartesianDatasourceInterface::Execute_GetNumSeries(MetricsCurrentRef3);
		if (max > 0)
		{
			for (int i = 0; i < max; ++i)
			{
				ConfigureSeries3BP(IKantanCartesianDatasourceInterface::Execute_GetSeriesId(MetricsCurrentRef3, i), true, true);
				EnableSeries3BP(IKantanCartesianDatasourceInterface::Execute_GetSeriesId(MetricsCurrentRef3, i), true);
				//AddEnablerButton1BP(IKantanCartesianDatasourceInterface::Execute_GetSeriesId(MetricsCurrentRef3, i));
			}
		}
	}*/
}

void UADSim2ChartWidget::AddEnablerButtons()
{
	/*if (MetricsCurrentRef1 != nullptr)
	{
		int32 max = IKantanCartesianDatasourceInterface::Execute_GetNumSeries(MetricsCurrentRef1);
		if (max > 0)
		{
			for (int i = 0; i < max; ++i)
			{
				AddEnablerButton1BP(IKantanCartesianDatasourceInterface::Execute_GetSeriesId(MetricsCurrentRef1, i));
			}
		}
	}*/
}

void UADSim2ChartWidget::SetupDataNameW1(FString NameStrW, FString NameStrNq, int32 MaxSim)
{
	//if (MetricsCurrentRef1 != nullptr)
	//{
		//MetricsCurrentRef1->SetDataName(NameStrW, NameStrNq, MaxSim);
	//}
}

void UADSim2ChartWidget::SetupDataNameW2(FString NameStrW, FString NameStrNq, int32 MaxSim)
{
	//if (MetricsCurrentRef2 != nullptr)
	//{
		//MetricsCurrentRef2->SetDataName(NameStrW, NameStrNq, MaxSim);
	//}
}

void UADSim2ChartWidget::SetupDataNameW3(FString NameStrW, FString NameStrNq, int32 MaxSim)
{
	//if (MetricsCurrentRef3 != nullptr)
	//{
		//MetricsCurrentRef3->SetDataName(NameStrW, NameStrNq, MaxSim);
	//}
}

void UADSim2ChartWidget::SetDataRef1(bool bNq, EChartType Type, TArray< FKantanSeriesData >* DataRef)
{
	switch (Type)
	{
	case EChartType::Transient:
		if (bNq) 
		{
			if(MetricsNq1_Trans != nullptr)	MetricsNq1_Trans->SetRefData(DataRef);
		}
		else
		{
			if(MetricsW1_Trans != nullptr) MetricsW1_Trans->SetRefData(DataRef);
		}
		break;
	case EChartType::Round:
		if (bNq)
		{
			if(MetricsNq1_Round != nullptr)MetricsNq1_Round->SetRefData(DataRef);
		}
		else
		{
			if(MetricsW1_Round != nullptr)MetricsW1_Round->SetRefData(DataRef);
		}
		break;
	case EChartType::Simulation:
		if (bNq)
		{
			if (MetricsNq1_IC != nullptr)MetricsNq1_IC->SetRefData(DataRef);
		}
		else
		{
			if (MetricsW1_IC != nullptr)MetricsW1_IC->SetRefData(DataRef);
		}
		break;
	default: 
		break;
	}
}

void UADSim2ChartWidget::SetDataRef2(bool bNq, EChartType Type, TArray< FKantanSeriesData >* DataRef)
{
	switch (Type)
	{
	case EChartType::Transient:
		if (bNq)
		{
			if (MetricsNq2_Trans != nullptr) MetricsNq2_Trans->SetRefData(DataRef);
		}
		else
		{
			if (MetricsW2_Trans != nullptr) MetricsW2_Trans->SetRefData(DataRef);
		}
		break;
	case EChartType::Round:
		if (bNq)
		{
			if (MetricsNq2_Round != nullptr)MetricsNq2_Round->SetRefData(DataRef);
		}
		else
		{
			if (MetricsW2_Round != nullptr)MetricsW2_Round->SetRefData(DataRef);
		}
		break;
	case EChartType::Simulation:
		if (bNq)
		{
			if (MetricsNq2_IC != nullptr)MetricsNq2_IC->SetRefData(DataRef);
		}
		else
		{
			if (MetricsW2_IC != nullptr)MetricsW2_IC->SetRefData(DataRef);
		}
		break;
	default:
		break;
	}
}

void UADSim2ChartWidget::SetDataRef3(bool bNq, EChartType Type, TArray< FKantanSeriesData >* DataRef)
{
	switch (Type)
	{
	case EChartType::Transient:
		if (bNq)
		{
			if (MetricsNq3_Trans != nullptr) MetricsNq3_Trans->SetRefData(DataRef);
		}
		else
		{
			if (MetricsW3_Trans != nullptr) MetricsW3_Trans->SetRefData(DataRef);
		}
		break;
	case EChartType::Round:
		if (bNq)
		{
			if (MetricsNq3_Round != nullptr)MetricsNq3_Round->SetRefData(DataRef);
		}
		else
		{
			if (MetricsW3_Round != nullptr)MetricsW3_Round->SetRefData(DataRef);
		}
		break;
	case EChartType::Simulation:
		if (bNq)
		{
			if (MetricsNq3_IC != nullptr)MetricsNq3_IC->SetRefData(DataRef);
		}
		else
		{
			if (MetricsW3_IC != nullptr)MetricsW3_IC->SetRefData(DataRef);
		}
		break;
	default:
		break;
	}
}

