#include <iostream>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include "grammar.hpp"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "graph.hpp"
#include "dungeonerator.hpp"

#pragma warning(disable: 4201) // Disable MSVC extension warnings
#include <SDL3/SDL_opengl.h>

constexpr int SCR_WIDTH = 1280;
constexpr int SCR_HEIGHT = 720;

constexpr float PI = 3.14159265359f;
constexpr float TWOPI = PI * 2.f;

SDL_Window* window;
SDL_Surface* surface;
uint32_t* pixels;

int main()
{
    std::cout << "Hello, World!" << std::endl;

    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {
        SDL_Log("SDL failed to initialize");
        return -2;
    }

    unsigned int flags = 0;

    window = SDL_CreateWindow("Dungeonerator", SCR_WIDTH, SCR_HEIGHT, flags);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    SDL_SetRenderVSync(renderer, 1);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsClassic();

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);


    surface = SDL_GetWindowSurface(window);
    pixels = static_cast<uint32_t*>(surface->pixels);

    bool running = true;
    bool showDemoWindow = true;

    glm::vec4 clearColor = {1.0f, 1.0f, 1.0f, 1.0f};

    bool applyGrammar = false;

    SymbolRegistry reg;
    reg.AddSymbol({false, "S"});
    reg.AddSymbol({false, "T"});

    reg.AddSymbol({true, "g"});
    reg.AddSymbol({true, "t"});

    std::vector<Rule> rules;
    Rule rule;
    rule.lhs = {"S"};
    rule.rhs = {{"T"}, {"g"}};
    rules.emplace_back(rule);
    rule.lhs = {"T"};
    rule.rhs = {{"t"}, {"T"}};
    rules.emplace_back(rule);
    rule.lhs = {"T"};
    rule.rhs = {"t"};
    rules.emplace_back(rule);

    Grammar formalGrammar(reg, rules);

    reg.AddSymbol({true, "f"});
    reg.AddSymbol({true, "r"});

    std::vector<Rule> rewriteRules;

    rule.lhs = {"t", "g"};
    rule.rhs = {"t", "r", "g"};
    rewriteRules.emplace_back(rule);
    rule.lhs = {"t", "t"};
    rule.rhs = {"t", "r", "t"};
    rewriteRules.emplace_back(rule);
    rule.lhs = {"t"};
    rule.rhs = {"f"};
    rewriteRules.emplace_back(rule);

    Grammar rewriteGrammar(reg, rewriteRules);

    Graph<VertexData, EdgeData> myGraph;

    auto a = myGraph.AddVertex(VertexData(0));
    auto b = myGraph.AddVertex(VertexData(1));
    auto c = myGraph.AddVertex(VertexData(2));

    myGraph.AddEdge(a, b, EdgeData(4));
    myGraph.AddEdge(b, c, EdgeData(5));
    myGraph.AddEdge(c, a, EdgeData(6));

    {
        auto& vertices = myGraph.VertexData();

        std::cout << "-----Vertices-----" << std::endl;

        for (const auto & vertex : vertices)
        {
            std::cout << vertex.first << ": data: " << vertex.second._data.a << std::endl;
            std::cout << "edges: " << std::endl;

            for (EdgeId edge : vertex.second._edges) {
                std::cout << edge << std::endl;
            }
        }

        auto& edges = myGraph.EdgeData();

        std::cout << "-----Edges-----" << std::endl;

        for (const auto& edge : edges) {

            std::cout << edge.first << ": data: " << edge.second._data.weight << std::endl;

            std::cout << edge.second._v1 << " " << edge.second._v2 << std::endl;
        }
    }

    myGraph.RemoveEdge(0, 1);

    myGraph.RemoveVertex(2);


    {
        auto& vertices = myGraph.VertexData();

        std::cout << "-----Vertices-----" << std::endl;

        for (const auto & vertex : vertices)
        {
            std::cout << vertex.first << ": data: " << vertex.second._data.a << std::endl;
            std::cout << "edges: " << std::endl;

            for (EdgeId edge : vertex.second._edges) {
                std::cout << edge << std::endl;
            }
        }

        auto& edges = myGraph.EdgeData();

        std::cout << "-----Edges-----" << std::endl;

        for (const auto& edge : edges) {

            std::cout << edge.first << ": data: " << edge.second._data.weight << std::endl;

            std::cout << edge.second._v1 << " " << edge.second._v2 << std::endl;
        }
    }

    Dungeon::DungeonGenerationData generationData(50, 10, 0.5, 5.0);
    Dungeon myDungeon(generationData, 100.f, 100.f);
    myDungeon.Generate();

    std::cout << "Vertices: " << std::endl;
    for (int i = 0; i < myDungeon.mVertices.size(); i++) {
        auto& vertex = myDungeon.mVertices[i];

        std::cout << "Vertex " << i << " at: " << vertex.mPx << ", " << vertex.mPy << " size of: " << vertex.mSize << std::endl;

        std::cout << "Edges: ";
        for (auto& edge : vertex.mConnections) {
            std::cout << edge << ", ";
        }

        std::cout << std::endl;
    }

    std::cout << "Edges: " << std::endl;

    for (int i = 0; i < myDungeon.mEdges.size(); i++) {
        auto& edge = myDungeon.mEdges[i];
        std::cout << i << ": " << edge.mNode1 << ", " << edge.mNode2 << std::endl;
    }

    while (running)
    {
        SDL_Event event;
        if (SDL_PollEvent(&event) > 0)
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            switch (event.type)
            {
                case SDL_EVENT_KEY_DOWN:
                if (event.key.key == SDLK_ESCAPE) {
                    running = false;
                }
                if (event.key.key == SDLK_P) {

                    std::cout << "Saving frame to BMP" << std::endl;
                    SDL_SaveBMP(surface, "out.bmp");
                    // framebuffer to png
                }
                if (event.key.key == SDLK_G) {
                    applyGrammar = true;
                }
                break;
                case SDL_EVENT_QUIT:
                    running = false;
                break;
            }
        }
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        if (showDemoWindow) {
            ImGui::ShowDemoWindow(&showDemoWindow);
        }


        ImGui::Begin("Hello");
        ImGui::DragFloat3("clear color", &clearColor.x, 1, 0, 255);
        ImGui::End();

        if (applyGrammar) {
            formalGrammar.PrintInfo();

            std::list<SymbolRegistry::SymbolID> startString = {reg.GetSymbol("S")};
            formalGrammar.ExecuteGrammar(startString);

            std::cout << "Grammar output: ";
            for (const auto& symbol : formalGrammar.GetString()) {
                std::cout << reg.GetSymbolData(symbol)->name;
            }
            endl(std::cout);

            rewriteGrammar.PrintInfo();
            rewriteGrammar.ExecuteGrammar(formalGrammar.GetString());

            std::cout << "Rewrite grammar output: ";
            for (const auto& symbol : rewriteGrammar.GetString()) {
                std::cout << reg.GetSymbolData(symbol)->name;
            }
            endl(std::cout);

            applyGrammar = false;
        }




//
//
//

        ImGui::Render();

        SDL_SetRenderDrawColor(renderer, static_cast<uint8_t>(clearColor.x), static_cast<uint8_t>(clearColor.y), static_cast<uint8_t>(clearColor.z), static_cast<uint8_t>(clearColor.w));
        SDL_RenderClear(renderer);

        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
