// Fill out your copyright notice in the Description page of Project Settings.


#include "ADSimPawn.h"

// Sets default values
AADSimPawn::AADSimPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AADSimPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AADSimPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AADSimPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

RNGThread::RNGThread(int Count, int minNumber, int maxNumber, int chunkCount)
{
	m_Kill = false;
	m_Pause = false;

	//Initialize FEvent (as a cross platform (Confirmed Mac/Windows))
	m_semaphore = FGenericPlatformProcess::GetSynchEventFromPool(false);;

	m_MinInt = minNumber;
	m_MaxInt = maxNumber;

	m_chunkCount = chunkCount;

	m_amount = Count;
	m_RandomVecs.Reserve(m_amount);

	Thread = FRunnableThread::Create(this, TEXT("RNGThread"), 0, TPri_BelowNormal);
}

RNGThread::~RNGThread()
{
	if (m_semaphore != nullptr)
	{
		//Cleanup the FEvent
		FGenericPlatformProcess::ReturnSynchEventToPool(m_semaphore);
		m_semaphore = nullptr;
	}

	if (Thread != nullptr)
	{
		//Cleanup the worker thread
		delete Thread;
		Thread = nullptr;
	}
}

void RNGThread::CreateThread()
{
	Thread = FRunnableThread::Create(this, TEXT("RNGThread"), 0, TPri_BelowNormal);
}

bool RNGThread::Init()
{
	//Init the Data 
	m_RandomVecs.Empty();
	return true;
}

uint32 RNGThread::Run()
{
	//Initial wait before starting
	FPlatformProcess::Sleep(0.03);

	while (!m_Kill)
	{
		if (m_Pause)
		{
			//FEvent->Wait(); will "sleep" the thread until it will get a signal "Trigger()"
			m_semaphore->Wait();

			if (m_Kill)
			{
				return 0;
			}
		}
		else
		{
			//Create temporal array (chunk)
			TArray<FVector> ChunkArray;
			ChunkArray.Reserve(m_chunkCount);

			//Calculate random vectors and put them to the temporal array
			//I did it so we won't lock/unlock FCritical section each time we generating a new FVector (Locking and Unlocking is somewhat expensive).
			for (int i = 0; i < m_chunkCount; i++)
			{
				FVector RandomVec;
				RandomVec.X = (float)FMath::RandRange((int)m_MinInt, (int)m_MaxInt);
				RandomVec.Y = (float)FMath::RandRange((int)m_MinInt, (int)m_MaxInt);
				RandomVec.Z = 0;
				ChunkArray.Emplace(RandomVec);
			}

			//Critical section:
			m_mutex.Lock();
			//We are locking our FCriticalSection so no other thread will access it
			//And thus it is a thread-safe access now

			//Append the temporal array to the Actual storage array/
			m_RandomVecs.Append(ChunkArray);
			//Get array size
			int num = m_RandomVecs.Num();

			//Unlock FCriticalSection so other threads may use it.
			m_mutex.Unlock();

			//Pause Condition - if we RandomVectors contains more vectors than m_amount we shall pause the thread to release system resources.
			if (num > m_amount)
			{
				m_Pause = true;
			}
			//A little sleep between the chunks (So CPU will rest a bit -- (may be omitted))
			FPlatformProcess::Sleep(0.05);
		}
	}

	return 0;
}

void RNGThread::PauseThread()
{
	m_Pause = true;
}

void RNGThread::ContinueThread()
{
	m_Pause = false;

	if (m_semaphore)
	{
		//Here is a FEvent signal "Trigger()" -> it will wake up the thread.
		m_semaphore->Trigger();
	}
}

void RNGThread::Stop()
{
	m_Kill = true; //Thread kill condition "while (!m_Kill){...}"
	m_Pause = false;

	if (m_semaphore)
	{
		//We shall signal "Trigger" the FEvent (in case the Thread is sleeping it shall wake up!!)
		m_semaphore->Trigger();
	}
}

//Use this method to kill the thread!!
void RNGThread::EnsureCompletion()
{
	Stop();

	if (Thread != nullptr)
	{
		Thread->WaitForCompletion();
	}
}

//if the array is not yet ready we will generate the vector on the caller thread.
FVector GenerateRandomVecInRange(int min, int max)
{
	FVector WanderingPoint(0, 0, 0);
	WanderingPoint.X = (float)FMath::RandRange((int)min, (int)max);
	WanderingPoint.Y = (float)FMath::RandRange((int)min, (int)max);
	WanderingPoint.Z = (float)FMath::RandRange((int)min, (int)max);
	return WanderingPoint;
}

bool RNGThread::IsThreadPaused()
{
	return (bool)m_Pause;
}

FVector RNGThread::GetRandomVector()
{
	//Here we are retrieving the Vector from our storage array in a thread safe manner 
	//Despite this is a member method of this class it will be called from another thread (most likely from the GameThread) (This is by the way true for each public member methods except the "Run()" method) - So we must ensure the thread safety!
	//Critical section:
	m_mutex.Lock();
	int lastIndex = m_RandomVecs.Num() - 1;
	if (lastIndex < 0)
	{
		//The array is not ready yet :-0
		m_mutex.Unlock(); //We must unlock the critical section before the return to avoid a deadlock.
		//return GenerateRandomVecInRange(m_MinInt, m_MaxInt);
		return GenerateRandomVecInRange(-1.0f, -1.0f);
	}
	FVector vec2ret;
	vec2ret = m_RandomVecs[lastIndex];
	m_RandomVecs.RemoveAt(lastIndex);

	//Some automation: if we have less than 10% random FVectors in our array we will UnPause the thread. (maybe omitted).
	/*if (m_RandomVecs.Num() < m_amount / 10)
	{
		RandomVecsPoolThreadHandle->ContinueThread();
	}*/

	//Critical section ends here.
	m_mutex.Unlock();

	//return random vector to the caller.
	return vec2ret;
}

int32 RNGThread::GetRandomVectorNum()
{
	int32 NumTemp = -1;
	m_mutex.Lock();
	NumTemp = m_RandomVecs.Num();
	m_mutex.Unlock();
	return NumTemp;
}
