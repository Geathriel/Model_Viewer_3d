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

void saveAsSimpleFormat(const std::vector<Triangle>& triangles, const std::string& filename) {
    std::ofstream outputFile(filename);

    if (!outputFile.is_open()) {
        std::cerr << "Nie mo¿na otworzyæ pliku wyjœciowego: " << filename << std::endl;
        return;
    }

    for (const auto& triangle : triangles) {
        for (int i = 0; i < 3; ++i) {
            outputFile << triangle.vertices[i].x << " "
                << triangle.vertices[i].y << " "
                << triangle.vertices[i].z << std::endl;
        }
    }

    outputFile.close();
}

int start() {
    // U¿ywamy tiny file dialogs do wybrania pliku
    const char* filterPatterns[1] = { "*.stl" };
    const char* filename = tinyfd_openFileDialog("Wybierz plik STL", "", 1, filterPatterns, NULL, 0);

    if (!filename) {
        std::cerr << "Nie wybrano pliku" << std::endl;
        return 1;
    }

    std::vector<Triangle> triangles = readSTL(filename);
    std::string outputFilename = "output.txt";
    saveAsSimpleFormat(triangles, outputFilename);

    return 0;
}