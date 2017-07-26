#include <cmath>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <deque>

#define GLFW_INCLUDE_GLU
#define GLM_ENABLE_EXPERIMENTAL
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// ディレクトリの設定ファイル
#include "common.h"

static int WIN_WIDTH   = 1000;                       // ウィンドウの幅
static int WIN_HEIGHT  = 1000;                       // ウィンドウの高さ
static const char *WIN_TITLE = "Coriolis_Bowling";   // ウィンドウのタイトル

static const float PI = 4.0 * std::atan(1.0);
static float theta = 0.0f;

static const std::string RENDER_SHADER       = std::string(SHADER_DIRECTORY) + "render";
static const std::string TEXTURE_SHADER      = std::string(SHADER_DIRECTORY) + "texture";

// スタート画面用のファイル
static const std::string START_OBJFILE       = std::string(DATA_DIRECTORY) + "square.obj";
static const std::string START_TEXFILE       = std::string(DATA_DIRECTORY) + "start.png";

// 背景用のファイル
static const std::string BACK_OBJFILE        = std::string(DATA_DIRECTORY) + "square.obj";
static const std::string BACK_TEXFILE        = std::string(DATA_DIRECTORY) + "space.png";

// ボーリングのピンのファイル
static const std::string BOWLINGPIN_OBJFILE  = std::string(DATA_DIRECTORY) + "bowling_pin/bowling.obj";
static const std::string BOWLINGPIN1_TEXFILE = std::string(DATA_DIRECTORY) + "bowling_pin/bowling_pin.png";
static const std::string BOWLINGPIN2_TEXFILE = std::string(DATA_DIRECTORY) + "bowling_pin/bowling_pin2.png";

// 投げる人間のファイル
static const std::string PERSON_OBJFILE      = std::string(DATA_DIRECTORY) + "stickman.OBJ";
static const std::string PERSON_TEXFILE      = std::string(DATA_DIRECTORY) + "color6.png";

// 円盤のファイル
static const std::string CYLINDER_OBJFILE    = std::string(DATA_DIRECTORY) + "cylinder_thin.obj";
static const std::string CYLINDER_TEXFILE    = std::string(DATA_DIRECTORY) + "cylinder_thin.png";

// ボーリングのボールのファイル
static const std::string BOWLINGBALL_OBJFILE = std::string(DATA_DIRECTORY) + "Bowling_ball/Bowling_ball.obj";
static const std::vector<std::string> BOWLINGBALL_TEXFILES = { std::string(DATA_DIRECTORY) + "color0.png",
                                                               std::string(DATA_DIRECTORY) + "color1.png",
                                                               std::string(DATA_DIRECTORY) + "color2.png",
                                                               std::string(DATA_DIRECTORY) + "color3.png",
                                                               std::string(DATA_DIRECTORY) + "color4.png",
                                                               std::string(DATA_DIRECTORY) + "color5.png",
                                                               std::string(DATA_DIRECTORY) + "color6.png",
                                                               std::string(DATA_DIRECTORY) + "color7.png",
                                                               std::string(DATA_DIRECTORY) + "color8.png",
                                                               std::string(DATA_DIRECTORY) + "color9.png" };

// 矢印のファイル
static const std::string ARROW_OBJFILE = std::string(DATA_DIRECTORY) + "arrow/arrow.obj";
static const std::vector<std::string> ARROW_TEXFILES = { std::string(DATA_DIRECTORY) + "color0.png",
                                                         std::string(DATA_DIRECTORY) + "color1.png",
                                                         std::string(DATA_DIRECTORY) + "color2.png",
                                                         std::string(DATA_DIRECTORY) + "color3.png",
                                                         std::string(DATA_DIRECTORY) + "color4.png",
                                                         std::string(DATA_DIRECTORY) + "color5.png",
                                                         std::string(DATA_DIRECTORY) + "color6.png",
                                                         std::string(DATA_DIRECTORY) + "color7.png",
                                                         std::string(DATA_DIRECTORY) + "color8.png",
                                                         std::string(DATA_DIRECTORY) + "color9.png" };


