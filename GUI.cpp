/*---IMGUI---*/
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h> 

/*---C++---*/
#include <Windows.h>
#include <cstdio>
#include <string>
using std::string;

/*---USER---*/
#include "Gui.hpp"
#include "ImFileDialog.hpp"
#include "injector.hpp"
#include "helper.hpp"

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void GUI::entry() {
    /* Initialize glfw */
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return;

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(200, 1, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL) return;
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    /* Initialize file dialog */
    ifd::FileDialog::Instance().CreateTexture = [](uint8_t* data, int w, int h, char fmt) -> void* {
        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, (fmt == 0) ? GL_BGRA : GL_RGBA, GL_UNSIGNED_BYTE, data);
        //glGenerateMipmap(GL_TEXTURE_2D); cause exception
        glBindTexture(GL_TEXTURE_2D, 0);

        return (void*)tex;
    };
    ifd::FileDialog::Instance().DeleteTexture = [](void* tex) {
        GLuint texID = (GLuint)((uintptr_t)tex);
        glDeleteTextures(1, &texID);
    };

    /* Message loop */
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Main Window
        {
            static bool p_bool = true;
            static float window_dpi = ImGui::GetWindowDpiScale();
            static char input_dll_path[FILENAME_MAX];
            static char input_process_name[FILENAME_MAX] = "GTA5.exe";
            static string dialog_dll_path;
            static string dialog_text;

            ImGui::Begin("DLL Injector By 4566z1", &p_bool, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
            if (!p_bool) { ImGui::End(); break; }

            ImGui::SetWindowSize({ 720, 360 });
            ImGui::SetWindowFontScale(window_dpi + 0.3f);

            // DLL Path input
            ImGui::Text("DLL Path");
            ImGui::SameLine();
            ImGui::InputText("##1", input_dll_path, FILENAME_MAX);
            ImGui::SameLine();
            if(ImGui::Button("Browser"))
                ifd::FileDialog::Instance().Open("DLLOpenDialog", "Open a DLL", "DLL file (*.dll){.dll}", false, "C:\\");
            
            // Process id input
            ImGui::Text("Process ");
            ImGui::SameLine();
            ImGui::InputText("##2", input_process_name, FILENAME_MAX);

            // Inject button
            if (ImGui::Button("Inject the dll"))
            {
                if (!strlen(input_dll_path)) dialog_text = "Dll path is empty!";
                else if (!strlen(input_process_name)) dialog_text = "ProcessName is empty";
                else {
                    auto process_id = Helper::GetProcessId(input_process_name);
                    if (process_id.has_value()) {
                        if (process_id.value() == -1) dialog_text = "GetProcessId internal error";
                        else dialog_text = std::move(Injector::injectProcess(process_id.value(), input_dll_path));
                    }
                    else {
                        dialog_text = "Process not found";
                    }
                }
            }
            ImGui::Separator();
            ImGui::TextColored({ 1.0f,0.75f,0.79f,1.0f }, dialog_text.c_str());

            ImGui::End();

            //File dialog
            if (ifd::FileDialog::Instance().IsDone("DLLOpenDialog")) {
                if (ifd::FileDialog::Instance().HasResult()) {
                    const std::vector<std::filesystem::path>& res = ifd::FileDialog::Instance().GetResults();
                    if(res.size()) dialog_dll_path = res[0].u8string();
                    strcpy_s(input_dll_path, dialog_dll_path.c_str());
                }
                ifd::FileDialog::Instance().Close();
            }
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}