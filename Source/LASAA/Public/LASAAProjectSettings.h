// Copyright Expertise centre for Digital Media, 2025. All rights reserved.

#pragma once

#include "SupportedCameraHardware.h"

// UE include
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

	/** Hardware info */

	UPROPERTY(EditAnywhere, Config, Category = Hardware, meta = (DisplayName = "IP address of device handling image processing"))
	FString ImageProcessingDeviceIPAddress = TEXT("0.0.0.0");

	UPROPERTY(EditAnywhere, Config, Category = Hardware, meta = (DisplayName = "Port of device handling image processing"))
	int32 ImageProcessingDevicePort = 8000;

	UPROPERTY(EditAnywhere, Config, Category = Hardware, meta = (DisplayName = "Camera Hardware"))
	ESupportedCameraHardware CameraHardware = ESupportedCameraHardware::SCH_MetaQuest3;

	// Keep markers in a file for now
	// TODO - restore when onpencv integration is ok
	//UPROPERTY(EditAnywhere, Config, Category = Markers, meta = (DisplayName = "Content of markers file", MultiLine = true))
	//FString Markers = "";
};
