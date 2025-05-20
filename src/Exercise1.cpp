#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <functional>

#include "../include/MarchingCubes.h"
#include "../include/ComputeNormals.h"
#include "../include/PlyWriter.h"

// create V and P matrices
glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1000.0f / 1000.0f, 0.001f, 1000.0f);
glm::vec3 cameraPosition(5.0f, 5.0f, 5.0f);
glm::vec3 cameraDirection(0.0f, 0.0f, 0.0f);
glm::vec3 up(0.0f, 1.0f, 0.0f);
glm::mat4 view = glm::lookAt(cameraPosition, cameraDirection, up);

// scalar field funcs
float f1(float x, float y, float z) {
    return y - (sin(x) * cos(z));
}

float f2(float x, float y, float z) {
    return x * x - y * y - z * z - z;
}

// function to setup shaders for drawing the cube wireframe
void setupShadersForCube(float min, float max, GLuint& VAO, GLuint& VBO, GLuint& EBO, GLuint& axesVAO, GLuint& axesVBO, GLuint& axesEBO, GLuint& shaderProgram) {
    // vertices of the cube
    float vertices[] = {
        min, min, min, 0.0f, 0.0f, 0.0f,  
        max, min, min, 0.0f, 0.0f, 0.0f,
        max, max, min, 0.0f, 0.0f, 0.0f,
        min, max, min, 0.0f, 0.0f, 0.0f,
        min, min, max, 0.0f, 0.0f, 0.0f,
        max, min, max, 0.0f, 0.0f, 0.0f,
        max, max, max, 0.0f, 0.0f, 0.0f,
        min, max, max, 0.0f, 0.0f, 0.0f, 
    };

    // indices which make up cube edges
    unsigned int indices[] = {
        0, 1,  1, 2,  2, 3,  3, 0,
        4, 5,  5, 6,  6, 7,  7, 4,
        0, 4,  1, 5,  2, 6,  3, 7
    };

    // vertices for the axes of the cube
    float axesVertices[] = {
        // x-axis (red)
        min, min, min,  1.0f, 0.0f, 0.0f,
        max + 0.2f, min, min,  1.0f, 0.0f, 0.0f,

        // y-axis (blue)
        min, min, min,  0.0f, 1.0f, 0.0f,
        min, max + 0.2f, min,  0.0f, 1.0f, 0.0f,

        // z-axis (green)
        min, min, min,  0.0f, 0.0f, 1.0f,
        min, min, max + 0.2f,  0.0f, 0.0f, 1.0f,

        // x-axis triangle tip (red)
        max + 0.4f, min, min, 1.0f, 0.0f, 0.0f,
        max + 0.18f, min + 0.08f, min, 1.0f, 0.0f, 0.0f,
        max + 0.18f, min - 0.08f, min, 1.0f, 0.0f, 0.0f,

        // y-axis triangle tip (blue)
        min, max + 0.4f, min, 0.0f, 1.0f, 0.0f,
        min, max + 0.18f, min + 0.08f, 0.0f, 1.0f, 0.0f,
        min, max + 0.18f, min - 0.08f, 0.0f, 1.0f, 0.0f,

        // z-axis triangle tip (green)
        min, min, max + 0.4f, 0.0f, 0.0f, 1.0f,
        min, min + 0.08f, max + 0.18f, 0.0f, 0.0f, 1.0f,
        min, min - 0.08f, max + 0.18f, 0.0f, 0.0f, 1.0f
    };

    // indices for axes
    unsigned int axesIndices[] = {
    0, 1,  
    2, 3, 
    4, 5,
    6, 7, 8, // x-axis triangle
    9, 10, 11, // y-axis triangle
    12, 13, 14  // z-axis triangle
    };

    // vertex shader source
    const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aColor;
    out vec3 fragColor;
    uniform mat4 projection;
    uniform mat4 view;
    uniform mat4 model;
    void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
        fragColor = aColor;
    })";
    // fragment shader source
    const char* fragmentShaderSource = R"(
    #version 330 core
    in vec3 fragColor;
    out vec4 FragColor;
    void main() {
        FragColor = vec4(fragColor, 1.0);
    })";

    // create and compile the vertex and fragment shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // create the shader program and attach shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // cleanup
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // create VAO, VBO, EBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    // bind VAO
    glBindVertexArray(VAO);

    // bind and fill VBO and EBO with the vertices of the cube and edge indices
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    /// color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // for axes
    glGenVertexArrays(1, &axesVAO);
    glGenBuffers(1, &axesVBO);
    glGenBuffers(1, &axesEBO);
   
    glBindVertexArray(axesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, axesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axesVertices), axesVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, axesEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(axesIndices), axesIndices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glUseProgram(0);
}

