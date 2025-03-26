// Copyright Expertise centre for Digital Media, 2024. All Rights Reserved.

#pragma once


#include "CoreMinimal.h"
#include "Chaos/DenseMatrix.h"
#include "Chaos/ImplicitQRSVD.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CloudRegistration.generated.h"

using namespace Chaos;

/**
 * Unreal function library to find the transformation that aligns the point clouds
 */
UCLASS()
class LASAA_API UCloudRegistration : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static PMatrix<double,3,3> calculateICPCovarianceMtx(int length, TArray<FVector> centerPCL, TArray<FVector> centerMCL)
	{
		PMatrix<double,3,3> covariance = PMatrix<double,3,3>();
		double sumXX = 0.0, sumXY = 0.0, sumXZ = 0.0;
		double sumYX = 0.0, sumYY = 0.0, sumYZ = 0.0;
		double sumZX = 0.0, sumZY = 0.0, sumZZ = 0.0;
		for (int i = 0; i < length; i++)
		{
			sumXX += centerPCL[i][0] * centerMCL[i][0];
			sumXY += centerPCL[i][0] * centerMCL[i][1];
			sumXZ += centerPCL[i][0] * centerMCL[i][2];
			sumYX += centerPCL[i][1] * centerMCL[i][0];
			sumYY += centerPCL[i][1] * centerMCL[i][1];
			sumYZ += centerPCL[i][1] * centerMCL[i][2];
			sumZX += centerPCL[i][2] * centerMCL[i][0];
			sumZY += centerPCL[i][2] * centerMCL[i][1];
			sumZZ += centerPCL[i][2] * centerMCL[i][2];
		}
		covariance.SetAt(0, 0, sumXX / length);
		covariance.SetAt(0, 1, sumXY / length);
		covariance.SetAt(0, 2, sumXZ / length);
		covariance.SetAt(1, 0, sumYX / length);
		covariance.SetAt(1, 1, sumYY / length);
		covariance.SetAt(1, 2, sumYZ / length);
		covariance.SetAt(2, 0, sumZX / length);
		covariance.SetAt(2, 1, sumZY / length);
		covariance.SetAt(2, 2, sumZZ / length);
		return covariance;
	}

	static FMatrix calculateTransformationMatrix(TArray<FVector> fromPCL3, TArray<FVector> toPCL3, TArray<double> weights)
	{
		if (fromPCL3.Num() < 3 || fromPCL3.Num() != toPCL3.Num())
		{
			return FMatrix::Identity;
		}

		FVector pclCOM = FVector(0,0,0);
		FVector mclCOM = FVector(0,0,0);

		TArray<FVector> centerPCL;
		TArray<FVector> centerMCL;

		double totalWeight = 0.0;
		for(int i = 0; i < fromPCL3.Num(); i++)
		{
			double weight = weights[i];
			totalWeight += weight;
			pclCOM += fromPCL3[i] * weight;
			mclCOM += toPCL3[i] * weight;
		}
		pclCOM *= (1.0/totalWeight);
		mclCOM *= (1.0/totalWeight);

		for (int i = 0; i < fromPCL3.Num(); i++)
		{
			FVector pclPt = fromPCL3[i] - pclCOM;
			centerPCL.Add(pclPt);
			FVector mclPt = toPCL3[i] - mclCOM;
			centerMCL.Add(mclPt);
		}
		
		int vectorSize = toPCL3.Num();

		PMatrix<double,3,3> covariance = calculateICPCovarianceMtx(vectorSize, centerPCL, centerMCL);
		PMatrix<double,3,3> ut;
		TVector<double,3> sigma;
		PMatrix<double,3,3> vt;
		
		SingularValueDecomposition(covariance.GetTransposed(), ut, sigma, vt);
		auto v = vt.GetTransposed();
		
		PMatrix<double,3,3> R = v * ut;
		if(R.Determinant() < 0)
		{
			vt.SetAt(2,0, vt.GetAt(2,0) * -1.);
			vt.SetAt(2,1, vt.GetAt(2,1) * -1.);
			vt.SetAt(2,2, vt.GetAt(2,2) * -1.);
			R = vt.GetTransposed() * ut;
		}
		
		FVector T = mclCOM - (R.GetTransposed() * pclCOM);
		
		FMatrix mat = FMatrix::Identity;
		mat.M[0][0] = R.GetAt(0,0);
		mat.M[0][1] = R.GetAt(0,1);
		mat.M[0][2] = R.GetAt(0,2);
		mat.M[0][3] = T[0];
		mat.M[1][0] = R.GetAt(1,0);
		mat.M[1][1] = R.GetAt(1,1);
		mat.M[1][2] = R.GetAt(1,2);
		mat.M[1][3] = T[1];
		mat.M[2][0] = R.GetAt(2,0);
		mat.M[2][1] = R.GetAt(2,1);
		mat.M[2][2] = R.GetAt(2,2);
		mat.M[2][3] = T[2];
		
		return mat.GetTransposed();
	}	
};
