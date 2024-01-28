#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <GL/glu.h>
#include <iostream>
#include <vector>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/Exporter.hpp>
#include <assimp/postprocess.h>
#include "imgui.h"
#include "imgui-SFML.h"
#include "tinyfiledialogs.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/glm.hpp>

struct Vertex {
    float x, y, z;
};

struct Triangle {
    Vertex vertices[3];
};

struct Object3D {
    std::vector<Triangle> model;
    glm::vec3 position;
    glm::quat orientation;

    Object3D(std::vector<Triangle> mdl, glm::vec3 pos, glm::quat orient = glm::quat())
        : model(std::move(mdl)), position(pos), orientation(orient) {}
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
    // O� X - Czerwona
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(length, 0.0f, 0.0f);

    // O� Y - Zielona
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, length, 0.0f);

    // O� Z - Niebieska
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, length);
    glEnd();
}

void drawModel(const Object3D& object) {
    glPushMatrix();
    glTranslatef(object.position.x, object.position.y, object.position.z);

    // Uwzględnienie orientacji obiektu
    auto orientation = glm::eulerAngles(object.orientation);
    glRotatef(glm::degrees(orientation.x), 1.0f, 0.0f, 0.0f);
    glRotatef(glm::degrees(orientation.y), 0.0f, 1.0f, 0.0f);
    glRotatef(glm::degrees(orientation.z), 0.0f, 0.0f, 1.0f);

    // Zastosowanie skalowania tylko podczas rysowania
    glScalef(0.1f, 0.1f, 0.1f);

    glBegin(GL_TRIANGLES);
    for (const auto& triangle : object.model) {
        for (int i = 0; i < 3; ++i) {
            glVertex3f(triangle.vertices[i].x, triangle.vertices[i].y, triangle.vertices[i].z);
        }
    }
    glEnd();
    glPopMatrix();
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
                // Usunięcie skalowania z funkcji załadunku
                triangle.vertices[k].x = vertex.x;
                triangle.vertices[k].y = vertex.y;
                triangle.vertices[k].z = vertex.z;
            }
            triangles.push_back(triangle);
        }
    }
    return triangles;
}



void exportModel(const Object3D& object, const std::string& filename) {
    Assimp::Exporter exporter;
    aiScene scene;

    // Tworzenie nowego mesh (siatki)
    scene.mMeshes = new aiMesh * [1];
    scene.mMeshes[0] = new aiMesh();
    scene.mMeshes[0]->mMaterialIndex = 0;

    // Utworzenie root node (korzenia)
    scene.mRootNode = new aiNode();
    scene.mRootNode->mMeshes = new unsigned int[1];
    scene.mRootNode->mMeshes[0] = 0;
    scene.mRootNode->mNumMeshes = 1;

    // Przypisanie liczby siatek
    scene.mNumMeshes = 1;

    // Przydzielenie pamięci na wierzchołki i indeksy
    auto* mesh = scene.mMeshes[0];
    mesh->mVertices = new aiVector3D[object.model.size() * 3];
    mesh->mNumVertices = object.model.size() * 3;
    mesh->mFaces = new aiFace[object.model.size()];
    mesh->mNumFaces = object.model.size();

    // Przepisywanie wierzchołków i tworzenie indeksów
    for (size_t i = 0; i < object.model.size(); ++i) {
        for (size_t j = 0; j < 3; ++j) {
            const Vertex& vertex = object.model[i].vertices[j];
            mesh->mVertices[i * 3 + j] = aiVector3D(vertex.x, vertex.y, vertex.z);
        }

        aiFace& face = mesh->mFaces[i];
        face.mIndices = new unsigned int[3];
        face.mNumIndices = 3;
        face.mIndices[0] = i * 3;
        face.mIndices[1] = i * 3 + 1;
        face.mIndices[2] = i * 3 + 2;
    }

    // Ustawienia materiału (opcjonalne)
    scene.mMaterials = new aiMaterial * [1];
    scene.mMaterials[0] = new aiMaterial();
    scene.mNumMaterials = 1;

    // Eksportowanie sceny do pliku
    aiReturn res = exporter.Export(&scene, "stl", filename);

    if (res != aiReturn_SUCCESS) {
        std::cerr << "Assimp error: " << exporter.GetErrorString() << std::endl;
    }
}

void initWindow(sf::RenderWindow& window) {
    window.create(sf::VideoMode(1920, 1080), "3D Model Viewer", sf::Style::Default, sf::ContextSettings(24));
    window.setVerticalSyncEnabled(true);
    ImGui::SFML::Init(window);


    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();

    ImGui::SFML::UpdateFontTexture(); 
}

