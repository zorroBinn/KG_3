#define _USE_MATH_DEFINES
#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include <queue>
#include <tuple>
#include <iostream>

// Параметры сферы
const float radius = 1.0f; // Радиус сферы
const int numPoints = 999; // Количество точек для решётки Фибоначчи

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
    float lightDir[] = { 0.0f, 0.0f, 1.0f }; // Направление света
    float lightLength = sqrt(lightDir[0] * lightDir[0] + lightDir[1] * lightDir[1] + lightDir[2] * lightDir[2]);
    float normalizedLightDir[] = { lightDir[0] / lightLength, lightDir[1] / lightLength, lightDir[2] / lightLength };

    // Скалярное произведение нормали и направления света
    float dotProduct = nx * normalizedLightDir[0] + ny * normalizedLightDir[1] + nz * normalizedLightDir[2];
    return fmax(0.0f, dotProduct); // Интенсивность от 0 до 1
}

// Построение сферы с использованием решётки Фибоначчи
void buildSphere() {
    const float goldenRatio = (1.0f + sqrt(5.0f)) / 2.0f; // Золотое сечение
    const float angleIncrement = 2.0f * M_PI / goldenRatio; // Угловое смещение

    vertices.clear();
    indices.clear();

    // Генерация точек на сфере
    for (int i = 0; i < numPoints; ++i) {
        float t = static_cast<float>(i) + 0.5f; // Сдвиг для устранения перекрытия
        float phi = acos(1 - 2.0f * t / numPoints); // Угол широты
        float theta = angleIncrement * i;          // Угол долготный

        float x = sin(phi) * cos(theta);
        float y = sin(phi) * sin(theta);
        float z = cos(phi);

        // Рассчитываем нормали и интенсивность света
        float nx, ny, nz;
        calculateNormal(x, y, z, nx, ny, nz);
        float intensity = calculateLightIntensity(nx, ny, nz);

        // Добавляем вершину в список
        vertices.push_back({ x, y, z, nx, ny, nz, intensity });
    }

    // Создание индексов для триангуляции с ближайшими соседями
    for (int i = 0; i < numPoints; ++i) {
        std::priority_queue<std::tuple<float, int>> closestPoints; // Расстояние, индекс

        for (int j = 0; j < numPoints; ++j) {
            if (i == j) continue;
            float dx = vertices[i].x - vertices[j].x;
            float dy = vertices[i].y - vertices[j].y;
            float dz = vertices[i].z - vertices[j].z;
            float distance = dx * dx + dy * dy + dz * dz; // Квадрат расстояния

            if (closestPoints.size() < 6) {
                closestPoints.emplace(distance, j);
            }
            else if (distance < std::get<0>(closestPoints.top())) {
                closestPoints.pop();
                closestPoints.emplace(distance, j);
            }
        }

        // Создаём треугольники с ближайшими соседями
        std::vector<int> neighbors;
        while (!closestPoints.empty()) {
            neighbors.push_back(std::get<1>(closestPoints.top()));
            closestPoints.pop();
        }

        for (size_t k = 0; k < neighbors.size(); ++k) {
            int next = neighbors[(k + 1) % neighbors.size()];
            indices.push_back(i);
            indices.push_back(neighbors[k]);
            indices.push_back(next);
        }
    }
}

// Рендеринг сферы с использованием закраски Гуро
void renderSphere() {
    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < indices.size(); i += 3) {
        for (int j = 0; j < 3; ++j) {
            const Vertex& v = vertices[indices[i + j]];
            glColor3f(v.intensity, v.intensity, v.intensity); // Устанавливаем цвет вершины
            glNormal3f(v.nx, v.ny, v.nz);
            glVertex3f(v.x, v.y, v.z);
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

    renderSphere(); // Рисуем сферу

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

    if (!reset) {
        cameraX = nextX;
        cameraZ = nextZ;
    }
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