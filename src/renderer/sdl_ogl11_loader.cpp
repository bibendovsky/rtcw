/*

OpenGL v1.1 loader for SDL.

Copyright (c) 2018 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
OR OTHER DEALINGS IN THE SOFTWARE.

*/


//
// OpenGL v1.1 loader for SDL.
//


#include "sdl_ogl11_loader.h"
#include "SDL.h"
#include "sdl_ogl11_loader_gl.h"


namespace
{


//
// v1.0
//

PFNGLACCUMPROC glAccum_;
PFNGLALPHAFUNCPROC glAlphaFunc_;
PFNGLBEGINPROC glBegin_;
PFNGLBITMAPPROC glBitmap_;
PFNGLBLENDFUNCPROC glBlendFunc_;
PFNGLCALLLISTPROC glCallList_;
PFNGLCALLLISTSPROC glCallLists_;
PFNGLCLEARPROC glClear_;
PFNGLCLEARACCUMPROC glClearAccum_;
PFNGLCLEARCOLORPROC glClearColor_;
PFNGLCLEARDEPTHPROC glClearDepth_;
PFNGLCLEARINDEXPROC glClearIndex_;
PFNGLCLEARSTENCILPROC glClearStencil_;
PFNGLCLIPPLANEPROC glClipPlane_;
PFNGLCOLOR3BPROC glColor3b_;
PFNGLCOLOR3BVPROC glColor3bv_;
PFNGLCOLOR3DPROC glColor3d_;
PFNGLCOLOR3DVPROC glColor3dv_;
PFNGLCOLOR3FPROC glColor3f_;
PFNGLCOLOR3FVPROC glColor3fv_;
PFNGLCOLOR3IPROC glColor3i_;
PFNGLCOLOR3IVPROC glColor3iv_;
PFNGLCOLOR3SPROC glColor3s_;
PFNGLCOLOR3SVPROC glColor3sv_;
PFNGLCOLOR3UBPROC glColor3ub_;
PFNGLCOLOR3UBVPROC glColor3ubv_;
PFNGLCOLOR3UIPROC glColor3ui_;
PFNGLCOLOR3UIVPROC glColor3uiv_;
PFNGLCOLOR3USPROC glColor3us_;
PFNGLCOLOR3USVPROC glColor3usv_;
PFNGLCOLOR4BPROC glColor4b_;
PFNGLCOLOR4BVPROC glColor4bv_;
PFNGLCOLOR4DPROC glColor4d_;
PFNGLCOLOR4DVPROC glColor4dv_;
PFNGLCOLOR4FPROC glColor4f_;
PFNGLCOLOR4FVPROC glColor4fv_;
PFNGLCOLOR4IPROC glColor4i_;
PFNGLCOLOR4IVPROC glColor4iv_;
PFNGLCOLOR4SPROC glColor4s_;
PFNGLCOLOR4SVPROC glColor4sv_;
PFNGLCOLOR4UBPROC glColor4ub_;
PFNGLCOLOR4UBVPROC glColor4ubv_;
PFNGLCOLOR4UIPROC glColor4ui_;
PFNGLCOLOR4UIVPROC glColor4uiv_;
PFNGLCOLOR4USPROC glColor4us_;
PFNGLCOLOR4USVPROC glColor4usv_;
PFNGLCOLORMASKPROC glColorMask_;
PFNGLCOLORMATERIALPROC glColorMaterial_;
PFNGLCOPYPIXELSPROC glCopyPixels_;
PFNGLCULLFACEPROC glCullFace_;
PFNGLDELETELISTSPROC glDeleteLists_;
PFNGLDEPTHFUNCPROC glDepthFunc_;
PFNGLDEPTHMASKPROC glDepthMask_;
PFNGLDEPTHRANGEPROC glDepthRange_;
PFNGLDISABLEPROC glDisable_;
PFNGLDRAWBUFFERPROC glDrawBuffer_;
PFNGLDRAWPIXELSPROC glDrawPixels_;
PFNGLEDGEFLAGPROC glEdgeFlag_;
PFNGLEDGEFLAGVPROC glEdgeFlagv_;
PFNGLENABLEPROC glEnable_;
PFNGLENDPROC glEnd_;
PFNGLENDLISTPROC glEndList_;
PFNGLEVALCOORD1DPROC glEvalCoord1d_;
PFNGLEVALCOORD1DVPROC glEvalCoord1dv_;
PFNGLEVALCOORD1FPROC glEvalCoord1f_;
PFNGLEVALCOORD1FVPROC glEvalCoord1fv_;
PFNGLEVALCOORD2DPROC glEvalCoord2d_;
PFNGLEVALCOORD2DVPROC glEvalCoord2dv_;
PFNGLEVALCOORD2FPROC glEvalCoord2f_;
PFNGLEVALCOORD2FVPROC glEvalCoord2fv_;
PFNGLEVALMESH1PROC glEvalMesh1_;
PFNGLEVALMESH2PROC glEvalMesh2_;
PFNGLEVALPOINT1PROC glEvalPoint1_;
PFNGLEVALPOINT2PROC glEvalPoint2_;
PFNGLFEEDBACKBUFFERPROC glFeedbackBuffer_;
PFNGLFINISHPROC glFinish_;
PFNGLFLUSHPROC glFlush_;
PFNGLFOGFPROC glFogf_;
PFNGLFOGFVPROC glFogfv_;
PFNGLFOGIPROC glFogi_;
PFNGLFOGIVPROC glFogiv_;
PFNGLFRONTFACEPROC glFrontFace_;
PFNGLFRUSTUMPROC glFrustum_;
PFNGLGENLISTSPROC glGenLists_;
PFNGLGETBOOLEANVPROC glGetBooleanv_;
PFNGLGETCLIPPLANEPROC glGetClipPlane_;
PFNGLGETDOUBLEVPROC glGetDoublev_;
PFNGLGETERRORPROC glGetError_;
PFNGLGETFLOATVPROC glGetFloatv_;
PFNGLGETINTEGERVPROC glGetIntegerv_;
PFNGLGETLIGHTFVPROC glGetLightfv_;
PFNGLGETLIGHTIVPROC glGetLightiv_;
PFNGLGETMAPDVPROC glGetMapdv_;
PFNGLGETMAPFVPROC glGetMapfv_;
PFNGLGETMAPIVPROC glGetMapiv_;
PFNGLGETMATERIALFVPROC glGetMaterialfv_;
PFNGLGETMATERIALIVPROC glGetMaterialiv_;
PFNGLGETPIXELMAPFVPROC glGetPixelMapfv_;
PFNGLGETPIXELMAPUIVPROC glGetPixelMapuiv_;
PFNGLGETPIXELMAPUSVPROC glGetPixelMapusv_;
PFNGLGETPOLYGONSTIPPLEPROC glGetPolygonStipple_;
PFNGLGETSTRINGPROC glGetString_;
PFNGLGETTEXENVFVPROC glGetTexEnvfv_;
PFNGLGETTEXENVIVPROC glGetTexEnviv_;
PFNGLGETTEXGENDVPROC glGetTexGendv_;
PFNGLGETTEXGENFVPROC glGetTexGenfv_;
PFNGLGETTEXGENIVPROC glGetTexGeniv_;
PFNGLGETTEXIMAGEPROC glGetTexImage_;
PFNGLGETTEXLEVELPARAMETERFVPROC glGetTexLevelParameterfv_;
PFNGLGETTEXLEVELPARAMETERIVPROC glGetTexLevelParameteriv_;
PFNGLGETTEXPARAMETERFVPROC glGetTexParameterfv_;
PFNGLGETTEXPARAMETERIVPROC glGetTexParameteriv_;
PFNGLHINTPROC glHint_;
PFNGLINDEXMASKPROC glIndexMask_;
PFNGLINDEXDPROC glIndexd_;
PFNGLINDEXDVPROC glIndexdv_;
PFNGLINDEXFPROC glIndexf_;
PFNGLINDEXFVPROC glIndexfv_;
PFNGLINDEXIPROC glIndexi_;
PFNGLINDEXIVPROC glIndexiv_;
PFNGLINDEXSPROC glIndexs_;
PFNGLINDEXSVPROC glIndexsv_;
PFNGLINITNAMESPROC glInitNames_;
PFNGLISENABLEDPROC glIsEnabled_;
PFNGLISLISTPROC glIsList_;
PFNGLLIGHTMODELFPROC glLightModelf_;
PFNGLLIGHTMODELFVPROC glLightModelfv_;
PFNGLLIGHTMODELIPROC glLightModeli_;
PFNGLLIGHTMODELIVPROC glLightModeliv_;
PFNGLLIGHTFPROC glLightf_;
PFNGLLIGHTFVPROC glLightfv_;
PFNGLLIGHTIPROC glLighti_;
PFNGLLIGHTIVPROC glLightiv_;
PFNGLLINESTIPPLEPROC glLineStipple_;
PFNGLLINEWIDTHPROC glLineWidth_;
PFNGLLISTBASEPROC glListBase_;
PFNGLLOADIDENTITYPROC glLoadIdentity_;
PFNGLLOADMATRIXDPROC glLoadMatrixd_;
PFNGLLOADMATRIXFPROC glLoadMatrixf_;
PFNGLLOADNAMEPROC glLoadName_;
PFNGLLOGICOPPROC glLogicOp_;
PFNGLMAP1DPROC glMap1d_;
PFNGLMAP1FPROC glMap1f_;
PFNGLMAP2DPROC glMap2d_;
PFNGLMAP2FPROC glMap2f_;
PFNGLMAPGRID1DPROC glMapGrid1d_;
PFNGLMAPGRID1FPROC glMapGrid1f_;
PFNGLMAPGRID2DPROC glMapGrid2d_;
PFNGLMAPGRID2FPROC glMapGrid2f_;
PFNGLMATERIALFPROC glMaterialf_;
PFNGLMATERIALFVPROC glMaterialfv_;
PFNGLMATERIALIPROC glMateriali_;
PFNGLMATERIALIVPROC glMaterialiv_;
PFNGLMATRIXMODEPROC glMatrixMode_;
PFNGLMULTMATRIXDPROC glMultMatrixd_;
PFNGLMULTMATRIXFPROC glMultMatrixf_;
PFNGLNEWLISTPROC glNewList_;
PFNGLNORMAL3BPROC glNormal3b_;
PFNGLNORMAL3BVPROC glNormal3bv_;
PFNGLNORMAL3DPROC glNormal3d_;
PFNGLNORMAL3DVPROC glNormal3dv_;
PFNGLNORMAL3FPROC glNormal3f_;
PFNGLNORMAL3FVPROC glNormal3fv_;
PFNGLNORMAL3IPROC glNormal3i_;
PFNGLNORMAL3IVPROC glNormal3iv_;
PFNGLNORMAL3SPROC glNormal3s_;
PFNGLNORMAL3SVPROC glNormal3sv_;
PFNGLORTHOPROC glOrtho_;
PFNGLPASSTHROUGHPROC glPassThrough_;
PFNGLPIXELMAPFVPROC glPixelMapfv_;
PFNGLPIXELMAPUIVPROC glPixelMapuiv_;
PFNGLPIXELMAPUSVPROC glPixelMapusv_;
PFNGLPIXELSTOREFPROC glPixelStoref_;
PFNGLPIXELSTOREIPROC glPixelStorei_;
PFNGLPIXELTRANSFERFPROC glPixelTransferf_;
PFNGLPIXELTRANSFERIPROC glPixelTransferi_;
PFNGLPIXELZOOMPROC glPixelZoom_;
PFNGLPOINTSIZEPROC glPointSize_;
PFNGLPOLYGONMODEPROC glPolygonMode_;
PFNGLPOLYGONSTIPPLEPROC glPolygonStipple_;
PFNGLPOPATTRIBPROC glPopAttrib_;
PFNGLPOPMATRIXPROC glPopMatrix_;
PFNGLPOPNAMEPROC glPopName_;
PFNGLPUSHATTRIBPROC glPushAttrib_;
PFNGLPUSHMATRIXPROC glPushMatrix_;
PFNGLPUSHNAMEPROC glPushName_;
PFNGLRASTERPOS2DPROC glRasterPos2d_;
PFNGLRASTERPOS2DVPROC glRasterPos2dv_;
PFNGLRASTERPOS2FPROC glRasterPos2f_;
PFNGLRASTERPOS2FVPROC glRasterPos2fv_;
PFNGLRASTERPOS2IPROC glRasterPos2i_;
PFNGLRASTERPOS2IVPROC glRasterPos2iv_;
PFNGLRASTERPOS2SPROC glRasterPos2s_;
PFNGLRASTERPOS2SVPROC glRasterPos2sv_;
PFNGLRASTERPOS3DPROC glRasterPos3d_;
PFNGLRASTERPOS3DVPROC glRasterPos3dv_;
PFNGLRASTERPOS3FPROC glRasterPos3f_;
PFNGLRASTERPOS3FVPROC glRasterPos3fv_;
PFNGLRASTERPOS3IPROC glRasterPos3i_;
PFNGLRASTERPOS3IVPROC glRasterPos3iv_;
PFNGLRASTERPOS3SPROC glRasterPos3s_;
PFNGLRASTERPOS3SVPROC glRasterPos3sv_;
PFNGLRASTERPOS4DPROC glRasterPos4d_;
PFNGLRASTERPOS4DVPROC glRasterPos4dv_;
PFNGLRASTERPOS4FPROC glRasterPos4f_;
PFNGLRASTERPOS4FVPROC glRasterPos4fv_;
PFNGLRASTERPOS4IPROC glRasterPos4i_;
PFNGLRASTERPOS4IVPROC glRasterPos4iv_;
PFNGLRASTERPOS4SPROC glRasterPos4s_;
PFNGLRASTERPOS4SVPROC glRasterPos4sv_;
PFNGLREADBUFFERPROC glReadBuffer_;
PFNGLREADPIXELSPROC glReadPixels_;
PFNGLRECTDPROC glRectd_;
PFNGLRECTDVPROC glRectdv_;
PFNGLRECTFPROC glRectf_;
PFNGLRECTFVPROC glRectfv_;
PFNGLRECTIPROC glRecti_;
PFNGLRECTIVPROC glRectiv_;
PFNGLRECTSPROC glRects_;
PFNGLRECTSVPROC glRectsv_;
PFNGLRENDERMODEPROC glRenderMode_;
PFNGLROTATEDPROC glRotated_;
PFNGLROTATEFPROC glRotatef_;
PFNGLSCALEDPROC glScaled_;
PFNGLSCALEFPROC glScalef_;
PFNGLSCISSORPROC glScissor_;
PFNGLSELECTBUFFERPROC glSelectBuffer_;
PFNGLSHADEMODELPROC glShadeModel_;
PFNGLSTENCILFUNCPROC glStencilFunc_;
PFNGLSTENCILMASKPROC glStencilMask_;
PFNGLSTENCILOPPROC glStencilOp_;
PFNGLTEXCOORD1DPROC glTexCoord1d_;
PFNGLTEXCOORD1DVPROC glTexCoord1dv_;
PFNGLTEXCOORD1FPROC glTexCoord1f_;
PFNGLTEXCOORD1FVPROC glTexCoord1fv_;
PFNGLTEXCOORD1IPROC glTexCoord1i_;
PFNGLTEXCOORD1IVPROC glTexCoord1iv_;
PFNGLTEXCOORD1SPROC glTexCoord1s_;
PFNGLTEXCOORD1SVPROC glTexCoord1sv_;
PFNGLTEXCOORD2DPROC glTexCoord2d_;
PFNGLTEXCOORD2DVPROC glTexCoord2dv_;
PFNGLTEXCOORD2FPROC glTexCoord2f_;
PFNGLTEXCOORD2FVPROC glTexCoord2fv_;
PFNGLTEXCOORD2IPROC glTexCoord2i_;
PFNGLTEXCOORD2IVPROC glTexCoord2iv_;
PFNGLTEXCOORD2SPROC glTexCoord2s_;
PFNGLTEXCOORD2SVPROC glTexCoord2sv_;
PFNGLTEXCOORD3DPROC glTexCoord3d_;
PFNGLTEXCOORD3DVPROC glTexCoord3dv_;
PFNGLTEXCOORD3FPROC glTexCoord3f_;
PFNGLTEXCOORD3FVPROC glTexCoord3fv_;
PFNGLTEXCOORD3IPROC glTexCoord3i_;
PFNGLTEXCOORD3IVPROC glTexCoord3iv_;
PFNGLTEXCOORD3SPROC glTexCoord3s_;
PFNGLTEXCOORD3SVPROC glTexCoord3sv_;
PFNGLTEXCOORD4DPROC glTexCoord4d_;
PFNGLTEXCOORD4DVPROC glTexCoord4dv_;
PFNGLTEXCOORD4FPROC glTexCoord4f_;
PFNGLTEXCOORD4FVPROC glTexCoord4fv_;
PFNGLTEXCOORD4IPROC glTexCoord4i_;
PFNGLTEXCOORD4IVPROC glTexCoord4iv_;
PFNGLTEXCOORD4SPROC glTexCoord4s_;
PFNGLTEXCOORD4SVPROC glTexCoord4sv_;
PFNGLTEXENVFPROC glTexEnvf_;
PFNGLTEXENVFVPROC glTexEnvfv_;
PFNGLTEXENVIPROC glTexEnvi_;
PFNGLTEXENVIVPROC glTexEnviv_;
PFNGLTEXGENDPROC glTexGend_;
PFNGLTEXGENDVPROC glTexGendv_;
PFNGLTEXGENFPROC glTexGenf_;
PFNGLTEXGENFVPROC glTexGenfv_;
PFNGLTEXGENIPROC glTexGeni_;
PFNGLTEXGENIVPROC glTexGeniv_;
PFNGLTEXIMAGE1DPROC glTexImage1D_;
PFNGLTEXIMAGE2DPROC glTexImage2D_;
PFNGLTEXPARAMETERFPROC glTexParameterf_;
PFNGLTEXPARAMETERFVPROC glTexParameterfv_;
PFNGLTEXPARAMETERIPROC glTexParameteri_;
PFNGLTEXPARAMETERIVPROC glTexParameteriv_;
PFNGLTRANSLATEDPROC glTranslated_;
PFNGLTRANSLATEFPROC glTranslatef_;
PFNGLVERTEX2DPROC glVertex2d_;
PFNGLVERTEX2DVPROC glVertex2dv_;
PFNGLVERTEX2FPROC glVertex2f_;
PFNGLVERTEX2FVPROC glVertex2fv_;
PFNGLVERTEX2IPROC glVertex2i_;
PFNGLVERTEX2IVPROC glVertex2iv_;
PFNGLVERTEX2SPROC glVertex2s_;
PFNGLVERTEX2SVPROC glVertex2sv_;
PFNGLVERTEX3DPROC glVertex3d_;
PFNGLVERTEX3DVPROC glVertex3dv_;
PFNGLVERTEX3FPROC glVertex3f_;
PFNGLVERTEX3FVPROC glVertex3fv_;
PFNGLVERTEX3IPROC glVertex3i_;
PFNGLVERTEX3IVPROC glVertex3iv_;
PFNGLVERTEX3SPROC glVertex3s_;
PFNGLVERTEX3SVPROC glVertex3sv_;
PFNGLVERTEX4DPROC glVertex4d_;
PFNGLVERTEX4DVPROC glVertex4dv_;
PFNGLVERTEX4FPROC glVertex4f_;
PFNGLVERTEX4FVPROC glVertex4fv_;
PFNGLVERTEX4IPROC glVertex4i_;
PFNGLVERTEX4IVPROC glVertex4iv_;
PFNGLVERTEX4SPROC glVertex4s_;
PFNGLVERTEX4SVPROC glVertex4sv_;
PFNGLVIEWPORTPROC glViewport_;


//
// v1.1
//

PFNGLARETEXTURESRESIDENTPROC glAreTexturesResident_;
PFNGLARRAYELEMENTPROC glArrayElement_;
PFNGLBINDTEXTUREPROC glBindTexture_;
PFNGLCOLORPOINTERPROC glColorPointer_;
PFNGLCOPYTEXIMAGE1DPROC glCopyTexImage1D_;
PFNGLCOPYTEXIMAGE2DPROC glCopyTexImage2D_;
PFNGLCOPYTEXSUBIMAGE1DPROC glCopyTexSubImage1D_;
PFNGLCOPYTEXSUBIMAGE2DPROC glCopyTexSubImage2D_;
PFNGLDELETETEXTURESPROC glDeleteTextures_;
PFNGLDISABLECLIENTSTATEPROC glDisableClientState_;
PFNGLDRAWARRAYSPROC glDrawArrays_;
PFNGLDRAWELEMENTSPROC glDrawElements_;
PFNGLEDGEFLAGPOINTERPROC glEdgeFlagPointer_;
PFNGLENABLECLIENTSTATEPROC glEnableClientState_;
PFNGLGENTEXTURESPROC glGenTextures_;
PFNGLGETPOINTERVPROC glGetPointerv_;
PFNGLINDEXPOINTERPROC glIndexPointer_;
PFNGLINDEXUBPROC glIndexub_;
PFNGLINDEXUBVPROC glIndexubv_;
PFNGLINTERLEAVEDARRAYSPROC glInterleavedArrays_;
PFNGLISTEXTUREPROC glIsTexture_;
PFNGLNORMALPOINTERPROC glNormalPointer_;
PFNGLPOLYGONOFFSETPROC glPolygonOffset_;
PFNGLPOPCLIENTATTRIBPROC glPopClientAttrib_;
PFNGLPRIORITIZETEXTURESPROC glPrioritizeTextures_;
PFNGLPUSHCLIENTATTRIBPROC glPushClientAttrib_;
PFNGLTEXCOORDPOINTERPROC glTexCoordPointer_;
PFNGLTEXSUBIMAGE1DPROC glTexSubImage1D_;
PFNGLTEXSUBIMAGE2DPROC glTexSubImage2D_;
PFNGLVERTEXPOINTERPROC glVertexPointer_;


} // namespace


