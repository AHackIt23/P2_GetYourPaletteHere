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
#include <stb/stb_image.h>

#include <SDL.h>
#include <SDL_opengl.h>
#include <GL/gl.h>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include "MedianCut.h"
#include "Color.h"

// helper to check OpenGL errors
void checkGLError(const char* location) {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error at " << location << ": " << err << std::endl;
    }
}

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

std::vector<Pixel> loadPixels(const std::string& filename, GLuint& resultsImageTexture, int& imageWidth, int& imageHeight) {
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

  // ONLY create texture if it doesn't exist or is 0!!!!!
  if (resultsImageTexture == 0) {
    glGenTextures(1, &resultsImageTexture);
    
    //DEBUG INFO RIGHT HERE!
    std::cout << "Texture ID: " << resultsImageTexture << std::endl;
    std::cout << "Image dimensions: " << width << "x" << height << std::endl;
  }
  
  glBindTexture(GL_TEXTURE_2D, resultsImageTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
  }
  //checkGLError("loadPixels - glTexImage2D");
  //checkGLError("loadPixels - glBindTexture");
  //glBindTexture(GL_TEXTURE_2D, 0);
  
  stbi_image_free(data);
  
  std::cout << "Texture created with ID: " << resultsImageTexture << std::endl;
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
std::vector<Pixel> kMeans(const std::vector<Pixel>& inputPixels, int cuts) {
  //to do: implement algorithm
  if (inputPixels.empty()) return {};

  std::vector<Pixel> centroids;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, inputPixels.size() - 1);

  //pick clusters randomly to allow for better variation potential
  centroids.push_back(inputPixels[dis(gen)]);

  //initialization
  for (int i = 1; i < cuts; ++i) {
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

    if (sum <= 0.0001) {
      centroids.push_back(inputPixels[dis(gen)]);
      continue;
    }

    std::uniform_real_distribution<double> real(0.0, sum);
    double target = real(gen);
    double current = 0;
    bool added = false;

    for (size_t j = 0; j < inputPixels.size(); ++j) {
      current += squared[j];
      if (current >= target) {
        centroids.push_back(inputPixels[j]);
        added = true;
        break;
      }
    }
    
    if (!added) {
      centroids.push_back(inputPixels[dis(gen)]);
    }
  }

  //clustering
  for (int k = 0; k < 100; ++k) {
    std::vector<std::vector<Pixel>> clusters(cuts);

    //assignment
     for (const auto& pixel : inputPixels) {
      double minimum = -1.0;
      int best = 0;

      for (int l = 0; l < cuts; ++l) {
        double d = distance(pixel, centroids[l]);
        if (minimum == -1.0 || d < minimum) {
          minimum = d;
          best = l;
        }
      }
      clusters[best].push_back(pixel);
     }

     std::vector<Pixel> newCentroids(cuts);
     bool changed = false;

     for (int i = 0; i < cuts; ++i) {
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

// median cut algorithm
std::vector<Pixel> MedianCutPalette(const std::vector<Pixel>& imagePixels, int numColors) {

std::vector<Color> colors;
colors.reserve(imagePixels.size());
for (const auto& p : imagePixels) {

colors.push_back(Color(p.r, p.g, p.b));
}

std::vector<Color> paletteColors = MedianCut::extractPalette(colors, numColors);

std::vector<Pixel> finalPalette;
finalPalette.reserve(paletteColors.size());
for (const auto& c : paletteColors) {
  finalPalette.push_back({c.r, c.g, c.b});
}
return finalPalette;

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
    std::cout << "Palette saved to: " << output.string() << '\n';
  }
  else {
    std::cerr << "Error: Unable to open output file.\n";
  }
}

// Earthy palette for UI<3

void setAestheticStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // only the colors we actually use
    ImVec4 softBeige = ImVec4(0.925f, 0.902f, 0.855f, 1.0f);    // #ECE6DA - background
    ImVec4 mossGreen = ImVec4(0.306f, 0.380f, 0.357f, 1.0f);    // #4E615B - buttons
    ImVec4 darkNavy = ImVec4(0.122f, 0.145f, 0.196f, 1.0f);     // #1F2532 - text
    
    // background
    style.Colors[ImGuiCol_WindowBg] = softBeige;
    style.Colors[ImGuiCol_ChildBg] = softBeige;
    
    // text
    style.Colors[ImGuiCol_Text] = darkNavy;
    style.Colors[ImGuiCol_TextDisabled] = mossGreen;
    
    // buttons
    style.Colors[ImGuiCol_Button] = mossGreen;
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.4f, 0.48f, 0.45f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = darkNavy;
    
    // frame
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.92f, 0.90f, 0.86f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = softBeige;
    style.Colors[ImGuiCol_FrameBgActive] = mossGreen;
    
    // headers
    style.Colors[ImGuiCol_Header] = mossGreen;
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.4f, 0.48f, 0.45f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = darkNavy;
    
    // scrollbar
    style.Colors[ImGuiCol_ScrollbarBg] = softBeige;
    style.Colors[ImGuiCol_ScrollbarGrab] = mossGreen;
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.4f, 0.48f, 0.45f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = darkNavy;
    
    // titles
    style.Colors[ImGuiCol_TitleBg] = mossGreen;
    style.Colors[ImGuiCol_TitleBgActive] = darkNavy;
    style.Colors[ImGuiCol_TitleBgCollapsed] = mossGreen;
    
    // rounded corners
    style.WindowRounding = 16.0f;
    style.ChildRounding = 12.0f;
    style.FrameRounding = 10.0f;
    style.GrabRounding = 10.0f;
    style.PopupRounding = 12.0f;
    style.ScrollbarRounding = 10.0f;
    style.TabRounding = 10.0f;
    
    // padding/spacing
    style.WindowPadding = ImVec2(20.0f, 20.0f);
    style.FramePadding = ImVec2(12.0f, 8.0f);
    style.ItemSpacing = ImVec2(12.0f, 10.0f);
    style.ItemInnerSpacing = ImVec2(10.0f, 6.0f);
    
    // no borders
    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 0.0f;
    style.PopupBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.TabBorderSize = 0.0f;
}

int main(int argc, char** argv) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
    std::cerr << "Error: " << SDL_GetError() << std::endl;
    return -1;
  }

  //Request OpenGl context profile
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

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
  setAestheticStyle();//we'll be doing a muted green aesthetic style<3
  
  ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
  ImGui_ImplOpenGL3_Init("#version 120");
  
  ProgramState state = Home;
  
  std::string draggedImagePath = "";
  std::vector<Pixel> kMeansPalette; //k means palette
  std::vector<Pixel> finalPalette; //median cut palette
  std::vector<Pixel> imagePixels; //vector of all image pixels
  GLuint ImageStorage = 0; //stores image as texture for results page
  int imageWidth = 0, imageHeight = 0; //dimensions tracker
  std::string errorMessage = ""; //to safely pass size error message to UI
  int m_selectedColors = 5; // Default to 5 colors

  //flag to safely run main algorithms outside for loading screen
  bool trigger = false;
  float loadingProgress = 0.0f;

  //Main Loop
  bool running = true;
  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT) running = false;
  
      //Drag and Drop functionality
      if (event.type == SDL_DROPFILE) {
        if (state == Home) {
          draggedImagePath = event.drop.file;
          errorMessage.clear();
          state = Loading;
          trigger = true;
          loadingProgress = 0.0f;
        }
        SDL_free(event.drop.file); //avoids memory leaks by mving outisde of state check
      }
    }

    if (state == Loading && trigger) { //allows for a full iteration so that load screen will appear
      if (loadingProgress < 1.0f) {
        loadingProgress += 0.05f;
      }
      else {
        state = Results;
      }
    }

    //Starting new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
  
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(1280, 720));
    ImGui::Begin("Home", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
  
    switch (state) {
      case Home: {
        float windowWidth = ImGui::GetWindowWidth();
        float windowHeight = ImGui::GetWindowHeight();
          
        //center
        float centerX = windowWidth / 2;

        if (!errorMessage.empty()) {
          ImGui::SetCursorPosX(centerX - ImGui::CalcTextSize(errorMessage.c_str()).x / 2);
          ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.3f, 1.0f), "%s", errorMessage.c_str());
          ImGui::Spacing();
        }

        // upload image text
        ImGui::SetCursorPosX(centerX - ImGui::CalcTextSize("Upload Image").x / 2);
        ImGui::Text("Upload Image");
        
        // drag and drop
        float rectWidth = 400;
        float rectHeight = 180;
        float rectX = (windowWidth - rectWidth) / 2;
        float rectY = ImGui::GetCursorPosY() + 10;
          
        // rectangle
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 rectMin = ImVec2(rectX, rectY);
        ImVec2 rectMax = ImVec2(rectX + rectWidth, rectY + rectHeight);
          
        drawList->AddRectFilled(rectMin, rectMax, IM_COL32(235, 225, 215, 255));
        drawList->AddRect(rectMin, rectMax, IM_COL32(200, 190, 180, 255));
          
        // drag and drop text centered
        const char* dragText = "Drag and drop an image here.";
        ImVec2 dragTextSize = ImGui::CalcTextSize(dragText);
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 dragTextPos = ImVec2( windowPos.x + rectX + (rectWidth - dragTextSize.x) / 2,
                                    windowPos.y + rectY + (rectHeight - dragTextSize.y) / 2);
        drawList->AddText(dragTextPos, IM_COL32(128, 115, 102, 255), dragText);

        // move cursor out rectangle
        ImGui::NewLine();
        ImGui::SetCursorPosY(rectY + rectHeight + 15);
        
        // File loaded indicator
        if (!draggedImagePath.empty()) {
          std::string fileName = draggedImagePath.substr(draggedImagePath.find_last_of("/\\") + 1);
          std::string loadedText = "✓ File loaded: " + fileName;
          float loadedTextWidth = ImGui::CalcTextSize(loadedText.c_str()).x;
          ImGui::SetCursorPosX(centerX - loadedTextWidth / 2);
          ImGui::TextColored(ImVec4(0.3f, 0.6f, 0.3f, 1.0f), "%s", loadedText.c_str());
          ImGui::Spacing();
        } 
        
        //Codespace local file input path field
        static char manualPath[256] = "";
          
        //center alighment for input box
        float inputFieldWidth = 200.0f;
        float loadBWidth = 70.0f;
        float inputGroupTotalW = inputFieldWidth + 10.0f + loadBWidth;

        ImGui::SetCursorPosX(centerX - inputGroupTotalW / 2);

        //render text input field + load trigger button
        ImGui::SetNextItemWidth(inputFieldWidth);
        ImGui::InputText("##ManualPath", manualPath, IM_ARRAYSIZE(manualPath));
        ImGui::SameLine();
        if (ImGui::Button("Load", ImVec2(loadBWidth, 0))) {
          if (manualPath[0] != '\0') {
            draggedImagePath = manualPath;
            errorMessage.clear();
          }
        }

        // move cursor out rectangle
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 25);

        //num of colors centered
        std::string colorsText = "Number of Colors:";
        float colorsTextWidth = ImGui::CalcTextSize(colorsText.c_str()).x;
        float radioWidth = 150; // Approximate width of 3 radio buttons
        float totalWidth = colorsTextWidth + 20 + radioWidth;
        ImGui::SetCursorPosX(centerX - totalWidth / 2);
        ImGui::Text("%s", colorsText.c_str());
        ImGui::SameLine();
        
        // radio buttons with visible!! background
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.85f, 0.80f, 0.75f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.90f, 0.85f, 0.80f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.3f, 0.5f, 0.4f, 1.0f));
          
        if (ImGui::RadioButton("3", m_selectedColors == 3)) m_selectedColors = 3;
        ImGui::SameLine();
        if (ImGui::RadioButton("4", m_selectedColors == 4)) m_selectedColors = 4;
        ImGui::SameLine();
        if (ImGui::RadioButton("5", m_selectedColors == 5)) m_selectedColors = 5;
          
        ImGui::PopStyleColor(3);
        
        // extract palette
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 35);
        ImGui::SetCursorPosX(centerX - 120);
        
        if (draggedImagePath.empty()) {
          ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
          ImGui::Button("Extract Palette", ImVec2(240, 52));
          ImGui::PopStyleColor();
        } else {
          if (ImGui::Button("Extract Palette", ImVec2(240, 52))) {
            state = Loading;
            trigger = true; //trigger main algorithms' execution
            loadingProgress = 0.0f;
          }
        }
        
        break;
      } //case Home closing
      
      case Loading: {
        float windowWidth = ImGui::GetWindowWidth();
        float windowHeight = ImGui::GetWindowHeight();

        //palette loading text parameters
        ImGui::SetCursorPosY((windowHeight / 2.0f) - 40.0f);
        ImGui::SetCursorPosX((windowWidth - ImGui::CalcTextSize("Palette Loading...").x) / 2.0f);
        ImGui::Text("Palette Extracting...");

        ImGui::Spacing();
        ImGui::SetCursorPosX((windowWidth - 300.0f) / 2.0f);
        
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.306f, 0.380f, 0.357f, 1.0f));
        ImGui::ProgressBar(loadingProgress, ImVec2(300, 20), "");
        ImGui::PopStyleColor();

        if (trigger) {
          loadingProgress += 0.05f;
          if (loadingProgress >= 1.0f) {
            if (!draggedImagePath.empty()) {
              imagePixels = loadPixels(draggedImagePath, ImageStorage, imageWidth, imageHeight);
              if (!imagePixels.empty()) {
                if ((imageWidth * imageHeight) >= 100000) { //images serve as dataset
                  //the pixels are the data for the dataset
                  kMeansPalette = kMeans(imagePixels, m_selectedColors);
                  finalPalette = MedianCutPalette(imagePixels, m_selectedColors);
                  state = Results;
                }
                else {
                  errorMessage= "Image too small: please drag and drop a larger image.";
                  state = Home;
                }
              }
              else {
                errorMessage = "Error reading pixels from provided data stream.";
                state = Home; // fail fallback
              }
            }
            else {
              state = Home; 
            }

            trigger = false;
            loadingProgress = 0.0f;
          }
        }
        break;
      } //case Loading closing
      
      case Results: {

        float availableWidth = ImGui::GetContentRegionAvail().x;
        float spacing = ImGui::GetStyle().ItemSpacing.x;

        float column1W = availableWidth * 0.28f; //40% of space for image
        float column2W = availableWidth * 0.35f; //30% for kMeand palette

        //left side has the image info instead of image
        ImGui::BeginChild("Left Image", ImVec2(column1W, -1.0f), ImGuiChildFlags_Borders);
          
        // show image info and status
        ImGui::Text("Image Processed Successfully!");
        if (!draggedImagePath.empty()) {
          std::string fileName = draggedImagePath.substr(draggedImagePath.find_last_of("/\\") + 1);
          ImGui::Text("File: %s", fileName.c_str());
        }
        ImGui::Separator();
        ImGui::Spacing();


        if (ImageStorage != 0) {
          //create image bounds
          float maxImageWidth = ImGui::GetContentRegionAvail().x;
          float displaySize = (maxImageWidth < 220.0f) ? maxImageWidth : 220.0f;
          ImGui::Image((ImTextureID)(intptr_t)ImageStorage, ImVec2(displaySize, displaySize));
        }
            
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
 
        // color toggle with visible background
        ImGui::Text("Number of Colors:");
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.85f, 0.80f, 0.75f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.90f, 0.85f, 0.80f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.3f, 0.5f, 0.4f, 1.0f));

        if (ImGui::RadioButton("3", m_selectedColors == 3)) {m_selectedColors = 3; }
        ImGui::SameLine();
        if (ImGui::RadioButton("4", m_selectedColors == 4)) {m_selectedColors = 4; }
        ImGui::SameLine();
        if (ImGui::RadioButton("5", m_selectedColors == 5)) {m_selectedColors = 5; }

        ImGui::PopStyleColor(3);
        ImGui::Spacing();
        ImGui::Spacing();
        
        if (ImGui::Button("Reset", ImVec2(ImGui::GetContentRegionAvail().x, 40))) {
          if (ImageStorage != 0) {
            //reset image texture to avoid memory leaks
            glBindTexture(GL_TEXTURE_2D, 0);
            glDeleteTextures(1, &ImageStorage);
            ImageStorage = 0;
          }
          draggedImagePath.clear();
          kMeansPalette.clear();
          finalPalette.clear();
          imagePixels.clear();
          state = Home;
        }
        ImGui::EndChild();

        ImGui::SameLine(0.0f, spacing / 2.0f);

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.306f, 0.380f, 0.357f, 0.4f));
        ImGui::BeginChild("Column1_Seperator", ImVec2(1.0f, 0.0f), ImGuiChildFlags_None, ImGuiWindowFlags_NoInputs);
        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::SameLine(0.0f, spacing / 2.0f);

        //Middle Column: K-Means Palette
        ImGui::SetNextWindowContentSize(ImVec2(450.0f, 0.0f));
        ImGui::BeginChild("Middle Column", ImVec2(column2W, 0), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::Text("K-Means Palette:");
        ImGui::Separator();
        ImGui::Spacing();

        for (size_t i = 0; i < kMeansPalette.size(); ++i) {
          std::string hexStringK = pixelToHex(kMeansPalette[i]);
          std::string rgbStringK = std::to_string(kMeansPalette[i].r) + "," + 
                                    std::to_string(kMeansPalette[i].g) + "," + 
                                    std::to_string(kMeansPalette[i].b);
          ImVec4 color = ImVec4(kMeansPalette[i].r / 255.0f, kMeansPalette[i].g / 255.0f, kMeansPalette[i].b / 255.0f, 1.0f);

          // color swatch
          ImGui::PushStyleColor(ImGuiCol_Button, color);
          ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
          ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
          ImGui::ColorButton(("##ColorK" + std::to_string(i)).c_str(), color, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip, ImVec2(40, 40));
          ImGui::PopStyleColor(3);

          ImGui::SameLine();

          // === HEX LINE ===
          ImGui::BeginGroup();
          ImGui::Text("Hex: %s", hexStringK.c_str());

          // Copy K Means Hex button
          ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.306f, 0.380f, 0.357f, 1.0f));
          ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.48f, 0.45f, 1.0f));
          ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.122f, 0.145f, 0.196f, 1.0f));
          if (ImGui::Button(("Copy Hex##KHex" + std::to_string(i)).c_str(), ImVec2(180, 0))) {
            ImGui::SetClipboardText(hexStringK.c_str());
          }
          ImGui::PopStyleColor(3);
          ImGui::EndGroup();

          ImGui::SameLine();

          // === K Means RGB LINE ===
          ImGui::BeginGroup();
          ImGui::Text("RGB: %s", rgbStringK.c_str());
            
          // Copy RGB button
          ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.306f, 0.380f, 0.357f, 1.0f));
          ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.48f, 0.45f, 1.0f));
          ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.122f, 0.145f, 0.196f, 1.0f));
          if (ImGui::Button(("Copy RGB##KRGB" + std::to_string(i)).c_str(), ImVec2(180, 0))) {
            ImGui::SetClipboardText(rgbStringK.c_str());
          }
          ImGui::PopStyleColor(3);
          ImGui::EndGroup();
            
          ImGui::Spacing();
        }

        // copy all K-buttons footer
        ImGui::Separator();
        ImGui::Text("Copy K-Means Cut:");
        ImGui::SameLine();
        
        // copy all K Means Hex button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.306f, 0.380f, 0.357f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.48f, 0.45f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.122f, 0.145f, 0.196f, 1.0f));
        if (ImGui::Button("Copy All Hex##KAllHex", ImVec2(130, 30))) {
          std::string hexString;
          for (const auto& p : kMeansPalette) {
            hexString += pixelToHex(p);
            hexString += " ";
          }
          if (!hexString.empty()) hexString.pop_back();
          ImGui::SetClipboardText(hexString.c_str());
          paletteOutputText(kMeansPalette);
        }
        ImGui::PopStyleColor(3);
        
        ImGui::SameLine();
          
        // copy all rgb button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.306f, 0.380f, 0.357f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.48f, 0.45f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.122f, 0.145f, 0.196f, 1.0f));
        if (ImGui::Button("Copy All RGB##KAllRGB", ImVec2(130, 30))) {
          std::string rgbString;
          for (const auto& p : kMeansPalette) {
            rgbString += std::to_string(p.r) + "," + std::to_string(p.g) + "," + std::to_string(p.b);
            rgbString += " ";
          }
          if (!rgbString.empty()) rgbString.pop_back();
          ImGui::SetClipboardText(rgbString.c_str());
        }
        ImGui::PopStyleColor(3);

        ImGui::Spacing();
        if (ImGui::Button("Copy K-Means Palette##KMeansPalette", ImVec2(430, 40))) {
          paletteOutputText(kMeansPalette);
        }
        ImGui::EndChild();
        ImGui::SameLine(0.0f, spacing);

        //Right Side: Median Cut Palette
        ImGui::SetNextWindowContentSize(ImVec2(450.0f, 0.0f));
        ImGui::BeginChild("Right Column", ImVec2(column2W, 0), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse); //0.0f causes Median palette to fill remaining space
        ImGui::Text("Median Cut Palette:");
        ImGui::Separator();
        ImGui::Spacing();

        for (size_t i = 0; i < finalPalette.size(); ++i) {
          std::string hexStringM = pixelToHex(finalPalette[i]);
          std::string rgbStringM = std::to_string(finalPalette[i].r) + "," + 
                                    std::to_string(finalPalette[i].g) + "," + 
                                    std::to_string(finalPalette[i].b);
          ImVec4 color = ImVec4(finalPalette[i].r / 255.0f, finalPalette[i].g / 255.0f, finalPalette[i].b / 255.0f, 1.0f);

          // color swatch
          ImGui::PushStyleColor(ImGuiCol_Button, color);
          ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
          ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
          ImGui::ColorButton(("##Color" + std::to_string(i)).c_str(), color, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoTooltip, ImVec2(40, 40));
          ImGui::PopStyleColor(3);
          
          ImGui::SameLine();
            
          // === HEX LINE ===
          ImGui::BeginGroup();
          ImGui::Text("Hex: %s", hexStringM.c_str());
              
          // Copy Median Cut Hex button
          ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.306f, 0.380f, 0.357f, 1.0f));
          ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.48f, 0.45f, 1.0f));
          ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.122f, 0.145f, 0.196f, 1.0f));
          if (ImGui::Button(("Copy Hex##MHex" + std::to_string(i)).c_str(), ImVec2(180, 0))) {
            ImGui::SetClipboardText(hexStringM.c_str());
          }
          ImGui::PopStyleColor(3);
          ImGui::EndGroup();

          ImGui::SameLine();
            
          // === Median Cut RGB LINE ===
          ImGui::BeginGroup();
          ImGui::Text("RGB: %s", rgbStringM.c_str());
            
          // Copy RGB button
          ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.306f, 0.380f, 0.357f, 1.0f));
          ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.48f, 0.45f, 1.0f));
          ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.122f, 0.145f, 0.196f, 1.0f));
          if (ImGui::Button(("Copy RGB##MRGB" + std::to_string(i)).c_str(), ImVec2(180, 0))) {
            ImGui::SetClipboardText(rgbStringM.c_str());
          }
          ImGui::PopStyleColor(3);
          ImGui::EndGroup();
            
          ImGui::Spacing();
        }

        // copy all buttons
        ImGui::Separator();
        ImGui::Text("Copy Medium Cut:");
        ImGui::SameLine();
        
        // copy all Medium Cut Hex button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.306f, 0.380f, 0.357f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.48f, 0.45f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.122f, 0.145f, 0.196f, 1.0f));
        if (ImGui::Button("Copy All Hex##MAllHex", ImVec2(130, 30))) {
          std::string hexString;
          for (const auto& p : finalPalette) {
            hexString += pixelToHex(p);
            hexString += " ";
          }
          if (!hexString.empty()) hexString.pop_back();
          ImGui::SetClipboardText(hexString.c_str());
          paletteOutputText(finalPalette);
        }
        ImGui::PopStyleColor(3);
        
        ImGui::SameLine();
          
        // copy all rgb button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.306f, 0.380f, 0.357f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.48f, 0.45f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.122f, 0.145f, 0.196f, 1.0f));
        if (ImGui::Button("Copy All RGB##MAllRGB", ImVec2(130, 30))) {
          std::string rgbString;
          for (const auto& p : finalPalette) {
            rgbString += std::to_string(p.r) + "," + std::to_string(p.g) + "," + std::to_string(p.b);
            rgbString += " ";
          }
          if (!rgbString.empty()) rgbString.pop_back();
          ImGui::SetClipboardText(rgbString.c_str());
        }
        ImGui::PopStyleColor(3);

        ImGui::Spacing();
        if (ImGui::Button("Copy Median Cut Palette##MedianPalette", ImVec2(430, 40))) {
          paletteOutputText(finalPalette);
        }
        ImGui::EndChild();
        break;
      } //results case closing
    }//switch case closing
        
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
    ImageStorage = 0;
  }
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
} //main function closing

