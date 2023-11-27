#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <GL/glu.h>
#include <iostream>
#include <vector>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "tinyfiledialogs.h"

struct Vertex {
    float x, y, z;
};

struct Triangle {
    Vertex vertices[3];
};
void drawGrid(float size, float step) {
    glBegin(GL_LINES);
    glColor3f(0.5, 0.5, 0.5); // Kolor siatki
    for (float i = -size; i <= size; i += step) {
        glVertex3f(i, 0, -size); glVertex3f(i, 0, size); // Linie poziome
        glVertex3f(-size, 0, i); glVertex3f(size, 0, i); // Linie pionowe
    }
    glEnd();
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

void drawModel(const std::vector<Triangle>& triangles) {
    glBegin(GL_TRIANGLES);
    for (const auto& triangle : triangles) {
        for (int i = 0; i < 3; ++i) {
            glVertex3f(triangle.vertices[i].x, triangle.vertices[i].y, triangle.vertices[i].z);
        }
    }
    glEnd();
}


std::vector<Triangle> loadModel(const std::string& filename) {
    std::vector<Triangle> triangles;
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
        return triangles;
    }

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[i];
        for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
            aiFace face = mesh->mFaces[j];
            Triangle triangle;
            for (unsigned int k = 0; k < 3; k++) {
                aiVector3D vertex = mesh->mVertices[face.mIndices[k]];
                triangle.vertices[k].x = vertex.x * 0.1f; // Skalowanie
                triangle.vertices[k].y = vertex.y * 0.1f;
                triangle.vertices[k].z = vertex.z * 0.1f;
            }
            triangles.push_back(triangle);
        }
    }
    return triangles;
}

// Funkcje drawAxes, drawGrid i drawModel pozostaj¹ bez zmian

int main() {
    const char* filterPatterns[5] = { "*.obj", "*.stl", "*.ply", "*.fbx", "*.3ds" };
    const char* filename = tinyfd_openFileDialog("Wybierz model 3D", "", 5, filterPatterns, NULL, 0);

    if (!filename) {
        std::cerr << "Nie wybrano pliku" << std::endl;
        return 1;
    }

    std::vector<Triangle> triangles = loadModel(filename);

    sf::Window window(sf::VideoMode(800, 600), "Model Viewer 3D", sf::Style::Default, sf::ContextSettings(24));
    window.setVerticalSyncEnabled(true);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, static_cast<float>(window.getSize().x) / window.getSize().y, 1.0f, 500.0f);

    float cameraX = 0.0f, cameraY = 5.0f, cameraZ = 5.0f;
    float angleX = 0.0f, angleY = 0.0f;
    const float rotationSpeed = 0.5f;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                case sf::Keyboard::W:
                    cameraZ -= 0.1f;
                    break;
                case sf::Keyboard::S:
                    cameraZ += 0.1f;
                    break;
                case sf::Keyboard::A:
                    cameraX -= 0.1f;
                    break;
                case sf::Keyboard::D:
                    cameraX += 0.1f;
                    break;
                case sf::Keyboard::Q:
                    angleY -= rotationSpeed;
                    break;
                case sf::Keyboard::E:
                    angleY += rotationSpeed;
                    break;
                case sf::Keyboard::Up:
                    angleX += rotationSpeed;
                    break;
                case sf::Keyboard::Down:
                    angleX -= rotationSpeed;
                    break;
                }
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(-cameraX, -cameraY, -cameraZ);
        glRotatef(angleX, 1.0f, 0.0f, 0.0f);
        glRotatef(angleY, 0.0f, 1.0f, 0.0f);

        drawGrid(20.0f, 1.0f);
        drawAxes(20.0f);
        drawModel(triangles);

        window.display();
    }

    return 0;
}

