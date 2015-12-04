#ifndef CHECKHEADER_SLIB_MEDIA_CAMERA
#define CHECKHEADER_SLIB_MEDIA_CAMERA

#include "definition.h"

#include "video_capture.h"

SLIB_MEDIA_NAMESPACE_BEGIN

class SLIB_EXPORT CameraInfo
{
public:
	String id;
	String name;
	String description;

public:
	CameraInfo();
};

class SLIB_EXPORT CameraParam : public VideoCaptureParam
{
public:
	String deviceId;

	sl_uint32 preferedFrameWidth;
	sl_uint32 preferedFrameHeight;

public:
	CameraParam();
	
public:
	void setFrontCamera();
	sl_bool isFrontCamera();
	
	void setBackCamera();
	sl_bool isBackCamera();
};

class SLIB_EXPORT Camera : public VideoCapture
{
	SLIB_DECLARE_OBJECT(Camera, VideoCapture)
protected:
	Camera();
public:
	~Camera();
	
public:
	static Ref<Camera> create(const CameraParam& param);
	static List<CameraInfo> getCamerasList();
	
};

SLIB_MEDIA_NAMESPACE_END

#endif