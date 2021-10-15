#include "ObjLoader.h"
#include "IndexBuffer.h"

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

// Borrowed from https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
#pragma region String Trimming

// trim from start (in place)
static inline void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
        }));
}

// trim from end (in place)
static inline void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
        }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string& s) {
    ltrim(s);
    rtrim(s);
}

#pragma endregion 

VertexArrayObject::Sptr ObjLoader::LoadFromFile(const std::string& filename)
{
    // Open our file in binary mode
    std::ifstream file;
    file.open(filename, std::ios::binary);

    // If our file fails to open, we will throw an error
    if (!file) {
        throw std::runtime_error("Failed to open file");
    }

    std::string line;

    // TODO: Load data from file
    std::vector<glm::vec3> positions; // from V command
    std::vector<glm::ivec3> vertices; // from face lines
    std::vector<glm::vec2> textures;	// textures UV
    std::vector<glm::vec3> normals;		// Normals

    // temporaries used for loading data
    glm::vec3 vecData;
    glm::ivec3 vertexIndices;
    glm::vec2 uvData;
    glm::vec3 normalData;

    std::vector<uint32_t> indexes;
    int count = 0;

   std::cout << "INITIAL VERTS: " << vertices.size() << "\n";
    // Read and process the entire file
    while (file.peek() != EOF) {
        // Read in the first part of the line (ex: f, v, vn, etc...)
        std::string command;
        file >> command;

        // We will ignore the rest of the line for comment lines
        if (command == "#") {
            std::getline(file, line);
        }
        // The v command defines a vertex's position
        else if (command == "v") {
            // Read in and store a position
            file >> vecData.x >> vecData.y >> vecData.z;
            positions.push_back(vecData);
        }
         //TODO: handle normals and textures
        else if (command == "vt") {
        
        	file >> uvData.x >> uvData.y;
        	textures.push_back(uvData);
        }
        
        else if (command == "vn") {
        	file >> normalData.x >> normalData.y >> normalData.z;
        	normals.push_back(normalData);
        }

        // The f command defines a polygon in the mesh
            // NOTE: make sure you triangulate in blender, otherwise it will
            // output quads instead of triangles
        else if (command == "f") {
            // Read the rest of the line from the file
            std::getline(file, line);
            // Trim whitespace from either end of the line
            trim(line);
            // Create a string stream so we can use streaming operators on it
            std::stringstream stream = std::stringstream(line);

            int dupeIndex = 0;                     

            for (int ix = 0; ix < 3; ix++) {
                
                std::cout << "VERTEX NUMBER: " << vertices.size() << "\n";
                bool dupeFlag = false;

                // Read in the 3 attributes (position, UV, normal)
                char separator;
                stream >> vertexIndices.x >> separator >> vertexIndices.y >> separator >> vertexIndices.z;

                // OBJ format uses 1-based indices
                vertexIndices -= glm::ivec3(1);

            // Compares Vertex Indices vs every other vertex
            // For efficiency I'm considering using key-value pairs for the engine 
            // since time complexity is currently Log(N^2)

            for (int i = 0; i < vertices.size();i++) {   
                // checks for dupes and logs their position
                if (vertexIndices.x == vertices[i].x && vertexIndices.y == vertices[i].y && vertexIndices.z == vertices[i].z) {
                    //std::cout << "I have found an imposta!\n";
                    count++;
            		dupeFlag = true;
                    dupeIndex = i;
                    
                    //std::cout << "Dupe Position: " << dupeIndex << "\n";
                   // std::cout << "Value at index: " << vertices[dupeIndex].x << "/" << vertices[dupeIndex].y << "/" << vertices[dupeIndex].z << "\n";
                    //std::cout << "Value of vertices: " << vertexIndices.x << "/" << vertexIndices.y << "/" << vertexIndices.z << "\n";
                    break;
            	}
            }
                

              // vertices.push_back(vertexIndices);
              // indexes.push_back(vertices.size()-1);

               if (!dupeFlag) {
                   std::cout << "Value of vertices: " << vertexIndices.x << "/" << vertexIndices.y << "/" << vertexIndices.z << "\n";

                   indexes.push_back(vertices.size());
                   vertices.push_back(vertexIndices);
                            
               }
               else {                   
                  indexes.push_back(dupeIndex);
                  
               }
               dupeFlag = false;
            }
        }
    }


    // TODO: Generate mesh from the data we loaded
    std::vector<VertexPosNormTexCol> vertexData;

    for (int ix = 0; ix < vertices.size(); ix++) {
        glm::ivec3 attribs = vertices[ix];

        // Extract attributes from lists (except color)
        glm::vec3 position = positions[attribs.x];
        glm::vec2 uv = textures[attribs.y];
        glm::vec3 normal = normals[attribs.z];
        glm::vec4 color = glm::vec4(1.0f);

        // Add the vertex to the mesh
        vertexData.push_back(VertexPosNormTexCol(position, normal, uv, color));
    }

    // Create a vertex buffer and load all our vertex data
    VertexBuffer::Sptr vertexBuffer = VertexBuffer::Create();
    vertexBuffer->LoadData(vertexData.data(), vertexData.size());

    // Create an Index Buffer and load the index data
    IndexBuffer::Sptr ibo = IndexBuffer::Create();
    ibo->LoadData(indexes.data(), indexes.size()); 

    // Create the VAO, and add the vertices
    VertexArrayObject::Sptr result = VertexArrayObject::Create();
    result->AddVertexBuffer(vertexBuffer, VertexPosNormTexCol::V_DECL);
    result->SetIndexBuffer(ibo);

    std::cout << "FINAL VERTEX: " << vertices.size() << "\n";
    std::cout << "FINAL INDEX: " << indexes.size() << "\n";
    std::cout << "DUPES COUNT: " << count << "\n";

    return result;
    //return VertexArrayObject::Create();

}