#pragma once
#include <OpenGL3D\Helpers\OGLVector4.h>
/// <summary>
/// OpenGL Graphic Engine Main Header
/// </summary>
class OpenGLGraphicEngine
{
public:
	//OpenGL Graphic Engine Main Method
	OpenGLGraphicEngine();
	~OpenGLGraphicEngine();

public:
	//So this Method Is To Clear The Window Context With One Single Color
	void Clear(const OGLVec4& color);
	
};

