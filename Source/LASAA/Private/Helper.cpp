// Fill out your copyright notice in the Description page of Project Settings.

#include "Helper.h"
#include <Eigen/Core>
#include <sstream>

using namespace Eigen;

void UHelper::ConvertCoordinateSystem(FTransform& Transform, const EAxis SrcXInDstAxis, const EAxis SrcYInDstAxis, const EAxis SrcZInDstAxis)
{
    // Unreal Engine:
    //   Front : X
    //   Right : Y
    //   Up    : Z
    //
    // OpenCV:
    //   Front : Z
    //   Right : X
    //   Up    : Yn

    FMatrix M12 = FMatrix::Identity;

    M12.SetColumn(0, UnitVectorFromAxisEnum(SrcXInDstAxis));
    M12.SetColumn(1, UnitVectorFromAxisEnum(SrcYInDstAxis));
    M12.SetColumn(2, UnitVectorFromAxisEnum(SrcZInDstAxis));
	
    Transform.SetFromMatrix(M12.GetTransposed() * Transform.ToMatrixWithScale() * M12);
}

FTransform UHelper::ConvertUnrealToOpenCV(FTransform Transform)
{
    ConvertCoordinateSystem(Transform, EAxis::Y, EAxis::Zn, EAxis::X);
    return Transform;
}

FTransform UHelper::ConvertOpenCVToUnreal(FTransform Transform)
{
    ConvertCoordinateSystem(Transform, EAxis::Z, EAxis::X, EAxis::Yn);
    return Transform;
}

Matrix3d UHelper::rodrigues(const Vector3d& rvec) {
    double theta = rvec.norm();
    Vector3d axis = rvec.normalized();
    Matrix3d axis_cross;
    axis_cross << 0, -axis(2), axis(1),
                  axis(2), 0, -axis(0),
                  -axis(1), axis(0), 0;
    return Matrix3d::Identity() + std::sin(theta) * axis_cross + (1 - std::cos(theta)) * axis_cross * axis_cross;
}

double UHelper::variance(const VectorXd& vec)
{
    double mean = vec.mean();
    double var = 0.0;
    for (int i = 0; i < vec.size(); ++i) {
        var += (vec(i) - mean) * (vec(i) - mean);
    }
    return var / vec.size();
}

FVector UHelper::eigenVectorToUnreal(const Vector3d& vec)
{
    return FVector(vec.x(), vec.y() ,vec.z());
}

FMatrix UHelper::eigenMatrixToUnreal(const Matrix4d& mat)
{
    FMatrix uMat;
    uMat.M[0][0] = mat(0,0);
    uMat.M[0][1] = mat(0,1);
    uMat.M[0][2] = mat(0,2);
    uMat.M[0][3] = mat(0,3);
    uMat.M[1][0] = mat(1,0);
    uMat.M[1][1] = mat(1,1);
    uMat.M[1][2] = mat(1,2);
    uMat.M[1][3] = mat(1,3);
    uMat.M[2][0] = mat(2,0);
    uMat.M[2][1] = mat(2,1);
    uMat.M[2][2] = mat(2,2);
    uMat.M[2][3] = mat(2,3);
    uMat.M[3][0] = mat(3,0);
    uMat.M[3][1] = mat(3,1);
    uMat.M[3][2] = mat(3,2);
    uMat.M[3][3] = mat(3,3);
    return uMat.GetTransposed();
}

Vector3d UHelper::unrealVectorToEigen(const FVector& vec)
{
    return Vector3d(vec.X, vec.Y, vec.Z);
}


//FString UHelper::EigenToString(const MatrixXd mat)
//{
//    std::ostringstream oss;
//    oss << mat;
//    return FString(oss.str().c_str());
//}

Matrix4d UHelper::extrinsicFromRt(const Matrix3d& R, const Vector3d& tvec)
{
    Matrix4d extrinsic = Matrix4d::Identity();
    extrinsic.block<3, 3>(0, 0) = R;
    extrinsic.block<3, 1>(0, 3) = tvec;
    return extrinsic;
}