static const glm::vec3 cameraPos = glm::vec3(0.0f, 100.0f, 0.0f);
static const glm::vec3 eyeTo     = glm::vec3(0.0f, 0.0f, 0.0f);
static const glm::vec3 upVec     = glm::vec3(0.0f, 0.0f, 1.0f);
static const glm::vec3 lightPos  = glm::vec3(0.0f, 1.0f, 0.0f);


struct Vertex {
    Vertex()
    : position(0.0f, 0.0f, 0.0f)
    , normal(0.0f, 0.0f, 0.0f)
    , texcoord(0.0f, 0.0f) {
    }
    
    Vertex(const glm::vec3 &pos, const glm::vec3 &norm, const glm::vec2 &uv)
    : position(pos)
    , normal(norm)
    , texcoord(uv) {
    }
    
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
};

struct Camera {
    glm::mat4 viewMat;
    glm::mat4 projMat;
};


struct RenderObject {
    GLuint programId;
    GLuint vaoId;
    GLuint vboId;
    GLuint iboId;
    GLuint textureId;
    int bufferSize;
    
    glm::mat4 modelMat;
    glm::vec3 ambiColor;
    glm::vec3 diffColor;
    glm::vec3 specColor;
    float shininess;
    
    void initialize() {
        programId = 0u;
        vaoId = 0u;
        vboId = 0u;
        iboId = 0u;
        textureId = 0u;
        bufferSize = 0;
        
        ambiColor = glm::vec3(0.0f, 0.0f, 0.0f);
        diffColor = glm::vec3(1.0f, 1.0f, 1.0f);
        specColor = glm::vec3(0.0f, 0.0f, 0.0f);
    }
    
