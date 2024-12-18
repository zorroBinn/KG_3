#define _USE_MATH_DEFINES
#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include <queue>
#include <tuple>
#include <iostream>

const float radius = 1.0f; //������ �����
const int N = 10; //���-�� ���������
const bool isGuro = 1; //������ �� �������� ����

//������
float cameraX = 0.0f, cameraY = 0.0f, cameraZ = 5.0f;
float cameraAngleHorizontal = 0.0f;
float cameraAngleVertical = 0.0f;
float cameraSpeed = 0.15f;
float lookSpeed = 0.04f;

//��������� ��� �������� ������ �������
struct Vertex {
    float x, y, z; //���������� �������
    float nx, ny, nz; //������� �������
    float intensity; //������������� ��������� � �������
};

std::vector<Vertex> vertices; //������ ���� ������
std::vector<unsigned int> indices; //������� ��� ������������

//������� ��� ������� �������
void calculateNormal(float x, float y, float z, float& nx, float& ny, float& nz) {
    float length = sqrt(x * x + y * y + z * z);
    nx = x / length;
    ny = y / length;
    nz = z / length;
}

//������� ��� ������� ������������� ����� (�������� ������)
float calculateLightIntensity(float nx, float ny, float nz) {
    float lightDir[] = { 0.0f, 0.0f, 1.0f }; //����������� �����
    float lightLength = sqrt(lightDir[0] * lightDir[0] + lightDir[1] * lightDir[1] + lightDir[2] * lightDir[2]);
    float normalizedLightDir[] = { lightDir[0] / lightLength, lightDir[1] / lightLength, lightDir[2] / lightLength };

    //��������� ������������ ������� � ����������� �����
    float dotProduct = nx * normalizedLightDir[0] + ny * normalizedLightDir[1] + nz * normalizedLightDir[2];
    return fmax(0.0f, dotProduct); //������������� �� 0 �� 1
}

//���������� ����� � ������������� � ��������� ��������� ������� ������� ����������������
void buildSphere() {
    vertices.clear();
    indices.clear();

    const int stacks = N; //���������� �������������� ������� (������)
    const int slices = N; //���������� ������������ ������� (�������)

    for (int i = 0; i <= stacks; ++i) {
        float phi = M_PI * i / stacks; //���� ������ [0, pi]
        for (int j = 0; j <= slices; ++j) {
            float theta = 2 * M_PI * j / slices; //���� ������� [0, 2*pi]

            //���������� ������� �������
            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi);
            float z = radius * sin(phi) * sin(theta);

            //���������� �������
            float nx, ny, nz;
            calculateNormal(x, y, z, nx, ny, nz);

            //������������� ���������
            float intensity = calculateLightIntensity(nx, ny, nz);

            //���������� �������
            vertices.push_back({ x, y, z, nx, ny, nz, intensity });
        }
    }

    //�������� �������� ��� ������������
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int first = i * (slices + 1) + j;
            int second = first + slices + 1;

            //����������� 1
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            //����������� 2
            indices.push_back(first + 1);
            indices.push_back(second);
            indices.push_back(second + 1);
        }
    }
}

//��������� ����� � �������������� �������� ����
void renderSphere() {
    glBegin(GL_TRIANGLES);
    if (isGuro)
    {
        for (size_t i = 0; i < indices.size(); i += 3) { //��� ����
            for (int j = 0; j < 3; ++j) {
                const Vertex& v = vertices[indices[i + j]];
                glColor3f(v.intensity, v.intensity, v.intensity); //������������� ���� �������
                glNormal3f(v.nx, v.ny, v.nz); //������������� ������� �������
                glVertex3f(v.x, v.y, v.z); //������������� ���������� �������
            }
        }
    }
    else
    {
        for (size_t i = 0; i < indices.size(); i += 3) { //��� ����
            const Vertex& v0 = vertices[indices[i]]; //���� ������ ������� ������������ ��� ������� ������� �����
            glColor3f(v0.intensity, v0.intensity, v0.intensity); //���������� ���� ������ ����� ������� ��� ����� ������������
            for (int j = 0; j < 3; ++j) {
                const Vertex& v = vertices[indices[i + j]];
                glNormal3f(v.nx, v.ny, v.nz);
                glVertex3f(v.x, v.y, v.z);
            }
        }
    }
    glEnd();
}

//�������-�������
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //������� ������
    glLoadIdentity(); //����� �������
    //��������� ������
    gluLookAt(cameraX, cameraY, cameraZ, cameraX + sin(cameraAngleHorizontal), cameraY + tan(cameraAngleVertical), cameraZ - cos(cameraAngleHorizontal), 0.0f, 1.0f, 0.0f);
    renderSphere(); //������ �����
    glutSwapBuffers(); //��������� ����� ������
}

//��������� ������
void keyboard(unsigned char key, int x, int y) {
    float nextX = cameraX, nextY = cameraY, nextZ = cameraZ;
    bool reset = false;
    switch (key) {
    case 'w': //������
        nextX += cameraSpeed * sin(cameraAngleHorizontal);
        nextZ -= cameraSpeed * cos(cameraAngleHorizontal);
        break;
    case 's': //�����
        nextX -= cameraSpeed * sin(cameraAngleHorizontal);
        nextZ += cameraSpeed * cos(cameraAngleHorizontal);
        break;
    case 'a': //�����
        nextX -= cameraSpeed * cos(cameraAngleHorizontal);
        nextZ -= cameraSpeed * sin(cameraAngleHorizontal);
        break;
    case 'd': //������
        nextX += cameraSpeed * cos(cameraAngleHorizontal);
        nextZ += cameraSpeed * sin(cameraAngleHorizontal);
        break;
    case 'q': //������� �����
        cameraAngleHorizontal -= lookSpeed;
        break;
    case 'e': //������� ������
        cameraAngleHorizontal += lookSpeed;
        break;
    case 'r': //����� ������
        cameraX = 0.0f, cameraY = 0.0f, cameraZ = 5.0f;
        cameraAngleHorizontal = 0.0f; cameraAngleVertical = 0.0f;
        reset = true;
        break;
    case 27: //�����
        exit(0);
    }

    if (!reset) {
        cameraX = nextX;
        cameraZ = nextZ;
    }
    glutPostRedisplay();
}

//��������� �������
void keyboardArrows(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP: //������� ������
        if (cameraY + cameraSpeed <= 4.0f) cameraY += cameraSpeed;
        break;
    case GLUT_KEY_DOWN: //�������� ������
        if (cameraY - cameraSpeed >= 0.0f) cameraY -= cameraSpeed;
        break;
    case GLUT_KEY_LEFT: //������ �����
        if (cameraAngleVertical + lookSpeed <= M_PI_2) cameraAngleVertical += lookSpeed;
        break;
    case GLUT_KEY_RIGHT: //������ ����
        if (cameraAngleVertical - lookSpeed >= -M_PI_2) cameraAngleVertical -= lookSpeed;
        break;
    }
    glutPostRedisplay();
}

//��������� ������� ����
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

//��������� OpenGL
void initGL() {
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //������ ���
    buildSphere();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("KG_3");
    initGL();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(keyboardArrows);
    glutMainLoop();
    return 0;
}