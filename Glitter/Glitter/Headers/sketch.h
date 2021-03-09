//
//  sketch.h
//  Glitter
//
//  Created by 胥皓 on 2019/6/29.
//

#ifndef sketch_h
#define sketch_h

#include <glad/glad.h> // holds all OpenGL type declarations
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "shader_m.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <cmath>
using namespace std;

const double sample_unit = 0.5f;


/*
* 两个向量之间的旋转角
* 首先明确几个数学概念：
* 1. 极轴沿逆时针转动的方向是正方向
* 2. 两个向量之间的夹角theta， 是指(A^B)/(|A|*|B|) = cos(theta)，0<=theta<=180 度， 而且没有方向之分
* 3. 两个向量的旋转角，是指从向量p1开始，逆时针旋转，转到向量p2时，所转过的角度， 范围是 0 ~ 360度
* 计算向量p1到p2的旋转角，算法如下：
* 首先通过点乘和arccosine的得到两个向量之间的夹角
* 然后判断通过差乘来判断两个向量之间的位置关系
* 如果p2在p1的顺时针方向, 返回arccose的角度值, 范围0 ~ 180.0(根据右手定理,可以构成正的面积)
* 否则返回 360.0 - arecose的值, 返回180到360(根据右手定理,面积为负)
*/


class Sketch{
public:
    
    vector<glm::vec3> morphing_stroke;
    vector<glm::vec3> morphing_stroke_ndc;
    vector<glm::vec3> morphing_stroke_normal;
    
    vector<glm::vec3> hole_points;
    vector<glm::vec3> hole_points_ndc;
    vector<unsigned int> hole_points_indices;
    
    
    float leftmost;
    float rightmost;
    float uppermost;
    float bottom;
    float center_x;
    float center_y;
    
    float circle_x;
    float circle_y;
    vector<glm::vec3> circle_stroke_ndc;
    
   
    unsigned int VAO;
    