// function to draw the cube edges
void drawCubeEdges(GLuint VAO, GLuint axesVAO, GLuint shaderProgram) {

    glm::mat4 model = glm::mat4(1.0f);

    // use the shader program 
    glUseProgram(shaderProgram);

    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // draw the edges of the cube as lines
    glLineWidth(1.5f);
    glBindVertexArray(VAO);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);

    // draw the axes of the cube
    glLineWidth(6.0f);
    glBindVertexArray(axesVAO);
    glDrawElements(GL_LINES, 6, GL_UNSIGNED_INT, 0);

    // draw the triangles at the tips of the axes
    glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, (void*)(6 * sizeof(unsigned int))); 

    glBindVertexArray(0);
    glUseProgram(0);
}

// function to set up shaders for marching volume
void setupShadersForMarching(GLuint& VAO, GLuint& VBOvertices, GLuint& VBOnormals, GLuint& shaderProgram, 
    std::vector<float>& vertices, std::vector<float>& normals) {

    // vertex shader source
    const char* vertexShaderSource = R"(
    #version 330 core
    layout(location = 0) in vec3 vertexPosition;
    layout(location = 1) in vec3 vertexNormal;
    out vec3 normal;
    out vec3 eye_dir;
    out vec3 light_dir;
    uniform mat4 MVP;
    uniform mat4 V;
    uniform vec3 lightDir;
    void main() {
        // compute position and normal
        gl_Position =  MVP * vec4(vertexPosition,1);
        normal = mat3(V) * vertexNormal;
        // compute eye and light directions to be passed to frag shader
        eye_dir = vec3(5, 5, 5) - (V * vec4(-vertexPosition, 1)).xyz;
        light_dir = (mat3(V) * lightDir) + eye_dir;
    })";
    // fragment shader source
    const char* fragmentShaderSource = R"(
    #version 330 core
    in vec3 normal;
    in vec3 eye_dir;
    in vec3 light_dir;
    out vec4 color;
    uniform vec4 modelColor;
    void main() {
        vec4 diffuseColor = modelColor;
        // ambient color
        vec4 ambient = vec4(0.2, 0.2, 0.2, 1) * diffuseColor;
        // specular color
        vec4 specular = vec4(1, 1, 1, 1);
        // shininess
        float alpha = 64;
        vec3 norm = normalize(normal);
        vec3 light = normalize(light_dir);
        vec3 eye = normalize(eye_dir);
        vec3 ref = reflect(-light, norm);
        float cosTheta = clamp(dot(norm, light), 0, 1);
        float cosAlpha = clamp(dot(eye, ref), 0, 1);
        // compute combination of colors
        color = ambient + (modelColor * cosTheta * 0.8) + (specular * pow(cosAlpha, alpha));
    })";

    // create and compile the vertex and fragment shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // create the shader program and attach shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // cleanup
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // create VBOs and VAO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBOvertices);
    glGenBuffers(1, &VBOnormals);

    // bind VAO
    glBindVertexArray(VAO);

    // bind and fill VBOs with vertices and normals
    glBindBuffer(GL_ARRAY_BUFFER, VBOvertices);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    // vertex attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, VBOnormals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glUseProgram(0);
}

// function to draw marching volume
void drawMarch(GLuint VAO, GLuint shaderProgram, std::vector<float>& vertices) {

    glm::mat4 model = glm::mat4(1.0f);

    // compute MVP matrix
    glm::mat4 MVP = projection * view * model;

    //color and light models
    GLfloat MODEL_COLOR[4] = { 0.0f, 1.0f, 1.0f, 1.0f };
    GLfloat LIGHT_DIRECTION[3] = { 5.0f, 5.0f, 5.0f };

    // use the shader program
    glUseProgram(shaderProgram);

    // get the uniform locations
    GLint mvpLoc = glGetUniformLocation(shaderProgram, "MVP");
    GLuint viewID = glGetUniformLocation(shaderProgram, "V");
    GLuint lightDirID = glGetUniformLocation(shaderProgram, "lightDir");
    GLuint colorID = glGetUniformLocation(shaderProgram, "modelColor");

    // send to the shader
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(MVP));
    glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);
    glUniform4fv(colorID, 1, MODEL_COLOR);
    glUniform3fv(lightDirID, 1, LIGHT_DIRECTION);
    
    // bind VAO and draw triangles
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);

    glBindVertexArray(0);
    glUseProgram(0);
}

