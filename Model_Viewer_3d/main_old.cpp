#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <GL/glu.h>

void drawAxes(float length) {
    glBegin(GL_LINES);
    // Oś X - Czerwona
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(length, 0.0, 0.0);

    // Oś Y - Zielona
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, length, 0.0);

    // Oś Z - Niebieska
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

int main() {
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

        // Rysowanie siatki i osi
        drawGrid(20.0f, 1.0f);
        drawAxes(20.0f);

        glBegin(GL_QUADS);
        // Front Face
        glColor3f(1.0, 0.0, 0.0); glVertex3f(-1.0, -1.0, 1.0);
        glColor3f(0.0, 1.0, 0.0); glVertex3f(1.0, -1.0, 1.0);
        glColor3f(0.0, 0.0, 1.0); glVertex3f(1.0, 1.0, 1.0);
        glColor3f(1.0, 1.0, 0.0); glVertex3f(-1.0, 1.0, 1.0);
        // Back Face
        glColor3f(1.0, 1.0, 0.0); glVertex3f(-1.0, -1.0, -1.0);
        glColor3f(0.0, 1.0, 1.0); glVertex3f(-1.0, 1.0, -1.0);
        glColor3f(1.0, 0.0, 1.0); glVertex3f(1.0, 1.0, -1.0);
        glColor3f(0.0, 0.0, 1.0); glVertex3f(1.0, -1.0, -1.0);
        // Top Face
        glColor3f(0.0, 1.0, 0.0); glVertex3f(-1.0, 1.0, -1.0);
        glColor3f(0.0, 1.0, 1.0); glVertex3f(-1.0, 1.0, 1.0);
        glColor3f(1.0, 0.0, 1.0); glVertex3f(1.0, 1.0, 1.0);
        glColor3f(1.0, 1.0, 0.0); glVertex3f(1.0, 1.0, -1.0);
        // Bottom Face
        glColor3f(1.0, 0.5, 0.0); glVertex3f(-1.0, -1.0, -1.0);
        glColor3f(1.0, 0.0, 0.0); glVertex3f(1.0, -1.0, -1.0);
        glColor3f(0.0, 0.0, 1.0); glVertex3f(1.0, -1.0, 1.0);
        glColor3f(0.0, 1.0, 0.0); glVertex3f(-1.0, -1.0, 1.0);
        // Right face
        glColor3f(1.0, 0.0, 1.0); glVertex3f(1.0, -1.0, -1.0);
        glColor3f(1.0, 1.0, 0.0); glVertex3f(1.0, 1.0, -1.0);
        glColor3f(0.0, 0.0, 1.0); glVertex3f(1.0, 1.0, 1.0);
        glColor3f(0.0, 1.0, 1.0); glVertex3f(1.0, -1.0, 1.0);
        // Left Face
        glColor3f(0.0, 1.0, 0.0); glVertex3f(-1.0, -1.0, -1.0);
        glColor3f(0.0, 0.0, 1.0); glVertex3f(-1.0, -1.0, 1.0);
        glColor3f(1.0, 0.0, 1.0); glVertex3f(-1.0, 1.0, 1.0);
        glColor3f(1.0, 1.0, 0.0); glVertex3f(-1.0, 1.0, -1.0);
        glEnd();

        window.display();
    }

    return 0;
}
