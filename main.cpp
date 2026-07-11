#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <random>
#include <filesystem>

#define Image_Processing
#include <std_image.h>

#include <SDL.h>
#include <sdl_opengl.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

struct Pixel {int r, g, b;};

enum ProgramState {Home, Loading, Results};

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
  
  unsigned char* data = stbi_load(filname.c_str(), &width, &height, &channels, 3);
  
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
  glGenTexture(1, &resultsImageTexture);
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
      out << pixeltoHex(p) << "\n";
    }
    out.close();
    std::cout << Palete saved to: " << output << '\n';
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

  SDL_Window* window = SDL_CreateWindow("Get Your Palette Here!", SDL_WINDOWPOS_CENTERED, 
                                  SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_RESIZEABLE);
  if (!window) {
    SDL_Quit();
    return -1;
  }
  
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!renderer) {
    SDL_DestroyWindow(window);
    SDL_Quit();
    return -1;
  }
  
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  
  ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer2_Init(renderer);
  
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

    //Starting new frame
    ImGui_ImplSDLRenderer2_NewFrame();
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
    
        ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() / 2 - 120, ImGui::GetWindowHeight() - 78));
        if (!draggedImagePath.empty() && ImGui::Button("Extract Palette", ImVec2(240, 52))) {
          state = Loading;
        }
        break;
      } //case Home closing
  
      case Loading: {
        //SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        //ImGui_ImplSDLRenderer2_DrawData(ImGui::GetDrawData());
      
        //SDL_RenderPresent(renderer);
        if (!draggedImagePath.empty()) {
          //load image and get list of [R,G,B} values
          std::vector<pixel> imagePixels = loadPixels(draggedImagePath, ImageStorage);
          if (!imagePixels.empty()) {
            //std::vector<Pixel> kMeansPixels = k means function call returning palette(dataset)
            //finalPalette = median mean function call returning 3 color palette(kMeansPixels)
            state = Results;
          }
          else {
            ImGui::Text("Error loading image...");
            state = Home; //loading fail
          }
          ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() / 2 - 60, ImGui::GetWindowHeight() / 2));
          ImGui::Text("Palette Loading...");
          ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() / 2 - 60, ImGui::GetWindowHeight() / 2 + 30));
          ImGui::ProgressBar(0.5f, ImVec2(300, 20));
          break;
        }
      } //case Loading closing
  
      case Results: {
        //ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        //ImGui::Begin("Results", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
    
        //Left Side: Image
        ImGui::BeginChild("Left Image", ImVec2(ImGui::GetWindowWidth() * 0.5f, 0), false);
        if (ImageStorage != 0) {
          ImGui::Image((void*)(intptr_t)ImageStorage, ImVec2(400, 400));
        }
        
        if (ImGui::Button("Reset")) {
            draggedImagePath.clear();
            finalPalette.clear();
            state = Home;
        }
        ImGui::EndChild();
    
        ImGui::SameLine();
    
        //Right Side: Palette
        ImGui::BeginChild("Right Column", ImVec2(0, 0), false);
        ImGui::Text("Color Palette:");
        for (size_t i = 0; i < finalPalette.size(); ++i) {
          std::string hexString = pixelToHex(finalPalette[i]);
          ImVec4 color = ImVec4(finalPalette[i].r / 255.0f, finalPalette[i].g / 255.0f, finalPalette[i].b / 255.0f, 1.0f);

          ImGui::ColorButton(("##Color" + std::to_string(i)).c_str(), color, ImGuiColorEditFlags_NoAlpha, ImVec2(100, 100));
          ImGui::Text("%s", hexString.c_str());
          ImGui::Spacing();
        }

        if (ImGui::Button("Copy Palette")) {
          paletteOutputText(finalPalette);
        }
        ImGui::EndChild();
        break;
      } //case Results closing
    } //switch closing
    
    ImGui::End(); //End home window
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(renderer);
  } //while(running) closing
  
  //clean up  
  ImGui_ImplSDLRenderer2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
} //main function closing
