#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <Eigen/Geometry>
#include <Eigen/StdVector>

#include "mesh.h"
#include "shader_m.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>
using namespace std;


bool point_in_polygon(vector<glm::vec3>& morphing_stroke_ndc, double testx, double testy);




class Model 
{
public:
    /*  Model Data */
   
    vector<Mesh> meshes;
    vector<glm::vec3> circle_vertices_ndc;
    
    glm::vec3 mouse_press_movement;

    /*  Functions   */
    // constructor, expects a filepath to a 3D model.
    Model(string const &path)
    {
        loadModel(path);
    }

    // draws the model, and thus all its meshes
    void Draw(Shader shader)
    {
        for(unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }
    void select_vertices(glm::mat4& model, glm::mat4& view,glm::mat4& projection, glm::vec3 current_color)
    {
    
        vector<unsigned int> roi;
        vector<glm::vec4> PosLs;
        vector<Vertex> ndc_vertices;

        for(unsigned int i = 0; i < meshes[0].vertices.size(); ++i){
            glm::vec3 aPos = meshes[0].vertices[i].Position;
           
            glm::vec4 PosL   = projection * view * model * glm::vec4(aPos, 1.0);
            PosL.x = PosL.x / PosL.w;
            PosL.y = PosL.y / PosL.w;
            PosL.z = PosL.z / PosL.w;
            PosLs.push_back(PosL);
            ndc_vertices.push_back(meshes[0].vertices[i]);
            ndc_vertices[i].Position.x = PosL.x;
            ndc_vertices[i].Position.y = PosL.y;
            ndc_vertices[i].Position.z = PosL.z;
        }
        for(unsigned int i = 0; i < meshes[0].vertices.size(); ++i){
            glm::vec3 aPos = meshes[0].vertices[i].Position;
            for(unsigned int j = 0; j < meshes[0].vertices.size(); ++j){
                glm::vec3 bPos = meshes[0].vertices[j].Position;
                if((aPos.x == bPos.x) && (aPos.y == bPos.y) && (aPos.z == bPos.z)){
                    cout<<i<<" "<<j<<"    ";
                }
                
            }
             cout<<endl;
           
            
        }

        for(unsigned int i = 0; i < meshes[0].indices.size() - 2; i = i + 3){
         //   cout<<meshes[0].indices[i]<<" ";
         //   cout<<meshes[0].indices[i+1]<<" ";
         //   cout<<meshes[0].indices[i+2]<<endl;
         
         
        }
        for(unsigned int i = 0; i < ndc_vertices.size(); ++i){
            float testx = ndc_vertices[i].Position.x;
            float testy = ndc_vertices[i].Position.y;
            bool inside = point_in_polygon(circle_vertices_ndc, testx, testy);
            if(inside){
                 roi.push_back(i);
            }
        }
        for(unsigned int i = 0; i < roi.size(); ++i){
            unsigned int index= roi[i];
            meshes[0].vertices[index].Color.x = current_color.x;
            meshes[0].vertices[index].Color.y = current_color.y;
            meshes[0].vertices[index].Color.z = current_color.z;
        }
        
        for(unsigned int i = 0; i < roi.size(); ++i){
            unsigned int index= roi[i];
            
            for(unsigned int j = 0; j < ndc_vertices.size(); ++j){
                double distance = sqrt((ndc_vertices[index].Position.x - ndc_vertices[j].Position.x) * (ndc_vertices[index].Position.x - ndc_vertices[j].Position.x) + (ndc_vertices[index].Position.y - ndc_vertices[j].Position.y) * (ndc_vertices[index].Position.y - ndc_vertices[j].Position.y));
                if(distance < 0.2){
                    if(meshes[0].vertices[j].Color.x == 0.2f){
                        meshes[0].vertices[j].Color.x = 0.4f;
                        meshes[0].vertices[j].Color.y = 0.4f;
                        meshes[0].vertices[j].Color.z = 0.4f;
                    }
                }
            }
   
        }
        
        
        
        meshes[0].setupMesh();
    }
    
    void deform(glm::mat4& model, glm::mat4& view,glm::mat4& projection)
    {
        vector<unsigned int> roi;
        vector<glm::vec4> PosLs;
        vector<Vertex> ndc_vertices;

        for(unsigned int i = 0; i < meshes[0].vertices.size(); ++i){
            glm::vec3 aPos = meshes[0].vertices[i].Position;
           
            glm::vec4 PosL   = projection * view * model * glm::vec4(aPos, 1.0);
            PosL.x = PosL.x / PosL.w;
            PosL.y = PosL.y / PosL.w;
            PosL.z = PosL.z / PosL.w;
            PosLs.push_back(PosL);
            ndc_vertices.push_back(meshes[0].vertices[i]);
            ndc_vertices[i].Position.x = PosL.x;
            ndc_vertices[i].Position.y = PosL.y;
            ndc_vertices[i].Position.z = PosL.z;
        }
        
        vector<vector<unsigned int> > adjacent_points;
        bool connection_states[meshes[0].vertices.size()][meshes[0].vertices.size()];
        for(unsigned int i = 0; i < meshes[0].vertices.size(); ++i){
            for(unsigned int j = 0; j < meshes[0].vertices.size(); ++j){
                connection_states[i][j] = false;
            }
        }
        adjacent_points.resize(meshes[0].vertices.size());
        
        for(unsigned int i = 0; i < meshes[0].indices.size() - 2; i = i + 3){
            unsigned int index0 = meshes[0].indices[i];
            unsigned int index1 = meshes[0].indices[i+1];
            unsigned int index2 = meshes[0].indices[i+2];
            
            bool repeated_index = false;
            bool repeated_another_index = false;
            for(unsigned int i = 0; i < adjacent_points[index0].size(); ++i){
                if(adjacent_points[index0][i] == index1){
                    repeated_index = true;
                }
                if(adjacent_points[index0][i] == index2){
                    repeated_another_index = true;
                }
            }
            if(!repeated_index){
                adjacent_points[index0].push_back(index1);
            }
            if(!repeated_another_index){
                adjacent_points[index0].push_back(index2);
            }
            
            repeated_index = repeated_another_index = false;
            for(unsigned int i = 0; i < adjacent_points[index1].size(); ++i){
                if(adjacent_points[index1][i] == index0){
                    repeated_index = true;
                }
                if(adjacent_points[index1][i] == index2){
                    repeated_another_index = true;
                }
            }
            if(!repeated_index){
                 adjacent_points[index1].push_back(index0);
            }
            if(!repeated_another_index){
                 adjacent_points[index1].push_back(index2);
            }
            
            
            repeated_index = repeated_another_index = false;
            for(unsigned int i = 0; i < adjacent_points[index2].size(); ++i){
                if(adjacent_points[index2][i] == index0){
                    repeated_index = true;
                }
                if(adjacent_points[index2][i] == index1){
                    repeated_another_index = true;
                }
            }
            if(!repeated_index){
                adjacent_points[index2].push_back(index0);
                      
            }
            if(!repeated_another_index){
                adjacent_points[index2].push_back(index1);
            }
        }
        
        for(unsigned int i = 0; i < meshes[0].vertices.size(); ++i){
            for(unsigned int j = 0; j < adjacent_points[i].size(); ++j){
                unsigned int index0 = i;
                unsigned int index1 = adjacent_points[i][j];
                connection_states[index0][index1] = true;
                connection_states[index1][index0] = true;

            }
        }
        
        
        Eigen::MatrixXf DL_matrix = Eigen::MatrixXf(meshes[0].vertices.size(), meshes[0].vertices.size());
        // L = I - D-1A
        // DL = D - A
        for(unsigned int i = 0; i < meshes[0].vertices.size(); ++i){
            for(unsigned int j = 0; j < meshes[0].vertices.size(); ++j){
                if(i == j){
                    DL_matrix(i, j) = adjacent_points[i].size();
                }
                else if(connection_states[i][j]){
                    DL_matrix(i, j) = -1;
                }
                else{
                    DL_matrix(i, j) = 0;
                }     
            }
        }
        
        Eigen::VectorXf vertices_x(meshes[0].vertices.size());
        Eigen::VectorXf vertices_y(meshes[0].vertices.size());
        Eigen::VectorXf vertices_z(meshes[0].vertices.size());
        
        for (unsigned int i = 0; i < meshes[0].vertices.size(); ++i)
        {
            vertices_x[i] = ndc_vertices[i].Position.x;
            vertices_y[i] = ndc_vertices[i].Position.y;
            vertices_z[i] = ndc_vertices[i].Position.z;
        }
        
        Eigen::VectorXf B_x, B_y, B_z;
        B_x = DL_matrix * vertices_x;
        B_y = DL_matrix * vertices_y;
        B_z = DL_matrix * vertices_z;
        
        
        vector<unsigned int> anchors;
        for(unsigned int i = 0; i < ndc_vertices.size(); ++i){
            if((ndc_vertices[i].Color.x == 0.7f) || (ndc_vertices[i].Color.x == 0.2f)){
                anchors.push_back(i);
            }
        }
        
        Eigen::MatrixXf DLs_matrix = Eigen::MatrixXf(meshes[0].vertices.size() + anchors.size(), meshes[0].vertices.size() + anchors.size());
        for(unsigned int i = 0; i < meshes[0].vertices.size(); ++i){
            for(unsigned int j = 0; j < meshes[0].vertices.size(); ++j){
                if(i == j){
                    DLs_matrix(i, j) = adjacent_points[i].size();
                }
                else if(connection_states[i][j]){
                    DLs_matrix(i, j) = -1;
                }
                else{
                    DLs_matrix(i, j) = 0;
                }
            }
        }
        for(unsigned int i = meshes[0].vertices.size(); i < meshes[0].vertices.size() + anchors.size(); ++i){
            for(unsigned int j = 0; j < meshes[0].vertices.size(); ++j){
                DLs_matrix(i, j) = 0;
            }
        }
        for(unsigned int i = 0; i < anchors.size(); ++i){
            unsigned int size = meshes[0].vertices.size();
            DLs_matrix(size + i, anchors[i]) = 1;
        }
        
        for(unsigned int i = 0; i < ndc_vertices.size(); ++i){
            if(0.7f == ndc_vertices[i].Color.x){
                roi.push_back(i);
                ndc_vertices[i].Position.x += mouse_press_movement.x;
                ndc_vertices[i].Position.y += mouse_press_movement.y;
            }
        }
        
        B_x.conservativeResize(meshes[0].vertices.size() + anchors.size());
        B_y.conservativeResize(meshes[0].vertices.size() + anchors.size());
        B_z.conservativeResize(meshes[0].vertices.size() + anchors.size());
        
        for (unsigned int i = 0; i < anchors.size(); ++i)
        {
            B_x[i + meshes[0].vertices.size()] = ndc_vertices[anchors[i]].Position.x;
            B_y[i + meshes[0].vertices.size()] = ndc_vertices[anchors[i]].Position.y;
            B_z[i + meshes[0].vertices.size()] = ndc_vertices[anchors[i]].Position.z;
            
        }
        
        Eigen::MatrixXf DLs_transpose = DLs_matrix.transpose();
        Eigen::MatrixXf DLs_transpose_DLs = DLs_transpose * DLs_matrix;
        
        Eigen::MatrixXf DLs_transpose_Bx = DLs_transpose * B_x;
        Eigen::MatrixXf DLs_transpose_By = DLs_transpose * B_y;
        Eigen::MatrixXf DLs_transpose_Bz = DLs_transpose * B_z;
        
        
        Eigen::VectorXf new_vertices_x;
        Eigen::VectorXf new_vertices_y;
        Eigen::VectorXf new_vertices_z;
        
        new_vertices_x = DLs_transpose_DLs.ldlt().solve(DLs_transpose_Bx);
        new_vertices_y = DLs_transpose_DLs.ldlt().solve(DLs_transpose_By);
        new_vertices_z = DLs_transpose_DLs.ldlt().solve(DLs_transpose_Bz);
      
        
        for (unsigned int i = 0; i < meshes[0].vertices.size(); ++i)
        {
            ndc_vertices[i].Position.x = new_vertices_x[i];
            ndc_vertices[i].Position.y = new_vertices_y[i];
            ndc_vertices[i].Position.z = new_vertices_z[i];
           
        }
        for(unsigned int i = 0; i <ndc_vertices.size(); ++i){
          
            float w = PosLs[i].w;
                
                ndc_vertices[i].Position.x = ndc_vertices[i].Position.x * w;
                ndc_vertices[i].Position.y = ndc_vertices[i].Position.y * w;
                ndc_vertices[i].Position.z = ndc_vertices[i].Position.z * w;
                
                glm::mat4 transform = projection * view * model;
             
                glm::vec4 point = glm::inverse(transform) * glm::vec4(ndc_vertices[i].Position, w);
                meshes[0].vertices[i].Position.x = point.x;
                meshes[0].vertices[i].Position.y = point.y;
                meshes[0].vertices[i].Position.z = point.z;
     
        }
        
        meshes[0].setupMesh();
    }
private:
    /*  Functions   */
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string const &path)
    {
        // read file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        // check for errors
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        // retrieve the directory path of the filepath
      //  directory = path.substr(0, path.find_last_of('/'));

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode *node, const aiScene *scene)
    {
        // process each mesh located at the current node
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }

