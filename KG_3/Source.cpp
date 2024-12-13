#define _USE_MATH_DEFINES
#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include <iostream>

// Параметры биконуса
const float height = 1.0f; // Высота одной стороны конуса
const float radius = 0.5f; // Радиус основания
const int slices = 9;   // Rоличество делений по окружности
const int stacks = 3;    // Rоличество делений по высоте

// Камера
float cameraX = 0.0f, cameraY = 0.0f, cameraZ = 5.0f;
float cameraAngleHorizontal = 0.0f;
float cameraAngleVertical = 0.0f;
float cameraSpeed = 0.15f;
float lookSpeed = 0.04f;

// Структура для хранения данных вершины
struct Vertex {
    float x, y, z;       // Координаты вершины
    float nx, ny, nz;    // Нормали вершины
    float intensity;     // Интенсивность освещения в вершине
};

std::vector<Vertex> vertices; // Список всех вершин
std::vector<unsigned int> indices; // Индексы для триангуляции

// Вспомогательная функция для расчёта нормали
void calculateNormal(float x, float y, float z, float& nx, float& ny, float& nz) {
    float length = sqrt(x * x + y * y + z * z);
    nx = x / length;
    ny = y / length;
    nz = z / length;
}

// Функция для расчёта интенсивности света (градации серого)
float calculateLightIntensity(float nx, float ny, float nz) {
    float lightDir[] = { 0.0f, 0.0f, 1.0f }; // Направление света от источника (0, 0, 10)
    float lightLength = sqrt(lightDir[0] * lightDir[0] + lightDir[1] * lightDir[1] + lightDir[2] * lightDir[2]);
    float normalizedLightDir[] = { lightDir[0] / lightLength, lightDir[1] / lightLength, lightDir[2] / lightLength };

    // Скалярное произведение нормали и направления света
    float dotProduct = nx * normalizedLightDir[0] + ny * normalizedLightDir[1] + nz * normalizedLightDir[2];
    return fmax(0.0f, dotProduct); // Интенсивность от 0 до 1
}

// Построение биконуса вращения
void buildBicone() {
    // Верхний конус
    for (int i = 0; i <= stacks; ++i) {
        float v = static_cast<float>(i) / stacks; // Нормализованное значение по высоте
        float y = height * (1.0f - v); // Высота уменьшается сверху вниз
        float r = radius * v;         // Радиус увеличивается от вершины

        for (int j = 0; j <= slices; ++j) {
            float u = static_cast<float>(j) / slices; // Нормализованное значение по окружности
            float theta = 2.0f * M_PI * u;            // Угол в радианах
            float x = r * cos(theta);
            float z = r * sin(theta);

            // Рассчитываем нормали и интенсивность света
            float nx, ny, nz;
            calculateNormal(x, y, z, nx, ny, nz);
            float intensity = calculateLightIntensity(nx, ny, nz);

            // Добавляем вершину в список
            vertices.push_back({ x, y, z, nx, ny, nz, intensity });
        }
    }

    // Нижний конус
    for (int i = 0; i <= stacks; ++i) {
        float v = static_cast<float>(i) / stacks; // Нормализованное значение по высоте
        float y = -height * v; // Высота уменьшается в отрицательном направлении
        float r = radius * (1.0f - v); // Радиус уменьшается

        for (int j = 0; j <= slices; ++j) {
            float u = static_cast<float>(j) / slices; // Нормализованное значение по окружности
            float theta = 2.0f * M_PI * u;            // Угол в радианах
            float x = r * cos(theta);
            float z = r * sin(theta);

            // Рассчитываем нормали и интенсивность света
            float nx, ny, nz;
            calculateNormal(x, y, z, nx, ny, nz);
            float intensity = calculateLightIntensity(nx, ny, nz);

            // Добавляем вершину в список
            vertices.push_back({ x, y, z, nx, ny, nz, intensity });
        }
    }

    // Триангуляция верхнего конуса
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int first = i * (slices + 1) + j;       // Индекс текущей вершины
            int second = first + slices + 1;       // Индекс вершины на следующем уровне

            // Два треугольника на квадрат
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    // Триангуляция нижнего конуса
    int offset = (stacks + 1) * (slices + 1); // Смещение для индексов нижнего конуса
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int first = offset + i * (slices + 1) + j;       // Индекс текущей вершины
            int second = first + slices + 1;                // Индекс вершины на следующем уровне

            // Два треугольника на квадрат
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
}

