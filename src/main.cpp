#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <ctime>

#include "poisson_disk_sampling.h"

#include "utils.h"
#include "veinnode.h"

#define MARGIN_RES 100

using namespace std;

int window_width = 1000, window_height = 1000;
vector<float> leafMargin;
vector<float> auxinSources;
vector<float> nodesDisplay;
float initGrowth = 1e-3f;
float uniformGrowth = initGrowth; // simulate growth throughout leaf
float marginGrowth = initGrowth; // simulate leaf margin growth
float smallChange = 1e-6f; // change in growth per frame
float srcSrcDist = 1.0f;
float srcNodeDist = 1.0f;
float nodeNodeDist = 1.0f;
float killDist = 1.0f;
float initUnitDist = 1.0f;
float unitDist = initUnitDist;
float petiole_x = 0.0f, petiole_y = 0.0f;
float org_x = 0.0f, org_y = 0.0f; // transformed origin
VeinNode* petiole;

GLint vModel_uniform, vView_uniform, vProjection_uniform;
glm::mat4 modelT, viewT, projectionT;//The model, view and projection transformations

void setupModelTransformation(unsigned int &);
void setupViewTransformation(unsigned int &);
void setupProjectionTransformation(unsigned int &);

float euclidDistance(float x1, float y1, float x2, float y2){
    return pow(pow(x1 - x2, 2) + pow(y1 - y2, 2), 0.5);
}

float getMarginDist(float phi){
    // using Gielis Superformula
    float m = 2.0, n1 = 1.0, n2 = 1.0, n3 = 1.0;
    float a = 2.0, b = 1.0;
    float raux = pow(abs(cos(m * phi / 4) / a), n2) + pow(abs(sin(m * phi / 4)) / b, n3);
    float r = pow(abs(raux), - 1 / n1) * 20;
    return r;
}

void drawLeafMargin(){
    for (float phi = 0.0; phi < 2 * M_PI; phi += M_PI / MARGIN_RES){
    float r = getMarginDist(phi);
    leafMargin.push_back(r * cos(phi));
    leafMargin.push_back(r * sin(phi));
    leafMargin.push_back(0.0);
    }
    petiole_x = leafMargin[MARGIN_RES * 3] - smallChange;
    petiole_y = leafMargin[MARGIN_RES * 3 + 1];
    petiole = new VeinNode(petiole_x, petiole_y);
}

void growLeafMargin(){
    for (int i = 0; i < leafMargin.size(); i += 3){
        float slope = (leafMargin[i+1] - petiole_y) / (leafMargin[i] - petiole_x);
        float theta = atan(slope);
        float distance = euclidDistance(leafMargin[i], leafMargin[i+1], petiole_x, petiole_y);
        leafMargin[i] += marginGrowth * distance * cos(theta);
        leafMargin[i+1] += marginGrowth * distance * sin(theta);
    }
    float slope = (org_y - petiole_y) / (org_x - petiole_x);
    float theta = atan(slope);
    float distance = euclidDistance(org_x, org_y, petiole_x, petiole_y);
    org_x += marginGrowth * distance * cos(theta);
    org_y += marginGrowth * distance * sin(theta);
    leafMargin[MARGIN_RES * 3] = petiole_x + smallChange; // petiole coordinates
    leafMargin[MARGIN_RES * 3 + 1] = petiole_y;           // remain constant
    uniformGrowth += smallChange;
    unitDist = initUnitDist * initGrowth / uniformGrowth;
    marginGrowth += smallChange;
}

void genAuxinSources(){
    srand(time(0));
    float x_min = leafMargin[MARGIN_RES * 3];
    float x_max = leafMargin[0];
    float y_max = leafMargin[MARGIN_RES * 3 / 2 + 1];
    float y_min = -y_max;
    array<float, 2> Xmin = {x_min, y_min};
    array<float, 2> Xmax = {x_max, y_max};
    vector<array<float, 2>> poissonRaw = thinks::PoissonDiskSampling(srcSrcDist * unitDist, Xmin, Xmax, rand() % 1000);
    for (auto p : poissonRaw){
        float angle = atan(p[1] / p[0]);
        if ((p[0] < 0 && p[1] > 0) || (p[0] < 0 && p[1] < 0)){ // 2nd & 3rd quadrant
            angle = M_PI + angle;
        }
        else if (p[0] > 0 && p[1] < 0){ // 4th quadrant
            angle = 2 * M_PI + angle;
        }
        int idx = 3 * static_cast<int>(angle * MARGIN_RES / M_PI);
        float p_dist = euclidDistance(org_x, org_y, p[0], p[1]);
        float margin_dist = euclidDistance(org_x, org_y, leafMargin[idx], leafMargin[idx+1]);
        float scale = margin_dist / getMarginDist(angle);
        margin_dist = scale * getMarginDist(angle);
        if (p_dist <= margin_dist){
            VeinNode* nearestNode = findNearestNode(petiole, p[0], p[1]);
            bool checkSrcDist = true;
            for (int i = 0; i < auxinSources.size(); i += 3){
                if (euclidDistance(p[0], p[1], auxinSources[i], auxinSources[i+1]) < srcSrcDist * unitDist){
                    checkSrcDist = false;
                }
            }
            if (euclidDistance(nearestNode, p[0], p[1]) > srcNodeDist * unitDist && checkSrcDist){
                auxinSources.push_back(p[0]);
                auxinSources.push_back(p[1]);
                auxinSources.push_back(0.0);
            }
        }
    }
}

