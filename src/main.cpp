#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <random>
#include <cmath>
#include <algorithm>
#include <numeric>
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

//convert pixel to hex string
std::string pixelToHex(const Pixel& p) {
  std::stringstream ss;
  ss << std::setfill('0') << std::setw(2) << std::hex << p.r
     << std::setfill('0') << std::setw(2) << std::hex << p.g
     << std::setfill('0') << std::setw(2) << std::hex << p.b;
  return ss.str();
}

std::vector<Pixel> loadPixels(const std::string& filename, GLuint& resultsImageTexture, int imageWidth, int imageHeight) {
  int width, height, channels;
  
  unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 3);
  
  //variables only passed to be updated
  imageWidth = width;
  imageHeight = height;
  
  if (!data) {
    std::cerr << "Failed to load image!" << filename << std::endl;
    return {};
  }
  size_t totalPixels = (size_t)width * height;
  std::vector<Pixel> pixels;
  pixels.reserve(totalPixels);
    
  for (size_t i = 0; i < totalPixels; ++i) {
    size_t index = i * 3;
    pixels.push_back({static_cast<int>(data[index]), 
                      static_cast<int>(data[index + 1]),
                      static_cast<int>(data[index + 2])
                    });
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

//kmeans helper function
double distance(const Pixel& p, const Pixel& pix) {
  return std::sqrt(std::pow(p.r - pix.r, 2) + std::pow(p.g - pix.g, 2) + std::pow(p.b - pix.b, 2));
}

Pixel average(const std::vector<Pixel>& cluster) {
  if (cluster.empty()) {
    return {0, 0, 0};
  }
  
  long long total_r = 0;
  long long total_g = 0;
  long long total_b = 0;

  for (const auto& p : cluster) {
    total_r += p.r;
    total_g += p.g;
    total_b += p.b;
  }
  double count = static_cast<double>(cluster.size());
  return {static_cast<int>(std::round(total_r / count)), 
          static_cast<int>(std::round(total_g / count)), 
          static_cast<int>(std::round(total_b / count))};
}

//k means lightness calculator
double calculateLightness(const Pixel& p) {
  return 0.299 * p.r + 0.587 * p.g + 0.114 * p.b;
}

//k means algorithm
std::vector<Pixel> kMeans(const std::vector<Pixel>& inputPixels) {
  //to do: implement algorithm
  if (inputPixels.empty()) return {};

  std::vector<Pixel> centroids;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, inputPixels.size() - 1);

  //pick clusters randomly to allow for better variation potential
  centroids.push_back(inputPixels[dis(gen)]);

  //initialization
  for (int i = 1; i < 3; ++i) {
    std::vector<double> squared(inputPixels.size());
    double sum = 0;

    for (size_t j = 0; j < inputPixels.size(); j++) {
      double minimum = -1.0;
      for (const auto& centroid : centroids) {
        double d = distance(inputPixels[j], centroid);
        if (minimum == -1.0 || d < minimum) {
          minimum = d;
        }
      }
      squared[j] = minimum * minimum;
      sum += squared[j];
    }

    std::uniform_real_distribution<double> real(0.0, sum);
    double target = real(gen);
    double current = 0;

    for (size_t j = 0; j < inputPixels.size(); ++j) {
      current += squared[j];
      if (current >= target) {
        centroids.push_back(inputPixels[j]);
        break;
      }
    }
  }

  //clustering
  for (int k = 0; k < 100; ++k) {
    std::vector<std::vector<Pixel>> clusters(3);

    //assignment
     for (const auto& pixel : inputPixels) {
      double minimum = -1.0;
      int best = 0;

      for (int l = 0; l < 3; ++l) {
        double d = distance(pixel, centroids[l]);
        if (minimum == -1.0 || d < minimum) {
          minimum = d;
          best = l;
        }
      }
      clusters[best].push_back(pixel);
     }

     std::vector<Pixel> newCentroids(3);
     bool changed = false;

     for (int i = 0; i < 3; ++i) {
      if (clusters[i].empty()) {
        //edge case: if no assigned pixels
        newCentroids[i] = inputPixels[dis(gen)];
        changed = true;
      }
      else {
        newCentroids[i] = average(clusters[i]);

        //distance check
        if (distance(newCentroids[i], centroids[i]) > 0.001) {
          changed = true;
        }
      }
    }

     centroids = newCentroids;
     if (!changed) break;
  }

  //sort palette based on lightness
  std::sort(centroids.begin(), centroids.end(), [](const Pixel& a, const Pixel& b) {
    return calculateLightness(a) < calculateLightness(b);
  });
  
  return centroids;
}

