#include <OpenGL3D\Graphic\OpenGLGraphicEngine.h>
#include <glad\glad.h>
#include <assert.h>
#include <stdexcept>



void OpenGLGraphicEngine::Clear(const OGLVec4& color)
{
	glClearColor(color.x, color.y, color.z, color.w);
	glClear(GL_COLOR_BUFFER_BIT);
}
