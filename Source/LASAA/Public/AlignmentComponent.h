// Copyright Expertise centre for Digital Media, 2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AlignmentComponent.generated.h"

class AAnchor;

UCLASS( Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LASAA_API UAlignmentComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
public:	
	UAlignmentComponent();

	// class to spawn external anchors
	TSubclassOf<AActor> extAnchorClass;
	// class to spawn internal anchors 
	TSubclassOf<AAnchor> intAnchorClass;

	// is executed every frame if tick is enabled
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// changes the alignment mode: weighted and static
	UFUNCTION(BlueprintCallable, Category="AnchorFunctions")
	FString changeMode();

	// aligns the cloud once
	UFUNCTION(BlueprintCallable, Category="AnchorFunctions")
	bool align();

	// configure component (must be called to let the component work)
	UFUNCTION(BlueprintCallable, Category="AnchorFunctions")
	void configureAlignmentComponent(TSubclassOf<AAnchor> INTAnchorClass, TSubclassOf<AActor> EXTAnchorClass, int alignmentMode, bool shouldTick);

	// the alignment mode: 0 - weighted, 1 - static
	int mode;

	// should component tick or not
	bool tick = true;

private:
	// indicates whether the anchors are localized
	bool localizedAnchors = false;
	// anchors to load from memory
	int numAnchorsToLoad = -1;
};
