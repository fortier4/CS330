//Connor Fortier
//Week Seven Project One - 3D Scene
//CS 330
//12/12/2021

//The purpose of this program is to output a 3D scene to the user which includes
//mulitple complex 3D objects with various shapes and textures, complex lighting,
//proper organization in space, camera controls via user input, and also input
//control to switch between orthographic projection and perspective projection.
//The image used is my own image, thus being free of any copyrights.
//Furthermore, the github tutorials were referenced for the coding of
//this program and credit goes to the author of said tutorial for any information
//found in this program that was referenced from the mentioned tutorial,
//likewise, I reused code from my module six milestone submission and give
//credit as due for reusing said work as per instruction requirements.
//Lastly, I avoided over complex shapes with many cylinders and spheres in order
//to keep the count of triangles below 1000 as requested per project requirements.

//FOUR PRIMITIVE SHAPES:
//pyramid: car key
//cube: zippo lighter and calculator
//plane: desk and ruler
//cylinder: zippo lighter hinge

//include statements for program
#include <iostream>                             //input and output stream
#include <cstdlib>                              //standard library inclusions
#include <GL/glew.h>                            //include glew library
#include <GLFW/glfw3.h>                         //include glfw library
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>                          //load image

//math includes
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnOpengl/camera.h>                 //include camera class

using namespace std;                            //using standard naming conventions

//shader 
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

namespace {
    //window title
    const char* const WINDOW_TITLE = "Connor Fortier - Project One";

    //declare and initialize variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    //store gl data for mesh
    struct GLMesh {
        GLuint vao;             //vertex array object
        GLuint vbo;             //vertex buffer object
        GLuint nVertices;       //num indices of mesh
    };

    //main window
    GLFWwindow* gWindow = nullptr;
    //triangle mesh
    GLMesh gMesh;
    //texture id
    GLuint gTextureId;
    glm::vec2 gUVScale(5.0f, 5.0f);
    GLint gTexWrapMode = GL_REPEAT;
    //shader
    GLuint gCubeProgramId;
    GLuint gLampProgramId;

    //camera
    Camera gCamera(glm::vec3(4.0f, 2.5f, 9.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    //timing between current frame and last frame
    float gDeltaTime = 0.0f;
    float gLastFrame = 0.0f;

    //object
    glm::vec3 gCubePosition(0.0f, 0.0f, 0.0f);
    glm::vec3 gCubeScale(2.0f);
    glm::vec3 gObjectColor(0.5f, 0.75f, 0.75f);
    
    //light
    glm::vec3 gLightPosition(2.0f, 3.0f, 3.0f);
    glm::vec3 gLightScale(0.1f);
    glm::vec3 gLightColor(1.0f, 1.0f, 1.0f);        //white

    //second light
    glm::vec3 gLightPositionTwo(8.0f, 5.0f, -8.0f);
    glm::vec3 gLightColorTwo(0.0f, 1.0f, 0.0f);     //green
    glm::vec3 gLightScaleTwo(0.2f);
}

//declare program functions
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreateMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);

//vertex shader
const GLchar* cubeVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position;          //position 0
    layout(location = 1) in vec3 normal;            //position 1
    layout(location = 2) in vec2 textureCoordinate; //position 2

    out vec3 vertexNormal;              //outgoing normals
    out vec3 vertexFragmentPos;         //outgoing color
    out vec2 vertexTextureCoordinate;   //outgoing texture

    //declare global variables
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main() {           //transform vertices
        gl_Position = projection * view * model * vec4(position, 1.0f);

        vertexFragmentPos = vec3(model * vec4(position, 1.0f));

        vertexNormal = mat3(transpose(inverse(model))) * normal;
        vertexTextureCoordinate = textureCoordinate;
    }
);

//cube fragment shader
const GLchar* cubeFragmentShaderSource = GLSL(440,

    in vec3 vertexNormal;               //incoming normals
    in vec3 vertexFragmentPos;          //incoming fragment
    in vec2 vertexTextureCoordinate;    //incoming texture

    out vec4 fragmentColor;             //cube output color

    //declare global variables
    uniform vec3 objectColor;
    uniform vec3 lightColor;
    uniform vec3 lightPos;
    uniform vec3 viewPosition;
    uniform sampler2D uTexture;     //texture variable
    uniform vec2 uvScale;
    uniform vec3 lightColorTwo;     //second light
    uniform vec3 lightPosTwo;       //second light

    void main() {           //calculate phong
        //calculate ambient
        float ambientStrength = 0.5f;                                           //ambient light intensity set at 50%
        vec3 ambient = ambientStrength * lightColor;                            //ambient light color same as fill light
        //second light
        float ambientStrengthTwo = 0.4f;                                        //second ambient light intensity set at 40%
        vec3 ambientTwo = ambientStrengthTwo * lightColorTwo;                   //second ambient light color same as fill light

        //calculate diffuse
        vec3 norm = normalize(vertexNormal);                                    //normalize vectors
        vec3 lightDirection = normalize(lightPos - vertexFragmentPos);          //calculate distance
        float impact = max(dot(norm, lightDirection), 0.0);                     //diffuse impact
        vec3 diffuse = impact * lightColor;                                     //generate diffuse light
        //second light
        vec3 lightDirectionTwo = normalize(lightPosTwo - vertexFragmentPos);    //second light calculate distance
        float impactTwo = max(dot(norm, lightDirectionTwo), 0.0);               //second light diffuse impact
        vec3 diffuseTwo = impactTwo * lightColorTwo;                            //second light generate diffuse light

        //specular lighting
        float specularIntensity = 1.0f;                                         //key specular light intensity set at 100%
        float highlightSize = 15.0f;                                            //specular light
        vec3 viewDir = normalize(viewPosition - vertexFragmentPos);             //view direction
        vec3 reflectDir = reflect(-lightDirection, norm);                       //reflection vector
        //specular component
        float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
        vec3 specular = specularIntensity * specularComponent * lightColor;
        //second light
        float specularIntensityTwo = 0.5f;                                      //second key specular light intensity set at 50%
        float highlightSizeTwo = 15.0f;                                         //second specular light
        vec3 reflectDirTwo = reflect(-lightDirectionTwo, norm);                 //second reflection vector
        //specular component second light
        float specularComponentTwo = pow(max(dot(viewDir, reflectDirTwo), 0.0), highlightSizeTwo);
        vec3 specularTwo = specularIntensityTwo * specularComponentTwo * lightColorTwo;

        //hold color to be used for all three component
        vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

        //phong result of first light + second light
        vec3 phong = (ambient + diffuse + specular) * textureColor.xyz;
        phong += (ambientTwo + diffuseTwo + specularTwo) * textureColor.xyz;

        fragmentColor = vec4(phong, 1.0f); //results to gpu
    }
);

