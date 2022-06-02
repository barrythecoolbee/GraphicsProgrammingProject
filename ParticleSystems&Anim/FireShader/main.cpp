#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <vector>

#include "shader.h"
#include "camera.h"
#include "model.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// glfw and input functions
// ------------------------
void processInput(GLFWwindow* window);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_input_callback(GLFWwindow* window, int button, int other, int action, int mods);
void cursor_input_callback(GLFWwindow* window, double posX, double posY);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// screen settings
// ---------------
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// global variables used for rendering
// -----------------------------------
Shader* shader;
Shader* fireShader;
Camera camera(glm::vec3(0.0f, 1.6f, 5.0f));

// global variables used for control
// ---------------------------------
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
float deltaTime;
bool isPaused = false; // stop camera movement when GUI is open


//-----------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------CONFIG------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------
struct Config
{
    GLuint particleCount = 4000;

    GLuint initVel;

    GLuint feedback[2];
    GLuint posBuf[2];
	GLuint velBuf[2];
    GLuint startTime[2];
    GLuint particleArray[2];

    GLuint updateParticles;
    GLuint drawBuf = 1;
    GLuint renderParticles;

    float Time = 0.0f;
    float H = 0.0f;
    float ParticleLifeTime = 4.0f;
    glm::vec3 acceleration = glm::vec3(0.0f, 0.1f, 0.0f);
    float rate = 0.001f;
    float particleSize = 50.0f;

    const char* textureName = "fire/fire.png";
	
} config;


void drawObjects();
void initParticlesBuffer();
void drawGui();

