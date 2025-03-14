#include "Anchor.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonSerializer.h"
#include "OculusXRAnchors.h"
#include "OculusXRAnchorComponent.h"
#include "OculusXRAnchorBPFunctionLibrary.h"

// Sets default values
AAnchor::AAnchor()
{
	PrimaryActorTick.bCanEverTick = false;

	sceneComponent = CreateDefaultSubobject<USceneComponent>("RootSceneComponent");
	RootComponent = sceneComponent;
}

// Called when the game starts or when spawned
void AAnchor::BeginPlay()
{
	Super::BeginPlay();
	allAnchors.Add(this);
}

void AAnchor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	eraseFromList();
	allAnchors.Remove(this);
	if(extPair != nullptr)
	{
		extPair->Destroy();
	}
}

bool AAnchor::createAnchor(FTransform extTrans)
{
	// check if the anchor component already exists
	UOculusXRAnchorComponent* anchorComponent = Cast<UOculusXRAnchorComponent>(GetComponentByClass(UOculusXRAnchorComponent::StaticClass()));

	// if exists, do not create a new one
	if(anchorComponent != nullptr)
		return false;

	UE_LOG(LogTemp, Display, TEXT("Creating anchor"));

	this->setGtExtPose(extTrans);

	// create the anchor via oculus sdk, and save the anchor once created
	EOculusXRAnchorResult::Type result;
	OculusXRAnchors::FOculusXRAnchors::CreateSpatialAnchor(this->GetActorTransform(), this,
		FOculusXRSpatialAnchorCreateDelegate::CreateLambda([this](EOculusXRAnchorResult::Type result, UOculusXRAnchorComponent* anchorComponent)
		{
			UE_LOG(LogTemp, Display, TEXT("Anchor created: %d"), result);
			this->setUuid(anchorComponent->GetUUID().ToString());
			this->save();
		}), result);
	return true;
}

bool AAnchor::save()
{
	// check if the anchor component exists
	UOculusXRAnchorComponent* anchorComponent = Cast<UOculusXRAnchorComponent>(GetComponentByClass(UOculusXRAnchorComponent::StaticClass()));

	// if not exists, do not try to save
	if(anchorComponent == nullptr)
		return false;

	UE_LOG(LogTemp, Display, TEXT("Saving anchor"));

	// save the anchor in the headset and write uuid with external pose to JSON file
	EOculusXRAnchorResult::Type result;
	OculusXRAnchors::FOculusXRAnchors::SaveAnchor(anchorComponent, EOculusXRSpaceStorageLocation::Local,
		FOculusXRAnchorSaveDelegate::CreateLambda([this](EOculusXRAnchorResult::Type result, UOculusXRAnchorComponent* anchorComponent)
		{
			// save uuid and orb pose to file
			UE_LOG(LogTemp, Display, TEXT("Saved anchor: %d"), result);

			this->addToList();
			this->writeToJson();
		}), result
		);
	return true;
}

void AAnchor::eraseFromList()
{
	int idx;
	bool found = anchorStorage.anchorUuids.Find(this->uuid, idx);
	if (found)
	{
		anchorStorage.anchorUuids.RemoveAt(idx);
		anchorStorage.anchorTransforms.RemoveAt(idx);
	}
}

void AAnchor::addToList()
{
	if(anchorStorage.anchorUuids.Num() >= 64)
		return;
	anchorStorage.anchorUuids.Add(this->uuid);
	anchorStorage.anchorTransforms.Add(this->gtExtPose);
}

void AAnchor::writeToJson()
{
	TSharedRef<FJsonObject> outJsonObject = MakeShareable(new FJsonObject);
	FJsonObjectConverter::UStructToJsonObject(FAnchorStruct::StaticStruct(), &anchorStorage, outJsonObject,0,0);

	FString outputString;
	TSharedRef<TJsonWriter<>> writer = TJsonWriterFactory<>::Create(&outputString);
	FJsonSerializer::Serialize(outJsonObject, writer);
	FFileHelper::SaveStringToFile(outputString, *filePath);
	UE_LOG(LogTemp, Display, TEXT("Wrote %d anchors to %s"), anchorStorage.anchorUuids.Num(), *filePath);
}

void AAnchor::readFromJson()
{
	FString out;
	FFileHelper::LoadFileToString(out, *filePath);
	
	TSharedPtr<FJsonObject> jObj;
	TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(out);
	if(!FJsonSerializer::Deserialize(reader, jObj))
	{
		UE_LOG(LogTemp, Display, TEXT("Can not create json from this string"));
		return;
	}
	FJsonObjectConverter::JsonObjectToUStruct(jObj.ToSharedRef(), FAnchorStruct::StaticStruct(), &anchorStorage, 0,0);
}

void AAnchor::erase()
{
	// check if anchor component exists
	UOculusXRAnchorComponent* anchorComponent = Cast<UOculusXRAnchorComponent>(GetComponentByClass(UOculusXRAnchorComponent::StaticClass()));

	// if not exists, remove the anchor
	if(anchorComponent == nullptr)
	{
		this->eraseFromList();
		this->writeToJson();
		this->Destroy();
		return;
	}

	// erase the anchor, once erased, also remove from storage
	EOculusXRAnchorResult::Type result;
	OculusXRAnchors::FOculusXRAnchors::EraseAnchor(anchorComponent,
		FOculusXRAnchorEraseDelegate::CreateLambda([this](EOculusXRAnchorResult::Type result, FOculusXRUUID UUID)
		{
			this->eraseFromList();
			this->writeToJson();
			this->Destroy();
		}), result);
}