//median mean algorithm
std::vector<Pixel> MedianMean(const std::vector<Pixel>& kPixels) {
  //to do: implement algorithm
  //mock implementation:
  std::vector<Pixel> mockPixels = { {255, 0, 0}, {0, 255, 0}, {0, 0, 255} };
  return mockPixels;
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
    std::cout << "Palette saved to: " << output << '\n';
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
  std::vector<Pixel> kMeansPalette; //k means palette
  std::vector<Pixel> MedianPalette; //median cut palette
  std::vector<Pixel> imagePixels; //vector of all image pixels
  GLuint ImageStorage = 0; //stores image as texture for results page
  int imageWidth = 0, imageHeight = 0; //dimensions tracker
  std::string errorMessage = ""; //to safely pass size error message to UI

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
            errorMessage.clear();
            SDL_free(event.drop.file);
        }
    }

    if (state == Loading && trigger) {
      if (loadScreenCount < 1) {
        loadScreenCount++;
      }
      else {
        if (!draggedImagePath.empty()) {
          imagePixels = loadPixels(draggedImagePath, ImageStorage, imageWidth, imageHeight);
          if (!imagePixels.empty()) {
            if ((imageWidth * imageHeight) >= 100000) { 
              //execute algorithms here!!
              kMeansPalette = kMeans(imagePixels);
              //MedianPalette = MedianMean(kMeansPixels)
              state = Results;
            }
            else {
              errorMessage= "Image too small: please drag and drop a larger image.";
              state = Home;
            }
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

        if (!errorMessage.empty()) {
          ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", errorMessage.c_str());
        }
    
        //stops boundary extension
        if (!draggedImagePath.empty()) {
          float windowWidth = ImGui::GetWindowWidth();
          float windowHeight = ImGui::GetWindowHeight();

          //force cursor position
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
          ImGui::Dummy(ImVec2(0.0f, safeY + 52.0f));
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
            kMeansPalette.clear();
            MedianPalette.clear();
            state = Home;
        }
        ImGui::EndChild();
    
        ImGui::SameLine();
    
        //Middle Column: K-Means Palette
        ImGui::BeginChild("Middle Column", ImVec2(windowWidth * 0.33f, 0), false);
        ImGui::Text("K-Means Palette:");
        ImGui::Separator();
        ImGui::Spacing();

        for (size_t i = 0; i < kMeansPalette.size(); ++i) {
          std::string hexString = pixelToHex(kMeansPalette[i]);
          ImVec4 color = ImVec4(kMeansPalette[i].r / 255.0f, kMeansPalette[i].g / 255.0f, kMeansPalette[i].b / 255.0f, 1.0f);

          ImGui::ColorButton(("##KMeansColor" + std::to_string(i)).c_str(), color, ImGuiColorEditFlags_NoAlpha, ImVec2(80, 80));
          ImGui::Text("%s", hexString.c_str());
          ImGui::Spacing();
          ImGui::Spacing();
        }

        if (ImGui::Button("Copy K-Means Palette", ImVec2(150, 40))) {
          paletteOutputText(kMeansPalette);
        }
        ImGui::EndChild();
        ImGui::SameLine();

        //Right Side: Palette
        ImGui::BeginChild("Right Column", ImVec2(0, 0), false);
        ImGui::Text("Median Cut Palette:");
        ImGui::Separator();
        ImGui::Spacing();

        for (size_t i = 0; i < MedianPalette.size(); ++i) {
          std::string hexString = pixelToHex(MedianPalette[i]);
          ImVec4 color = ImVec4(MedianPalette[i].r / 255.0f, MedianPalette[i].g / 255.0f, MedianPalette[i].b / 255.0f, 1.0f);

          ImGui::ColorButton(("##MedianColor" + std::to_string(i)).c_str(), color, ImGuiColorEditFlags_NoAlpha, ImVec2(80, 80));
          ImGui::Text("%s", hexString.c_str());
          ImGui::Spacing();
          ImGui::Spacing();
        }

        if (ImGui::Button("Copy Median Palette", ImVec2(150, 40))) {
          paletteOutputText(MedianPalette);
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