//
// v1.0
//

void APIENTRY glAccum(GLenum op, GLfloat value)
{
	glAccum_(op, value);
}

void APIENTRY glAlphaFunc(GLenum func, GLfloat ref)
{
	glAlphaFunc_(func, ref);
}

void APIENTRY glBegin(GLenum mode)
{
	glBegin_(mode);
}

void APIENTRY glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte* bitmap)
{
	glBitmap_(width, height, xorig, yorig, xmove, ymove, bitmap);
}

void APIENTRY glBlendFunc(GLenum sfactor, GLenum dfactor)
{
	glBlendFunc_(sfactor, dfactor);
}

void APIENTRY glCallList(GLuint list)
{
	glCallList_(list);
}

void APIENTRY glCallLists(GLsizei n, GLenum type, const void* lists)
{
	glCallLists_(n, type, lists);
}

void APIENTRY glClear(GLbitfield mask)
{
	glClear_(mask);
}

void APIENTRY glClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	glClearAccum_(red, green, blue, alpha);
}

void APIENTRY glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	glClearColor_(red, green, blue, alpha);
}

void APIENTRY glClearDepth(GLdouble depth)
{
	glClearDepth_(depth);
}

void APIENTRY glClearIndex(GLfloat c)
{
	glClearIndex_(c);
}

void APIENTRY glClearStencil(GLint s)
{
	glClearStencil_(s);
}

void APIENTRY glClipPlane(GLenum plane, const GLdouble* equation)
{
	glClipPlane_(plane, equation);
}

void APIENTRY glColor3b(GLbyte red, GLbyte green, GLbyte blue)
{
	glColor3b_(red, green, blue);
}

void APIENTRY glColor3bv(const GLbyte* v)
{
	glColor3bv_(v);
}

void APIENTRY glColor3d(GLdouble red, GLdouble green, GLdouble blue)
{
	glColor3d_(red, green, blue);
}

void APIENTRY glColor3dv(const GLdouble* v)
{
	glColor3dv_(v);
}

void APIENTRY glColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
	glColor3f_(red, green, blue);
}

void APIENTRY glColor3fv(const GLfloat* v)
{
	glColor3fv_(v);
}

void APIENTRY glColor3i(GLint red, GLint green, GLint blue)
{
	glColor3i_(red, green, blue);
}

void APIENTRY glColor3iv(const GLint* v)
{
	glColor3iv_(v);
}

void APIENTRY glColor3s(GLshort red, GLshort green, GLshort blue)
{
	glColor3s_(red, green, blue);
}

void APIENTRY glColor3sv(const GLshort* v)
{
	glColor3sv_(v);
}

void APIENTRY glColor3ub(GLubyte red, GLubyte green, GLubyte blue)
{
	glColor3ub_(red, green, blue);
}

void APIENTRY glColor3ubv(const GLubyte* v)
{
	glColor3ubv_(v);
}

void APIENTRY glColor3ui(GLuint red, GLuint green, GLuint blue)
{
	glColor3ui_(red, green, blue);
}

void APIENTRY glColor3uiv(const GLuint* v)
{
	glColor3uiv_(v);
}

void APIENTRY glColor3us(GLushort red, GLushort green, GLushort blue)
{
	glColor3us_(red, green, blue);
}

void APIENTRY glColor3usv(const GLushort* v)
{
	glColor3usv_(v);
}

void APIENTRY glColor4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha)
{
	glColor4b_(red, green, blue, alpha);
}

void APIENTRY glColor4bv(const GLbyte* v)
{
	glColor4bv_(v);
}

void APIENTRY glColor4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha)
{
	glColor4d_(red, green, blue, alpha);
}

void APIENTRY glColor4dv(const GLdouble* v)
{
	glColor4dv_(v);
}

void APIENTRY glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	glColor4f_(red, green, blue, alpha);
}

void APIENTRY glColor4fv(const GLfloat* v)
{
	glColor4fv_(v);
}

void APIENTRY glColor4i(GLint red, GLint green, GLint blue, GLint alpha)
{
	glColor4i_(red, green, blue, alpha);
}

void APIENTRY glColor4iv(const GLint* v)
{
	glColor4iv_(v);
}

void APIENTRY glColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha)
{
	glColor4s_(red, green, blue, alpha);
}

void APIENTRY glColor4sv(const GLshort* v)
{
	glColor4sv_(v);
}

void APIENTRY glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
	glColor4ub_(red, green, blue, alpha);
}

void APIENTRY glColor4ubv(const GLubyte* v)
{
	glColor4ubv_(v);
}

void APIENTRY glColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha)
{
	glColor4ui_(red, green, blue, alpha);
}

void APIENTRY glColor4uiv(const GLuint* v)
{
	glColor4uiv_(v);
}

void APIENTRY glColor4us(GLushort red, GLushort green, GLushort blue, GLushort alpha)
{
	glColor4us_(red, green, blue, alpha);
}

void APIENTRY glColor4usv(const GLushort* v)
{
	glColor4usv_(v);
}

void APIENTRY glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
	glColorMask_(red, green, blue, alpha);
}

void APIENTRY glColorMaterial(GLenum face, GLenum mode)
{
	glColorMaterial_(face, mode);
}

