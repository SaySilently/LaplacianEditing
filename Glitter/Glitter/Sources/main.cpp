#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include "shader_m.h"
#include "camera.h"
#include "model.h"
#include "sketch.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>

//void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void process_mouse(GLFWwindow *window, Model& our_model, Sketch& circle);


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

// camera
Camera camera(glm::vec3(0.0f, 1.6f, 3.0f));
glm::vec3 mouse_first_position;
glm::vec3 mouse_second_position;


bool c_press = false;
bool c_release = false;
bool draw_circle = false;
bool space_press = false;
bool space_release = false;
bool select_vertices = false;
bool deform = false;
bool mouse_leftbutton_press = false;

// timing
//float deltaTime = 0.0f;
//float lastFrame = 0.0f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_DECORATED, GL_FALSE);
    
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LaplacianEditing", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
  //  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);

    // build and compile shaders
    // -------------------------
    Shader ourShader("/Users/xuhao/LaplacianEditing/Glitter/Glitter/Shaders/model.vs", "/Users/xuhao/LaplacianEditing/Glitter/Glitter/Shaders/model.fs");
    Shader sketch_shader("/Users/xuhao/LaplacianEditing/Glitter/Glitter/Shaders/sketch.vs", "/Users/xuhao/LaplacianEditing/Glitter/Glitter/Shaders/sketch.fs");
    
    // load models
    // -----------
    Model ourModel("/Users/xuhao/LaplacianEditing/Glitter/Glitter/knight.obj");

  
    Sketch circle;

    
    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(1.00f, 1.00f, 1.00f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        process_mouse(window, ourModel, circle);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.1f, 1.1f, 1.1f));    // it's a bit too big for our scene, so scale it down
        
        
        sketch_shader.use();
               
        circle.draw_circle_points();
        ourModel.circle_vertices_ndc = circle.circle_stroke_ndc;
        if(select_vertices){
            ourModel.select_vertices(model, view, projection, glm::vec3(0.7, 0.7, 0.7));
            select_vertices = false;
            
        }
        if(deform){
            ourModel.deform(model, view, projection);
            deform = false;
        }
      
        circle.circle_stroke_ndc.clear();

        ourShader.use();
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);
        

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    
    
    
    if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS){
        c_press = true;
    }
    if(c_press){
        if(glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE){
            c_release = true;
            c_press = false;
            draw_circle = !draw_circle;
        }
    }
    
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
        space_press = true;
    }
    if(space_press){
        if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE){
            space_release = true;
            space_press = false;
            select_vertices = true;
        }
    }
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        camera.ProcessKeyboard(FORWARD, 0.1);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        camera.ProcessKeyboard(BACKWARD, 0.1);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        camera.ProcessKeyboard(LEFT, 0.1);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        camera.ProcessKeyboard(RIGHT, 0.1);
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
           camera.ProcessMouseMovement(0.0, 10.0);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
         camera.ProcessMouseMovement(0.0, -10.0);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
        camera.ProcessMouseMovement(-10.0, 0.0);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
       camera.ProcessMouseMovement(10.0, 0.0);
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


void process_mouse(GLFWwindow *window, Model& our_model, Sketch& circle)
{

    int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
  
    if(draw_circle){
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        xpos = (xpos / 400.0) - 1.0;
        ypos  = (-ypos / 400.0) + 1.0;
        
        circle.circle_x = xpos;
        circle.circle_y = ypos;
        
        circle.compute_circle_points(0.1, 0.001);
    }
    if ((state == GLFW_RELEASE) && (true == mouse_leftbutton_press)){
        mouse_leftbutton_press = false;
        deform = true;
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        xpos = (xpos / 400.0) - 1.0;
        ypos  = (-ypos / 400.0) + 1.0;
        
        glm::vec3 a_ndc;
        a_ndc.x = xpos;
        a_ndc.y = ypos;
        a_ndc.z = 0.0f;
        
        mouse_second_position = a_ndc;
        our_model.mouse_press_movement = mouse_second_position - mouse_first_position;
      //  cout<<our_model.mouse_press_movement.x<<" "<<our_model.mouse_press_movement.y<<endl;

          
    }
    else if((state == GLFW_PRESS) && (false == mouse_leftbutton_press)){
       
        mouse_leftbutton_press = true;
           
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        xpos = (xpos / 400.0) - 1.0;
        ypos  = (-ypos / 400.0) + 1.0;
            
        glm::vec3 a_ndc;
        a_ndc.x = xpos;
        a_ndc.y = ypos;
        a_ndc.z = 0.0f;
            
        mouse_first_position = a_ndc;
    }
    else if((state == GLFW_PRESS) && (mouse_leftbutton_press)){
       
        mouse_leftbutton_press = true;
           
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        xpos = (xpos / 400.0) - 1.0;
        ypos  = (-ypos / 400.0) + 1.0;
            
        glm::vec3 a_ndc;
        a_ndc.x = xpos;
        a_ndc.y = ypos;
        a_ndc.z = 0.0f;
            
        mouse_second_position = a_ndc;
       
      //  cout<<our_model.mouse_press_movement.x<<" "<<our_model.mouse_press_movement.y<<endl;
    }
    
    
    
}

