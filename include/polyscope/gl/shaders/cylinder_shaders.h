// Copyright 2017-2019, Nicholas Sharp and the Polyscope contributors. http://polyscope.run.
#pragma once

#include "polyscope/gl/shaders.h"

namespace polyscope {
namespace gl {

// clang-format off

static const VertShader PASSTHRU_CYLINDER_VERT_SHADER = {
    // uniforms
    {
    }, 

    // attributes
    {
        {"a_position_tail", GLData::Vector3Float},
        {"a_position_tip", GLData::Vector3Float},
    },

    // source
    POLYSCOPE_GLSL(150,
        in vec3 a_position_tail;
        in vec3 a_position_tip;

        out vec3 position_tip;

        void main()
        {
            gl_Position = vec4(a_position_tail,1.0);
            position_tip = a_position_tip;
        }
    )
};



static const GeomShader CYLINDER_GEOM_SHADER = {
    
    // uniforms
    {
        {"u_modelView", GLData::Matrix44Float},
        {"u_projMatrix", GLData::Matrix44Float},
        {"u_radius", GLData::Float},
    }, 

    // attributes
    {
    },

    // source
    POLYSCOPE_GLSL(150,
        layout(points) in;
        layout(triangle_strip, max_vertices=32) out;
        in vec3 position_tip[];
        uniform mat4 u_modelView;
        uniform mat4 u_projMatrix;
        uniform float u_radius;
        out vec3 cameraNormal;

        void main()   {
            mat4 PV = u_projMatrix * u_modelView;

            const int nTheta = 8;
            const float PI = 3.14159;
            const float delTheta = 2.*PI / nTheta;            

            // Points along the central axis
            vec3 rootP = gl_in[0].gl_Position.xyz;
            vec3 capP = position_tip[0];
            vec3 shaft = capP - rootP;

            // Orthogonal basis
            const vec3 arbVec = vec3(0.129873, -.70892, .58972);
            vec3 radX = normalize(cross(shaft, arbVec));
            vec3 radY = normalize(cross(shaft, radX));

            // Generate each panel around the vector
            for(int iTheta = 0; iTheta < nTheta; iTheta++) {

                float theta0 = delTheta * iTheta;
                float theta1 = delTheta * (iTheta+1);

                float x0 = cos(theta0);
                float y0 = sin(theta0);
                float x1 = cos(theta1);
                float y1 = sin(theta1);

                vec3 norm0 = (x0 * radX + y0 * radY);
                vec3 norm1 = (x1 * radX + y1 * radY);

                { // Lower left
                    vec4 worldPos = vec4(rootP + norm0 * u_radius, 1.);
                    gl_Position = PV * worldPos;
                    cameraNormal = mat3(u_modelView) * norm0;
                    EmitVertex();
                }
                
                { // Lower right
                    vec4 worldPos = vec4(rootP + norm1 * u_radius, 1.);
                    gl_Position = PV * worldPos;
                    cameraNormal = mat3(u_modelView) * norm1;
                    EmitVertex();
                }
                
                { // Upper left
                    vec4 worldPos = vec4(capP + norm0 * u_radius, 1.);
                    gl_Position = PV * worldPos;
                    cameraNormal = mat3(u_modelView) * norm0;
                    EmitVertex();
                }
                
                { // Upper right
                    vec4 worldPos = vec4(capP + norm1 * u_radius, 1.);
                    gl_Position = PV * worldPos;
                    cameraNormal = mat3(u_modelView) * norm1;
                    EmitVertex();
                }
        
                EndPrimitive();

            }

        }
    )
};



static const FragShader CYLINDER_FRAG_SHADER = {
    
    // uniforms
    {
        {"u_color", GLData::Vector3Float},
    }, 

    // attributes
    {
    },
    
    // textures 
    {
        {"t_mat_r", 2},
        {"t_mat_g", 2},
        {"t_mat_b", 2},
    },
    
    // output location
    "outputF",
 
    // source
    POLYSCOPE_GLSL(150,
        uniform vec3 u_color;
        uniform sampler2D t_mat_r;
        uniform sampler2D t_mat_g;
        uniform sampler2D t_mat_b;
        in vec3 cameraNormal;
        out vec4 outputF;

        // Forward declarations of methods from <shaders/common.h>
        vec4 lightSurfaceMat(vec3 normal, vec3 color, sampler2D t_mat_r, sampler2D t_mat_g, sampler2D t_mat_b);

        void main()
        {
           outputF = lightSurfaceMat(cameraNormal, u_color, t_mat_r, t_mat_g, t_mat_b);
        }
    )
};

// clang-format on

} // namespace gl
} // namespace polyscope