//lamp shader
const GLchar* lampVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; //position 0

    //declare global variables
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main() {           //transform vertices
        gl_Position = projection * view * model * vec4(position, 1.0f);
    }
);

//fragment shader
const GLchar* lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor;             //lamp color

    void main() {
        fragmentColor = vec4(1.0f);     //white
    }
);

//flip y axis
void flipImageVertically(unsigned char* image, int width, int height, int channels) {
    for (int j = 0; j < height / 2; ++j) {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i) {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}

//main function to run program
int main(int argc, char* argv[]) {
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    //create mesh function call
    UCreateMesh(gMesh);

    //create cube shader program
    if (!UCreateShaderProgram(cubeVertexShaderSource, cubeFragmentShaderSource, gCubeProgramId))
        return EXIT_FAILURE;
    //create lamp shader program
    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId))
        return EXIT_FAILURE;

    //load texture
    const char* texFilename = "base.png";                           //image file name, file located in solution directory
    if (!UCreateTexture(texFilename, gTextureId)) {
        cout << "Failed to load texture!" << texFilename << endl;   //output to user load fail
        return EXIT_FAILURE;                                        //return to exit with fail status
    }
    //tell opengl which texture unit each sample belongs to
    glUseProgram(gCubeProgramId);
    //set texture as texture unit 0
    glUniform1i(glGetUniformLocation(gCubeProgramId, "uTexture"), 0);

    //set background color to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    //render loop
    while (!glfwWindowShouldClose(gWindow)) {
        //timing per frame
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        //process user input
        UProcessInput(gWindow);

        //output each frame to user
        URender();

        glfwPollEvents();
    }

    //release mesh from memory
    UDestroyMesh(gMesh);                    //entire 3D scene

    //release texture from memory
    UDestroyTexture(gTextureId);

    //release shaders from memory
    UDestroyShaderProgram(gCubeProgramId);  //all objects
    UDestroyShaderProgram(gLampProgramId);  //both lights

    //exit program with success code
    exit(EXIT_SUCCESS);
}

//initialize GLFW GLEW create window
bool UInitialize(int argc, char* argv[], GLFWwindow** window) {
    //glfw initialize
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    //create window
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

    //capture mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //initialize glew
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult) {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    //output users opengl version
    cout << "INFO: User OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;        //return true
}

//process all input for relevant key presses
void UProcessInput(GLFWwindow* window) {
    //declare and initialize camera speed constant
    static const float cameraSpeed = 2.5f;

    //if escape key, exit program
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    //if w key, move camera forward
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    //if s key, move camera backward
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    //if a key, move camera left
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    //if d key, move camera right
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);

    //if e key, move camera straight up
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(UP, gDeltaTime);
    //if q key, move camera straight down
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);

    //if p key, switch to ortho projection
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        glm::mat4 projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
    else
        glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
}

//call for window resize
void UResizeWindow(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

//called for user mouse movement
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos) {
    if (gFirstMouse) {      //if mouse movement
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    //reverse y axis
    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos;

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}

//mouse scroll
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    gCamera.ProcessMouseScroll(yoffset);
}