    void buildShader(const std::string &basename) {
        const std::string vertShaderFile = basename + ".vert";
        const std::string fragShaderFile = basename + ".frag";
        
        // シェーダの用意
        GLuint vertShaderId = glCreateShader(GL_VERTEX_SHADER);
        GLuint fragShaderId = glCreateShader(GL_FRAGMENT_SHADER);
        
        // ファイルの読み込み (Vertex shader)
        std::ifstream vertFileInput(vertShaderFile.c_str(), std::ios::in);
        if (!vertFileInput.is_open()) {
            fprintf(stderr, "Failed to load vertex shader: %s\n", vertShaderFile.c_str());
            exit(1);
        }
        std::istreambuf_iterator<char> vertDataBegin(vertFileInput);
        std::istreambuf_iterator<char> vertDataEnd;
        const std::string vertFileData(vertDataBegin,vertDataEnd);
        const char *vertShaderCode = vertFileData.c_str();
        
        // ファイルの読み込み (Fragment shader)
        std::ifstream fragFileInput(fragShaderFile.c_str(), std::ios::in);
        if (!fragFileInput.is_open()) {
            fprintf(stderr, "Failed to load fragment shader: %s\n", fragShaderFile.c_str());
            exit(1);
        }
        std::istreambuf_iterator<char> fragDataBegin(fragFileInput);
        std::istreambuf_iterator<char> fragDataEnd;
        const std::string fragFileData(fragDataBegin,fragDataEnd);
        const char *fragShaderCode = fragFileData.c_str();
        
        // シェーダのコンパイル
        GLint compileStatus;
        glShaderSource(vertShaderId, 1, &vertShaderCode, NULL);
        glCompileShader(vertShaderId);
        glGetShaderiv(vertShaderId, GL_COMPILE_STATUS, &compileStatus);
        if (compileStatus == GL_FALSE) {
            fprintf(stderr, "Failed to compile vertex shader!\n");
            
            GLint logLength;
            glGetShaderiv(vertShaderId, GL_INFO_LOG_LENGTH, &logLength);
            if (logLength > 0) {
                GLsizei length;
                char *errmsg = new char[logLength + 1];
                glGetShaderInfoLog(vertShaderId, logLength, &length, errmsg);
                
                std::cerr << errmsg << std::endl;
                fprintf(stderr, "%s", vertShaderCode);
                
                delete[] errmsg;
            }
        }
        
        glShaderSource(fragShaderId, 1, &fragShaderCode, NULL);
        glCompileShader(fragShaderId);
        glGetShaderiv(fragShaderId, GL_COMPILE_STATUS, &compileStatus);
        if (compileStatus == GL_FALSE) {
            fprintf(stderr, "Failed to compile fragment shader!\n");
            
            GLint logLength;
            glGetShaderiv(fragShaderId, GL_INFO_LOG_LENGTH, &logLength);
            if (logLength > 0) {
                GLsizei length;
                char *errmsg = new char[logLength + 1];
                glGetShaderInfoLog(fragShaderId, logLength, &length, errmsg);
                
                std::cerr << errmsg << std::endl;
                fprintf(stderr, "%s", vertShaderCode);
                
                delete[] errmsg;
            }
        }
        
        // シェーダプログラムの用意
        programId = glCreateProgram();
        glAttachShader(programId, vertShaderId);
        glAttachShader(programId, fragShaderId);
        
        GLint linkState;
        glLinkProgram(programId);
        glGetProgramiv(programId, GL_LINK_STATUS, &linkState);
        if (linkState == GL_FALSE) {
            fprintf(stderr, "Failed to link shaders!\n");
            
            GLint logLength;
            glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &logLength);
            if (logLength > 0) {
                GLsizei length;
                char *errmsg = new char[logLength + 1];
                glGetProgramInfoLog(programId, logLength, &length, errmsg);
                
                std::cerr << errmsg << std::endl;
                delete[] errmsg;
            }
            
            exit(1);
        }
    }
    
    void loadOBJ(const std::string &filename) {
        // Load OBJ file.
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string err;
        bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename.c_str());
        if (!err.empty()) {
            std::cerr << "[WARNING] " << err << std::endl;
        }
        
        if (!success) {
            std::cerr << "Failed to load OBJ file: " << filename << std::endl;
            exit(1);
        }
        
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        for (int s = 0; s < shapes.size(); s++) {
            const tinyobj::shape_t &shape = shapes[s];
            for (int i = 0; i < shape.mesh.indices.size(); i++) {
                const tinyobj::index_t &index = shapes[s].mesh.indices[i];
                
                Vertex vertex;
                if (index.vertex_index >= 0) {
                    vertex.position = glm::vec3(
                                                attrib.vertices[index.vertex_index * 3 + 0],
                                                attrib.vertices[index.vertex_index * 3 + 1],
                                                attrib.vertices[index.vertex_index * 3 + 2]
                                                );
                }
                
                if (index.normal_index >= 0) {
                    vertex.normal = glm::vec3(
                                              attrib.normals[index.normal_index * 3 + 0],
                                              attrib.normals[index.normal_index * 3 + 1],
                                              attrib.normals[index.normal_index * 3 + 2]
                                              );
                }
                
                if (index.texcoord_index >= 0) {
                    vertex.texcoord = glm::vec2(
                                                attrib.texcoords[index.texcoord_index * 2 + 0],
                                                1.0f - attrib.texcoords[index.texcoord_index * 2 + 1]
                                                );
                }
                
                indices.push_back(vertices.size());
                vertices.push_back(vertex);
            }
        }
        
        // Prepare VAO.
        glGenVertexArrays(1, &vaoId);
        glBindVertexArray(vaoId);
        
        glGenBuffers(1, &vboId);
        glBindBuffer(GL_ARRAY_BUFFER, vboId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(),
                     vertices.data(), GL_DYNAMIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));
        
        glGenBuffers(1, &iboId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(),
                     indices.data(), GL_STATIC_DRAW);
        bufferSize = indices.size();
        
        glBindVertexArray(0);
    }
    
    void loadTexture(const std::string &filename) {
        int texWidth, texHeight, channels;
        unsigned char *bytes = stbi_load(filename.c_str(), &texWidth, &texHeight, &channels, STBI_rgb_alpha);
        if (!bytes) {
            fprintf(stderr, "Failed to load image file: %s\n", filename.c_str());
            exit(1);
        }
        
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        
        glBindTexture(GL_TEXTURE_2D, 0);
        
        stbi_image_free(bytes);
    }
    
    void draw(const Camera &camera) {
        glUseProgram(programId);
        
        GLuint location;
        location = glGetUniformLocation(programId, "u_ambColor");
        glUniform3fv(location, 1, glm::value_ptr(ambiColor));
        location = glGetUniformLocation(programId, "u_diffColor");
        glUniform3fv(location, 1, glm::value_ptr(diffColor));
        location = glGetUniformLocation(programId, "u_specColor");
        glUniform3fv(location, 1, glm::value_ptr(specColor));
        location = glGetUniformLocation(programId, "u_shininess");
        glUniform1f(location, shininess);
        
        glm::mat4 mvMat, mvpMat, normMat;
        mvMat = camera.viewMat * modelMat;
        mvpMat = camera.projMat * mvMat;
        normMat = glm::transpose(glm::inverse(mvMat));
        
        location = glGetUniformLocation(programId, "u_lightPos");
        glUniform3fv(location, 1, glm::value_ptr(lightPos));
        location = glGetUniformLocation(programId, "u_lightMat");
        glUniformMatrix4fv(location, 1, false, glm::value_ptr(camera.viewMat));
        location = glGetUniformLocation(programId, "u_mvMat");
        glUniformMatrix4fv(location, 1, false, glm::value_ptr(mvMat));
        location = glGetUniformLocation(programId, "u_mvpMat");
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mvpMat));
        location = glGetUniformLocation(programId, "u_normMat");
        glUniformMatrix4fv(location, 1, false, glm::value_ptr(normMat));
        
        if (textureId != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureId);
            location = glGetUniformLocation(programId, "u_isTextured");
            glUniform1i(location, 1);
            location = glGetUniformLocation(programId, "u_texture");
            glUniform1i(location, 0);
        } else {
            location = glGetUniformLocation(programId, "u_isTextured");
            glUniform1i(location, 0);
        }
        
        glBindVertexArray(vaoId);
        glDrawElements(GL_TRIANGLES, bufferSize, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        
        glUseProgram(0);
    }
};