void handleEvents(sf::RenderWindow& window, bool& running, glm::vec3& cameraPos, float& cameraAngleX, float& cameraAngleY, int& selectedModelIndex, const size_t modelsCount) {
    sf::Event event;
    while (window.pollEvent(event)) {
        ImGui::SFML::ProcessEvent(event);

        if (event.type == sf::Event::Closed) {
            running = false;
        }

        if (event.type == sf::Event::KeyPressed) {
            // Obsługa klawisza Tab tylko wtedy, gdy ImGui nie korzysta z klawiatury
            if (event.key.code == sf::Keyboard::Tab && !ImGui::GetIO().WantCaptureKeyboard) {
                selectedModelIndex = (selectedModelIndex + 1) % modelsCount;
                continue; // Pomiń pozostałą część obsługi zdarzeń klawiatury
            }

            // Pozostała obsługa zdarzeń klawiatury jeśli ImGui nie przechwytuje klawiatury
            if (!ImGui::GetIO().WantCaptureKeyboard) {
                const float moveSpeed = 0.1f;
                const float rotateSpeed = 2.0f;
                glm::vec3 forward(
                    cos(glm::radians(cameraAngleY)) * cos(glm::radians(cameraAngleX)),
                    sin(glm::radians(cameraAngleX)),
                    sin(glm::radians(cameraAngleY)) * cos(glm::radians(cameraAngleX))
                );
                forward = glm::normalize(forward);
                glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
                glm::vec3 up = glm::normalize(glm::cross(right, forward));

                switch (event.key.code) {
                case sf::Keyboard::S:
                    cameraPos += right * moveSpeed;
                    break;
                case sf::Keyboard::W:
                    cameraPos -= right * moveSpeed;
                    break;
                case sf::Keyboard::A:
                    cameraPos -= forward * moveSpeed;
                    break;
                case sf::Keyboard::D:
                    cameraPos += forward * moveSpeed;
                    break;
                case sf::Keyboard::Q:
                    cameraAngleY -= rotateSpeed;
                    break;
                case sf::Keyboard::E:
                    cameraAngleY += rotateSpeed;
                    break;
                case sf::Keyboard::R:
                    cameraPos += up * moveSpeed;
                    break;
                case sf::Keyboard::F:
                    cameraPos -= up * moveSpeed;
                    break;
                case sf::Keyboard::Up:
                    cameraAngleX += rotateSpeed;
                    break;
                case sf::Keyboard::Down:
                    cameraAngleX -= rotateSpeed;
                    break;
                case sf::Keyboard::Left:
                    cameraAngleY += rotateSpeed;
                    break;
                case sf::Keyboard::Right:
                    cameraAngleY -= rotateSpeed;
                    break;
                    // Brak przypadku dla sf::Keyboard::Tab, ponieważ już został obsłużony powyżej
                }
            }
        }

        if (event.type == sf::Event::Resized) {
            glViewport(0, 0, event.size.width, event.size.height);
        }
    }
}

void drawScene(sf::RenderWindow& window, const std::vector<Object3D>& models, const glm::vec3& cameraPos, float cameraAngleX, float cameraAngleY, int selectedModelIndex) {
    window.clear();
    window.pushGLStates();

    // Ustawienie perspektywy
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, static_cast<float>(window.getSize().x) / window.getSize().y, 0.1f, 1000.0f);

    // Ustawienie widoku kamery
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(-cameraPos.x, -cameraPos.y, -cameraPos.z);
    glRotatef(cameraAngleX, 1.0f, 0.0f, 0.0f);
    glRotatef(cameraAngleY, 0.0f, 1.0f, 0.0f);

    // Rysowanie siatki i osi
    drawGrid(100.0f, 1.0f);
    drawAxes(100.0f);

    // Rysowanie wszystkich modeli
    for (size_t i = 0; i < models.size(); ++i) {
        if (i == selectedModelIndex) {
            // Wyróżnienie wybranego modelu, np. poprzez zmianę koloru
            glColor3f(1.0f, 0.0f, 0.0f); // Przykładowo, ustawienie na czerwony
        }
        else {
            glColor3f(1.0f, 1.0f, 1.0f); // Normalny kolor dla niewybranych modeli
        }
        drawModel(models[i]);

        // Przywrócenie domyślnego koloru po rysowaniu modelu
        glColor3f(1.0f, 1.0f, 1.0f);
    }

    window.popGLStates();
    ImGui::SFML::Render(window);
    window.display();
}

