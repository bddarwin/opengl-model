#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnOpengl/camera.h> // Camera class
#include <shader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>      // Image loading Utility functions


using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Benjamin Darwin - Milestone Three"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbos[2];     // Handles for the vertex buffer objects
        GLuint nIndices;    // Number of indices of the mesh
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Triangle mesh data
    GLMesh gMesh, gMesh2, gMesh3, gMesh4, gMesh5;
    // Texture id
    GLuint gTextureId, gTextureId2, gTextureId3, gTextureId4, gTextureId5;
    // Shader program
    GLuint gProgramId;

    // camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 3.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;
    bool persp = true;

    // timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreateMesh(GLMesh& mesh, GLMesh& mesh2, GLMesh& mesh3);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
layout(location = 1) in vec4 color;  // Color data from Vertex Attrib Pointer 1
layout(location = 2) in vec2 textureCoordinate;

out vec4 vertexColor; // variable to transfer color data to the fragment shader
out vec2 vertexTextureCoordinate;
//Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
    vertexColor = color; // references incoming color data
    vertexTextureCoordinate = textureCoordinate;
}
);


/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,
    in vec4 vertexColor; // Variable to hold incoming color data from vertex shader
in vec2 vertexTextureCoordinate;
out vec4 fragmentColor;
uniform sampler2D uTexture;
void main()
{
    //fragmentColor = vec4(vertexColor);
    fragmentColor = texture(uTexture, vertexTextureCoordinate); // Sends texture to the GPU for rendering
}
);

void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}

int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the mesh
    UCreateMesh(gMesh, gMesh2, gMesh3); // Calls the function to create the Vertex Buffer Object

    // Create the shader program
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;


    // Load texture (relative to project's directory)
    const char* texFilename = "../bricks.jfif";
    if (!UCreateTexture(texFilename, gTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    // Load texture (relative to project's directory)
    const char* texFilename2 = "../grass.jpg";
    if (!UCreateTexture(texFilename2, gTextureId2))
    {
        cout << "Failed to load texture " << texFilename2 << endl;
        return EXIT_FAILURE;
    }
    // Load texture (relative to project's directory)
    const char* texFilename3 = "../concrete.jpg";
    if (!UCreateTexture(texFilename3, gTextureId3))
    {
        cout << "Failed to load texture " << texFilename3 << endl;
        return EXIT_FAILURE;
    }

    // Tell OpenGL for each sampler which texture unit it belongs to (only has to be done once).
    glUseProgram(gProgramId);
    // We set the texture as texture unit 0.
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);
    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(gMesh);

    // Release texture
    UDestroyTexture(gTextureId);

    // Release shader program
    UDestroyShaderProgram(gProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        if (persp == true) {
            glm::mat4 projection = glm::perspective(45.0f, (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
            persp = false;
        }
        else {
            glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
            persp = true;
        }
    }
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}

// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


// Functioned called to render a frame
void URender()
{
    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. Scales the object by 2
    glm::mat4 scale = glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));
    // 2. Rotates shape by 15 degrees in the x+y axis
    glm::mat4 rotation = glm::rotate(15.0f, glm::vec3(0.0, 1.0f, 0.0f));
    // 3. Place object at the origin
    glm::mat4 translation = glm::translate(glm::vec3(0.0f, 0.0f, -14.0f));
    // Model matrix: transformations are applied right-to-left order
    glm::mat4 model = translation * rotation * scale;

    // Transforms the camera: move the camera back (z axis)
    //glm::mat4 view = glm::translate(glm::vec3(0.0f, 0.0f, -5.0f));

    // camera/view transformation
    glm::mat4 view = gCamera.GetViewMatrix();

    // Creates a perspective projection
    //glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

    // Creates a orthographic projection
    glm::mat4 projection = glm::perspective(45.0f, (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

    // Set the shader to be used
    glUseProgram(gProgramId);

    Shader lightingShader("5.1.light_casters.vs", "5.1.light_casters.fs");
    // shader configuration
    // --------------------
    lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);

    // be sure to activate shader when setting uniforms/drawing objects
    lightingShader.use();
    lightingShader.setVec3("light.direction", -0.2f, -1.0f, -0.3f);
    lightingShader.setVec3("viewPos", gCamera.Position);

    // light properties
    lightingShader.setVec3("light.ambient", 1.0f, 1.0f, 1.2f);
    lightingShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
    lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

    // material properties
    lightingShader.setFloat("material.shininess", 32.0f);

    // view/projection transformations
    //glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
    //glm::mat4 view = gCamera.GetViewMatrix();
    lightingShader.setMat4("projection", projection);
    lightingShader.setMat4("view", view);

    // world transformation
    //glm::mat4 model = glm::mat4(1.0f);
    lightingShader.setMat4("model", model);

    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");

    //glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    //glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    //glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, gMesh.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle

    glBindVertexArray(gMesh2.vao);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId2);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, gMesh2.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle

    glBindVertexArray(gMesh3.vao);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId3);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, gMesh3.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle


    // Deactivate the Vertex Array Object
    glBindVertexArray(0);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}