int AAnchor::loadAnchors(UClass* extClass, UClass* anchorClass, AActor* newOwner)
{
	UE_LOG(LogTemp, Display, TEXT("Loading anchors from %s"), *filePath);

	// read anchor uuids and external poses from JSON file into anchorStorage
	readFromJson();

	uint32 maxToLoad = 64;

	// obtain uuids
	EOculusXRSpaceStorageLocation queryLocation = EOculusXRSpaceStorageLocation::Local;
	TArray<FOculusXRUUID> uuids;
	for (FString strid : anchorStorage.anchorUuids)
	{
		FOculusXRUUID uuid = UOculusXRAnchorBPFunctionLibrary::StringToAnchorUUID(strid);
		uuids.Add(uuid);
	}

	UE_LOG(LogTemp, Display, TEXT("%d UUIDS loaded"), uuids.Num());
	
	// Query the anchors with the uuids from anchorStorage
	EOculusXRAnchorResult::Type result;
	OculusXRAnchors::FOculusXRAnchors::QueryAnchors(uuids, queryLocation,
		FOculusXRAnchorQueryDelegate::CreateLambda([extClass, anchorClass, newOwner](EOculusXRAnchorResult::Type result, const TArray<FOculusXRSpaceQueryResult>& results)
		{
			// if query successfull
			if(result == EOculusXRAnchorResult::Success)
			{
				// create each of the anchors
				for(auto& it : results)
				{
					UE_LOG(LogTemp, Display, TEXT("Spawning with uuid: %s"), *it.UUID.ToString())
					UE_LOG(LogTemp, Display, TEXT("Before spawning"))
					// Log the extclass and anchorclass
					// spawn the spatial anchor using the given class (must be derived from AAnchor class)
					AActor* spawned = UOculusXRAnchorBPFunctionLibrary::SpawnActorWithAnchorQueryResults(
						 newOwner->GetWorld(),
						it,
						anchorClass,
						nullptr,
						nullptr,
						ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
					UE_LOG(LogTemp, Display, TEXT("After spawning"))
					// cast to AAnchor and fill the uuid and gt external pose
					AAnchor* spawnedAnchor = Cast<AAnchor>(spawned);
					spawnedAnchor->setUuid(it.UUID.ToString());
					spawnedAnchor->setGtExtPose(anchorStorage.getTransform(it.UUID.ToString()));

					// also spawn the external anchor in relation to the anchor parent
					AActor* extAnchor = newOwner->GetWorld()->SpawnActor<AActor>(extClass);
					extAnchor->SetActorTransform(anchorStorage.getTransform(it.UUID.ToString()));
					extAnchor->AttachToActor(newOwner, FAttachmentTransformRules::KeepRelativeTransform);
					spawnedAnchor->setExtPairAndCalibrationOffset(extAnchor);
				}
			}
		}), result
		);
	return uuids.Num();
}

// delete all anchors, TODO erase anchors
void AAnchor::resetAnchors()
{
	anchorStorage.anchorTransforms.Empty();
	anchorStorage.anchorUuids.Empty();

	TArray<AAnchor*> copy = allAnchors;
	for(AAnchor* anchor: copy)
	{
		if(anchor)
			anchor->Destroy();
	}
	writeToJson();
}

void AAnchor::setUuid(FString id)
{
	this->uuid = id;
}

void AAnchor::addCalibrationOffset()
{
	FTransform currentPose = this->gtExtPose;
	FTransform newPose = cam2xr.Inverse() * currentPose;
	extPair->SetActorRelativeTransform(newPose);
}

int AAnchor::getNumAnchors()
{
	return allAnchors.Num();
}

void AAnchor::setCalibrationOffset(const FTransform& calibration)
{
	AAnchor::cam2xr = calibration;
	for(AAnchor* anchor: AAnchor::allAnchors)
	{
		anchor->addCalibrationOffset();
	}
}

double AAnchor::getAvgPairDistance()
{
	double distance = 0;
	for (AAnchor* anchor: AAnchor::allAnchors)
	{
		double localDistance = FVector::Distance(anchor->getCurrentExtLocation(), anchor->GetActorLocation());
		distance += localDistance;
	}
	return distance/allAnchors.Num();
}

FString AAnchor::getUuid()
{
	return uuid;
}

void AAnchor::setGtExtPose(FTransform extTrans)
{
	this->gtExtPose = extTrans;
}

FVector AAnchor::getCurrentExtLocation()
{
	if(extPair != nullptr)
		return extPair->GetActorLocation();
	return FVector::Zero();
}

FTransform AAnchor::getCurrentExtPose()
{
	if(extPair != nullptr)
		return extPair->GetActorTransform();
	return FTransform::Identity;
}

void AAnchor::setExtPairAndCalibrationOffset(AActor* extActor)
{
	this->extPair = extActor;
	addCalibrationOffset();
}

AActor* AAnchor::getExtPair()
{
	return extPair;
}