    Mesh processMesh(aiMesh *mesh, const aiScene *scene)
    {
        // data to fill
        vector<Vertex> vertices;
        vector<unsigned int> indices;


        // Walk through each of the mesh's vertices
        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // normals
          //  vector.x = mesh->mNormals[i].x;
          //  vector.y = mesh->mNormals[i].y;
          //  vector.z = mesh->mNormals[i].z;
            vector.x = rand() / double(RAND_MAX);
            vector.y = rand() / double(RAND_MAX);
            vector.z = rand() / double(RAND_MAX);
            
            vertex.Normal = vector;
            
            vector.x = 0.2;
            vector.y = 0.2;
            vector.z = 0.2;
            vertex.Color = vector;
            
            vertices.push_back(vertex);
        }
        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for(unsigned int j = 0; j < face.mNumIndices; j++){
               
                indices.push_back(face.mIndices[j]);
            }
          
        }
        
        unsigned int old_new_indices[vertices.size()];
        for(unsigned int i = 0; i < vertices.size(); ++i){
            old_new_indices[i] = i;
        }
        for(unsigned int i = 0; i < vertices.size(); ++i){
            glm::vec3 first_vertex = vertices[i].Position;
            for(unsigned int j = i + 1; j < vertices.size(); ++j){
                glm::vec3 second_vertex = vertices[j].Position;
                if((first_vertex.x == second_vertex.x) && (first_vertex.y == second_vertex.y) && (first_vertex.z == second_vertex.z)){
                    if(old_new_indices[j] == j){
                        old_new_indices[j] = i;
                    }
                }
                
            }
        }
        for(unsigned int i = 0; i < indices.size(); ++i){
            unsigned int old_index = indices[i];
            
            unsigned int new_index = old_new_indices[old_index];
            
            indices[i] = new_index;
         
        }
        
        vector<Vertex> new_vertices;
        for(unsigned int i = 0; i < vertices.size(); ++i){
            if(i == old_new_indices[i]){
                new_vertices.push_back(vertices[i]);
                old_new_indices[i] = new_vertices.size() - 1;
            }
        }
        
        for(unsigned int i = 0; i < indices.size(); ++i){
            unsigned int old_index = indices[i];
            
            unsigned int new_index = old_new_indices[old_index];
            
            indices[i] = new_index;
         
        }
        
        // return a mesh object created from the extracted mesh data
        return Mesh(new_vertices, indices);
    }

    
};


/*
 Copyright (c) 1970-2003, Wm. Randolph Franklin
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimers.
 Redistributions in binary form must reproduce the above copyright notice in the documentation and/or other materials provided with the distribution.
 The name of W. Randolph Franklin may not be used to endorse or promote products derived from this Software without specific prior written permission.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
bool point_in_polygon(vector<glm::vec3>& morphing_stroke_ndc, double testx, double testy)
{
    int i, j, c = 0;
    int nvert = morphing_stroke_ndc.size();
    
    for (i = 0, j = nvert-1; i < nvert; j = i++) {
        
        if ( ((morphing_stroke_ndc[i].y >=testy) != (morphing_stroke_ndc[j].y>=testy)) &&
            (testx <= (morphing_stroke_ndc[j].x -morphing_stroke_ndc[i].x) * (testy-morphing_stroke_ndc[i].y) / (morphing_stroke_ndc[j].y-morphing_stroke_ndc[i].y) + morphing_stroke_ndc[i].x) )
            c = !c;
    }
  //  cout<<c<<endl;
    if(1 == c){
        return true;
    }
    return false;
    
}

#endif