void APIENTRY glCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type)
{
	glCopyPixels_(x, y, width, height, type);
}

void APIENTRY glCullFace(GLenum mode)
{
	glCullFace_(mode);
}

void APIENTRY glDeleteLists(GLuint list, GLsizei range)
{
	glDeleteLists_(list, range);
}

void APIENTRY glDepthFunc(GLenum func)
{
	glDepthFunc_(func);
}

void APIENTRY glDepthMask(GLboolean flag)
{
	glDepthMask_(flag);
}

void APIENTRY glDepthRange(GLdouble n, GLdouble f)
{
	glDepthRange_(n, f);
}

void APIENTRY glDisable(GLenum cap)
{
	glDisable_(cap);
}

void APIENTRY glDrawBuffer(GLenum buf)
{
	glDrawBuffer_(buf);
}

void APIENTRY glDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
{
	glDrawPixels_(width, height, format, type, pixels);
}

void APIENTRY glEdgeFlag(GLboolean flag)
{
	glEdgeFlag_(flag);
}

void APIENTRY glEdgeFlagv(const GLboolean* flag)
{
	glEdgeFlagv_(flag);
}

void APIENTRY glEnable(GLenum cap)
{
	glEnable_(cap);
}

void APIENTRY glEnd()
{
	glEnd_();
}

void APIENTRY glEndList()
{
	glEndList_();
}

void APIENTRY glEvalCoord1d(GLdouble u)
{
	glEvalCoord1d_(u);
}

void APIENTRY glEvalCoord1dv(const GLdouble* u)
{
	glEvalCoord1dv_(u);
}

void APIENTRY glEvalCoord1f(GLfloat u)
{
	glEvalCoord1f_(u);
}

void APIENTRY glEvalCoord1fv(const GLfloat* u)
{
	glEvalCoord1fv_(u);
}

void APIENTRY glEvalCoord2d(GLdouble u, GLdouble v)
{
	glEvalCoord2d_(u, v);
}

void APIENTRY glEvalCoord2dv(const GLdouble* u)
{
	glEvalCoord2dv_(u);
}

void APIENTRY glEvalCoord2f(GLfloat u, GLfloat v)
{
	glEvalCoord2f_(u, v);
}

void APIENTRY glEvalCoord2fv(const GLfloat* u)
{
	glEvalCoord2fv_(u);
}

void APIENTRY glEvalMesh1(GLenum mode, GLint i1, GLint i2)
{
	glEvalMesh1_(mode, i1, i2);
}

void APIENTRY glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2)
{
	glEvalMesh2_(mode, i1, i2, j1, j2);
}

void APIENTRY glEvalPoint1(GLint i)
{
	glEvalPoint1_(i);
}

void APIENTRY glEvalPoint2(GLint i, GLint j)
{
	glEvalPoint2_(i, j);
}

void APIENTRY glFeedbackBuffer(GLsizei size, GLenum type, GLfloat* buffer)
{
	glFeedbackBuffer_(size, type, buffer);
}

void APIENTRY glFinish()
{
	glFinish_();
}

void APIENTRY glFlush()
{
	glFlush_();
}

void APIENTRY glFogf(GLenum pname, GLfloat param)
{
	glFogf_(pname, param);
}

void APIENTRY glFogfv(GLenum pname, const GLfloat* params)
{
	glFogfv_(pname, params);
}

void APIENTRY glFogi(GLenum pname, GLint param)
{
	glFogi_(pname, param);
}

void APIENTRY glFogiv(GLenum pname, const GLint* params)
{
	glFogiv_(pname, params);
}

void APIENTRY glFrontFace(GLenum mode)
{
	glFrontFace_(mode);
}

void APIENTRY glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
	glFrustum_(left, right, bottom, top, zNear, zFar);
}

GLuint APIENTRY glGenLists(GLsizei range)
{
	return glGenLists_(range);
}

void APIENTRY glGetBooleanv(GLenum pname, GLboolean* data)
{
	glGetBooleanv_(pname, data);
}

void APIENTRY glGetClipPlane(GLenum plane, GLdouble* equation)
{
	glGetClipPlane_(plane, equation);
}

void APIENTRY glGetDoublev(GLenum pname, GLdouble* data)
{
	glGetDoublev_(pname, data);
}

GLenum APIENTRY glGetError()
{
	return glGetError_();
}

void APIENTRY glGetFloatv(GLenum pname, GLfloat* data)
{
	glGetFloatv_(pname, data);
}

void APIENTRY glGetIntegerv(GLenum pname, GLint* data)
{
	glGetIntegerv_(pname, data);
}

void APIENTRY glGetLightfv(GLenum light, GLenum pname, GLfloat* params)
{
	glGetLightfv_(light, pname, params);
}

void APIENTRY glGetLightiv(GLenum light, GLenum pname, GLint* params)
{
	glGetLightiv_(light, pname, params);
}

void APIENTRY glGetMapdv(GLenum target, GLenum query, GLdouble* v)
{
	glGetMapdv_(target, query, v);
}

void APIENTRY glGetMapfv(GLenum target, GLenum query, GLfloat* v)
{
	glGetMapfv_(target, query, v);
}

void APIENTRY glGetMapiv(GLenum target, GLenum query, GLint* v)
{
	glGetMapiv_(target, query, v);
}

void APIENTRY glGetMaterialfv(GLenum face, GLenum pname, GLfloat* params)
{
	glGetMaterialfv_(face, pname, params);
}

void APIENTRY glGetMaterialiv(GLenum face, GLenum pname, GLint* params)
{
	glGetMaterialiv_(face, pname, params);
}

void APIENTRY glGetPixelMapfv(GLenum map, GLfloat* values)
{
	glGetPixelMapfv_(map, values);
}

void APIENTRY glGetPixelMapuiv(GLenum map, GLuint* values)
{
	glGetPixelMapuiv_(map, values);
}

void APIENTRY glGetPixelMapusv(GLenum map, GLushort* values)
{
	glGetPixelMapusv_(map, values);
}

void APIENTRY glGetPolygonStipple(GLubyte* mask)
{
	glGetPolygonStipple_(mask);
}

const GLubyte* glGetString(GLenum name)
{
	return glGetString_(name);
}

void APIENTRY glGetTexEnvfv(GLenum target, GLenum pname, GLfloat* params)
{
	glGetTexEnvfv_(target, pname, params);
}

void APIENTRY glGetTexEnviv(GLenum target, GLenum pname, GLint* params)
{
	glGetTexEnviv_(target, pname, params);
}

void APIENTRY glGetTexGendv(GLenum coord, GLenum pname, GLdouble* params)
{
	glGetTexGendv_(coord, pname, params);
}

void APIENTRY glGetTexGenfv(GLenum coord, GLenum pname, GLfloat* params)
{
	glGetTexGenfv_(coord, pname, params);
}

void APIENTRY glGetTexGeniv(GLenum coord, GLenum pname, GLint* params)
{
	glGetTexGeniv_(coord, pname, params);
}

void APIENTRY glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, void* pixels)
{
	glGetTexImage_(target, level, format, type, pixels);
}

void APIENTRY glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat* params)
{
	glGetTexLevelParameterfv_(target, level, pname, params);
}

void APIENTRY glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint* params)
{
	glGetTexLevelParameteriv_(target, level, pname, params);
}

void APIENTRY glGetTexParameterfv(GLenum target, GLenum pname, GLfloat* params)
{
	glGetTexParameterfv_(target, pname, params);
}

void APIENTRY glGetTexParameteriv(GLenum target, GLenum pname, GLint* params)
{
	glGetTexParameteriv_(target, pname, params);
}

void APIENTRY glHint(GLenum target, GLenum mode)
{
	glHint_(target, mode);
}

void APIENTRY glIndexMask(GLuint mask)
{
	glIndexMask_(mask);
}

void APIENTRY glIndexd(GLdouble c)
{
	glIndexd_(c);
}

void APIENTRY glIndexdv(const GLdouble* c)
{
	glIndexdv_(c);
}

void APIENTRY glIndexf(GLfloat c)
{
	glIndexf_(c);
}

void APIENTRY glIndexfv(const GLfloat* c)
{
	glIndexfv_(c);
}

void APIENTRY glIndexi(GLint c)
{
	glIndexi_(c);
}

void APIENTRY glIndexiv(const GLint* c)
{
	glIndexiv_(c);
}

void APIENTRY glIndexs(GLshort c)
{
	glIndexs_(c);
}

void APIENTRY glIndexsv(const GLshort* c)
{
	glIndexsv_(c);
}

void APIENTRY glInitNames()
{
	glInitNames_();
}

GLboolean APIENTRY glIsEnabled(GLenum cap)
{
	return glIsEnabled_(cap);
}

GLboolean APIENTRY glIsList(GLuint list)
{
	return glIsList_(list);
}

void APIENTRY glLightModelf(GLenum pname, GLfloat param)
{
	glLightModelf_(pname, param);
}

void APIENTRY glLightModelfv(GLenum pname, const GLfloat* params)
{
	glLightModelfv_(pname, params);
}

void APIENTRY glLightModeli(GLenum pname, GLint param)
{
	glLightModeli_(pname, param);
}

void APIENTRY glLightModeliv(GLenum pname, const GLint* params)
{
	glLightModeliv_(pname, params);
}

void APIENTRY glLightf(GLenum light, GLenum pname, GLfloat param)
{
	glLightf_(light, pname, param);
}

void APIENTRY glLightfv(GLenum light, GLenum pname, const GLfloat* params)
{
	glLightfv_(light, pname, params);
}

void APIENTRY glLighti(GLenum light, GLenum pname, GLint param)
{
	glLighti_(light, pname, param);
}

void APIENTRY glLightiv(GLenum light, GLenum pname, const GLint* params)
{
	glLightiv_(light, pname, params);
}

void APIENTRY glLineStipple(GLint factor, GLushort pattern)
{
	glLineStipple_(factor, pattern);
}

void APIENTRY glLineWidth(GLfloat width)
{
	glLineWidth_(width);
}

void APIENTRY glListBase(GLuint base)
{
	glListBase_(base);
}

void APIENTRY glLoadIdentity()
{
	glLoadIdentity_();
}

void APIENTRY glLoadMatrixd(const GLdouble* m)
{
	glLoadMatrixd_(m);
}

void APIENTRY glLoadMatrixf(const GLfloat* m)
{
	glLoadMatrixf_(m);
}

void APIENTRY glLoadName(GLuint name)
{
	glLoadName_(name);
}

void APIENTRY glLogicOp(GLenum opcode)
{
	glLogicOp_(opcode);
}

void APIENTRY glMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble* points)
{
	glMap1d_(target, u1, u2, stride, order, points);
}

void APIENTRY glMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat* points)
{
	glMap1f_(target, u1, u2, stride, order, points);
}

void APIENTRY glMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble* points)
{
	glMap2d_(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
}

void APIENTRY glMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat* points)
{
	glMap2f_(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
}

void APIENTRY glMapGrid1d(GLint un, GLdouble u1, GLdouble u2)
{
	glMapGrid1d_(un, u1, u2);
}

void APIENTRY glMapGrid1f(GLint un, GLfloat u1, GLfloat u2)
{
	glMapGrid1f_(un, u1, u2);
}

void APIENTRY glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2)
{
	glMapGrid2d_(un, u1, u2, vn, v1, v2);
}

void APIENTRY glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2)
{
	glMapGrid2f_(un, u1, u2, vn, v1, v2);
}

void APIENTRY glMaterialf(GLenum face, GLenum pname, GLfloat param)
{
	glMaterialf_(face, pname, param);
}

void APIENTRY glMaterialfv(GLenum face, GLenum pname, const GLfloat* params)
{
	glMaterialfv_(face, pname, params);
}

void APIENTRY glMateriali(GLenum face, GLenum pname, GLint param)
{
	glMateriali_(face, pname, param);
}

void APIENTRY glMaterialiv(GLenum face, GLenum pname, const GLint* params)
{
	glMaterialiv_(face, pname, params);
}

void APIENTRY glMatrixMode(GLenum mode)
{
	glMatrixMode_(mode);
}

void APIENTRY glMultMatrixd(const GLdouble* m)
{
	glMultMatrixd_(m);
}

void APIENTRY glMultMatrixf(const GLfloat* m)
{
	glMultMatrixf_(m);
}

void APIENTRY glNewList(GLuint list, GLenum mode)
{
	glNewList_(list, mode);
}

void APIENTRY glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz)
{
	glNormal3b_(nx, ny, nz);
}

void APIENTRY glNormal3bv(const GLbyte* v)
{
	glNormal3bv_(v);
}

void APIENTRY glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz)
{
	glNormal3d_(nx, ny, nz);
}

void APIENTRY glNormal3dv(const GLdouble* v)
{
	glNormal3dv_(v);
}

void APIENTRY glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
	glNormal3f_(nx, ny, nz);
}

void APIENTRY glNormal3fv(const GLfloat* v)
{
	glNormal3fv_(v);
}

void APIENTRY glNormal3i(GLint nx, GLint ny, GLint nz)
{
	glNormal3i_(nx, ny, nz);
}

void APIENTRY glNormal3iv(const GLint* v)
{
	glNormal3iv_(v);
}

void APIENTRY glNormal3s(GLshort nx, GLshort ny, GLshort nz)
{
	glNormal3s_(nx, ny, nz);
}

void APIENTRY glNormal3sv(const GLshort* v)
{
	glNormal3sv_(v);
}

void APIENTRY glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
	glOrtho_(left, right, bottom, top, zNear, zFar);
}

