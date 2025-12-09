// Copyright Expertise centre for Digital Media, 2025. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "LASAAProjectSettings.generated.h"

/**
 * Implements the settings for the LASAA Plugin Project Settings
 */
UCLASS(Config = LASAA, defaultconfig, meta = (DisplayName = "LASAA Plugin"))
class LASAA_API ULASAAProjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Constructors */

	ULASAAProjectSettings();

	/** Save info */

	UPROPERTY(EditAnywhere, Config, Category = Markers, meta = (DisplayName = "Content of markers file", MultiLine = true))
	FString Markers = "";
};
