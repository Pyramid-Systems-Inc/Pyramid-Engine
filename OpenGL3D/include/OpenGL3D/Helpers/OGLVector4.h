#pragma once
#include <OpenGL3D\OGLPrerequisites.h>

class OGLVec4
{
public:
	OGLVec4()
	{
	}
	OGLVec4(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w)
	{
	}
	~OGLVec4()
	{
	}

public:
	f32 x = 0, y = 0, z = 0, w = 0;

};