    Sketch()
    {
        ;
    }
    void simplify_a_stroke(unsigned int start_index, unsigned int end_index, vector<glm::vec3> &sketches)
    {
        vector<glm::vec3> temp;
        
        unsigned int i, j;
        i = j = 0;
        glm::vec3 a_temp;
        a_temp.x = sketches[start_index].x;
        a_temp.y = sketches[start_index].y;
        a_temp.z = sketches[start_index].z;
      //  cout<<a_temp.x<<" "<<a_temp.y<<endl;
        temp.push_back(a_temp);
        for(i = 0, j = start_index + 1;  (j < end_index);){
            float x1 = temp[i].x;
            float y1 = temp[i].y;
            float x2 = sketches[j].x;
            float y2 = sketches[j].y;
            float distance = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
            if(distance < sample_unit){
             //   cout<<"< sample_unit"<<endl;
                j = j + 1;
            }
            if((distance >= sample_unit) && (distance < 2 * sample_unit)){
             //   cout<<">= sample_unit) && (distance < 2 * sample_unit"<<endl;
                glm::vec3 b_temp;
                b_temp.x = sketches[j].x;
                b_temp.y = sketches[j].y;
                b_temp.z = sketches[j].z;
            //    cout<<b_temp.x<<" "<<b_temp.y<<endl;
                temp.push_back(b_temp);
                
                i = i + 1;
                j = j + 1;
            }
            else{
              //  cout<<"else"<<endl;
                glm::vec3 c_temp;
             //   cout<<temp[i].x<<" "<<temp[i].y<<" "<<sketches[j].x<<" "<<sketches[j].y<<endl;
                c_temp.x = (temp[i].x + sketches[j].x) / 2.0;
                c_temp.y = (temp[i].y + sketches[j].y) / 2.0;
                c_temp.z = (temp[i].z + sketches[j].z) / 2.0;
             //   cout<<c_temp.x<<" "<<c_temp.y<<endl;
                temp.push_back(c_temp);
               // temp[i + 1].x =  (temp[i].x + sketches[j].x) / 2.0;
               // temp[i + 1].y =  (temp[i].y + sketches[j].y) / 2.0;
                i = i + 1;
               
            }
        }
        a_temp.x = sketches[end_index].x;
        a_temp.y = sketches[end_index].y;
        a_temp.z = sketches[end_index].z;
        temp.push_back(a_temp);
       
     //   i = i + 1;
        
        sketches = temp;
       // cout<<sketches.size()<<endl;
    }
    void compute_normals(vector<glm::vec3> &sketches)
    {
        
        glm::vec3 vector_point_last = sketches[0] - sketches[sketches.size() - 1];
        glm::vec3 vector_point_current = sketches[1] - sketches[0];
        glm::vec3 normal = vector_point_last + vector_point_current;
        normal = glm::rotate(normal, glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));
        glm::normalize(normal);
        morphing_stroke_normal.push_back(normal);
        for(unsigned int i = 1; i < sketches.size() - 1; ++i){
            vector_point_last = sketches[i] - sketches[i-1];
            vector_point_current = sketches[i+1] - sketches[i];
            normal = vector_point_last + vector_point_current;
            normal = glm::rotate(normal, glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));
            glm::normalize(normal);
            morphing_stroke_normal.push_back(normal);
        }
        vector_point_last = sketches[sketches.size() - 1] - sketches[sketches.size() - 2];
        vector_point_current = sketches[0] - sketches[sketches.size() - 1];
        normal = vector_point_last + vector_point_current;
        normal = glm::rotate(normal, glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));
        glm::normalize(normal);
        morphing_stroke_normal.push_back(normal);
        
    }
    void draw_points()
    {
        setup_sketch(morphing_stroke_ndc);
        unsigned int number_of_vertices;
        glBindVertexArray(VAO);
        
        number_of_vertices = morphing_stroke_ndc.size();
        glDrawArrays(GL_POINTS, 0, number_of_vertices);
    }
    void draw_circle_points()
    {
        setup_sketch(circle_stroke_ndc);
        unsigned int number_of_vertices;
        glBindVertexArray(VAO);
        
        number_of_vertices = circle_stroke_ndc.size();
        glDrawArrays(GL_LINE_STRIP, 0, number_of_vertices);
    }
    void draw_moprhingstroke()
    {
        setup_sketch(morphing_stroke_ndc);
        unsigned int number_of_vertices;
        glBindVertexArray(VAO);
        
        number_of_vertices = morphing_stroke_ndc.size();
        glDrawArrays(GL_LINE_STRIP, 0, number_of_vertices);
    }
    void compute_boundingbox()
    {
        leftmost = rightmost = morphing_stroke_ndc[0].x;
        uppermost = bottom = morphing_stroke_ndc[0].y;
        cout<<"sssss"<<endl;
        for(unsigned int i = 1; i < morphing_stroke_ndc.size(); ++i){
         //   cout<<leftmost<<" "<<rightmost<<" "<<uppermost<<" "<<bottom<<endl;
            if(morphing_stroke_ndc[i].x < leftmost){
                leftmost = morphing_stroke_ndc[i].x;
            }
            if(morphing_stroke_ndc[i].x > rightmost){
                rightmost = morphing_stroke_ndc[i].x;
            }
            if(morphing_stroke_ndc[i].y > uppermost){
                uppermost = morphing_stroke_ndc[i].y;
            }
            if(morphing_stroke_ndc[i].y < bottom){
                bottom = morphing_stroke_ndc[i].y;
            }
        }
        center_x = (leftmost + rightmost) / 2;
        center_y = (uppermost + bottom ) / 2;
        
    }
    void clear_morphingstroke()
    {
        morphing_stroke.clear();
        morphing_stroke_ndc.clear();
    }
    void compute_circle_points(float radius, float precision)
    {

        glm::vec3 temp;
        temp.x = circle_x;
        temp.y = circle_y + radius;
        temp.z = 0.0f;
        circle_stroke_ndc.push_back(temp);
        
        for(float f = circle_x + precision; f < circle_x + radius; f += precision){
            float y1 = sqrt(radius * radius - (f - circle_x) * (f - circle_x)) + circle_y;
          //  float y2 = -sqrt(radius * radius - (f - circle_x) * (f - circle_x)) + circle_y;
            temp.x = f;
            temp.y = y1;
            circle_stroke_ndc.push_back(temp);
        }
        temp.x = circle_x + radius;
        temp.y = circle_y;
        circle_stroke_ndc.push_back(temp);
        
        for(float f = circle_x + radius - precision ; f > circle_x ; f -= precision){
            float y1 = -sqrt(radius * radius - (f - circle_x) * (f - circle_x)) + circle_y;
          //  float y2 = -sqrt(radius * radius - (f - circle_x) * (f - circle_x)) + circle_y;
            temp.x = f;
            temp.y = y1;
            circle_stroke_ndc.push_back(temp);
        }
        temp.x = circle_x ;
        temp.y = circle_y - radius;
        circle_stroke_ndc.push_back(temp);
        
        for(float f = circle_x - precision ; f > circle_x - radius; f -= precision){
            float y1 = -sqrt(radius * radius - (f - circle_x) * (f - circle_x)) + circle_y;
          //  float y2 = -sqrt(radius * radius - (f - circle_x) * (f - circle_x)) + circle_y;
            temp.x = f;
            temp.y = y1;
            circle_stroke_ndc.push_back(temp);
        }
        temp.x = circle_x - radius;
        temp.y = circle_y;
        circle_stroke_ndc.push_back(temp);
        
        for(float f = circle_x - radius + precision; f < circle_x ; f += precision){
            float y1 = sqrt(radius * radius - (f - circle_x) * (f - circle_x)) + circle_y;
          //  float y2 = -sqrt(radius * radius - (f - circle_x) * (f - circle_x)) + circle_y;
            temp.x = f;
            temp.y = y1;
            circle_stroke_ndc.push_back(temp);
        }
        
        
    }

private:
    /*  Render data  */
    unsigned int VBO;
    
    /*  Functions    */
    // initializes all the buffer objects/arrays
    void setup_sketch(vector<glm::vec3> &sketches)
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        glBindVertexArray(VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sketches.size() * sizeof(sketches), &sketches[0], GL_STREAM_DRAW);
       
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
        // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
        glBindVertexArray(0);
      }
    
    
    
};


#endif /* sketch_h */
