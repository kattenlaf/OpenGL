#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

int main(void) {
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    // Set open gl version, 3.3 in this case and set open gl to CORE mode
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1); // Syncs to vsync of monitor

    // glewInit must be called after a valid opengl context exists
    if (glewInit() != GLEW_OK) {
        std::cout << "Error" << std::endl;
    }

    std::cout << glGetString(GL_VERSION) << std::endl;

    {
        float positions[]{
            100.0f, 100.0f, 0.0f, 0.0f, // 0
            200.0f, 100.0f, 1.0f, 0.0f, // 1
            200.0f, 200.0f, 1.0f, 1.0f, // 2
            100.0f, 200.0f, 0.0f, 1.0f  // 3
        };

        // Index buffer so we can render a square
        unsigned int indices[] = {
            0, 1, 2, // First Triangle
            2, 3, 0 // Second Triangle
        };

        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        VertexArray va;
        // Number of vertices multiplied by the number of values stored in each vertex multiplied by the sizeof the type of the value in the vertex
        VertexBuffer vb(positions, 4 * 4 * sizeof(float));
        VertexBufferLayout layout;
        layout.Push<float>(2); // vertex positions
        layout.Push<float>(2); // texture coordinates
        va.AddBuffer(vb, layout);

        IndexBuffer ib(indices, 6);

        // Create projection matrix [orthographic matrix]
        // Read up Model View Project Matrix u_MVP
        // Model Matrix, View Matrix, Projection Matrix
        // Represents the model or actual object being draw, 
        // View Represents how the object is being seen (ie the camera), 
        // Projection defines how the model is actually mapped to the screen
        glm::mat4 proj = glm::ortho(-0.0f, 640.0f, 0.0f, 960.0f, -1.0f, 1.0f);
        // Simulate moving camera to the right, thus moving everything else to the left
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(-100, 0, 0));
        // Model matrix to move the rendered model as we desire
        //glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(200, 200, 0));

        //// Multiplication is in inverse order memory layout of these matrices is in column major order, thus we can view this multiplication as
        //// from right to left, model * view * proj -> thus mvp
        //glm::mat4 mvp = proj * view * model;

        std::string filepath = "res/shaders/Basic.shader";
        Shader shader = Shader(filepath);
        shader.Bind();
        shader.SetUniform4f("u_Color", 0.8f, 0.3f, 0.8f, 1.0f);

        Texture texture("res/textures/dragonball.png");
        texture.Bind();
        shader.SetUniform1i("u_Texture", 0);

        va.Unbind();
        ib.Unbind();
        shader.Unbind();

        Renderer renderer;
        // Setup ImGui binding
        ImGui::CreateContext();
        ImGui_ImplGlfwGL3_Init(window, true);
        ImGui::StyleColorsDark();

        glm::vec3 translation = glm::vec3(200, 200, 0);

        float r = 0.0f;
        float increment = 0.05f;
        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window)) {
            /* Render here */
            renderer.Clear();

            ImGui_ImplGlfwGL3_NewFrame();

            glm::mat4 model = glm::translate(glm::mat4(1.0f), translation);
            glm::mat4 mvp = proj * view * model;

            shader.Bind();
            shader.SetUniform4f("u_Color", r, 0.3f, 0.8f, 1.0f);
            shader.SetUniformMat4f("u_MVP", mvp);
            renderer.Draw(va, ib, shader);

            if (r > 1.0f) {
                increment = -0.01f;
            }
            else if (r < 0.0f) {
                increment = 0.01f;
            }

            r += increment;

            {
                ImGui::SliderFloat3("Translation", &translation.x, 0.0f, 640.0f);
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            }

            ImGui::Render();
            ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();
        }
    }

    // Cleanup
    ImGui_ImplGlfwGL3_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}