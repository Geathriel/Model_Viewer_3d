Wstęp
Ta dokumentacja opisuje proces instalacji i uruchomienia aplikacji 3D Model Viewer. Aplikacja jest kompatybilna z systemam Windows

Wymagane Komponenty
C++ Compiler ( MSVC dla Windows)
CMake (wersja 3.10 lub nowsza)
SFML (wersja 2.5.1 lub nowsza)
Assimp (wersja 5.0.1 lub nowsza)
ImGui i ImGui-SFML
GLM (OpenGL Mathematics)

Kompilacja
Otwórz terminal lub wiersz poleceń w folderze projektu.
Utwórz folder build: mkdir build && cd build
Uruchom CMake do konfiguracji projektu: cmake ..
Uruchom kompilację: cmake --build .

Uruchomienie Aplikacji
Po skompilowaniu, uruchom wykonywalny plik aplikacji, który znajduje się w folderze build.

Troubleshooting
Upewnij się, że wszystkie zależności są zainstalowane i odpowiednio skonfigurowane.
W przypadku błędów kompilacji, sprawdź czy wersje bibliotek są kompatybilne.
Jeśli występują problemy związane ze ścieżkami do bibliotek, sprawdź pliki konfiguracyjne CMake.
