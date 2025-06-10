#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm> 
#include <GL/glew.h>
#include <GL/glut.h>


struct Vector3 {
    float x, y, z;
};

struct Triangle {
    unsigned int indices[3];
};

std::vector<Vector3> gPositions;
std::vector<Vector3> gNormals;
std::vector<Triangle> gTriangles;


float gTotalTimeElapsed = 0;
int gTotalFrames = 0;
GLuint gTimer;


const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 1280;


void tokenize(char* string, std::vector<std::string>& tokens, const char* delimiter);
int face_index(const char* string);
void load_mesh(std::string fileName);

void init_timer();
void start_timing();
float stop_timing();

void initGL();
void display();
void renderSceneQ1();
void reshape(int w, int h);

void idle();

void tokenize(char* string, std::vector<std::string>& tokens, const char* delimiter) {
    char* next_token = NULL;
    char* token = strtok_s(string, delimiter, &next_token);
    while (token != NULL) {
        tokens.push_back(std::string(token));
        token = strtok_s(NULL, delimiter, &next_token);
    }
}

int face_index(const char* string) {
    int length = strlen(string);
    char* copy = new char[length + 1];
   
    strcpy_s(copy, length + 1, string); 

    std::vector<std::string> tokens;
    char* mutable_copy_for_tokenize = new char[length + 1];
   
    strcpy_s(mutable_copy_for_tokenize, length + 1, copy); 

    tokenize(mutable_copy_for_tokenize, tokens, "/");
    delete[] copy;
    delete[] mutable_copy_for_tokenize;

    if (!tokens.empty()) {
        return atoi(tokens.front().c_str());
    }
    else {
        printf("ERROR: Bad face specifier format!\n");
        exit(1);
    }
    return 0;
}


void load_mesh(std::string fileName) {
    printf("DEBUG: Attempting to load mesh from file: %s\n", fileName.c_str());
    std::ifstream fin(fileName.c_str());
    if (!fin.is_open()) {
        printf("ERROR: Unable to load mesh from %s!\n", fileName.c_str());
        exit(1);
    }

    float xmin = FLT_MAX, xmax = -FLT_MAX;
    float ymin = FLT_MAX, ymax = -FLT_MAX;
    float zmin = FLT_MAX, zmax = -FLT_MAX;

    char line[1024];
    while (true) {
        fin.getline(line, 1024);
        if (fin.eof()) break;
        if (strlen(line) <= 1) continue;

        std::vector<std::string> tokens;
        char* line_copy = new char[strlen(line) + 1];
        strcpy_s(line_copy, strlen(line) + 1, line); 
        tokenize(line_copy, tokens, " ");
        delete[] line_copy;

        if (tokens.empty()) continue;

        if (tokens[0] == "v") {
            if (tokens.size() < 4) continue;
            float x = atof(tokens[1].c_str());
            float y = atof(tokens[2].c_str());
            float z = atof(tokens[3].c_str());

            xmin = std::min(x, xmin); xmax = std::max(x, xmax);
            ymin = std::min(y, ymin); ymax = std::max(y, ymax);
            zmin = std::min(z, zmin); zmax = std::max(z, zmax);

            Vector3 position = { x, y, z };
            gPositions.push_back(position);
        }
        else if (tokens[0] == "vn") {
            if (tokens.size() < 4) continue;
            float x = atof(tokens[1].c_str());
            float y = atof(tokens[2].c_str());
            float z = atof(tokens[3].c_str());
            Vector3 normal = { x, y, z };
            gNormals.push_back(normal);
        }
        else if (tokens[0] == "f") {
            if (tokens.size() < 4) continue;
            unsigned int a = face_index(tokens[1].c_str());
            unsigned int b = face_index(tokens[2].c_str());
            unsigned int c = face_index(tokens[3].c_str());
            Triangle triangle;
            triangle.indices[0] = a - 1;
            triangle.indices[1] = b - 1;
            triangle.indices[2] = c - 1;
            gTriangles.push_back(triangle);
        }
    }
    fin.close();
    printf("Loaded mesh from %s. (%lu vertices, %lu normals, %lu triangles)\n",
        fileName.c_str(), gPositions.size(), gNormals.size(), gTriangles.size());
    printf("Mesh bounding box is: (%0.4f, %0.4f, %0.4f) to (%0.4f, %0.4f, %0.4f)\n",
        xmin, ymin, zmin, xmax, ymax, zmax);
}

