// Copyright Expertise centre for Digital Media, 2025. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "SupportedCameraHardware.generated.h"

UENUM(BlueprintType)
enum class ESupportedCameraHardware : uint8
{
    SCH_MetaQuest3       UMETA(DisplayName = "Meta Quest 3"),
    SCH_Unsupported      UMETA(DisplayName = "UNSUPPORTED")
};