void APIENTRY glPassThrough(GLfloat token)
{
	glPassThrough_(token);
}

void APIENTRY glPixelMapfv(GLenum map, GLsizei mapsize, const GLfloat* values)
{
	glPixelMapfv_(map, mapsize, values);
}

void APIENTRY glPixelMapuiv(GLenum map, GLsizei mapsize, const GLuint* values)
{
	glPixelMapuiv_(map, mapsize, values);
}

void APIENTRY glPixelMapusv(GLenum map, GLsizei mapsize, const GLushort* values)
{
	glPixelMapusv_(map, mapsize, values);
}

void APIENTRY glPixelStoref(GLenum pname, GLfloat param)
{
	glPixelStoref_(pname, param);
}

void APIENTRY glPixelStorei(GLenum pname, GLint param)
{
	glPixelStorei_(pname, param);
}

void APIENTRY glPixelTransferf(GLenum pname, GLfloat param)
{
	glPixelTransferf_(pname, param);
}

void APIENTRY glPixelTransferi(GLenum pname, GLint param)
{
	glPixelTransferi_(pname, param);
}

void APIENTRY glPixelZoom(GLfloat xfactor, GLfloat yfactor)
{
	glPixelZoom_(xfactor, yfactor);
}

void APIENTRY glPointSize(GLfloat size)
{
	glPointSize_(size);
}

void APIENTRY glPolygonMode(GLenum face, GLenum mode)
{
	glPolygonMode_(face, mode);
}

void APIENTRY glPolygonStipple(const GLubyte* mask)
{
	glPolygonStipple_(mask);
}

void APIENTRY glPopAttrib()
{
	glPopAttrib_();
}

void APIENTRY glPopMatrix()
{
	glPopMatrix_();
}

void APIENTRY glPopName()
{
	glPopName_();
}

void APIENTRY glPushAttrib(GLbitfield mask)
{
	glPushAttrib_(mask);
}

void APIENTRY glPushMatrix()
{
	glPushMatrix_();
}

void APIENTRY glPushName(GLuint name)
{
	glPushName_(name);
}

void APIENTRY glRasterPos2d(GLdouble x, GLdouble y)
{
	glRasterPos2d_(x, y);
}

void APIENTRY glRasterPos2dv(const GLdouble* v)
{
	glRasterPos2dv_(v);
}

void APIENTRY glRasterPos2f(GLfloat x, GLfloat y)
{
	glRasterPos2f_(x, y);
}

void APIENTRY glRasterPos2fv(const GLfloat* v)
{
	glRasterPos2fv_(v);
}

void APIENTRY glRasterPos2i(GLint x, GLint y)
{
	glRasterPos2i_(x, y);
}

void APIENTRY glRasterPos2iv(const GLint* v)
{
	glRasterPos2iv_(v);
}

void APIENTRY glRasterPos2s(GLshort x, GLshort y)
{
	glRasterPos2s_(x, y);
}

void APIENTRY glRasterPos2sv(const GLshort* v)
{
	glRasterPos2sv_(v);
}

void APIENTRY glRasterPos3d(GLdouble x, GLdouble y, GLdouble z)
{
	glRasterPos3d_(x, y, z);
}

void APIENTRY glRasterPos3dv(const GLdouble* v)
{
	glRasterPos3dv_(v);
}

void APIENTRY glRasterPos3f(GLfloat x, GLfloat y, GLfloat z)
{
	glRasterPos3f_(x, y, z);
}

void APIENTRY glRasterPos3fv(const GLfloat* v)
{
	glRasterPos3fv_(v);
}

void APIENTRY glRasterPos3i(GLint x, GLint y, GLint z)
{
	glRasterPos3i_(x, y, z);
}

void APIENTRY glRasterPos3iv(const GLint* v)
{
	glRasterPos3iv_(v);
}

void APIENTRY glRasterPos3s(GLshort x, GLshort y, GLshort z)
{
	glRasterPos3s_(x, y, z);
}

void APIENTRY glRasterPos3sv(const GLshort* v)
{
	glRasterPos3sv_(v);
}

void APIENTRY glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	glRasterPos4d_(x, y, z, w);
}

void APIENTRY glRasterPos4dv(const GLdouble* v)
{
	glRasterPos4dv_(v);
}

void APIENTRY glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	glRasterPos4f_(x, y, z, w);
}

void APIENTRY glRasterPos4fv(const GLfloat* v)
{
	glRasterPos4fv_(v);
}

void APIENTRY glRasterPos4i(GLint x, GLint y, GLint z, GLint w)
{
	glRasterPos4i_(x, y, z, w);
}

void APIENTRY glRasterPos4iv(const GLint* v)
{
	glRasterPos4iv_(v);
}

void APIENTRY glRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
	glRasterPos4s_(x, y, z, w);
}

void APIENTRY glRasterPos4sv(const GLshort* v)
{
	glRasterPos4sv_(v);
}

void APIENTRY glReadBuffer(GLenum src)
{
	glReadBuffer_(src);
}

void APIENTRY glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels)
{
	glReadPixels_(x, y, width, height, format, type, pixels);
}

void APIENTRY glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
	glRectd_(x1, y1, x2, y2);
}

void APIENTRY glRectdv(const GLdouble* v1, const GLdouble* v2)
{
	glRectdv_(v1, v2);
}

void APIENTRY glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
	glRectf_(x1, y1, x2, y2);
}

void APIENTRY glRectfv(const GLfloat* v1, const GLfloat* v2)
{
	glRectfv_(v1, v2);
}

void APIENTRY glRecti(GLint x1, GLint y1, GLint x2, GLint y2)
{
	glRecti_(x1, y1, x2, y2);
}

void APIENTRY glRectiv(const GLint* v1, const GLint* v2)
{
	glRectiv_(v1, v2);
}

void APIENTRY glRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2)
{
	glRects_(x1, y1, x2, y2);
}

void APIENTRY glRectsv(const GLshort* v1, const GLshort* v2)
{
	glRectsv_(v1, v2);
}

GLint APIENTRY glRenderMode(GLenum mode)
{
	return glRenderMode_(mode);
}

void APIENTRY glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
	glRotated_(angle, x, y, z);
}

void APIENTRY glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	glRotatef_(angle, x, y, z);
}

void APIENTRY glScaled(GLdouble x, GLdouble y, GLdouble z)
{
	glScaled_(x, y, z);
}

void APIENTRY glScalef(GLfloat x, GLfloat y, GLfloat z)
{
	glScalef_(x, y, z);
}

void APIENTRY glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
	glScissor_(x, y, width, height);
}

void APIENTRY glSelectBuffer(GLsizei size, GLuint* buffer)
{
	glSelectBuffer_(size, buffer);
}

void APIENTRY glShadeModel(GLenum mode)
{
	glShadeModel_(mode);
}

void APIENTRY glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
	glStencilFunc_(func, ref, mask);
}

void APIENTRY glStencilMask(GLuint mask)
{
	glStencilMask_(mask);
}

void APIENTRY glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
	glStencilOp_(fail, zfail, zpass);
}

void APIENTRY glTexCoord1d(GLdouble s)
{
	glTexCoord1d_(s);
}

void APIENTRY glTexCoord1dv(const GLdouble* v)
{
	glTexCoord1dv_(v);
}

void APIENTRY glTexCoord1f(GLfloat s)
{
	glTexCoord1f_(s);
}

void APIENTRY glTexCoord1fv(const GLfloat* v)
{
	glTexCoord1fv_(v);
}

void APIENTRY glTexCoord1i(GLint s)
{
	glTexCoord1i_(s);
}

void APIENTRY glTexCoord1iv(const GLint* v)
{
	glTexCoord1iv_(v);
}

void APIENTRY glTexCoord1s(GLshort s)
{
	glTexCoord1s_(s);
}

void APIENTRY glTexCoord1sv(const GLshort* v)
{
	glTexCoord1sv_(v);
}

void APIENTRY glTexCoord2d(GLdouble s, GLdouble t)
{
	glTexCoord2d_(s, t);
}

void APIENTRY glTexCoord2dv(const GLdouble* v)
{
	glTexCoord2dv_(v);
}

void APIENTRY glTexCoord2f(GLfloat s, GLfloat t)
{
	glTexCoord2f_(s, t);
}

void APIENTRY glTexCoord2fv(const GLfloat* v)
{
	glTexCoord2fv_(v);
}

void APIENTRY glTexCoord2i(GLint s, GLint t)
{
	glTexCoord2i_(s, t);
}

void APIENTRY glTexCoord2iv(const GLint* v)
{
	glTexCoord2iv_(v);
}

void APIENTRY glTexCoord2s(GLshort s, GLshort t)
{
	glTexCoord2s_(s, t);
}

void APIENTRY glTexCoord2sv(const GLshort* v)
{
	glTexCoord2sv_(v);
}

void APIENTRY glTexCoord3d(GLdouble s, GLdouble t, GLdouble r)
{
	glTexCoord3d_(s, t, r);
}

void APIENTRY glTexCoord3dv(const GLdouble* v)
{
	glTexCoord3dv_(v);
}

void APIENTRY glTexCoord3f(GLfloat s, GLfloat t, GLfloat r)
{
	glTexCoord3f_(s, t, r);
}

void APIENTRY glTexCoord3fv(const GLfloat* v)
{
	glTexCoord3fv_(v);
}

void APIENTRY glTexCoord3i(GLint s, GLint t, GLint r)
{
	glTexCoord3i_(s, t, r);
}

void APIENTRY glTexCoord3iv(const GLint* v)
{
	glTexCoord3iv_(v);
}

void APIENTRY glTexCoord3s(GLshort s, GLshort t, GLshort r)
{
	glTexCoord3s_(s, t, r);
}

void APIENTRY glTexCoord3sv(const GLshort* v)
{
	glTexCoord3sv_(v);
}

void APIENTRY glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
	glTexCoord4d_(s, t, r, q);
}

void APIENTRY glTexCoord4dv(const GLdouble* v)
{
	glTexCoord4dv_(v);
}

void APIENTRY glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
	glTexCoord4f_(s, t, r, q);
}

void APIENTRY glTexCoord4fv(const GLfloat* v)
{
	glTexCoord4fv_(v);
}

void APIENTRY glTexCoord4i(GLint s, GLint t, GLint r, GLint q)
{
	glTexCoord4i_(s, t, r, q);
}

void APIENTRY glTexCoord4iv(const GLint* v)
{
	glTexCoord4iv_(v);
}

void APIENTRY glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q)
{
	glTexCoord4s_(s, t, r, q);
}

void APIENTRY glTexCoord4sv(const GLshort* v)
{
	glTexCoord4sv_(v);
}

void APIENTRY glTexEnvf(GLenum target, GLenum pname, GLfloat param)
{
	glTexEnvf_(target, pname, param);
}

void APIENTRY glTexEnvfv(GLenum target, GLenum pname, const GLfloat* params)
{
	glTexEnvfv_(target, pname, params);
}

void APIENTRY glTexEnvi(GLenum target, GLenum pname, GLint param)
{
	glTexEnvi_(target, pname, param);
}

void APIENTRY glTexEnviv(GLenum target, GLenum pname, const GLint* params)
{
	glTexEnviv_(target, pname, params);
}

void APIENTRY glTexGend(GLenum coord, GLenum pname, GLdouble param)
{
	glTexGend_(coord, pname, param);
}

void APIENTRY glTexGendv(GLenum coord, GLenum pname, const GLdouble* params)
{
	glTexGendv_(coord, pname, params);
}

void APIENTRY glTexGenf(GLenum coord, GLenum pname, GLfloat param)
{
	glTexGenf_(coord, pname, param);
}

void APIENTRY glTexGenfv(GLenum coord, GLenum pname, const GLfloat* params)
{
	glTexGenfv_(coord, pname, params);
}

void APIENTRY glTexGeni(GLenum coord, GLenum pname, GLint param)
{
	glTexGeni_(coord, pname, param);
}

void APIENTRY glTexGeniv(GLenum coord, GLenum pname, const GLint* params)
{
	glTexGeniv_(coord, pname, params);
}

void APIENTRY glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void* pixels)
{
	glTexImage1D_(target, level, internalformat, width, border, format, type, pixels);
}

void APIENTRY glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels)
{
	glTexImage2D_(target, level, internalformat, width, height, border, format, type, pixels);
}

void APIENTRY glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
	glTexParameterf_(target, pname, param);
}

void APIENTRY glTexParameterfv(GLenum target, GLenum pname, const GLfloat* params)
{
	glTexParameterfv_(target, pname, params);
}

void APIENTRY glTexParameteri(GLenum target, GLenum pname, GLint param)
{
	glTexParameteri_(target, pname, param);
}

void APIENTRY glTexParameteriv(GLenum target, GLenum pname, const GLint* params)
{
	glTexParameteriv_(target, pname, params);
}

void APIENTRY glTranslated(GLdouble x, GLdouble y, GLdouble z)
{
	glTranslated_(x, y, z);
}

void APIENTRY glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
	glTranslatef_(x, y, z);
}

void APIENTRY glVertex2d(GLdouble x, GLdouble y)
{
	glVertex2d_(x, y);
}

void APIENTRY glVertex2dv(const GLdouble* v)
{
	glVertex2dv_(v);
}

void APIENTRY glVertex2f(GLfloat x, GLfloat y)
{
	glVertex2f_(x, y);
}

void APIENTRY glVertex2fv(const GLfloat* v)
{
	glVertex2fv_(v);
}

void APIENTRY glVertex2i(GLint x, GLint y)
{
	glVertex2i_(x, y);
}

void APIENTRY glVertex2iv(const GLint* v)
{
	glVertex2iv_(v);
}

