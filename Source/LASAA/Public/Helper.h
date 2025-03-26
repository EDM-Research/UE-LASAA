// Copyright Expertise centre for Digital Media, 2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include <Eigen/Core>
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Helper.generated.h"

using namespace Eigen;

/**
 * Helper library with several convert functions, requires Eigen
 */
UCLASS()
class LASAA_API UHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/** Enumeration to specify any cartesian axis in positive or negative directions */
	enum class EAxis
	{
		X, Y, Z,
		Xn, Yn, Zn,
	};

	// These axes must match the order in which they are declared in EAxis
	inline static const TArray<FVector> UnitVectors =
	{
		{  1,  0,  0 }, //  X
		{  0,  1,  0 }, //  Y
		{  0,  0,  1 }, //  Z
		{ -1,  0,  0 }, // -X
		{  0, -1,  0 }, // -Y
		{  0,  0, -1 }, // -Z
	};

	static const FVector& UnitVectorFromAxisEnum(const EAxis Axis)
	{
		return UnitVectors[std::underlying_type_t<EAxis>(Axis)];
	};

	/** Converts in-place the coordinate system of the given FTransform by specifying the source axes in terms of the destionation axes */
	static void ConvertCoordinateSystem(FTransform& Transform, const EAxis DstXInSrcAxis, const EAxis DstYInSrcAxis, const EAxis DstZInSrcAxis);

	/** Converts in-place an FTransform in Unreal coordinates to OpenCV coordinates */
	UFUNCTION(BlueprintCallable, Category="OpenCVHelperFunctions")
	static FTransform ConvertUnrealToOpenCV(FTransform Transform);

	/** Converts in-place an FTransform in OpenCV coordinates to Unreal coordinates */
	UFUNCTION(BlueprintCallable, Category="OpenCVHelperFunctions")
	static FTransform ConvertOpenCVToUnreal(FTransform Transform);

	static Matrix4d extrinsicFromRt(const Matrix3d& R, const Vector3d& tvec);
	static Matrix3d rodrigues(const Vector3d& rvec);
	static double variance(const VectorXd& vec);
	static FVector eigenVectorToUnreal(const Vector3d& vec);
	static FMatrix eigenMatrixToUnreal(const Matrix4d& mat);
	static Vector3d unrealVectorToEigen(const FVector& vec);
};
