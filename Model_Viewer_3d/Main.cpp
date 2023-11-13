#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <GL/glu.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "tinyfiledialogs.h"

struct Vertex {
    float x, y, z;
};

struct Triangle {
    Vertex vertices[3];
};

std::vector<Triangle> readSTL(const std::string& filename) {
    std::vector<Triangle> triangles;
    std::ifstream stlFile(filename);
    std::string line;

    if (!stlFile.is_open()) {
        std::cerr << "Nie mo¿na otworzyæ pliku: " << filename << std::endl;
        return triangles;
    }

    while (std::getline(stlFile, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "facet") {
            Triangle triangle;
            for (int i = 0; i < 3; ++i) {
                std::getline(stlFile, line); // Skip "outer loop"
                std::getline(stlFile, line); // Read vertex data

                std::istringstream vertexLine(line);
                std::string vertexToken;
                vertexLine >> vertexToken; // Skip "vertex"

                vertexLine >> triangle.vertices[i].x >> triangle.vertices[i].y >> triangle.vertices[i].z;
            }
            triangles.push_back(triangle);
            std::getline(stlFile, line); // Skip "endloop"
            std::getline(stlFile, line); // Skip "endfacet"
        }
    }

    stlFile.close();
    return triangles;
}

void drawAxes(float length) {
    glBegin(GL_LINES);
    // Oœ X - Czerwona
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(length, 0.0, 0.0);

    // Oœ Y - Zielona
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, length, 0.0);

    // Oœ Z - Niebieska
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, length);
    glEnd();
}

void drawGrid(float size, float step) {
    glBegin(GL_LINES);
    glColor3f(0.5, 0.5, 0.5); // Kolor siatki
    for (float i = -size; i <= size; i += step) {
        glVertex3f(i, 0, -size); glVertex3f(i, 0, size); // Linie poziome
        glVertex3f(-size, 0, i); glVertex3f(size, 0, i); // Linie pionowe
    }
    glEnd();
}

void drawModel(const std::vector<Triangle>& triangles) {
    glBegin(GL_TRIANGLES);
    for (const auto& triangle : triangles) {
        for (int i = 0; i < 3; ++i) {
            glVertex3f(triangle.vertices[i].x, triangle.vertices[i].y, triangle.vertices[i].z);
        }
    }
    glEnd();
}

int main() {
    // U¿ywamy tiny file dialogs do wybrania pliku
    const char* filterPatterns[1] = { "*.stl" };
    const char* filename = tinyfd_openFileDialog("Wybierz plik STL", "", 1, filterPatterns, NULL, 0);

    if (!filename) {
        std::cerr << "Nie wybrano pliku" << std::endl;
        return 1;
    }

    std::vector<Triangle> triangles = readSTL(filename);

    sf::Window window(sf::VideoMode(800, 600), "Model Viewer 3D", sf::Style::Default, sf::ContextSettings(24));
    window.setVerticalSyncEnabled(true);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, static_cast<float>(window.getSize().x) / window.getSize().y, 1.0f, 500.0f);

    float cameraX = 0.0f, cameraY = 5.0f, cameraZ = 5.0f;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::W) cameraZ -= 0.1f;
                if (event.key.code == sf::Keyboard::S) cameraZ += 0.1f;
                if (event.key.code == sf::Keyboard::A) cameraX -= 0.1f;
                if (event.key.code == sf::Keyboard::D) cameraX += 0.1f;
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(cameraX, cameraY, cameraZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

        drawGrid(20.0f, 1.0f);
        drawAxes(20.0f);

        glColor3f(1.0f, 1.0f, 1.0f); // Ustaw kolor rysowania modelu
        drawModel(triangles); // Rysuj model

        window.display();
    }

    return 0;
}
