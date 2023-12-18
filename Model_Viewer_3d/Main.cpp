#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <GL/glu.h>
#include <iostream>
#include <vector>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "imgui.h"
#include "imgui-SFML.h"
#include "tinyfiledialogs.h"
#include <SFML/Graphics/RenderWindow.hpp>

struct Vertex {
    float x, y, z;
};

struct Triangle {
    Vertex vertices[3];
};

void drawGrid(float size, float step) {
    glBegin(GL_LINES);
    glColor3f(0.5f, 0.5f, 0.5f); // Kolor siatki
    for (float i = -size; i <= size; i += step) {
        glVertex3f(i, 0.0f, -size); glVertex3f(i, 0.0f, size); // Linie poziome
        glVertex3f(-size, 0.0f, i); glVertex3f(size, 0.0f, i); // Linie pionowe
    }
    glEnd();
}

void drawAxes(float length) {
    glBegin(GL_LINES);
    // Oœ X - Czerwona
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(length, 0.0f, 0.0f);

    // Oœ Y - Zielona
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, length, 0.0f);

    // Oœ Z - Niebieska
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, length);
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

void initWindow(sf::RenderWindow& window) {
    window.create(sf::VideoMode(1920, 1080), "3D Model Viewer", sf::Style::Default, sf::ContextSettings(24));
    window.setVerticalSyncEnabled(true);
    ImGui::SFML::Init(window);

    // Opcjonalne: £adowanie w³asnej czcionki (jeœli nie chcesz u¿ywaæ domyœlnej)
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    // Jeœli masz w³asn¹ czcionkê, za³aduj j¹ tutaj
    // io.Fonts->AddFontFromFileTTF("path_to_your_font.ttf", 16.0f);
    ImGui::SFML::UpdateFontTexture(); // Aktualizuje teksturê czcionki
}

void handleEvents(sf::RenderWindow& window, bool& running, float& cameraX, float& cameraY, float& cameraZ, float& cameraAngleX, float& cameraAngleY) {
    sf::Event event;
    while (window.pollEvent(event)) {
        ImGui::SFML::ProcessEvent(event);

        if (event.type == sf::Event::Closed) {
            running = false;
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
                cameraAngleY -= 0.5f;
                break;
            case sf::Keyboard::E:
                cameraAngleY += 0.5f;
                break;
            case sf::Keyboard::Up:
                cameraAngleX += 0.5f;
                break;
            case sf::Keyboard::Down:
                cameraAngleX -= 0.5f;
                break;
            }
        }

        if (event.type == sf::Event::Resized) {
            glViewport(0, 0, event.size.width, event.size.height);
        }
    }
}

void drawScene(sf::RenderWindow& window, const std::vector<std::vector<Triangle>>& models, float cameraX, float cameraY, float cameraZ, float cameraAngleX, float cameraAngleY) {
    window.clear();
    window.pushGLStates();

    // Ustawienie perspektywy
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, static_cast<float>(window.getSize().x) / window.getSize().y, 0.1f, 1000.0f);

    // Ustawienie widoku kamery
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(-cameraX, -cameraY, -cameraZ);
    glRotatef(cameraAngleX, 1.0f, 0.0f, 0.0f);
    glRotatef(cameraAngleY, 0.0f, 1.0f, 0.0f);

    // Rysowanie siatki i osi (powinny byæ rysowane pierwsze)
    drawGrid(100.0f, 1.0f); // Zmieniono rozmiar siatki, aby by³a widoczna
    drawAxes(100.0f); // Zmieniono d³ugoœæ osi, aby by³y widoczne

    // Rysowanie wszystkich modeli
    for (const auto& model : models) {
        drawModel(model);
    }

    window.popGLStates();
    ImGui::SFML::Render(window);
    window.display();
}


void handleMenu(std::vector<std::vector<Triangle>>& models) {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Load Model")) {
                const char* filterPatterns[5] = { "*.obj", "*.stl", "*.ply", "*.fbx", "*.3ds" };
                const char* filename = tinyfd_openFileDialog("Load a 3D Model", "", 5, filterPatterns, NULL, 0);
                if (filename) {
                    models.push_back(loadModel(filename));
                }
            }

            if (ImGui::MenuItem("Add Another Model")) {
                const char* filterPatterns[5] = { "*.obj", "*.stl", "*.ply", "*.fbx", "*.3ds" };
                const char* filename = tinyfd_openFileDialog("Add another 3D Model", "", 5, filterPatterns, NULL, 0);
                if (filename) {
                    models.push_back(loadModel(filename));
                }
            }

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

int main() {
    sf::RenderWindow window;
    initWindow(window);

    std::vector<std::vector<Triangle>> models;
    bool running = true;

    float cameraX = 0.0f, cameraY = 0.0f, cameraZ = 5.0f;
    float cameraAngleX = 0.0f, cameraAngleY = 0.0f;

    // Zegar do aktualizacji ImGui
    sf::Clock deltaClock;

    while (running) {
        handleEvents(window, running, cameraX, cameraY, cameraZ, cameraAngleX, cameraAngleY);
        ImGui::SFML::Update(window, deltaClock.restart());
        handleMenu(models);
        drawScene(window, models, cameraX, cameraY, cameraZ, cameraAngleX, cameraAngleY);
    }

    ImGui::SFML::Shutdown();
    return 0;
}