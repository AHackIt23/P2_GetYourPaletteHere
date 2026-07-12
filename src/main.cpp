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
#include <sdl_opengl.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <GL/gl.h>

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
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
  checkGLError("loadPixels - glTexImage2D");
  checkGLError("loadPixels - glBindTexture");
  glBindTexture(GL_TEXTURE_2D, 0);
  
  stbi_image_free(data);
  
  std::cout << "Texture created with ID: " << resultsImageTexture << std::endl;
  return pixels;
}

//k means algorithm
std::vector<Pixel> kMean(const std::vector<Pixel>& inputPixels) {
  //to do: implement algorithm
  std::vector<Pixel> mockPixels = {
      {255, 0, 0},
      {0, 255, 0},
      {0, 0, 255}
  }; // mock vector totest app play
  return mockPixels;
}

//median mean algorithm
std::vector<Pixel> MedianMean(const std::vector<Pixel>& kPixels) {
  //to do: implement algorithm
  return kPixels;
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
    std::cout << "Palette saved to: " << output << '\n';
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

  SDL_Window* window = SDL_CreateWindow("Get Your Palette Here!", SDL_WINDOWPOS_CENTERED, 
                                  SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_RESIZABLE);
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
  setAestheticStyle();//we'll be doing a muted green aesthetic style<3
  
  ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer2_Init(renderer);
  
  ProgramState state = Home;
  
  std::string draggedImagePath = "";
  std::vector<Pixel> finalPalette; //median cut palette
  GLuint ImageStorage = 0; //stores image as texture for results page
  int m_selectedColors = 5; // Default to 5 colors
  int m_selectedAlgorithm = 0; // 0 = Median Cut, 1 = K-Means
  
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
          std::cout << "File dropped: " << draggedImagePath << std::endl;
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
    float windowWidth = ImGui::GetWindowWidth();
    float windowHeight = ImGui::GetWindowHeight();
    
    //center
    float centerX = windowWidth / 2;

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
    float dragTextWidth = ImGui::CalcTextSize(dragText).x;
    ImGui::SetCursorPos(ImVec2(rectX + (rectWidth - dragTextWidth) / 2, rectY + (rectHeight - 20) / 2
    ));
    ImGui::TextColored(ImVec4(0.5f, 0.45f, 0.4f, 1.0f), "%s", dragText);
    
    // File loaded indicator
    if (!draggedImagePath.empty()) {
        std::string fileName = draggedImagePath.substr(draggedImagePath.find_last_of("/\\") + 1);
        std::string loadedText = "✓ File loaded: " + fileName;
        float loadedTextWidth = ImGui::CalcTextSize(loadedText.c_str()).x;
        ImGui::SetCursorPos(ImVec2(rectX + (rectWidth - loadedTextWidth) / 2, rectY + rectHeight + 10
        ));
        ImGui::TextColored(ImVec4(0.3f, 0.6f, 0.3f, 1.0f), "%s", loadedText.c_str());
    }
    
    // move cursor out rectangle
    ImGui::SetCursorPosY(rectY + rectHeight + 50);

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
    
    // ALGORITHM TOGGLE!!!!!!! (Median Cut vs K-Means)
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
    std::string algoText = "Algorithm:";
    float algoTextWidth = ImGui::CalcTextSize(algoText.c_str()).x;
    float algoRadioWidth = 200; // Approximate width of 2 radio buttons
    float algoTotalWidth = algoTextWidth + 20 + algoRadioWidth;
    ImGui::SetCursorPosX(centerX - algoTotalWidth / 2);
    ImGui::Text("%s", algoText.c_str());
    ImGui::SameLine();
    
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.85f, 0.80f, 0.75f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.90f, 0.85f, 0.80f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.3f, 0.5f, 0.4f, 1.0f));
    
    if (ImGui::RadioButton("Median Cut", m_selectedAlgorithm == 0)) m_selectedAlgorithm = 0;
    ImGui::SameLine();
    if (ImGui::RadioButton("K-Means", m_selectedAlgorithm == 1)) m_selectedAlgorithm = 1;
    
    ImGui::PopStyleColor(3);
    
    // extract palette
    ImGui::SetCursorPos(ImVec2(
        centerX - 120,
        ImGui::GetCursorPosY() + 30
    ));
    
    if (draggedImagePath.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
        ImGui::Button("Extract Palette", ImVec2(240, 52));
        ImGui::PopStyleColor();
    } else {
        if (ImGui::Button("Extract Palette", ImVec2(240, 52))) {
            state = Loading;
        }
    }
    
    ImGui::Dummy(ImVec2(0, 0));
    break;
} //case Home closing
  
      case Loading: {
        //SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        //ImGui_ImplSDLRenderer2_DrawData(ImGui::GetDrawData());
      
        //SDL_RenderPresent(renderer);
        if (!draggedImagePath.empty()) {
          //load image and get list of [R,G,B} values
          
          
          
          std::cout << "Loading pixels from: " << draggedImagePath << std::endl;
          std::vector<Pixel> imagePixels = loadPixels(draggedImagePath, ImageStorage);
          std::cout << "Loaded " << imagePixels.size() << " pixels" << std::endl;
          if (!imagePixels.empty()) {

            //std::vector<Pixel> kMeansPixels = k means function call returning palette(dataset)
            //finalPalette = median mean function call returning 3 color palette(kMeansPixels)

          // using the median cut algorithm
          // using the selected algorithm
          if (m_selectedAlgorithm == 0) {
              // Median Cut
              finalPalette = MedianCutPalette(imagePixels, m_selectedColors);
          } else {
              // K-Means (placeholder - Amkia will replace this)
              finalPalette = kMean(imagePixels);
              // Resize to match selected color count
              if ((int)finalPalette.size() > m_selectedColors) {
                  finalPalette.resize(m_selectedColors);
              }
          }

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
    //left side has the image info instead of image
    ImGui::BeginChild("Left Image", ImVec2(ImGui::GetWindowWidth() * 0.5f, 0), true);
    
    // show image info and status
    ImGui::Text("Image Processed Successfully!");
    if (!draggedImagePath.empty()) {
        std::string fileName = draggedImagePath.substr(draggedImagePath.find_last_of("/\\") + 1);
        ImGui::Text("File: %s", fileName.c_str());
    }
    ImGui::Text("Colors Extracted: %d", (int)finalPalette.size());
    const char* algoNames[] = {"Median Cut", "K-Means"};
    ImGui::Text("Algorithm: %s", algoNames[m_selectedAlgorithm]);
    
    // color toggle with visible background
    ImGui::Text("Number of Colors:");
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.85f, 0.80f, 0.75f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.90f, 0.85f, 0.80f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.3f, 0.5f, 0.4f, 1.0f));

    if (ImGui::RadioButton("3", m_selectedColors == 3)) {
        m_selectedColors = 3;
        if (!draggedImagePath.empty()) {
            std::vector<Pixel> imagePixels = loadPixels(draggedImagePath, ImageStorage);
            if (!imagePixels.empty()) {
                if (m_selectedAlgorithm == 0) {
                    finalPalette = MedianCutPalette(imagePixels, m_selectedColors);
                } else {
                    finalPalette = kMean(imagePixels);
                    if ((int)finalPalette.size() > m_selectedColors) {
                        finalPalette.resize(m_selectedColors);
                    }
                }
            }
        }
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("4", m_selectedColors == 4)) {
        m_selectedColors = 4;
        if (!draggedImagePath.empty()) {
            std::vector<Pixel> imagePixels = loadPixels(draggedImagePath, ImageStorage);
            if (!imagePixels.empty()) {
                if (m_selectedAlgorithm == 0) {
                    finalPalette = MedianCutPalette(imagePixels, m_selectedColors);
                } else {
                    finalPalette = kMean(imagePixels);
                    if ((int)finalPalette.size() > m_selectedColors) {
                        finalPalette.resize(m_selectedColors);
                    }
                }
            }
        }
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("5", m_selectedColors == 5)) {
        m_selectedColors = 5;
        if (!draggedImagePath.empty()) {
            std::vector<Pixel> imagePixels = loadPixels(draggedImagePath, ImageStorage);
            if (!imagePixels.empty()) {
                if (m_selectedAlgorithm == 0) {
                    finalPalette = MedianCutPalette(imagePixels, m_selectedColors);
                } else {
                    finalPalette = kMean(imagePixels);
                    if ((int)finalPalette.size() > m_selectedColors) {
                        finalPalette.resize(m_selectedColors);
                    }
                }
            }
        }
    }

    ImGui::PopStyleColor(3);
    
    if (ImGui::Button("Reset")) {
        draggedImagePath.clear();
        finalPalette.clear();
        state = Home;
        ImageStorage = 0;
    }
    ImGui::EndChild();

    ImGui::SameLine();

    //right side will have the palette
    ImGui::BeginChild("Right Column", ImVec2(0, 0), true);
    ImGui::Text("Color Palette:");
    ImGui::Text("%d colors extracted", (int)finalPalette.size());

    for (size_t i = 0; i < finalPalette.size(); ++i) {
        std::string hexString = pixelToHex(finalPalette[i]);
        std::string rgbString = std::to_string(finalPalette[i].r) + "," + 
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
        ImGui::Text("Hex: %s", hexString.c_str());
        ImGui::SameLine();
        
        // Copy Hex button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.306f, 0.380f, 0.357f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.48f, 0.45f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.122f, 0.145f, 0.196f, 1.0f));
        if (ImGui::Button(("Copy Hex##" + std::to_string(i)).c_str())) {
            ImGui::SetClipboardText(hexString.c_str());
        }
        ImGui::PopStyleColor(3);
        
        // === RGB LINE ===
        ImGui::Text("RGB: %s", rgbString.c_str());
        ImGui::SameLine();
        
        // Copy RGB button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.306f, 0.380f, 0.357f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.48f, 0.45f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.122f, 0.145f, 0.196f, 1.0f));
        if (ImGui::Button(("Copy RGB##" + std::to_string(i)).c_str())) {
            ImGui::SetClipboardText(rgbString.c_str());
        }
        ImGui::PopStyleColor(3);
        
        ImGui::Spacing();
    }

    // copy all buttons
    ImGui::Text("Copy All:");
    ImGui::SameLine();
    
    // copy all hex button
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.306f, 0.380f, 0.357f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.48f, 0.45f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.122f, 0.145f, 0.196f, 1.0f));
    if (ImGui::Button("Copy All Hex", ImVec2(100, 30))) {
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
    if (ImGui::Button("Copy All RGB", ImVec2(100, 30))) {
        std::string rgbString;
        for (const auto& p : finalPalette) {
            rgbString += std::to_string(p.r) + "," + std::to_string(p.g) + "," + std::to_string(p.b);
            rgbString += " ";
        }
        if (!rgbString.empty()) rgbString.pop_back();
        ImGui::SetClipboardText(rgbString.c_str());
    }
    ImGui::PopStyleColor(3);

    ImGui::EndChild();
    break;
}
    } //switch closing
    
    ImGui::End(); //End home window
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
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
