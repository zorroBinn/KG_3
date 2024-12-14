#define _USE_MATH_DEFINES
#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include <iostream>

// ��������� �����
const float radius = 1.0f; // ������ �����
const int numPoints = 100; // ���������� ����� ��� ������� ���������

// ������
float cameraX = 0.0f, cameraY = 0.0f, cameraZ = 5.0f;
float cameraAngleHorizontal = 0.0f;
float cameraAngleVertical = 0.0f;
float cameraSpeed = 0.15f;
float lookSpeed = 0.04f;

// ��������� ��� �������� ������ �������
struct Vertex {
    float x, y, z;       // ���������� �������
    float nx, ny, nz;    // ������� �������
    float intensity;     // ������������� ��������� � �������
};

std::vector<Vertex> vertices; // ������ ���� ������
std::vector<unsigned int> indices; // ������� ��� ������������

// ��������������� ������� ��� ������� �������
void calculateNormal(float x, float y, float z, float& nx, float& ny, float& nz) {
    float length = sqrt(x * x + y * y + z * z);
    nx = x / length;
    ny = y / length;
    nz = z / length;
}

// ������� ��� ������� ������������� ����� (�������� ������)
float calculateLightIntensity(float nx, float ny, float nz) {
    float lightDir[] = { 0.0f, 0.0f, 1.0f }; // ����������� �����
    float lightLength = sqrt(lightDir[0] * lightDir[0] + lightDir[1] * lightDir[1] + lightDir[2] * lightDir[2]);
    float normalizedLightDir[] = { lightDir[0] / lightLength, lightDir[1] / lightLength, lightDir[2] / lightLength };

    // ��������� ������������ ������� � ����������� �����
    float dotProduct = nx * normalizedLightDir[0] + ny * normalizedLightDir[1] + nz * normalizedLightDir[2];
    return fmax(0.0f, dotProduct); // ������������� �� 0 �� 1
}

// ���������� ����� � �������������� ������� ���������
void buildSphere() {
    const float goldenRatio = (1.0f + sqrt(5.0f)) / 2.0f;
    const float angleIncrement = 2.0f * M_PI / goldenRatio;

    for (int i = 0; i < numPoints; ++i) {
        float t = static_cast<float>(i) / (numPoints - 1); // ��������������� ��������
        float z = 1.0f - 2.0f * t; // Z-���������� �� 1 �� -1
        float r = sqrt(1.0f - z * z); // ������ �� ������ ������
        float theta = angleIncrement * i; // ���� �� �������

        float x = r * cos(theta);
        float y = r * sin(theta);

        // ������������ ������� � ������������� �����
        float nx, ny, nz;
        calculateNormal(x, y, z, nx, ny, nz);
        float intensity = calculateLightIntensity(nx, ny, nz);

        // ��������� ������� � ������
        vertices.push_back({ x, y, z, nx, ny, nz, intensity });
    }

    // �������� �������� ��� ������������ �������
    for (int i = 0; i < numPoints - 1; ++i) {
        indices.push_back(i);
        indices.push_back(i + 1);
        indices.push_back((i + 2) % numPoints); // ��������� ������������
    }
}

// ��������� ����� � �������������� �������� ����
void renderSphere() {
    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < indices.size(); i += 3) {
        for (int j = 0; j < 3; ++j) {
            const Vertex& v = vertices[indices[i + j]];
            glColor3f(v.intensity, v.intensity, v.intensity); // ������������� ���� �������
            glNormal3f(v.nx, v.ny, v.nz);
            glVertex3f(v.x, v.y, v.z);
        }
    }
    glEnd();
}

// �������-�������
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // ������� ������
    glLoadIdentity(); // ����� �������

    // ��������� ������
    gluLookAt(cameraX, cameraY, cameraZ,
        cameraX + sin(cameraAngleHorizontal),
        cameraY + tan(cameraAngleVertical),
        cameraZ - cos(cameraAngleHorizontal),
        0.0f, 1.0f, 0.0f);

    renderSphere(); // ������ �����

    glutSwapBuffers(); // ��������� ����� ������
}

// ��������� ������
void keyboard(unsigned char key, int x, int y) {
    float nextX = cameraX, nextY = cameraY, nextZ = cameraZ;
    bool reset = false;
    switch (key) {
    case 'w': // ������
        nextX += cameraSpeed * sin(cameraAngleHorizontal);
        nextZ -= cameraSpeed * cos(cameraAngleHorizontal);
        break;
    case 's': // �����
        nextX -= cameraSpeed * sin(cameraAngleHorizontal);
        nextZ += cameraSpeed * cos(cameraAngleHorizontal);
        break;
    case 'a': // �����
        nextX -= cameraSpeed * cos(cameraAngleHorizontal);
        nextZ -= cameraSpeed * sin(cameraAngleHorizontal);
        break;
    case 'd': // ������
        nextX += cameraSpeed * cos(cameraAngleHorizontal);
        nextZ += cameraSpeed * sin(cameraAngleHorizontal);
        break;
    case 'q': // ������� �����
        cameraAngleHorizontal -= lookSpeed;
        break;
    case 'e': // ������� ������
        cameraAngleHorizontal += lookSpeed;
        break;
    case 'r': // ����� ������
        cameraX = 0.0f, cameraY = 2.0f, cameraZ = 8.0f;
        cameraAngleHorizontal = 0.0f; cameraAngleVertical = 0.0f;
        reset = true;
        break;
    case 27: // �����
        exit(0);
    }

    if (!reset) {
        cameraX = nextX;
        cameraZ = nextZ;
    }
    glutPostRedisplay();
}

// ��������� �������
void keyboardArrows(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP: // ������� ������
        if (cameraY + cameraSpeed <= 4.0f) cameraY += cameraSpeed;
        break;
    case GLUT_KEY_DOWN: // �������� ������
        if (cameraY - cameraSpeed >= 0.0f) cameraY -= cameraSpeed;
        break;
    case GLUT_KEY_LEFT: // ������ �����
        if (cameraAngleVertical + lookSpeed <= M_PI_2) cameraAngleVertical += lookSpeed;
        break;
    case GLUT_KEY_RIGHT: // ������ ����
        if (cameraAngleVertical - lookSpeed >= -M_PI_2) cameraAngleVertical -= lookSpeed;
        break;
    }
    glutPostRedisplay();
}

// ��������� ������� ����
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

// ��������� OpenGL
void initGL() {
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // ������ ���
    buildSphere();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Sphere");

    initGL();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(keyboardArrows);

    glutMainLoop();
    return 0;
}