//-----------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------MAIN------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------
int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Fire", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_input_callback);
    glfwSetKeyCallback(window, key_input_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
	
    fireShader = new Shader("shaders/fire.vert", "shaders/fire.frag");
    shader = fireShader;

    // get subroutine indices
    config.renderParticles = glGetSubroutineIndex(shader->ID, GL_VERTEX_SHADER, "render");
    config.updateParticles = glGetSubroutineIndex(shader->ID, GL_VERTEX_SHADER, "update");
	
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // Set the point size
    glPointSize(config.particleSize); //size of texture/particle
    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
    glActiveTexture(GL_TEXTURE0);
    loadTexture(config.textureName);
	
    initParticlesBuffer();

    // Dear IMGUI init
    // ---------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 440 core");
	
    // render loop
    // -----------   
    while (!glfwWindowShouldClose(window))
    {
        static float lastFrame = 0.0f;
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        config.Time = currentFrame;
		config.H = deltaTime;

        processInput(window);
        		
        shader->use();
        
        drawObjects();

        if (isPaused) {
            drawGui();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    // -------
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
	
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

//-----------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------FOUNTAIN----------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------
float randFloat() {
    return ( (float) rand() / RAND_MAX);
}

void initParticlesBuffer() {
	
	// Generate the buffers
	glGenBuffers(2, config.posBuf);
	glGenBuffers(2, config.velBuf);
	glGenBuffers(2, config.startTime);
	glGenBuffers(1, &config.initVel);

	// Initialize the buffers
	int size = config.particleCount * 3 * sizeof(float);
	glBindBuffer(GL_ARRAY_BUFFER, config.posBuf[0]); //buffer A
	glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_COPY);
	glBindBuffer(GL_ARRAY_BUFFER, config.posBuf[1]); //buffer B
	glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_COPY);
	glBindBuffer(GL_ARRAY_BUFFER, config.velBuf[0]);
	glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_COPY);
	glBindBuffer(GL_ARRAY_BUFFER, config.velBuf[1]);
	glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_COPY);
	glBindBuffer(GL_ARRAY_BUFFER, config.initVel);
	glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, config.startTime[0]);
	glBufferData(GL_ARRAY_BUFFER, config.particleCount * sizeof(float), NULL, GL_DYNAMIC_COPY);
	glBindBuffer(GL_ARRAY_BUFFER, config.startTime[1]);
	glBufferData(GL_ARRAY_BUFFER, config.particleCount * sizeof(float), NULL, GL_DYNAMIC_COPY);
	
	//Instead of using the origin for all particles, use a random x location
	GLfloat *data = new GLfloat[config.particleCount * 3];
	for (int i = 0; i < config.particleCount * 3; i+=3) {
        data[i] = glm::mix(-2.0f, 2.0f, randFloat());
        data[i + 1] = 0.0f;
        data[i + 2] = 0.0f;
	}
	glBindBuffer(GL_ARRAY_BUFFER, config.posBuf[0]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	
    //Fill the first velocity buffer with random numbers
    glm::vec3 v(0.0f);
    float velocity, theta, phi; 
	
	//x and z components are zero and the y component contains a random speed
    for (int i = 0; i < config.particleCount; i++) {

        data[3 * i] = 0.0f;
        data[3 * i + 1] = glm::mix(0.1f, 0.5f, randFloat());
        data[3 * i + 2] = 0.0f;
    }
	
	glBindBuffer(GL_ARRAY_BUFFER, config.velBuf[0]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);	
	
    glBindBuffer(GL_ARRAY_BUFFER, config.initVel);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);

	//fill the first start time buffer
    delete[] data;
	data = new GLfloat[config.particleCount];
	float time = 0.0f, rate = config.rate;
	
	for(int i = 0; i < config.particleCount; i++) {
		data[i] = time;
		time += rate;
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, config.startTime[0]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, config.particleCount * sizeof(float), data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	delete[] data;

	//create vertex arrays for each set of buffers
	glGenVertexArrays(2, config.particleArray);
	
	//Set up particle array 0
	glBindVertexArray(config.particleArray[0]);
	glBindBuffer(GL_ARRAY_BUFFER, config.posBuf[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, config.velBuf[0]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, config.startTime[0]);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(2);
	
    glBindBuffer(GL_ARRAY_BUFFER, config.initVel);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(3);

	//Set up particle array 1
	glBindVertexArray(config.particleArray[1]);
	glBindBuffer(GL_ARRAY_BUFFER, config.posBuf[1]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, config.velBuf[1]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);
	
	glBindBuffer(GL_ARRAY_BUFFER, config.startTime[1]);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(2);
	
	glBindBuffer(GL_ARRAY_BUFFER, config.initVel);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(3);
	
	glBindVertexArray(0);

	//Setup the feedback objects
    glGenTransformFeedbacks(2, config.feedback);

    //Transform feedback 0
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, config.feedback[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, config.posBuf[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, config.velBuf[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, config.startTime[0]);

    //Transform feedback 1
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, config.feedback[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, config.posBuf[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, config.velBuf[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, config.startTime[1]);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
}

void drawObjects()
{

    //Select the subroutine for particle updating
    glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &config.updateParticles);

    shader->setSampler2D("ParticleTexture", 0);
    shader->setFloat("ParticleLifetime", config.ParticleLifeTime);
    shader->setVec3("Accel", config.acceleration);
    shader->setFloat("Time", config.Time);
    shader->setFloat("H", config.H);
	
	//Disable rendering
	glEnable(GL_RASTERIZER_DISCARD);

	//Bind the feedback obj. for the buffers to be drawn
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, config.feedback[config.drawBuf]);

	//Draw points from input buffer with transform feedback
    glBeginTransformFeedback(GL_POINTS);
    glBindVertexArray(config.particleArray[1 - config.drawBuf]);
    glDrawArrays(GL_POINTS, 0, config.particleCount);
    glEndTransformFeedback();

	//Enable rendering
	glDisable(GL_RASTERIZER_DISCARD);
	
	//Select the subroutine for particle rendering
	glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &config.renderParticles);
	glClear(GL_COLOR_BUFFER_BIT);

	// camera parameters
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);

    glm::mat4 mv = view * model;
    shader->setMat4("MVP", projection * mv);

	//Draw the sprites from the feedback buffer
	glBindVertexArray(config.particleArray[config.drawBuf]);
	glDrawTransformFeedback(GL_POINTS, config.feedback[config.drawBuf]);

	//Swap buffers
	config.drawBuf = 1 - config.drawBuf;
}

//-----------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------GUI-------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------
void drawGui() {

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        ImGui::Begin("Settings");

        ImGui::Text("Fire: ");
        ImGui::SliderFloat("Particle Lifetime", &config.ParticleLifeTime, 2.0f, 4.0f);
        ImGui::SliderFloat("AccelerationY", (float*)&config.acceleration.y, -2.0f, 2.0f);
        ImGui::SliderFloat("AccelerationZ", (float*)&config.acceleration.z, -2.0f, 2.0f);
        ImGui::Separator();

        
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

//-----------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------INPUT------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (isPaused)
        return;

    // movement commands
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void cursor_input_callback(GLFWwindow* window, double posX, double posY){

    // camera rotation
    static bool firstMouse = true;
    if (firstMouse)
    {
        lastX = (float)posX;
        lastY = (float)posY;
        firstMouse = false;
    }

    float xoffset = (float)posX - lastX;
    float yoffset = lastY - (float)posY; // reversed since y-coordinates go from bottom to top

    lastX = (float)posX;
    lastY = (float)posY;

    if (isPaused)
        return;

    // we use the handy camera class from LearnOpenGL to handle our camera
    camera.ProcessMouseMovement(xoffset, yoffset);
}


void key_input_callback(GLFWwindow* window, int button, int other, int action, int mods){
    // controls pause mode
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
        isPaused = !isPaused;
        glfwSetInputMode(window, GLFW_CURSOR, isPaused ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }

}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll((float)yoffset);
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}