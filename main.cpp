#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <random>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <SDL.h>
#include <SDL_opengl.h>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

struct Pixel {int r, g, b;};

enum ProgramState {Home, Loading, Results};

/*#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <random>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "SDL.h"
#include <SDL_opengl.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

struct Pixel {int r, g, b;};

enum ProgramState {Home, Loading, Results};
*/

//convert pixel to hex string
std::string pixelToHex(const Pixel& p) {
  std::stringstream ss;
  ss << std::setfill('0') << std::setw(2) << std::hex << p.r
     << std::setfill('0') << std::setw(2) << std::hex << p.g
     << std::setfill('0') << std::setw(2) << std::hex << p.b;
  return ss.str();
}

std::vector<Pixel> loadPixels(const std::string& filename, GLuint& resultsImageTexture) {
  int width, height, channels;
  
  unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 3);
  
  if (!data) {
    std::cerr << "Failed to load image!" << filename << std::endl;
    return {};
  }
  size_t totalPixels = (size_t)width * height;
  std::vector<Pixel> pixels;
  pixels.reserve(totalPixels);
    
  for (size_t i = 0; i < totalPixels; ++i) {
    size_t index = i * 3;
    pixels.push_back({static_cast<int>(data[index]), static_cast<int>(data[index + 1]),
                     static_cast<int>(data[index + 2])});
  }

  //Generate final image texture
  glGenTextures(1, &resultsImageTexture);
  glBindTexture(GL_TEXTURE_2D, resultsImageTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
  glBindTexture(GL_TEXTURE_2D, 0);
    
  stbi_image_free(data);
  return pixels;
}


//text for palette colors
void paletteOutputText(const std::vector<Pixel>& palette) {
  std::filesystem::path output = std::filesystem::current_path() /"palette.txt";
  
  std::ofstream out(output);
  if (out.is_open()) {
    for (const auto& p : palette) {
      out << pixelToHex(p) << "\n";
    }
    out.close();
    std::cout << "Palete saved to: " << output << '\n';
  }
  else {
    std::cerr << "Error: Unable to open output file.\n";
  }
}

