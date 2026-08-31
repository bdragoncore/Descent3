/*
 * D3RenderTest - Basic 3D rendering tests (quads, cube, framebuffer).
 * Linked into d3_render_tests executable.
 */

#include "render_test_base.h"
#include "renderer.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <gtest/gtest.h>
#include <cmath>
#include <cstdlib>

class D3RenderTest : public D3RenderTestBase {
protected:
    D3RenderTest() : D3RenderTestBase(256, 256) {}

    void setup3DView(float zoom = 1.0f) {
        rend_SetZBufferState(1);
        rend_SetLighting(LS_GOURAUD);
        rend_SetAlphaType(AT_ALWAYS);
        rend_SetAlphaValue(255);
        rend_SetColorModel(CM_RGB);

        glDisable(GL_BLEND);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glShadeModel(GL_SMOOTH);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-1.0 * zoom, 1.0 * zoom, -1.0 * zoom, 1.0 * zoom, -10.0, 10.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        // Move view back so cube at origin is in front of the camera (ortho looks down -Z).
        glTranslatef(0.0f, 0.0f, -2.0f);
    }

    void drawQuad3D(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4,
                    float y4, float z4, float r, float g, float b) {
        glColor3f(r, g, b);
        glBegin(GL_QUADS);
        glVertex3f(x1, y1, z1);
        glVertex3f(x2, y2, z2);
        glVertex3f(x3, y3, z3);
        glVertex3f(x4, y4, z4);
        glEnd();
    }
};

TEST_F(D3RenderTest, RenderSingleColoredQuad) {
    BeginFrame();
    setup3DView(2.0f);

    drawQuad3D(-0.8f, -0.8f, -1.0f, 0.0f, -0.8f, -1.0f, 0.0f, 0.0f, -1.0f, -0.8f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f);

    glFlush();
    glFinish();

    ReadPixels();
    SavePNG("SingleColoredQuad");
}

TEST_F(D3RenderTest, RenderMultipleQuadsDepthTest) {
    BeginFrame();
    setup3DView(2.0f);

    drawQuad3D(-0.8f, -0.8f, -1.0f, 0.0f, -0.8f, -1.0f, 0.0f, 0.0f, -1.0f, -0.8f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f);
    drawQuad3D(-0.2f, -0.2f, 0.0f, 0.6f, -0.2f, 0.0f, 0.6f, 0.6f, 0.0f, -0.2f, 0.6f, 0.0f, 0.0f, 1.0f, 0.0f);
    drawQuad3D(0.0f, 0.0f, 1.0f, 0.8f, 0.0f, 1.0f, 0.8f, 0.8f, 1.0f, 0.0f, 0.8f, 1.0f, 0.0f, 0.0f, 1.0f);

    glFlush();
    glFinish();

    ReadPixels();
    SavePNG("MultipleQuadsDepth");
}

TEST_F(D3RenderTest, RenderRotatingCubeDeterministic) {
    BeginFrame();
    setup3DView(1.5f);

    srand(42);
    float rotX = (rand() % 60) * (M_PI / 180.0f);
    float rotY = (rand() % 60) * (M_PI / 180.0f);
    float rotZ = (rand() % 60) * (M_PI / 180.0f);

    glRotatef(rotX * 180.0f / M_PI, 1.0f, 0.0f, 0.0f);
    glRotatef(rotY * 180.0f / M_PI, 0.0f, 1.0f, 0.0f);
    glRotatef(rotZ * 180.0f / M_PI, 0.0f, 0.0f, 1.0f);

    float faceColor[6][3] = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f},
                             {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f}};

    drawQuad3D(-0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, faceColor[0][0], faceColor[0][1], faceColor[0][2]);
    drawQuad3D(-0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, faceColor[1][0], faceColor[1][1], faceColor[1][2]);
    drawQuad3D(-0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, faceColor[2][0], faceColor[2][1], faceColor[2][2]);
    drawQuad3D(-0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, faceColor[3][0], faceColor[3][1], faceColor[3][2]);
    drawQuad3D(0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, faceColor[4][0], faceColor[4][1], faceColor[4][2]);
    drawQuad3D(-0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, faceColor[5][0], faceColor[5][1], faceColor[5][2]);

    glFlush();
    glFinish();

    ReadPixels();
    SavePNG("RotatingCubeDeterministic");
}

TEST_F(D3RenderTest, FramebufferConsistency) {
    BeginFrame();
    setup3DView(1.5f);

    glRotatef(30.0f, 0.0f, 0.0f, 1.0f);
    drawQuad3D(-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.5f, 0.5f, 0.0f, -0.5f, 0.5f, 0.0f, 1.0f, 0.5f, 0.0f);

    glFlush();
    glFinish();
    ReadPixels();

    BeginFrame();
    setup3DView(1.5f);
    glRotatef(30.0f, 0.0f, 0.0f, 1.0f);
    drawQuad3D(-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.5f, 0.5f, 0.0f, -0.5f, 0.5f, 0.0f, 1.0f, 0.5f, 0.0f);

    glFlush();
    glFinish();
    ReadPixels();
    SavePNG("FramebufferConsistency");
}

TEST_F(D3RenderTest, RenderSameSceneTwice) {
    BeginFrame();
    setup3DView(1.5f);
    glRotatef(45.0f, 0.0f, 0.0f, 1.0f);
    glColor3f(1.0f, 0.5f, 0.0f);
    glBegin(GL_TRIANGLES);
    glVertex3f(0.0f, 0.5f, 0.0f);
    glVertex3f(-0.5f, -0.5f, 0.0f);
    glVertex3f(0.5f, -0.5f, 0.0f);
    glEnd();
    glFlush();
    glFinish();
    ReadPixels();

    BeginFrame();
    setup3DView(1.5f);
    glRotatef(45.0f, 0.0f, 0.0f, 1.0f);
    glColor3f(1.0f, 0.5f, 0.0f);
    glBegin(GL_TRIANGLES);
    glVertex3f(0.0f, 0.5f, 0.0f);
    glVertex3f(-0.5f, -0.5f, 0.0f);
    glVertex3f(0.5f, -0.5f, 0.0f);
    glEnd();
    glFlush();
    glFinish();
    ReadPixels();
    SavePNG("RenderSameSceneTwice");
}