// Implements the UCreateMesh function
void UCreateMesh(GLMesh& mesh, GLMesh& mesh2, GLMesh& mesh3)
{
    // Position and Color data
    GLfloat verts[] = {
        // Vertex Positions    // Colors (r,g,b,a) //Texture Coordinates

        //Building shapes in a 5x6 grid, starting at -1.0 and going to 1.0, with an offset of 0.25 for each gridline.
        //Left side of base 
        -0.5f, -0.5f, -0.5f, 0.66f, 0.66f, 0.66f, 1.0f, -5.0f, -5.0f, //Back left bottom - Point 0
        -0.5f, -0.5f,  0.5f, 0.66f, 0.66f, 0.66f, 1.0f, -5.0f, -5.0f,//Front left bottom. - Point 1
        -0.5f,  0.5f, -0.5f, 0.66f, 0.66f, 0.66f, 1.0f, -5.0f, 5.0f,//Back left top - Point 2 
        -0.5f,  0.5f,  0.5f, 0.66f, 0.66f, 0.66f, 1.0f, -5.0f, 5.0f,//Front left top - Point 3

        //Right side of base.
        0.5f, -0.5f, -0.5f, 0.66f, 0.66f, 0.66f, 1.0f, 5.0f, -5.0f,//Back right bottom - Point 4
        0.5f, -0.5f,  0.5f, 0.66f, 0.66f, 0.66f, 1.0f, 5.0f, -5.0f,//Front right bottom - Point 5
        0.5f,  0.5f, -0.5f, 0.66f, 0.66f, 0.66f, 1.0f, 5.0f, 5.0f,//Back right top - Point 6
        0.5f,  0.5f,  0.5f, 0.66f, 0.66f, 0.66f, 1.0f, 5.0f, 5.0f,//Front right top - Point 7

        //Top left Addon.
        //Uses points 2, 3 for Bottom left back and Bottom left front
        -0.5f, 0.7f, -0.5f, 0.66f, 0.66f, 0.66f, 1.0f, -5.0f, 7.0f,//Back left top - Point 8
        -0.5f, 0.7f, 0.5f, 0.66f, 0.66f, 0.66f, 1.0f, -5.0f, 7.0f,//Front left top - Point 9.
        -0.3f, 0.7f, -0.5f, 0.66f, 0.66f, 0.66f, 1.0f, -3.0f, 7.0f,//Back right top - Point 10.
        -0.3f, 0.7f, 0.5f, 0.66f, 0.66f, 0.66f, 1.0f, -3.0f, 7.0f,//Front right top - Point 11.
        -0.3f, 0.5f, -0.5f, 0.66f, 0.66f, 0.66f, 1.0f, -3.0f, 5.0f,//Back right bottom - Point 12.
        -0.3f, 0.5f, 0.5f, 0.66f, 0.66f, 0.66f, 1.0f, -3.0f, 5.0f,//Front right bottom - Point 13.

        //Top right addon.
        //Uses points 6, 7, for bottom back right and bottom front right
        0.5f, 0.7f, -0.5f, 0.66f, 0.66f, 0.66f, 1.0f, 5.0f, 7.0f,//Back right top - Point 14
        0.5f, 0.7f, 0.5f, 0.66f, 0.66f, 0.66f, 1.0f, 5.0f, 7.0f,//Front right top - Point 15
        0.3f, 0.7f, -0.5f, 0.66f, 0.66f, 0.66f, 1.0f, 3.0, 7.0f,//Back left top - Point 16
        0.3f, 0.7f, 0.5f, 0.66f, 0.66f, 0.66f, 1.0f, 3.0f, 7.0f,//Front left top - Point 17
        0.3f, 0.5f, -0.5f, 0.66f, 0.66f, 0.66f, 1.0f, 3.0f, 5.0f,//Back left bottom - Point 18.
        0.3f, 0.5f, 0.5f, 0.66f, 0.66f, 0.66f, 1.0f, 3.0f, 5.0f,//Front left bottom - Point 19.

        //Top middle addon.
        //This shape has no points shared with the other, so lets make the left and the right side separate. 
        0.1f, 0.7f, -0.5f, 0.66f, 0.66f, 0.66f, 1.0f, 1.0f, 7.0f,//Back right top - Point 20
        0.1f, 0.7f, 0.5f, 0.66f, 0.66f, 0.66f, 1.0f, 1.0f, 7.0f,//Front right top - Point 21
        0.1f, 0.5f, -0.5f, 0.66f, 0.66f, 0.66f, 1.0f, 1.0f, 5.0f,//Back right bottom - Point 22
        0.1f, 0.5f, 0.5f, 0.66f, 0.66f, 0.66f, 1.0f, 1.0f, 5.0f,//Front right bottom - Point 23
        -0.1f, 0.7f, -0.5f, 0.66f, 0.66f, 0.66f, 1.0f, -1.0f, 7.0f,//Back left top - Point 24
       -0.1f, 0.7f, 0.5f, 0.66f, 0.66f, 0.66f, 1.0f, -1.0f, 7.0f,//Front left top - Point 25
       -0.1f, 0.5f, -0.5f, 0.66f, 0.66f, 0.66f, 1.0f, -1.0f, 5.0f, //Back left bottom - Point 26
       -0.1f, 0.5f, 0.5f, 0.66f, 0.66f, 0.66f, 1.0f, -1.0f, 5.0f,//Front left bottom - Point 27


       //Plane on the ground.
       -5.0f, -0.5f, -5.0f, 0.5f, 0.75f, 0.3f, 1.0f, 0.0f, 0.0f,//Back left - Point 28
       -5.0f, -0.5f, 5.0f, 0.5f, 0.75f, 0.3f, 1.0f, 5.0f, 0.0f,//Front left - Point 29
       5.0f, -0.5f, -5.0f, 0.5f, 0.75f, 0.3f, 1.0f, 0.0f, 5.0f,//Back right - Point 30
       5.0f, -0.5f, 5.0f, 0.5f, 0.75f, 0.3f, 1.0f, 5.0f, 5.0f,//Front right - Point 31

        //Left side of base 
        -0.5f, -0.5f, -0.5f, 0.66f, 0.66f, 0.66f, 1.0f, -5.0f, -5.0f, //Back left bottom - Point 32
        -0.5f, -0.5f,  0.5f, 0.66f, 0.66f, 0.66f, 1.0f, 5.0f, -5.0f,//Front left bottom. - Point 33
        -0.5f,  0.7f, -0.5f, 0.66f, 0.66f, 0.66f, 1.0f, -5.0f, 7.0f,//Back left top - Point 34 
        -0.5f,  0.7f,  0.5f, 0.66f, 0.66f, 0.66f, 1.0f, 5.0f, 7.0f,//Front left top - Point 35

         //Right side of base.
        0.5f, -0.5f, -0.5f, 0.66f, 0.66f, 0.66f, 1.0f, -5.0f, -5.0f,//Back right bottom - Point 36
        0.5f, -0.5f,  0.5f, 0.66f, 0.66f, 0.66f, 1.0f, 5.0f, -5.0f,//Front right bottom - Point 37
        0.5f,  0.7f, -0.5f, 0.66f, 0.66f, 0.66f, 1.0f, -5.0f, 7.0f,//Back right top - Point 38
        0.5f,  0.7f,  0.5f, 0.66f, 0.66f, 0.66f, 1.0f, 5.0f, 7.0f,//Front right top - Point 39

        //Next building - Left Wall
        -2.5f, -0.5f, -0.5f, 0.66f, 0.66f, 0.66f, 0.0f, 0.0f, 0.0f, //Back left bottom- Point 40
        -3.5f, -0.5f, -0.5f, 0.66f, 0.66f, 0.66f, 5.0f, 0.0f, 5.0f, //Front left bottom- Point 41
        -3.5f, -0.1f, -0.5f, 0.66f, 0.66f, 0.66f, 0.0f, 5.0f, 0.0f, //Front left top - Point 42
        -2.5f, -0.1f, -0.5f, 0.66f, 0.66f, 0.66f, 5.0f, 5.0f, 5.0f, //Back left top - Point 43

        //Next Building - Front Wall
        -3.5f, -0.5f, -0.5f, 0.66f, 0.66f, 0.66f, 5.0f, 0.0f, 0.0f, //Front left bottom- Point 44
        -3.5f, -0.1f, -0.5f, 0.66f, 0.66f, 0.66f, 0.0f, 5.0f, 0.0f, //Front left top - Point 45
        -3.5f, -0.5f, 1.0f, 0.66f, 0.66f, 0.66f, 1.0f, 0.0f, 5.0f, //Front right bottom - Point 46
        -3.5f, -0.1f, 1.0f, 0.66f, 0.66f, 0.66f, 1.0f, 5.0f, 5.0f, //Front right top - Point 47. 

        //Next Building - Right Wall
        -2.5f, -0.5f,  1.0f, 0.66f, 0.66f, 0.66f, 0.0f, 0.0f, 0.0f, //Back left bottom- Point 48
        -3.5f, -0.5f, 1.0f, 0.66f, 0.66f, 0.66f, 5.0f, 0.0f, 5.0f, //Front left bottom- Point 49
        -3.5f, -0.1f, 1.0f, 0.66f, 0.66f, 0.66f, 0.0f, 5.0f, 0.0f, //Front left top - Point 50
        -2.5f, -0.1f, 1.0f, 0.66f, 0.66f, 0.66f, 5.0f, 5.0f, 5.0f, //Back left top - Point 51

        //Next Building - Back Wall
        -2.5f, -0.5f, -0.5f, 0.66f, 0.66f, 0.66f, 5.0f, 0.0f, 0.0f, //Front left bottom- Point 52
        -2.5f, -0.1f, -0.5f, 0.66f, 0.66f, 0.66f, 0.0f, 5.0f, 0.0f, //Front left top - Point 53
        -2.5f, -0.5f, 1.0f, 0.66f, 0.66f, 0.66f, 1.0f, 0.0f, 5.0f, //Front right bottom - Point 54
        -2.5f, -0.1f, 1.0f, 0.66f, 0.66f, 0.66f, 1.0f, 5.0f, 5.0f, //Front right top - Point 55

        -3.5f, -0.1f, -0.5f, 0.66f, 0.66f, 0.66f, 0.0f, 0.0f, 0.0f, //Front left top - Point 56
        -2.5f, -0.1f, -0.5f, 0.66f, 0.66f, 0.66f, 5.0f, 0.0f, 0.0f, //Back left top - Point 57
        -3.0f, -0.1f, -0.5f, 0.66f, 0.66f, 0.66f, 5.0f, 0.0f, 0.0f, //Center bottom - Point 58
        -3.0f, 0.4f, -0.5f, 0.66f, 0.66f, 0.66f, 5.0f, 0.0f, 0.0f, //Center bottom - Point 59
        -3.3f, 0.3f, -0.5f, 0.66f, 0.66f, 0.66f, 5.0f, 0.0f, 0.0f, //Center bottom - Point 60
        -3.2f, 0.36f, -0.5f, 0.66f, 0.66f, 0.66f, 5.0f, 0.0f, 0.0f, //Center bottom - Point 61
        -3.4f, 0.2f, -0.5f, 0.66f, 0.66f, 0.66f, 5.0f, 0.0f, 0.0f, //Center bottom - Point 62
        -2.7f, 0.3f, -0.5f, 0.66f, 0.66f, 0.66f, 5.0f, 0.0f, 0.0f, //Center bottom - Point 63
        -2.8f, 0.36f, -0.5f, 0.66f, 0.66f, 0.66f, 5.0f, 0.0f, 0.0f, //Center bottom - Point 64
        -2.6f, 0.2f, -0.5f, 0.66f, 0.66f, 0.66f, 5.0f, 0.0f, 0.0f, //Center bottom - Point 65
        -2.55f, 0.1f, -0.5f, 0.66f, 0.66f, 0.66f, 5.0f, 0.0f, 0.0f, //Center bottom - Point 66
        -3.45f, 0.1f, -0.5f, 0.66f, 0.66f, 0.66f, 5.0f, 0.0f, 0.0f, //Center bottom - Point 67

        -3.5f, -0.1f, 1.0f, 0.66f, 0.66f, 0.66f, 0.0f, 0.0f, 0.0f, //Front left top - Point 68
        -2.5f, -0.1f, 1.0f, 0.66f, 0.66f, 0.66f, 5.0f, 0.0f, 0.0f, //Back left top - Point 69
        -3.0f, -0.1f, 1.0f, 0.66f, 0.66f, 0.66f, 5.0f, 0.0f, 0.0f, //Center bottom - Point 70
        -3.0f, 0.4f, 1.0f, 0.66f, 0.66f, 0.66f, 5.0f, 0.0f, 0.0f, //Center bottom - Point 71
        -3.3f, 0.3f, 1.0f, 0.66f, 0.66f, 0.66f, 5.0f, 0.0f, 0.0f, //Center bottom - Point 72
        -3.2f, 0.36f, 1.0f, 0.66f, 0.66f, 0.66f, 5.0f, 0.0f, 0.0f, //Center bottom - Point 73
        -3.4f, 0.2f, 1.0f, 0.66f, 0.66f, 0.66f, 5.0f, 0.0f, 0.0f, //Center bottom - Point 74
        -2.7f, 0.3f, 1.0f, 0.66f, 0.66f, 0.66f, 5.0f, 0.0f, 0.0f, //Center bottom - Point 75
        -2.8f, 0.36f, 1.0f, 0.66f, 0.66f, 0.66f, 5.0f, 0.0f, 0.0f, //Center bottom - Point 76
        -2.6f, 0.2f, 1.0f, 0.66f, 0.66f, 0.66f, 5.0f, 0.0f, 0.0f, //Center bottom - Point 77
        -2.55f, 0.1f, 1.0f, 0.66f, 0.66f, 0.66f, 5.0f, 0.0f, 0.0f, //Center bottom - Point 78
        -3.45f, 0.1f, 1.0f, 0.66f, 0.66f, 0.66f, 5.0f, 0.0f, 0.0f, //Center bottom - Point 79
    };

    // Index data to share position data
    GLushort indices[] = {

        //Basic Building Shape
        //Left side
        32, 33, 35,
        32, 34, 35,
        //Right side
       36, 37, 39,
       36, 38, 39,
       //Bottom
       0, 1, 5,
       0, 4, 5,
       //Front
       1, 3, 5,
       3, 5, 7,
       //Back
       0, 2, 4,
       2, 4, 6,
       //Top
       2, 3, 7,
       2, 7, 6,

       //Top left addon
       //Left side
      // 2, 3, 8,
      // 3, 8, 9,
       //Front
       3, 9, 13,
       9, 11, 13,
       //Top
       8, 9, 10,
       9, 10, 11,
       //Back
       2, 8, 10,
       2, 10, 12,
       //Right side
       10, 11, 12,
       11, 12, 13,
       //Bottom is covered by the top of the bigger shape already. 

       //Top right addon.
       //Left side
       16, 17, 18,
       17, 18, 19,
       //Front
       15, 17, 19,
       15, 19, 7,
       //Top
       14, 15, 16,
       15, 16, 17,
       //Back
       14, 16, 6,
       16, 18, 6,
       //Right side
      // 14, 15, 6,
       //15, 7, 6,
       //Bottom is covered by the top of the bigger shape already.

       //Top center addon.
       //Bottom still covered, even though this doesn't share any points with the other objects. 
       //Left side 
       24, 25, 26,
       25, 26, 27,
       //Right side
       20, 21, 22,
       21, 22, 23,
       //Front
       21, 23, 25,
       23, 25, 27,
       //Back
       20, 22, 24,
       22, 24, 26,
       //Top
       20, 21, 25,
       20, 24, 25,
    };

    GLushort indices2[] = {
        //Plane
        28, 29, 30,
        29, 30, 31,
    };

    GLushort indices3[] = {
        42, 41, 40,
        40, 42, 43,
        44, 45, 46,
        45, 46, 47,
        48, 49, 50,
        48, 50, 51,
        52, 53, 54,
        53, 54, 55,
        58, 56, 67,
        58, 67, 62,
        58, 62, 60,
        58, 60, 61,
        58, 61, 59,
        58, 59, 64,
        58, 64, 63,
        58, 63, 65,
        58, 65, 66,
        58, 66, 57,

        70, 68, 79,
        70, 79, 74,
        70, 74, 72,
        70, 72, 73,
        70, 73, 71,
        70, 71, 76,
        70, 76, 75,
        70, 75, 77,
        70, 77, 78,
        70, 78, 69,

        //Then the roof
        56, 68, 67,
        68, 67, 74,
        67, 74, 62,
        67, 74, 79,
        62, 74, 60,
        60, 74, 72,
        60, 61, 73,
        72, 73, 60,
        61, 59, 73,
        73, 71, 59,
        59, 64, 76,
        59, 71, 76,
        64, 76, 75,
        63, 75, 64,
        63, 75, 77,
        63, 65, 77,
        65, 77, 78,
        65, 78, 66,
        66, 78, 57,
        57, 78, 69
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;
    const GLuint floatsPerUV = 2;

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor + floatsPerUV);

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerColor)));
    glEnableVertexAttribArray(2);

    glGenVertexArrays(1, &mesh2.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh2.vao);

    //Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh2.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh2.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh2.nIndices = sizeof(indices2) / sizeof(indices2[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh2.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices2), indices2, GL_STATIC_DRAW);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerColor)));
    glEnableVertexAttribArray(2);


    glGenVertexArrays(1, &mesh3.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh3.vao);

    //Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh3.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh3.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh3.nIndices = sizeof(indices3) / sizeof(indices3[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh3.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices3), indices3, GL_STATIC_DRAW);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerColor)));
    glEnableVertexAttribArray(2);
}


void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(2, mesh.vbos);
}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}

bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}


void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}