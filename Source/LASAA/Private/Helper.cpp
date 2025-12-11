// Copyright Expertise centre for Digital Media, 2024. All Rights Reserved.

#include "Helper.h"
#include "LASAAProjectSettings.h"

// UE import
#include "ImageUtils.h"
#include "OSCManager.h"
#include "OSCClient.h"
#include "OSCMessage.h"

// Third-party import
#include <Eigen/Core>
#include <sstream>
#include "SimpleCamera2Test.h"

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

TArray<float> UHelper::GetCameraCalibration()
{
    const ESupportedCameraHardware CameraHardware = GetDefault<ULASAAProjectSettings>()->CameraHardware;
    switch (CameraHardware)
    {
    case ESupportedCameraHardware::SCH_MetaQuest3:
        return GetQuest3CameraCalibration();
    case ESupportedCameraHardware::SCH_Unsupported:
        return TArray<float>();
    default:
        return TArray<float>();
    }
}

TArray<uint8> UHelper::GetCameraImageData()
{
    // Get camera texture
    UTexture2D* CameraTexture = nullptr;
    const ESupportedCameraHardware CameraHardware = GetDefault<ULASAAProjectSettings>()->CameraHardware;
    switch (CameraHardware)
    {
    case ESupportedCameraHardware::SCH_MetaQuest3:
        CameraTexture = USimpleCamera2Test::GetCameraTexture();
        break;
    case ESupportedCameraHardware::SCH_Unsupported:
        return TArray<uint8>();
    default:
        return TArray<uint8>();
    }

    if (CameraTexture)
    {
        // Get image
        FImage Image;
        if (FImageUtils::GetTexture2DSourceImage(CameraTexture, Image))
        {
            // Get data
            TArray64<uint8> ImageData;
            if (FImageUtils::CompressImage(ImageData, TEXT("png"), Image))
            {
                TArray<uint8> ImageDataUE(ImageData);
                return ImageDataUE;
            }
        }
    }

    return TArray<uint8>();
}

UOSCClient* UHelper::CreateProcessingDeviceOSCClient(UObject* Outer)
{
    const ULASAAProjectSettings* Settings = GetDefault<ULASAAProjectSettings>();
    const FString IpAddress = Settings->ImageProcessingDeviceIPAddress;
    const int32 Port = Settings->ImageProcessingDevicePort;

    UOSCClient* MyClient = UOSCManager::CreateOSCClient(IpAddress, Port,
        TEXT("ProcessingDeviceClient"), Outer);

    return MyClient;
}

void UHelper::SendCameraCalibrationToClient(UOSCClient* Client, const TArray<float>& CameraCalibration)
{
    if (Client && CameraCalibration.Num() == 10)
    {
        FOSCMessage Message;
        Message = UOSCManager::SetOSCMessageAddress(Message, 
            UOSCManager::ConvertStringToOSCAddress(TEXT("/calibration")));

        for (float Value : CameraCalibration)
        {
            UOSCManager::AddFloat(Message, Value);
        }

        Client->SendOSCMessage(Message);
    }
}

TArray<float> UHelper::GetAndSendCameraCalibrationToClient(UOSCClient* Client)
{
    const TArray<float> CameraCalibration = GetCameraCalibration();
    SendCameraCalibrationToClient(Client, CameraCalibration);
    return CameraCalibration;
}

void UHelper::SendCameraImageDataToClient(UOSCClient* Client, const TArray<uint8>& CameraImageData)
{
    if (Client && !CameraImageData.IsEmpty())
    {
        FOSCMessage Message;
        Message = UOSCManager::SetOSCMessageAddress(Message,
            UOSCManager::ConvertStringToOSCAddress(TEXT("/imageData")));

        UOSCManager::AddBlob(Message, CameraImageData);

        Client->SendOSCMessage(Message);
    }
}

TArray<uint8> UHelper::GetAndSendCameraImageDataToClient(UOSCClient* Client)
{
    const TArray<uint8> CameraImageData = GetCameraImageData();
    SendCameraImageDataToClient(Client, CameraImageData);
    return CameraImageData;
}

bool UHelper::GetAnchorTransformFromOSCMessage(FOSCMessage Message, FTransform& AnchorTransform)
{
    AnchorTransform = FTransform();
    
    TArray<float> Values;
    for (int i = 1; i < 17; i++)
    {
        float Value;
        if (!UOSCManager::GetFloat(Message, i, Value))
        {
            return false;
        }
        Values.Add(Value);
    }

    FMatrix ValuesMatrix(
        FPlane4d(Values[0], Values[1], Values[2], Values[3] * 100.),
        FPlane4d(Values[4], Values[5], Values[6], Values[7] * 100.),
        FPlane4d(Values[8], Values[9], Values[10], Values[11] * 100.),
        FPlane4d(Values[12], Values[13], Values[14], Values[15])
    );

    AnchorTransform = ConvertOpenCVToUnreal(FTransform(ValuesMatrix.GetTransposed()));

    return true;
}

void UHelper::SendRequestForAnchorPoseToClient(UOSCClient* Client)
{
    if (Client)
    {
        FOSCMessage Message;
        Message = UOSCManager::SetOSCMessageAddress(Message,
            UOSCManager::ConvertStringToOSCAddress(TEXT("/requestAnchorPose")));

        UOSCManager::AddString(Message, TEXT("requestAnchorPose"));

		Client->SendOSCMessage(Message);
    }
}

TArray<float> UHelper::GetQuest3CameraCalibration()
{
    // Get distortion
    TArray<float> Distortion = USimpleCamera2Test::GetLensDistortion();
    if (Distortion.Num() < 5)
    {
        return TArray<float>();
    }

    // Get intrinsics
    float Fx = USimpleCamera2Test::GetCameraFx();
    float Fy = USimpleCamera2Test::GetCameraFy();
    FVector2D PrincipalPoint = USimpleCamera2Test::GetPrincipalPoint();
    float Cx = PrincipalPoint.X;
    float Cy = PrincipalPoint.Y;
    float Skew = USimpleCamera2Test::GetCameraSkew();

    // Apply scale
    FIntPoint CalibResolution = USimpleCamera2Test::GetCalibrationResolution();
    FIntPoint OriginalResolution = USimpleCamera2Test::GetOriginalResolution();
    double ScaleX = static_cast<double>(CalibResolution.X) / OriginalResolution.X;
    double ScaleY = static_cast<double>(CalibResolution.Y) / OriginalResolution.Y;
    Fx *= ScaleX;
    Cx *= ScaleX;
    Fy *= ScaleY;
    Cy *= ScaleY;

    TArray<float> Calibration({ Fx, Fy, Cx, Cy, Skew });
    Calibration.Append(Distortion);
    return Calibration;
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