RenderObject startDisp;
RenderObject background;
RenderObject cylinder;
RenderObject bowlingPin1;
RenderObject bowlingPin2;
RenderObject bowlingBalls[10];
RenderObject person;
RenderObject arrow;

Camera camera1;
Camera camera2;

std::vector<float> ballRun;
std::vector<float> ballSpeedY;
std::vector<float> ballPosY;
std::deque<glm::vec3> ballPos;
std::deque<glm::vec3> startPos;
std::deque<glm::vec3> goalPos;
std::deque<glm::vec4> phi;

float arrowAngle = 0.0f;
float arrowAngleSpeed = 0.015f;
float arrowColor = 5.0f;
float ballAngle;
float ballSpeed[10] = {0.0005f, 0.001f, 0.0015f, 0.002f, 0.003f, 0.006f, 0.015f, 0.03f, 0.065f, 0.1f};
float g = 0.0005f;

bool throwing = false;
bool hit = false;

enum {
    GAME_MODE_START,
    GAME_MODE_PLAY,
    GAME_MODE_CLEAR
};

enum {
    FIRST_PERSON,
    BIRD_EYE,
};

int gameMode = GAME_MODE_START;
int cameraMode = FIRST_PERSON;
int modeselect = -1;
int arrowColorIndex = 5;


