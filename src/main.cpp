#include "cpu.hpp"
#include "helpers.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_memory_editor.h"
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>

#include "portable-file-dialogs.h"

#include <cstdio>
#include <cassert>
#include <cstring>
#include <vector>


struct Instruction
{
    FormattedInstruction instruction;
    uint32_t address;
    bool hasBreakpoint;
};
static CPU cpu;
static CPU initialState{};
static std::vector<Instruction> instructionListing;


static void FileOpenButtonPressed()
{
    auto dialog = pfd::open_file("Select RISC-V ELF file");
    std::vector<std::string> selectedFiles = dialog.result();
    if (!selectedFiles.empty()) {
        std::vector<uint8_t> buffer = ReadEntireFile(selectedFiles[0]);
        ParseELFResult result = cpu.InitializeFromELF(buffer.data(), buffer.size());
        if (result != ParseELFResult::Ok) {
            pfd::message("Invalid ELF file", ParseELFResultMessage(result),
                pfd::choice::ok, pfd::icon::error);
        }
    }
    instructionListing.clear();
    instructionListing.reserve(cpu.memory.Size);
    for (uint32_t i = 0; i+4 <= cpu.memory.Size; i += 4) {
        uint32_t word = cpu.memory.Read<uint32_t>(i);
        if (DecodeInstruction(word) != InstructionType::ILLEGAL)
            instructionListing.push_back({FormatInstruction(word), i, false});
    }
    initialState = cpu;
}

static void DebugStartButtonPressed()
{
    while (true) {
        if (!cpu.Step()) break;
        if (instructionListing[cpu.pc/4].hasBreakpoint) break;
    }
}

static void DebugStopButtonPressed()
{
    printf("DebugStopButtonPressed!\n");
}

static void DebugRestartButtonPressed()
{
    cpu = initialState;
}

static void DebugStepOverButtonPressed()
{
    printf("DebugStepOverButtonPressed!\n");
}

static void DebugStepIntoButtonPressed()
{
    memset(cpu.intRegs.didChange, false, cpu.intRegs.Size);
    memset(cpu.fltRegs.didChange, false, cpu.fltRegs.Size);
    memset(cpu.memory.didChange, false, cpu.memory.Size);
    cpu.Step();
}

static void DebugStepOutButtonPressed()
{
    printf("DebugStepOutButtonPressed!\n");
}

static void GLFWErrorCallback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static bool MemoryHighlightFn(const ImU8* data, size_t off)
{
    (void) data;
    return cpu.memory.didChange[off] || ((off & ~0b11) == cpu.pc);
}


typedef void (*ButtonPressCallback)();
struct Button
{
    ButtonPressCallback callbackFunc;
    Texture texture;
};