void findNearestNodes(){
    vector<float> newAuxinSrcs;
    for (int i = 0; i < auxinSources.size(); i += 3){
        VeinNode* tmp = findNearestNode(petiole, auxinSources[i], auxinSources[i+1]);
        if (euclidDistance(tmp, auxinSources[i], auxinSources[i+1]) > killDist * unitDist){
            newAuxinSrcs.push_back(auxinSources[i]);
            newAuxinSrcs.push_back(auxinSources[i+1]);
            newAuxinSrcs.push_back(0.0f);
            tmp->addNewAuxinSrc(auxinSources[i], auxinSources[i+1]);
        }
    }
    auxinSources = newAuxinSrcs;
}

int main(int, char *argv[])
{
    GLFWwindow *window = setupWindow(window_width, window_height);
    ImGuiIO &io = ImGui::GetIO(); // Create IO object

    ImVec4 clear_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    unsigned int shaderProgram = createProgram("./shaders/vshader.vs", "./shaders/fshader.fs");
    glUseProgram(shaderProgram);

    setupModelTransformation(shaderProgram);
    setupViewTransformation(shaderProgram);
    setupProjectionTransformation(shaderProgram);

    GLuint VAO_margin, VBO_margin;
    glGenVertexArrays(1, &VAO_margin);
    glGenBuffers(1, &VBO_margin);

    GLuint VAO_petiole, VBO_petiole;
    glGenVertexArrays(1, &VAO_petiole);
    glGenBuffers(1, &VBO_petiole);

    GLuint VAO_auxinSrc, VBO_auxinSrc;
    glGenVertexArrays(1, &VAO_auxinSrc);
    glGenBuffers(1, &VBO_auxinSrc);

    GLuint VAO_node, VBO_node;
    glGenVertexArrays(1, &VAO_node);
    glGenBuffers(1, &VBO_node);

    bool display_srcs = false;

    drawLeafMargin();

    // Display loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Render();

        int display_w, display_h;

        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        genAuxinSources();

        findNearestNodes();

        nodesDisplay.clear();
        flattenTree(petiole, nodesDisplay);

        glBindVertexArray(VAO_margin);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_margin);
        glBufferData(GL_ARRAY_BUFFER, leafMargin.size() * sizeof(float), &leafMargin[0], GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(VAO_auxinSrc);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_auxinSrc);
        glBufferData(GL_ARRAY_BUFFER, auxinSources.size() * sizeof(float), &auxinSources[0], GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(VAO_node);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_node);
        glBufferData(GL_ARRAY_BUFFER, nodesDisplay.size() * sizeof(float), &nodesDisplay[0], GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glUseProgram(shaderProgram);

        glBindVertexArray(VAO_margin);
        glDrawArrays(GL_LINE_LOOP, 0, leafMargin.size() / 3);

        glBindVertexArray(VAO_auxinSrc);
        glDrawArrays(GL_POINTS, 0, auxinSources.size() / 3);

        glBindVertexArray(VAO_node);
        glDrawArrays(GL_LINES, 0, nodesDisplay.size() / 3);

        glUseProgram(0);
        
        placeNewNodes(petiole, nodeNodeDist);

        growLeafMargin();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    cleanup(window);
    return 0;
}

void setupModelTransformation(unsigned int &program)
{
    //Modelling transformations (Model -> World coordinates)
    modelT = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, -15.0, 0.0));//Model coordinates are the world coordinates
    modelT = glm::rotate(modelT, (GLfloat)M_PI/2, glm::vec3(0.0, 0.0, 1.0));
    //Pass on the modelling matrix to the vertex shader
    glUseProgram(program);
    vModel_uniform = glGetUniformLocation(program, "vModel");
    if(vModel_uniform == -1){
        fprintf(stderr, "Could not bind location: vModel\n");
        exit(0);
    }
    glUniformMatrix4fv(vModel_uniform, 1, GL_FALSE, glm::value_ptr(modelT));
}


void setupViewTransformation(unsigned int &program)
{
    //Viewing transformations (World -> Camera coordinates
    viewT = glm::lookAt(glm::vec3(0.0, 0.0, 100.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

    //Pass-on the viewing matrix to the vertex shader
    glUseProgram(program);
    vView_uniform = glGetUniformLocation(program, "vView");
    if(vView_uniform == -1){
        fprintf(stderr, "Could not bind location: vView\n");
        exit(0);
    }
    glUniformMatrix4fv(vView_uniform, 1, GL_FALSE, glm::value_ptr(viewT));
}

void setupProjectionTransformation(unsigned int &program)
{
    //Projection transformation
    projectionT = glm::perspective(45.0f, (GLfloat)window_width/(GLfloat)window_height, 0.1f, 1000.0f);

    //Pass on the projection matrix to the vertex shader
    glUseProgram(program);
    vProjection_uniform = glGetUniformLocation(program, "vProjection");
    if(vProjection_uniform == -1){
        fprintf(stderr, "Could not bind location: vProjection\n");
        exit(0);
    }
    glUniformMatrix4fv(vProjection_uniform, 1, GL_FALSE, glm::value_ptr(projectionT));
}