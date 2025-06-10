About
===
rendering mash with immediate mode
---
How to Use
===
1. Download zip piles.  
   
![download zip](https://github.com/user-attachments/assets/3e76e9d2-5325-42a3-ba52-2bb3064c0a58)

2. Unzip the folder  
3. open "OpenglViewer.sln"  
![leanch](https://github.com/user-attachments/assets/1ed43ef3-d812-4b75-809d-fe1077eabf9b)
---
Result of assignmet8-1 
FPS=1377

![스크린샷 2025-05-08 162804](https://github.com/user-attachments/assets/e4b9e9e5-8f1c-4161-b903-0df9afa9ce07)

---
Explanation
---
Call the load_mesh() function of load_mesh.cpp to store all the geometric data in the bunny.obj file in CPU memory (RAM).

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

Prepare a timer for FPS measurements by invoking the init_timer() function of the provided frame_timer.cpp.

![image](https://github.com/user-attachments/assets/257fc2c5-2454-4c5a-aa8d-75d29f70d136)

With initGL() function, I constructed a rendering environment based on a fixed function pipeline, such as lighting, materials, etc., according to the task specification.

![image](https://github.com/user-attachments/assets/737a4cce-e911-4092-99f3-968659cc5f76)

The rendersSceneQ1() function uses a for loop within the glBegin/End block.
The loop is designed to implement the core mode of instant mode, where the CPU traverses all the triangular data in RAM and sends each vertex information to the GPU individually via the glVertex3f and glNormal3f functions.

![image](https://github.com/user-attachments/assets/3311b23e-d109-4d33-983c-26034d52a97a)

Every frame, a display() function is called to adjust the size and position of the model and execute the actual drawing function renderSceneQ1().

![image](https://github.com/user-attachments/assets/7b8bec6d-bc16-4024-9a1e-24f2b17770d9)

The main function loads data into CPU memory with load_mesh and then immediately enters the rendering loop without a separate GPU data preparation process.

![image](https://github.com/user-attachments/assets/a0dfa9d2-3776-4383-9ab4-ca4235b49326)