void init_timer() {
    glGenQueries(1, &gTimer);
}

void start_timing() {
    glBeginQuery(GL_TIME_ELAPSED, gTimer);
}

float stop_timing() {
    glEndQuery(GL_TIME_ELAPSED);
    GLint available = GL_FALSE;
    while (available == GL_FALSE) {
        glGetQueryObjectiv(gTimer, GL_QUERY_RESULT_AVAILABLE, &available);
    }
    GLint result;
    glGetQueryObjectiv(gTimer, GL_QUERY_RESULT, &result);
    return result / (1000.0f * 1000.0f * 1000.0f); 
}

void initGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);

    GLfloat ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f }; 
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);

    GLfloat lightAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f }; 
    GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f }; 
    GLfloat lightSpecular[] = { 0.0f, 0.0f, 0.0f, 1.0f }; 
    GLfloat lightPosition[] = { 1.0f, 1.0f, 1.0f, 0.0f }; 
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    GLfloat materialAmbient[] = { 1.0f, 1.0f, 1.0f, 1.0f }; 
    GLfloat materialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f }; 
    GLfloat materialSpecular[] = { 0.0f, 0.0f, 0.0f, 1.0f }; 
    GLfloat materialShininess[] = { 0.0f }; 

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

 
    glTranslatef(0.1f, -1.0f, -1.5f);
    glScalef(10.0f, 10.0f, 10.0f);
   

    start_timing();
    renderSceneQ1();
    float timeElapsed = stop_timing();

    gTotalFrames++;
    gTotalTimeElapsed += timeElapsed;
    float fps = 0.0f;
    if (gTotalTimeElapsed > 0.00001) {
        fps = gTotalFrames / gTotalTimeElapsed;
    }

    char windowTitle[256];
    
    sprintf_s(windowTitle, sizeof(windowTitle), "OpenGL Bunny (Q1 - Immediate): %.2f FPS", fps); 
    glutSetWindowTitle(windowTitle);

    glutSwapBuffers();
}

void renderSceneQ1() {
    glBegin(GL_TRIANGLES);
    for (const auto& triangle : gTriangles) {
        for (int i = 0; i < 3; ++i) {
            unsigned int vertexIndex = triangle.indices[i];
            if (vertexIndex < gNormals.size() && vertexIndex < gPositions.size()) { 
                glNormal3f(gNormals[vertexIndex].x, gNormals[vertexIndex].y, gNormals[vertexIndex].z);
                glVertex3f(gPositions[vertexIndex].x, gPositions[vertexIndex].y, gPositions[vertexIndex].z);
            }
        }
    }
    glEnd();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);


    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float l = -0.1f;
    float r = 0.1f;
    float b = -0.1f;
    float t = 0.1f;
    float n_val = 0.1f;
    float f_val = 1000.0f;
    glFrustum(l, r, b, t, n_val, f_val);

    glMatrixMode(GL_MODELVIEW);
}

void idle() {
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("OpenGL Bunny (Q1 - Immediate Mode)");

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        return 1;
    }
    fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

    init_timer();
    load_mesh("bunny.obj");

    if (gPositions.empty()) {
        fprintf(stderr, "Mesh loading failed or mesh is empty. Exiting.\n");
        return 1;
    }

    initGL();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);

    printf("Press 'ESC' to quit.\n");

    glutMainLoop();

    return 0;
}