// Рендеринг биконуса с использованием закраски Гуро
void renderBicone() {
    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < indices.size(); i += 3) {
        for (int j = 0; j < 3; ++j) {
            const Vertex& v = vertices[indices[i + j]]; // Получение данных вершины
            glColor3f(v.intensity, v.intensity, v.intensity); // Устанавливаем цвет вершины (градации серого)
            glNormal3f(v.nx, v.ny, v.nz); // Устанавливаем нормаль вершины
            glVertex3f(v.x, v.y, v.z); // Устанавливаем координаты вершины
        }
    }
    glEnd();
}

// Дисплей-функция
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Очистка экрана
    glLoadIdentity(); // Сброс матрицы

    // Установка камеры
    gluLookAt(cameraX, cameraY, cameraZ,
        cameraX + sin(cameraAngleHorizontal),
        cameraY + tan(cameraAngleVertical),
        cameraZ - cos(cameraAngleHorizontal),
        0.0f, 1.0f, 0.0f);

    renderBicone(); // Рисуем биконус

    glutSwapBuffers(); // Обновляем буфер экрана
}

// Обработка клавиш
void keyboard(unsigned char key, int x, int y) {
    float nextX = cameraX, nextY = cameraY, nextZ = cameraZ;
    bool reset = false;
    switch (key) {
    case 'w': // Вперед
        nextX += cameraSpeed * sin(cameraAngleHorizontal);
        nextZ -= cameraSpeed * cos(cameraAngleHorizontal);
        break;
    case 's': // Назад
        nextX -= cameraSpeed * sin(cameraAngleHorizontal);
        nextZ += cameraSpeed * cos(cameraAngleHorizontal);
        break;
    case 'a': // Влево
        nextX -= cameraSpeed * cos(cameraAngleHorizontal);
        nextZ -= cameraSpeed * sin(cameraAngleHorizontal);
        break;
    case 'd': // Вправо
        nextX += cameraSpeed * cos(cameraAngleHorizontal);
        nextZ += cameraSpeed * sin(cameraAngleHorizontal);
        break;
    case 'q': // Поворот влево
        cameraAngleHorizontal -= lookSpeed;
        break;
    case 'e': // Поворот вправо
        cameraAngleHorizontal += lookSpeed;
        break;
    case 'r': // Сброс камеры
        cameraX = 0.0f, cameraY = 2.0f, cameraZ = 8.0f;
        cameraAngleHorizontal = 0.0f; cameraAngleVertical = 0.0f;
        reset = true;
        break;
    case 27: // Выход
        exit(0);
    }

    // Проверяем, не выходит ли камера за пределы
    if (!reset) cameraX = nextX;
    if (!reset) cameraZ = nextZ;
    glutPostRedisplay();
}

// Обработка стрелок
void keyboardArrows(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP: // Поднять камеру
        if (cameraY + cameraSpeed <= 4.0f) cameraY += cameraSpeed;
        break;
    case GLUT_KEY_DOWN: // Опустить камеру
        if (cameraY - cameraSpeed >= 0.0f) cameraY -= cameraSpeed;
        break;
    case GLUT_KEY_LEFT: // Взгляд вверх
        if (cameraAngleVertical + lookSpeed <= M_PI_2) cameraAngleVertical += lookSpeed;
        break;
    case GLUT_KEY_RIGHT: // Взгляд вниз
        if (cameraAngleVertical - lookSpeed >= -M_PI_2) cameraAngleVertical -= lookSpeed;
        break;
    }
    glutPostRedisplay();
}

// Изменение размера окна
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

// Настройка OpenGL
void initGL() {
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Черный фон
    buildBicone();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Bicone");

    initGL();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(keyboardArrows);

    glutMainLoop();
    return 0;
}