void APIENTRY glVertex2s(GLshort x, GLshort y)
{
	glVertex2s_(x, y);
}

void APIENTRY glVertex2sv(const GLshort* v)
{
	glVertex2sv_(v);
}

void APIENTRY glVertex3d(GLdouble x, GLdouble y, GLdouble z)
{
	glVertex3d_(x, y, z);
}

void APIENTRY glVertex3dv(const GLdouble* v)
{
	glVertex3dv_(v);
}

void APIENTRY glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
	glVertex3f_(x, y, z);
}

void APIENTRY glVertex3fv(const GLfloat* v)
{
	glVertex3fv_(v);
}

void APIENTRY glVertex3i(GLint x, GLint y, GLint z)
{
	glVertex3i_(x, y, z);
}

void APIENTRY glVertex3iv(const GLint* v)
{
	glVertex3iv_(v);
}

void APIENTRY glVertex3s(GLshort x, GLshort y, GLshort z)
{
	glVertex3s_(x, y, z);
}

void APIENTRY glVertex3sv(const GLshort* v)
{
	glVertex3sv_(v);
}

void APIENTRY glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	glVertex4d_(x, y, z, w);
}

void APIENTRY glVertex4dv(const GLdouble* v)
{
	glVertex4dv_(v);
}

void APIENTRY glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	glVertex4f_(x, y, z, w);
}

void APIENTRY glVertex4fv(const GLfloat* v)
{
	glVertex4fv_(v);
}

void APIENTRY glVertex4i(GLint x, GLint y, GLint z, GLint w)
{
	glVertex4i_(x, y, z, w);
}

void APIENTRY glVertex4iv(const GLint* v)
{
	glVertex4iv_(v);
}

void APIENTRY glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
	glVertex4s_(x, y, z, w);
}

void APIENTRY glVertex4sv(const GLshort* v)
{
	glVertex4sv_(v);
}

void APIENTRY glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	glViewport_(x, y, width, height);
}



//
// v1.1
//

GLboolean APIENTRY glAreTexturesResident(GLsizei n, const GLuint* textures, GLboolean* residences)
{
	return glAreTexturesResident_(n, textures, residences);
}

void APIENTRY glArrayElement(GLint i)
{
	glArrayElement_(i);
}

void APIENTRY glBindTexture(GLenum target, GLuint texture)
{
	glBindTexture_(target, texture);
}

void APIENTRY glColorPointer(GLint size, GLenum type, GLsizei stride, const void* pointer)
{
	glColorPointer_(size, type, stride, pointer);
}

void APIENTRY glCopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
	glCopyTexImage1D_(target, level, internalformat, x, y, width, border);
}

void APIENTRY glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
	glCopyTexImage2D_(target, level, internalformat, x, y, width, height, border);
}

void APIENTRY glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
	glCopyTexSubImage1D_(target, level, xoffset, x, y, width);
}

void APIENTRY glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	glCopyTexSubImage2D_(target, level, xoffset, yoffset, x, y, width, height);
}

void APIENTRY glDeleteTextures(GLsizei n, const GLuint* textures)
{
	glDeleteTextures_(n, textures);
}

void APIENTRY glDisableClientState(GLenum array)
{
	glDisableClientState_(array);
}

void APIENTRY glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
	glDrawArrays_(mode, first, count);
}

void APIENTRY glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices)
{
	glDrawElements_(mode, count, type, indices);
}

void APIENTRY glEdgeFlagPointer(GLsizei stride, const void* pointer)
{
	glEdgeFlagPointer_(stride, pointer);
}

void APIENTRY glEnableClientState(GLenum array)
{
	glEnableClientState_(array);
}

void APIENTRY glGenTextures(GLsizei n, GLuint* textures)
{
	glGenTextures_(n, textures);
}

void APIENTRY glGetPointerv(GLenum pname, void** params)
{
	glGetPointerv_(pname, params);
}

void APIENTRY glIndexPointer(GLenum type, GLsizei stride, const void* pointer)
{
	glIndexPointer_(type, stride, pointer);
}

void APIENTRY glIndexub(GLubyte c)
{
	glIndexub_(c);
}

void APIENTRY glIndexubv(const GLubyte* c)
{
	glIndexubv_(c);
}

void APIENTRY glInterleavedArrays(GLenum format, GLsizei stride, const void* pointer)
{
	glInterleavedArrays_(format, stride, pointer);
}

GLboolean APIENTRY glIsTexture(GLuint texture)
{
	return glIsTexture_(texture);
}

void APIENTRY glNormalPointer(GLenum type, GLsizei stride, const void* pointer)
{
	glNormalPointer_(type, stride, pointer);
}

void APIENTRY glPolygonOffset(GLfloat factor, GLfloat units)
{
	glPolygonOffset_(factor, units);
}

void APIENTRY glPopClientAttrib()
{
	glPopClientAttrib_();
}

void APIENTRY glPrioritizeTextures(GLsizei n, const GLuint* textures, const GLfloat* priorities)
{
	glPrioritizeTextures_(n, textures, priorities);
}

void APIENTRY glPushClientAttrib(GLbitfield mask)
{
	glPushClientAttrib_(mask);
}

void APIENTRY glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void* pointer)
{
	glTexCoordPointer_(size, type, stride, pointer);
}

void APIENTRY glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void* pixels)
{
	glTexSubImage1D_(target, level, xoffset, width, format, type, pixels);
}

