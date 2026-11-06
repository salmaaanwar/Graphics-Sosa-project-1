// Exercise production rendering and mouse callbacks in a hidden window.
#include "Renderer.h"
static glm::vec2 testMouse(-99);
static bool testMouseToWorld(GLFWwindow*, const glm::mat4&, glm::vec2& world) { world = testMouse; return true; }
#define mouseToWorld testMouseToWorld
#define main gameApplicationMain
#include "../main.cpp"
#undef main
#undef mouseToWorld
#include <fstream>
#include <cstdlib>

static void check(bool value, const char* message) {
    if (!value) { std::cerr << "FAILED: " << message << '\n'; std::exit(1); }
}
int main(int argc, char** argv) {
    check(glfwInit(),"GLFW initialization");
    glfwWindowHint(GLFW_VISIBLE,GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,0);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    auto* window = glfwCreateWindow(800,600,"Audio controls test",nullptr,nullptr);
    check(window != nullptr,"hidden window");
    glfwMakeContextCurrent(window);
    check(gladLoadGL(glfwGetProcAddress) != 0,"OpenGL entry points");
    auto vs = compileShader(GL_VERTEX_SHADER,vertexShaderSrc), fs = compileShader(GL_FRAGMENT_SHADER,fragmentShaderSrc);
    auto program = glCreateProgram(); glAttachShader(program,vs); glAttachShader(program,fs); glLinkProgram(program);
    GLint linked; glGetProgramiv(program,GL_LINK_STATUS,&linked); check(linked != 0,"shader link");
    glUseProgram(program);
    check(loadKitchenBackground(),"image assets load");
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    float vertices[] = {-0.5f,-0.5f,0, .5f,-.5f,0, .5f,.5f,0, -.5f,-.5f,0, .5f,.5f,0, -.5f,.5f,0};
    GLuint vao,vbo; glGenVertexArrays(1,&vao); glBindVertexArray(vao);
    glGenBuffers(1,&vbo); glBindBuffer(GL_ARRAY_BUFFER,vbo); glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),nullptr); glEnableVertexAttribArray(0);
    auto t = glGetUniformLocation(program,"u_Transform"), c = glGetUniformLocation(program,"uColor"), p = glGetUniformLocation(program,"u_Projection");
    std::filesystem::path output = argc > 1 ? argv[1] : "audio-captures";
    std::filesystem::create_directories(output);
    initializeAudio({},AudioMode::Recording);
    for (auto size : {glm::ivec2(800,600),glm::ivec2(1280,720),glm::ivec2(600,800),glm::ivec2(320,240),
                      glm::ivec2(1920,1080),glm::ivec2(2560,1080),glm::ivec2(800,800)}) {
        glfwSetWindowSize(window,size.x,size.y); glfwPollEvents();
        int width,height; glfwGetFramebufferSize(window,&width,&height); framebufferSizeCallback(window,width,height);
        auto render = [&](const std::string& name) {
            glClearColor(.1f,.1f,.1f,1); glClear(GL_COLOR_BUFFER_BIT);
            glUniformMatrix4fv(p,1,GL_FALSE,glm::value_ptr(projection));
            drawKitchenBackground(t,c,projection);
            if (getGameState() == STATE_MAIN_MENU) drawMainMenu(t,c,0,0,0,-99,-99);
            else drawPauseMenu(t,c,-99,-99);
            glFinish(); check(glGetError() == GL_NO_ERROR,"audio controls render without errors");
            std::vector<unsigned char> pixels(width*height*3); glPixelStorei(GL_PACK_ALIGNMENT,1);
            glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,pixels.data());
            auto path = output/(std::to_string(size.x)+"x"+std::to_string(size.y)+"-"+name+".ppm");
            std::ofstream file(path,std::ios::binary); file << "P6\n" << width << ' ' << height << "\n255\n";
            for (int row=height-1;row>=0;--row) file.write(reinterpret_cast<const char*>(pixels.data()+row*width*3),width*3);
            return pixels;
        };
        returnToMainMenu(); setAudioVolume(AudioBus::Music,.35f); setAudioVolume(AudioBus::Effects,.75f);
        auto normal = render("menu");
        keyCallback(window,GLFW_KEY_TAB,0,GLFW_PRESS,0);
        check(normal != render("focus"),"keyboard focus visible");
        keyCallback(window,GLFW_KEY_RIGHT,0,GLFW_PRESS,0);
        check(std::fabs(getAudioVolume(AudioBus::Music)-.40f)<.001f,"real keyboard callback adjusts slider");
        testMouse = glm::vec2(-.15f,-.65f);
        mouseButtonCallback(window,GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS,0);
        updateAudioControlsMouse(.37f,-.65f,true);
        mouseButtonCallback(window,GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE,0);
        check(getAudioVolume(AudioBus::Music) == 1 && getGameState() == STATE_MAIN_MENU,"real mouse callback consumes drag");
        restartGame(); pauseGame();
        testMouse = glm::vec2(.665f,-.81f);
        mouseButtonCallback(window,GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS,0);
        mouseButtonCallback(window,GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE,0);
        check(getAudioVolume(AudioBus::Effects)==0 && getGameState()==STATE_PAUSED,"pause effects mute does not activate menu buttons");
        render("paused-muted");
    }
    shutdownAudio(); releaseKitchenBackground();
    glDeleteBuffers(1,&vbo); glDeleteVertexArrays(1,&vao); glDeleteShader(vs); glDeleteShader(fs); glDeleteProgram(program);
    glfwDestroyWindow(window); glfwTerminate();
    std::cout << "Audio controls passed production callbacks and rendering at seven window sizes.\n";
}
