#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>

#include "GameObject.hpp"
#include "Scene.hpp"
#include "Physics.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

bool checkCollision(const sf::FloatRect& a, const sf::FloatRect& b) {
    return a.findIntersection(b).has_value();
}

float snapValue(float value, float gridSize) {
    return std::round(value / gridSize) * gridSize;
}

sf::Vector2f snapPosition(sf::Vector2f position, float gridSize) {
    return {
        snapValue(position.x, gridSize),
        snapValue(position.y, gridSize)
    };
}

int main() {
    sf::RenderWindow window(
        sf::VideoMode({1000, 700}),
        "Real Engine Research - ImGui Editor"
    );

    window.setFramerateLimit(60);

    // Initialize Dear ImGui
    if (!ImGui::SFML::Init(window, false)) {
        std::cerr << "Failed to initialize ImGui-SFML\n";
        return 1;
    }

    // Register a default font
    ImGuiIO& io = ImGui::GetIO();

    ImFont* font = io.Fonts->AddFontFromFileTTF(
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        18.0f
    );

    if (font == nullptr) {
        std::cerr << "Failed to load Arial font\n";
    }


    // Editor settings
    bool playMode = false;
    bool showGrid = true;
    bool snapToGrid = false;

    const float gridSize = 40.f;
    const sf::Vector2f playerStart{50.f, 50.f};

    const float moveSpeed = 250.f;
    const float jumpStrength = -500.f;

    // Create grid
    sf::VertexArray grid(sf::PrimitiveType::Lines);

    for (float x = 0.f; x <= 1000.f; x += gridSize) {
        sf::Vertex top;
        top.position = {x, 0.f};
        top.color = sf::Color(65, 65, 75);

        sf::Vertex bottom;
        bottom.position = {x, 700.f};
        bottom.color = sf::Color(65, 65, 75);

        grid.append(top);
        grid.append(bottom);
    }

    for (float y = 0.f; y <= 700.f; y += gridSize) {
        sf::Vertex left;
        left.position = {0.f, y};
        left.color = sf::Color(65, 65, 75);

        sf::Vertex right;
        right.position = {1000.f, y};
        right.color = sf::Color(65, 65, 75);

        grid.append(left);
        grid.append(right);
    }

    // Player
    sf::RectangleShape player({40.f, 40.f});
    player.setFillColor(sf::Color::Green);
    player.setPosition(playerStart);

    sf::Vector2f velocity{0.f, 0.f};

    Physics physics;
    Scene scene;

    sf::Clock clock;
    sf::Clock deltaClock;

    // Scene objects
    std::vector<GameObject> objects;

    objects.emplace_back(
        sf::Vector2f{200.f, 150.f},
        sf::Vector2f{200.f, 40.f},
        sf::Color::Red, 1
    );

    objects.emplace_back(
        sf::Vector2f{450.f, 250.f},
        sf::Vector2f{40.f, 200.f},
        sf::Color::Blue, 2
    );

    objects.emplace_back(
        sf::Vector2f{100.f, 450.f},
        sf::Vector2f{200.f, 40.f},
        sf::Color::Magenta, 3
    );

    objects.emplace_back(
        sf::Vector2f{300.f, 300.f},
        sf::Vector2f{140.f, 140.f},
        sf::Color::Cyan, 0
    );

    objects.emplace_back(
        sf::Vector2f{350.f, 350.f},
        sf::Vector2f{140.f, 140.f},
        sf::Color::Yellow, 4
    );

    std::vector<GameObject> sceneBackup;
    std::vector<std::size_t> renderOrder;

    int selectedObject = -1;
    int objectCounter = 0;

    bool dragging = false;
    sf::Vector2f dragOffset{0.f, 0.f};

    // Update drawing order based on Z-layer
    auto updateRenderOrder = [&]() {
        renderOrder.resize(objects.size());

        std::iota(renderOrder.begin(), renderOrder.end(), 0);

        std::stable_sort(
            renderOrder.begin(),
            renderOrder.end(),
            [&objects](std::size_t a, std::size_t b) {
                return objects[a].getLayer() <
                       objects[b].getLayer();
            }
        );
    };

    // Select an object
    auto selectObject = [&](int index) {
        for (auto& object : objects) {
            object.setSelected(false);
        }

        selectedObject = -1;
        dragging = false;

        if (index >= 0 &&
            index < static_cast<int>(objects.size())) {

            selectedObject = index;
            objects[index].setSelected(true);
        }
    };

    // Create an object
    auto createObject = [&]() {
        sf::Vector2f position{200.f, 200.f};

        if (snapToGrid) {
            position = snapPosition(position, gridSize);
        }

        sf::Color colors[] = {
            sf::Color::Red,
            sf::Color::Blue,
            sf::Color::Magenta,
            sf::Color::Cyan,
            sf::Color::Yellow,
            sf::Color::Green
        };

        sf::Color color = colors[objectCounter % 6];
        objectCounter++;

        objects.emplace_back(
            position,
            sf::Vector2f{80.f, 80.f},
            color,
            5
        );

        selectObject(static_cast<int>(objects.size()) - 1);
        updateRenderOrder();
    };

    // Delete selected object
    auto deleteObject = [&]() {
        if (selectedObject < 0 ||
            selectedObject >= static_cast<int>(objects.size())) {
            return;
        }

        objects.erase(objects.begin() + selectedObject);

        selectedObject = -1;
        dragging = false;

        updateRenderOrder();
    };

    // Save scene
    auto saveScene = [&]() {
        scene.save(objects, "scenes/example.scene");
        std::cout << "Scene saved\n";
    };

    // Load scene
    auto loadScene = [&]() {
        if (scene.load(objects, "scenes/example.scene")) {
            selectObject(-1);
            updateRenderOrder();
            std::cout << "Scene loaded\n";
        }
    };

    // Reset player
    auto resetPlayer = [&]() {
        player.setPosition(playerStart);
        velocity = {0.f, 0.f};
        physics.reset();
    };

    // Switch between editor and game
    auto toggleMode = [&]() {
        if (!playMode) {
            sceneBackup = objects;

            selectObject(-1);
            resetPlayer();

            physics.setGravityEnabled(true);
            playMode = true;

            std::cout << "Entered Play Mode\n";
        } else {
            objects = sceneBackup;

            selectObject(-1);
            resetPlayer();

            physics.setGravityEnabled(false);
            playMode = false;

            std::cout << "Returned to Edit Mode\n";
        }

        updateRenderOrder();
    };

    updateRenderOrder();

    // Main loop
    while (window.isOpen()) {
        float deltaTime = std::min(
            clock.restart().asSeconds(),
            0.033f
        );

        while (const auto event = window.pollEvent()) {
            // Pass events to ImGui
            ImGui::SFML::ProcessEvent(window, *event);

            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            // Prevent editor shortcuts while typing in ImGui
            bool keyboardCaptured =
                ImGui::GetIO().WantCaptureKeyboard;

            if (const auto* key =
                    event->getIf<sf::Event::KeyPressed>()) {

                if (key->code == sf::Keyboard::Key::Escape) {
                    window.close();
                }

                if (keyboardCaptured) {
                    continue;
                }

                if (key->code == sf::Keyboard::Key::Tab) {
                    toggleMode();
                    continue;
                }

                if (playMode) {
                    if (key->code == sf::Keyboard::Key::R) {
                        resetPlayer();
                    }

                    if (key->code == sf::Keyboard::Key::Space &&
                        physics.isGrounded()) {
                        velocity.y = jumpStrength;
                    }

                    continue;
                }

                // Editor keyboard shortcuts
                if (key->code == sf::Keyboard::Key::H) {
                    showGrid = !showGrid;
                }

                if (key->code == sf::Keyboard::Key::T) {
                    snapToGrid = !snapToGrid;
                }

                if (key->code == sf::Keyboard::Key::N) {
                    createObject();
                }

                if (key->code == sf::Keyboard::Key::F5) {
                    saveScene();
                }

                if (key->code == sf::Keyboard::Key::F9) {
                    loadScene();
                }

                if (selectedObject >= 0) {
                    GameObject& object = objects[selectedObject];
                    sf::Vector2f size = object.getSize();

                    if (key->code == sf::Keyboard::Key::Q) {
                        object.setLayer(object.getLayer() - 1);
                    }

                    if (key->code == sf::Keyboard::Key::E) {
                        object.setLayer(object.getLayer() + 1);
                    }

                    if (key->code == sf::Keyboard::Key::LBracket) {
                        size.x = std::max(20.f, size.x - 10.f);
                        object.setSize(size);
                    }

                    if (key->code == sf::Keyboard::Key::RBracket) {
                        size.x += 10.f;
                        object.setSize(size);
                    }

                    if (key->code == sf::Keyboard::Key::Hyphen) {
                        size.y = std::max(20.f, size.y - 10.f);
                        object.setSize(size);
                    }

                    if (key->code == sf::Keyboard::Key::Equal) {
                        size.y += 10.f;
                        object.setSize(size);
                    }

                    if (key->code == sf::Keyboard::Key::Delete ||
                        key->code == sf::Keyboard::Key::Backspace) {
                        deleteObject();
                    }
                }

                updateRenderOrder();
            }

            // Don't edit objects while playing
            if (playMode) {
                continue;
            }

            // Don't interact with scene through ImGui windows
            if (ImGui::GetIO().WantCaptureMouse) {
                continue;
            }

            // Select objects with mouse
            if (const auto* mouse =
                    event->getIf<sf::Event::MouseButtonPressed>()) {

                if (mouse->button == sf::Mouse::Button::Left) {
                    sf::Vector2f mousePosition =
                        window.mapPixelToCoords(mouse->position);

                    updateRenderOrder();
                    selectObject(-1);

                    for (auto it = renderOrder.rbegin();
                         it != renderOrder.rend(); ++it) {

                        std::size_t index = *it;

                        if (objects[index].getBounds()
                                .contains(mousePosition)) {

                            selectObject(static_cast<int>(index));

                            dragOffset =
                                mousePosition -
                                objects[index].getPosition();

                            dragging = true;
                            break;
                        }
                    }
                }
            }

            if (const auto* mouse =
                    event->getIf<sf::Event::MouseButtonReleased>()) {

                if (mouse->button == sf::Mouse::Button::Left) {
                    dragging = false;
                }
            }

            if (const auto* mouse =
                    event->getIf<sf::Event::MouseMoved>()) {

                if (dragging && selectedObject >= 0) {
                    sf::Vector2f mousePosition =
                        window.mapPixelToCoords(mouse->position);

                    sf::Vector2f newPosition =
                        mousePosition - dragOffset;

                    if (snapToGrid) {
                        newPosition =
                            snapPosition(newPosition, gridSize);
                    }

                    objects[selectedObject].setPosition(newPosition);
                }
            }
        }

        // Update ImGui
        ImGui::SFML::Update(window, deltaClock.restart());

        // -----------------------------
        // EDITOR CONTROLS PANEL
        // -----------------------------
        ImGui::Begin("Editor Controls");

        ImGui::Text("Real Engine Research");
        ImGui::Separator();

        ImGui::Text(
            "Current Mode: %s",
            playMode ? "PLAY" : "EDIT"
        );

        if (ImGui::Button(playMode ? "Stop Playing" : "Play Scene")) {
            toggleMode();
        }

        ImGui::Separator();

        if (!playMode) {
            ImGui::Checkbox("Show Grid", &showGrid);
            ImGui::Checkbox("Snap to Grid", &snapToGrid);

            if (ImGui::Button("Save Scene")) {
                saveScene();
            }

            ImGui::SameLine();

            if (ImGui::Button("Load Scene")) {
                loadScene();
            }

            if (ImGui::Button("Create Object")) {
                createObject();
            }

            ImGui::Text("Objects: %d", static_cast<int>(objects.size()));
        } else {
            if (ImGui::Button("Reset Player")) {
                resetPlayer();
            }

            ImGui::Text("A/D: Move");
            ImGui::Text("Space: Jump");
        }

        ImGui::End();

        // -----------------------------
        // SCENE HIERARCHY PANEL
        // -----------------------------
        if (!playMode) {
            ImGui::Begin("Scene Hierarchy");

            ImGui::Text("Scene Objects");
            ImGui::Separator();

            for (int i = 0; i < static_cast<int>(objects.size()); i++) {
                std::string name =
                    "GameObject " + std::to_string(i);

                bool selected = (selectedObject == i);

                if (ImGui::Selectable(name.c_str(), selected)) {
                    selectObject(i);
                }
            }

            ImGui::Separator();

            if (ImGui::Button("Add Object")) {
                createObject();
            }

            if (selectedObject >= 0) {
                if (ImGui::Button("Delete Selected")) {
                    deleteObject();
                }
            }

            ImGui::End();
        }

        // -----------------------------
        // INSPECTOR PANEL
        // -----------------------------
        if (!playMode) {
            ImGui::Begin("Inspector");
            ImGui::Text("FONT TEST: 0123456789 ABCDEFGHIJKLMNOPQRSTUVWXYZ");
            ImGui::Text("VALUES: 100 200 350 450 999");
            


            if (selectedObject >= 0 &&
                selectedObject < static_cast<int>(objects.size())) {

                GameObject& object = objects[selectedObject];

                ImGui::Text(
                    "Selected: GameObject %d",
                    selectedObject
                );

                ImGui::Separator();

                // Position
                sf::Vector2f position = object.getPosition();

                float pos[2] = {
                    position.x,
                    position.y
                };

                if (ImGui::DragFloat2("Position", pos, 1.f)) {
                    object.setPosition({pos[0], pos[1]});
                }

                // Size
                sf::Vector2f size = object.getSize();

                float dimensions[2] = {
                    size.x,
                    size.y
                };

                if (ImGui::DragFloat2("Dimensions", dimensions, 1.f)) {
                    dimensions[0] = std::max(20.f, dimensions[0]);
                    dimensions[1] = std::max(20.f, dimensions[1]);

                    object.setSize({
                        dimensions[0],
                        dimensions[1]
                    });
                }

                // Z-layer
                int layer = object.getLayer();

                if (ImGui::InputInt("Z-Layer", &layer)) {
                    object.setLayer(layer);
                }

                // Color
                sf::Color color = object.getColor();

                float rgba[4] = {
                    color.r / 255.f,
                    color.g / 255.f,
                    color.b / 255.f,
                    color.a / 255.f
                };

                if (ImGui::ColorEdit4("Color", rgba)) {
                    object.setColor(
                        sf::Color(
                            static_cast<std::uint8_t>(rgba[0] * 255.f),
                            static_cast<std::uint8_t>(rgba[1] * 255.f),
                            static_cast<std::uint8_t>(rgba[2] * 255.f),
                            static_cast<std::uint8_t>(rgba[3] * 255.f)
                        )
                    );
                }

                ImGui::Separator();

                if (ImGui::Button("Delete Object")) {
                    deleteObject();
                }
            } else {
                ImGui::Text("No object selected.");
                ImGui::Text("Select an object in the scene");
                ImGui::Text("or Scene Hierarchy panel.");
            }

            ImGui::End();
        }

        // -----------------------------
        // PLAYER PHYSICS
        // -----------------------------
        if (playMode) {
            velocity.x = 0.f;

            if (!ImGui::GetIO().WantCaptureKeyboard) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
                    velocity.x -= moveSpeed;
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
                    velocity.x += moveSpeed;
                }
            }

            sf::Vector2f oldPosition = player.getPosition();

            player.move({velocity.x * deltaTime, 0.f});

            bool horizontalCollision = false;

            for (const auto& object : objects) {
                if (checkCollision(
                        player.getGlobalBounds(),
                        object.getBounds())) {

                    horizontalCollision = true;
                    break;
                }
            }

            if (horizontalCollision) {
                player.setPosition(oldPosition);
            }

            physics.update(player, velocity, objects, deltaTime);

            sf::Vector2f position = player.getPosition();

            position.x = std::clamp(position.x, 0.f, 960.f);

            player.setPosition(position);
        }

        // -----------------------------
        // RENDER EVERYTHING
        // -----------------------------
        updateRenderOrder();

        window.clear(sf::Color(30, 30, 40));

        if (!playMode && showGrid) {
            window.draw(grid);
        }

        for (std::size_t index : renderOrder) {
            objects[index].draw(window);
        }

        if (playMode) {
            window.draw(player);
        }

        // Draw ImGui on top of SFML
        ImGui::SFML::Render(window);

        window.display();
    }

    // Clean up ImGui
    ImGui::SFML::Shutdown();

    return 0;
}