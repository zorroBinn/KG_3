#define _USE_MATH_DEFINES
#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include <queue>
#include <tuple>
#include <iostream>

const float radius = 1.0f; //Радиус сферы
const int N = 10; //Кол-во разбиений
const bool isGuro = 1; //Делать ли закраску Гуро

//Камера
float cameraX = 0.0f, cameraY = 0.0f, cameraZ = 5.0f;
float cameraAngleHorizontal = 0.0f;
float cameraAngleVertical = 0.0f;
float cameraSpeed = 0.15f;
float lookSpeed = 0.04f;

//Структура для хранения данных вершины
struct Vertex {
    float x, y, z; //Координаты вершины
    float nx, ny, nz; //Нормали вершины
    float intensity; //Интенсивность освещения в вершине
};

std::vector<Vertex> vertices; //Список всех вершин
std::vector<unsigned int> indices; //Индексы для триангуляции

//Функция для расчёта нормали
void calculateNormal(float x, float y, float z, float& nx, float& ny, float& nz) {
    float length = sqrt(x * x + y * y + z * z);
    nx = x / length;
    ny = y / length;
    nz = z / length;
}

//Функция для расчёта интенсивности света (градации серого)
float calculateLightIntensity(float nx, float ny, float nz) {
    float lightDir[] = { 0.0f, 0.0f, 1.0f }; //Направление света
    float lightLength = sqrt(lightDir[0] * lightDir[0] + lightDir[1] * lightDir[1] + lightDir[2] * lightDir[2]);
    float normalizedLightDir[] = { lightDir[0] / lightLength, lightDir[1] / lightLength, lightDir[2] / lightLength };

    //Скалярное произведение нормали и направления света
    float dotProduct = nx * normalizedLightDir[0] + ny * normalizedLightDir[1] + nz * normalizedLightDir[2];
    return fmax(0.0f, dotProduct); //Интенсивность от 0 до 1
}

//Построение сферы с триангуляцией и рассчётом освещения каждого участка триангулирования
void buildSphere() {
    vertices.clear();
    indices.clear();

    const int stacks = N; //Количество горизонтальных делений (широта)
    const int slices = N; //Количество вертикальных делений (долгота)

    for (int i = 0; i <= stacks; ++i) {
        float phi = M_PI * i / stacks; //Угол широты [0, pi]
        for (int j = 0; j <= slices; ++j) {
            float theta = 2 * M_PI * j / slices; //Угол долготы [0, 2*pi]

            //Вычисление позиции вершины
            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi);
            float z = radius * sin(phi) * sin(theta);

            //Вычисление нормали
            float nx, ny, nz;
            calculateNormal(x, y, z, nx, ny, nz);

            //Интенсивность освещения
            float intensity = calculateLightIntensity(nx, ny, nz);

            //Добавление вершины
            vertices.push_back({ x, y, z, nx, ny, nz, intensity });
        }
    }

    //Создание индексов для триангуляции
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int first = i * (slices + 1) + j;
            int second = first + slices + 1;

            //Треугольник 1
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            //Треугольник 2
            indices.push_back(first + 1);
            indices.push_back(second);
            indices.push_back(second + 1);
        }
    }
}

//Рендеринг сферы с использованием закраски Гуро
void renderSphere() {
    glBegin(GL_TRIANGLES);
    if (isGuro)
    {
        for (size_t i = 0; i < indices.size(); i += 3) { //Для Гуро
            for (int j = 0; j < 3; ++j) {
                const Vertex& v = vertices[indices[i + j]];
                glColor3f(v.intensity, v.intensity, v.intensity); //Устанавливаем цвет вершины
                glNormal3f(v.nx, v.ny, v.nz); //Устанавливаем нормаль вершины
                glVertex3f(v.x, v.y, v.z); //Устанавливаем координаты вершины
            }
        }
    }
    else
    {
        for (size_t i = 0; i < indices.size(); i += 3) { //Без Гуро
            const Vertex& v0 = vertices[indices[i]]; //Берём первую вершину треугольника для расчёта единого цвета
            glColor3f(v0.intensity, v0.intensity, v0.intensity); //Используем цвет только одной вершины для всего треугольника
            for (int j = 0; j < 3; ++j) {
                const Vertex& v = vertices[indices[i + j]];
                glNormal3f(v.nx, v.ny, v.nz);
                glVertex3f(v.x, v.y, v.z);
            }
        }
    }
    glEnd();
}

//Дисплей-функция
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Очистка экрана
    glLoadIdentity(); //Сброс матрицы
    //Установка камеры
    gluLookAt(cameraX, cameraY, cameraZ, cameraX + sin(cameraAngleHorizontal), cameraY + tan(cameraAngleVertical), cameraZ - cos(cameraAngleHorizontal), 0.0f, 1.0f, 0.0f);
    renderSphere(); //Рисуем сферу
    glutSwapBuffers(); //Обновляем буфер экрана
}

//Обработка клавиш
void keyboard(unsigned char key, int x, int y) {
    float nextX = cameraX, nextY = cameraY, nextZ = cameraZ;
    bool reset = false;
    switch (key) {
    case 'w': //Вперед
        nextX += cameraSpeed * sin(cameraAngleHorizontal);
        nextZ -= cameraSpeed * cos(cameraAngleHorizontal);
        break;
    case 's': //Назад
        nextX -= cameraSpeed * sin(cameraAngleHorizontal);
        nextZ += cameraSpeed * cos(cameraAngleHorizontal);
        break;
    case 'a': //Влево
        nextX -= cameraSpeed * cos(cameraAngleHorizontal);
        nextZ -= cameraSpeed * sin(cameraAngleHorizontal);
        break;
    case 'd': //Вправо
        nextX += cameraSpeed * cos(cameraAngleHorizontal);
        nextZ += cameraSpeed * sin(cameraAngleHorizontal);
        break;
    case 'q': //Поворот влево
        cameraAngleHorizontal -= lookSpeed;
        break;
    case 'e': //Поворот вправо
        cameraAngleHorizontal += lookSpeed;
        break;
    case 'r': //Сброс камеры
        cameraX = 0.0f, cameraY = 0.0f, cameraZ = 5.0f;
        cameraAngleHorizontal = 0.0f; cameraAngleVertical = 0.0f;
        reset = true;
        break;
    case 27: //Выход
        exit(0);
    }

    if (!reset) {
        cameraX = nextX;
        cameraZ = nextZ;
    }
    glutPostRedisplay();
}

//Обработка стрелок
void keyboardArrows(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP: //Поднять камеру
        if (cameraY + cameraSpeed <= 4.0f) cameraY += cameraSpeed;
        break;
    case GLUT_KEY_DOWN: //Опустить камеру
        if (cameraY - cameraSpeed >= 0.0f) cameraY -= cameraSpeed;
        break;
    case GLUT_KEY_LEFT: //Взгляд вверх
        if (cameraAngleVertical + lookSpeed <= M_PI_2) cameraAngleVertical += lookSpeed;
        break;
    case GLUT_KEY_RIGHT: //Взгляд вниз
        if (cameraAngleVertical - lookSpeed >= -M_PI_2) cameraAngleVertical -= lookSpeed;
        break;
    }
    glutPostRedisplay();
}

//Изменение размера окна
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

//Настройка OpenGL
void initGL() {
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //Черный фон
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