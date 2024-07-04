#include "AlignmentComponent.h"
#include "CloudRegistration.h"
#include "Anchor.h"

UAlignmentComponent::UAlignmentComponent()
{
	// component can tick, should not be changed
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UAlignmentComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(false);
}

// Called every frame
void UAlignmentComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// if not localized yet, try to load or wait for them to localize
	if(localizedAnchors == false)
	{
		if(numAnchorsToLoad < 0)
		{
			numAnchorsToLoad = AAnchor::loadAnchors(this->extAnchorClass, this->intAnchorClass, this->GetOwner());
		}
		if (AAnchor::allAnchors.Num() < numAnchorsToLoad)
			return;
		localizedAnchors = true;
	}

	// align once with the loaded anchors
	bool ret = align();
	// disable tick if necessary, but only after the align has happened once
	if(!tick && ret)
	{
		SetComponentTickEnabled(false);
	}
}

// change the mode and return a string for debugging purposes
FString UAlignmentComponent::changeMode()
{
	if(mode == 0)
	{
		mode = 1;
		align();
		return "Static";
	}
	else
	{
		mode = 0;
		align();
		return "Weighted";
	}
}

// Align the clouds once, returns true on success
bool UAlignmentComponent::align()
{
	// if anchors is zero, return true (no anchors found)
	if(AAnchor::getNumAnchors() == 0)
	{
		return true;
	}

	FMatrix transformationMat;

	// if there are less than 3 anchors, compute the transformation matrix to align with the first anchor only 
	if(AAnchor::getNumAnchors() > 0 && AAnchor::getNumAnchors() < 3)
	{
		AAnchor* anchor = AAnchor::allAnchors[0];
		if(anchor->GetActorLocation().Equals(FVector(0,0,0)))
		{
			return false;
		}
		FTransform transformation = anchor->getCurrentExtPose().Inverse() * anchor->GetActorTransform();
		transformationMat = transformation.ToMatrixWithScale();
	}
	// calculate the transformation matrix with cloud registration
	else
	{
		TArray<FVector> fromPCL;
		TArray<FVector> toPCL;
		TArray<double> weights;

		// add each anchor to the lists and calculate the weight (distance)
		for (AAnchor* anchor : AAnchor::allAnchors)
		{
			float distance = FVector::Distance(anchor->GetActorLocation(), GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetCameraLocation());
			fromPCL.Add(anchor->getCurrentExtLocation());

			if(anchor->GetActorLocation().Equals(FVector(0,0,0)))
			{
				return false;
			}
			
			toPCL.Add(anchor->GetActorLocation());
			
			double weight = 1;

			if (mode == 0)
				weight = 1.0 / FMath::Pow(distance, 2);
			weights.Add(weight);
		}

		// calculate transformation matrix
		transformationMat = UCloudRegistration::calculateTransformationMatrix(fromPCL, toPCL, weights);

		// no transformation necessary
		if(transformationMat == FMatrix::Identity)
		{
			return true;
		}
	}

	// apply transformation
	FTransform transformation;
	transformation.SetFromMatrix(transformationMat);
	FTransform transform = GetOwner()->GetActorTransform();
	transform = transform * transformation ;
	GetOwner()->SetActorTransform(transform);
	return true;
}

void UAlignmentComponent::configureAlignmentComponent(TSubclassOf<AAnchor> INTAnchorClass,
	TSubclassOf<AActor> EXTAnchorClass, int alignmentMode, bool shouldTick)
{
	intAnchorClass = INTAnchorClass;
	extAnchorClass = EXTAnchorClass;
	mode = alignmentMode;
	tick = shouldTick;
	SetComponentTickEnabled(true);
}