//mouse button
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT: {
        if (action == GLFW_PRESS)                           //if left mouse button
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
                               break;

    case GLFW_MOUSE_BUTTON_MIDDLE: {
        if (action == GLFW_PRESS)                           //if middle mouse button
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
                                 break;

    case GLFW_MOUSE_BUTTON_RIGHT: {
        if (action == GLFW_PRESS)                           //if right mouse button
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
                                break;

    default:
        cout << "Unhandled mouse button event" << endl;     //exception handling
        break;
    }
}

//render frame
void URender() {
    //z-depth
    glEnable(GL_DEPTH_TEST);

    //clear frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //activate VAO
    glBindVertexArray(gMesh.vao);

    //draw cube and set shader
    glUseProgram(gCubeProgramId);

    //matrix transformations
    glm::mat4 model = glm::translate(gCubePosition) * glm::scale(gCubeScale);

    //camera transformation
    glm::mat4 view = gCamera.GetViewMatrix();

    //perspective projection
    glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

    //pass transform matrices to shader
    GLint modelLoc = glGetUniformLocation(gCubeProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gCubeProgramId, "view");
    GLint projLoc = glGetUniformLocation(gCubeProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //reference matrix uniforms
    GLint objectColorLoc = glGetUniformLocation(gCubeProgramId, "objectColor");
    GLint lightColorLoc = glGetUniformLocation(gCubeProgramId, "lightColor");
    GLint lightPositionLoc = glGetUniformLocation(gCubeProgramId, "lightPos");
    GLint viewPositionLoc = glGetUniformLocation(gCubeProgramId, "viewPosition");
    //second light
    GLint lightColorLocTwo = glGetUniformLocation(gCubeProgramId, "lightColorTwo");
    GLint lightPositionLocTwo = glGetUniformLocation(gCubeProgramId, "lightPosTwo");

    //pass color light camera data
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    glUniform3f(lightColorLocTwo, gLightColorTwo.r, gLightColorTwo.g, gLightColorTwo.b);//second light
    glUniform3f(lightPositionLocTwo, gLightPositionTwo.x, gLightPositionTwo.y, gLightPositionTwo.z);//second light
    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    GLint UVScaleLoc = glGetUniformLocation(gCubeProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    //bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId);

    //draw triangles for mesh object
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    //lamp
    glUseProgram(gLampProgramId);

    //first light
    //transform smaller cube
    model = glm::translate(gLightPosition) * glm::scale(gLightScale);

    //lamp shader program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    //lamp shader matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //draw triangles for light object
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    //second light
    //transform smaller cube for second light source
    model = glm::translate(gLightPositionTwo) * glm::scale(gLightScaleTwo);

    //lamp shader program second light
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    //lamp shader matrix uniforms for second light
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);     //draw second light

    //deactivate VAO
    glBindVertexArray(0);
    glUseProgram(0);

    //swap buffers
    glfwSwapBuffers(gWindow);
}

//implement UCreateMesh
void UCreateMesh(GLMesh& mesh) {
    //vertex data for 3D objects
    GLfloat verts[] = {
        //PLANE: desk
        //plane drawn as two seperate triangles
        //coordinates           //surface normals           //texture coordinates
         6.0f, 0.0f,  6.0f,     0.0f, 1.0f, 0.0f,           0.0f, 0.0f,
         6.0f, 0.0f, -6.0f,     0.0f, 1.0f, 0.0f,           1.0f, 0.0f,
        -6.0f, 0.0f,  6.0f,     0.0f, 1.0f, 0.0f,           0.0f, 1.0f,

        //triangle two
        //position
         6.0f, 0.0f, -6.0f,     0.0f, 1.0f, 0.0f,           1.0f, 0.0f,
        -6.0f, 0.0f, -6.0f,     0.0f, 1.0f, 0.0f,           1.0f, 1.0f,
        -6.0f, 0.0f,  6.0f,     0.0f, 1.0f, 0.0f,           0.0f, 1.0f,


        //CYLINDER: zippo lighter hinge
        //CYLINDER enlarged for clarity and created using triangular coordinates due to limitations on time
        //I understand the cylinder could be drawn better, however, I ran short on time due to
        //work and tried to make a cylinder the best I could to avoid a zero on the project by
        //meeting the minimum project requirements by including at least four different primative shapes
        //CYLINDER BASE
        //coordinates           //surface normals           //texture coordinates
        //triangle one
        0.9f, 0.0f, -0.2f,      0.0f, -1.0f, 0.0f,      1.0f, 0.0f,
        0.9f, 0.0f, -0.1f,      0.0f, -1.0f, 0.0f,      0.0f, 0.0f,
        0.75f, 0.0f, -0.15f,    0.0f, -1.0f, 0.0f,      0.5f, 1.0f,

        //triangle two
        0.9f, 0.0f, -0.1f,      0.0f, -1.0f, 0.0f,      1.0f, 0.0f,
        0.8f, 0.0f, 0.0f,       0.0f, -1.0f, 0.0f,      0.0f, 0.0f,
        0.75f, 0.0f, -0.15f,    0.0f, -1.0f, 0.0f,      0.5f, 1.0f,

        //triangle three
        0.8f, 0.0f, 0.0f,       0.0f, -1.0f, 0.0f,      1.0f, 0.0f,
        0.7f, 0.0f, 0.0f,       0.0f, -1.0f, 0.0f,      0.0f, 0.0f,
        0.75f, 0.0f, -0.15f,    0.0f, -1.0f, 0.0f,      0.5f, 1.0f,

        //triangle four
        0.7f, 0.0f, 0.0f,       0.0f, -1.0f, 0.0f,      1.0f, 0.0f,
        0.6f, 0.0f, -0.1f,      0.0f, -1.0f, 0.0f,      0.0f, 0.0f,
        0.75f, 0.0f, -0.15f,    0.0f, -1.0f, 0.0f,      0.5f, 1.0f,

        //triangle five
        0.6f, 0.0f, -0.1f,      0.0f, -1.0f, 0.0f,      1.0f, 0.0f,
        0.6f, 0.0f, -0.2f,      0.0f, -1.0f, 0.0f,      0.0f, 0.0f,
        0.75f, 0.0f, -0.15f,    0.0f, -1.0f, 0.0f,      0.5f, 1.0f,

        //triangle six
        0.6f, 0.0f, -0.2f,      0.0f, -1.0f, 0.0f,      1.0f, 0.0f,
        0.7f, 0.0f, -0.3f,      0.0f, -1.0f, 0.0f,      0.0f, 0.0f,
        0.75f, 0.0f, -0.15f,    0.0f, -1.0f, 0.0f,      0.5f, 1.0f,

        //triangle seven
        0.7f, 0.0f, -0.3f,      0.0f, -1.0f, 0.0f,      1.0f, 0.0f,
        0.8f, 0.0f, -0.3f,      0.0f, -1.0f, 0.0f,      0.0f, 0.0f,
        0.75f, 0.0f, -0.15f,    0.0f, -1.0f, 0.0f,      0.5f, 1.0f,

        //triangle eight
        0.8f, 0.0f, -0.3f,      0.0f, -1.0f, 0.0f,      1.0f, 0.0f,
        0.9f, 0.0f, -0.2f,      0.0f, -1.0f, 0.0f,      0.0f, 0.0f,
        0.75f, 0.0f, -0.15f,    0.0f, -1.0f, 0.0f,      0.5f, 1.0f,

        //CYLINDER TOP
        //coordinates           //surface normals       //texture coordinates
        //triangle one
        0.9f, 0.2f, -0.2f,      0.0f, 1.0f, 0.0f,       1.0f, 0.0f,
        0.9f, 0.2f, -0.1f,      0.0f, 1.0f, 0.0f,       0.0f, 0.0f,
        0.75f, 0.2f, -0.15f,    0.0f, 1.0f, 0.0f,       0.5f, 1.0f,

        //triangle two
        0.9f, 0.2f, -0.1f,      0.0f, 1.0f, 0.0f,       1.0f, 0.0f,
        0.8f, 0.2f, 0.0f,       0.0f, 1.0f, 0.0f,       0.0f, 0.0f,
        0.75f, 0.2f, -0.15f,    0.0f, 1.0f, 0.0f,       0.5f, 1.0f,

        //triangle three
        0.8f, 0.2f, 0.0f,       0.0f, 1.0f, 0.0f,       1.0f, 0.0f,
        0.7f, 0.2f, 0.0f,       0.0f, 1.0f, 0.0f,       0.0f, 0.0f,
        0.75f, 0.2f, -0.15f,    0.0f, 1.0f, 0.0f,       0.5f, 1.0f,

        //triangle four
        0.7f, 0.2f, 0.0f,       0.0f, 1.0f, 0.0f,       1.0f, 0.0f,
        0.6f, 0.2f, -0.1f,      0.0f, 1.0f, 0.0f,       0.0f, 0.0f,
        0.75f, 0.2f, -0.15f,    0.0f, 1.0f, 0.0f,       0.5f, 1.0f,

        //triangle five
        0.6f, 0.2f, -0.1f,      0.0f, 1.0f, 0.0f,       1.0f, 0.0f,
        0.6f, 0.2f, -0.2f,      0.0f, 1.0f, 0.0f,       0.0f, 0.0f,
        0.75f, 0.2f, -0.15f,    0.0f, 1.0f, 0.0f,       0.5f, 1.0f,

        //triangle six
        0.6f, 0.2f, -0.2f,      0.0f, 1.0f, 0.0f,       1.0f, 0.0f,
        0.7f, 0.2f, -0.3f,      0.0f, 1.0f, 0.0f,       0.0f, 0.0f,
        0.75f, 0.2f, -0.15f,    0.0f, 1.0f, 0.0f,       0.5f, 1.0f,

        //triangle seven
        0.7f, 0.2f, -0.3f,      0.0f, 1.0f, 0.0f,       1.0f, 0.0f,
        0.8f, 0.2f, -0.3f,      0.0f, 1.0f, 0.0f,       0.0f, 0.0f,
        0.75f, 0.2f, -0.15f,    0.0f, 1.0f, 0.0f,       0.5f, 1.0f,

        //triangle eight
        0.8f, 0.2f, -0.3f,      0.0f, 1.0f, 0.0f,       1.0f, 0.0f,
        0.9f, 0.2f, -0.2f,      0.0f, 1.0f, 0.0f,       0.0f, 0.0f,
        0.75f, 0.2f, -0.15f,    0.0f, 1.0f, 0.0f,       0.5f, 1.0f,


        //CYLINDER WALLS
        //coordinates           //surface normals       //texture coordinates
        //sector 1
        //triangle one
        0.9f, 0.2f, -0.1f,      1.0f, 0.0f, 0.0f,       0.0f, 1.0f,
        0.9f, 0.2f, -0.2f,      1.0f, 0.0f, 0.0f,       1.0f, 1.0f,
        0.9f, 0.0f, -0.1f,      1.0f, 0.0f, 0.0f,       0.0f, 0.0f,

        //triangle two
        0.9f, 0.2f, -0.2f,      1.0f, 0.0f, 0.0f,       1.0f, 1.0f,
        0.9f, 0.0f, -0.1f,      1.0f, 0.0f, 0.0f,       0.0f, 0.0f,
        0.9f, 0.0f, -0.2f,      1.0f, 0.0f, 0.0f,       1.0f, 0.0f,

        //sector 2
        //triangle three
        0.8f, 0.2f, 0.0f,       0.5f, 0.0f, 0.5f,       0.0f, 1.0f,
        0.9f, 0.2f, -0.1f,      0.5f, 0.0f, 0.5f,       1.0f, 1.0f,
        0.8f, 0.0f, 0.0f,       0.5f, 0.0f, 0.5f,       0.0f, 0.0f,

        //triangle four
        0.9f, 0.2f, -0.1f,      0.5f, 0.0f, 0.5f,       1.0f, 1.0f,
        0.8f, 0.0f, 0.0f,       0.5f, 0.0f, 0.5f,       0.0f, 0.0f,
        0.9f, 0.0f, -0.1f,      0.5f, 0.0f, 0.5f,       1.0f, 0.0f,

        //sector 3
        //triangle five
        0.7f, 0.2f, 0.0f,       0.0f, 0.0f, 1.0f,       0.0f, 1.0f,
        0.8f, 0.2f, 0.0f,       0.0f, 0.0f, 1.0f,       1.0f, 1.0f,
        0.7f, 0.0f, 0.0f,       0.0f, 0.0f, 1.0f,       0.0f, 0.0f,

        //triangle six
        0.8f, 0.2f, 0.0f,       0.0f, 0.0f, 1.0f,       1.0f, 1.0f,
        0.7f, 0.0f, 0.0f,       0.0f, 0.0f, 1.0f,       0.0f, 0.0f,
        0.8f, 0.0f, 0.0f,       0.0f, 0.0f, 1.0f,       1.0f, 0.0f,

        //sector 4
        //triangle seven
        0.6f, 0.2f, -0.1f,      -0.5f, 0.0f, 0.5f,      0.0f, 1.0f,
        0.7f, 0.2f, 0.0f,       -0.5f, 0.0f, 0.5f,      1.0f, 1.0f,
        0.6f, 0.0f, -0.1f,      -0.5f, 0.0f, 0.5f,      0.0f, 0.0f,

        //triangle eight
        0.7f, 0.2f, 0.0f,       -0.5f, 0.0f, 0.5f,      1.0f, 1.0f,
        0.6f, 0.0f, -0.1f,      -0.5f, 0.0f, 0.5f,      0.0f, 0.0f,
        0.7f, 0.0f, 0.0f,       -0.5f, 0.0f, 0.5f,      1.0f, 0.0f,

        //sector 5
        //triangle nine
        0.6f, 0.2f, -0.2f,      -1.0f, 0.0f, 0.0f,      0.0f, 1.0f,
        0.6f, 0.2f, -0.1f,      -1.0f, 0.0f, 0.0f,      1.0f, 1.0f,
        0.6f, 0.0f, -0.2f,      -1.0f, 0.0f, 0.0f,      0.0f, 0.0f,

        //triangle ten
        0.6f, 0.2f, -0.1f,      -1.0f, 0.0f, 0.0f,      1.0f, 1.0f,
        0.6f, 0.0f, -0.2f,      -1.0f, 0.0f, 0.0f,      0.0f, 0.0f,
        0.6f, 0.0f, -0.1f,      -1.0f, 0.0f, 0.0f,      1.0f, 0.0f,

        //sector 6
        //triangle eleven
        0.7f, 0.2f, -0.3f,      -0.5f, 0.0f, -0.5f,     0.0f, 1.0f,
        0.6f, 0.2f, -0.2f,      -0.5f, 0.0f, -0.5f,     1.0f, 1.0f,
        0.7f, 0.0f, -0.3f,      -0.5f, 0.0f, -0.5f,     0.0f, 0.0f,

        //triangle twelve
        0.6f, 0.2f, -0.2f,      -0.5f, 0.0f, -0.5f,     1.0f, 1.0f,
        0.7f, 0.0f, -0.3f,      -0.5f, 0.0f, -0.5f,     0.0f, 0.0f,
        0.6f, 0.0f, -0.2f,      -0.5f, 0.0f, -0.5f,     1.0f, 0.0f,

        //sector 7
        //triangle thirteen
        0.8f, 0.2f, -0.3f,      0.0f, 0.0f, -1.0f,      0.0f, 1.0f,
        0.7f, 0.2f, -0.3f,      0.0f, 0.0f, -1.0f,      1.0f, 1.0f,
        0.8f, 0.0f, -0.3f,      0.0f, 0.0f, -1.0f,      0.0f, 0.0f,

        //triangle fourteen
        0.7f, 0.2f, -0.3f,      0.0f, 0.0f, -1.0f,      1.0f, 1.0f,
        0.8f, 0.0f, -0.3f,      0.0f, 0.0f, -1.0f,      0.0f, 0.0f,
        0.7f, 0.0f, -0.3f,      0.0f, 0.0f, -1.0f,      1.0f, 0.0f,

        //sector 8
        //triangle fifteen
        0.9f, 0.2f, -0.2f,      0.5f, 0.0f, -0.5f,      0.0f, 1.0f,
        0.8f, 0.2f, -0.3f,      0.5f, 0.0f, -0.5f,      1.0f, 1.0f,
        0.9f, 0.0f, -0.2f,      0.5f, 0.0f, -0.5f,      0.0f, 0.0f,

        //triangle sixteen
        0.8f, 0.2f, -0.3f,      0.5f, 0.0f, -0.5f,      1.0f, 1.0f,
        0.9f, 0.0f, -0.2f,      0.5f, 0.0f, -0.5f,      0.0f, 0.0f,
        0.8f, 0.0f, -0.3f,      0.5f, 0.0f, -0.5f,      1.0f, 0.0f,

        //CUBE: zippo lighter base and cap
        //reused from milestone six
        //coordinates           //surface normals           //texture coordinates
        //triangle one
        0.0f, 0.0f, 0.0f,       0.0f, -1.0f, 0.0f,          0.0f, 0.0f,
        0.75f, 0.0f, 0.0f,      0.0f, -1.0f, 0.0f,          0.0f, 1.0f,
        0.75f, 0.0f, 0.75f,     0.0f, -1.0f, 0.0f,          1.0f, 1.0f,

        //triangle two
        0.0f, 0.0f, 0.0f,       0.0f, -1.0f, 0.0f,          0.0f, 0.0f,
        0.75f, 0.0f, 0.75f,     0.0f, -1.0f, 0.0f,          1.0f, 1.0f,
        0.0f, 0.0f, 0.75f,      0.0f, -1.0f, 0.0f,          1.0f, 0.0f,

        //triangle three
        0.75f, 0.0f, 0.0f,      0.0f, -1.0f, 0.0f,          1.0f, 0.0f,
        0.75f, 0.0f, 0.75f,     0.0f, -1.0f, 0.0f,          0.0f, 0.0f,
        1.0f, 0.0f, 0.75f,      0.0f, -1.0f, 0.0f,          0.0f, 1.0f,

        //triangle four
        0.75f, 0.0f, 0.0f,      0.0f, -1.0f, 0.0f,          1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,       0.0f, -1.0f, 0.0f,          1.0f, 1.0f,
        1.0f, 0.0f, 0.75f,      0.0f, -1.0f, 0.0f,          0.0f, 1.0f,

        //triangle five
        0.0f, 0.0f, 0.0f,       0.0f, 0.0f, -1.0f,          1.0f, 0.0f,
        0.75, 0.2f, 0.0f,       0.0f, 0.0f, -1.0f,          0.0f, 1.0f,
        0.0f, 0.2f, 0.0f,       0.0f, 0.0f, -1.0f,          1.0f, 1.0f,

        //triangle six
        0.0f, 0.0f, 0.0f,       0.0f, 0.0f, -1.0f,          1.0f, 0.0f,
        0.75, 0.0f, 0.0f,       0.0f, 0.0f, -1.0f,          0.f, 0.0f,
        0.75f, 0.2f, 0.0f,      0.0f, 0.0f, -1.0f,          0.0f, 1.0f,

        //triangle seven
        0.75f, 0.0f, 0.0f,      0.0f, 0.0f, -1.0f,          1.0f, 0.0f,
        1.0f, 0.2f, 0.0f,       0.0f, 0.0f, -1.0f,          0.0f, 1.0f,
        0.75f, 0.2f, 0.0f,      0.0f, 0.0f, -1.0f,          1.0f, 1.0f,

        //triangle eight
        0.75f, 0.0f, 0.0f,      0.0f, 0.0f, -1.0f,          1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,       0.0f, 0.0f, -1.0f,          0.0f, 0.0f,
        1.0f, 0.2f, 0.0f,       0.0f, 0.0f, -1.0f,          0.0f, 1.0f,

        //triangle nine
        1.0f, 0.0f, 0.0f,       1.0f, 0.0f, 0.0f,           1.0f, 0.0f,
        1.0f, 0.2f, 0.0f,       1.0f, 0.0f, 0.0f,           1.0f, 1.0f,
        1.0f, 0.2f, 0.75f,      1.0f, 0.0f, 0.0f,           0.0f, 1.0f,

        //triangle ten
        1.0f, 0.0f, 0.0f,       1.0f, 0.0f, 0.0f,           1.0f, 0.0f,
        1.0f, 0.2f, 0.75f,      1.0f, 0.0f, 0.0f,           0.0f, 1.0f,
        1.0f, 0.0f, 0.75f,      1.0f, 0.0f, 0.0f,           0.0f, 0.0f,

        //triangle eleven
        1.0f, 0.0f, 0.75f,      0.0f, 0.0f, 1.0f,           1.0f, 0.0f,
        1.0f, 0.2f, 0.75f,      0.0f, 0.0f, 1.0f,           1.0f, 1.0f,
        0.75f, 0.0f, 0.75f,     0.0f, 0.0f, 1.0f,           0.0f, 0.0f,

        //triangle twelve
        1.0f, 0.2f, 0.75f,      0.0f, 0.0f, 1.0f,           1.0f, 1.0f,
        0.75f, 0.2f, 0.75f,     0.0f, 0.0f, 1.0f,           0.0f, 1.0f,
        0.75f, 0.0f, 0.75f,     0.0f, 0.0f, 1.0f,           0.0f, 0.0f,

        //triangle thirteen
        0.75f, 0.0f, 0.75f,     0.0f, 0.0f, 1.0f,           1.0f, 0.0f,
        0.75f, 0.2f, 0.75f,     0.0f, 0.0f, 1.0f,           1.0f, 1.0f,
        0.0f, 0.0f, 0.75f,      0.0f, 0.0f, 1.0f,           0.0f, 0.0f,

        //triangle fourteen
        0.75f, 0.2f, 0.75f,     0.0f, 0.0f, 1.0f,           1.0f, 1.0f,
        0.0f, 0.2f, 0.75f,      0.0f, 0.0f, 1.0f,           0.0f, 1.0f,
        0.0f, 0.0f, 0.75f,      0.0f, 0.0f, 1.0f,           0.0f, 0.0f,

        //triangle fifteen
        0.0f, 0.0f, 0.0f,       -1.0f, 0.0f, 0.0f,          0.0f, 0.0f,
        0.0f, 0.0f, 0.75f,      -1.0f, 0.0f, 0.0f,          1.0f, 0.0f,
        0.0f, 0.2f, 0.75f,      -1.0f, 0.0f, 0.0f,          1.0f, 1.0f,

        //triangle sixteen
        0.0f, 0.0f, 0.0f,       -1.0f, 0.0f, 0.0f,          0.0f, 0.0f,
        0.0f, 0.2f, 0.0f,       -1.0f, 0.0f, 0.0f,          0.0f, 1.0f,
        0.0f, 0.2f, 0.75f,      -1.0f, 0.0f, 0.0f,          1.0f, 1.0f,

        //triangle seventeen
        0.75f, 0.0f, 0.0f,      1.0f, 0.0f, 0.0f,           0.0f, 0.0f,
        0.75f, 0.2f, 0.0f,      1.0f, 0.0f, 0.0f,           0.0f, 1.0f,
        0.75f, 0.2f, 0.75f,     1.0f, 0.0f, 0.0f,           1.0f, 1.0f,

        //triangle eighteen
        0.75f, 0.0f, 0.0f,      1.0f, 0.0f, 0.0f,           0.0f, 0.0f,
        0.75f, 0.0f, 0.75f,     1.0f, 0.0f, 0.0f,           1.0f, 0.0f,
        0.75f, 0.2f, 0.75f,     1.0f, 0.0f, 0.0f,           1.0f, 1.0f,

        //triangle nineteen
        0.0f, 0.2f, 0.0f,       0.0f, 1.0f, 0.0f,           0.0f, 0.0f,
        0.75f, 0.2f, 0.0f,      0.0f, 1.0f, 0.0f,           0.0f, 1.0f,
        0.75f, 0.2f, 0.75f,     0.0f, 1.0f, 0.0f,           1.0f, 1.0f,

        //triangle twenty
        0.0f, 0.2f, 0.0f,       0.0f, 1.0f, 0.0f,           0.0f, 0.0f,
        0.75f, 0.2f, 0.75f,     0.0f, 1.0f, 0.0f,           1.0f, 1.0f,
        0.0f, 0.2f, 0.75f,      0.0f, 1.0f, 0.0f,           1.0f, 0.0f,

        //triangle twentyone
        0.75f, 0.2f, 0.0f,      0.0f, 1.0f, 0.0f,           1.0f, 1.0f,
        1.0f, 0.2f, 0.0f,       0.0f, 1.0f, 0.0f,           1.0f, 0.0f,
        1.0f, 0.2f, 0.75f,      0.0f, 1.0f, 0.0f,           0.0f, 0.0f,

        //triangle twentytwo
        1.0f, 0.2f, 0.75f,      0.0f, 1.0f, 0.0f,           0.0f, 0.0f,
        0.75f, 0.2f, 0.0f,      0.0f, 1.0f, 0.0f,           1.0f, 1.0f,
        0.75f, 0.2f, 0.75f,     0.0f, 1.0f, 0.0f,           0.0f, 1.0f,


        //PYRAMID: car key
        //the car key could be drawn more realistic, however, I ran short on time and tried
        //to make a pyramid the best I could to include at least four different primative shapes
        //the car key will be made up of a cube by using several triangles, and will have the
        //pyramid at the top, and the car key will be drawn laying on its side, without the key chain.
        //cube base
        //coordinates           //surface normals       //texture coordinates
        //triangle one
        2.0f, 0.0f, 0.0f,       0.0f, -1.0f, 0.0f,      1.0f, 0.0f,
        3.0f, 0.0f, 0.8f,       0.0f, -1.0f, 0.0f,      0.0f, 1.0f,
        2.0f, 0.0f, 0.8f,       0.0f, -1.0f, 0.0f,      0.0f, 0.0f,

        //triangle two
        2.0f, 0.0f, 0.0f,       0.0f, -1.0f, 0.0f,      1.0f, 0.0f,
        3.0f, 0.0f, 0.0f,       0.0f, -1.0f, 0.0f,      1.0f, 1.0f,
        3.0f, 0.0f, 0.8f,       0.0f, -1.0f, 0.0f,      0.0f, 1.0f,

        //triangle three
        2.0f, 0.0f, 0.0f,       0.0f, 0.0f, -1.0f,      1.0f, 0.0f,
        3.0f, 0.25f, 0.0f,      0.0f, 0.0f, -1.0f,      0.0f, 1.0f,
        2.0f, 0.25f, 0.0f,      0.0f, 0.0f, -1.0f,      1.0f, 1.0f,

        //triangle four
        2.0f, 0.0f, 0.0f,       0.0f, 0.0f, -1.0f,      1.0f, 0.0f,
        3.0f, 0.25f, 0.0f,      0.0f, 0.0f, -1.0f,      0.0f, 1.0f,
        3.0f, 0.0f, 0.0f,       0.0f, 0.0f, -1.0f,      0.0f, 0.0f,

        //triangle five
        3.0f, 0.25f, 0.8f,      0.0f, 0.0f, 1.0f,       1.0f, 1.0f,
        3.0f, 0.0f, 0.8f,       0.0f, 0.0f, 1.0f,       1.0f, 0.0f,
        2.0f, 0.0f, 0.8f,       0.0f, 0.0f, 1.0f,       0.0f, 0.0f,

        //triangle six
        3.0f, 0.25f, 0.8f,      0.0f, 0.0f, 1.0f,       1.0f, 1.0f,
        2.0f, 0.0f, 0.8f,       0.0f, 0.0f, 1.0f,       0.0f, 0.0f,
        2.0f, 0.25f, 0.8f,      0.0f, 0.0f, 1.0f,       0.0f, 1.0f,

        //triangle seven
        2.0f, 0.0f, 0.0f,       -1.0f, 0.0f, 0.0f,      0.0f, 0.0f,
        2.0f, 0.0f, 0.8f,       -1.0f, 0.0f, 0.0f,      1.0f, 0.0f,
        2.0f, 0.25f, 0.8f,      -1.0f, 0.0f, 0.0f,      1.0f, 1.0f,

        //triangle eight
        2.0f, 0.0f, 0.0f,       -1.0f, 0.0f, 0.0f,      0.0f, 0.0f,
        2.0f, 0.25f, 0.8f,      -1.0f, 0.0f, 0.0f,      1.0f, 1.0f,
        2.0f, 0.25f, 0.0f,      -1.0f, 0.0f, 0.0f,      0.0f, 1.0f,

        //triangle nine
        2.0f, 0.25f, 0.0f,      0.0f, 1.0f, 0.0f,       0.0f, 0.0f,
        3.0f, 0.25f, 0.8f,      0.0f, 1.0f, 0.0f,       1.0f, 1.0f,
        2.0f, 0.25f, 0.8f,      0.0f, 1.0f, 0.0f,       1.0f, 0.0f,

        //triangle ten
        2.0f, 0.25f, 0.0f,      0.0f, 1.0f, 0.0f,       0.0f, 0.0f,
        3.0f, 0.25f, 0.8f,      0.0f, 1.0f, 0.0f,       1.0f, 1.0f,
        3.0f, 0.25f, 0.0f,      0.0f, 1.0f, 0.0f,       0.0f, 1.0f,

        //PYRAMID top
        //coordinates           //surface normals       //texture coordinates
        //triangle one
        3.0f, 0.25f, 0.0f,      0.5f, 0.5f, 0.0f,       0.0f, 0.0f,
        3.2f, 0.125f, 0.4f,     0.5f, 0.5f, 0.0f,       0.5f, 1.0f,
        3.0f, 0.25f, 0.8f,      0.5f, 0.5f, 0.0f,       1.0f, 0.0f,

        //triangle two
        3.0f, 0.25f, 0.0f,      0.5f, 0.0f, -0.5f,      1.0f, 0.0f,
        3.0f, 0.0f, 0.0f,       0.5f, 0.0f, -0.5f,      0.0f, 0.0f,
        3.2f, 0.125f, 0.4f,     0.5f, 0.0f, -0.5f,      0.5f, 1.0f,

        //triangle three
        3.0f, 0.0f, 0.0f,       0.5f, -0.5f, 0.0f,      1.0f, 0.0f,
        3.2f, 0.125f, 0.4f,     0.5f, -0.5f, 0.0f,      0.5f, 1.0f,
        3.0f, 0.0f, 0.8f,       0.5f, -0.5f, 0.0f,      0.0f, 0.0f,

        //triangle four
        3.0f, 0.0f, 0.8f,       0.5f, 0.0f, 0.5f,       1.0f, 0.0f,       
        3.2f, 0.125f, 0.4f,     0.5f, 0.0f, 0.5f,       0.5f, 1.0f,
        3.0f, 0.25f, 0.8f,      0.5f, 0.0f, 0.5f,       0.0f, 0.0f,
        
        //triangle five
        3.0f, 0.0f, 0.0f,       1.0f, 0.0f, 0.0f,       1.0f, 0.0f,
        3.0f, 0.25f, 0.0f,      1.0f, 0.0f, 0.0f,       1.0f, 1.0f,
        3.0f, 0.25f, 0.8f,      1.0f, 0.0f, 0.0f,       0.0f, 1.0f,

        //triangle six
        3.0f, 0.0f, 0.0f,       1.0f, 0.0f, 0.0f,       1.0f, 0.0f,
        3.0f, 0.25f, 0.8f,      1.0f, 0.0f, 0.0f,       0.0f, 1.0f,
        3.0f, 0.0f, 0.8f,       1.0f, 0.0f, 0.0f,       0.0f, 0.0f,
        

        //CUBE: calculator
        //coordinates           //surface normals       //texture coordinates
        //triangle one
        0.0f, 0.0f, -5.0f,      0.0f, -1.0f, 0.0f,      1.0f, 0.0f,
        4.0f, 0.0f, -3.0f,      0.0f, -1.0f, 0.0f,      0.0f, 1.0f,
        0.0f, 0.0f, -3.0f,      0.0f, -1.0f, 0.0f,      0.0f, 0.0f,

        //triangle two
        0.0f, 0.0f, -5.0f,      0.0f, -1.0f, 0.0f,      1.0f, 0.0f,
        4.0f, 0.0f, -3.0f,      0.0f, -1.0f, 0.0f,      0.0f, 1.0f,
        4.0f, 0.0f, -5.0f,      0.0f, -1.0f, 0.0f,      1.0f, 1.0f,

        //triangle three
        0.0f, 0.0f, -5.0f,      0.0f, 0.0f, -1.0f,      1.0f, 0.0f,
        0.0f, 0.4f, -5.0f,      0.0f, 0.0f, -1.0f,      1.0f, 1.0f,
        4.0f, 0.4f, -5.0f,      0.0f, 0.0f, -1.0f,      0.0f, 1.0f,

        //triangle four
        0.0f, 0.0f, -5.0f,      0.0f, 0.0f, -1.0f,      1.0f, 0.0f,
        4.0f, 0.4f, -5.0f,      0.0f, 0.0f, -1.0f,      0.0f, 1.0f,
        4.0f, 0.0f, -5.0f,      0.0f, 0.0f, -1.0f,      0.0f, 0.0f,

        //triangle five
        4.0f, 0.4f, -5.0f,      1.0f, 0.0f, 0.0f,       1.0f, 1.0f,
        4.0f, 0.0f, -5.0f,      1.0f, 0.0f, 0.0f,       1.0f, 0.0f,
        4.0f, 0.4f, -3.0f,      1.0f, 0.0f, 0.0f,       0.0f, 1.0f,

        //triangle six
        4.0f, 0.0f, -5.0f,      1.0f, 0.0f, 0.0f,       1.0f, 0.0f,
        4.0f, 0.4f, -3.0f,      1.0f, 0.0f, 0.0f,       0.0f, 1.0f,
        4.0f, 0.0f, -3.0f,      1.0f, 0.0f, 0.0f,       0.0f, 0.0f,

        //triangle seven
        4.0f, 0.0f, -3.0f,      0.0f, 0.0f, 1.0f,       1.0f, 0.0f,
        4.0f, 0.4f, -3.0f,      0.0f, 0.0f, 1.0f,       1.0f, 1.0f,
        0.0f, 0.0f, -3.0f,      0.0f, 0.0f, 1.0f,       0.0f, 0.0f,

        //triangle eight
        4.0f, 0.4f, -3.0f,      0.0f, 0.0f, 1.0f,       1.0f, 1.0f,
        0.0f, 0.0f, -3.0f,      0.0f, 0.0f, 1.0f,       0.0f, 0.0f,
        0.0f, 0.4f, -3.0f,      0.0f, 0.0f, 1.0f,       0.0f, 1.0f,

        //triangle nine
        0.0f, 0.0f, -3.0f,      -1.0f, 0.0f, 0.0f,      1.0f, 0.0f,
        0.0f, 0.4f, -3.0f,      -1.0f, 0.0f, 0.0f,      1.0f, 1.0f,
        0.0f, 0.0f, -5.0f,      -1.0f, 0.0f, 0.0f,      0.0f, 0.0f,

        //triangle ten
        0.0f, 0.4f, -3.0f,      -1.0f, 0.0f, 0.0f,      1.0f, 1.0f,
        0.0f, 0.0f, -5.0f,      -1.0f, 0.0f, 0.0f,      0.0f, 0.0f,
        0.0f, 0.4f, -5.0f,      -1.0f, 0.0f, 0.0f,      0.0f, 1.0f,

        //triangle eleven
        0.0f, 0.4f, -5.0f,      0.0f, 1.0f, 0.0f,       0.0f, 0.0f,
        4.0f, 0.4f, -3.0f,      0.0f, 1.0f, 0.0f,       1.0f, 1.0f,
        0.0f, 0.4f, -3.0f,      0.0f, 1.0f, 0.0f,       1.0f, 0.0f,

        //triangle twelve
        0.0f, 0.4f, -5.0f,      0.0f, 1.0f, 0.0f,       0.0f, 0.0f,
        4.0f, 0.4f, -3.0f,      0.0f, 1.0f, 0.0f,       1.0f, 1.0f,
        4.0f, 0.4f, -5.0f,      0.0f, 1.0f, 0.0f,       0.0f, 1.0f,

        //cylinder - left half
        //coordinates           //surface normals           //texture coordinates
        //triangle one
        //DID NOT HAVE TIME TO COMPLETE THIS SECTION

        //cylinder - right half
        //coordinates           //surface normals           //texture coordinates
        //triangle one
        //DID NOT HAVE TIME TO COMPLETE THIS SECTION


        //PLANE: ruler
        //coordinates           //surface normals           //texture coordinates
        //trianle one
        0.0f, 0.1f, -1.75f,     0.0f, 1.0f, 0.0f,           0.0f, 0.0f,
        0.0f, 0.1f, -1.0f,      0.0f, 1.0f, 0.0f,           1.0f, 0.0f,
        6.0f, 0.1f, -1.0f,      0.0f, 1.0f, 0.0f,           1.0f, 1.0f,

        //triangle two
        0.0f, 0.1f, -1.75f,     0.0f, 1.0f, 0.0f,           0.0f, 0.0f,
        6.0f, 0.1f, -1.75f,     0.0f, 1.0f, 0.0f,           0.0f, 1.0f,
        6.0f, 0.1f, -1.0f,      0.0f, 1.0f, 0.0f,           1.0f, 1.0f
    };

    const GLuint floatsPerVertex = 3;       //position
    const GLuint floatsPerNormal = 3;       //normals
    const GLuint floatsPerUV = 2;           //texture

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    //create 2 buffers
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);        //activate buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);    //send vertex data to GPU

    //count between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    //create vertex attribute pointers
    //position
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);
    //normals
    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
    //texture
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

//release mesh data from memory
void UDestroyMesh(GLMesh& mesh) {
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
}

//create texture
bool UCreateTexture(const char* filename, GLuint& textureId) {
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image) {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        //set wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        //set filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0);            //unbind texture

        return true;        //return true
    }

    //error
    return false;           //return false
}

//release texture from memory
void UDestroyTexture(GLuint textureId) {
    glGenTextures(1, &textureId);
}

//create shader function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId) {
    //error reporting
    int success = 0;
    char infoLog[512];

    //create shader object
    programId = glCreateProgram();

    //create fragment shader object
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    //retreive shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    //compile vertex shader
    glCompileShader(vertexShaderId);
    //error check
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success) {         //if fail
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;       //return false
    }

    glCompileShader(fragmentShaderId);      //compile fragment shader
    //error check
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;       //return false
    }

    //attach compiled shaders to shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   //link program and shader
    //error check
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;           //return false
    }

    //use shader
    glUseProgram(programId);

    return true;                //return true
}

//release shader from memory
void UDestroyShaderProgram(GLuint programId) {
    glDeleteProgram(programId);
}