// function to handle user input (camera movement)
void handleUserInput(GLFWwindow* window, float& r, float& theta, float& phi) {
    // set speeds
    static const float moveSpeed = 0.0075f;
    static const float rotateSpeed = 0.5f;

    // handle up arrow
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        // ensure r is always greater than 0
        r = std::max(r - moveSpeed, 0.01f);
    }
    // handle down arrow
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        // add move speed to r (away from origin)
        r += moveSpeed;
    }

    // handle mouse input
    static double mouseLastX = -1, mouseLastY = -1;
    double xpos, ypos;

    // get current mouse pos
    glfwGetCursorPos(window, &xpos, &ypos);

    // if left click is pressed
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {

        // if cursor has moved
        if (mouseLastX >= 0 && mouseLastY >= 0) {
            // update spherical coordinates
            theta -= rotateSpeed * float(xpos - mouseLastX);
            phi += rotateSpeed * float(ypos - mouseLastY);
            // avoid flip
            phi = glm::clamp(phi, -89.0f, 89.0f);
        }
    }
    // set the last mouse position
    mouseLastX = xpos;
    mouseLastY = ypos;

    // update camera position and direction
    cameraPosition.x = r * sin(glm::radians(theta)) * cos(glm::radians(phi));
    cameraPosition.y = r * sin(glm::radians(phi));
    cameraPosition.z = r * cos(glm::radians(theta)) * cos(glm::radians(phi));
    cameraDirection = glm::normalize(-cameraPosition);

    // update the view matrix accordingly
    view = glm::lookAt(cameraPosition, cameraPosition + cameraDirection, up);
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    // MSAA
    glfwWindowHint(GLFW_SAMPLES, 4);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1000, 1000, "Exercise1", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        return -1;
    }

    // enable depth test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // define min and max for march
    float min = -5.0f;
    float max = 5.0f;

    // setup the shaders for drawing the cube edges
    GLuint VAO, VBO, EBO, axesVAO, axesVBO, axesEBO, shaderProgram;
    setupShadersForCube(min, max, VAO, VBO, EBO, axesVAO, axesVBO, axesEBO, shaderProgram);

    // setup shaders for drawing the marching volume
    GLuint VAOmarch, VBOvert, VBOnorm, shaderProgramMarch;
    float stepSize = 0.03f;
    float isoVal = 0.0f;
    // call marching cubes function to get vertices
    std::vector<float> vertices = marching_cubes(f1, isoVal, min, max, stepSize);
    // call compute normals function to get normals
    std::vector<float> normals = compute_normals(vertices);
    setupShadersForMarching(VAOmarch, VBOvert, VBOnorm, shaderProgramMarch, vertices, normals);

    //// write the ply
    //std::string fileName = "Function1";
    //writePLY(vertices, normals, fileName);
    
    // set clear color
    glClearColor(0.2f, 0.2f, 0.3f, 0.0f);

    // spherical coordinate system
    float r = 30.0f;
    float theta = 45.0f;
    float phi = 45.0f;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // handle input
        handleUserInput(window, r, theta, phi);

        // draw the cube edges
        drawCubeEdges(VAO, axesVAO, shaderProgram);

        // draw the marching volume
        drawMarch(VAOmarch, shaderProgramMarch, vertices);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    // cleanup buffers before exit
    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &axesVAO);
    glDeleteBuffers(1, &axesVBO);
    glDeleteBuffers(1, &axesEBO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBOvert);
    glDeleteBuffers(1, &VBOnorm);
    glDeleteVertexArrays(1, &VAOmarch);
    glDeleteProgram(shaderProgram);
    glDeleteProgram(shaderProgramMarch);

    glfwTerminate();
    return 0;
}
