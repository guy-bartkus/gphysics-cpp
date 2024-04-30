#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include "imgui.h"
#include "imgui-SFML.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <stdio.h>
#include "Vec2.h"
#include "ForceBit.h"
#include "Matter.h"
#include "Defines.h"

struct ZoneMatch {
    int8_t zoneIdx = -1;
    float angle = 0;
};

int main()
{
    sf::Mouse mouse;
    sf::Clock deltaClock;
    sf::Clock fpsClock;

    Vec2 windowSize(1000, 1000);
    Vec2 center = windowSize / 2.0f;

    sf::Color bgColor(50, 50, 50);
    sf::CircleShape bgCircle(center.x, 64);
    bgCircle.setFillColor(sf::Color::Black);

    sf::RenderWindow window(sf::VideoMode(windowSize.x, windowSize.y), "GPhysics | Simulation", sf::Style::Titlebar | sf::Style::Close);
    sf::RenderWindow controlsWindow(sf::VideoMode(250, 230), "GPhysics | Controls");
    window.setFramerateLimit(60);

    ImGui::SFML::Init(controlsWindow);

    sf::Vector2i windowPos = window.getPosition();
    controlsWindow.setPosition(sf::Vector2i(windowPos.x+windowSize.x, windowPos.y));

    sf::VertexArray points(sf::Points);
    std::vector<ForceBit> forcebits;
    std::vector<Matter> matters;

    sf::Font roboto;
    if (!roboto.loadFromFile("RobotoMono-Regular.ttf")) {
        std::cerr << "Couldn't find RobotoMono-Regular.ttf" << std::endl;
        exit(1);
    }

    sf::Text cpuTime;
    cpuTime.setCharacterSize(16);
    cpuTime.setFillColor(sf::Color::White);
    cpuTime.setFont(roboto);

    sf::Text gpuTime;
    gpuTime.setCharacterSize(16);
    gpuTime.setFillColor(sf::Color::White);
    gpuTime.setFont(roboto);
    gpuTime.setPosition(sf::Vector2f(0, 20));

    sf::Text fpsText;
    fpsText.setCharacterSize(16);
    fpsText.setFillColor(sf::Color::White);
    fpsText.setFont(roboto);
    fpsText.setPosition(sf::Vector2f(0, 40));
    fpsText.setString("FPS:");

    sf::Text fpsNumber;
    fpsNumber.setCharacterSize(16);
    fpsNumber.setFillColor(sf::Color::Green);
    fpsNumber.setFont(roboto);
    fpsNumber.setPosition(sf::Vector2f(50, 40));

    double totalTimeCPU = 0;
    double totalTimeGPU = 0;
    int totalDelta = 0;
    uint16_t frame = 0;

    float spawnRadius = (windowSize.x / 2);
    const float spawnRadiusSquared = spawnRadius*spawnRadius;
    uint32_t forcebitsSize = 0;
    uint8_t mattersSize = 0;
    int spawnRate = 100;
    int renderAmount = 100;
    int fbSize = 1;
    float fbVelocity = 2.0f;
    const float radiiSum = (ForceBit::s_radius + Matter::s_radius);
    const float radiiSumSquared = (ForceBit::s_radius + Matter::s_radius)* (ForceBit::s_radius + Matter::s_radius);
    bool skip = false;

    fpsClock.restart();

    while (window.isOpen())
    {
        //skip = !skip;
        // Physics update

        std::chrono::high_resolution_clock::time_point cpuStart = std::chrono::high_resolution_clock::now();

        points.clear();

        uint32_t removed = 0;

        for (uint8_t i = 0; i < mattersSize; i++) {
            Matter& matter = matters[i];
            Vec2 newPos = matter.pos + (matter.vel * fbVelocity);

            if (newPos.distance(center) + ((Matter::s_radius + ForceBit::s_radius*2))+1 > spawnRadius) {
                matter.reset();
                continue;
            }

            matter.pos = newPos;
            matter.setPosition(newPos.x, newPos.y);
        }

        for (uint32_t i = 0; i < forcebitsSize;) {
            ForceBit& fb = forcebits[i];
            Vec2 newPos = (fb.vel*fbVelocity) + fb.pos;

            //newPos.x < 0 || newPos.x > windowSize.x || newPos.y < 0 || newPos.y > windowSize.y
            if (newPos.distanceSquared(center) > spawnRadiusSquared) { //newPos.distanceSquared(center) > spawnRadiusSquared
                std::swap(fb, forcebits.back());
                forcebits.pop_back();
                --forcebitsSize;
                continue;
            }
            else {
                fb.pos = newPos;
                fb.steps += fbVelocity;
                if (i / (float)forcebitsSize < renderAmount/100.0f) { // NOTE TO FUTURE SELF: Only calculate this when renderAmount changes instead
                    Vertex v(fb.pos.toVector2f(), fb.color);
                    points.append(v);
                }
                ++i;
            }

            if (fb.passing != nullptr) {
                if (fb.pos.distanceSquared(fb.passing->pos) <= radiiSumSquared) continue;

                fb.passing = nullptr;
            }

            if (skip) continue;

            for (uint8_t i2 = 0; i2 < mattersSize; i2++) {
                Matter& matter = matters[i2];

                Vec2 relPos = fb.pos - matter.pos;

                

                if (fb.pos.distanceSquared(matter.pos) <= radiiSumSquared) {
                    if (fb.type != matter.type) {
                        fb.passing = &matter;
                        fb.switchPolarity();
                        break;

                    }
                    Vec2 dirOpp = fb.vel * -1;

                    ZoneMatch zoneMatch;

                    for (uint8_t i3 = 0; i3 < matter.zoneCount; i3++) {
                        Vec2 zoneDir = matter.zones[i3].dir;

                        if (zoneDir.dot(dirOpp) >= 0) {
                            float angle = zoneDir.angleBetween(dirOpp);
                            //std::cout << "Possible zone: " << (int)i3 << std::endl;

                            if (zoneMatch.zoneIdx == -1 || abs(angle) < abs(zoneMatch.angle)) {
                                zoneMatch.zoneIdx = i3;
                                zoneMatch.angle = angle;
                            }
                        }
                    }

                    //std::cout << "Hit zone #" << (int)zoneMatch.zoneIdx << std::endl;

                    int8_t oppIdx = (zoneMatch.zoneIdx + matter.zoneCount / 2) % matter.zoneCount;

                    Zone& zone = matter.zones[zoneMatch.zoneIdx];
                    Zone& oppZone = matter.zones[oppIdx];

                    // std::cout << zoneMatch.angle << std::endl;
                    matter.addRot(zoneMatch.angle / float(matter.mass - (Matter::s_mass - 1))); // Rotate electron by hit angle offset from matched zone

                    if (oppZone.bits > 0) { // Relfect
                        matter.mass -= 1;
                        // std::cout << (int)matter.mass << std::endl;

                        fb.vel = zone.dir;
                        fb.calcExitSteps(spawnRadius, center);
                        fb.pos = matter.pos + (zone.dir * radiiSum);

                        ForceBit oppForceBit;
                        oppForceBit.pos = matter.pos + (oppZone.dir * radiiSum);
                        oppForceBit.vel = oppZone.dir;
                        oppForceBit.calcExitSteps(spawnRadius, center);
                        oppForceBit.setPolarity(matter.type);
                        forcebits.push_back(oppForceBit);
                        --oppZone.bits;
                        ++forcebitsSize;

                        matter.calcVel();
                        break;
                    }
                    else { // Stick
                        std::swap(fb, forcebits.back());
                        forcebits.pop_back();
                        --forcebitsSize;
                        ++zone.bits;
                        matter.mass += 1;
                        matter.calcVel();
                        break; // To ensure we don't accidentially "stick" this force bit to another tron once it has already stuck to one.
                    }
                }

            }

        }

        //forcebits.reserve(forcebitsSize+spawnRate);

        for (uint32_t i = 0; i < spawnRate; i++) {
            uint8_t type = (rand() > RAND_MAX / 2 ? 0 : 1);
            Vec2 fbPos = Vec2::fromAngle(randFloat * PI2) * (spawnRadius - (float)ForceBit::s_radius) + center;
            Vec2 fbDir = (center - fbPos).rotate(randFloat * (PI_HALF - -PI_HALF) + -PI_HALF).normalize();
            ForceBit fb(fbPos.x, fbPos.y, fbDir, type);
            fb.calcExitSteps(spawnRadius, center);
            forcebits.push_back(fb);
        }

        forcebitsSize += spawnRate;

        // End physics update

        sf::Event event;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space) controlsWindow.setVisible(true);
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    matters.push_back({0, (float)event.mouseButton.x, (float)event.mouseButton.y, 4});
                    mattersSize++;
                }

                if (event.mouseButton.button == sf::Mouse::Right) {
                    matters.push_back({1, (float)event.mouseButton.x, (float)event.mouseButton.y, 4 });
                    mattersSize++;
                }

                if (event.mouseButton.button == sf::Mouse::Middle && mattersSize > 0) {
                    Vec2 mousePos((float)event.mouseButton.x, (float)event.mouseButton.y);
                    for (int16_t i = mattersSize-1; i > -1; i--) {
                        Matter& matter = matters[i];

                        //std::cout << mousePos.distance(matter.pos) << " " << (int)i << std::endl;

                        if (mousePos.distance(matter.pos) <= Matter::s_radius) {
                            matters.erase(matters.begin()+i);
                            --mattersSize;
                            break;
                        }
                    }
                }
            }
        }

        while (controlsWindow.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(controlsWindow, event);

            if (event.type == sf::Event::Closed) {
                controlsWindow.setVisible(false);
            }
        }

        std::chrono::high_resolution_clock::time_point cpuEnd = std::chrono::high_resolution_clock::now();
        totalTimeCPU += std::chrono::duration_cast<std::chrono::microseconds>(cpuEnd - cpuStart).count();

        window.clear(bgColor);

        window.draw(bgCircle);
        glPointSize(fbSize);
        window.draw(points);
        for (uint8_t i = 0; i < mattersSize; i++) window.draw(matters[i]);
        window.draw(cpuTime);
        window.draw(gpuTime);
        window.draw(fpsText);
        window.draw(fpsNumber);

        std::chrono::high_resolution_clock::time_point gpuEnd = std::chrono::high_resolution_clock::now();
        totalTimeGPU += std::chrono::duration_cast<std::chrono::microseconds>(gpuEnd - cpuEnd).count();

        window.display();

        frame++;

        if (frame % 5 == 0) {
            controlsWindow.clear(bgColor);
            ImGui::SFML::Update(controlsWindow, deltaClock.restart());

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
            ImGui::Begin("##main", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
            ImGui::SetWindowPos(ImVec2(10, 10));
            ImGui::SetWindowSize(ImVec2(230, 210));
            ImGui::PushItemWidth(210);

            ImGui::Text("Force bit spawn rate:");
            ImGui::SliderInt("##spawnrate", &spawnRate, 0, 2500, "%d per frame");

            ImGui::Text("Force bit velocity:");
            ImGui::SliderFloat("##velocity", &fbVelocity, 0.5f, 4.0f, "%f");

            ImGui::Text("Force bit size:");
            ImGui::SliderInt("##size", &fbSize, 1, 4, "%d");

            ImGui::Text("Force bits rendered:");
            ImGui::SliderInt("##render", &renderAmount, 0, 100, "%d%%");

            ImGui::Dummy(ImVec2(0, 4));

            if (ImGui::Button("Delete All Matter", ImVec2(210, 0))) {
                matters.resize(0);
                mattersSize = 0;
            }

            ImGui::PopStyleVar();
            ImGui::PopItemWidth();
            ImGui::End();

            ImGui::SFML::Render(controlsWindow);
            controlsWindow.display();
        }

        if (frame >= 30) {
            int cpuAverage = totalTimeCPU / frame;
            int gpuAverage = totalTimeGPU / frame;
            int fpsAverage = fpsClock.restart().asMicroseconds() / frame;
            int fps = 1000000 / fpsAverage;
            totalTimeCPU = 0;
            totalTimeGPU = 0;
            totalDelta = 0;
            frame = 0;

            wchar_t cpuString[16];
            wchar_t gpuString[16];

            swprintf(cpuString, 16, L"CPU: %d %ls", cpuAverage, L"\u00B5s");
            swprintf(gpuString, 16, L"GPU: %d %ls", gpuAverage, L"\u00B5s");

            if (fps > 55) {
                fpsNumber.setFillColor(sf::Color::Green);
            }
            else if (fps > 49) {
                fpsNumber.setFillColor(sf::Color::Yellow);
            }
            else {
                fpsNumber.setFillColor(sf::Color::Red);
            }

            cpuTime.setString(cpuString);
            gpuTime.setString(gpuString);
            fpsNumber.setString(std::to_string(fps));

            std::cout << forcebitsSize << " " << totalDelta << std::endl;
        }
    }
}