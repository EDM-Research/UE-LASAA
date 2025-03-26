// Copyright Expertise centre for Digital Media, 2024. All Rights Reserved.


#include "XRCalibration.h"
#include <Eigen/Core>
#include <Eigen/SVD>
#include <unsupported/Eigen/NonLinearOptimization>
#include "Anchor.h"

using namespace Eigen;

UXRCalibration::UXRCalibration()
{
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UXRCalibration::BeginPlay()
{
	Super::BeginPlay();

	loadFromFile();
	AAnchor::setCalibrationOffset(GetRelativeTransform());
}


// Called every frame
void UXRCalibration::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UXRCalibration::calibrate()
{
	if(calibrated)
	{
		UE_LOG(LogTemp, Display, TEXT("Calibration failed: already calibrated"));
		return;
	}
	
	int numAnchors = AAnchor::allAnchors.Num();
	if(numAnchors <= 6)
	{
		UE_LOG(LogTemp, Display, TEXT("Calibration failed: not enough anchors placed yet"));
		return;
	}

	MatrixXd pointsI(numAnchors,3);
	MatrixXd pointsE(numAnchors,3);
	int row = 0;
	for(AAnchor* anchor: AAnchor::allAnchors)
	{
		FVector pointI = anchor->GetActorLocation();
		FVector pointE = anchor->getCurrentExtLocation();
		
		pointsI.row(row) << pointI.X, pointI.Y, pointI.Z;
		pointsE.row(row) << pointE.X, pointE.Y, pointE.Z;
		++row;
	}
	
	VectorXd initialGuess(6);
	initialGuess.setZero();

	Model model (pointsI, pointsE);
	NumericalDiff<Model> numDiff(model);
	
	LevenbergMarquardt<NumericalDiff<Model>, double> lm(numDiff);
	lm.parameters.maxfev = 1000000;
	lm.parameters.ftol = 1.0e-13;
	VectorXd z = initialGuess;
	int ret = lm.minimize(z);

	Matrix3d R = UHelper::rodrigues(z.head<3>());
	Matrix4d transformationMatrix = UHelper::extrinsicFromRt(R, z.tail<3>());
	FMatrix transformationMat = UHelper::eigenMatrixToUnreal(transformationMatrix);

	FTransform transformation;
	transformation.SetFromMatrix(transformationMat);
	
	UE_LOG(LogTemp, Display, TEXT("%s"), *transformation.ToString());

	Matrix4d A = Matrix4d::Zero();
	Vector3d t = Vector3d::Zero();
	for (int i = 0; i < numAnchors; i++)
	{
		AAnchor* anchor = AAnchor::allAnchors[i];
		FTransform cam2w = anchor->getCurrentExtPose() * transformation;
		FTransform xr2w = anchor->GetActorTransform();

		FTransform cam2xr = cam2w * xr2w.Inverse();

		auto quat = cam2xr.GetRotation();
		Vector4d q {quat.X, quat.Y, quat.Z, quat.W};
		A += q * q.transpose();

		auto uT = cam2xr.GetTranslation();
		t += Vector3d{uT.X, uT.Y, uT.Z};
	}
	A /= numAnchors;
	JacobiSVD svd(A, Eigen::ComputeThinU | Eigen::ComputeThinV);
	VectorXd singularValues = svd.singularValues();
	MatrixXd U = svd.matrixU();

	UE::Math::TQuat avgQ {U(0, 0), U(1,0), U(2,0), U(3,0)};
	Vector3d avgTE = t / numAnchors;
	UE::Math::TVector avgT{avgTE.x(), avgTE.y(), avgTE.z()};

	FTransform avgCam2XR;
	avgCam2XR.SetRotation(avgQ);
	avgCam2XR.SetTranslation(avgT);
	
	SetRelativeTransform(avgCam2XR);
	writeToFile();
	
	AAnchor::setCalibrationOffset(GetRelativeTransform());
	calibrated = true;
}


void UXRCalibration::resetCalibration()
{
	SetRelativeTransform(FTransform::Identity);
	AAnchor::setCalibrationOffset(GetRelativeTransform());
	calibrated = false;
}

void UXRCalibration::changeCalibration()
{
	if (filename == filenameSpatial)
	{
		filename = filenameOld;
	}
	else
	{
		filename = filenameSpatial;
	}
	loadFromFile();
	AAnchor::setCalibrationOffset(GetRelativeTransform());
}

void UXRCalibration::loadFromFile()
{
	TArray<FString> outStrings;
	FFileHelper::LoadANSITextFileToStrings(*FPaths::ConvertRelativePathToFull(FPaths::ProjectPluginsDir() + "LASAA/" + filename), NULL, outStrings);

	FMatrix mat;
	int i = 0;
	for (FString line: outStrings)
	{
		int j = 0;
		TArray<FString> vals;
		line.ParseIntoArray(vals, TEXT(","));

		for (FString val: vals)
		{
			mat.M[i][j] = FCString::Atof(*val);
			if (j == 3)
				mat.M[i][j] = mat.M[i][j] * 100;
			j++;
		}
		i++;
	}

	FTransform transform;
	transform.SetFromMatrix(mat.GetTransposed());

	transform = UHelper::ConvertOpenCVToUnreal(transform);
	transform = transform.Inverse();
	SetRelativeTransform(transform);

	UE_LOG(LogTemp, Display, TEXT("%s"), *transform.ToString());
	calibrated = true;
}

void UXRCalibration::writeToFile(int i)
{
	FTransform transform = GetRelativeTransform();
	transform = transform.Inverse();
	transform = UHelper::ConvertUnrealToOpenCV(transform);
	
	FMatrix mat = transform.ToMatrixWithScale().GetTransposed();

	TArray<FString> allStrings;
	for(int row = 0; row < 4 ; row++)
	{
		FString sRow = "";
		for(int col = 0; col < 4; col++)
		{
			if(col == 3)
			{
				if (row < 3)
				{
					sRow += FString::SanitizeFloat(mat.M[row][col] / 100);
				}
				else
				{
					sRow += FString::SanitizeFloat(mat.M[row][col]);
				}
			}
			else
			{
				sRow += FString::SanitizeFloat(mat.M[row][col]) + ",";
			}
		}
		allStrings.Add(sRow);
	}
	if (i == 0)
		FFileHelper::SaveStringArrayToFile(allStrings, *FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() + "CalibrationData/" + filenameSpatial));
	else
		FFileHelper::SaveStringArrayToFile(allStrings, *FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() + "CalibrationData/" + filenameSpatial + FString::FromInt(i)));
}