// #define DO_TESTS
#ifdef DO_TESTS
void DoTests();
int main()
{
    DoTests();
    return 0;
#else
int main()
{
#endif

    glfwSetErrorCallback(GLFWErrorCallback);
    if (!glfwInit())
        return 1;

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "cpu", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Load images
    Button buttons[] = {
        { .callbackFunc = FileOpenButtonPressed,      .texture = LoadTextureFromFile("./images/folder-opened.png")   },
        { .callbackFunc = DebugStartButtonPressed,    .texture = LoadTextureFromFile("./images/debug-start.png")     },
        // { .callbackFunc = DebugStopButtonPressed,     .texture = LoadTextureFromFile("./images/debug-stop.png")      },
        { .callbackFunc = DebugRestartButtonPressed,  .texture = LoadTextureFromFile("./images/debug-restart.png")   },
        // { .callbackFunc = DebugStepOverButtonPressed, .texture = LoadTextureFromFile("./images/debug-step-over.png") },
        { .callbackFunc = DebugStepIntoButtonPressed, .texture = LoadTextureFromFile("./images/debug-step-into.png") },
        // { .callbackFunc = DebugStepOutButtonPressed,  .texture = LoadTextureFromFile("./images/debug-step-out.png")  },
    };
    size_t numButtons = sizeof(buttons) / sizeof(buttons[0]);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    // io.ConfigViewportsNoAutoMerge = true;
    // io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    

    ImU32 highlightColor = IM_COL32(255, 0, 0, 255);
    MemoryEditor memEdit;
    memEdit.ReadOnly = true;
    memEdit.HighlightFn = MemoryHighlightFn;
    memEdit.HighlightColor = highlightColor;
    memEdit.OptShowAscii = false;

    while (!glfwWindowShouldClose(window)) {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            if (ImGui::Begin("DockSpace", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground)) {
                ImGuiID dockspaceID = ImGui::GetID("DockSpace");
                ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
            }
            ImGui::End();
            ImGui::PopStyleVar(3);


            if (ImGui::Begin("Buttons")) {
                for (size_t i = 0; i < numButtons; ++i) {
                    const Button& btn = buttons[i];
                    if (ImGui::ImageButton((ImTextureID)(intptr_t)btn.texture.id, ImVec2(32, 32))) {
                        btn.callbackFunc();
                    }
                    ImGui::SameLine();
                }
            }
            ImGui::End();


            if (ImGui::Begin("Code")) {
                for (auto& [instruction, address, hasBreakpoint] : instructionListing) {
                    bool isCurrentInstruction = address == cpu.pc;
                    char buff[32];
                    snprintf(buff, sizeof(buff), "##%08X:", address);
                    ImGui::Checkbox(buff, &hasBreakpoint);
                    ImGui::SameLine();
                    if (isCurrentInstruction) ImGui::PushStyleColor(ImGuiCol_Text, highlightColor);
                    ImGui::Text("%08X: %s", address, instruction.buffer);
                    if (isCurrentInstruction) ImGui::PopStyleColor();
                }
            }
            ImGui::End();


            if (ImGui::Begin("Registers")) {
                for (uint32_t i = 0; i < cpu.intRegs.Size; ++i) {
                    {
                        uint32_t x = cpu.intRegs.Read(i);
                        bool didChange = cpu.intRegs.didChange[i];
                        if (didChange) ImGui::PushStyleColor(ImGuiCol_Text, highlightColor);
                        int length = snprintf(NULL, 0, "%d", x);
                        ImGui::Text("%*sx%u: %02X %02X %02X %02X  (%d)%*s",
                            i < 10, "", i, (x >> 24) & 0xFF, (x >> 16) & 0xFF, (x >> 8) & 0xFF, (x >> 0) & 0xFF, x, 15-length, "");
                        if (didChange) ImGui::PopStyleColor();
                    }
                    ImGui::SameLine();
                    {
                        float y = cpu.fltRegs.Read(i);
                        uint32_t x = bit_cast<uint32_t>(y);
                        bool didChange = cpu.fltRegs.didChange[i];
                        if (didChange) ImGui::PushStyleColor(ImGuiCol_Text, highlightColor);
                        ImGui::Text("%*sx%u: %02X %02X %02X %02X  (%f)\n",
                            i < 10, "", i, (x >> 24) & 0xFF, (x >> 16) & 0xFF, (x >> 8) & 0xFF, (x >> 0) & 0xFF, y);
                        if (didChange) ImGui::PopStyleColor();
                    }
                }

                ImGui::NewLine();
                ImGui::Text("pc:  %02X %02X %02X %02X  (%d)\n",
                    (cpu.pc >> 24) & 0xFF, (cpu.pc >> 16) & 0xFF, (cpu.pc >> 8) & 0xFF, (cpu.pc >> 0) & 0xFF, cpu.pc);
                uint32_t fcsr = cpu.csr.Read(CSR_fcsr);
                ImGui::Text("       frm  NV DZ OF UF NX");
                ImGui::Text("fcsr:  %d%d%d   %d  %d  %d  %d  %d",
                    (fcsr >> 7) & 1, (fcsr >> 6) & 1, (fcsr >> 5) & 1, (fcsr >> 4) & 1, (fcsr >> 3) & 1, (fcsr >> 2) & 1, (fcsr >> 1) & 1, (fcsr >> 0) & 1);
            }
            ImGui::End();


            memEdit.DrawWindow("Memory", cpu.memory.buffer, cpu.memory.Size);
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
