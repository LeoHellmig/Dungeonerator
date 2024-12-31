#include <iostream>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include "dungeonerator.hpp"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

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

    window = SDL_CreateWindow("Dungenerator", SCR_WIDTH, SCR_HEIGHT, flags);

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

    std::vector<Grammar::Symbol> alphabet = {{true, "g"}, {true, "t"}};
    // std::unique_ptr<std::list<Grammar::Symbol>> nonTerminals;
    // nonTerminals->emplace_back(false, "S");
    // nonTerminals->emplace_back(false, "T");

    std::vector<Grammar::Rule> rules;
    Grammar::Rule rule;
    rule.lhs = {{false, "S"}};
    rule.rhs = {{false, "T"}, {true, "g"}};
    rules.emplace_back(rule);
    rule.lhs = {{false, "T"}};
    rule.rhs = {{true, "t"}, {false, "T"}};
    rules.emplace_back(rule);
    rule.lhs = {{false, "T"}};
    rule.rhs = {{true, "t"}};
    rules.emplace_back(rule);

    Grammar grammar;

    grammar.SetAlphabet(alphabet);
    grammar.SetRules(rules);


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

//
//
//
        ImGui::Begin("Hello");
        ImGui::DragFloat3("clear color", &clearColor.x, 1, 0, 255);
        ImGui::End();


        if (applyGrammar) {
            grammar.ApplyRules({false, "S"});

            for (Grammar::Symbol& symbol : grammar.mString) {
                std::cout << symbol.name << std::endl;
            }

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