void handleMenu(std::vector<Object3D>& models, int selectedModelIndex) {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Load Model")) {
                const char* filterPatterns[5] = { "*.obj", "*.stl", "*.ply", "*.fbx", "*.3ds" };
                const char* filename = tinyfd_openFileDialog("Load a 3D Model", "", 5, filterPatterns, NULL, 0);
                if (filename) {
                    // Ustawienie pozycji modelu w zależności od jego indeksu w liście
                    glm::vec3 position(0.0f, 0.0f, 0.0f);
                    models.emplace_back(loadModel(filename), position);
                }
            }

            if (ImGui::MenuItem("Add Another Model")) {
                const char* filterPatterns[5] = { "*.obj", "*.stl", "*.ply", "*.fbx", "*.3ds" };
                const char* filename = tinyfd_openFileDialog("Add another 3D Model", "", 5, filterPatterns, NULL, 0);
                if (filename) {
                    // Ustawienie pozycji dla kolejnego modelu
                    glm::vec3 position(models.size() * 2.0f, 0.0f, 0.0f);
                    models.emplace_back(loadModel(filename), position);
                }
            }

            if (ImGui::MenuItem("Export Model")) {
                if (selectedModelIndex != -1 && selectedModelIndex < models.size()) {
                    const char* filterPatterns[1] = { "*.stl" };

                    const char* savePath = tinyfd_saveFileDialog(
                        "Save 3D Model",
                        "",
                        1, // Liczba wzorców filtrów
                        filterPatterns,
                        NULL // Brak opisu, ponieważ mamy tylko jeden typ pliku
                    );

                    // Sprawdzenie, czy użytkownik wybrał ścieżkę i zapisanie pliku
                    if (savePath) {
                        std::string savePathStr = savePath;
                        // Sprawdź, czy ścieżka zawiera rozszerzenie .stl, jeśli nie - dodaj je
                        if (savePathStr.find(".stl") == std::string::npos) {
                            savePathStr += ".stl";
                        }
                        exportModel(models[selectedModelIndex], savePathStr);
                    }
                }
            }





            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void controlPanel(std::vector<Object3D>& models, int& selectedModelIndex) {
    if (selectedModelIndex != -1 && selectedModelIndex < models.size()) {
        ImGui::Begin("Control Panel");
        ImGui::Text("Selected Model Position");
        ImGui::InputFloat("X", &models[selectedModelIndex].position.x);
        ImGui::InputFloat("Y", &models[selectedModelIndex].position.y);
        ImGui::InputFloat("Z", &models[selectedModelIndex].position.z);

        // Dodanie kontrolek do obracania obiektu
        ImGui::Text("Selected Model Rotation");
        float rotation[3] = {
            glm::degrees(glm::eulerAngles(models[selectedModelIndex].orientation).x),
            glm::degrees(glm::eulerAngles(models[selectedModelIndex].orientation).y),
            glm::degrees(glm::eulerAngles(models[selectedModelIndex].orientation).z)
        };
        if (ImGui::InputFloat3("Rotation", rotation)) {
            models[selectedModelIndex].orientation = glm::quat(glm::radians(glm::vec3(rotation[0], rotation[1], rotation[2])));
        }

        ImGui::End();
    }
}


int main() {
    sf::RenderWindow window;
    initWindow(window);

    std::vector<Object3D> models;  // Lista przechowująca obiekty 3D
    bool running = true;
    int selectedModelIndex = -1;  // Początkowo żaden model nie jest wybrany
   
    glm::vec3 cameraPos(0.0f, 0.0f, 10.0f);
    float cameraAngleX = 0.0f, cameraAngleY = 0.0f;

    sf::Clock deltaClock;

    while (running) {
        handleEvents(window, running, cameraPos, cameraAngleX, cameraAngleY, selectedModelIndex, models.size());
        ImGui::SFML::Update(window, deltaClock.restart());
        handleMenu(models, selectedModelIndex);  // Zaktualizowane wywołanie handleMenu

        if (selectedModelIndex != -1) {
            controlPanel(models, selectedModelIndex);  // Wywołanie panelu kontrolnego
        }

        drawScene(window, models, cameraPos, cameraAngleX, cameraAngleY, selectedModelIndex);
        ImGui::SFML::Render(window);
    }

    ImGui::SFML::Shutdown();
    return 0;
}