int main(int argc, char** argv) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
    std::cerr << "Error: " << SDL_GetError() << std::endl;
    return -1;
  }

  //Request OpenGl context profile
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

  SDL_Window* window = SDL_CreateWindow("Get Your Palette Here!", SDL_WINDOWPOS_CENTERED, 
                                  SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  if (!window) {
    SDL_Quit();
    return -1;
  }
  
  //Switch to OpenGL Context
  SDL_GLContext gl_context = SDL_GL_CreateContext(window);
  SDL_GL_MakeCurrent(window, gl_context);
  SDL_GL_SetSwapInterval(1); //vsync
  
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  
  ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
  ImGui_ImplOpenGL3_Init("#version 130");
  
  ProgramState state = Home;
  
  std::string draggedImagePath = "";
  std::vector<Pixel> finalPalette; //median cut palette
  GLuint ImageStorage = 0; //stores image as texture for results page
  
  //generating dataset of 120,000 pixels
  std::vector<Pixel> dataset;
  dataset.reserve(120000);
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> colorLimits(0, 255);
  for (int i = 0; i < 120000; i++) {
    dataset.push_back({colorLimits(gen), colorLimits(gen), colorLimits(gen)});
  }
  
  //flag to safely run main algorithms outside for loading screen
  bool trigger = false;
  int loadScreenCount = 0;

  //Main Loop
  bool running = true;
  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) running = false;
  
        //Drag and Drop functionality
        if (event.type == SDL_DROPFILE && state == Home) {
            draggedImagePath = event.drop.file;
            SDL_free(event.drop.file);
        }
    }

    if (state == Loading && trigger) {
      if (loadScreenCount < 1) {
        loadScreenCount++;
      }
      else {
        if (!draggedImagePath.empty()) {
          std::vector<Pixel> imagePixels = loadPixels(draggedImagePath, ImageStorage);
          if (!imagePixels.empty()) {
            //execute algorithms here!!
            //std::vector<Pixel> kMeansPixels = k means function call returning palette(dataset)
            //finalPalette = median mean function call returning 3 color palette(kMeansPixels)
            
            state = Results;
          }
          else {
            state = Home; // fail fallback
          }
        }
        else {
          state = Home; //safety fallback
        }
        trigger = false; //reset flag
        loadScreenCount = 0; //reset counter
      }
    }

    //Starting new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
  
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("Home", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
  
    switch (state) {
      case Home: {
        ImGui::Text("Drag and drop an image here.");
        if (!draggedImagePath.empty()) {
           ImGui::Text("File loaded: %s", draggedImagePath.c_str());
        }
    
        //stops boundary extension
        if (!draggedImagePath.empty()) {
          float windowWidth = ImGui::GetWindowWidth();
          float windowHeight = ImGui::GetWindowHeight();

          //force cursor porition
          float safeY = windowHeight - 80.0f;
          if (safeY < ImGui::GetCursorPosY()) {
            safeY = ImGui::GetCursorPosY();
          } // vertical ceiling check

          ImGui::SetCursorPosY(safeY);
          ImGui::SetCursorPosX((windowWidth - 240.0f) / 2.0f);

          //draw button
          if (ImGui::Button("Extract Palette", ImVec2(240, 52))) {
            state = Loading;
            trigger = true; //trigger main algorithms' execution
            loadScreenCount = 0;
          }

          //boundary validation
          ImGui::Dummy(ImVec2(0.0f, 0.0f));
        }
        break;
      } //case Home closing
  
      case Loading: {
        float windowWidth = ImGui::GetWindowWidth();
        float windowHeight = ImGui::GetWindowHeight();

        // position cursor safely in vertical center
        float middleY = (windowHeight / 2.0f) - 30.0f;
        if (middleY > ImGui::GetCursorPosY()) {
          ImGui::SetCursorPosY(middleY);
        }

        //palette loading text parameters
        ImGui::SetCursorPosX((windowWidth - ImGui::CalcTextSize("Palette Loading...").x) / 2.0f);
        ImGui::Text("Palette Loading...");

        //removed loading bar functionality becaus of partial implementation
        //ImGui::SetCursorPosX((windowWidth - 300.0f) / 2.0f);
        //ImGui::ProgressBar(0.5f, ImVec2(300, 20));

        ImGui::Dummy(ImVec2(0.0f, 0.0f)); //seals cursor adjustment
        break;
      } //case Loading closing
  
      case Results: {
        float windowWidth = ImGui::GetWindowWidth();

        ImGui::BeginChild("Left Image", ImVec2(windowWidth * 0.5f, 0), false);
        if (ImageStorage != 0) {
          ImGui::Image((ImTextureID)(intptr_t)ImageStorage, ImVec2(400, 400));
        }
        
        ImGui::Spacing();
        if (ImGui::Button("Reset", ImVec2(120, 40))) {
            draggedImagePath.clear();
            finalPalette.clear();
            state = Home;
        }
        ImGui::EndChild();
    
        ImGui::SameLine();
    
        //Right Side: Palette
        ImGui::BeginChild("Right Column", ImVec2(0, 0), false);
        ImGui::Text("Color Palette:");
        ImGui::Separator();
        ImGui::Spacing();

        for (size_t i = 0; i < finalPalette.size(); ++i) {
          std::string hexString = pixelToHex(finalPalette[i]);
          ImVec4 color = ImVec4(finalPalette[i].r / 255.0f, finalPalette[i].g / 255.0f, finalPalette[i].b / 255.0f, 1.0f);

          ImGui::ColorButton(("##Color" + std::to_string(i)).c_str(), color, ImGuiColorEditFlags_NoAlpha, ImVec2(100, 100));
          ImGui::Text("%s", hexString.c_str());
          ImGui::Spacing();
          ImGui::Spacing();
        }

        if (ImGui::Button("Copy Palette", ImVec2(150, 40))) {
          paletteOutputText(finalPalette);
        }
        ImGui::EndChild();
        break;
      } //case Results closing
    } //switch closing
    
    ImGui::End(); //End home window
    ImGui::Render();

    glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
    glClearColor(0.01f, 0.1f, 0.1f, 0.1f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
  }
  
  //clean up  
  if (ImageStorage != 0) {
    glDeleteTextures(1, &ImageStorage);
  }
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
} //main function closing
