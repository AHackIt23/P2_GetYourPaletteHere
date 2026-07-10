int main(int argc, char** argc) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) return -1;

SDL* window = SLD_CreateWindow("Get Your Palette Here!", SDL_WINDOWPOS_CENTERE, 
                                SDL_WINDOWPOS_CENTERED< 1200, 780, SDL_WINDOW_RESIZEABLE);

IMGUI_CHECKVERSION();
ImGui::CreateContext();
ImGui_ImplSDL2_InitForSDLRender(window, renderer);
