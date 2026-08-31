/*
 * SoftwareGLTest - Software rendering tests with cube rotations.
 * Linked into d3_render_tests executable.
 */

#include "render_test_base.h"
#include "renderer.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <gtest/gtest.h>

class SoftwareGLTest : public D3RenderTestBase {
protected:
    SoftwareGLTest() : D3RenderTestBase(512, 512) {}

    void drawCube() {
        glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
        glBegin(GL_QUADS);
        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(0.5f, -0.5f, -0.5f);
        glVertex3f(0.5f, 0.5f, -0.5f);
        glVertex3f(-0.5f, 0.5f, -0.5f);
        glEnd();

        glColor3f(0.0f, 1.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex3f(-0.5f, -0.5f, 0.5f);
        glVertex3f(0.5f, -0.5f, 0.5f);
        glVertex3f(0.5f, 0.5f, 0.5f);
        glVertex3f(-0.5f, 0.5f, 0.5f);
        glEnd();

        glColor3f(0.0f, 0.0f, 1.0f);
        glBegin(GL_QUADS);
        glVertex3f(-0.5f, 0.5f, -0.5f);
        glVertex3f(0.5f, 0.5f, -0.5f);
        glVertex3f(0.5f, 0.5f, 0.5f);
        glVertex3f(-0.5f, 0.5f, 0.5f);
        glEnd();

        glColor3f(1.0f, 1.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(0.5f, -0.5f, -0.5f);
        glVertex3f(0.5f, -0.5f, 0.5f);
        glVertex3f(-0.5f, -0.5f, 0.5f);
        glEnd();

        glColor3f(0.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        glVertex3f(0.5f, -0.5f, -0.5f);
        glVertex3f(0.5f, 0.5f, -0.5f);
        glVertex3f(0.5f, 0.5f, 0.5f);
        glVertex3f(0.5f, -0.5f, 0.5f);
        glEnd();

        glColor3f(1.0f, 0.0f, 1.0f);
        glBegin(GL_QUADS);
        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(-0.5f, 0.5f, -0.5f);
        glVertex3f(-0.5f, 0.5f, 0.5f);
        glVertex3f(-0.5f, -0.5f, 0.5f);
        glEnd();
    }

    void setup3DScene() {
        rend_SetZBufferState(1);
        rend_SetLighting(LS_NONE);
        rend_SetAlphaType(AT_ALWAYS);
        rend_SetAlphaValue(255);
        rend_SetColorModel(CM_RGB);

        glDisable(GL_BLEND);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glShadeModel(GL_FLAT);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45.0, 1.0, 0.1, 100.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }
};

TEST_F(SoftwareGLTest, RenderQuadToFramebuffer) {
    BeginFrame();
    setup3DScene();

    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    glEnd();

    glFlush();
    glFinish();

    RenderTestNameOverlay();
    ReadPixels();
    SavePNG("RenderQuadToFramebuffer");
}

TEST_F(SoftwareGLTest, Render3DCube) {
    BeginFrame();
    setup3DScene();
    gluLookAt(0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    drawCube();

    glFlush();
    glFinish();

    RenderTestNameOverlay();
    ReadPixels();
    SavePNG("Render3DCube");
}

TEST_F(SoftwareGLTest, CubeRotationX45) {
    BeginFrame();
    setup3DScene();
    gluLookAt(0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glRotatef(45.0f, 1.0f, 0.0f, 0.0f);

    drawCube();

    glFlush();
    glFinish();

    RenderTestNameOverlay();
    ReadPixels();
    SavePNG("CubeRotationX45");
}

TEST_F(SoftwareGLTest, CubeRotationY45) {
    BeginFrame();
    setup3DScene();
    gluLookAt(0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glRotatef(45.0f, 0.0f, 1.0f, 0.0f);

    drawCube();

    glFlush();
    glFinish();

    RenderTestNameOverlay();
    ReadPixels();
    SavePNG("CubeRotationY45");
}

TEST_F(SoftwareGLTest, CubeRotationXY) {
    BeginFrame();
    setup3DScene();
    gluLookAt(0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glRotatef(30.0f, 1.0f, 1.0f, 0.0f);

    drawCube();

    glFlush();
    glFinish();

    RenderTestNameOverlay();
    ReadPixels();
    SavePNG("CubeRotationXY");
}

TEST_F(SoftwareGLTest, CubeRotationXYZ) {
    BeginFrame();
    setup3DScene();
    gluLookAt(0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glRotatef(25.0f, 1.0f, 1.0f, 1.0f);

    drawCube();

    glFlush();
    glFinish();

    RenderTestNameOverlay();
    ReadPixels();
    SavePNG("CubeRotationXYZ");
}

TEST_F(SoftwareGLTest, CubeEdgeOn) {
    BeginFrame();
    setup3DScene();
    gluLookAt(0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

    drawCube();

    glFlush();
    glFinish();

    RenderTestNameOverlay();
    ReadPixels();
    SavePNG("CubeEdgeOn");
}

TEST_F(SoftwareGLTest, CubeCorner) {
    BeginFrame();
    setup3DScene();
    gluLookAt(0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glRotatef(35.0f, 1.0f, 1.0f, 1.0f);

    drawCube();

    glFlush();
    glFinish();

    RenderTestNameOverlay();
    ReadPixels();
    SavePNG("CubeCorner");
}