void initializeGL() {
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    startDisp.initialize();
    startDisp.loadOBJ(START_OBJFILE);
    startDisp.buildShader(TEXTURE_SHADER);
    startDisp.loadTexture(START_TEXFILE);
    
    background.initialize();
    background.loadOBJ(BACK_OBJFILE);
    background.buildShader(TEXTURE_SHADER);
    background.loadTexture(BACK_TEXFILE);
    
    cylinder.initialize();
    cylinder.loadOBJ(CYLINDER_OBJFILE);
    cylinder.buildShader(RENDER_SHADER);
    cylinder.loadTexture(CYLINDER_TEXFILE);
    cylinder.modelMat = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
    
    bowlingPin1.initialize();
    bowlingPin1.loadOBJ(BOWLINGPIN_OBJFILE);
    bowlingPin1.buildShader(RENDER_SHADER);
    bowlingPin1.loadTexture(BOWLINGPIN1_TEXFILE);
    bowlingPin1.modelMat = glm::translate(glm::vec3(0.0f, 0.1f, -0.9f)) * glm::scale(glm::vec3(0.015f, 0.015f, 0.015f));
    
    bowlingPin2.initialize();
    bowlingPin2.loadOBJ(BOWLINGPIN_OBJFILE);
    bowlingPin2.buildShader(RENDER_SHADER);
    bowlingPin2.loadTexture(BOWLINGPIN2_TEXFILE);
    bowlingPin2.modelMat = glm::translate(glm::vec3(0.0f, 0.1f, -0.9f)) * glm::scale(glm::vec3(0.015f, 0.015f, 0.015f));
    
    for (int i=0; i<10; i++){
        bowlingBalls[i].initialize();
        bowlingBalls[i].loadOBJ(BOWLINGBALL_OBJFILE);
        bowlingBalls[i].buildShader(RENDER_SHADER);
        bowlingBalls[i].loadTexture(BOWLINGBALL_TEXFILES[i]);
        bowlingBalls[i].modelMat = glm::translate(glm::vec3(0.0f, 0.15f, 0.75f)) * glm::scale(glm::vec3(0.04f, 0.04f, 0.04f));
    }
    
    person.loadOBJ(PERSON_OBJFILE);
    person.buildShader(RENDER_SHADER);
    person.loadTexture(PERSON_TEXFILE);
    person.modelMat = glm::translate(glm::vec3(0.0f, 0.2f, 0.93f)) * glm::scale(glm::vec3(0.002f, 0.002f, 0.002f));
    
    arrow.initialize();
    arrow.loadOBJ(ARROW_OBJFILE);
    arrow.buildShader(RENDER_SHADER);
    arrow.loadTexture(ARROW_TEXFILES[5]);
    arrow.modelMat = glm::translate(glm::vec3(0.0f, 0.1f, 0.90f)) * glm::scale(glm::vec3(0.04f, 0.04f, 0.04f));
    
    camera1.projMat = glm::perspective(45.0f, (float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 1000.0f);
    camera1.viewMat = glm::lookAt(glm::vec3(0.0f, 1.0f, 1.45f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    
    camera2.projMat = glm::perspective(45.0f, (float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 1000.0f);
    camera2.viewMat = glm::lookAt(glm::vec3(1.5f, 1.5f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
}


void paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    switch (gameMode) {
        case GAME_MODE_START:
        {
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            startDisp.draw(camera1);
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
        }
        break;
    
        case GAME_MODE_PLAY:
        {
            if (modeselect == -1){
                glDisable(GL_DEPTH_TEST);
                background.draw(camera1);
                glEnable(GL_DEPTH_TEST);
                
                cylinder.draw(camera1);
                person.draw(camera1);
                arrow.draw(camera1);
                
                // ボールがピンに当たったら赤くする
                if(hit==false){
                    bowlingPin1.draw(camera1);
                }
                else{
                    bowlingPin2.draw(camera1);
                }
                
                // ボールの色を変化させる
                for(int i=0; i<startPos.size(); i++){
                    int ballColorIndex = i / 7 ;
                    bowlingBalls[ballColorIndex].modelMat = glm::translate(ballPos[i]) * glm::scale(glm::vec3(0.04f, 0.04f, 0.04f)) * glm::rotate(theta, glm::vec3(0.0f, 1.0f, 0.0f))* glm::rotate(phi[i][0], glm::vec3(phi[i][1], phi[i][2], phi[i][3]));
                    bowlingBalls[ballColorIndex].draw(camera1);
                }
            }
            else{
                glDisable(GL_DEPTH_TEST);
                background.draw(camera2);
                glEnable(GL_DEPTH_TEST);

                cylinder.draw(camera2);
                person.draw(camera2);
                arrow.draw(camera2);
                
                // ボールがピンに当たったら赤くする
                if(hit==false){
                    bowlingPin1.draw(camera2);
                }
                else{
                    bowlingPin2.draw(camera2);
                }

                // ボールの色を変化させる
                for(int i=0; i<startPos.size(); i++){
                    int ballColorIndex = i / 7;
                    bowlingBalls[ballColorIndex].modelMat = glm::translate(ballPos[i]) * glm::scale(glm::vec3(0.04f, 0.04f, 0.04f)) * glm::rotate(theta, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(phi[i][0], glm::vec3(phi[i][1], phi[i][2], phi[i][3]));
                    bowlingBalls[ballColorIndex].draw(camera2);
                }
            }
        }
    }
}


void resizeGL(GLFWwindow *window, int width, int height) {
    // ユーザ管理のウィンドウサイズを変更
    WIN_WIDTH = width;
    WIN_HEIGHT = height;
    
    // GLFW管理のウィンドウサイズを変更
    glfwSetWindowSize(window, WIN_WIDTH, WIN_HEIGHT);
    
    // 実際のウィンドウサイズ (ピクセル数) を取得
    int renderBufferWidth, renderBufferHeight;
    glfwGetFramebufferSize(window, &renderBufferWidth, &renderBufferHeight);
    
    // ビューポート変換の更新
    glViewport(0, 0, renderBufferWidth, renderBufferHeight);
}


// アニメーションのためのアップデート
void animate() {
    if (gameMode == GAME_MODE_PLAY) {
        theta += 2.0f * PI / 360.0f;  // 10分の1回転
        camera1.viewMat = glm::lookAt(glm::vec3(1.45*sin(theta), 1.0f, 1.45*cos(theta)), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        
        bowlingPin1.modelMat = glm::translate(glm::vec3(-0.9*sin(theta), 0.1f, -0.9*cos(theta))) * glm::scale(glm::vec3(0.015, 0.015, 0.015));
        bowlingPin2.modelMat = glm::translate(glm::vec3(-0.9*sin(theta), 0.1f, -0.9*cos(theta))) * glm::scale(glm::vec3(0.015, 0.015, 0.015));
        
        cylinder.modelMat = glm::rotate(theta, glm::vec3(0.0f, 1.0f, 0.0f));
        
        person.modelMat =  glm::translate(glm::vec3(0.93*sin(theta+0.07) , 0.2f, 0.93*cos(theta+0.07))) * glm::scale(glm::vec3(0.003f, 0.003f, 0.003f)) * glm::rotate(theta+PI, glm::vec3(0.0f, 1.0f, 0.0f));
        
        arrow.modelMat =  glm::translate(glm::vec3(0.75f*sin(theta), 0.1f, 0.75f*cos(theta))) * glm::scale(glm::vec3(0.04f, 0.04f, 0.04f)) * glm::rotate(theta + PI/2 + arrowAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    
        if(throwing){
            for(int i=0; i<startPos.size(); i++){
                ballRun[i] += ballSpeed[arrowColorIndex];
                phi[i][0] -= 2.0f * PI / 90.0f;
                
                ballPos[i] = ballRun[i] * goalPos[i] + (1-ballRun[i]) * startPos[i];
                
                if(ballPos[i][0]*ballPos[i][0] + ballPos[i][2]*ballPos[i][2] >= 1.0f){
                    ballSpeedY[i] += g;
                    ballPosY[i] -= ballSpeedY[i];
                    ballPos[i][1] = ballPosY[i];
                }
            }
            if(ballPos[0][1] <= -5.0f){
                ballRun.erase(ballRun.begin());
                ballSpeedY.erase(ballSpeedY.begin());
                ballPosY.erase(ballPosY.begin());
                ballPos.erase(ballPos.begin());
                startPos.erase(startPos.begin());
                goalPos.erase(goalPos.begin());
                phi.erase(phi.begin());
            }
  
            // 当たり判定
            hit = false;
            for(int i=0; i<startPos.size(); i++){
                glm::vec3 diff = ballPos[i] - glm::vec3(-0.9*sin(theta), 0.1f, -0.9*cos(theta));
                if (glm::length(diff) <= 0.08f){
                    hit = true;
                    continue;
                }
            }
            
        }
        else {
            bowlingBalls[0].modelMat = glm::translate(glm::vec3(0.75*sin(theta) , 0.1f, 0.75*cos(theta))) * glm::scale(glm::vec3(0.04f, 0.04f, 0.04f)) * glm::rotate(theta, glm::vec3(0.0f, 1.0f, 0.0f));
        }
    }
}


void initArrow(int iarrowColorIndex){
    arrow.initialize();
    arrow.loadOBJ(ARROW_OBJFILE);
    arrow.buildShader(RENDER_SHADER);
    arrow.loadTexture(ARROW_TEXFILES[iarrowColorIndex]);
    arrow.modelMat =  glm::translate(glm::vec3(0.75f*sin(theta), 0.1f, 0.75f*cos(theta))) * glm::scale(glm::vec3(0.04f, 0.04f, 0.04f)) * glm::rotate(theta + PI/2 + arrowAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    
}


void keyboard(GLFWwindow *window) {
    int state;
    
    if (gameMode == GAME_MODE_PLAY) {
        // left
        state = glfwGetKey(window, GLFW_KEY_LEFT);
        if (state == GLFW_PRESS || state == GLFW_REPEAT) {
            arrowAngle += arrowAngleSpeed;
            arrowAngle = std::min(arrowAngle, +1.5f);
        }
        
        // right
        state = glfwGetKey(window, GLFW_KEY_RIGHT);
        if (state == GLFW_PRESS || state == GLFW_REPEAT) {
            arrowAngle -= arrowAngleSpeed;
            arrowAngle = std::max(arrowAngle, -1.5f);
        }
        
        // up
        state = glfwGetKey(window, GLFW_KEY_UP);
        if (state == GLFW_PRESS) {
            arrowColor += 0.2f;
            arrowColorIndex = int(arrowColor);
            arrowColorIndex = std::min(arrowColorIndex, 9);
            arrowColorIndex = std::max(arrowColorIndex, 0);
            initArrow(arrowColorIndex);
        }
        
        // down
        state = glfwGetKey(window, GLFW_KEY_DOWN);
        if (state == GLFW_PRESS) {
            arrowColor -= 0.2f;
            arrowColorIndex = int(arrowColor);
            arrowColorIndex = std::min(arrowColorIndex, 9);
            arrowColorIndex = std::max(arrowColorIndex, 0);
            initArrow(arrowColorIndex);
        }
    }
}


void keyboardCallback(GLFWwindow *window, int key, int scanmode, int action, int mods) {
    if (gameMode == GAME_MODE_PLAY) {
        // Space --- viewmode change
        if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
            modeselect *= -1;
        }
        
        // Enter --- throwing
        if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
            throwing = true;
            ballAngle = arrowAngle;
            if (startPos.size() < 70){
                startPos.push_back(glm::vec3(0.75f*sin(theta), 0.15f, 0.75f*cos(theta)));
                goalPos.push_back(glm::vec3(0.75f*sin(theta-(PI-2*ballAngle)), 0.15f, 0.75f*cos(theta-(PI-2*ballAngle))));
                ballRun.push_back(0.0f);
                ballSpeedY.push_back(0.0f);
                ballPosY.push_back(0.15f);
                ballPos.push_back(glm::vec3(0.75f*sin(theta), 0.15f, 0.75f*cos(theta)));
                phi.push_back(glm::vec4(0.0f, sin(theta)/2, 0.0f, cos(theta)/2));
            }
        }
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            gameMode = GAME_MODE_START;
        }
    }
    else {
        if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
            if (gameMode == GAME_MODE_START) {
                gameMode = GAME_MODE_PLAY;
                arrowAngle = 0.0f;
            }
        }
    }
}


int main(int argc, char **argv) {
    // OpenGLを初期化する
    if (glfwInit() == GL_FALSE) {
        fprintf(stderr, "Initialization failed!\n");
        return 1;
    }
    
    // OpenGLのバージョン設定 (Macの場合には必ず必要)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Windowの作成
    GLFWwindow *window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, WIN_TITLE,
                                          NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Window creation failed!");
        glfwTerminate();
        return 1;
    }
    
    // OpenGLの描画対象にWindowを追加
    glfwMakeContextCurrent(window);
    
    // GLEWを初期化する (glfwMakeContextCurrentの後でないといけない)
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "GLEW initialization failed!\n");
        return 1;
    }
    
    // ウィンドウのリサイズを扱う関数の登録
    glfwSetWindowSizeCallback(window, resizeGL);
    
    // キーボードコールバック関数の登録
    glfwSetKeyCallback(window, keyboardCallback);
    
    initializeGL();

    // メインループ
    while (glfwWindowShouldClose(window) == GL_FALSE) {
        
        // 描画
        paintGL();
        
        // アニメーション
        animate();
        
        keyboard(window);
        
        // 描画用バッファの切り替え
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
