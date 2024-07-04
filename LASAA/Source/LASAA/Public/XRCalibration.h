// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Eigen/Core>
#include "Helper.h"
#include "Components/SceneComponent.h"
#include "XRCalibration.generated.h"

using namespace Eigen;

// Generic functor
template<typename _Scalar, int NX = Eigen::Dynamic, int NY = Eigen::Dynamic>
struct Functor
{
	typedef _Scalar Scalar;
	enum {
		InputsAtCompileTime = NX,
		ValuesAtCompileTime = NY
	};
	typedef Eigen::Matrix<Scalar,InputsAtCompileTime,1> InputType;
	typedef Eigen::Matrix<Scalar,ValuesAtCompileTime,1> ValueType;
	typedef Eigen::Matrix<Scalar,ValuesAtCompileTime,InputsAtCompileTime> JacobianType;

	int m_inputs, m_values;

	Functor() : m_inputs(InputsAtCompileTime), m_values(ValuesAtCompileTime) {}
	Functor(int inputs, int values) : m_inputs(inputs), m_values(values) {}

	int inputs() const { return m_inputs; }
	int values() const { return m_values; }

};

// class to load the XR calibration and calibrate again, requires Eigen
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LASAA_API UXRCalibration : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UXRCalibration();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="XRCalibration")
	void calibrate();

	UFUNCTION(BlueprintCallable, Category="XRCalibration")
	void resetCalibration();

	UFUNCTION(BlueprintCallable, Category="XRCalibration")
	void changeCalibration();
private:
	void loadFromFile();
	void writeToFile(int i = 0);
	
	bool calibrated = false;
	const FString filenameSpatial = "anchorCalibrate.txt";
	const FString filenameOld = "controllerCalibrate.txt";
	FString filename = filenameSpatial;
	
	struct Model : Functor<double>
	{
		MatrixXd& points0;
		MatrixXd& points1;

		Model(MatrixXd& points0, MatrixXd& points1) : Functor<double>(6, 6), points0{ points0 }, points1{ points1 } {}

		int operator()(const VectorXd& params, VectorXd& fvec) const
		{
			Vector3d rvec = params.head<3>();
			Vector3d tvec = params.tail<3>();

			Matrix3d R = UHelper::rodrigues(rvec);
			Matrix4d transformation = UHelper::extrinsicFromRt(R, tvec);

			MatrixXd points1Homogenous(points1.rows(), points1.cols() + 1);

			points1Homogenous << points1, MatrixXd::Ones(points1.rows(), 1);

			MatrixXd transformed = (transformation * points1Homogenous.transpose()).transpose();

			VectorXd distances(points0.rows(), 1);
			distances.setZero();
			for (int i = 0; i < points0.rows(); ++i)
			{
				distances(i) = (points0.row(i) - transformed.block<1, 3>(i, 0)).norm();
			}
			fvec.setZero();
			fvec(0) = distances.mean() * UHelper::variance(distances);
			return 0;
		}
	};
};