void APIENTRY glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
{
	glTexSubImage2D_(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

void APIENTRY glVertexPointer(GLint size, GLenum type, GLsizei stride, const void* pointer)
{
	glVertexPointer_(size, type, stride, pointer);
}


class SdlOgl11Loader::Detail
{
public:
	Detail() = delete;


	static std::string error_message_;


	template<typename T>
	static void resolve_symbol(
		const char* const symbol_name,
		T& symbol)
	{
		symbol = reinterpret_cast<T>(::SDL_GL_GetProcAddress(symbol_name));
	}

	static bool initialize_v1_0()
	{
		resolve_symbol("glAccum", glAccum_);
		resolve_symbol("glAlphaFunc", glAlphaFunc_);
		resolve_symbol("glBegin", glBegin_);
		resolve_symbol("glBitmap", glBitmap_);
		resolve_symbol("glBlendFunc", glBlendFunc_);
		resolve_symbol("glCallList", glCallList_);
		resolve_symbol("glCallLists", glCallLists_);
		resolve_symbol("glClear", glClear_);
		resolve_symbol("glClearAccum", glClearAccum_);
		resolve_symbol("glClearColor", glClearColor_);
		resolve_symbol("glClearDepth", glClearDepth_);
		resolve_symbol("glClearIndex", glClearIndex_);
		resolve_symbol("glClearStencil", glClearStencil_);
		resolve_symbol("glClipPlane", glClipPlane_);
		resolve_symbol("glColor3b", glColor3b_);
		resolve_symbol("glColor3bv", glColor3bv_);
		resolve_symbol("glColor3d", glColor3d_);
		resolve_symbol("glColor3dv", glColor3dv_);
		resolve_symbol("glColor3f", glColor3f_);
		resolve_symbol("glColor3fv", glColor3fv_);
		resolve_symbol("glColor3i", glColor3i_);
		resolve_symbol("glColor3iv", glColor3iv_);
		resolve_symbol("glColor3s", glColor3s_);
		resolve_symbol("glColor3sv", glColor3sv_);
		resolve_symbol("glColor3ub", glColor3ub_);
		resolve_symbol("glColor3ubv", glColor3ubv_);
		resolve_symbol("glColor3ui", glColor3ui_);
		resolve_symbol("glColor3uiv", glColor3uiv_);
		resolve_symbol("glColor3us", glColor3us_);
		resolve_symbol("glColor3usv", glColor3usv_);
		resolve_symbol("glColor4b", glColor4b_);
		resolve_symbol("glColor4bv", glColor4bv_);
		resolve_symbol("glColor4d", glColor4d_);
		resolve_symbol("glColor4dv", glColor4dv_);
		resolve_symbol("glColor4f", glColor4f_);
		resolve_symbol("glColor4fv", glColor4fv_);
		resolve_symbol("glColor4i", glColor4i_);
		resolve_symbol("glColor4iv", glColor4iv_);
		resolve_symbol("glColor4s", glColor4s_);
		resolve_symbol("glColor4sv", glColor4sv_);
		resolve_symbol("glColor4ub", glColor4ub_);
		resolve_symbol("glColor4ubv", glColor4ubv_);
		resolve_symbol("glColor4ui", glColor4ui_);
		resolve_symbol("glColor4uiv", glColor4uiv_);
		resolve_symbol("glColor4us", glColor4us_);
		resolve_symbol("glColor4usv", glColor4usv_);
		resolve_symbol("glColorMask", glColorMask_);
		resolve_symbol("glColorMaterial", glColorMaterial_);
		resolve_symbol("glCopyPixels", glCopyPixels_);
		resolve_symbol("glCullFace", glCullFace_);
		resolve_symbol("glDeleteLists", glDeleteLists_);
		resolve_symbol("glDepthFunc", glDepthFunc_);
		resolve_symbol("glDepthMask", glDepthMask_);
		resolve_symbol("glDepthRange", glDepthRange_);
		resolve_symbol("glDisable", glDisable_);
		resolve_symbol("glDrawBuffer", glDrawBuffer_);
		resolve_symbol("glDrawPixels", glDrawPixels_);
		resolve_symbol("glEdgeFlag", glEdgeFlag_);
		resolve_symbol("glEdgeFlagv", glEdgeFlagv_);
		resolve_symbol("glEnable", glEnable_);
		resolve_symbol("glEnd", glEnd_);
		resolve_symbol("glEndList", glEndList_);
		resolve_symbol("glEvalCoord1d", glEvalCoord1d_);
		resolve_symbol("glEvalCoord1dv", glEvalCoord1dv_);
		resolve_symbol("glEvalCoord1f", glEvalCoord1f_);
		resolve_symbol("glEvalCoord1fv", glEvalCoord1fv_);
		resolve_symbol("glEvalCoord2d", glEvalCoord2d_);
		resolve_symbol("glEvalCoord2dv", glEvalCoord2dv_);
		resolve_symbol("glEvalCoord2f", glEvalCoord2f_);
		resolve_symbol("glEvalCoord2fv", glEvalCoord2fv_);
		resolve_symbol("glEvalMesh1", glEvalMesh1_);
		resolve_symbol("glEvalMesh2", glEvalMesh2_);
		resolve_symbol("glEvalPoint1", glEvalPoint1_);
		resolve_symbol("glEvalPoint2", glEvalPoint2_);
		resolve_symbol("glFeedbackBuffer", glFeedbackBuffer_);
		resolve_symbol("glFinish", glFinish_);
		resolve_symbol("glFlush", glFlush_);
		resolve_symbol("glFogf", glFogf_);
		resolve_symbol("glFogfv", glFogfv_);
		resolve_symbol("glFogi", glFogi_);
		resolve_symbol("glFogiv", glFogiv_);
		resolve_symbol("glFrontFace", glFrontFace_);
		resolve_symbol("glFrustum", glFrustum_);
		resolve_symbol("glGenLists", glGenLists_);
		resolve_symbol("glGetBooleanv", glGetBooleanv_);
		resolve_symbol("glGetClipPlane", glGetClipPlane_);
		resolve_symbol("glGetDoublev", glGetDoublev_);
		resolve_symbol("glGetError", glGetError_);
		resolve_symbol("glGetFloatv", glGetFloatv_);
		resolve_symbol("glGetIntegerv", glGetIntegerv_);
		resolve_symbol("glGetLightfv", glGetLightfv_);
		resolve_symbol("glGetLightiv", glGetLightiv_);
		resolve_symbol("glGetMapdv", glGetMapdv_);
		resolve_symbol("glGetMapfv", glGetMapfv_);
		resolve_symbol("glGetMapiv", glGetMapiv_);
		resolve_symbol("glGetMaterialfv", glGetMaterialfv_);
		resolve_symbol("glGetMaterialiv", glGetMaterialiv_);
		resolve_symbol("glGetPixelMapfv", glGetPixelMapfv_);
		resolve_symbol("glGetPixelMapuiv", glGetPixelMapuiv_);
		resolve_symbol("glGetPixelMapusv", glGetPixelMapusv_);
		resolve_symbol("glGetPolygonStipple", glGetPolygonStipple_);
		resolve_symbol("glGetString", glGetString_);
		resolve_symbol("glGetTexEnvfv", glGetTexEnvfv_);
		resolve_symbol("glGetTexEnviv", glGetTexEnviv_);
		resolve_symbol("glGetTexGendv", glGetTexGendv_);
		resolve_symbol("glGetTexGenfv", glGetTexGenfv_);
		resolve_symbol("glGetTexGeniv", glGetTexGeniv_);
		resolve_symbol("glGetTexImage", glGetTexImage_);
		resolve_symbol("glGetTexLevelParameterfv", glGetTexLevelParameterfv_);
		resolve_symbol("glGetTexLevelParameteriv", glGetTexLevelParameteriv_);
		resolve_symbol("glGetTexParameterfv", glGetTexParameterfv_);
		resolve_symbol("glGetTexParameteriv", glGetTexParameteriv_);
		resolve_symbol("glHint", glHint_);
		resolve_symbol("glIndexMask", glIndexMask_);
		resolve_symbol("glIndexd", glIndexd_);
		resolve_symbol("glIndexdv", glIndexdv_);
		resolve_symbol("glIndexf", glIndexf_);
		resolve_symbol("glIndexfv", glIndexfv_);
		resolve_symbol("glIndexi", glIndexi_);
		resolve_symbol("glIndexiv", glIndexiv_);
		resolve_symbol("glIndexs", glIndexs_);
		resolve_symbol("glIndexsv", glIndexsv_);
		resolve_symbol("glInitNames", glInitNames_);
		resolve_symbol("glIsEnabled", glIsEnabled_);
		resolve_symbol("glIsList", glIsList_);
		resolve_symbol("glLightModelf", glLightModelf_);
		resolve_symbol("glLightModelfv", glLightModelfv_);
		resolve_symbol("glLightModeli", glLightModeli_);
		resolve_symbol("glLightModeliv", glLightModeliv_);
		resolve_symbol("glLightf", glLightf_);
		resolve_symbol("glLightfv", glLightfv_);
		resolve_symbol("glLighti", glLighti_);
		resolve_symbol("glLightiv", glLightiv_);
		resolve_symbol("glLineStipple", glLineStipple_);
		resolve_symbol("glLineWidth", glLineWidth_);
		resolve_symbol("glListBase", glListBase_);
		resolve_symbol("glLoadIdentity", glLoadIdentity_);
		resolve_symbol("glLoadMatrixd", glLoadMatrixd_);
		resolve_symbol("glLoadMatrixf", glLoadMatrixf_);
		resolve_symbol("glLoadName", glLoadName_);
		resolve_symbol("glLogicOp", glLogicOp_);
		resolve_symbol("glMap1d", glMap1d_);
		resolve_symbol("glMap1f", glMap1f_);
		resolve_symbol("glMap2d", glMap2d_);
		resolve_symbol("glMap2f", glMap2f_);
		resolve_symbol("glMapGrid1d", glMapGrid1d_);
		resolve_symbol("glMapGrid1f", glMapGrid1f_);
		resolve_symbol("glMapGrid2d", glMapGrid2d_);
		resolve_symbol("glMapGrid2f", glMapGrid2f_);
		resolve_symbol("glMaterialf", glMaterialf_);
		resolve_symbol("glMaterialfv", glMaterialfv_);
		resolve_symbol("glMateriali", glMateriali_);
		resolve_symbol("glMaterialiv", glMaterialiv_);
		resolve_symbol("glMatrixMode", glMatrixMode_);
		resolve_symbol("glMultMatrixd", glMultMatrixd_);
		resolve_symbol("glMultMatrixf", glMultMatrixf_);
		resolve_symbol("glNewList", glNewList_);
		resolve_symbol("glNormal3b", glNormal3b_);
		resolve_symbol("glNormal3bv", glNormal3bv_);
		resolve_symbol("glNormal3d", glNormal3d_);
		resolve_symbol("glNormal3dv", glNormal3dv_);
		resolve_symbol("glNormal3f", glNormal3f_);
		resolve_symbol("glNormal3fv", glNormal3fv_);
		resolve_symbol("glNormal3i", glNormal3i_);
		resolve_symbol("glNormal3iv", glNormal3iv_);
		resolve_symbol("glNormal3s", glNormal3s_);
		resolve_symbol("glNormal3sv", glNormal3sv_);
		resolve_symbol("glOrtho", glOrtho_);
		resolve_symbol("glPassThrough", glPassThrough_);
		resolve_symbol("glPixelMapfv", glPixelMapfv_);
		resolve_symbol("glPixelMapuiv", glPixelMapuiv_);
		resolve_symbol("glPixelMapusv", glPixelMapusv_);
		resolve_symbol("glPixelStoref", glPixelStoref_);
		resolve_symbol("glPixelStorei", glPixelStorei_);
		resolve_symbol("glPixelTransferf", glPixelTransferf_);
		resolve_symbol("glPixelTransferi", glPixelTransferi_);
		resolve_symbol("glPixelZoom", glPixelZoom_);
		resolve_symbol("glPointSize", glPointSize_);
		resolve_symbol("glPolygonMode", glPolygonMode_);
		resolve_symbol("glPolygonStipple", glPolygonStipple_);
		resolve_symbol("glPopAttrib", glPopAttrib_);
		resolve_symbol("glPopMatrix", glPopMatrix_);
		resolve_symbol("glPopName", glPopName_);
		resolve_symbol("glPushAttrib", glPushAttrib_);
		resolve_symbol("glPushMatrix", glPushMatrix_);
		resolve_symbol("glPushName", glPushName_);
		resolve_symbol("glRasterPos2d", glRasterPos2d_);
		resolve_symbol("glRasterPos2dv", glRasterPos2dv_);
		resolve_symbol("glRasterPos2f", glRasterPos2f_);
		resolve_symbol("glRasterPos2fv", glRasterPos2fv_);
		resolve_symbol("glRasterPos2i", glRasterPos2i_);
		resolve_symbol("glRasterPos2iv", glRasterPos2iv_);
		resolve_symbol("glRasterPos2s", glRasterPos2s_);
		resolve_symbol("glRasterPos2sv", glRasterPos2sv_);
		resolve_symbol("glRasterPos3d", glRasterPos3d_);
		resolve_symbol("glRasterPos3dv", glRasterPos3dv_);
		resolve_symbol("glRasterPos3f", glRasterPos3f_);
		resolve_symbol("glRasterPos3fv", glRasterPos3fv_);
		resolve_symbol("glRasterPos3i", glRasterPos3i_);
		resolve_symbol("glRasterPos3iv", glRasterPos3iv_);
		resolve_symbol("glRasterPos3s", glRasterPos3s_);
		resolve_symbol("glRasterPos3sv", glRasterPos3sv_);
		resolve_symbol("glRasterPos4d", glRasterPos4d_);
		resolve_symbol("glRasterPos4dv", glRasterPos4dv_);
		resolve_symbol("glRasterPos4f", glRasterPos4f_);
		resolve_symbol("glRasterPos4fv", glRasterPos4fv_);
		resolve_symbol("glRasterPos4i", glRasterPos4i_);
		resolve_symbol("glRasterPos4iv", glRasterPos4iv_);
		resolve_symbol("glRasterPos4s", glRasterPos4s_);
		resolve_symbol("glRasterPos4sv", glRasterPos4sv_);
		resolve_symbol("glReadBuffer", glReadBuffer_);
		resolve_symbol("glReadPixels", glReadPixels_);
		resolve_symbol("glRectd", glRectd_);
		resolve_symbol("glRectdv", glRectdv_);
		resolve_symbol("glRectf", glRectf_);
		resolve_symbol("glRectfv", glRectfv_);
		resolve_symbol("glRecti", glRecti_);
		resolve_symbol("glRectiv", glRectiv_);
		resolve_symbol("glRects", glRects_);
		resolve_symbol("glRectsv", glRectsv_);
		resolve_symbol("glRenderMode", glRenderMode_);
		resolve_symbol("glRotated", glRotated_);
		resolve_symbol("glRotatef", glRotatef_);
		resolve_symbol("glScaled", glScaled_);
		resolve_symbol("glScalef", glScalef_);
		resolve_symbol("glScissor", glScissor_);
		resolve_symbol("glSelectBuffer", glSelectBuffer_);
		resolve_symbol("glShadeModel", glShadeModel_);
		resolve_symbol("glStencilFunc", glStencilFunc_);
		resolve_symbol("glStencilMask", glStencilMask_);
		resolve_symbol("glStencilOp", glStencilOp_);
		resolve_symbol("glTexCoord1d", glTexCoord1d_);
		resolve_symbol("glTexCoord1dv", glTexCoord1dv_);
		resolve_symbol("glTexCoord1f", glTexCoord1f_);
		resolve_symbol("glTexCoord1fv", glTexCoord1fv_);
		resolve_symbol("glTexCoord1i", glTexCoord1i_);
		resolve_symbol("glTexCoord1iv", glTexCoord1iv_);
		resolve_symbol("glTexCoord1s", glTexCoord1s_);
		resolve_symbol("glTexCoord1sv", glTexCoord1sv_);
		resolve_symbol("glTexCoord2d", glTexCoord2d_);
		resolve_symbol("glTexCoord2dv", glTexCoord2dv_);
		resolve_symbol("glTexCoord2f", glTexCoord2f_);
		resolve_symbol("glTexCoord2fv", glTexCoord2fv_);
		resolve_symbol("glTexCoord2i", glTexCoord2i_);
		resolve_symbol("glTexCoord2iv", glTexCoord2iv_);
		resolve_symbol("glTexCoord2s", glTexCoord2s_);
		resolve_symbol("glTexCoord2sv", glTexCoord2sv_);
		resolve_symbol("glTexCoord3d", glTexCoord3d_);
		resolve_symbol("glTexCoord3dv", glTexCoord3dv_);
		resolve_symbol("glTexCoord3f", glTexCoord3f_);
		resolve_symbol("glTexCoord3fv", glTexCoord3fv_);
		resolve_symbol("glTexCoord3i", glTexCoord3i_);
		resolve_symbol("glTexCoord3iv", glTexCoord3iv_);
		resolve_symbol("glTexCoord3s", glTexCoord3s_);
		resolve_symbol("glTexCoord3sv", glTexCoord3sv_);
		resolve_symbol("glTexCoord4d", glTexCoord4d_);
		resolve_symbol("glTexCoord4dv", glTexCoord4dv_);
		resolve_symbol("glTexCoord4f", glTexCoord4f_);
		resolve_symbol("glTexCoord4fv", glTexCoord4fv_);
		resolve_symbol("glTexCoord4i", glTexCoord4i_);
		resolve_symbol("glTexCoord4iv", glTexCoord4iv_);
		resolve_symbol("glTexCoord4s", glTexCoord4s_);
		resolve_symbol("glTexCoord4sv", glTexCoord4sv_);
		resolve_symbol("glTexEnvf", glTexEnvf_);
		resolve_symbol("glTexEnvfv", glTexEnvfv_);
		resolve_symbol("glTexEnvi", glTexEnvi_);
		resolve_symbol("glTexEnviv", glTexEnviv_);
		resolve_symbol("glTexGend", glTexGend_);
		resolve_symbol("glTexGendv", glTexGendv_);
		resolve_symbol("glTexGenf", glTexGenf_);
		resolve_symbol("glTexGenfv", glTexGenfv_);
		resolve_symbol("glTexGeni", glTexGeni_);
		resolve_symbol("glTexGeniv", glTexGeniv_);
		resolve_symbol("glTexImage1D", glTexImage1D_);
		resolve_symbol("glTexImage2D", glTexImage2D_);
		resolve_symbol("glTexParameterf", glTexParameterf_);
		resolve_symbol("glTexParameterfv", glTexParameterfv_);
		resolve_symbol("glTexParameteri", glTexParameteri_);
		resolve_symbol("glTexParameteriv", glTexParameteriv_);
		resolve_symbol("glTranslated", glTranslated_);
		resolve_symbol("glTranslatef", glTranslatef_);
		resolve_symbol("glVertex2d", glVertex2d_);
		resolve_symbol("glVertex2dv", glVertex2dv_);
		resolve_symbol("glVertex2f", glVertex2f_);
		resolve_symbol("glVertex2fv", glVertex2fv_);
		resolve_symbol("glVertex2i", glVertex2i_);
		resolve_symbol("glVertex2iv", glVertex2iv_);
		resolve_symbol("glVertex2s", glVertex2s_);
		resolve_symbol("glVertex2sv", glVertex2sv_);
		resolve_symbol("glVertex3d", glVertex3d_);
		resolve_symbol("glVertex3dv", glVertex3dv_);
		resolve_symbol("glVertex3f", glVertex3f_);
		resolve_symbol("glVertex3fv", glVertex3fv_);
		resolve_symbol("glVertex3i", glVertex3i_);
		resolve_symbol("glVertex3iv", glVertex3iv_);
		resolve_symbol("glVertex3s", glVertex3s_);
		resolve_symbol("glVertex3sv", glVertex3sv_);
		resolve_symbol("glVertex4d", glVertex4d_);
		resolve_symbol("glVertex4dv", glVertex4dv_);
		resolve_symbol("glVertex4f", glVertex4f_);
		resolve_symbol("glVertex4fv", glVertex4fv_);
		resolve_symbol("glVertex4i", glVertex4i_);
		resolve_symbol("glVertex4iv", glVertex4iv_);
		resolve_symbol("glVertex4s", glVertex4s_);
		resolve_symbol("glVertex4sv", glVertex4sv_);
		resolve_symbol("glViewport", glViewport_);

		return
			glAccum_ &&
			glAlphaFunc_ &&
			glBegin_ &&
			glBitmap_ &&
			glBlendFunc_ &&
			glCallList_ &&
			glCallLists_ &&
			glClear_ &&
			glClearAccum_ &&
			glClearColor_ &&
			glClearDepth_ &&
			glClearIndex_ &&
			glClearStencil_ &&
			glClipPlane_ &&
			glColor3b_ &&
			glColor3bv_ &&
			glColor3d_ &&
			glColor3dv_ &&
			glColor3f_ &&
			glColor3fv_ &&
			glColor3i_ &&
			glColor3iv_ &&
			glColor3s_ &&
			glColor3sv_ &&
			glColor3ub_ &&
			glColor3ubv_ &&
			glColor3ui_ &&
			glColor3uiv_ &&
			glColor3us_ &&
			glColor3usv_ &&
			glColor4b_ &&
			glColor4bv_ &&
			glColor4d_ &&
			glColor4dv_ &&
			glColor4f_ &&
			glColor4fv_ &&
			glColor4i_ &&
			glColor4iv_ &&
			glColor4s_ &&
			glColor4sv_ &&
			glColor4ub_ &&
			glColor4ubv_ &&
			glColor4ui_ &&
			glColor4uiv_ &&
			glColor4us_ &&
			glColor4usv_ &&
			glColorMask_ &&
			glColorMaterial_ &&
			glCopyPixels_ &&
			glCullFace_ &&
			glDeleteLists_ &&
			glDepthFunc_ &&
			glDepthMask_ &&
			glDepthRange_ &&
			glDisable_ &&
			glDrawBuffer_ &&
			glDrawPixels_ &&
			glEdgeFlag_ &&
			glEdgeFlagv_ &&
			glEnable_ &&
			glEnd_ &&
			glEndList_ &&
			glEvalCoord1d_ &&
			glEvalCoord1dv_ &&
			glEvalCoord1f_ &&
			glEvalCoord1fv_ &&
			glEvalCoord2d_ &&
			glEvalCoord2dv_ &&
			glEvalCoord2f_ &&
			glEvalCoord2fv_ &&
			glEvalMesh1_ &&
			glEvalMesh2_ &&
			glEvalPoint1_ &&
			glEvalPoint2_ &&
			glFeedbackBuffer_ &&
			glFinish_ &&
			glFlush_ &&
			glFogf_ &&
			glFogfv_ &&
			glFogi_ &&
			glFogiv_ &&
			glFrontFace_ &&
			glFrustum_ &&
			glGenLists_ &&
			glGetBooleanv_ &&
			glGetClipPlane_ &&
			glGetDoublev_ &&
			glGetError_ &&
			glGetFloatv_ &&
			glGetIntegerv_ &&
			glGetLightfv_ &&
			glGetLightiv_ &&
			glGetMapdv_ &&
			glGetMapfv_ &&
			glGetMapiv_ &&
			glGetMaterialfv_ &&
			glGetMaterialiv_ &&
			glGetPixelMapfv_ &&
			glGetPixelMapuiv_ &&
			glGetPixelMapusv_ &&
			glGetPolygonStipple_ &&
			glGetString_ &&
			glGetTexEnvfv_ &&
			glGetTexEnviv_ &&
			glGetTexGendv_ &&
			glGetTexGenfv_ &&
			glGetTexGeniv_ &&
			glGetTexImage_ &&
			glGetTexLevelParameterfv_ &&
			glGetTexLevelParameteriv_ &&
			glGetTexParameterfv_ &&
			glGetTexParameteriv_ &&
			glHint_ &&
			glIndexMask_ &&
			glIndexd_ &&
			glIndexdv_ &&
			glIndexf_ &&
			glIndexfv_ &&
			glIndexi_ &&
			glIndexiv_ &&
			glIndexs_ &&
			glIndexsv_ &&
			glInitNames_ &&
			glIsEnabled_ &&
			glIsList_ &&
			glLightModelf_ &&
			glLightModelfv_ &&
			glLightModeli_ &&
			glLightModeliv_ &&
			glLightf_ &&
			glLightfv_ &&
			glLighti_ &&
			glLightiv_ &&
			glLineStipple_ &&
			glLineWidth_ &&
			glListBase_ &&
			glLoadIdentity_ &&
			glLoadMatrixd_ &&
			glLoadMatrixf_ &&
			glLoadName_ &&
			glLogicOp_ &&
			glMap1d_ &&
			glMap1f_ &&
			glMap2d_ &&
			glMap2f_ &&
			glMapGrid1d_ &&
			glMapGrid1f_ &&
			glMapGrid2d_ &&
			glMapGrid2f_ &&
			glMaterialf_ &&
			glMaterialfv_ &&
			glMateriali_ &&
			glMaterialiv_ &&
			glMatrixMode_ &&
			glMultMatrixd_ &&
			glMultMatrixf_ &&
			glNewList_ &&
			glNormal3b_ &&
			glNormal3bv_ &&
			glNormal3d_ &&
			glNormal3dv_ &&
			glNormal3f_ &&
			glNormal3fv_ &&
			glNormal3i_ &&
			glNormal3iv_ &&
			glNormal3s_ &&
			glNormal3sv_ &&
			glOrtho_ &&
			glPassThrough_ &&
			glPixelMapfv_ &&
			glPixelMapuiv_ &&
			glPixelMapusv_ &&
			glPixelStoref_ &&
			glPixelStorei_ &&
			glPixelTransferf_ &&
			glPixelTransferi_ &&
			glPixelZoom_ &&
			glPointSize_ &&
			glPolygonMode_ &&
			glPolygonStipple_ &&
			glPopAttrib_ &&
			glPopMatrix_ &&
			glPopName_ &&
			glPushAttrib_ &&
			glPushMatrix_ &&
			glPushName_ &&
			glRasterPos2d_ &&
			glRasterPos2dv_ &&
			glRasterPos2f_ &&
			glRasterPos2fv_ &&
			glRasterPos2i_ &&
			glRasterPos2iv_ &&
			glRasterPos2s_ &&
			glRasterPos2sv_ &&
			glRasterPos3d_ &&
			glRasterPos3dv_ &&
			glRasterPos3f_ &&
			glRasterPos3fv_ &&
			glRasterPos3i_ &&
			glRasterPos3iv_ &&
			glRasterPos3s_ &&
			glRasterPos3sv_ &&
			glRasterPos4d_ &&
			glRasterPos4dv_ &&
			glRasterPos4f_ &&
			glRasterPos4fv_ &&
			glRasterPos4i_ &&
			glRasterPos4iv_ &&
			glRasterPos4s_ &&
			glRasterPos4sv_ &&
			glReadBuffer_ &&
			glReadPixels_ &&
			glRectd_ &&
			glRectdv_ &&
			glRectf_ &&
			glRectfv_ &&
			glRecti_ &&
			glRectiv_ &&
			glRects_ &&
			glRectsv_ &&
			glRenderMode_ &&
			glRotated_ &&
			glRotatef_ &&
			glScaled_ &&
			glScalef_ &&
			glScissor_ &&
			glSelectBuffer_ &&
			glShadeModel_ &&
			glStencilFunc_ &&
			glStencilMask_ &&
			glStencilOp_ &&
			glTexCoord1d_ &&
			glTexCoord1dv_ &&
			glTexCoord1f_ &&
			glTexCoord1fv_ &&
			glTexCoord1i_ &&
			glTexCoord1iv_ &&
			glTexCoord1s_ &&
			glTexCoord1sv_ &&
			glTexCoord2d_ &&
			glTexCoord2dv_ &&
			glTexCoord2f_ &&
			glTexCoord2fv_ &&
			glTexCoord2i_ &&
			glTexCoord2iv_ &&
			glTexCoord2s_ &&
			glTexCoord2sv_ &&
			glTexCoord3d_ &&
			glTexCoord3dv_ &&
			glTexCoord3f_ &&
			glTexCoord3fv_ &&
			glTexCoord3i_ &&
			glTexCoord3iv_ &&
			glTexCoord3s_ &&
			glTexCoord3sv_ &&
			glTexCoord4d_ &&
			glTexCoord4dv_ &&
			glTexCoord4f_ &&
			glTexCoord4fv_ &&
			glTexCoord4i_ &&
			glTexCoord4iv_ &&
			glTexCoord4s_ &&
			glTexCoord4sv_ &&
			glTexEnvf_ &&
			glTexEnvfv_ &&
			glTexEnvi_ &&
			glTexEnviv_ &&
			glTexGend_ &&
			glTexGendv_ &&
			glTexGenf_ &&
			glTexGenfv_ &&
			glTexGeni_ &&
			glTexGeniv_ &&
			glTexImage1D_ &&
			glTexImage2D_ &&
			glTexParameterf_ &&
			glTexParameterfv_ &&
			glTexParameteri_ &&
			glTexParameteriv_ &&
			glTranslated_ &&
			glTranslatef_ &&
			glVertex2d_ &&
			glVertex2dv_ &&
			glVertex2f_ &&
			glVertex2fv_ &&
			glVertex2i_ &&
			glVertex2iv_ &&
			glVertex2s_ &&
			glVertex2sv_ &&
			glVertex3d_ &&
			glVertex3dv_ &&
			glVertex3f_ &&
			glVertex3fv_ &&
			glVertex3i_ &&
			glVertex3iv_ &&
			glVertex3s_ &&
			glVertex3sv_ &&
			glVertex4d_ &&
			glVertex4dv_ &&
			glVertex4f_ &&
			glVertex4fv_ &&
			glVertex4i_ &&
			glVertex4iv_ &&
			glVertex4s_ &&
			glVertex4sv_ &&
			glViewport_ &&
			true;
	}

	static void uninitialize_v1_0()
	{
		glAccum_ = nullptr;
		glAlphaFunc_ = nullptr;
		glBegin_ = nullptr;
		glBitmap_ = nullptr;
		glBlendFunc_ = nullptr;
		glCallList_ = nullptr;
		glCallLists_ = nullptr;
		glClear_ = nullptr;
		glClearAccum_ = nullptr;
		glClearColor_ = nullptr;
		glClearDepth_ = nullptr;
		glClearIndex_ = nullptr;
		glClearStencil_ = nullptr;
		glClipPlane_ = nullptr;
		glColor3b_ = nullptr;
		glColor3bv_ = nullptr;
		glColor3d_ = nullptr;
		glColor3dv_ = nullptr;
		glColor3f_ = nullptr;
		glColor3fv_ = nullptr;
		glColor3i_ = nullptr;
		glColor3iv_ = nullptr;
		glColor3s_ = nullptr;
		glColor3sv_ = nullptr;
		glColor3ub_ = nullptr;
		glColor3ubv_ = nullptr;
		glColor3ui_ = nullptr;
		glColor3uiv_ = nullptr;
		glColor3us_ = nullptr;
		glColor3usv_ = nullptr;
		glColor4b_ = nullptr;
		glColor4bv_ = nullptr;
		glColor4d_ = nullptr;
		glColor4dv_ = nullptr;
		glColor4f_ = nullptr;
		glColor4fv_ = nullptr;
		glColor4i_ = nullptr;
		glColor4iv_ = nullptr;
		glColor4s_ = nullptr;
		glColor4sv_ = nullptr;
		glColor4ub_ = nullptr;
		glColor4ubv_ = nullptr;
		glColor4ui_ = nullptr;
		glColor4uiv_ = nullptr;
		glColor4us_ = nullptr;
		glColor4usv_ = nullptr;
		glColorMask_ = nullptr;
		glColorMaterial_ = nullptr;
		glCopyPixels_ = nullptr;
		glCullFace_ = nullptr;
		glDeleteLists_ = nullptr;
		glDepthFunc_ = nullptr;
		glDepthMask_ = nullptr;
		glDepthRange_ = nullptr;
		glDisable_ = nullptr;
		glDrawBuffer_ = nullptr;
		glDrawPixels_ = nullptr;
		glEdgeFlag_ = nullptr;
		glEdgeFlagv_ = nullptr;
		glEnable_ = nullptr;
		glEnd_ = nullptr;
		glEndList_ = nullptr;
		glEvalCoord1d_ = nullptr;
		glEvalCoord1dv_ = nullptr;
		glEvalCoord1f_ = nullptr;
		glEvalCoord1fv_ = nullptr;
		glEvalCoord2d_ = nullptr;
		glEvalCoord2dv_ = nullptr;
		glEvalCoord2f_ = nullptr;
		glEvalCoord2fv_ = nullptr;
		glEvalMesh1_ = nullptr;
		glEvalMesh2_ = nullptr;
		glEvalPoint1_ = nullptr;
		glEvalPoint2_ = nullptr;
		glFeedbackBuffer_ = nullptr;
		glFinish_ = nullptr;
		glFlush_ = nullptr;
		glFogf_ = nullptr;
		glFogfv_ = nullptr;
		glFogi_ = nullptr;
		glFogiv_ = nullptr;
		glFrontFace_ = nullptr;
		glFrustum_ = nullptr;
		glGenLists_ = nullptr;
		glGetBooleanv_ = nullptr;
		glGetClipPlane_ = nullptr;
		glGetDoublev_ = nullptr;
		glGetError_ = nullptr;
		glGetFloatv_ = nullptr;
		glGetIntegerv_ = nullptr;
		glGetLightfv_ = nullptr;
		glGetLightiv_ = nullptr;
		glGetMapdv_ = nullptr;
		glGetMapfv_ = nullptr;
		glGetMapiv_ = nullptr;
		glGetMaterialfv_ = nullptr;
		glGetMaterialiv_ = nullptr;
		glGetPixelMapfv_ = nullptr;
		glGetPixelMapuiv_ = nullptr;
		glGetPixelMapusv_ = nullptr;
		glGetPolygonStipple_ = nullptr;
		glGetString_ = nullptr;
		glGetTexEnvfv_ = nullptr;
		glGetTexEnviv_ = nullptr;
		glGetTexGendv_ = nullptr;
		glGetTexGenfv_ = nullptr;
		glGetTexGeniv_ = nullptr;
		glGetTexImage_ = nullptr;
		glGetTexLevelParameterfv_ = nullptr;
		glGetTexLevelParameteriv_ = nullptr;
		glGetTexParameterfv_ = nullptr;
		glGetTexParameteriv_ = nullptr;
		glHint_ = nullptr;
		glIndexMask_ = nullptr;
		glIndexd_ = nullptr;
		glIndexdv_ = nullptr;
		glIndexf_ = nullptr;
		glIndexfv_ = nullptr;
		glIndexi_ = nullptr;
		glIndexiv_ = nullptr;
		glIndexs_ = nullptr;
		glIndexsv_ = nullptr;
		glInitNames_ = nullptr;
		glIsEnabled_ = nullptr;
		glIsList_ = nullptr;
		glLightModelf_ = nullptr;
		glLightModelfv_ = nullptr;
		glLightModeli_ = nullptr;
		glLightModeliv_ = nullptr;
		glLightf_ = nullptr;
		glLightfv_ = nullptr;
		glLighti_ = nullptr;
		glLightiv_ = nullptr;
		glLineStipple_ = nullptr;
		glLineWidth_ = nullptr;
		glListBase_ = nullptr;
		glLoadIdentity_ = nullptr;
		glLoadMatrixd_ = nullptr;
		glLoadMatrixf_ = nullptr;
		glLoadName_ = nullptr;
		glLogicOp_ = nullptr;
		glMap1d_ = nullptr;
		glMap1f_ = nullptr;
		glMap2d_ = nullptr;
		glMap2f_ = nullptr;
		glMapGrid1d_ = nullptr;
		glMapGrid1f_ = nullptr;
		glMapGrid2d_ = nullptr;
		glMapGrid2f_ = nullptr;
		glMaterialf_ = nullptr;
		glMaterialfv_ = nullptr;
		glMateriali_ = nullptr;
		glMaterialiv_ = nullptr;
		glMatrixMode_ = nullptr;
		glMultMatrixd_ = nullptr;
		glMultMatrixf_ = nullptr;
		glNewList_ = nullptr;
		glNormal3b_ = nullptr;
		glNormal3bv_ = nullptr;
		glNormal3d_ = nullptr;
		glNormal3dv_ = nullptr;
		glNormal3f_ = nullptr;
		glNormal3fv_ = nullptr;
		glNormal3i_ = nullptr;
		glNormal3iv_ = nullptr;
		glNormal3s_ = nullptr;
		glNormal3sv_ = nullptr;
		glOrtho_ = nullptr;
		glPassThrough_ = nullptr;
		glPixelMapfv_ = nullptr;
		glPixelMapuiv_ = nullptr;
		glPixelMapusv_ = nullptr;
		glPixelStoref_ = nullptr;
		glPixelStorei_ = nullptr;
		glPixelTransferf_ = nullptr;
		glPixelTransferi_ = nullptr;
		glPixelZoom_ = nullptr;
		glPointSize_ = nullptr;
		glPolygonMode_ = nullptr;
		glPolygonStipple_ = nullptr;
		glPopAttrib_ = nullptr;
		glPopMatrix_ = nullptr;
		glPopName_ = nullptr;
		glPushAttrib_ = nullptr;
		glPushMatrix_ = nullptr;
		glPushName_ = nullptr;
		glRasterPos2d_ = nullptr;
		glRasterPos2dv_ = nullptr;
		glRasterPos2f_ = nullptr;
		glRasterPos2fv_ = nullptr;
		glRasterPos2i_ = nullptr;
		glRasterPos2iv_ = nullptr;
		glRasterPos2s_ = nullptr;
		glRasterPos2sv_ = nullptr;
		glRasterPos3d_ = nullptr;
		glRasterPos3dv_ = nullptr;
		glRasterPos3f_ = nullptr;
		glRasterPos3fv_ = nullptr;
		glRasterPos3i_ = nullptr;
		glRasterPos3iv_ = nullptr;
		glRasterPos3s_ = nullptr;
		glRasterPos3sv_ = nullptr;
		glRasterPos4d_ = nullptr;
		glRasterPos4dv_ = nullptr;
		glRasterPos4f_ = nullptr;
		glRasterPos4fv_ = nullptr;
		glRasterPos4i_ = nullptr;
		glRasterPos4iv_ = nullptr;
		glRasterPos4s_ = nullptr;
		glRasterPos4sv_ = nullptr;
		glReadBuffer_ = nullptr;
		glReadPixels_ = nullptr;
		glRectd_ = nullptr;
		glRectdv_ = nullptr;
		glRectf_ = nullptr;
		glRectfv_ = nullptr;
		glRecti_ = nullptr;
		glRectiv_ = nullptr;
		glRects_ = nullptr;
		glRectsv_ = nullptr;
		glRenderMode_ = nullptr;
		glRotated_ = nullptr;
		glRotatef_ = nullptr;
		glScaled_ = nullptr;
		glScalef_ = nullptr;
		glScissor_ = nullptr;
		glSelectBuffer_ = nullptr;
		glShadeModel_ = nullptr;
		glStencilFunc_ = nullptr;
		glStencilMask_ = nullptr;
		glStencilOp_ = nullptr;
		glTexCoord1d_ = nullptr;
		glTexCoord1dv_ = nullptr;
		glTexCoord1f_ = nullptr;
		glTexCoord1fv_ = nullptr;
		glTexCoord1i_ = nullptr;
		glTexCoord1iv_ = nullptr;
		glTexCoord1s_ = nullptr;
		glTexCoord1sv_ = nullptr;
		glTexCoord2d_ = nullptr;
		glTexCoord2dv_ = nullptr;
		glTexCoord2f_ = nullptr;
		glTexCoord2fv_ = nullptr;
		glTexCoord2i_ = nullptr;
		glTexCoord2iv_ = nullptr;
		glTexCoord2s_ = nullptr;
		glTexCoord2sv_ = nullptr;
		glTexCoord3d_ = nullptr;
		glTexCoord3dv_ = nullptr;
		glTexCoord3f_ = nullptr;
		glTexCoord3fv_ = nullptr;
		glTexCoord3i_ = nullptr;
		glTexCoord3iv_ = nullptr;
		glTexCoord3s_ = nullptr;
		glTexCoord3sv_ = nullptr;
		glTexCoord4d_ = nullptr;
		glTexCoord4dv_ = nullptr;
		glTexCoord4f_ = nullptr;
		glTexCoord4fv_ = nullptr;
		glTexCoord4i_ = nullptr;
		glTexCoord4iv_ = nullptr;
		glTexCoord4s_ = nullptr;
		glTexCoord4sv_ = nullptr;
		glTexEnvf_ = nullptr;
		glTexEnvfv_ = nullptr;
		glTexEnvi_ = nullptr;
		glTexEnviv_ = nullptr;
		glTexGend_ = nullptr;
		glTexGendv_ = nullptr;
		glTexGenf_ = nullptr;
		glTexGenfv_ = nullptr;
		glTexGeni_ = nullptr;
		glTexGeniv_ = nullptr;
		glTexImage1D_ = nullptr;
		glTexImage2D_ = nullptr;
		glTexParameterf_ = nullptr;
		glTexParameterfv_ = nullptr;
		glTexParameteri_ = nullptr;
		glTexParameteriv_ = nullptr;
		glTranslated_ = nullptr;
		glTranslatef_ = nullptr;
		glVertex2d_ = nullptr;
		glVertex2dv_ = nullptr;
		glVertex2f_ = nullptr;
		glVertex2fv_ = nullptr;
		glVertex2i_ = nullptr;
		glVertex2iv_ = nullptr;
		glVertex2s_ = nullptr;
		glVertex2sv_ = nullptr;
		glVertex3d_ = nullptr;
		glVertex3dv_ = nullptr;
		glVertex3f_ = nullptr;
		glVertex3fv_ = nullptr;
		glVertex3i_ = nullptr;
		glVertex3iv_ = nullptr;
		glVertex3s_ = nullptr;
		glVertex3sv_ = nullptr;
		glVertex4d_ = nullptr;
		glVertex4dv_ = nullptr;
		glVertex4f_ = nullptr;
		glVertex4fv_ = nullptr;
		glVertex4i_ = nullptr;
		glVertex4iv_ = nullptr;
		glVertex4s_ = nullptr;
		glVertex4sv_ = nullptr;
		glViewport_ = nullptr;
	}

	static bool initialize_v1_1()
	{
		resolve_symbol("glAreTexturesResident", glAreTexturesResident_);
		resolve_symbol("glArrayElement", glArrayElement_);
		resolve_symbol("glBindTexture", glBindTexture_);
		resolve_symbol("glColorPointer", glColorPointer_);
		resolve_symbol("glCopyTexImage1D", glCopyTexImage1D_);
		resolve_symbol("glCopyTexImage2D", glCopyTexImage2D_);
		resolve_symbol("glCopyTexSubImage1D", glCopyTexSubImage1D_);
		resolve_symbol("glCopyTexSubImage2D", glCopyTexSubImage2D_);
		resolve_symbol("glDeleteTextures", glDeleteTextures_);
		resolve_symbol("glDisableClientState", glDisableClientState_);
		resolve_symbol("glDrawArrays", glDrawArrays_);
		resolve_symbol("glDrawElements", glDrawElements_);
		resolve_symbol("glEdgeFlagPointer", glEdgeFlagPointer_);
		resolve_symbol("glEnableClientState", glEnableClientState_);
		resolve_symbol("glGenTextures", glGenTextures_);
		resolve_symbol("glGetPointerv", glGetPointerv_);
		resolve_symbol("glIndexPointer", glIndexPointer_);
		resolve_symbol("glIndexub", glIndexub_);
		resolve_symbol("glIndexubv", glIndexubv_);
		resolve_symbol("glInterleavedArrays", glInterleavedArrays_);
		resolve_symbol("glIsTexture", glIsTexture_);
		resolve_symbol("glNormalPointer", glNormalPointer_);
		resolve_symbol("glPolygonOffset", glPolygonOffset_);
		resolve_symbol("glPopClientAttrib", glPopClientAttrib_);
		resolve_symbol("glPrioritizeTextures", glPrioritizeTextures_);
		resolve_symbol("glPushClientAttrib", glPushClientAttrib_);
		resolve_symbol("glTexCoordPointer", glTexCoordPointer_);
		resolve_symbol("glTexSubImage1D", glTexSubImage1D_);
		resolve_symbol("glTexSubImage2D", glTexSubImage2D_);
		resolve_symbol("glVertexPointer", glVertexPointer_);

		return
			glAreTexturesResident_ &&
			glArrayElement_ &&
			glBindTexture_ &&
			glColorPointer_ &&
			glCopyTexImage1D_ &&
			glCopyTexImage2D_ &&
			glCopyTexSubImage1D_ &&
			glCopyTexSubImage2D_ &&
			glDeleteTextures_ &&
			glDisableClientState_ &&
			glDrawArrays_ &&
			glDrawElements_ &&
			glEdgeFlagPointer_ &&
			glEnableClientState_ &&
			glGenTextures_ &&
			glGetPointerv_ &&
			glIndexPointer_ &&
			glIndexub_ &&
			glIndexubv_ &&
			glInterleavedArrays_ &&
			glIsTexture_ &&
			glNormalPointer_ &&
			glPolygonOffset_ &&
			glPopClientAttrib_ &&
			glPrioritizeTextures_ &&
			glPushClientAttrib_ &&
			glTexCoordPointer_ &&
			glTexSubImage1D_ &&
			glTexSubImage2D_ &&
			glVertexPointer_ &&
			true;
	}

	static void uninitialize_v1_1()
	{
		glAreTexturesResident_ = nullptr;
		glArrayElement_ = nullptr;
		glBindTexture_ = nullptr;
		glColorPointer_ = nullptr;
		glCopyTexImage1D_ = nullptr;
		glCopyTexImage2D_ = nullptr;
		glCopyTexSubImage1D_ = nullptr;
		glCopyTexSubImage2D_ = nullptr;
		glDeleteTextures_ = nullptr;
		glDisableClientState_ = nullptr;
		glDrawArrays_ = nullptr;
		glDrawElements_ = nullptr;
		glEdgeFlagPointer_ = nullptr;
		glEnableClientState_ = nullptr;
		glGenTextures_ = nullptr;
		glGetPointerv_ = nullptr;
		glIndexPointer_ = nullptr;
		glIndexub_ = nullptr;
		glIndexubv_ = nullptr;
		glInterleavedArrays_ = nullptr;
		glIsTexture_ = nullptr;
		glNormalPointer_ = nullptr;
		glPolygonOffset_ = nullptr;
		glPopClientAttrib_ = nullptr;
		glPrioritizeTextures_ = nullptr;
		glPushClientAttrib_ = nullptr;
		glTexCoordPointer_ = nullptr;
		glTexSubImage1D_ = nullptr;
		glTexSubImage2D_ = nullptr;
		glVertexPointer_ = nullptr;
	}
}; // SdlOgl11Loader::Detail


std::string SdlOgl11Loader::Detail::error_message_;


bool SdlOgl11Loader::initialize()
{
	Detail::error_message_.clear();

	uninitialize();

	auto sdl_result = 0;

	if (!::SDL_WasInit(SDL_INIT_VIDEO))
	{
		sdl_result = ::SDL_InitSubSystem(SDL_INIT_VIDEO);

		if (sdl_result != 0)
		{
			Detail::error_message_ = "Failed to initialize SDL video subsystem. ";
			Detail::error_message_ += ::SDL_GetError();

			return false;
		}
	}

	sdl_result = ::SDL_GL_LoadLibrary(nullptr);

	if (sdl_result != 0)
	{
		Detail::error_message_ = "Failed to load system OpenGL library. ";
		Detail::error_message_ += ::SDL_GetError();

		return false;
	}

	if (!Detail::initialize_v1_0())
	{
		Detail::error_message_ = "Failed to resolve OpenGL v1.0 functions.";

		uninitialize();
		return false;
	}

	if (!Detail::initialize_v1_1())
	{
		Detail::error_message_ = "Failed to resolve OpenGL v1.1 functions.";

		uninitialize();
		return false;
	}

	return true;
}

void SdlOgl11Loader::uninitialize()
{
	Detail::uninitialize_v1_0();
	Detail::uninitialize_v1_1();

	::SDL_GL_UnloadLibrary();
}

const std::string& SdlOgl11Loader::get_error_message()
{
	return Detail::error_message_;
}
